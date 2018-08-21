# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

function CheckTextForReleaseNotesTags
{
    Param(
        [Parameter()]
        [String[]]$FormattedText
    ) 

    $definedTags = GetDefinedTags
    
    $allTags = @()
    foreach ($line in $FormattedText)
    {
        $tags = @()
        while ($line -match "^\s*\[(\w+)\](.+)*$") 
        {
            $tag,$line = $matches[1,2]
            $tags += $tag
        }

        if (!$tags)
        {
            Write-Verbose @"
No tags in line: 
    $line
"@
            continue
        }

        if ($tags -contains $definedTags.noReleaseTag)
        {
            Write-Verbose "The `"No Release Notes`" tag was detected - this PR won't generate release notes"
            # if there's only one tag in the entire line, the length will return the string length instead of 1
            if ($tags.count -gt 1)
            {
                Write-Host $tags.count
                Write-Host "Additional tags detected after [NORELEASENOTES] tag that will not be used: $($tags[1..($tags.length-1)])"
                return $false 
            }
            return $true
        }

        $containsSectionTag = $false

        # make sure the tags contain only one sectionTags
        foreach ($tag in $tags)
        {
            if ($definedTags.sectionTags -contains $tag)
            {
                if ($containsSectionTag)
                {
                    Write-Host "Each Release Note line must contain only 1 section tag: $($definedTags.sectionTags). The following line has more than one valid section tag: `n$line"
                    return $false
                }
                $containsSectionTag = $true
            }
            else
            {
                Write-Host "Invalid tag provided: $tag. Each release note line must contain exactly one section tag: $($definedTags.sectionTags). To instead have no release notes for this PR, use: $NoReleaseTag"
                return $false
            }
        }

        if (!$containsSectionTag)
        {
            Write-Host "Each Release Note line must contain 1 section tag: $($definedTags.sectionTags). The following line is missing a valid section tag: `n$line"
            return $false
        }

        Write-Verbose @"
Found Tags: $tags in line: 
    $line
"@
        if ($line -notmatch "[\d\w]+")
        {
            Write-Host @"
Each Release Notes line must contain release note text after the tags, on the same line. For example:
[Minor] This text is the release note line that will get added to the release notes file :)
"@
            return $false
        }

        $allTags += $tags
    }
    
    if (!$allTags) 
    {
        Write-Host "No tags found - please tag your PR title or description appropriately for release notes."
        return $false
    }
    return $true
}

function GetDefinedTags
{
    Param(
        [Parameter()]
        [String]$DefinedTagsFile
    )

    if (! $DefinedTagsFile) {
         $DefinedTagsFile = (Join-Path $PSScriptRoot "ReleaseNotesTags.json")
    }

    Write-Verbose "Reading defined tags from $DefinedTagsFile"
    if (!(Test-Path $DefinedTagsFile))
    {
        Write-Error "Cannot read in $DefinedTagsFile file containing standard tag names"
    }
    $tagFile = Get-Content -Path $DefinedTagsFile | ConvertFrom-Json
    Write-Verbose ($tagFile | Format-Table -Wrap | Out-String)
    return $tagFile
}

function GetCommitInfoObject
{
    Param(
        [Parameter(Mandatory=$true)]
        [String]$sha1
    )

    $GitCommand = . (Join-Path $PSScriptRoot "GetGitCommand.ps1")
    if (!$GitCommand) 
    {
        Write-Error "Unable to find git"
    }

    $commitInfo = & $GitCommand show -s --format='%B' $sha1
    
    $descriptionLines = @()

    foreach ($line in $commitInfo)
    {
        if ($line -match "Merged PR ([\d]+):(.+|$)")
        {
            $prNum = $matches[1]
            if ($matches[2])
            {
                $descriptionLines += $matches[2].Trimstart(' ')
            }
            Write-Verbose "PR number found: $prNum with title: $descriptionLines"
        }
        # Related work items
        elseif ($line -match "^Related work items: #(.+)")
        {
            $WorkItems = $Matches[1] -split ', #'
            Write-Verbose "Work Items found: $WorkItems"
        }
        # Release note line
        else
        {
            $descriptionLines += $line
            Write-Verbose "Adding description line: $line"
        }
    }

    return [PSObject]@{
        PRNum = $prNum;
        WorkItems = $workItems;
        Description = $descriptionLines;
    }
}

Export-ModuleMember -Function 'GetDefinedTags'
Export-ModuleMember -Function 'CheckTextForReleaseNotesTags'
Export-ModuleMember -Function 'GetCommitInfoObject'
