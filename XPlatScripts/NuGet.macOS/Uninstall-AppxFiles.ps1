<#
.SYNOPSIS
Uninstalls all passed-in appx files, removing packages that take these on as dependencies if necessary.

.PARAMETER AppxFiles
The list of appx files to uninstall.

.PARAMETER DeviceIp
If passed in, we'll use WinAppDeploy to remove the files on the device at this IP.
#>
[CmdletBinding()]
Param(
    [Parameter(Mandatory)]
    [String[]]$AppxFiles,
    [Parameter()]
    [String]$DeviceIp
)

$ErrorActionPreference = "Stop"
$Verbose = [bool]$PSBoundParameters["Verbose"]

$GetAppxDetailsFromFile = "$PSScriptRoot/Get-AppxDetailsFromFile.ps1"
$WinAppDeploy = "$PSScriptRoot/WinAppDeploy.ps1"

function main
{
    $appxPackageNames = $AppxFiles | Foreach-Object { (. $GetAppxDetailsFromFile $_).Package.Identity.Name }
    if ($DeviceIp)
    {
        removeDevicePackagesWithNames $appxPackageNames
    }
    else
    {
        getAllAppxPackages | Where-Object { $appxPackageNames -contains $_.Name } | Remove-AppxPackage -Verbose:$Verbose -ErrorAction Ignore | Write-Verbose
    }
}


function removeDevicePackagesWithNames($names)
{
    $installedPackages = . $WinAppDeploy -DeviceIp $DeviceIp -Command list
    foreach ($name in $names)
    {
        $installedPackages = removeDevicePackageWithName $name $installedPackages
    }
}

function removeDevicePackageWithName($name, $installedPackages)
{
    if ($name -eq "Microsoft.VCLibs.140.00")
    {
        Write-Verbose "The system depends on $name so we can't uninstall it. If a test relies on a different version of this package we're out of luck."
        return $installedPackages
    }

    $package = $installedPackages | Where-Object { $_.StartsWith("${name}_") }
    if (! $package)
    {
        Write-Verbose "Package $name is not installed."
        return $installedPackages
    }
    if ($package.Count -gt 1)
    {
        Write-Error "Multiple [$name] packages installed: [$package]"
    }

    # We'll be removing this so remove it from the list of packages we have installed
    $installedPackages = $installedPackages | Where-Object { $_ -ne $package }

    try
    {
        tryRemoveDevicePackage $package
    }
    catch
    {
        # If other packages have this package as a dependency the uninstall will fail.
        # In that case remove those packages. Windows will then automatically uninstall this package.
        $namesWithDependencies = extractPackageNamesWithDependenciesFromErrorMessage $_.exception.message
        if (! $namesWithDependencies)
        {
            throw
        }

        foreach ($nameWithDependency in $namesWithDependencies)
        {
            $installedPackages = removeDevicePackageWithName $nameWithDependency $installedPackages
        }
    }

    return $installedPackages
}

function extractPackageNamesWithDependenciesFromErrorMessage($errorMessage)
{
    if ($errorMessage -match "package\(s\)\s+(.+?)\s+currently depends on the framework")
    {
        return $matches[1] -split "\s+"
    }

    return @()
}

function tryRemoveDevicePackage($package)
{
    . $WinAppDeploy -DeviceIp $DeviceIp -Command uninstall -AdditionalArgs @("-package", $package) | Write-Verbose
}

# We can only get appx packages installed for other users if we're admin. Try
# that first, but if it fails we're probably still ok so fall back to just
# uninstalling packages we have access to.
function getAllAppxPackages
{
    try
    {
        return Get-AppxPackage -AllUsers
    }
    catch
    {
        return Get-AppxPackage
    }
}


main