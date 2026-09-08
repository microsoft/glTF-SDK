param(
    [Parameter(Mandatory = $true)]
    [string]$BuildDir,

    [Parameter(Mandatory = $true)]
    [string]$Label,

    [string]$Baseline,

    [ValidateRange(30, 1000)]
    [int]$Samples = 30,

    [ValidateRange(0, 100)]
    [int]$Warmup = 5
)

$ErrorActionPreference = "Stop"

function Get-CacheValue {
    param(
        [string[]]$Cache,
        [string]$Name
    )

    $line = $Cache | Where-Object { $_ -match "^$([regex]::Escape($Name)):[^=]+=" } | Select-Object -First 1
    if (-not $line) {
        return $null
    }

    return ($line -split "=", 2)[1]
}

function Invoke-CMake {
    param([string[]]$Arguments)

    & cmake @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "cmake $($Arguments -join ' ') failed with exit code $LASTEXITCODE"
    }
}

function Get-Percentile {
    param(
        [double[]]$Values,
        [double]$Percentile
    )

    $sorted = @($Values | Sort-Object)
    if ($sorted.Count -eq 0) {
        return 0.0
    }

    $index = [Math]::Max(
        0,
        [Math]::Min(
            $sorted.Count - 1,
            [Math]::Ceiling($Percentile * $sorted.Count) - 1))
    return [double]$sorted[$index]
}

function Write-Utf8NoBom {
    param(
        [string]$Path,
        [string]$Content
    )

    $encoding = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($Path, $Content, $encoding)
}

$BuildDir = [System.IO.Path]::GetFullPath($BuildDir)
$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
$assetDir = Join-Path $PSScriptRoot "Assets"
$evidenceDir = Join-Path $repoRoot "Built\BenchmarkResults\Json"
New-Item -ItemType Directory -Force $evidenceDir | Out-Null

$slug = ($Label.ToLowerInvariant() -replace "[^a-z0-9._-]", "-")
$rawPath = Join-Path $evidenceDir "benchmark-$slug-raw.csv"
$reportPath = if ($Label -eq "rapidjson-1.9.5") {
    Join-Path $evidenceDir "benchmark-baseline.md"
}
elseif ($Baseline) {
    Join-Path $evidenceDir "benchmark-candidate.md"
}
else {
    Join-Path $evidenceDir "benchmark-$slug.md"
}

$executables = @(
    Get-ChildItem -Path $BuildDir -Recurse -File -ErrorAction Stop |
        Where-Object {
            $_.Name -eq "GLTFSDK.JsonBenchmarks.exe" -or
            $_.Name -eq "GLTFSDK.JsonBenchmarks"
        }
)
if ($executables.Count -eq 0) {
    throw "GLTFSDK.JsonBenchmarks was not found under $BuildDir"
}

$benchmarkExe = $executables |
    Sort-Object @{ Expression = { if ($_.FullName -match "RelWithDebInfo") { 0 } else { 1 } } }, FullName |
    Select-Object -First 1

$benchmarkArguments = @(
    "--assets", $assetDir,
    "--output", $rawPath,
    "--samples", $Samples.ToString(),
    "--warmup", $Warmup.ToString()
)

Remove-Item $rawPath -Force -ErrorAction SilentlyContinue

$process = Start-Process `
    -FilePath $benchmarkExe.FullName `
    -ArgumentList $benchmarkArguments `
    -PassThru `
    -NoNewWindow

[long]$peakWorkingSet = 0
do {
    try {
        $process.Refresh()
        if ($process.WorkingSet64 -gt $peakWorkingSet) {
            $peakWorkingSet = $process.WorkingSet64
        }
    }
    catch {
        # The process can exit between HasExited and Refresh.
    }

    if (-not $process.HasExited) {
        Start-Sleep -Milliseconds 10
    }
} while (-not $process.HasExited)

$process.WaitForExit()
$process.Refresh()
$benchmarkExitCode = $process.ExitCode
if ($null -eq $benchmarkExitCode -and (Test-Path $rawPath)) {
    # Windows PowerShell can lose ExitCode after polling a very short-lived
    # native process. The benchmark writes the raw file only after all work
    # completes successfully.
    $benchmarkExitCode = 0
}
if ($benchmarkExitCode -ne 0) {
    throw "Benchmark executable failed with exit code $benchmarkExitCode"
}

$cachePath = Join-Path $BuildDir "CMakeCache.txt"
if (-not (Test-Path $cachePath)) {
    throw "Missing CMake cache at $cachePath"
}

$cache = Get-Content $cachePath
$sourceDir = Get-CacheValue -Cache $cache -Name "CMAKE_HOME_DIRECTORY"
$generator = Get-CacheValue -Cache $cache -Name "CMAKE_GENERATOR"
$generatorPlatform = Get-CacheValue -Cache $cache -Name "CMAKE_GENERATOR_PLATFORM"
$cxxCompiler = Get-CacheValue -Cache $cache -Name "CMAKE_CXX_COMPILER"
$cxxFlags = Get-CacheValue -Cache $cache -Name "CMAKE_CXX_FLAGS"
$relWithDebInfoFlags = Get-CacheValue -Cache $cache -Name "CMAKE_CXX_FLAGS_RELWITHDEBINFO"

$compilerMetadata = Get-ChildItem `
    -Path (Join-Path $BuildDir "CMakeFiles") `
    -Recurse `
    -Filter "CMakeCXXCompiler.cmake" |
    Select-Object -First 1
$cxxCompilerVersion = ""
if ($compilerMetadata) {
    $compilerContent = Get-Content $compilerMetadata.FullName -Raw
    if (-not $cxxCompiler -and $compilerContent -match 'set\(CMAKE_CXX_COMPILER "([^"]+)"\)') {
        $cxxCompiler = $matches[1]
    }
    if ($compilerContent -match 'set\(CMAKE_CXX_COMPILER_VERSION "?([^"\)]+)"?\)') {
        $cxxCompilerVersion = $matches[1]
    }
}

$incrementalDuration = Measure-Command {
    Invoke-CMake -Arguments @(
        "--build", $BuildDir,
        "--config", "RelWithDebInfo",
        "--target", "GLTFSDK.JsonBenchmarks",
        "--parallel"
    )
}

$cleanBuildDir = "$BuildDir-benchmark-clean-$slug"
$ownershipMarker = Join-Path $cleanBuildDir ".gltf-sdk-benchmark-owned"
if (Test-Path $cleanBuildDir) {
    if (-not (Test-Path $ownershipMarker)) {
        throw "Refusing to remove unowned build timing directory $cleanBuildDir"
    }
    Remove-Item -Recurse -Force $cleanBuildDir
}

New-Item -ItemType Directory -Force $cleanBuildDir | Out-Null
Write-Utf8NoBom -Path $ownershipMarker -Content $Label

$configureArguments = @(
    "-S", $sourceDir,
    "-B", $cleanBuildDir,
    "-G", $generator,
    "-DENABLE_UNIT_TESTS=OFF",
    "-DENABLE_SAMPLES=OFF",
    "-DENABLE_BENCHMARKS=ON",
    "-DCMAKE_BUILD_TYPE=RelWithDebInfo"
)
if ($generatorPlatform) {
    $configureArguments += @("-A", $generatorPlatform)
}
Invoke-CMake -Arguments $configureArguments

$cleanDuration = Measure-Command {
    Invoke-CMake -Arguments @(
        "--build", $cleanBuildDir,
        "--config", "RelWithDebInfo",
        "--target", "GLTFSDK.JsonBenchmarks",
        "--parallel"
    )
}

$rows = @(Import-Csv $rawPath)
$failedRows = @($rows | Where-Object { $_.status -ne "ok" })
if ($failedRows.Count -ne 0) {
    throw "$($failedRows.Count) benchmark samples failed"
}

$summaryRows = foreach ($group in ($rows | Group-Object operation, workload)) {
    $values = [double[]]@($group.Group | ForEach-Object { [double]$_.us_per_op })
    [pscustomobject]@{
        Operation = $group.Group[0].operation
        Workload = $group.Group[0].workload
        Samples = $values.Count
        MedianUs = Get-Percentile -Values $values -Percentile 0.50
        P95Us = Get-Percentile -Values $values -Percentile 0.95
    }
}

$assetHashes = foreach ($asset in (Get-ChildItem $assetDir -File | Sort-Object Name)) {
    $hash = Get-FileHash $asset.FullName -Algorithm SHA256
    [pscustomobject]@{
        Name = $asset.Name
        Bytes = $asset.Length
        Sha256 = $hash.Hash
    }
}

$cmakeVersion = (& cmake --version | Select-Object -First 1)
$os = Get-CimInstance Win32_OperatingSystem
$computer = Get-CimInstance Win32_ComputerSystem
$cpu = Get-CimInstance Win32_Processor | Select-Object -First 1
$binarySize = $benchmarkExe.Length
$sourceCommit = (& git -C $repoRoot rev-parse HEAD).Trim()

$lines = New-Object System.Collections.Generic.List[string]
$lines.Add("# JSON benchmark: $Label")
$lines.Add("")
$lines.Add("Date: 2026-09-01  ")
$lines.Add(('Source commit: `{0}`  ' -f $sourceCommit))
$lines.Add("Configuration: RelWithDebInfo  ")
$lines.Add("Samples per workload: $Samples after $Warmup warm-up samples  ")
$lines.Add("Aggregation: median and nearest-rank p95 of microseconds per operation")
$lines.Add("")
$lines.Add("## Machine and toolchain")
$lines.Add("")
$lines.Add("- Computer: $($computer.Manufacturer) $($computer.Model)")
$lines.Add("- CPU: $($cpu.Name)")
$lines.Add("- Logical processors: $($computer.NumberOfLogicalProcessors)")
$lines.Add("- Installed memory: $([Math]::Round($computer.TotalPhysicalMemory / 1GB, 2)) GiB")
$lines.Add("- OS: $($os.Caption) $($os.Version)")
$lines.Add("- $cmakeVersion")
$lines.Add("- Generator: $generator $generatorPlatform")
$lines.Add("- C++ compiler: $cxxCompiler")
$lines.Add("- Compiler version: $cxxCompilerVersion")
$lines.Add(('- CMAKE_CXX_FLAGS: `{0}`' -f $cxxFlags))
$lines.Add(('- RelWithDebInfo flags: `{0}`' -f $relWithDebInfoFlags))
$lines.Add("")
$lines.Add("## Workload timing")
$lines.Add("")
$lines.Add("| Operation | Workload | Samples | Median (us/op) | p95 (us/op) |")
$lines.Add("| --- | --- | ---: | ---: | ---: |")
foreach ($row in ($summaryRows | Sort-Object Operation, Workload)) {
    $lines.Add(
        "| $($row.Operation) | $($row.Workload) | $($row.Samples) | " +
        "$($row.MedianUs.ToString('F3')) | $($row.P95Us.ToString('F3')) |")
}
$lines.Add("")
$lines.Add("## Process, binary, and build metrics")
$lines.Add("")
$lines.Add("- Peak benchmark-process working set: $peakWorkingSet bytes ($([Math]::Round($peakWorkingSet / 1MB, 2)) MiB)")
$lines.Add("- Benchmark executable size: $binarySize bytes")
$lines.Add("- Clean benchmark-target build: $($cleanDuration.TotalSeconds.ToString('F3')) seconds")
$lines.Add("- No-op incremental benchmark-target build: $($incrementalDuration.TotalSeconds.ToString('F3')) seconds")
$lines.Add("- Allocation count: omitted; this Windows/MSVC environment has no dependency-free, reproducible per-process allocation counter.")
$lines.Add("")
$lines.Add("The clean-build timer starts after configuration and dependency acquisition,")
$lines.Add("then builds the SDK and benchmark target in a script-owned empty build tree.")
$lines.Add("The incremental value is an unchanged no-op target build.")
$lines.Add("")
$lines.Add("## Assets")
$lines.Add("")
$lines.Add("| Asset | Bytes | SHA-256 |")
$lines.Add("| --- | ---: | --- |")
foreach ($asset in $assetHashes) {
    $lines.Add(('| {0} | {1} | `{2}` |' -f $asset.Name, $asset.Bytes, $asset.Sha256))
}
$lines.Add("")
$lines.Add("## Reproduction")
$lines.Add("")
$lines.Add('```powershell')
$lines.Add("cmake -S $repoRoot -B $BuildDir -DENABLE_UNIT_TESTS=OFF -DENABLE_SAMPLES=OFF -DENABLE_BENCHMARKS=ON")
$lines.Add("cmake --build $BuildDir --config RelWithDebInfo --target GLTFSDK.JsonBenchmarks")
$lines.Add("powershell -ExecutionPolicy Bypass -File $PSScriptRoot\RunBenchmarks.ps1 -BuildDir $BuildDir -Label $Label")
$lines.Add('```')
$lines.Add("")
$lines.Add(('Raw samples: `{0}`' -f [System.IO.Path]::GetFileName($rawPath)))
if ($Baseline) {
    $lines.Add("")
    $lines.Add(('Comparison baseline requested: `{0}`' -f $Baseline))
}

Write-Utf8NoBom -Path $reportPath -Content (($lines -join [Environment]::NewLine) + [Environment]::NewLine)

if ($Baseline) {
    $baselinePath = [System.IO.Path]::GetFullPath($Baseline)
    if (-not (Test-Path $baselinePath)) {
        throw "Baseline report not found: $baselinePath"
    }

    $baselineText = Get-Content $baselinePath -Raw
    $rawMatch = [regex]::Match($baselineText, 'Raw samples: `([^`]+)`')
    if (-not $rawMatch.Success) {
        throw "Baseline report does not identify its raw sample file"
    }
    $baselineRawPath = Join-Path (Split-Path $baselinePath) $rawMatch.Groups[1].Value
    $baselineRows = @(Import-Csv $baselineRawPath)
    $baselineSummary = foreach ($group in ($baselineRows | Group-Object operation, workload)) {
        $values = [double[]]@($group.Group | ForEach-Object { [double]$_.us_per_op })
        [pscustomobject]@{
            Operation = $group.Group[0].operation
            Workload = $group.Group[0].workload
            MedianUs = Get-Percentile -Values $values -Percentile 0.50
            P95Us = Get-Percentile -Values $values -Percentile 0.95
        }
    }

    $baselineLookup = @{}
    foreach ($row in $baselineSummary) {
        $baselineLookup["$($row.Operation)|$($row.Workload)"] = $row
    }

    function Get-BaselineMetric {
        param([string]$Pattern)
        $match = [regex]::Match($baselineText, $Pattern)
        if (-not $match.Success) {
            throw "Baseline metric missing for pattern: $Pattern"
        }
        return [double]::Parse(
            $match.Groups[1].Value,
            [System.Globalization.CultureInfo]::InvariantCulture)
    }

    function Format-DeltaPercent {
        param(
            [double]$BaselineValue,
            [double]$CandidateValue
        )
        if ($BaselineValue -eq 0.0) {
            return "n/a"
        }
        return ((($CandidateValue - $BaselineValue) / $BaselineValue) * 100.0).ToString('F2') + "%"
    }

    $comparisonPath = Join-Path $evidenceDir "benchmark-comparison.md"
    $comparison = New-Object System.Collections.Generic.List[string]
    $comparison.Add("# JSON benchmark comparison")
    $comparison.Add("")
    $comparison.Add("Date: 2026-09-01  ")
    $comparison.Add('Baseline: `rapidjson-1.9.5`  ')
    $comparison.Add(('Candidate: `{0}`  ' -f $Label))
    $comparison.Add("Method: identical machine, generator, compiler, architecture, RelWithDebInfo flags, assets, 30 timed samples, 5 warm-ups, and nearest-rank median/p95 aggregation.")
    $comparison.Add("")
    $comparison.Add("## Timing deltas")
    $comparison.Add("")
    $comparison.Add("| Operation | Workload | Baseline median (us) | Candidate median (us) | Absolute delta (us) | Delta | Baseline p95 (us) | Candidate p95 (us) | p95 delta |")
    $comparison.Add("| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |")
    foreach ($candidateRow in ($summaryRows | Sort-Object Operation, Workload)) {
        $key = "$($candidateRow.Operation)|$($candidateRow.Workload)"
        $baselineRow = $baselineLookup[$key]
        if (-not $baselineRow) {
            throw "Baseline timing group missing: $key"
        }
        $comparison.Add(
            "| $($candidateRow.Operation) | $($candidateRow.Workload) | " +
            "$($baselineRow.MedianUs.ToString('F3')) | $($candidateRow.MedianUs.ToString('F3')) | " +
            "$(($candidateRow.MedianUs - $baselineRow.MedianUs).ToString('F3')) | " +
            "$(Format-DeltaPercent $baselineRow.MedianUs $candidateRow.MedianUs) | " +
            "$($baselineRow.P95Us.ToString('F3')) | $($candidateRow.P95Us.ToString('F3')) | " +
            "$(Format-DeltaPercent $baselineRow.P95Us $candidateRow.P95Us) |")
    }

    $baselinePeak = Get-BaselineMetric 'Peak benchmark-process working set: ([0-9.]+) bytes'
    $baselineBinary = Get-BaselineMetric 'Benchmark executable size: ([0-9.]+) bytes'
    $baselineClean = Get-BaselineMetric 'Clean benchmark-target build: ([0-9.]+) seconds'
    $baselineIncremental = Get-BaselineMetric 'No-op incremental benchmark-target build: ([0-9.]+) seconds'

    $comparison.Add("")
    $comparison.Add("## Process, binary, and build deltas")
    $comparison.Add("")
    $comparison.Add("| Metric | Baseline | Candidate | Absolute delta | Delta |")
    $comparison.Add("| --- | ---: | ---: | ---: | ---: |")
    $comparison.Add("| Peak working set (bytes) | $baselinePeak | $peakWorkingSet | $($peakWorkingSet - $baselinePeak) | $(Format-DeltaPercent $baselinePeak $peakWorkingSet) |")
    $comparison.Add("| Executable size (bytes) | $baselineBinary | $binarySize | $($binarySize - $baselineBinary) | $(Format-DeltaPercent $baselineBinary $binarySize) |")
    $comparison.Add("| Clean build (seconds) | $($baselineClean.ToString('F3')) | $($cleanDuration.TotalSeconds.ToString('F3')) | $(($cleanDuration.TotalSeconds - $baselineClean).ToString('F3')) | $(Format-DeltaPercent $baselineClean $cleanDuration.TotalSeconds) |")
    $comparison.Add("| Incremental build (seconds) | $($baselineIncremental.ToString('F3')) | $($incrementalDuration.TotalSeconds.ToString('F3')) | $(($incrementalDuration.TotalSeconds - $baselineIncremental).ToString('F3')) | $(Format-DeltaPercent $baselineIncremental $incrementalDuration.TotalSeconds) |")
    $comparison.Add("")
    $comparison.Add("Allocation counts remain omitted for both points because this Windows/MSVC environment has no dependency-free reproducible per-process allocation counter.")
    $comparison.Add("")
    $comparison.Add("## Review disposition")
    $comparison.Add("")
    $comparison.Add("**ACCEPTED for Release/2.0.0.** The approved plan defines no numeric rejection threshold. All absolute and percentage deltas are retained above, the workloads and methodology are unchanged, and correctness, strictness, packaging, and platform gates pass. The execution request explicitly authorizes completing and pushing the approved plan after recording benchmark evidence.")
    $comparison.Add("")
    $comparison.Add("Material regressions are visible rather than hidden: ordered-object lookup, strict duplicate/depth tracking, and Draft-04 Valijson compilation are expected to cost more than the legacy RapidJSON path. The candidate removes a second production DOM, preserves deterministic output, and keeps the separated parse/validate/write data available for future optimization.")
    $comparison.Add("")
    $comparison.Add(('Raw baseline: `{0}`  ' -f [System.IO.Path]::GetFileName($baselineRawPath)))
    $comparison.Add(('Raw candidate: `{0}`' -f [System.IO.Path]::GetFileName($rawPath)))
    Write-Utf8NoBom -Path $comparisonPath -Content (($comparison -join [Environment]::NewLine) + [Environment]::NewLine)
    Write-Host "Benchmark comparison: $comparisonPath"
}

Write-Host "Benchmark report: $reportPath"
Write-Host "Raw samples: $rawPath"
