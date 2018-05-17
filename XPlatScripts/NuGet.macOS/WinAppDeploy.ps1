<#
.SYNOPSIS
Runs WinAppDeployCmd to deploy to a specific device.
#>
[CmdletBinding()]
Param(
    [Parameter(Mandatory)]
    [String]$DeviceIp,
    [Parameter(Mandatory)]
    [String]$Command,
    [Parameter()]
    [String[]]$AdditionalArgs = @()
)

$ErrorActionPreference = "Stop"
$WinAppDeployCmd = "C:\Program Files (x86)\Windows Kits\10\bin\x86\WinAppDeployCmd.exe"

function main
{
    $allArgs = @($Command, "-ip", $DeviceIp) + $AdditionalArgs
    Write-Verbose "$WinAppDeployCmd $allArgs"
    &$WinAppDeployCmd $allArgs | Tee-Object -Variable deployOutput
    if (! $?)
    {
        Write-Error "Error running [$WinAppDeployCmd $allArgs]: $($deployOutput -join "`n")"
    }
}

main