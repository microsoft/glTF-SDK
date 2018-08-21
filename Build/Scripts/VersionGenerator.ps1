# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

<#
.SYNOPSIS
Determines the build/package version based on the most recent tags in the git branch hierarchy, or a provided override version number.

.DESCRIPTION
Determines the next version number either for release or pre-release, with the ability to override all version numbers.
It will use git describe and the branch name in combination with the version overrides to determine the new version.

MASTER branch:
    - Branch name "master". By default, will +1 to the last found tag's patch number, unless overrides are provided
PATCH branch:
    - Branch name "Release/a.b", overrided version must match a.b
    - By default, will +1 to the last found tag's patch number, unless overrides are provided
HOTFIX branch:
    - Branch name "Release/a.b.c", overrided version numbers must match a.b.c
    - By default, will +1 to the last found tag's hotfix number, unless overrides are provided

.PARAMETER SourceDirectory 
Optional. The root source tree to scan, which must be a valid git repo with current branch master, or a patch/hotfix branch

.PARAMETER VersionOverride 
Optional. The version number to override with, in format "vMajor.vMinor.vPatch.vHotfix". Must be either all overrided 
    with numbers, or must be left null let the script decide the next version.

.PARAMETER GenerateReleaseVersion 
Optional. Defaults to pre-release (false). If true, generates release version number instead

.EXAMPLE
VersionNumberGenerator.ps1 -SourceDirectory "D:\Build vNext\Builds\cb11432a\Infrastructure\Code" -VersionOverride "1.2.3.4" -GenerateReleaseVersion True

.RESULT EXAMPLE
1.2.3.4 for release 
<Major>.<Minor>.<Patch>.<Hotfix>

1.2.3.4-b05-g94f7eb5 for prerelease
<Major>.<Minor>.<Patch>.<Hotfix>-b<2-digit-#-commits>-<SHA1-gitcommit-hash>

CoApp follows semver 1.0.0 so the part after <Hotfix> cannot include periods.
#>

[CmdletBinding()]

param(
    [string]$SourceDirectory,
    [string]$VersionOverride,
    [string]$GenerateReleaseVersion = $false
)

$ErrorActionPreference = "Stop"

# TO DO: comparing if versionoverride is a string of $null is a workaround
# to deal with Command Line build step that doesn't allow passing in empty parameters
if ($VersionOverride -eq '$null')
{
    $VersionOverride = $null
}

# Initialize global params 
if (!$SourceDirectory) 
{
    $SourceDirectory = (Get-Item $PSScriptRoot).Parent.Parent.FullName
}

$GitCommand = . (Join-Path $PSScriptRoot "GetGitCommand.ps1")
if (!$GitCommand) 
{
    Write-Error "Unable to find git"
}

[Boolean]$GenerateReleaseVersion = [System.Convert]::ToBoolean($GenerateReleaseVersion)

Enum BranchType
{
    Master
    Patch
    Hotfix
}


function Main 
{
    Write-Verbose "Begin script VersionNumberGenerator.ps1"
    Write-Verbose "Source Directory = $SourceDirectory"
    Write-Verbose "GenerateReleaseVersion = $GenerateReleaseVersion"
    Write-Verbose "VersionOverride = $VersionOverride"

    $overridedVerNumbers = ParseVersionOverride $VersionOverride
    Push-Location $SourceDirectory
    $gitTraceOriginal = $env:GIT_TRACE
    
    try
    {
        $env:GIT_TRACE = 1

        $tagInfo = GetLastTagInfo

        if ($overridedVerNumbers)
        {
            $versionNums = $overridedVerNumbers
        }
        else
        {
            $branchObject = GetBranchInfo

            # deep copy
            $versionNums = $tagInfo.VerNums | foreach { $_ }
            
            if ($branchObject.Type -eq [BranchType]::Master)
            {
                $versionNums[2] = [int]($versionNums[2]) + 1
                $versionNums[3] = 0
            }
            else
            {
                $verNumLength = $branchObject.VerNums.length

                # if git describe doesn't match branch name, use the branch name and set remaining numbers to 0
                if ($branchObject.VerNums[0..$verNumLength] | where { $versionNums[0..$verNumLength] -notcontains $_})
                {
                    Write-Verbose "Last tag doesn't match branch name - using branch name version instead"
                    [array]::copy($branchObject.VerNums, $versionNums, $verNumLength)
                
                    if ($branchObject.Type -eq [BranchType]::Hotfix -or $branchObject.Type -eq [BranchType]::Patch)
                    {
                        $versionNums[3] = 0
                    }
                    if ($branchObject.Type -eq [BranchType]::Patch)
                    {
                        $versionNums[2] = 0
                    }
                }
                else
                {
                    if ($branchObject.Type -eq [BranchType]::Hotfix)
                    {
                        $versionNums[3] = [int]($versionNums[3]) + 1
                    }
                    elseif ($branchObject.Type -eq [BranchType]::Patch)
                    {
                        $versionNums[2] = [int]($versionNums[2]) + 1
                        $versionNums[3] = 0
                    }
                }
            }
        }
    
        $versionString = FormatVersionNumsIntoString $versionNums $GenerateReleaseVersion $tagInfo.CommitInfo
        
        Write-Host "##vso[task.setvariable variable=GLTFSDKVersionNumber;]$versionString"

        Write-Verbose "New version number is: $versionString"
        Write-Verbose "Set GLTFSDKVersionNumber to $versionString"
        Write-Verbose "Exiting script VersionNumberGenerator.ps1"
    }

    finally
    {
        Pop-Location
        $env:GIT_TRACE = $gitTraceOriginal
    }

    return $versionString
}

# Checks that the provided override version is valid and returns a string array containing the separate version parts.
function ParseVersionOverride($VersionOverride)
{
    if ($VersionOverride -match "^(\d+)\.(\d+)\.(\d+)\.(\d+)$")
    {
        return $Matches[1..4]
    }
    elseif (!$VersionOverride)
    {
        return $null
    }
    Write-Error "Version Override is not valid: must be empty or a 4-part version number, ex. 1.4.6.0"
}

function GetBranchInfo()
{
    # If this is a PR build, derive versions using the branch we're merging to
    if ($Env:SYSTEM_PULLREQUEST_TARGETBRANCH) {
        $gitBranchName = $Env:SYSTEM_PULLREQUEST_TARGETBRANCH
    # If this is a central build that isn't a PR, use the branch we're building on
    } elseif ($Env:BUILD_SOURCEBRANCH) {
        $gitBranchName = $Env:BUILD_SOURCEBRANCH
    # Otherwise, just make git calls to see if there's a branch on the commit we're building
    } else {
        $gitBranchName = & $GitCommand name-rev --name-only HEAD
    }

    if ($gitBranchName -match "^refs/heads/(.+)$") {
        $gitBranchName = $matches[1]
    }

    Write-Verbose "Branch name: $gitBranchName"
    $info = [PSObject]@{ Type=""; VerNums=""}

    if ($gitBranchName -match "^Release/(\d+)\.(\d+).(\d+)$")
    {
        $info.Type = [BranchType]::Hotfix
        $info.VerNums = $Matches[1..3]
    }
    elseif ($gitBranchName -match "^Release/(\d+)\.(\d+)$")
    {
    
        $info.Type = [BranchType]::Patch
        $info.VerNums = $Matches[1..2]
    }
    else
    {
        $info.Type = [BranchType]::Master
    }
    
    Write-Verbose "$($info.Type) branch detected"
    return $info
}

# Formats the given array of version numbers into a final version string
# Pre-release versions are suffixed with additional commit information
function FormatVersionNumsIntoString($versionNums, $generateRelease, $commitInfo)
{
    $result = $versionNums -join '.'

    if (!$generateRelease) 
    {
        $result += "-b"
        $result += $commitInfo
    }

    return $result
}

# Calls git describe to find the last tag, given overrided version numbers. If there is no tag found, it will error or continue depending on $noMatchShouldError
# Note: the [0-9]* in the git match will look for a single digit and then wildcard, so "1a" would match even though it is an invalid version number. 
#       So, immediately afterwards, we make sure it uses only digits using more powerful regex matching
function GetLastTagInfo
{
    $lastTag = & $GitCommand describe --tags --long --dirty --always --first-parent --match "[rv][0-9]*.[0-9]*.[0-9]"

    Write-Verbose "Git describe result: $lastTag"
    
    if ($lastTag -match "[rv](\d+)\.(\d+)\.(\d+)\.(\d+)-(\d+)(-g\S{7})\S*$")
    {
        $info = [PSObject]@{ VerNums=""; CommitInfo=""}
        $info.VerNums = $Matches[1..4]
        $info.CommitInfo = ("{0:D2}" -f [int]$Matches[5]) + $Matches[6]
        return $info
    }
    Write-Error "No tag found with format: $formatString. Erroring out."
}

Main
