# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

<#
.SYNOPSIS
Makes a VSTS RESTful API call.

.DESCRIPTION
This method is mostly just a wrapper around Invoke-RestMethod. It sets up some
VSTS-specific defaults and automatically sets up authentication if invoked from
a build definition that has 'Allow Scripts to Access OAuth Token' checked.

.PARAMETER Uri
The full URL of the RESTful call we're making.

.PARAMETER Method
The HTTP method to use in this call.

.PARAMETER Body
Data we'll include in our request. If it isn't a string we'll convert to json before sending it.

.PARAMETER ApiVersion
The version of the RESTful method we're using. See: https://www.visualstudio.com/en-us/docs/integrate/get-started/rest/basics#versions

.PARAMETER PatPath
Should only be used when called locally. The path to a file containing an encrypted PAT token. You can also provide a PatPath by setting the environment variable VSTS_PAT_PATH.

Instructions for setting this up locally are here: https://microsoft.sharepoint.com/:o:/r/teams/segcentral/_layouts/15/WopiFrame.aspx?sourcedoc=%7B817af5db-69b2-413b-a333-4acfb9a5c0bf%7D

.PARAMETER FullResponse
By default, we return the response body as a JSON object. If FullResponse is set, we return the full Invoke-WebRequest response.

.PARAMETER MaxTries
We retry with incremental backoff if VSTS returns a 500. MaxTries is maximum number of times we'll retry before giving up.
#>
[CmdletBinding()]
Param(
    [Parameter(Mandatory)]
    [String]$Uri,
    [Parameter()]
    [String]$Method = "Default",
    [Parameter()]
    $Body,
    [Parameter()]
    [String]$ApiVersion,
    [Parameter()]
    [String]$PatPath = $env:VSTS_PAT_PATH,
    [Parameter()]
    [Switch]$FullResponse,
    [Parameter()]
    [Int]$MaxTries = 10
)

$ErrorActionPreference = "stop"
$Verbose = [bool]$PSBoundParameters["Verbose"]
$GetAuthString = Join-Path $PSScriptRoot "Get-VstsRestAuthString.ps1"
$InvokeWithRetries = Join-Path $PSScriptRoot "Invoke-CommandWithRetries.ps1"

$PatDocUrl = "https://microsoft.sharepoint.com/:o:/r/teams/segcentral/_layouts/15/WopiFrame.aspx?sourcedoc=%7B817af5db-69b2-413b-a333-4acfb9a5c0bf%7D"

function main
{
    $headers = @{
        "Accept" = getAcceptString
        "Authorization" = . $GetAuthString -PatPath $PatPath -Verbose:$Verbose
    }

    if ($Body -ne $null -and $Body.GetType().Name -ne "String")
    {
        $Body = $Body | ConvertTo-Json -Depth 100 -Compress
        Write-Verbose "Request body:`n$Body"
    }

    
    $resp = . $InvokeWithRetries -MaxTries $MaxTries -IsErrorRetriable ${function:isErrorRetriable} -Verbose:$Verbose -Command {
        Invoke-WebRequest -Uri $Uri -Method $Method -Body $Body -ContentType "application/json" -Headers $headers -UseBasicParsing -Verbose:$Verbose
    }

    if ($resp.StatusCode -eq 203)
    {
        Write-Error @"
Got a redirect to a sign-in page; this usually happens when authentication fails.
Follow the instructions here to cache a valid PAT token: $PatDocUrl
To see what you're using as a PAT token, run:
[PSCredential]::new("foo", (Get-Content `$env:VSTS_PAT_PATH | ConvertTo-SecureString)).GetNetworkCredential().Password
"@
    }

    $resp | Add-Member -MemberType NoteProperty -Name "Content" -Value ($resp.Content | ConvertFrom-Json) -Force

    if ($FullResponse)
    {
        return $resp
    }
    else
    {
        return $resp.Content
    }
}

function getAcceptString
{
    $acceptString = "application/json"
    if ($ApiVersion)
    {
        $acceptString += ";api-version=$ApiVersion"
    }

    return $acceptString
}

function isErrorRetriable([Management.Automation.ErrorRecord]$err)
{
    $statusCode = $err.Exception.Response.StatusCode
    return $statusCode -and ([int]$statusCode -ge 500)
}

main
