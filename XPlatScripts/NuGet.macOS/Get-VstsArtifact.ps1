# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

<#
.SYNOPSIS
Download a VSTS build artifact.

.PARAMETER BuildId
The unique ID (NOT build number) of the build containing the artifact we want to download.

.PARAMETER ArtifactName
The name of the artifact we want to download.

.PARAMETER ItemPath
Supplying this parameter allows downloading of sub folder or file of an artifact. The item path starts from the root of the artifact folder

.PARAMETER OutPath
The local filesystem path we'll extract the artifact to.

.PARAMETER TeamCollectionUri
The uri of the collection, e.g. https://microsoft.visualstudio.com/.
Is set to the current collection if invoked through a build.

.PARAMETER TeamProject
The name of the project, e.g. Apps.
Is set to the current project if invoked through a build.

.PARAMETER PatPath
The path to a PAT token for making VSTS REST API calls.

.PARAMETER MaxTries
The maximum number of times we'll try downloading the artifact before giving up.
#>
[CmdletBinding()]
Param(
    [Parameter(Mandatory)]
    [String]$BuildId,
    [Parameter(Mandatory)]
    [String]$ArtifactName,
    [Parameter()]
    [String]$ItemPath,    
    [Parameter(Mandatory)]
    [String]$OutPath,
    [Parameter()]
    [String]$TeamCollectionUri = $env:SYSTEM_TEAMFOUNDATIONCOLLECTIONURI,
    [Parameter()]
    [String]$TeamProject = $env:SYSTEM_TEAMPROJECT,
    [Parameter()]
    [String]$PatPath = $env:VSTS_PAT_PATH,
    [Parameter()]
    [Int]$MaxTries = 10
)

$ErrorActionPreference = "stop"
$Verbose = [bool]$PSBoundParameters["Verbose"]
$GetAuthString = Join-Path $PSScriptRoot "Get-VstsRestAuthString.ps1"
$InvokeVsts = Join-Path $PSScriptRoot "Invoke-VstsRestMethod.ps1"
$InvokeWithRetries = Join-Path $PSScriptRoot "Invoke-CommandWithRetries.ps1"

Add-Type -AssemblyName System.IO.Compression.FileSystem

function main
{
    $tempFile = [IO.Path]::GetTempFileName()
    try
    {
        $downloadUrl = getDownloadUrl
        downloadArtifact $downloadUrl $tempFile    

        Write-Verbose "Extracting $tempFile to $OutPath"
        [IO.Compression.ZipFile]::ExtractToDirectory($tempFile, $OutPath)        

        # If $ItemPath is defined then move the subfolder $ItemPath into the root of $OutPath
        # If $ItemPath is not defined then retain the full paths for backward compatibility.
        if($ItemPath)
        {            
            $toMove = "$OutPath/$ArtifactName/$ItemPath"
            Move-Item $toMove $OutPath -Force
            Remove-Item "$OutPath/$ArtifactName" -Force -Recurse
        }
        
    }
    finally
    {
        Remove-Item $tempFile -Force
    }

}

function getDownloadUrl
{   

    $uri = "$TeamCollectionUri`DefaultCollection/$TeamProject/_apis/build/builds/$BuildId/artifacts"
    $artifacts = . $InvokeVsts -Uri $uri -ApiVersion "2.0" -PatPath $PatPath -Verbose:$Verbose
    $artifact = $artifacts.value | Where-Object { $_.name -eq $ArtifactName }
    if (! $artifact)
    {
        Write-Error "No artifact with name $ArtifactName"
    }

    $downloadUrl = $artifact.resource.downloadUrl
    if (! $downloadUrl)
    {
        Write-Error "Artifact $ArtifactName doesn't have a download URL. Is it a UNC drop?"
    }

    # VSTS Artifact API does not explicitly support downloading sub folders or files, this generates the link VSTS would provide through the artifacts page
    if($ItemPath)
    {
        $artifactItemPath = [uri]::EscapeDataString($ItemPath)       
        
        $artifactId = ($artifact.resource.data -split "/")[1]
        return "${TeamCollectionUri}_apis/resources/Containers/${artifactId}?itemPath=${ArtifactName}%2F${artifactItemPath}&%24format=zip"
    }

    return $downloadUrl
}

function downloadArtifact($downloadUrl, $outFile)
{
    # Using WebClient because Invoke-WebRequest is WAAAY too slow.
    # TODO: Port Invoke-VstsRestMethod to use WebClient, and then use Invoke-VstsRestMethod here.
    Write-Verbose "Downloading ${ArtifactName}: $downloadUrl => $outFile"
    $client = New-Object System.Net.WebClient
    $client.Headers.Add("Authorization", (. $GetAuthString -PatPath $PatPath -Verbose:$Verbose))
    . $InvokeWithRetries -MaxTries $MaxTries -IsErrorRetriable ${function:isErrorRetriable} -Verbose:$Verbose -Command {
        $client.DownloadFile($downloadUrl, $outFile)
    }
}

function isErrorRetriable([Management.Automation.ErrorRecord]$err)
{
    $statusCode = $err.Exception.Response.StatusCode
    return $statusCode -and ([int]$statusCode -ge 500)
}

main
