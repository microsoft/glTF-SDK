# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

<#
.SYNOPSIS
Scans the source tree for various file types and modifies their version numbers to all be the same
depending on the version of the build

.DESCRIPTION
Scans the source tree for Windows Store App package.appxmanifest files, CoApp autopkg files and NuGet nuspec files.
For each one modifies the version information stored in the package metadata appropriately.
Nuget and CoApp builds (ie based on autopkg or nuspec files) have their version (xml) tag replaced directly by the
version string. AppxManifest files only have the X.Y.Z modified because they cannot contain any other semantic
information within their version information.

.PARAMETER SourceDirectory 
MANDATORY. The root source tree to scan.

.PARAMETER ModifyWindowsStoreApps
Whether or not to modify the package.appxmanifest files for Windows Store Apps.

.PARAMETER ModifyCoAppPackages
Whether or not to modify the autopkg files for c++ nuget packages generated with CoApp.

.PARAMETER ModifyNuGetPackages
Whether or not to modify the nuspec files for non-c++ nuget packages.

.PARAMETER WriteVersionHeader
Whether or not to modify the SpectreVersionInternal.h header file that is compiled into the Spectre::Utils::GetPackageVersion() method.

.PARAMETER ExcludeFolders
A semi-colon delimited list of folder patterns to ignore.

.PARAMETER VersionString
The version number we will use.

.EXAMPLE
VersionModifier.ps1 -VersionString "1.2.3.4" -SourceDirectory "D:\Build vNext\Builds\cb11432a\Infrastructure\Code" -ModifyWindowsStoreApps true -ExcludeFolders "Microsoft.Lift.WorkflowActivities.Tests"

#>
[CmdletBinding()]
Param(
    [Parameter(Mandatory)][string]$VersionString,
    [string]$SourceDirectory = $env:BUILD_SOURCESDIRECTORY,
    [string]$CommitId = $env:BUILD_SOURCEVERSION,
    [string]$BranchName = $env:BUILD_SOURCEBRANCH,
    [string]$ModifyWindowsStoreApps = $true,
    [string]$ModifyCoAppPackages = $true,
    [string]$ModifyNuGetPackages = $true,
    [string]$WriteVersionHeader = $true,
    [string]$ExcludeFolders
)

$ErrorActionPreference = "Stop"

# Initialize global params
if (!$SourceDirectory) 
{
    $SourceDirectory = (Get-Item $PSScriptRoot).Parent.Parent.FullName
}

[Boolean]$ModifyWindowsStoreApps = [System.Convert]::ToBoolean($ModifyWindowsStoreApps)
[Boolean]$ModifyCoAppPackages = [System.Convert]::ToBoolean($ModifyCoAppPackages)
[Boolean]$ModifyNuGetPackages = [System.Convert]::ToBoolean($ModifyNuGetPackages)
[Boolean]$WriteVersionHeader = [System.Convert]::ToBoolean($WriteVersionHeader)
[Boolean]$IsReleaseVersion = [System.Convert]::ToBoolean($IsReleaseVersion)


function Main
{
    if ($VersionString -notmatch "^((\d+)\.(\d+)\.(\d+)\.(\d+))")
    {
        Write-Error "Version string $VersionString must include 4 digits separated by ."
    }
    $versionNumString = $Matches[1]

    Write-Verbose "Entering script VersionModifier.ps1"
    Write-Verbose "Source Directory = $SourceDirectory"
    Write-Verbose "Full version string is: $VersionString, version numbers are $versionNumString"

    $buildInfo = "Built from git commit $($CommitId) ($BranchName) on build machine $(hostname) at $(Get-Date), version $VersionString"
    Write-Verbose "Build info: $buildInfo"

    if ($ModifyWindowsStoreApps)
    {
        ModifyPackageAppxManifest $versionNumString
    }

    if ($ModifyCoAppPackages)
    {
        ModifyAutoPkgFiles $buildInfo
    }

    if ($ModifyNuGetPackages)
    {
        ModifyNuSpecPackages $buildInfo
    }

    if ($WriteVersionHeader)
    {
        WriteVersionHeader
    }

    Write-Verbose "Exiting script VersionModifier.ps1"
}

<#make sure the reference apps still build
 We aren't going to be publishing any of these (Although it'd be nice if we could, because then we could just run WACK tests real easy)
 So, make sure that the version number in the appxmanifest still allows it to build, check if dashes are allowed and letters
 Want to try to keep the hash in the release version if possible.
 Use our release version, not this funky logic here
 Version in appxmanifest must be in format: "Major.Minor.Build.Revision". Value must contain a valid four-part version number. 
# Reads the version number from the given package.appxmanifest
# Modifies the version numbers in package.appxmanifest files
#>
function ModifyPackageAppxManifest
{
    Foreach ($file in GetFileWrapper "*.appxmanifest")
    {
        Write-Verbose $file
            
        [xml]$appxData = Get-Content $file

        Write-Verbose "Replacing $($appxData.Package.Identity.Version) with $VersionString"
            
        $appxData.Package.Identity.Version = $VersionString
        $appxData.Save($file)
    }
}

# Reads various properties from the given nuspec file and returns them in a hash-table
# Writes the new version number, tags and releaseNotes (the build info) into the nuSpec file
# Modifies the version numbers in nuspec files
function ModifyNuSpecPackages($buildInfo)
{
    Foreach ($file in GetFileWrapper "*.nuspec")
    {
        Write-Verbose $file

        [xml]$nuspecData = Get-Content $file

        Write-Verbose "Replacing version $($nuspecData.package.metadata.version) with $VersionString"
        Write-Verbose "Replacing release notes with: $buildInfo"
            
        $nuspecData.package.metadata.version = $VersionString
        $nuspecData.package.metadata.releaseNotes = $buildInfo

        foreach ($dependency in $nuspecData.package.metadata.dependencies.dependency) {
            if (IsPackageNameInternal $dependency.id) {
                Write-Verbose "Replacing dependency $($dependency.id) version $($dependency.version) with $VersionString"
                $dependency.version = $VersionString
            }
        }

        $nuspecData.Save($file)
    }
}

# Modifies the version numbers in autopkg files
function ModifyAutoPkgFiles($buildInfo)
{
    Foreach ($file in GetFileWrapper "*.autopkg")
    {
        Write-Verbose $file

        $newContent = ""
        Foreach ($line in Get-Content $file)
        {
            # Match line of the form "<whitespace?>version<whitespace?>:<whitespace?><versionString><whitespace?><semicolon>"
            if ($line -match "(\s+)?(version)(\s+)?:(\s+)?([\S\.]+)(\s+)?;")
            {
                Write-Verbose "Replacing $oldVersion with $VersionString"
                $line = $line.Replace($matches[5], $VersionString)
            }
            # Match dependency line of the form "Microsoft.Lift.*/<version>;" or "Microsoft.Lift.*/<version>,"
            elseif ($line -match "^\s+(\S+?)\/([0-9\.]+)[;,]$" -and (IsPackageNameInternal $matches[1]))
            {
                $packageName = $matches[1]
                $oldVersion = $matches[2]
                Write-Verbose "Replacing dependency $packageName version $oldVersion with $VersionString"
                $line = $line.Replace($oldVersion, $VersionString)
            }
            # Match "releaseNotes" line which uses a string enclosed in double-quotes with an optional at symbol at the start
            elseif ($line -match "(\s+)?(releaseNotes)(\s+)?:(\s+)?(@)?\""(.*)\""(\s+)?;")
            {
                # Replace releaseNotes with new value (the build info). We don't do a blind replace of the old release notes with the new because that fails if
                # the existing release notes are a substring that is found elsewhere in the line. Instead we reconstruct the string, including whitespace and
                # optional @ symbol, but replacing the current release notes section with the new one.
                    
                Write-Verbose "Replacing release notes with: $buildInfo"
                $line = "$($matches[1])$($matches[2])$($matches[3]):$($matches[4])$($matches[5])""$buildInfo""$($matches[7]);"
            }
            $newContent += "$line`n"
        }

        Set-Content $file $newContent
    }
}

function IsPackageNameInternal([String]$packageName) {
    return $packageName.StartsWith("Microsoft.Lift")
}

# Writes Version.h file in nuspec files
# [void] prevents stringBuilder.Append from outputting capacity/length information
function WriteVersionHeader
{
	$outPath = $SourceDirectory + "\Built\Int\";
	$outFile = $outPath + "SpectreVersionInternal.h";

	# This just writes SpectreVersionInternal.h into the intermediate folder
	Write-Verbose "Writing $outFile with SPECTRE_PACKAGE_VERSION = $VersionString"

	# just overwrite the whole .h file
    $stringBuilder = New-Object System.Text.StringBuilder
	[void]$stringBuilder.Append("// Copyright (c) Microsoft Corporation. All rights reserved." + [Environment]::NewLine)
	[void]$stringBuilder.Append("// Licensed under the MIT License."                           + [Environment]::NewLine)
	[void]$stringBuilder.Append([Environment]::NewLine)
	[void]$stringBuilder.Append("#pragma once" + [Environment]::NewLine)
	[void]$stringBuilder.Append([Environment]::NewLine)
	[void]$stringBuilder.Append("// This file is autogenerated by VersionModifier.ps1" + [Environment]::NewLine)
	[void]$stringBuilder.Append([Environment]::NewLine)
	[void]$stringBuilder.Append("#define SPECTRE_PACKAGE_VERSION " + '"' + $VersionString + '"' + [Environment]::NewLine)

	$item = New-Item $outPath -ItemType Directory -Force
	Out-File -encoding "UTF8" -filepath $outFile -inputobject $stringBuilder.ToString() -force
}

# This function is a wrapper around Get-ChildItem so we can mock the return
function GetFileWrapper($searchPattern)
{
    $files = Get-ChildItem $SourceDirectory -Recurse -Include $searchPattern
    $fileNames = $files | Foreach-Object { $_.FullName }
    return $fileNames | Where-Object { ! (ShouldIgnoreFile $_) }
}

function ShouldIgnoreFile($fileName)
{
    foreach ($excludeFolder in ($ExcludeFolders.Split(";") | Where-Object { $_ }))
    {
        if ($fileName.Contains($excludeFolder))
        {
            Write-Verbose "Ignoring $fileName due to exclusion folder $ignore"
            return $true
        }
    }

    return $false
}

Main
