# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

# Based on the build reason, modify the build output location and set some 
# default VSTS variables that will control behavior. If these variables have
# been explicitly set this script won't override them.
# See build.reason at: https://www.visualstudio.com/en-us/docs/build/define/variables
[CmdletBinding()]

param(
    [string]$SourceDirectory = $env:BUILD_SOURCESDIRECTORY,
    [string]$CommitId = $env:BUILD_SOURCEVERSION,
    [string]$BranchName = $env:BUILD_SOURCEBRANCH,
    [string]$VersionOverride,
    [string]$GenerateReleaseVersion = $false,
    [string]$ModifyWindowsStoreApps = $true, 
    [string]$ModifyCoAppPackages = $true,
    [string]$ModifyNuGetPackages = $true, 
    [string]$WriteVersionHeader = $true, 
    [string]$ExcludeFolders
)

$ErrorActionPreference = "Stop"

function Main
{
    $versionString = . "$PSScriptRoot\VersionGenerator.ps1" -SourceDirectory $SourceDirectory -VersionOverride $VersionOverride -GenerateReleaseVersion $GenerateReleaseVersion -Verbose
    . "$PSScriptRoot\VersionModifier.ps1" -SourceDirectory $SourceDirectory -CommitId $CommitId -BranchName $BranchName -ExcludeFolders $ExcludeFolders -VersionString $versionString -ModifyWindowsStoreApps $ModifyWindowsStoreApps -ModifyCoAppPackages $ModifyCoAppPackages -ModifyNuGetPackages $ModifyNuGetPackages -WriteVersionHeader $WriteVersionHeader -Verbose
}

Main
