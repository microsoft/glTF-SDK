# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

<# .SYNOPSIS
    Cross-platform XESSerializeForPostbuild script. Checks if this build is the last running build, and then sets XESSERIALPOSTBUILDREADY to true.

    .DESCRIPTION
    In-house implementation of XESSerializeForPostbuild step
    
    .PARAMETER SerializeTaskName
    The name of this serialize task step in VSTS to query for.

    .PARAMETER BuildId
    The id number of the build. By default, is set to env:BUILD_BUILDID

    .PARAMETER AgentName
    The name of the agent that is registered with the pool. By default, set to env:AGENT_NAME

    .PARAMETER TeamCollectionUri
    The uri of the collection, e.g. https://microsoft.visualstudio.com/. By default, is set to env:SYSTEM_TEAMFOUNDATIONCOLLECTIONURI

    .PARAMETER TeamProject
    The name of the project, e.g. Apps. By default, is set to env:SYSTEM_TEAMPROJECT

    .PARAMETER PatPath
    The path to a PAT token for making VSTS REST API calls.
#>

[CmdletBinding()]
Param(
    [Parameter()]
    [string]$SerializeTaskName = $env:TASK_DISPLAYNAME,
    [Parameter()]
    [string]$AgentName = $env:AGENT_NAME,
    [Parameter()]
    [string]$BuildId = $env:BUILD_BUILDID,
    [Parameter()]
    [string]$TeamCollectionUri = $env:SYSTEM_TEAMFOUNDATIONCOLLECTIONURI,
    [Parameter()]
    [string]$TeamProject = $env:SYSTEM_TEAMPROJECT,
    [Parameter()]
    [string]$PatPath
)

$ErrorActionPreference = "stop"
$RestScript = [io.path]::combine($PSScriptRoot, "Invoke-VstsRestMethod.ps1")

function main
{
    Write-Host @"
Agent name: $AgentName
Build id: $BuildId
TeamcollectionUri: $TeamCollectionUri
TeamProject: $TeamProject
Serialize Task Name: $SerializeTaskName
"@
    
    if (isLastBuild)
    {
        # Set the serialize for postbuild env variable to true
        Write-Host @"
This is the last build. Setting 'XESSERIALPOSTBUILDREADY' to true.
##vso[task.setvariable variable=XESSERIALPOSTBUILDREADY]true
"@

    }
    else
    {
        Write-Host "Not the last build. Moving on."
    }
    return
}

function isLastBuild
{
    $buildInfo = getBuildInfo
    while (!(hasSerializeTaskDataInitialized $buildInfo))
    {
        Write-Host "Current machine does not have a start time yet for $SerializeTaskName. Waiting 10 seconds..."
        Start-Sleep -s 10 #poll every 10 seconds 
        $buildInfo = getBuildInfo
    }

    $serializeTaskList = getSerializeTasksFromBuildInfo $buildInfo
    $jobList = getJobsFromBuildInfo $buildInfo

    $numberOfSerializeTasks = $serializeTaskList.Count
    $numberOfJobs = $jobList.Count

    if ($numberOfSerializeTasks -lt $numberOfJobs)
    {
        Write-Host "Not all jobs initialized yet - only $numberOfSerializeTasks of $numberOfJobs"
        return $false
    }

    $nullTaskList = $serializeTaskList | Where-Object {!$_.startTime} | Sort-Object workerName
    $sortedTaskList = $serializeTaskList | Where-Object {$_.startTime -ne $null} | Sort-Object startTime

    Write-Verbose "null task list: $($nullTaskList | Out-String)"
    Write-Verbose "sorted task list: $($sortedTaskList | Out-String)"

    return ($sortedTaskList[-1].workerName -eq $AgentName -and !$nullTaskList)
}

function getBuildInfo
{
    $uri = "$TeamCollectionUri`DefaultCollection/$TeamProject/_apis/build/builds/$BuildId/timeline?api-version=2.0"
    $buildInfo = . $RestScript -Uri $uri -PatPath $PatPath
    
    Write-Verbose "result: $($buildInfo.records | Out-String)"
    return $buildInfo
}

# There can only be one currently-running serialize task for this agent
function hasSerializeTaskDataInitialized($buildInfo)
{
    $serializeTaskList = getSerializeTasksFromBuildInfo $buildInfo
    foreach ($task in $serializeTaskList)
    {
        if ($task.workerName -match $AgentName -and $task.state -match "inProgress")
        {
            return ($task.startTime -ne $null)
        }
    }
    return $false
}

function getSerializeTasksFromBuildInfo($buildInfo)
{
    return $buildInfo.records | Where-Object {$_.type -match "Task" -and $_.name -match $SerializeTaskName}
}

function getJobsFromBuildInfo($buildInfo)
{
    return $buildInfo.records | Where-Object {$_.type -match "Job"}
}

Main