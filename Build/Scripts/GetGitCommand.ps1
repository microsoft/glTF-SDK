# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

<#
.SYNOPSIS
Gets Git.exe location.

.DESCRIPTION
Looks for git.exe in multiple program file locations.

.PARAMETER GitLocations
Additional locations in which to search for git.exe

#>
[CmdletBinding()]
Param(
    [Parameter()]
    [string[]]$GitLocations
)

function main {
    $searchLocations = $GitLocations + @("git", "${env:ProgramFiles(x86)}\Git\bin\git.exe", "${env:ProgramFiles}\Git\bin\git.exe", "${env:ProgramW6432}\Git\bin\git.exe")
    return ($searchLocations | ? { Get-Command $_ -ErrorAction SilentlyContinue } | Select -First 1)
}

main
