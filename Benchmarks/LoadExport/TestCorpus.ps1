param(
    [Parameter(Mandatory = $true)]
    [string]$BuildDir,

    [string]$AssetRoot,

    [ValidateSet("Release")]
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"

$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
$manifest = Get-Content (Join-Path $PSScriptRoot "assets.json") -Raw |
    ConvertFrom-Json
$BuildDir = [System.IO.Path]::GetFullPath($BuildDir)
if (-not $AssetRoot) {
    $AssetRoot = Join-Path $repoRoot (
        "Built\Int\LoadExportAssets\" + $manifest.commit)
}
$AssetRoot = [System.IO.Path]::GetFullPath($AssetRoot)

& (Join-Path $PSScriptRoot "FetchAssets.ps1") `
    -Destination $AssetRoot `
    -VerifyOnly | Out-Null

$executables = @(
    Get-ChildItem -Path $BuildDir -Recurse -File |
        Where-Object {
            $_.Name -eq "GLTFSDK.LoadExportBenchmarks.exe" -or
            $_.Name -eq "GLTFSDK.LoadExportBenchmarks"
        }
)
if ($executables.Count -eq 0) {
    throw "GLTFSDK.LoadExportBenchmarks was not found under $BuildDir"
}
$executable = $executables |
    Sort-Object `
        @{ Expression = {
            if ($_.FullName -match [regex]::Escape("\$Configuration\")) {
                0
            }
            else {
                1
            }
        } },
        FullName |
    Select-Object -First 1

$branch = (& git -C $repoRoot branch --show-current | Out-String).Trim()
$sourceCommit = (& git -C $repoRoot rev-parse HEAD | Out-String).Trim()
if ($LASTEXITCODE -ne 0) {
    throw "Unable to resolve the source commit"
}
$implementation = if ($branch -eq "perf/Release-1.9.5-load-export") {
    "rapidjson-1.9.5"
}
else {
    "nlohmann-2.0.0"
}

$runRoot = Join-Path $BuildDir "LoadExportCorpusValidation"
$marker = Join-Path $runRoot ".gltf-sdk-load-export-corpus-test"
if (Test-Path $runRoot) {
    if (-not (Test-Path $marker -PathType Leaf)) {
        throw "Refusing to remove unowned validation directory $runRoot"
    }
    Remove-Item $runRoot -Recurse -Force
}
New-Item -ItemType Directory -Force $runRoot | Out-Null
[System.IO.File]::WriteAllText($marker, "owned")

try {
    foreach ($selection in $manifest.selection) {
        New-Item -ItemType Directory -Force `
            (Join-Path $runRoot ("$($selection.id)-export")) | Out-Null
        New-Item -ItemType Directory -Force `
            (Join-Path $runRoot ("$($selection.id)-roundtrip")) | Out-Null
    }

    $arguments = @(
        "--assets", $AssetRoot,
        "--output-root", $runRoot,
        "--implementation", $implementation,
        "--source-commit", $sourceCommit,
        "--sample", "0",
        "--order-seed", "38",
        "--warmup"
    )
    foreach ($selection in $manifest.selection) {
        $arguments += @("--case", [string]$selection.id)
    }

    & $executable.FullName @arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Corpus extension/resource round-trip validation failed"
    }
}
finally {
    if (Test-Path $marker -PathType Leaf) {
        Remove-Item $runRoot -Recurse -Force
    }
}

Write-Host (
    "Validated {0} corpus cases with {1} at {2}" -f
    @($manifest.selection).Count,
    $implementation,
    $sourceCommit)
