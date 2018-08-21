# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

# Based on the build reason, modify the build output location and set some 
# default VSTS variables that will control behavior. If these variables have
# been explicitly set this script won't override them.
# See build.reason at: https://www.visualstudio.com/en-us/docs/build/define/variables
[CmdletBinding()]
Param(
    [Parameter(Mandatory)]
    [String]$BuildDefinitionBase,
    [Parameter()]
    [String]$BuildReason
)

$ErrorActionPreference = "Stop"

if (! $BuildReason) {
    $BuildReason = $Env:BUILD_REASON

    if (! $BuildReason) {
        Write-Error "BuildReason must be provided if this script wasn't invoked from VSTS"
    }
}

function main {
    $buildConfig = getConfigForBuildReason $BuildReason

    # We want to update <definition name> to be $base.$qualifier so e.g. PR builds go to a
    # separate path. We are actually updating the reserved variable that indicates the build
    # definition name, but it doesn't seem to have any adverse effects.
    setVstsVariable "Build.DefinitionName" "${BuildDefinitionBase}.$($buildConfig.name)"

    setDefaultVariables $buildConfig.config
}

function getConfigForBuildReason([String]$buildReason) {
    $allConfig = Get-Content "$PSScriptRoot\SetBuildVariables.json" | ConvertFrom-Json
    foreach ($config in $allConfig) {
        foreach ($reason in $config.reasons) {
            if ($reason -match $buildReason) {
                Write-Verbose "Reason '$buildReason' matches $($config | ConvertTo-Json)"
                return $config
            }
        }

        Write-Verbose "Reason '$buildReason' does not match $($config | ConvertTo-Json)"
    }

    Write-Error "Unable to find a config matching reason $buildReason."
}

function setDefaultVariables([PSCustomObject]$config) {
    foreach ($configEntry in $config.PSObject.Properties) {
        setDefaultVstsVariable $configEntry.Name $configEntry.Value
    }
}

function setDefaultVstsVariable([String]$name, [String]$value) {
    $setValue = (Get-Item -ErrorAction SilentlyContinue "Env:$name").value
    if ($setValue) {
        Write-Verbose "Variable $name is already set to $setValue; not overriding it."
        return
    }
    setVstsVariable $name $value
}

# https://github.com/Microsoft/vsts-tasks/blob/master/docs/authoring/commands.md
function setVstsVariable([String]$name, [String]$value) {
    Write-Verbose "Setting $name to $value"
    Write-Host "##vso[task.setvariable variable=$name;]$value"
}

main
