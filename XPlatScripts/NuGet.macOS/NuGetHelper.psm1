# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

<#
.SYNOPSIS
Helper module for using NuGet on MacOS.

.DESCRIPTION
Provides common abd useful functions to be used by the scripts.
#>

# ------------

# Write out a NuGet config file with packageSourceCredentials added in
function Install-NuGetConfig
{
    Param(
        [Parameter(Mandatory)]
        [String]$NuGetConfigSource,
        [Parameter()]
        [String]$NuGetConfigDest = [io.path]::GetTempFileName(),
        [Parameter()]
        [String]$AccessToken,
        [Parameter()]
        [String]$PATFilename
    )

    Copy-Item $NuGetConfigSource $NugetConfigDest | Out-Null

    return $NugetConfigDest
}

# Obtain personal access token from commandline or pat.txt
function getAccessToken($AccessToken, $PATFilename)
{
    if (-not [String]::IsNullOrEmpty($AccessToken))
    {
        Write-Verbose "Using commandline-supplied access token"
        return $AccessToken
    }

    if (Test-Path $PATFilename)
    {
        Write-Verbose "Using access token from file $PATFilename"
        return Get-Content $PATFilename
    }

    Write-Verbose "Continuing without an access token"
    return $null
}

function Start-NuGet
{
    $nugetCommandLocations = @(
        "nuget",
        "/usr/local/bin/nuget",
        "/Library/Frameworks/Mono.framework/Versions/Current/Commands/nuget"
    )

    $nugetCommand = $nugetCommandLocations | Where-Object { NugetCommandIsValid $_ } | Select -First 1
    Write-Verbose "Using nuget at: $nugetCommand"

    if (-not $nugetCommand)
    {
        Write-Error "NuGet not found."
    }

    Write-Verbose "Arguments = $args"
    Start-Process -FilePath $nugetCommand -Wait -Args $args
}

function NugetCommandIsValid($nuget, $minVerNumber = "4.0.0.0")
{
    if (!(Get-Command $nuget -ErrorAction SilentlyContinue))
    {
        return $false
    }

    $nugetVersionString = . $nuget "help" | Select -First 1
    
    if ($nugetVersionString -match ".+((\d+)\.(\d+)\.(\d+)\.(\d+))")
    {
        $versionString = $matches[1]
        Write-Verbose "Nuget found at $nuget with version number $versionString. Comparing to minimum version $minVerNum"

        return ([Version]$versionString -ge [Version]$minVerNum)
    }

    Write-Error "Nuget command provided at path $nuget does not have `"nuget help`" command that lists a version number."
}

Export-ModuleMember -function Install-NuGetConfig
Export-ModuleMember -function Start-NuGet
