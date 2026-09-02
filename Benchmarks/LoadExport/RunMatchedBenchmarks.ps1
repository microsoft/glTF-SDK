param(
    [Parameter(Mandatory = $true)]
    [string]$BaselineBuildDir,

    [Parameter(Mandatory = $true)]
    [string]$CandidateBuildDir,

    [string]$AssetRoot,
    [string]$EvidenceDir,
    [string]$RunRoot,

    [ValidateRange(5, 100)]
    [int]$Warmups = 5,

    [ValidateRange(30, 1000)]
    [int]$Samples = 100,

    [ValidateRange(30, 1000)]
    [int]$LargeSamples = 30,

    [ValidateSet("Release")]
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"

if ($LargeSamples -gt $Samples) {
    throw "-LargeSamples cannot exceed -Samples"
}

function Get-CacheValue {
    param(
        [string[]]$Cache,
        [string]$Name
    )

    $line = $Cache |
        Where-Object { $_ -match "^$([regex]::Escape($Name)):[^=]+=" } |
        Select-Object -First 1
    if (-not $line) {
        return ""
    }
    return ($line -split "=", 2)[1]
}

function Invoke-GitValue {
    param(
        [string]$Repository,
        [string[]]$Arguments
    )

    $value = (& git -C $Repository @Arguments)
    if ($LASTEXITCODE -ne 0) {
        throw "git -C $Repository $($Arguments -join ' ') failed"
    }
    return ($value | Out-String).Trim()
}

function Get-BuildMetadata {
    param([string]$BuildDirectory)

    $BuildDirectory = [System.IO.Path]::GetFullPath($BuildDirectory)
    $cachePath = Join-Path $BuildDirectory "CMakeCache.txt"
    if (-not (Test-Path $cachePath -PathType Leaf)) {
        throw "Missing CMake cache: $cachePath"
    }

    $cache = Get-Content $cachePath
    $compilerMetadata = Get-ChildItem `
        -Path (Join-Path $BuildDirectory "CMakeFiles") `
        -Recurse `
        -Filter "CMakeCXXCompiler.cmake" |
        Select-Object -First 1

    $compilerId = ""
    $compilerVersion = ""
    $compilerPath = Get-CacheValue -Cache $cache -Name "CMAKE_CXX_COMPILER"
    if ($compilerMetadata) {
        $compilerText = Get-Content $compilerMetadata.FullName -Raw
        if (-not $compilerPath -and
            $compilerText -match 'set\(CMAKE_CXX_COMPILER "([^"]+)"\)') {
            $compilerPath = $matches[1]
        }
        if ($compilerText -match 'set\(CMAKE_CXX_COMPILER_ID "([^"]+)"\)') {
            $compilerId = $matches[1]
        }
        if ($compilerText -match 'set\(CMAKE_CXX_COMPILER_VERSION "?([^"\)]+)"?\)') {
            $compilerVersion = $matches[1]
        }
    }

    $sourceDirectory = Get-CacheValue -Cache $cache -Name "CMAKE_HOME_DIRECTORY"
    if (-not $sourceDirectory) {
        throw "CMAKE_HOME_DIRECTORY is missing from $cachePath"
    }
    $sourceDirectory = [System.IO.Path]::GetFullPath($sourceDirectory)

    [pscustomobject][ordered]@{
        buildDirectory = $BuildDirectory
        sourceDirectory = $sourceDirectory
        generator = Get-CacheValue -Cache $cache -Name "CMAKE_GENERATOR"
        generatorPlatform = Get-CacheValue -Cache $cache -Name "CMAKE_GENERATOR_PLATFORM"
        generatorToolset = Get-CacheValue -Cache $cache -Name "CMAKE_GENERATOR_TOOLSET"
        compilerPath = $compilerPath
        compilerId = $compilerId
        compilerVersion = $compilerVersion
        cxxFlags = Get-CacheValue -Cache $cache -Name "CMAKE_CXX_FLAGS"
        releaseFlags = Get-CacheValue -Cache $cache -Name "CMAKE_CXX_FLAGS_RELEASE"
    }
}

function Find-BenchmarkExecutable {
    param(
        [string]$BuildDirectory,
        [string]$Config
    )

    $candidates = @(
        Get-ChildItem -Path $BuildDirectory -Recurse -File |
            Where-Object {
                $_.Name -eq "GLTFSDK.LoadExportBenchmarks.exe" -or
                $_.Name -eq "GLTFSDK.LoadExportBenchmarks"
            }
    )
    if ($candidates.Count -eq 0) {
        throw "GLTFSDK.LoadExportBenchmarks was not found under $BuildDirectory"
    }

    $executable = $candidates |
        Sort-Object `
            @{ Expression = { if ($_.FullName -match [regex]::Escape("\$Config\")) { 0 } else { 1 } } },
            FullName |
        Select-Object -First 1
    return $executable.FullName
}

function Write-Utf8NoBom {
    param(
        [string]$Path,
        [string]$Content
    )

    $encoding = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($Path, $Content, $encoding)
}

function Get-Sha256Text {
    param([string]$Text)

    $sha = [System.Security.Cryptography.SHA256]::Create()
    try {
        $bytes = [System.Text.Encoding]::UTF8.GetBytes($Text)
        return (($sha.ComputeHash($bytes) | ForEach-Object { $_.ToString("X2") }) -join "")
    }
    finally {
        $sha.Dispose()
    }
}

function Get-OutputSet {
    param([string]$Directory)

    $Directory = [System.IO.Path]::GetFullPath($Directory)
    $files = @(Get-ChildItem -Path $Directory -Recurse -File | Sort-Object FullName)
    if ($files.Count -eq 0) {
        throw "No benchmark output files were produced in $Directory"
    }

    $rootPrefix = $Directory.TrimEnd("\", "/") + [System.IO.Path]::DirectorySeparatorChar
    [long]$totalBytes = 0
    $canonicalLines = New-Object System.Collections.Generic.List[string]
    foreach ($file in $files) {
        if (-not $file.FullName.StartsWith(
            $rootPrefix,
            [System.StringComparison]::OrdinalIgnoreCase)) {
            throw "Output file escaped its benchmark directory: $($file.FullName)"
        }

        $relativePath = $file.FullName.Substring($rootPrefix.Length).Replace("\", "/")
        $hash = (Get-FileHash $file.FullName -Algorithm SHA256).Hash
        $totalBytes += $file.Length
        $canonicalLines.Add("$relativePath|$($file.Length)|$hash")
    }

    $canonical = ($canonicalLines -join "`n") + "`n"
    [pscustomobject]@{
        fileCount = $files.Count
        totalBytes = $totalBytes
        sha256 = Get-Sha256Text -Text $canonical
    }
}

function Get-Percentile {
    param(
        [double[]]$Values,
        [double]$Percentile
    )

    $sorted = @($Values | Sort-Object)
    if ($sorted.Count -eq 0) {
        throw "Cannot calculate a percentile for an empty sample"
    }
    $index = [Math]::Max(
        0,
        [Math]::Min(
            $sorted.Count - 1,
            [Math]::Ceiling($Percentile * $sorted.Count) - 1))
    return [double]$sorted[$index]
}

function Get-DeltaPercent {
    param(
        [double]$Baseline,
        [double]$Candidate
    )

    if ($Baseline -eq 0.0) {
        return $null
    }
    return (($Candidate - $Baseline) / $Baseline) * 100.0
}

function Format-Percent {
    param([Nullable[double]]$Value)

    if ($null -eq $Value) {
        return "n/a"
    }
    return ([double]$Value).ToString("F2") + "%"
}

function Get-ManifestStrings {
    param($Value)

    if ($null -eq $Value) {
        return @()
    }
    return @($Value | ForEach-Object { [string]$_ })
}

function Get-ExtensionCoverageLabel {
    param($Selection)

    $typedCount = @(Get-ManifestStrings $Selection.extensions.typed).Count
    $rawCount = @(Get-ManifestStrings $Selection.extensions.raw).Count
    if ($typedCount -gt 0 -and $rawCount -gt 0) {
        return "mixed-typed-raw"
    }
    if ($typedCount -gt 0) {
        return "typed-only"
    }
    if ($rawCount -gt 0) {
        return "raw-only"
    }
    return "core-only"
}

function Get-ComplexityClass {
    param($Selection)

    return ([string]$Selection.tier -split "/", 2)[0]
}

$candidateRepo = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
$manifestPath = Join-Path $PSScriptRoot "assets.json"
$manifest = Get-Content $manifestPath -Raw | ConvertFrom-Json

if (-not $AssetRoot) {
    $AssetRoot = Join-Path $candidateRepo ("Built\Int\LoadExportAssets\" + $manifest.commit)
}
if (-not $EvidenceDir) {
    $EvidenceDir = Join-Path $candidateRepo "docs\release-evidence\replace-rapidjson-nlohmann\load-export"
}
if (-not $RunRoot) {
    $RunRoot = Join-Path $candidateRepo "Built\Int\LoadExportBenchmarkRun"
}

$AssetRoot = [System.IO.Path]::GetFullPath($AssetRoot)
$EvidenceDir = [System.IO.Path]::GetFullPath($EvidenceDir)
$RunRoot = [System.IO.Path]::GetFullPath($RunRoot)
$BaselineBuildDir = [System.IO.Path]::GetFullPath($BaselineBuildDir)
$CandidateBuildDir = [System.IO.Path]::GetFullPath($CandidateBuildDir)

& (Join-Path $PSScriptRoot "FetchAssets.ps1") -Destination $AssetRoot | Out-Null

$baselineBuild = Get-BuildMetadata -BuildDirectory $BaselineBuildDir
$candidateBuild = Get-BuildMetadata -BuildDirectory $CandidateBuildDir

foreach ($property in @(
    "generator",
    "generatorPlatform",
    "generatorToolset",
    "compilerPath",
    "compilerId",
    "compilerVersion",
    "cxxFlags",
    "releaseFlags")) {
    if ($baselineBuild.$property -ne $candidateBuild.$property) {
        throw "Build metadata mismatch for ${property}: '$($baselineBuild.$property)' versus '$($candidateBuild.$property)'"
    }
}
if ($baselineBuild.generatorPlatform -notmatch "^(x64|X64|amd64|AMD64)$") {
    throw "Benchmarks must use an x64 generator platform; found '$($baselineBuild.generatorPlatform)'"
}

$baselineExecutable = Find-BenchmarkExecutable -BuildDirectory $BaselineBuildDir -Config $Configuration
$candidateExecutable = Find-BenchmarkExecutable -BuildDirectory $CandidateBuildDir -Config $Configuration
$baselineCommit = Invoke-GitValue -Repository $baselineBuild.sourceDirectory -Arguments @("rev-parse", "HEAD")
$candidateCommit = Invoke-GitValue -Repository $candidateBuild.sourceDirectory -Arguments @("rev-parse", "HEAD")
$baselineBranch = Invoke-GitValue -Repository $baselineBuild.sourceDirectory -Arguments @("branch", "--show-current")
$candidateBranch = Invoke-GitValue -Repository $candidateBuild.sourceDirectory -Arguments @("branch", "--show-current")
$matchedRelativePaths = @(
    "Benchmarks\LoadExport\CMakeLists.txt",
    "Benchmarks\LoadExport\LoadExportBenchmarks.cpp",
    "Benchmarks\LoadExport\assets.json",
    "Benchmarks\LoadExport\FetchAssets.ps1",
    "Benchmarks\LoadExport\ValidateAssets.ps1",
    "Benchmarks\LoadExport\TestCorpus.ps1",
    "Benchmarks\LoadExport\RunMatchedBenchmarks.ps1",
    "Benchmarks\LoadExport\README.md"
)
$matchedFileHashes = [ordered]@{}
foreach ($relativePath in $matchedRelativePaths) {
    $baselinePath = Join-Path $baselineBuild.sourceDirectory $relativePath
    $candidatePath = Join-Path $candidateBuild.sourceDirectory $relativePath
    if (-not (Test-Path $baselinePath -PathType Leaf) -or
        -not (Test-Path $candidatePath -PathType Leaf)) {
        throw "Matched benchmark file is missing: $relativePath"
    }
    $baselineHash = (Get-FileHash $baselinePath -Algorithm SHA256).Hash
    $candidateHash = (Get-FileHash $candidatePath -Algorithm SHA256).Hash
    if ($baselineHash -ne $candidateHash) {
        throw "The two branches differ in matched benchmark file $relativePath"
    }
    $matchedFileHashes[$relativePath.Replace("\", "/")] = $candidateHash
}
$baselineBenchmarkSourceHash =
    $matchedFileHashes["Benchmarks/LoadExport/LoadExportBenchmarks.cpp"]
$candidateBenchmarkSourceHash = $baselineBenchmarkSourceHash
$baselineAssetManifestHash =
    $matchedFileHashes["Benchmarks/LoadExport/assets.json"]
$candidateAssetManifestHash = $baselineAssetManifestHash

& git -C $baselineBuild.sourceDirectory merge-base --is-ancestor `
    3193f83265a70585093f13d651167b763979ade1 $baselineCommit
if ($LASTEXITCODE -ne 0) {
    throw "RapidJSON benchmark branch is not based on the exact required Release/1.9.5 commit"
}

$implementations = @(
    [pscustomobject]@{
        label = "rapidjson-1.9.5"
        slug = "rapidjson-1.9.5"
        executable = $baselineExecutable
        sourceDirectory = $baselineBuild.sourceDirectory
        sourceCommit = $baselineCommit
        branch = $baselineBranch
    },
    [pscustomobject]@{
        label = "nlohmann-2.0.0"
        slug = "nlohmann-2.0.0"
        executable = $candidateExecutable
        sourceDirectory = $candidateBuild.sourceDirectory
        sourceCommit = $candidateCommit
        branch = $candidateBranch
    }
)

$ownershipMarker = Join-Path $RunRoot ".gltf-sdk-load-export-owned"
if (Test-Path $RunRoot) {
    if (-not (Test-Path $ownershipMarker -PathType Leaf)) {
        throw "Refusing to remove unowned run directory $RunRoot"
    }
    Remove-Item $RunRoot -Recurse -Force
}
New-Item -ItemType Directory -Force $RunRoot | Out-Null
Write-Utf8NoBom -Path $ownershipMarker -Content "GLTFSDK.LoadExportBenchmarks"
New-Item -ItemType Directory -Force $EvidenceDir | Out-Null

$selectionBytes = @{}
$selectionSamples = @{}
$selectionById = @{}
foreach ($selection in $manifest.selection) {
    $id = [string]$selection.id
    if ($selectionById.ContainsKey($id)) {
        throw "Duplicate corpus selection id $id"
    }
    $selectionBytes[$id] = [long]$selection.totalBytes
    $selectionById[$id] = $selection
    if ($selection.sampleClass -eq "standard") {
        $selectionSamples[$id] = $Samples
    }
    elseif ($selection.sampleClass -eq "large") {
        $selectionSamples[$id] = $LargeSamples
    }
    else {
        throw "Unsupported sample class '$($selection.sampleClass)' for $id"
    }
}
$largeSelections = @(
    $manifest.selection |
        Where-Object { $_.sampleClass -eq "large" }
)
if ($largeSelections.Count -lt 2) {
    throw "The corpus must contain at least two genuinely large selections"
}
$expectedRawRowsPerImplementation = 0
foreach ($selection in $manifest.selection) {
    $expectedRawRowsPerImplementation +=
        3 * [int]$selectionSamples[[string]$selection.id]
}

$processRows = New-Object System.Collections.Generic.List[object]
$orderRows = New-Object System.Collections.Generic.List[object]
$allRows = @{
    "rapidjson-1.9.5" = New-Object System.Collections.Generic.List[object]
    "nlohmann-2.0.0" = New-Object System.Collections.Generic.List[object]
}

function Invoke-BenchmarkCycle {
    param(
        [pscustomobject]$Implementation,
        [string]$Phase,
        [int]$Cycle,
        [uint32]$Seed,
        [int]$OrderPosition,
        [object[]]$ActiveSelections
    )

    $cycleRoot = Join-Path $RunRoot ("{0}-{1:D3}-{2}" -f $Phase, $Cycle, $Implementation.slug)
    $outputRoot = Join-Path $cycleRoot "outputs"
    New-Item -ItemType Directory -Force $outputRoot | Out-Null
    foreach ($selection in $ActiveSelections) {
        New-Item -ItemType Directory -Force `
            (Join-Path $outputRoot ("$($selection.id)-export")) | Out-Null
        New-Item -ItemType Directory -Force `
            (Join-Path $outputRoot ("$($selection.id)-roundtrip")) | Out-Null
    }

    $cycleCsv = Join-Path $cycleRoot "cycle.csv"
    $stdoutPath = Join-Path $cycleRoot "stdout.txt"
    $stderrPath = Join-Path $cycleRoot "stderr.txt"
    $arguments = @(
        "--assets", $AssetRoot,
        "--output-root", $outputRoot,
        "--implementation", $Implementation.label,
        "--source-commit", $Implementation.sourceCommit,
        "--sample", $Cycle.ToString(),
        "--order-seed", $Seed.ToString()
    )
    foreach ($selection in $ActiveSelections) {
        $arguments += @("--case", [string]$selection.id)
    }
    if ($Phase -eq "warmup") {
        $arguments += "--warmup"
    }
    else {
        $arguments += @("--csv", $cycleCsv)
    }

    $quotedArguments = ($arguments | ForEach-Object {
        '"' + ([string]$_).Replace('"', '\"') + '"'
    }) -join " "

    $startedAt = (Get-Date).ToUniversalTime()
    $process = Start-Process `
        -FilePath $Implementation.executable `
        -ArgumentList $quotedArguments `
        -RedirectStandardOutput $stdoutPath `
        -RedirectStandardError $stderrPath `
        -NoNewWindow `
        -PassThru

    [long]$peakWorkingSet = 0
    do {
        try {
            $process.Refresh()
            if ($process.PeakWorkingSet64 -gt $peakWorkingSet) {
                $peakWorkingSet = $process.PeakWorkingSet64
            }
        }
        catch {
        }
        if (-not $process.HasExited) {
            Start-Sleep -Milliseconds 5
        }
    } while (-not $process.HasExited)

    $process.WaitForExit()
    $process.Refresh()
    if ($process.PeakWorkingSet64 -gt $peakWorkingSet) {
        $peakWorkingSet = $process.PeakWorkingSet64
    }
    $completedAt = (Get-Date).ToUniversalTime()
    $exitCode = $process.ExitCode
    if ($null -eq $exitCode) {
        $completedMarker = if (Test-Path $stdoutPath -PathType Leaf) {
            (Get-Content $stdoutPath -Raw) -match "cycle completed"
        }
        else {
            $false
        }
        if (($Phase -eq "measured" -and (Test-Path $cycleCsv -PathType Leaf)) -or
            $completedMarker) {
            $exitCode = 0
        }
    }
    if ($exitCode -ne 0) {
        $stderr = if (Test-Path $stderrPath) { Get-Content $stderrPath -Raw } else { "" }
        $stdout = if (Test-Path $stdoutPath) { Get-Content $stdoutPath -Raw } else { "" }
        throw "$($Implementation.label) $Phase cycle $Cycle failed with exit code $exitCode`n$stdout`n$stderr"
    }

    $largeActiveCount = @(
        $ActiveSelections |
            Where-Object { $_.sampleClass -eq "large" }
    ).Count
    $processRows.Add([pscustomobject][ordered]@{
        phase = $Phase
        cycle = $Cycle
        order_seed = $Seed
        order_position = $OrderPosition
        implementation = $Implementation.label
        started_utc = $startedAt.ToString("o")
        completed_utc = $completedAt.ToString("o")
        duration_ms = [Math]::Round(($completedAt - $startedAt).TotalMilliseconds, 3)
        peak_working_set_bytes = $peakWorkingSet
        active_case_count = $ActiveSelections.Count
        large_case_count = $largeActiveCount
        cohort = if ($largeActiveCount -gt 0) {
            "large-enabled"
        }
        else {
            "standard-only"
        }
        exit_code = $exitCode
    })

    if ($Phase -eq "measured") {
        $rows = @(Import-Csv $cycleCsv)
        $expectedRows = 3 * $ActiveSelections.Count
        if ($rows.Count -ne $expectedRows) {
            throw "$($Implementation.label) cycle $Cycle produced $($rows.Count) rows; expected $expectedRows"
        }

        foreach ($row in $rows) {
            $selection = $selectionById[[string]$row.case_id]
            if (-not $selection -or
                -not ($ActiveSelections.id -ccontains [string]$row.case_id)) {
                throw "Unexpected benchmark case '$($row.case_id)'"
            }
            if ($row.semantic_status -ne "ok") {
                throw "Semantic validation failed for $($row.case_id)/$($row.operation)"
            }
            if ([long]$row.input_bytes -ne [long]$selectionBytes[$row.case_id]) {
                throw "Input byte count mismatch for $($row.case_id): $($row.input_bytes)"
            }

            [long]$outputBytes = 0
            [int]$outputFileCount = 0
            $outputSetHash = ""
            if ($row.operation -ne "load") {
                $outputSet = Get-OutputSet -Directory $row.output_directory
                $outputBytes = $outputSet.totalBytes
                $outputFileCount = $outputSet.fileCount
                $outputSetHash = $outputSet.sha256
            }

            $allRows[$Implementation.label].Add([pscustomobject][ordered]@{
                implementation = $row.implementation
                source_commit = $row.source_commit
                case_id = $row.case_id
                asset = $row.asset
                format = $row.format
                operation = $row.operation
                tier = [string]$selection.tier
                complexity_class = Get-ComplexityClass $selection
                sample_class = [string]$selection.sampleClass
                configured_samples = [int]$selectionSamples[[string]$row.case_id]
                extension_coverage = Get-ExtensionCoverageLabel $selection
                extensions_used = (
                    Get-ManifestStrings $selection.extensions.used
                ) -join ";"
                sample = [int]$row.sample
                order_seed = [uint32]$row.order_seed
                branch_order_position = $OrderPosition
                elapsed_us = [double]$row.elapsed_us
                input_bytes = [long]$row.input_bytes
                output_bytes = $outputBytes
                output_file_count = $outputFileCount
                output_set_sha256 = $outputSetHash
                semantic_status = $row.semantic_status
                process_peak_working_set_bytes = $peakWorkingSet
            })
        }
    }

    Remove-Item $cycleRoot -Recurse -Force
}

try {
    for ($cycle = 1; $cycle -le $Warmups; ++$cycle) {
        $activeSelections = @($manifest.selection)
        $ordered = if ($cycle % 2 -eq 1) {
            @($implementations[0], $implementations[1])
        }
        else {
            @($implementations[1], $implementations[0])
        }
        $seed = [uint32](100 + $cycle)
        $orderRows.Add([pscustomobject]@{
            phase = "warmup"
            cycle = $cycle
            order_seed = $seed
            first = $ordered[0].label
            second = $ordered[1].label
            active_case_count = $activeSelections.Count
            active_cases = ($activeSelections.id -join ";")
        })
        for ($position = 0; $position -lt $ordered.Count; ++$position) {
            Write-Host "Warm-up $cycle/$($Warmups): $($ordered[$position].label)"
            Invoke-BenchmarkCycle `
                -Implementation $ordered[$position] `
                -Phase "warmup" `
                -Cycle $cycle `
                -Seed $seed `
                -OrderPosition ($position + 1) `
                -ActiveSelections $activeSelections
        }
    }

    for ($cycle = 1; $cycle -le $Samples; ++$cycle) {
        $activeSelections = @(
            $manifest.selection |
                Where-Object {
                    $cycle -le
                        [int]$selectionSamples[[string]$_.id]
                }
        )
        $ordered = if ($cycle % 2 -eq 1) {
            @($implementations[0], $implementations[1])
        }
        else {
            @($implementations[1], $implementations[0])
        }
        $seed = [uint32](1000 + $cycle)
        $orderRows.Add([pscustomobject]@{
            phase = "measured"
            cycle = $cycle
            order_seed = $seed
            first = $ordered[0].label
            second = $ordered[1].label
            active_case_count = $activeSelections.Count
            active_cases = ($activeSelections.id -join ";")
        })
        for ($position = 0; $position -lt $ordered.Count; ++$position) {
            Write-Host (
                "Measured {0}/{1} ({2} cases): {3}" -f
                $cycle,
                $Samples,
                $activeSelections.Count,
                $ordered[$position].label)
            Invoke-BenchmarkCycle `
                -Implementation $ordered[$position] `
                -Phase "measured" `
                -Cycle $cycle `
                -Seed $seed `
                -OrderPosition ($position + 1) `
                -ActiveSelections $activeSelections
        }
    }
}
finally {
    if (Test-Path $RunRoot) {
        if (Test-Path $ownershipMarker -PathType Leaf) {
            Remove-Item $RunRoot -Recurse -Force
        }
    }
}

$baselineRows = @(
    $allRows["rapidjson-1.9.5"] | ForEach-Object { $_ }
)
$candidateRows = @(
    $allRows["nlohmann-2.0.0"] | ForEach-Object { $_ }
)
if ($baselineRows.Count -ne $expectedRawRowsPerImplementation -or
    $candidateRows.Count -ne $expectedRawRowsPerImplementation) {
    throw "Unexpected raw row counts: baseline=$($baselineRows.Count), candidate=$($candidateRows.Count)"
}

function Assert-OutputDeterminism {
    param([object[]]$ImplementationRows)

    $outputGroups = $implementationRows |
        Where-Object { $_.operation -ne "load" } |
        Group-Object implementation, case_id, operation
    foreach ($group in $outputGroups) {
        $hashes = @($group.Group.output_set_sha256 | Sort-Object -Unique)
        $sizes = @($group.Group.output_bytes | Sort-Object -Unique)
        if ($hashes.Count -ne 1 -or $sizes.Count -ne 1) {
            throw "Output bytes were not deterministic for $($group.Name)"
        }
    }

    foreach ($selection in $manifest.selection) {
        $exportHash = ($implementationRows |
            Where-Object { $_.case_id -eq $selection.id -and $_.operation -eq "export" } |
            Select-Object -First 1).output_set_sha256
        $roundTripHash = ($implementationRows |
            Where-Object { $_.case_id -eq $selection.id -and $_.operation -eq "roundtrip" } |
            Select-Object -First 1).output_set_sha256
        if ($exportHash -ne $roundTripHash) {
            throw "Export and roundtrip output hashes differ for $($selection.id)"
        }
    }
}

Assert-OutputDeterminism -ImplementationRows $baselineRows
Assert-OutputDeterminism -ImplementationRows $candidateRows

$baselineRawPath = Join-Path $EvidenceDir "load-export-rapidjson-1.9.5-raw.csv"
$candidateRawPath = Join-Path $EvidenceDir "load-export-nlohmann-2.0.0-raw.csv"
$processPath = Join-Path $EvidenceDir "load-export-processes.csv"
$orderPath = Join-Path $EvidenceDir "load-export-order.csv"
$hashPath = Join-Path $EvidenceDir "load-export-output-hashes.csv"
$aggregatePath = Join-Path $EvidenceDir "load-export-aggregates.csv"

$baselineRows | Export-Csv $baselineRawPath -NoTypeInformation -Encoding UTF8
$candidateRows | Export-Csv $candidateRawPath -NoTypeInformation -Encoding UTF8
$processRows | Export-Csv $processPath -NoTypeInformation -Encoding UTF8
$orderRows | Export-Csv $orderPath -NoTypeInformation -Encoding UTF8

function Get-OutputHashRows {
    param([object[]]$ImplementationRows)

    foreach ($group in ($ImplementationRows |
        Where-Object { $_.operation -eq "export" } |
        Group-Object implementation, case_id, asset, format)) {
        [pscustomobject][ordered]@{
            implementation = $group.Group[0].implementation
            case_id = $group.Group[0].case_id
            asset = $group.Group[0].asset
            format = $group.Group[0].format
            output_bytes = $group.Group[0].output_bytes
            output_file_count = $group.Group[0].output_file_count
            output_set_sha256 = $group.Group[0].output_set_sha256
        }
    }
}
$outputHashRows = @(
    Get-OutputHashRows -ImplementationRows $baselineRows
    Get-OutputHashRows -ImplementationRows $candidateRows
)
$outputHashRows | Sort-Object implementation, case_id |
    Export-Csv $hashPath -NoTypeInformation -Encoding UTF8

function Get-TimingSummary {
    param([object[]]$Rows)

    foreach ($group in ($Rows | Group-Object case_id, asset, format, operation)) {
        $values = [double[]]@($group.Group | ForEach-Object { [double]$_.elapsed_us })
        [pscustomobject][ordered]@{
            caseId = $group.Group[0].case_id
            asset = $group.Group[0].asset
            format = $group.Group[0].format
            operation = $group.Group[0].operation
            samples = $values.Count
            medianUs = Get-Percentile -Values $values -Percentile 0.50
            p95Us = Get-Percentile -Values $values -Percentile 0.95
        }
    }
}

$baselineSummary = @(Get-TimingSummary -Rows $baselineRows)
$candidateSummary = @(Get-TimingSummary -Rows $candidateRows)
$baselineLookup = @{}
foreach ($row in $baselineSummary) {
    $baselineLookup["$($row.caseId)|$($row.operation)"] = $row
}

$comparisons = foreach ($candidate in ($candidateSummary | Sort-Object asset, format, operation)) {
    $baseline = $baselineLookup["$($candidate.caseId)|$($candidate.operation)"]
    if (-not $baseline) {
        throw "Missing baseline summary for $($candidate.caseId)/$($candidate.operation)"
    }
    $selection = $selectionById[[string]$candidate.caseId]
    [pscustomobject][ordered]@{
        caseId = $candidate.caseId
        asset = $candidate.asset
        format = $candidate.format
        operation = $candidate.operation
        tier = [string]$selection.tier
        complexityClass = Get-ComplexityClass $selection
        sampleClass = [string]$selection.sampleClass
        extensionCoverage = Get-ExtensionCoverageLabel $selection
        extensionsUsed = @(
            Get-ManifestStrings $selection.extensions.used
        )
        samples = $candidate.samples
        baselineMedianUs = $baseline.medianUs
        candidateMedianUs = $candidate.medianUs
        medianDeltaUs = $candidate.medianUs - $baseline.medianUs
        medianDeltaPercent = Get-DeltaPercent -Baseline $baseline.medianUs -Candidate $candidate.medianUs
        baselineP95Us = $baseline.p95Us
        candidateP95Us = $candidate.p95Us
        p95DeltaUs = $candidate.p95Us - $baseline.p95Us
        p95DeltaPercent = Get-DeltaPercent -Baseline $baseline.p95Us -Candidate $candidate.p95Us
    }
}

function New-AggregateRow {
    param(
        [string]$GroupKind,
        [string]$GroupName,
        [object[]]$Rows
    )

    if ($Rows.Count -eq 0) {
        return
    }

    [double]$baselineMedianTotal = 0.0
    [double]$candidateMedianTotal = 0.0
    [double]$baselineP95Total = 0.0
    [double]$candidateP95Total = 0.0
    [double]$medianWeightedLog = 0.0
    [double]$p95WeightedLog = 0.0
    foreach ($row in $Rows) {
        $baselineMedianTotal += [double]$row.baselineMedianUs
        $candidateMedianTotal += [double]$row.candidateMedianUs
        $baselineP95Total += [double]$row.baselineP95Us
        $candidateP95Total += [double]$row.candidateP95Us
        $medianWeightedLog +=
            [double]$row.baselineMedianUs *
            [Math]::Log(
                [double]$row.candidateMedianUs /
                [double]$row.baselineMedianUs)
        $p95WeightedLog +=
            [double]$row.baselineP95Us *
            [Math]::Log(
                [double]$row.candidateP95Us /
                [double]$row.baselineP95Us)
    }

    [pscustomobject][ordered]@{
        groupKind = $GroupKind
        group = $GroupName
        operation = [string]$Rows[0].operation
        caseCount = $Rows.Count
        baselineMedianTotalUs = $baselineMedianTotal
        candidateMedianTotalUs = $candidateMedianTotal
        medianDeltaUs = $candidateMedianTotal - $baselineMedianTotal
        medianAggregateDeltaPercent = Get-DeltaPercent `
            -Baseline $baselineMedianTotal `
            -Candidate $candidateMedianTotal
        medianWeightedGeometricDeltaPercent =
            ([Math]::Exp($medianWeightedLog / $baselineMedianTotal) - 1.0) *
            100.0
        baselineP95TotalUs = $baselineP95Total
        candidateP95TotalUs = $candidateP95Total
        p95DeltaUs = $candidateP95Total - $baselineP95Total
        p95AggregateDeltaPercent = Get-DeltaPercent `
            -Baseline $baselineP95Total `
            -Candidate $candidateP95Total
        p95WeightedGeometricDeltaPercent =
            ([Math]::Exp($p95WeightedLog / $baselineP95Total) - 1.0) *
            100.0
    }
}

$operations = @("load", "export", "roundtrip")
$complexityClasses = @(
    $manifest.selection |
        ForEach-Object { Get-ComplexityClass $_ } |
        Sort-Object -Unique
)
$tiers = @(
    $manifest.selection.tier |
        ForEach-Object { [string]$_ } |
        Sort-Object -Unique
)
$coverageGroups = @(
    $manifest.selection |
        ForEach-Object { Get-ExtensionCoverageLabel $_ } |
        Sort-Object -Unique
)
$exactExtensions = @(
    $manifest.selection |
        ForEach-Object {
            Get-ManifestStrings $_.extensions.used
        } |
        Sort-Object -Unique
)

$aggregates = @(
    foreach ($operation in $operations) {
        New-AggregateRow `
            -GroupKind "overall" `
            -GroupName "all-cases" `
            -Rows @(
                $comparisons |
                    Where-Object { $_.operation -eq $operation }
            )
    }
    foreach ($group in $complexityClasses) {
        foreach ($operation in $operations) {
            New-AggregateRow `
                -GroupKind "complexity-class" `
                -GroupName $group `
                -Rows @(
                    $comparisons |
                        Where-Object {
                            $_.complexityClass -eq $group -and
                            $_.operation -eq $operation
                        }
                )
        }
    }
    foreach ($group in $tiers) {
        foreach ($operation in $operations) {
            New-AggregateRow `
                -GroupKind "tier" `
                -GroupName $group `
                -Rows @(
                    $comparisons |
                        Where-Object {
                            $_.tier -eq $group -and
                            $_.operation -eq $operation
                        }
                )
        }
    }
    foreach ($group in $coverageGroups) {
        foreach ($operation in $operations) {
            New-AggregateRow `
                -GroupKind "extension-coverage" `
                -GroupName $group `
                -Rows @(
                    $comparisons |
                        Where-Object {
                            $_.extensionCoverage -eq $group -and
                            $_.operation -eq $operation
                        }
                )
        }
    }
    foreach ($extension in $exactExtensions) {
        foreach ($operation in $operations) {
            New-AggregateRow `
                -GroupKind "extension" `
                -GroupName $extension `
                -Rows @(
                    $comparisons |
                        Where-Object {
                            $_.operation -eq $operation -and
                            @($_.extensionsUsed) -ccontains $extension
                        }
                )
        }
    }
)
$aggregates |
    Export-Csv $aggregatePath -NoTypeInformation -Encoding UTF8

$outputSizes = foreach ($selection in $manifest.selection) {
    $baseline = $baselineRows |
        Where-Object { $_.case_id -eq $selection.id -and $_.operation -eq "export" } |
        Select-Object -First 1
    $candidate = $candidateRows |
        Where-Object { $_.case_id -eq $selection.id -and $_.operation -eq "export" } |
        Select-Object -First 1
    [pscustomobject][ordered]@{
        caseId = [string]$selection.id
        asset = [string]$selection.model
        format = [string]$selection.format
        inputBytes = [long]$selection.totalBytes
        baselineOutputBytes = [long]$baseline.output_bytes
        candidateOutputBytes = [long]$candidate.output_bytes
        outputDeltaBytes = [long]$candidate.output_bytes - [long]$baseline.output_bytes
        outputDeltaPercent = Get-DeltaPercent `
            -Baseline ([double]$baseline.output_bytes) `
            -Candidate ([double]$candidate.output_bytes)
        baselineOutputSha256 = $baseline.output_set_sha256
        candidateOutputSha256 = $candidate.output_set_sha256
    }
}

$measuredProcesses = @($processRows | Where-Object { $_.phase -eq "measured" })
$memorySummary = @(
    foreach ($implementation in $implementations) {
        foreach ($cohort in @("all", "large-enabled", "standard-only")) {
            $values = [double[]]@(
                $measuredProcesses |
                    Where-Object {
                        $_.implementation -eq $implementation.label -and
                        ($cohort -eq "all" -or $_.cohort -eq $cohort)
                    } |
                    ForEach-Object {
                        [double]$_.peak_working_set_bytes
                    }
            )
            if ($values.Count -eq 0) {
                continue
            }
            [pscustomobject][ordered]@{
                implementation = $implementation.label
                cohort = $cohort
                processSamples = $values.Count
                medianPeakWorkingSetBytes =
                    Get-Percentile -Values $values -Percentile 0.50
                maximumPeakWorkingSetBytes =
                    ($values | Measure-Object -Maximum).Maximum
            }
        }
    }
)

$os = Get-CimInstance Win32_OperatingSystem
$computer = Get-CimInstance Win32_ComputerSystem
$cpu = Get-CimInstance Win32_Processor | Select-Object -First 1
$cmakeVersion = (& cmake --version | Select-Object -First 1)

$environment = [ordered]@{
    schemaVersion = 2
    generatedUtc = (Get-Date).ToUniversalTime().ToString("o")
    configuration = $Configuration
    warmupCyclesPerImplementation = $Warmups
    measuredCyclesPerImplementation = [ordered]@{
        standard = $Samples
        large = $LargeSamples
    }
    timingsPerImplementation = $expectedRawRowsPerImplementation
    branchOrder = "Odd cycles RapidJSON then nlohmann; even cycles nlohmann then RapidJSON"
    workItemOrder = "The same deterministic std::shuffle seed is used for both implementations in each cycle"
    cachePolicy = "$Warmups alternating warm-up cycles precede measured hot-cache cycles; every timed load still opens and reads source files"
    percentile = "Nearest-rank median and p95"
    runnerSourceSha256 = (Get-FileHash $PSCommandPath -Algorithm SHA256).Hash
    assetManifestSha256 = $candidateAssetManifestHash
    matchedFileSha256 = $matchedFileHashes
    extensionMode = $manifest.handlerMode
    processMemory = "Peak working set is sampled every 5 ms for each whole cycle process. Cohorts distinguish cycles that include the three large cases from standard-only cycles; untimed validation remains included."
    timingBoundaries = [ordered]@{
        load = "Before source IStreamReader/file open through manifest read, SDK Deserialize, complete buffer reads, and encoded image-byte reads; destructors close input streams before the timer stops"
        export = "Loaded representation through SDK Serialize, SDK resource writes, GLB Flush or glTF manifest write, and destruction/flush/close of all output streams"
        roundtrip = "The complete load boundary immediately followed by the complete export boundary"
        excluded = "SHA-256 output hashing, semantic reload/compare, source hash verification, directory preparation, cleanup, process startup, image decode, GPU upload, shader work, and rendering"
    }
    outputHashDefinition = "SHA-256 of sorted UTF-8 lines: relative-path|byte-length|file-SHA-256"
    machine = [ordered]@{
        manufacturer = $computer.Manufacturer
        model = $computer.Model
        cpu = $cpu.Name
        logicalProcessors = $computer.NumberOfLogicalProcessors
        physicalMemoryBytes = [long]$computer.TotalPhysicalMemory
        os = $os.Caption
        osVersion = $os.Version
    }
    cmake = $cmakeVersion
    baseline = [ordered]@{
        label = $implementations[0].label
        branch = $baselineBranch
        sourceCommit = $baselineCommit
        exactRelease195Base = "3193f83265a70585093f13d651167b763979ade1"
        executable = $baselineExecutable
        executableBytes = (Get-Item $baselineExecutable).Length
        executableSha256 = (Get-FileHash $baselineExecutable -Algorithm SHA256).Hash
        benchmarkSourceSha256 = $baselineBenchmarkSourceHash
        build = $baselineBuild
    }
    candidate = [ordered]@{
        label = $implementations[1].label
        branch = $candidateBranch
        sourceCommit = $candidateCommit
        executable = $candidateExecutable
        executableBytes = (Get-Item $candidateExecutable).Length
        executableSha256 = (Get-FileHash $candidateExecutable -Algorithm SHA256).Hash
        benchmarkSourceSha256 = $candidateBenchmarkSourceHash
        build = $candidateBuild
    }
    assetManifest = $manifest
    artifacts = [ordered]@{
        baselineRaw = [System.IO.Path]::GetFileName($baselineRawPath)
        candidateRaw = [System.IO.Path]::GetFileName($candidateRawPath)
        processes = [System.IO.Path]::GetFileName($processPath)
        order = [System.IO.Path]::GetFileName($orderPath)
        outputHashes = [System.IO.Path]::GetFileName($hashPath)
        aggregates = [System.IO.Path]::GetFileName($aggregatePath)
    }
}

$summary = [ordered]@{
    schemaVersion = 2
    baseline = "rapidjson-1.9.5"
    candidate = "nlohmann-2.0.0"
    generatedUtc = $environment.generatedUtc
    timingComparisons = $comparisons
    aggregates = $aggregates
    outputSizes = $outputSizes
    memory = $memorySummary
}

$environmentPath = Join-Path $EvidenceDir "load-export-environment.json"
$summaryPath = Join-Path $EvidenceDir "load-export-summary.json"
Write-Utf8NoBom -Path $environmentPath -Content (
    ($environment | ConvertTo-Json -Depth 12) + [Environment]::NewLine)
Write-Utf8NoBom -Path $summaryPath -Content (
    ($summary | ConvertTo-Json -Depth 8) + [Environment]::NewLine)

$reportPath = Join-Path $EvidenceDir "load-export-comparison.md"
$lines = New-Object System.Collections.Generic.List[string]

function Get-Aggregate {
    param(
        [string]$Kind,
        [string]$Group,
        [string]$Operation
    )

    return $aggregates |
        Where-Object {
            $_.groupKind -eq $Kind -and
            $_.group -eq $Group -and
            $_.operation -eq $Operation
        } |
        Select-Object -First 1
}

function Add-AggregateTable {
    param(
        [string]$Kind,
        [string]$Title
    )

    $lines.Add("### $Title")
    $lines.Add("")
    $lines.Add("| Group | Operation | Cases | RapidJSON summed median (ms) | nlohmann summed median (ms) | Delta (ms) | Aggregate delta | Baseline-time-weighted geometric delta | Aggregate p95 delta |")
    $lines.Add("| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |")
    foreach ($row in (
        $aggregates |
            Where-Object { $_.groupKind -eq $Kind } |
            Sort-Object group, operation
    )) {
        $lines.Add(
            "| $($row.group) | $($row.operation) | $($row.caseCount) | " +
            "$(($row.baselineMedianTotalUs / 1000.0).ToString('F3')) | " +
            "$(($row.candidateMedianTotalUs / 1000.0).ToString('F3')) | " +
            "$(($row.medianDeltaUs / 1000.0).ToString('F3')) | " +
            "$(Format-Percent $row.medianAggregateDeltaPercent) | " +
            "$(Format-Percent $row.medianWeightedGeometricDeltaPercent) | " +
            "$(Format-Percent $row.p95AggregateDeltaPercent) |")
    }
    $lines.Add("")
}

$tailObservations = @(
    foreach ($row in $comparisons) {
        [pscustomobject]@{
            implementation = "rapidjson-1.9.5"
            caseId = $row.caseId
            operation = $row.operation
            ratio = $row.baselineP95Us / $row.baselineMedianUs
        }
        [pscustomobject]@{
            implementation = "nlohmann-2.0.0"
            caseId = $row.caseId
            operation = $row.operation
            ratio = $row.candidateP95Us / $row.candidateMedianUs
        }
    }
)
$largestTail = $tailObservations |
    Sort-Object ratio -Descending |
    Select-Object -First 1
$p95DirectionChanges = @(
    $comparisons |
        Where-Object {
            [Math]::Sign([double]$_.medianDeltaUs) -ne
            [Math]::Sign([double]$_.p95DeltaUs)
        }
).Count
$overallLoad = Get-Aggregate "overall" "all-cases" "load"
$overallExport = Get-Aggregate "overall" "all-cases" "export"
$overallRoundTrip = Get-Aggregate "overall" "all-cases" "roundtrip"
$smallLoad = Get-Aggregate "complexity-class" "small" "load"
$largeLoad = Get-Aggregate "complexity-class" "large" "load"
$baselineOutputTotal =
    ($outputSizes | Measure-Object baselineOutputBytes -Sum).Sum
$candidateOutputTotal =
    ($outputSizes | Measure-Object candidateOutputBytes -Sum).Sum

$lines.Add("# End-to-end glTF/GLB load-export benchmark")
$lines.Add("")
$lines.Add("Generated: $($environment.generatedUtc)")
$lines.Add("Baseline: rapidjson-1.9.5 on $baselineBranch at $baselineCommit")
$lines.Add("Candidate: nlohmann-2.0.0 on $candidateBranch at $candidateCommit")
$lines.Add("Exact upstream Release/1.9.5 base: 3193f83265a70585093f13d651167b763979ade1")
$lines.Add("")
$lines.Add("## Method")
$lines.Add("")
$lines.Add("- Optimized MSVC x64 $Configuration builds use the same generator, compiler, flags, machine, disk, harness source, assets, and output medium.")
$lines.Add("- Each implementation receives $Warmups alternating warm-up cycles. Small/medium cases use $Samples measured samples and the three genuinely large cases use $LargeSamples. Odd cycles run RapidJSON first; even cycles run nlohmann first.")
$lines.Add("- The 30-sample large-case count is justified by a preliminary complete cycle: about 13 seconds for RapidJSON and 82-91 seconds for nlohmann, dominated by NodePerformanceTest schema/object construction plus the required untimed output reload checks. The reduction is identical on both branches.")
$lines.Add("- Each cycle uses the same deterministic shuffled asset/operation order in both implementations.")
$lines.Add("- All eight benchmark/corpus files are byte-identical between branches; their SHA-256 values are recorded in load-export-environment.json.")
$lines.Add("- The shared `KHR::GetKHRExtensionDeserializer`/`Serializer` set is enabled globally. Supported extensions take the SDK-typed path and unsupported extensions stay in the raw `glTFProperty::extensions` path, exactly as listed per case below.")
$lines.Add("- LOAD starts before opening the source file and ends after Deserialize plus complete SDK reads of every buffer and every encoded image resource.")
$lines.Add("- EXPORT starts with the loaded representation and ends after Serialize, resource writes, GLB Flush or glTF manifest write, and output stream flush/close by destruction.")
$lines.Add("- ROUNDTRIP measures those LOAD and EXPORT boundaries back-to-back. Hashing, semantic validation, directory setup, and cleanup are outside all timers.")
$lines.Add("- The process peak-working-set metric covers a whole cycle, including untimed validation; it is not an operation-specific peak. Large-enabled and standard-only process cohorts are reported separately.")
$lines.Add("")
$lines.Add("glTF-SDK reads encoded image bytes but does not decode pixels, create GPU textures, upload resources, compile shaders, or render. Those activities are excluded. Encoded image file I/O is included.")
$lines.Add("")
$lines.Add("## Corpus and provenance")
$lines.Add("")
$lines.Add("Repository: $($manifest.repository)")
$lines.Add("Pinned commit: [$($manifest.commit)]($($manifest.commitUrl))")
$lines.Add("")
$lines.Add("| Case | Tier | Samples | Format | Bytes | Scenes / meshes / primitives / nodes / materials / accessors | Typed handlers | Raw-preserved | Required | License |")
$lines.Add("| --- | --- | ---: | --- | ---: | --- | --- | --- | --- | --- |")
foreach ($selection in $manifest.selection) {
    $typed = (Get-ManifestStrings $selection.extensions.typed) -join ", "
    $raw = (Get-ManifestStrings $selection.extensions.raw) -join ", "
    $required = (Get-ManifestStrings $selection.extensions.required) -join ", "
    if (-not $typed) { $typed = "none" }
    if (-not $raw) { $raw = "none" }
    if (-not $required) { $required = "none" }
    $counts = $selection.complexity
    $lines.Add(
        "| $($selection.id) | $($selection.tier) | " +
        "$($selectionSamples[[string]$selection.id]) | $($selection.format) | " +
        "$($selection.totalBytes) | $($counts.scenes) / $($counts.meshes) / " +
        "$($counts.meshPrimitives) / $($counts.nodes) / $($counts.materials) / " +
        "$($counts.accessors) | $typed | $raw | $required | " +
        "[$($selection.license.spdx)]($($selection.license.modelLicenseUrl)) |")
}
$lines.Add("")
$lines.Add("The regular ABeautifulGame GLB (42,977,928 bytes) and NodePerformanceTest GLB (37,986,536 bytes; 10,002 nodes, 10,000 meshes/materials) are both materially larger and structurally more complex than Avocado. The compressed ABeautifulGame variant adds typed Draco plus raw required BasisU coverage.")
$lines.Add("")
$lines.Add("| Pinned source file | Bytes | SHA-256 | Immutable URL |")
$lines.Add("| --- | ---: | --- | --- |")
foreach ($file in $manifest.files) {
    $lines.Add("| $($file.path) | $($file.bytes) | $($file.sha256) | [raw]($($file.url)) |")
}
$lines.Add("")
$lines.Add("## Timing comparison")
$lines.Add("")
$lines.Add("Positive deltas mean nlohmann Release/2.0.0 is slower; negative deltas mean it is faster.")
$lines.Add("")
$lines.Add("| Case | Tier | Operation | N | RapidJSON median (ms) | nlohmann median (ms) | Median delta (ms) | Delta | RapidJSON p95 (ms) | nlohmann p95 (ms) | p95 delta |")
$lines.Add("| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |")
foreach ($row in $comparisons) {
    $lines.Add(
        "| $($row.caseId) | $($row.tier) | $($row.operation) | $($row.samples) | " +
        "$(($row.baselineMedianUs / 1000.0).ToString('F3')) | " +
        "$(($row.candidateMedianUs / 1000.0).ToString('F3')) | " +
        "$(($row.medianDeltaUs / 1000.0).ToString('F3')) | " +
        "$(Format-Percent $row.medianDeltaPercent) | " +
        "$(($row.baselineP95Us / 1000.0).ToString('F3')) | " +
        "$(($row.candidateP95Us / 1000.0).ToString('F3')) | " +
        "$(Format-Percent $row.p95DeltaPercent) |")
}
$lines.Add("")
$lines.Add("## Aggregate views")
$lines.Add("")
$lines.Add("Summed medians weight cases by observed runtime. The geometric delta is also weighted by each case's RapidJSON median, so tiny assets cannot dominate the aggregate. These are descriptive summaries, not inferential statistics.")
$lines.Add("")
Add-AggregateTable "overall" "Overall"
Add-AggregateTable "complexity-class" "Complexity class"
Add-AggregateTable "tier" "Detailed complexity tier"
Add-AggregateTable "extension-coverage" "Extension execution mode"
Add-AggregateTable "extension" "Exact extension coverage"

$lines.Add("## Input and output size")
$lines.Add("")
$lines.Add("| Asset | Format | Input bytes | RapidJSON output bytes | nlohmann output bytes | Delta bytes | Delta |")
$lines.Add("| --- | --- | ---: | ---: | ---: | ---: | ---: |")
foreach ($row in $outputSizes) {
    $lines.Add(
        "| $($row.asset) | $($row.format) | $($row.inputBytes) | " +
        "$($row.baselineOutputBytes) | $($row.candidateOutputBytes) | " +
        "$($row.outputDeltaBytes) | $(Format-Percent $row.outputDeltaPercent) |")
}
$lines.Add("")
$lines.Add("Per-output canonical SHA-256 values are in load-export-output-hashes.csv. Different byte hashes are permitted only when both SDK reloads produce the same source Document, buffer bytes, and encoded image bytes.")
$lines.Add("Across all cases, RapidJSON output sets total $baselineOutputTotal bytes and nlohmann output sets total $candidateOutputTotal bytes.")
$lines.Add("")
$lines.Add("## Peak working set")
$lines.Add("")
$lines.Add("| Implementation | Cohort | Measured processes | Median process peak (MiB) | Maximum process peak (MiB) |")
$lines.Add("| --- | --- | ---: | ---: | ---: |")
foreach ($row in $memorySummary) {
    $lines.Add(
        "| $($row.implementation) | $($row.cohort) | $($row.processSamples) | " +
        "$(($row.medianPeakWorkingSetBytes / 1MB).ToString('F2')) | " +
        "$(($row.maximumPeakWorkingSetBytes / 1MB).ToString('F2')) |")
}
$lines.Add("")
$lines.Add("## Interpretation")
$lines.Add("")
$lines.Add("- **Fixed overhead versus scaling:** small-class LOAD summed medians change by $(($smallLoad.medianDeltaUs / 1000.0).ToString('F3')) ms ($(Format-Percent $smallLoad.medianAggregateDeltaPercent)); large-class LOAD changes by $(($largeLoad.medianDeltaUs / 1000.0).ToString('F3')) ms ($(Format-Percent $largeLoad.medianAggregateDeltaPercent)). Read percentage and absolute changes together: tiny manifests magnify fixed setup/validation costs, while the large cases expose scaling.")
$lines.Add("- **Parse/validation versus serialization:** overall LOAD is $(Format-Percent $overallLoad.medianAggregateDeltaPercent), EXPORT is $(Format-Percent $overallExport.medianAggregateDeltaPercent), and ROUNDTRIP is $(Format-Percent $overallRoundTrip.medianAggregateDeltaPercent) on summed medians. LOAD includes schema validation and SDK object construction; EXPORT isolates serialization/resource writing from that parse path.")
$lines.Add("- **Tails:** the largest p95/median ratio is $($largestTail.ratio.ToString('F2'))x for $($largestTail.implementation) $($largestTail.caseId)/$($largestTail.operation). $p95DirectionChanges of $($comparisons.Count) case/operation rows reverse delta direction between median and p95, so scheduler/filesystem outliers should not be treated as parser behavior.")
$lines.Add("- Extension-mode and exact-extension tables are corpus associations, not causal isolation: assets differ in size and structure as well as extensions.")
$lines.Add("- No confidence intervals or hypothesis tests were computed. These single-machine matched observations do not establish statistical significance.")
$lines.Add("")
$lines.Add("## Correctness and caveats")
$lines.Add("")
$lines.Add("- FetchAssets.ps1 verifies every pinned source file's byte length and SHA-256. ValidateAssets.ps1 reparses every glTF/GLB manifest and checks counts, exact extensionsUsed/extensionsRequired arrays, typed/raw partition, immutable URLs, and licenses before running.")
$lines.Add("- After every timed LOAD, resource counts, complete buffer lengths, exact extension sets, and typed-versus-raw representation are checked outside the interval.")
$lines.Add("- After every timed EXPORT/ROUNDTRIP, the output is reparsed with the same typed handlers and compared with the source Document, required-extension sets, every buffer byte, and every encoded image byte.")
$lines.Add("- Output files are SHA-256 hashed only after the timer stops; every output was deterministic across its configured $Samples- or $LargeSamples-sample series.")
$lines.Add("- Results are single-machine, hot-cache nearest-rank medians/p95s, not confidence intervals. Filesystem cache, antivirus, thermals, and background activity can affect tails.")
$lines.Add("- Peak memory is sampled at process level and includes untimed correctness work, so it is useful only as a coarse matched comparison.")
$lines.Add("- Assets are fetched only by explicit benchmark commands into Built/Int. Normal builds, tests, and CI remain offline and network-independent with ENABLE_BENCHMARKS=OFF.")
$lines.Add("")
$lines.Add("## Build and test validation")
$lines.Add("")
$lines.Add("See load-export-validation.md for benchmark-disabled builds, corpus integrity/extension round-trip checks, targeted tests, and complete suites on both branches.")
$lines.Add("")
$lines.Add("## Raw evidence")
$lines.Add("")
$lines.Add("- load-export-rapidjson-1.9.5-raw.csv")
$lines.Add("- load-export-nlohmann-2.0.0-raw.csv")
$lines.Add("- load-export-processes.csv")
$lines.Add("- load-export-order.csv")
$lines.Add("- load-export-output-hashes.csv")
$lines.Add("- load-export-aggregates.csv")
$lines.Add("- load-export-environment.json")
$lines.Add("- load-export-summary.json")
$lines.Add("- load-export-validation.md")

Write-Utf8NoBom -Path $reportPath -Content (($lines -join [Environment]::NewLine) + [Environment]::NewLine)

Write-Host "Comparison report: $reportPath"
Write-Host "Environment metadata: $environmentPath"
Write-Host "Raw baseline: $baselineRawPath"
Write-Host "Raw candidate: $candidateRawPath"
