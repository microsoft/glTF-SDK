# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

<# .SYNOPSIS
    Checks a Pull Request to make sure it is formatted to automatically generate release notes.

    .DESCRIPTION
    Given a Pull Request's ID number, checks the PR's title and description to ensure it contains correctly formatted release notes.
    This is only a format checker, it does not generate release notes from the Pull Request.

    .PARAMETER PullRequestID
    Optional. The ID number of the Pull Request. This is defaulted to env:SYSTEM_PULLREQUEST_PULLREQUESTID, which is set when invoked from a PR build.

    .PARAMETER RepoName
    Optional. The git repository name, this is defaulted to env:BUILD_REPOSITORY_NAME, which is set when invoked from a PR build.

    .PARAMETER DefinedTagsFilePath
    Optional. The .json file containing the defined valid tags we use to generate release notes. The path should be relative to this script.

    .PARAMETER LocalPATLocation
    Optional. The location of a local PAT token to make authorized VSTS REST calls to get info for the given PR and repo. 
    For use when running locally, as this script will use env:SYSTEM_ACCESSTOKEN by default, which is set when invoked from a PR build.
#>
[CmdletBinding()]
Param(
    [Parameter()]
    [String]$PullRequestID = $env:SYSTEM_PULLREQUEST_PULLREQUESTID,
    [Parameter()]
    [String]$RepoName = $env:BUILD_REPOSITORY_NAME,
    [Parameter()]
    [String]$LocalPATLocation
)

$ErrorActionPreference = "Stop"

function Main
{
    Import-Module (Join-Path $PSScriptRoot "ReleaseNotesHelper.psm1")

    PrintReleaseNoteFormatInfo

    if (!$PullRequestID)
    {
        Write-Error "Pull Request ID cannot be null."
    }
    
    $prInfo = GetPRInfo
    $validFormat = CheckTextForReleaseNotesTags $prInfo


    if (!$validFormat)
    {
        Write-Error "Please tag your PR title or description appropriately for release notes."
    }

    Write-Verbose "Finished checking - the PR is in a correct format to generate release notes"
}

function PrintReleaseNoteFormatInfo
{
    Write-Host @"
This script will check the format of the PR to ensure it will work with the automated release notes system.
Your PR must contain at least one formatted release note entry, either within the PR's title or the PR's description.
The format of this release note entry must begin with tags.
If your PR does not involve any changes that should generate release notes, use the [NORELEASENOTES] tag.
To generate release notes, put each release note line on a single line each, and prefix it with tags to describe the file and section.

Within each release note file, you must define what kind of change was made. For each release note line, specify one (and only one) section tag. The project tag values are:
    [FEATURE]
    [BREAKING]
    [MINOR]
    [BUG]
    
As an example, here is a PR description you may have: 
   
[FEATURE] Adding new feature that does something and everything. This is a very long release note line that must not contain any line breaks.
[BUG] Fixed hang in ViewerApp
And if I were to type text in my PR title or description, without any tags in front, they won't be included in the release notes.

Pull Request ID provided for this build: $PullRequestID
"@
}

# Use REST apis to get the current Pull Request information
function GetPRInfo()
{
    # edits to a PR title/description happens extremely quickly, GET calls get updated info.
    $url = "https://microsoft.visualstudio.com/DefaultCollection/Apps/_apis/git/repositories/$RepoName/pullRequests/$PullRequestID" + "?api-version=3.0"

    $result = Rest $url

    Write-Verbose $result

    $prInfo = @($result.title)
    $prInfo += $result.description -split '\n'

    Write-Verbose "PR text: $prInfo"
    
    if (!$prInfo)
    {
        Write-Error "PR title and description are empty for PR id: $PullRequestID"
    }

    return $prInfo
}

function Rest($url) 
{
    if ($env:SYSTEM_ACCESSTOKEN)
    {
        $authorization = "Bearer $env:SYSTEM_ACCESSTOKEN"
    }
    # Running locally
    else
    {
        Write-Host "No SYSTEM_ACCESSTOKEN environment variable found. Getting local PAT token at location $LocalPATLocation"
        $encodedPat = getEncodedPat 
        $authorization = "Basic $encodedPat"
    }

    return Invoke-RestMethod $url -Headers @{Authorization = $authorization}
}

# Decrypt a secure string, and then convert to base64 (expected by auth headers)
# Taken from https://github.com/DarqueWarrior/team/blob/master/src/team.psm1
function getEncodedPat() 
{
    if (!$LocalPATLocation)
    {
        Write-Error "No local pat provided, and no value in `$env:SYSTEM_ACCESSTOKEN."
    }

    [SecureString]$encryptedPat = Get-Content $LocalPATLocation | ConvertTo-SecureString

    $credential = New-Object System.Management.Automation.PSCredential "foo",$encryptedPat
    $decryptedPat = $credential.GetNetworkCredential().Password
    return [System.Convert]::ToBase64String([System.Text.Encoding]::UTF8.GetBytes(":$decryptedPat"))
}

Main
