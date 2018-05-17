# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

<#
.SYNOPSIS
Get the auth string needed to make a VSTS REST call.

.PARAMETER PatPath
Should only be used when called locally. The path to a file containing an encrypted PAT token. You can also provide a PatPath by setting the environment variable VSTS_PAT_PATH.

Can set up this file by doing the following:
* Create a PAT token here: https://microsoft.visualstudio.com/_details/security/tokens
* Run the following in PowerShell:
    $pat = (Get-Credential).password # (set password to your PAT; username doesn't matter)
    Set-Content "c:\creds\microsoftPat" ($pat | ConvertFrom-SecureString)
#>
[CmdletBinding()]
Param(
    [Parameter()]
    [String]$PatPath = $env:VSTS_PAT_PATH
)

$ErrorActionPreference = "stop"
$PatDocUrl = "https://microsoft.sharepoint.com/:o:/r/teams/segcentral/_layouts/15/WopiFrame.aspx?sourcedoc=%7B817af5db-69b2-413b-a333-4acfb9a5c0bf%7D"

function main
{
    if ($env:SYSTEM_ACCESSTOKEN)
    {
        return "Bearer $env:SYSTEM_ACCESSTOKEN"
    }

    if ($PatPath)
    {
        $encodedPat = getEncodedPat
        return "Basic $encodedPat"
    }

    Write-Error @"
No credentials available to invoke VSTS methods.
If this script was called from a VSTS build definition, ensure 'Allow Scripts to Access OAuth Token' is checked.
To get this script working locally, follow the instructions here: $PatDocUrl
"@
}

# Decrypt a secure string, and then convert to base64 (expected by auth headers)
# Taken from https://github.com/DarqueWarrior/team/blob/master/src/team.psm1
function getEncodedPat
{
    [SecureString]$encryptedPat = Get-Content $PatPath | ConvertTo-SecureString
    $credential = New-Object System.Management.Automation.PSCredential "foo",$encryptedPat
    $decryptedPat = $credential.GetNetworkCredential().Password
    return [System.Convert]::ToBase64String([System.Text.Encoding]::UTF8.GetBytes(":$decryptedPat"))
}

main
