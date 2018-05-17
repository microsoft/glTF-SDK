# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

<#
.SYNOPSIS
Restore NuGet packages and generate XCode config for search paths.

.DESCRIPTION
This script recursively searches for packages.config files in the
filesystem, runs NuGet to restore the packages in them one by one,
then writes out an XCode config file that sets search paths properly.

.PARAMETER XCodeConfigFile
The path to the XCode config file we'll create.

.PARAMETER PackagesDirectory
The directory where NuGet packages will be restored to.

.PARAMETER PackagesConfigRoot
The root directory where the recursive search for packages.config files
will begin.

.PARAMETER NuGetConfigSource
The path to the Nuget.Config file we'll use as a starting point for creating
our nuget.config file with credentials attached.

.PARAMETER AccessToken
Clear text PersonalAccessToken to use for credentials.

.PARAMETER PATFilename
File containing PersonalAccessToken, default ~/.nuget/pat.txt
#>

[CmdletBinding()]
Param (
    [Parameter(Mandatory)]
    [String]$XCodeConfigFile,
    [Parameter(Mandatory)]
    [String]$PackagesDirectory,
    [Parameter(Mandatory)]
    [String]$PackagesConfigRoot,
    [Parameter(Mandatory)]
    [String]$NugetConfigSource,
    [Parameter()]
    [String]$AccessToken,
    [Parameter()]
    [string]$PATFilename = '~/.nuget/pat.txt'
)

$ErrorActionPreference = "stop"

Import-Module (Join-Path $PSScriptRoot "NuGetHelper.psm1") -Force

function main
{
    $temporaryNugetConfig = Install-NuGetConfig -AccessToken $AccessToken -PATFilename $PATFilename -NuGetConfigSource $NugetConfigSource

    try
    {
        nugetRestore $temporaryNugetConfig
    }
    finally
    {
        Remove-Item -Path $temporaryNugetConfig -ErrorAction "SilentlyContinue"
    }

    createCmakeListsFile
}

# NuGet restore on all package.config files under $PackagesConfigRoot
function nugetRestore($nugetConfigPath)
{
    foreach ($configFile in getPackageConfigFiles)
    {
        Start-NuGet restore $configFile -ConfigFile $nugetConfigPath -OutputDirectory $PackagesDirectory -DisableParallelProcessing
    }
}

# Parse packages.config file and save search path keys for later
function parsePackagesXml($packagesFile, $xcodeConfigs)
{
    # Read in config as XML
    [xml]$xml = Get-Content $packagesFile

    # Add config keys uniquely
    foreach ($package in $xml.packages.package)
    {
        $id = $package.id
        $idup = $id.ToUpper() -replace '\.','_'
        $key = "NUGET_$idup"

        if (!$xcodeConfigs.ContainsKey($key))
        {
            $version = $package.version
            $val = Join-Path -Path $PackagesDirectory -ChildPath "$id"
            $xcodeConfigs.Add($key, $val)
        }
    }
}

function getPackageConfigFiles
{
    return (Get-ChildItem $PackagesConfigRoot -Recurse packages.config).FullName
}

function createCmakeListsFile
{
    $cmakeListsFile = Join-Path $PackagesDirectory "CMakeLists.txt"
    printCmakeListsFile | Out-File $cmakeListsFile -Force -Encoding ASCII
}

function getNormalizedVersion($version)
{
    $release,$prerelease = $version.split("-", 2)
    $parts = $release.split(".")
    while ($parts.length -gt 3 -and $parts[-1] -eq 0)
    {
        $parts = $parts[0..($parts.length-2)]
    }
    $release = $parts -join "."

    return $release, $prerelease -ne $null -join "-"
}

function printCmakeListsFile
{
    foreach ($configFile in getPackageConfigFiles)
    {
        [xml]$config = Get-Content $configFile
        foreach ($package in $config.SelectNodes("/packages/package"))
        {
            $version = getNormalizedVersion $package.getAttribute("version")
            $name = $package.getAttribute("id")
            $path = Join-Path $PackagesDirectory ($name + "." + $version) | Resolve-Path
            # Convert Windows style path with "\" to Unix style path using "/" as delimiter, otherwise CMake complains on Windows host
            $path = $path -replace "\\", "/"

            Write-Verbose "list(APPEND CMAKE_MODULE_PATH `"$path`")"
            Write-Output "list(APPEND CMAKE_MODULE_PATH `"$path`")"
        }
    }

    # Pass back the new CMAKE_MODULE_PATH to the parent scope
    Write-Output "set(CMAKE_MODULE_PATH `${CMAKE_MODULE_PATH} PARENT_SCOPE)"
}

Main
