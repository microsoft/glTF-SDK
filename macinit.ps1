# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

<#
.SYNOPSIS
Setup the xcode workspace for building on MacOS.
This should be run after every git pull or clean and should be safe to run any time.

.DESCRIPTION
Fetches NuGet packages and updates any configuration required for the xcode workspace to load and build.
Could go so far as to update or generate the xcode projects if needed.

.EXAMPLE
macosini.ps1
#>

[CmdletBinding()]
param(
    [switch]$Clean,
    [switch]$NoMacOS,
    [switch]$NoIOS,
    [switch]$NoSimulator,
    [switch]$NoUnitTests
)

$ErrorActionPreference = "stop"

if ($NoUnitTests) {$ENABLE_UNIT_TESTS = "OFF"} else {$ENABLE_UNIT_TESTS = "ON"}

function CleanFiles()
{
    Remove-Item "$PSScriptRoot/Build/packages" -Recurse -Force -ErrorAction Ignore | Write-Host
    Remove-Item "$PSScriptRoot/Built" -Recurse -Force -ErrorAction Ignore | Write-Host
}

function RestoreNuGet()
{
    Write-Host "Restoring NuGet packages"
    &"$PSScriptRoot/XPlatScripts/NuGet.macOS/NuGetRestore.ps1" -XCodeConfigFile "$PSScriptRoot/Built/Int/XCode.xcconfig" -PackagesDirectory "$PSScriptRoot/Build/packages" -PackagesConfigRoot "$PSScriptRoot/Build/macOSPackageRoot" -NugetConfigSource "$PSScriptRoot/NuGet.Config"
}

function GenerateSpectreVersion()
{
    Write-Host "Generating SpectreVersionInternal.h"
    &"$PSScriptRoot/Build/Scripts/VersionGeneratorAndModifier.ps1" -sourceDirectory $PSScriptRoot -Exclude "$PSScriptRoot/Build/packages;$PSScriptRoot/Build/NuGet/FBX" -ModifyCoAppPackages false -ModifyNuGetPackages false -ModifyWindowsStoreApps false -WriteVersionHeader true
}

function GenerateProjectsMacOS()
{
    Write-Host "Generate MacOS Projects"
    New-Item -Path "$PSScriptRoot/Built/Int" -Name cmake_macos -ItemType Directory -Force | Out-Null
    Push-Location "$PSScriptRoot/Built/Int/cmake_macos" | Out-Null
    try
    {
        cmake -G Xcode ../../.. -DENABLE_UNIT_TESTS="$ENABLE_UNIT_TESTS" | Write-Host
    }
    finally
    {
        Pop-Location | Out-Null
    }
}

function GenerateProjectsIOS()
{
    Write-Host "Generate iOS Projects"
    New-Item -Path "$PSScriptRoot/Built/Int" -Name cmake_ios -ItemType Directory -Force | Out-Null
    Push-Location "$PSScriptRoot/Built/Int/cmake_ios" | Out-Null
    try
    {
        cmake -G Xcode ../../.. -DCMAKE_TOOLCHAIN_FILE="$PSScriptRoot/Build/CMake/ios.toolchain.cmake" -DIOS_PLATFORM=OS -DIOS_DEPLOYMENT_TARGET="9.0" -DENABLE_UNIT_TESTS="$ENABLE_UNIT_TESTS" | Write-Host
    }
    finally
    {
        Pop-Location | Out-Null
    }
}

function GenerateProjectsIOSSimulator()
{
    Write-Host "Generate iOS Simulator Projects"
    New-Item -Path "$PSScriptRoot/Built/Int" -Name cmake_ios_simulator -ItemType Directory -Force | Out-Null
    Push-Location "$PSScriptRoot/Built/Int/cmake_ios_simulator" | Out-Null
    try
    {
        cmake -G Xcode ../../.. -DCMAKE_TOOLCHAIN_FILE="$PSScriptRoot/Build/CMake/ios.toolchain.cmake" -DIOS_PLATFORM=SIMULATOR64 -DIOS_DEPLOYMENT_TARGET="9.0" -DENABLE_UNIT_TESTS="$ENABLE_UNIT_TESTS" | Write-Host
    }
    finally
    {
        Pop-Location | Out-Null
    }
}

function Main()
{
    if ($Clean)
    {
        CleanFiles
    }
    RestoreNuGet
    GenerateSpectreVersion

    if (!$NoMacOS)
    {
        GenerateProjectsMacOS
    }

    if (!$NoIOS)
    {
        GenerateProjectsIOS
    }

    if (!$NoSimulator)
    {
        GenerateProjectsIOSSimulator
    }

}

Main
