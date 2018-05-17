[CmdletBinding()]
Param(
    [Parameter(Mandatory)]
    [String]$NewVersion,
    [Parameter()]
    [String[]]$RootPackages
)

$ErrorActionPreference = "stop"
$Verbose = [bool]$PSBoundParameters["Verbose"]
$InvokeVsts = Join-Path $PSScriptRoot "Invoke-VstsRestMethod.ps1"

function main
{
    if (! $RootPackages)
    {
        $RootPackages = Get-Content (Join-Path $PSScriptRoot "CanvasCoreNugetPackages.config")
    }

    $packageRegistry = getPackageRegistry
    $closure = @{}
    $rootPackages | % { $closure[$_] = (getActualPackageVersion $_ $NewVersion $packageRegistry) }
    foreach ($packageName in $rootPackages)
    {
        addPackageAndDirectDependencies $packageName $closure[$packageName] $closure $packageRegistry
    }

    return $closure
}

function addPackageAndDirectDependencies($packageName, $packageVersion, $closure, $packageRegistry)
{
    $packageDependencies = getPackageDependencies $packageName $packageVersion $packageRegistry

    foreach ($dependency in $packageDependencies)
    {
        addDependencyToClosure $dependency $closure $packageRegistry
    }
}

function addDependencyToClosure($dependency, $closure, $packageRegistry)
{
    $packageName = $dependency.packageName
    $packageVersion = getMinimumDependencyVersion $dependency $packageRegistry
    if (! $packageVersion)
    {
        Write-Verbose "Didn't find a valid version for $packageName"
        return
    }

    Write-Verbose "Using version $packageVersion for $packageName"

    $existing = $closure[$packageName]
    if ($existing -and ($existing -ne $packageVersion))
    {
        # For now, we're just assuming that multiple packages can't depend on different versions of the same dependency.
        # This works right now, but we may want to enhance this in the future by e.g. taking the maximum if
        # there are conflicting versions.
        Write-Error "Package $packageName has two conflicting versions: $packageVersion =/= $existing"
    }

    $closure[$packageName] = $packageVersion
}

function getMinimumDependencyVersion($dependency, $packageRegistry)
{
    $packageInfo = $packageRegistry | ? { $_.name -eq $dependency.packageName }

    if (! $packageInfo)
    {
        Write-Warning "Not updating version for dependency $($dependency.packageName) because it isn't in the BigPark feed"
        return $null
    }

    # Right now Canvas dependency versions are only specified with inclusive mins
    if (! ($dependency.versionRange -match "\[(.+), \)"))
    {
        Write-Error "Don't know how to parse dependency $($dependency.packageName) range: $($dependency.versionRange)"
    }

    $partialMin = $matches[1]
    Write-Verbose "Looking for the earliest version of $($dependency.packageName) that is at least $partialMin"

    $versionsInRange = $packageInfo.versions.version | ? { (compareVersions $_ $partialMin) -ge 0 }
    if ($versionsInRange.count -eq 0)
    {
        Write-Error "No available versions of $($dependency.packageName) in range $($dependency.versionRange)"
    }

    Write-Verbose "Available versions: $($versionsInRange -join ", ")"

    return getMinimumVersion $versionsInRange
}

function getMinimumVersion($versions)
{
    if (! $versions)
    {
        return $versions
    }

    $min = $null
    foreach ($version in $versions)
    {
        if (($min -eq $null) -or ((compareVersions $version $min) -lt 0))
        {
            $min = $version.ToString()
        }
    }

    return $min
}

# Return -1 if a < b, 1 if a > b, or 0 if a == b
# Doesn't do complicated nuget v3 prerelease comparisons
function compareVersions($a, $b)
{
    $aRelease,$aPrerelease = $a.split("-", 2)
    $bRelease,$bPrerelease = $b.split("-", 2)

    $aParts = [int[]]$aRelease.split(".")
    $bParts = [int[]]$bRelease.split(".")

    # Pad out the version numbers with zeroes. 
    # e.g. 1.1 == 1.1.0
    while ($aParts.count -lt $bParts.count)
    {
        $aParts += @(0)
    }
    while ($bParts.count -lt $aParts.count)
    {
        $bParts += @(0)
    }

    foreach ($idx in (0..($aParts.count-1)))
    {
        $cmp = compareScalars $aParts[$idx] $bParts[$idx]
        if ($cmp -ne 0)
        {
            return $cmp
        }
    }

    # If we're here the release part of the number is equal, so compare the prerelease part
    # Note we're doing an alphabetic comparison, so e.g. 1.0.0-b2 > 1.0.0-b11
    # This shouldn't matter because we don't typically take dependencies on prerelease versions
    $cmp = compareScalars $aPrerelease $bPrerelease
    return $cmp
}

function compareScalars($a, $b)
{
    if ($a -lt $b)
    {
        return -1
    }

    if ($a -gt $b)
    {
        return 1
    }

    if ($a -eq $b)
    {
        return 0
    }

    Write-Error "Objects $a and $b aren't comparable"
}

function getPackageRegistry
{
    return (. $InvokeVsts "https://microsoft.feeds.visualstudio.com/DefaultCollection/_apis/packaging/feeds/BigPark/packages?includeAllVersions=true" -Verbose:$Verbose).value
}

function getPackageDependencies($packageName, $packageVersion, $packageRegistry)
{
    $packageInfo = $packageRegistry | ? { $_.name -eq $packageName }
    if (! $packageInfo)
    {
        Write-Error "No package in the BigPark feed with name $packageName"
    }

    $shallowVersionInfo = $packageInfo.versions | ? { $_.version -eq $packageVersion }
    if (! $shallowVersionInfo)
    {
        Write-Error "Version $packageVersion of $packageName is not in BigPark feed"
    }

    $versionUrl = "$($packageInfo.url)/Versions/$($shallowVersionInfo.id)"
    return (. $InvokeVsts $versionUrl -Verbose:$Verbose).dependencies
}

function getActualPackageVersion($packageName, $packageVersion, $packageRegistry)
{
    $availableVersions = ($packageRegistry | ? { $_.name -eq $packageName }).versions.version

    if ($availableVersions -contains $packageVersion)
    {
        return $packageVersion
    }

    $normalizedVersion = getNormalizedVersion $packageVersion
    if ($availableVersions -contains $normalizedVersion)
    {
        return $normalizedVersion
    }

    Write-Error "Version $packageVersion or $normalizedVersion of $packageName is not in BigPark feed"
}

function getNormalizedVersion($version)
{
    $release,$prerelease = $version.split("-", 2)
    $parts = $release.split(".")
    while ($parts.length -gt 3 -and $parts[-1] -eq 0)
    {
        $parts = $parts[0..($parts.length-2)]
    }
    $release = $parts -join "."
    return "$release-$prerelease"
}

main