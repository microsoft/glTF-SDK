<#
.SYNOPSIS
Gets nuget.exe location.

.DESCRIPTION
Looks for nuget.exe in multiple program file locations.

.PARAMETER NuGetLocations
Additional locations in which to search for nuget.exe

.PARAMETER MinVerNumber
Minimum version number that the nuget command must be

#>
[CmdletBinding()]

Param(
    [Parameter()]
    [string[]]$NuGetLocations,
    [Parameter()]
    [string]$MinVerNumber = "4.0.0.0"
)

$ErrorActionPreference = "Stop"

function main
{
    $searchLocations = $NuGetLocations + @(
        "nuget",
        "/usr/local/bin/nuget",
        "/Library/Frameworks/Mono.framework/Versions/Current/Commands/nuget"
    )

    return ($searchLocations | Where-Object { NugetCommandIsValid $_ } | Select -First 1)
}

function NugetCommandIsValid($nuget)
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

main