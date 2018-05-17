[CmdletBinding()]
Param(
    [Parameter(Mandatory)]
    $AppxFilePath
)

$ErrorActionPreference = "stop"
[Reflection.Assembly]::LoadWithPartialName('System.IO.Compression.FileSystem') | Out-Null

$APPX_MANIFEST = "AppxManifest.xml"
function main
{
    $file = [IO.Compression.ZipFile]::OpenRead($AppxFilePath).Entries | Where-Object { $_.FullName -eq $APPX_MANIFEST }
    if (! $file)
    {
        Write-Error "Appx $AppxFilePath does not contain a $APPX_MANIFEST"
    }
    
    $reader = New-Object IO.StreamReader($file.Open())
    return [xml]($reader.ReadToEnd())
}

main