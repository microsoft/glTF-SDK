# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

<#
.SYNOPSIS
Push a NuGet package to a feed.

.DESCRIPTION
This script pushes the nuget package to the feed. It takes care of necessary authentication.

.PARAMETER Source
The name of the NuGet source

.PARAMETER Packages
The packages to push
#>

[CmdletBinding()]
Param(
    [Parameter(Mandatory)]
    [String]$Source,
    [Parameter(Mandatory)]
    [String]$Packages
)

$ErrorActionPreference = "stop"

function main
{
    $allPackages = Get-Item $Packages
    foreach($currentPackage in $allPackages)
    {
        dotnet nuget push "$currentPackage" --source "$Source" --api-key VSTS
    }
}

main
