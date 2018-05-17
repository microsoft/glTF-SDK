<#
.SYNOPSIS
Updates all projects to use the passed-in nuget package closure.
#>
[CmdletBinding()]
Param(
    [Parameter(Mandatory)]
    [Hashtable]$NuGetPackageClosure,
    [Parameter()]
    [String]$SourceDirectory,
    [Parameter()]
    [String]$ExcludeFolders = "Build\packages"
)

if (! $SourceDirectory) {
    $SourceDirectory = (Get-Item $PSScriptRoot).Parent.Parent.FullName
}

$ErrorActionPreference = "stop"

function main
{
    updatePackagesConfigVersions
    updateVcxprojVersions
    updateProjectJsonVersions
    updateCsprojVersions
}

function updatePackagesConfigVersions
{
    Write-Verbose "Updating the version for packages.config files"

    foreach ($file in (getFiles "packages.config"))
    {
        Write-Verbose $file

        [xml]$xml = Get-Content $file

        foreach ($package in $xml.packages.package)
        {
            $newVersion = $NuGetPackageClosure[$package.id]
            if ($newVersion)
            {
                if ($newVersion -eq $package.version)
                {
                    Write-Verbose "$($package.id) is already at desired package version $newVersion"
                }
                else
                {
                    Write-Verbose "Updating $($package.id) from $($package.version) to $newVersion"
                    $package.version = $newVersion
                }
            }
            else
            {
                Write-Verbose "No version provided for $($package.id), so just using the checked-in version."
            }
        }
        $xml.Save($file)
    }
}

function updateVcxprojVersions
{
    Write-Verbose "Updating the version for vcxproj files"
    
    foreach ($file in (getFiles "*.vcxproj"))
    {
        Write-Verbose $file

        [xml]$xml = Get-Content $file
        $attributes = $xml.SelectNodes("//*[local-name()='Import' or local-name()='Error']/attribute::*[local-name()='Text' or local-name()='Condition' or local-name()='Project']")
        
        foreach ($attribute in $attributes)
        { 
            foreach ($package in $NuGetPackageClosure.Keys)
            {
                $attribute.Value = updateProjectVersionNumber $attribute.Value $package $NuGetPackageClosure[$package]
            }
        } 

        $xml.Save($file)
    }
}

function updateCsprojVersions
{
    Write-Verbose "Updating the version for csproj files"
    
    foreach ($file in (getFiles "*.csproj"))
    {
        Write-Verbose $file

        [xml]$xml = Get-Content $file

        $nodes = $xml.SelectNodes("//*[local-name()='HintPath' or local-name()='Reference/HintPath']")
        $attributes = $xml.SelectNodes("//*[local-name()='Import' or local-name()='Error' or local-name()='Reference/HintPath']/attribute::*[local-name()='Text' or local-name()='Condition' or local-name()='Project']")
        
        foreach ($attribute in $attributes)
        { 
            foreach ($package in $NuGetPackageClosure.Keys)
            {
                $attribute.Value = updateProjectVersionNumber $attribute.Value $package $NuGetPackageClosure[$package]
            }
        }
        
        foreach ($node in $nodes)
        { 
            foreach ($package in $NuGetPackageClosure.Keys)
            {
                $node.'#text' = updateProjectVersionNumber $node.'#text' $package $NuGetPackageClosure[$package]
            }
        }

        $xml.Save($file)
    }
}

function updateProjectJsonVersions
{
    Write-Verbose "Updating the version for project.json files"
    
    foreach ($file in (getFiles "project.json"))
    {
        Write-Verbose $file

        $json = Get-Content $file | ConvertFrom-Json
        
        foreach ($package in $NuGetPackageClosure.Keys)
        {
            if ($json.dependencies.$package)
            {
                $json.dependencies.$package = $NuGetPackageClosure[$package]
            }
        }

        $json | ConvertTo-Json -Depth 100 | Out-File $file -Force -Encoding ascii
    }
}

# .csproj, vcxproj, etc. files contain text lines pointing to nuget packages
# use regex replace to have these point to the updated nuget packages
function updateProjectVersionNumber($string, $packageName, $newVersion)
{
    $escapedPackage = [Regex]::Escape($packageName)
    $newString = $string -replace "\\$escapedPackage\.(\d+\.[0-9a-z\.\-]+)\\", "\$packageName.$newVersion\"
    if ($newString -ne $string)
    {
        Write-Verbose "Replacing $packageName version:`n$string`n$newString"
    }
    return $newString
}

function getFiles($searchPattern)
{
    $files = Get-ChildItem $SourceDirectory -Recurse -Include $searchPattern
    $fileNames = $files | Foreach-Object { $_.FullName }
    return $fileNames | Where-Object { ! (shouldIgnoreFile $_) }
}

function shouldIgnoreFile($fileName)
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

main