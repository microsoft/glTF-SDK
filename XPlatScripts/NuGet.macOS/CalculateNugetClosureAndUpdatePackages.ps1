[CmdletBinding()]
Param(
    [Parameter(Mandatory)]
    [String]$NewVersion,
    [Parameter()]
    [String[]]$RootPackages,
    [Parameter()]
    [String]$SourceDirectory,
    [Parameter()]
    [String]$ExcludeFolders = "Build\packages"
)

$ErrorActionPreference = "stop"
$Verbose = [bool]$PSBoundParameters["Verbose"]
$CalculateClosure = Join-Path $PSScriptRoot "CalculateNugetPackageClosure.ps1"
$UpdatePackages = Join-Path $PSScriptRoot "UpdateNugetPackages.ps1"

function main
{
    $closure = . $CalculateClosure -NewVersion $NewVersion -RootPackages $RootPackages -Verbose:$Verbose
    . $UpdatePackages -NuGetPackageClosure $closure -SourceDirectory $SourceDirectory -ExcludeFolders $ExcludeFolders -Verbose:$Verbose
}

main