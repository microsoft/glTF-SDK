$ErrorActionPreference = "Stop"

$searchLocations = $GitLocations + @("git", "${env:ProgramFiles(x86)}\Git\bin\git.exe", "${env:ProgramFiles}\Git\bin\git.exe", "${env:ProgramW6432}\Git\bin\git.exe")
$git = ($searchLocations | ? { Get-Command $_ -ErrorAction SilentlyContinue } | Select -First 1)
if (! $git)
{
    Write-Error "Unable to find git"
}

return $git