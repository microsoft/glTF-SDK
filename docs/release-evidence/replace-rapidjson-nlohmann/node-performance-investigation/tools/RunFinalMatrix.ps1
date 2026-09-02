param(
    [string]$BuildDir = "E:\Base3D\glTF-SDK-T39-profile\Built\Int\t39-profile",
    [string]$SampleAssets = "E:\Base3D\glTF-SDK\Built\Int\LoadExportAssets\9429648735279342b4c32b8745f7904196607379",
    [string]$DerivedAssets = "E:\Base3D\glTF-SDK-T39-profile\Built\Int\t39-assets",
    [string]$ResultRoot = "E:\Base3D\glTF-SDK-T39-profile\Built\Int\t39-results\final"
)

$ErrorActionPreference = "Stop"
$exe = Join-Path $BuildDir "Benchmarks\NodePerformance\Release\GLTFSDK.NodePerformanceInvestigation.exe"
if (-not (Test-Path $exe)) {
    throw "Investigation executable not found: $exe"
}

New-Item -ItemType Directory -Force -Path $ResultRoot | Out-Null
$serializationRoot = Join-Path $ResultRoot "serialized"
New-Item -ItemType Directory -Force -Path $serializationRoot | Out-Null

function Invoke-Investigation {
    param(
        [string]$Group,
        [string]$Assets,
        [string]$Case,
        [string]$Variant,
        [int]$Warmups,
        [int]$Samples,
        [switch]$CountOperations,
        [switch]$IncludeSerialization
    )

    $name = "$Group--$Case--$Variant.csv"
    $output = Join-Path $ResultRoot $name
    $arguments = @(
        "--assets", $Assets,
        "--case", $Case,
        "--variant", $Variant,
        "--output", $output,
        "--warmups", [string]$Warmups,
        "--samples", [string]$Samples
    )
    if ($CountOperations) {
        $arguments += "--count-operations"
    }
    if ($IncludeSerialization) {
        $arguments += @(
            "--include-serialization",
            "--output-root", $serializationRoot
        )
    }

    Write-Host "Running $Group $Case $Variant ($Warmups warmups, $Samples samples)"
    & $exe @arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Investigation failed for $Case / $Variant"
    }
}

$nodeVariants = @(
    @("current", 1, 7),
    @("ordered-fallback", 1, 5),
    @("plain-ordered-fallback", 2, 10),
    @("sax-ordered-fallback", 2, 10),
    @("sax-scalar-unique-fallback", 3, 15),
    @("cached-sax-scalar-unique-fallback", 3, 15),
    @("sax-disable-root", 2, 10),
    @("sax-default-fallback", 2, 7),
    @("sax-default-scalar-unique-fallback", 2, 10),
    @("sax-indexed-fallback", 2, 7),
    @("sax-indexed-scalar-unique-fallback", 2, 10)
)
foreach ($entry in $nodeVariants) {
    Invoke-Investigation -Group "variants" -Assets $SampleAssets `
        -Case "NodePerformanceTest-glb" -Variant $entry[0] `
        -Warmups $entry[1] -Samples $entry[2]
}

$controls = @(
    "Avocado-glb",
    "SpecularTest-glb",
    "ABeautifulGame-glb",
    "ABeautifulGame-draco-glb"
)
foreach ($case in $controls) {
    Invoke-Investigation -Group "controls" -Assets $SampleAssets `
        -Case $case -Variant "current" -Warmups 2 -Samples 15
    Invoke-Investigation -Group "controls" -Assets $SampleAssets `
        -Case $case -Variant "sax-scalar-unique-fallback" `
        -Warmups 2 -Samples 15
}

$scales = @(
    "NodePerformance-100",
    "NodePerformance-1000",
    "NodePerformance-5000"
)
foreach ($case in $scales) {
    Invoke-Investigation -Group "scaling" -Assets $DerivedAssets `
        -Case $case -Variant "current" -Warmups 1 -Samples 7
    Invoke-Investigation -Group "scaling" -Assets $DerivedAssets `
        -Case $case -Variant "sax-scalar-unique-fallback" `
        -Warmups 2 -Samples 10
}

Invoke-Investigation -Group "counts" -Assets $SampleAssets `
    -Case "NodePerformanceTest-glb" -Variant "current" `
    -Warmups 0 -Samples 1 -CountOperations
Invoke-Investigation -Group "counts" -Assets $SampleAssets `
    -Case "NodePerformanceTest-glb" `
    -Variant "sax-scalar-unique-fallback" `
    -Warmups 0 -Samples 1 -CountOperations

Invoke-Investigation -Group "serialization" -Assets $SampleAssets `
    -Case "NodePerformanceTest-glb" `
    -Variant "sax-scalar-unique-fallback" `
    -Warmups 2 -Samples 5 -IncludeSerialization
Invoke-Investigation -Group "serialization" -Assets $SampleAssets `
    -Case "ABeautifulGame-glb" `
    -Variant "sax-scalar-unique-fallback" `
    -Warmups 1 -Samples 5 -IncludeSerialization
Invoke-Investigation -Group "serialization" -Assets $SampleAssets `
    -Case "Avocado-glb" `
    -Variant "sax-scalar-unique-fallback" `
    -Warmups 1 -Samples 5 -IncludeSerialization
