<#
.SYNOPSIS
Installs all passed-in appx files.

.PARAMETER AppxFiles
The list of appx files to install.

.PARAMETER DeviceIp
If passed in, we'll use WinAppDeploy to install the files onto the device at this IP.
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

$WinAppDeploy = "$PSScriptRoot/WinAppDeploy.ps1"

function main
{
    if ($DeviceIp)
    {
        foreach ($appxFile in $appxFiles)
        {
            installAppxFile $appxFile
            
        }
    }
    else
    {
        $AppxFiles | Foreach-Object { Add-AppxPackage -Path $_ -Verbose:$Verbose -ErrorAction Ignore }
    }
}

function installAppxFile($appxFile)
{
    while ($true)
    {
        try
        {
            . $WinAppDeploy -DeviceIp $DeviceIp -Command install -AdditionalArgs @("-file", $appxFile) | Write-Verbose
            return
        }
        catch
        {
            $errorMessage = $_.exception.message
            if ($errorMessage -match "A higher version .+ of this package is already installed")
            {
                Write-Host "Not installing $appxFile because a higher version is already installed."
                return
            }
            
            if ($errorMessage -notmatch "The process cannot access the file because it is being used by another process")
            {
                throw
            }

            Write-Host "Cannot install $appxFile because the phone is trying to use an old version. Sleeping and trying again."
            Start-Sleep -Seconds 10
        }

    }
}

main