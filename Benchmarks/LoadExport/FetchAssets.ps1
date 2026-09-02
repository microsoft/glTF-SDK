param(
    [string]$Destination,
    [switch]$ForceDownload,
    [switch]$VerifyOnly
)

$ErrorActionPreference = "Stop"

$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
$manifestPath = Join-Path $PSScriptRoot "assets.json"
$manifest = Get-Content $manifestPath -Raw | ConvertFrom-Json

if ($ForceDownload -and $VerifyOnly) {
    throw "-ForceDownload and -VerifyOnly cannot be combined"
}

if (-not $Destination) {
    $Destination = Join-Path $repoRoot ("Built\Int\LoadExportAssets\" + $manifest.commit)
}
$Destination = [System.IO.Path]::GetFullPath($Destination)
New-Item -ItemType Directory -Force $Destination | Out-Null

[Net.ServicePointManager]::SecurityProtocol =
    [Net.ServicePointManager]::SecurityProtocol -bor [Net.SecurityProtocolType]::Tls12

function Test-AssetFile {
    param(
        [string]$Path,
        [long]$ExpectedBytes,
        [string]$ExpectedHash
    )

    if (-not (Test-Path $Path -PathType Leaf)) {
        return $false
    }

    $item = Get-Item $Path
    if ($item.Length -ne $ExpectedBytes) {
        return $false
    }

    $actualHash = (Get-FileHash $Path -Algorithm SHA256).Hash
    return $actualHash -eq $ExpectedHash
}

foreach ($file in $manifest.files) {
    $relativePath = [string]$file.path
    $targetPath = Join-Path $Destination ($relativePath -replace "/", "\")
    $expectedBytes = [long]$file.bytes
    $expectedHash = ([string]$file.sha256).ToUpperInvariant()

    if (-not $ForceDownload -and
        (Test-AssetFile -Path $targetPath -ExpectedBytes $expectedBytes -ExpectedHash $expectedHash)) {
        Write-Host "Verified $relativePath"
        continue
    }
    if ($VerifyOnly) {
        throw "Missing or invalid pinned corpus file: $relativePath"
    }

    $parent = Split-Path $targetPath -Parent
    New-Item -ItemType Directory -Force $parent | Out-Null
    $downloadPath = "$targetPath.download"
    Remove-Item $downloadPath -Force -ErrorAction SilentlyContinue

    $downloaded = $false
    for ($attempt = 1; $attempt -le 3 -and -not $downloaded; ++$attempt) {
        try {
            Write-Host "Downloading $relativePath (attempt $attempt)"
            Invoke-WebRequest -UseBasicParsing -Uri ([string]$file.url) -OutFile $downloadPath
            if (-not (Test-AssetFile -Path $downloadPath -ExpectedBytes $expectedBytes -ExpectedHash $expectedHash)) {
                throw "Downloaded bytes or SHA-256 do not match the pinned manifest"
            }
            $downloaded = $true
        }
        catch {
            Remove-Item $downloadPath -Force -ErrorAction SilentlyContinue
            if ($attempt -eq 3) {
                throw
            }
            Start-Sleep -Seconds $attempt
        }
    }

    Move-Item $downloadPath $targetPath -Force
}

& (Join-Path $PSScriptRoot "ValidateAssets.ps1") `
    -AssetRoot $Destination `
    -ManifestPath $manifestPath

Write-Host "Pinned glTF-Sample-Assets commit: $($manifest.commit)"
Write-Host "Verified asset root: $Destination"
Write-Output $Destination
