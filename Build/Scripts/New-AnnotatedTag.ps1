# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

<#
.SYNOPSIS
Creates a new annotated git tag using a VSTS RESTful API call.

.DESCRIPTION
Constructs and invokes the following RESTful API call:
https://www.visualstudio.com/en-us/docs/integrate/api/git/annotatedtags#create-annotated-tag

We do things this way rather than using git directly because of auth issues.
Git repos cloned through automatic builds aren't set up to authenticate against
their remotes, so we can't make any git calls that affect the central repo.
REST calls are very easy to authenticate against from build definitions.
#>
[CmdletBinding()]
Param(
    [Parameter(Mandatory)]
    [String]$TagName,
    [Parameter(Mandatory)]
    [String]$TagMessage,
    [Parameter()]
    [String]$RepoName = $Env:BUILD_REPOSITORY_NAME,
    [Parameter()]
    [String]$Sha1 = $Env:BUILD_SOURCEVERSION
)

$ErrorActionPreference = "stop"
$Verbose = [bool]$PSBoundParameters["Verbose"]

function main {
    if (! $RepoName) {
        Write-Error "Must provide a RepoName if this isn't invoked through VSTS"
    }

    elseif (! $Sha1) {
        Write-Error "Must provide a Sha1 if this isn't invoked through VSTS"
    }
    else
    {
        $uri = "https://microsoft.visualstudio.com/DefaultCollection/Apps/_apis/git/repositories/$RepoName/annotatedTags"
        $body = @{
            "name" = "$TagName"
            "message" = "$TagMessage"
            "taggedObject" = @{
                "objectId" = "$Sha1"
            }
        }

        &"$PSScriptRoot\..\..\XplatScripts\NuGet.macOS\Invoke-VstsRestMethod.ps1" -Uri "$uri" -Body $body -Method "Post" -ApiVersion "4.0-preview" -Verbose
    }
}

main
