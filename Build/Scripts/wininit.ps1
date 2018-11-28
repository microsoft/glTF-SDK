# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

<#
.SYNOPSIS
Setup the Visual Studio solution for building on Windows.
This should be run after every git pull or clean and should be safe to run any time.

.DESCRIPTION
Runs the cmake build required to generate the solutions for building all variants of the GLTFSDK on Windows.
Regenerates SpectreVersionInternal.h

.PARAMETER Clean
Clean the output folders before building

.EXAMPLE
wininit.ps1
#>

[CmdletBinding()]
param(
    [switch]$Clean,
    [switch]$NoArm,
    [switch]$NoArm64,
    [switch]$Nox64,
    [switch]$NoWin32
)

$ErrorActionPreference = "stop"

function CleanFiles()
{
    Remove-Item "$PSScriptRoot/../../Built" -Recurse -Force -ErrorAction Ignore | Write-Host
    Remove-Item "$PSScriptRoot/../../packages" -Recurse -Force -ErrorAction Ignore | Write-Host
}

function GeneratePlatform($platform, $path)
{
    Write-Host "Generate $platform Solution"
    New-Item -Path "$PSScriptRoot/../../Built/Int" -Name $path -ItemType Directory -Force | Out-Null
    Push-Location "$PSScriptRoot/../../Built/Int/$path" | Out-Null
    try
    {
        $argList = @(
            "-G", "Visual Studio 15 2017",
            "-A", "$platform"
        )

        & cmake $argList "..\..\.."
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

    if (!$NoWin32)
    {
        GeneratePlatform "Win32" "cmake_Win32"
    }

    if (!$Nox64)
    {
        GeneratePlatform "x64" "cmake_x64"
    }
    
    if (!$NoArm)
    {
        GeneratePlatform "ARM" "cmake_ARM"
    }
    
    if (!$NoArm64)
    {
        GeneratePlatform "ARM64" "cmake_ARM64"
    }
}

Main
