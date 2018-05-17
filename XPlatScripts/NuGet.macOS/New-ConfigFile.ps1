# This is simple enough to be an inline script, but we can't currently pass in arguments
# to inline scripts using the cross-platform PowerShell task.
[CmdletBinding()]
Param(
    [Parameter(Mandatory)]
    [String]$Path,
    [Parameter(Mandatory)]
    [String]$Contents
)

$ErrorActionPreference = "Stop"

function main
{
    $directory = Split-Path $Path
    New-Item $directory -ErrorAction Ignore -Type Directory
    $Contents | Out-File -Encoding ascii $Path
}

main