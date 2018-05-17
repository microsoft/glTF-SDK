<#
/**************************************************************
*                                                             *
#  Copyright (c) Microsoft Corporation. All rights reserved.  *
#  Licensed under the MIT License.                            *
*                                                             *
**************************************************************/
.SYNOPSIS
Invokes CMake to build GLTFSDK for android targets.
This should be run after every git pull or clean and should be safe to run any time.

.DESCRIPTION
It first creates ninja files as native build target. Then invokes ninja to kick off
actual build process.
Assumes, CMake, ninja and Android-ndk are available.

.EXAMPLE
androidbuild.ps1 -arm64
#>

[CmdletBinding()]
param(
    [switch]$Clean,
    [switch]$Rebuild,
    [switch]$NoUnitTests,
    [switch]$arm64,
    [switch]$arm32,
    [switch]$x86,
    [switch]$x86_64,
    [switch]$NoDebug
)

$ErrorActionPreference = "stop"
if($NoDebug) {$BuildType = "Release"} else { $BuildType = "Debug"}

function GenerateNinjaFiles()
{
    $AndroidABI = findABI
    $BuildDirName = getBuildDirName
    Write-Host "Generating Android ninja files for $AndroidABI"
    New-Item -Path "$PSScriptRoot\Built\Int" -Name $BuildDirName -ItemType Directory -Force | Out-Null
    Push-Location "$PSScriptRoot\Built\Int\$BuildDirName" | Out-Null

    try
    {
        if (Test-Path Env:ANDROID_HOME)
        {
            $AndroidNDKRoot = "$Env:ANDROID_HOME\ndk-bundle"
        }
        else
        {
            $Appdata = [Environment]::GetFolderPath('ApplicationData')
            $AndroidNDKRoot = "$Appdata\..\Local\Android\Sdk\ndk-bundle"
        }

        $AndroidToolChain = "$AndroidNDKRoot\build\cmake\android.toolchain.cmake"
        $AndroidPlatform = "android-19"

        if ($NoUnitTests)
        {
            $ENABLE_UNIT_TESTS = "OFF"
        }
        else
        {
            $ENABLE_UNIT_TESTS = "ON"
        }

        cmake ..\..\.. -DANDROID_ABI="$AndroidABI" -DANDROID_PLATFORM="$AndroidPlatform" -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=intermediates -DCMAKE_BUILD_TYPE="$BuildType" -DANDROID_NDK="$AndroidNDKRoot" -DCMAKE_TOOLCHAIN_FILE="$AndroidToolChain" -DCMAKE_CXX_FLAGS=-fexceptions -DANDROID_STL=c++_static -GNinja -DANDROID_OS_PLATFORM=ANDROID -DENABLE_UNIT_TESTS="$ENABLE_UNIT_TESTS" | Write-Host
    }
    finally
    {
        Pop-Location | Out-Null
    }
}

function BuildTarget()
{
    $BuildDirName = getBuildDirName
    Push-Location "$PSScriptRoot\Built\Int\$BuildDirName" | Out-Null
    try
    {
        cmake --build . --target install --config "$BuildType"
    }
    finally
    {
        Pop-Location | Out-Null
    }
}

function cleanAllTargets()
{
    Remove-Item "$PSScriptRoot\Built" -Recurse -Force -ErrorAction Ignore | Write-Host
}

function cleanTarget()
{
    # Delete both compilation and installation directories.
    $BuildDirName = getBuildDirName
    Remove-Item "$PSScriptRoot\Built\Int\$BuildDirName" -Recurse -Force -ErrorAction Ignore | Write-Host
    Remove-Item "$PSScriptRoot\Built\Out\$BuildDirName\$BuildType" -Recurse -Force -ErrorAction Ignore | Write-Host
}

function findABI()
{
    $ABI = "x86"
    if($arm32)
    {
        $ABI = "armeabi-v7a"
    }
    elseif($arm64)
    {
        $ABI = "arm64-v8a"
    }
    elseif($x86_64)
    {
        $abi = "x86_64"
    }
    return $ABI
}

function getBuildDirName()
{
    $AndroidABI = findABI
    $BuildDirName = "android_$AndroidABI"
    return $BuildDirName
}

function Main()
{
    if($Clean)
    {
        cleanTarget
    }
    else
    {
        if($Rebuild)
        {
            cleanTarget
        }
        GenerateNinjaFiles
        BuildTarget
    }
}

Main
