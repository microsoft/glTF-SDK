# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

<#
.SYNOPSIS
Create NuGet access token file from plaintext PAT.

.DESCRIPTION
macOS credentials for NuGet do not work in the same way as
Windows due to OS support differences.  We create a file
~/.nuget/pat.txt to hold the required personal access token
to be used by NuGet.

This file should be encrypted but currently is not, again due
to OS support issues.

.PARAMETER AccessToken
Takes a plaintext PersonalAccessToken and writes it out to PATFilename.
Note: macOS Powershell does not implement required functionality to
encrypt the PAT so is stored in plaintext.

.PARAMETER PATFilename
The filename to write the credentials (access token) to.  Default is
~/.nuget/pat.txt
#>

[CmdletBinding()]
Param(
    [Parameter(Mandatory)]
    [string]$AccessToken,
    [string]$PATFilename = '~/.nuget/pat.txt'
)

$ErrorActionPreference = "Stop"

# create pat directory if it doesn't exist
$parentDir = Split-Path -Parent $PATFilename
New-Item $parentDir -type directory -force | Out-Null

# write token to file
$AccessToken | Out-File $PATFilename
Write-Host "Wrote access token to $PATFilename"
