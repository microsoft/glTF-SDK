param(
    [Parameter(Mandatory = $true)]
    [string]$AssetRoot,

    [string]$ManifestPath = (Join-Path $PSScriptRoot "assets.json")
)

$ErrorActionPreference = "Stop"

function Assert-Condition {
    param(
        [bool]$Condition,
        [string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

function Get-StringArray {
    param($Value)

    if ($null -eq $Value) {
        return @()
    }
    return @($Value | ForEach-Object { [string]$_ })
}

function Assert-StringSequence {
    param(
        [string[]]$Actual,
        [string[]]$Expected,
        [string]$Description
    )

    Assert-Condition ($Actual.Count -eq $Expected.Count) `
        "$Description count is $($Actual.Count); expected $($Expected.Count)"
    for ($index = 0; $index -lt $Expected.Count; ++$index) {
        Assert-Condition ($Actual[$index] -ceq $Expected[$index]) `
            "$Description differs at index $index"
    }
}

function Read-AssetJson {
    param(
        [string]$Path,
        [string]$Format
    )

    if ($Format -eq "gltf") {
        return Get-Content $Path -Raw | ConvertFrom-Json
    }
    if ($Format -ne "glb") {
        throw "Unsupported asset format '$Format'"
    }

    $stream = [System.IO.File]::OpenRead($Path)
    $reader = New-Object System.IO.BinaryReader($stream)
    try {
        Assert-Condition ($stream.Length -ge 20) "GLB is shorter than its header"
        Assert-Condition ($reader.ReadUInt32() -eq 0x46546C67) "Invalid GLB magic"
        Assert-Condition ($reader.ReadUInt32() -eq 2) "Unsupported GLB version"
        Assert-Condition ($reader.ReadUInt32() -eq $stream.Length) `
            "GLB declared length differs from file length"

        [uint32]$jsonLength = $reader.ReadUInt32()
        Assert-Condition ($reader.ReadUInt32() -eq 0x4E4F534A) `
            "The first GLB chunk is not JSON"
        Assert-Condition ($jsonLength -le ($stream.Length - $stream.Position)) `
            "GLB JSON chunk exceeds the file"

        $jsonBytes = $reader.ReadBytes($jsonLength)
        Assert-Condition ($jsonBytes.Length -eq $jsonLength) `
            "Unable to read the complete GLB JSON chunk"
        $json = [System.Text.Encoding]::UTF8.GetString($jsonBytes)
        $json = $json.TrimEnd([char[]]@(0, 32))
        return $json | ConvertFrom-Json
    }
    finally {
        $reader.Dispose()
        $stream.Dispose()
    }
}

function Get-ArrayCount {
    param($Value)

    if ($null -eq $Value) {
        return 0
    }
    return @($Value).Count
}

$ManifestPath = [System.IO.Path]::GetFullPath($ManifestPath)
$AssetRoot = [System.IO.Path]::GetFullPath($AssetRoot)
$manifest = Get-Content $ManifestPath -Raw | ConvertFrom-Json

Assert-Condition ($manifest.schemaVersion -eq 2) `
    "Unsupported corpus manifest schema version"
Assert-Condition ([string]$manifest.commit -match "^[0-9a-fA-F]{40}$") `
    "The corpus commit must be a full immutable SHA"

$rawPrefix =
    "https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/" +
    "$($manifest.commit)/"
$blobPrefix =
    "https://github.com/KhronosGroup/glTF-Sample-Assets/blob/" +
    "$($manifest.commit)/"
$typedHandlers = @(
    Get-StringArray $manifest.typedHandlerExtensions |
        Sort-Object -Unique
)
Assert-Condition ($typedHandlers.Count -eq 11) `
    "The shared Release/1.9.5-lineage typed handler list is incomplete"

$filesByPath = @{}
foreach ($file in $manifest.files) {
    $relativePath = [string]$file.path
    Assert-Condition (-not $filesByPath.ContainsKey($relativePath)) `
        "Duplicate corpus file '$relativePath'"
    Assert-Condition ([string]$file.url -ceq ($rawPrefix + $relativePath)) `
        "Corpus file URL is not pinned to the declared commit: $relativePath"

    $path = Join-Path $AssetRoot ($relativePath -replace "/", "\")
    Assert-Condition (Test-Path $path -PathType Leaf) `
        "Missing corpus file '$relativePath'"
    $item = Get-Item $path
    Assert-Condition ($item.Length -eq [long]$file.bytes) `
        "Corpus file byte length differs for '$relativePath'"
    $hash = (Get-FileHash $path -Algorithm SHA256).Hash
    Assert-Condition ($hash -ceq ([string]$file.sha256).ToUpperInvariant()) `
        "Corpus file SHA-256 differs for '$relativePath'"

    $filesByPath[$relativePath] = $file
}

$selectionIds = @{}
foreach ($selection in $manifest.selection) {
    $id = [string]$selection.id
    Assert-Condition (-not $selectionIds.ContainsKey($id)) `
        "Duplicate corpus selection '$id'"
    $selectionIds[$id] = $true

    $entrypoint = [string]$selection.entrypoint
    Assert-Condition ($filesByPath.ContainsKey($entrypoint)) `
        "Selection '$id' entrypoint is absent from the file manifest"
    Assert-Condition (
        (Get-StringArray $selection.files) -ccontains $entrypoint
    ) "Selection '$id' does not include its entrypoint"
    Assert-Condition (
        [string]$selection.sourceUrl -ceq ($rawPrefix + $entrypoint)
    ) "Selection '$id' source URL is not immutable"
    Assert-Condition (
        [string]$selection.license.modelLicenseUrl -ceq
            ($blobPrefix + "Models/$($selection.model)/LICENSE.md")
    ) "Selection '$id' model license URL is not immutable"
    Assert-Condition (
        $manifest.sampleClasses.PSObject.Properties.Name -ccontains
            [string]$selection.sampleClass
    ) "Selection '$id' has an unknown sample class"

    [long]$totalBytes = 0
    foreach ($relativePath in (Get-StringArray $selection.files)) {
        Assert-Condition ($filesByPath.ContainsKey($relativePath)) `
            "Selection '$id' references unknown file '$relativePath'"
        $totalBytes += [long]$filesByPath[$relativePath].bytes
    }
    Assert-Condition ($totalBytes -eq [long]$selection.totalBytes) `
        "Selection '$id' total bytes differ from its file set"

    $entrypointPath =
        Join-Path $AssetRoot ($entrypoint -replace "/", "\")
    $entrypointFile = Get-Item $entrypointPath
    $entrypointHash =
        (Get-FileHash $entrypointPath -Algorithm SHA256).Hash
    Assert-Condition (
        $entrypointFile.Length -eq [long]$selection.entrypointBytes
    ) "Selection '$id' entrypoint byte length differs"
    Assert-Condition (
        $entrypointHash -ceq
            ([string]$selection.entrypointSha256).ToUpperInvariant()
    ) "Selection '$id' entrypoint SHA-256 differs"

    $assetJson = Read-AssetJson `
        -Path $entrypointPath `
        -Format ([string]$selection.format)
    $complexity = $selection.complexity
    foreach ($name in @(
        "scenes",
        "meshes",
        "nodes",
        "materials",
        "accessors",
        "animations",
        "skins",
        "images",
        "textures"
    )) {
        $actualCount = Get-ArrayCount $assetJson.$name
        $expectedCount = [int]$complexity.$name
        Assert-Condition ($actualCount -eq $expectedCount) `
            "Selection '$id' $name count is $actualCount; expected $expectedCount"
    }
    [int]$primitiveCount = 0
    foreach ($mesh in @($assetJson.meshes)) {
        $primitiveCount += Get-ArrayCount $mesh.primitives
    }
    Assert-Condition (
        $primitiveCount -eq [int]$complexity.meshPrimitives
    ) "Selection '$id' mesh primitive count differs"

    $used = Get-StringArray $assetJson.extensionsUsed
    $required = Get-StringArray $assetJson.extensionsRequired
    $expectedUsed = Get-StringArray $selection.extensions.used
    $expectedRequired = Get-StringArray $selection.extensions.required
    Assert-StringSequence `
        -Actual $used `
        -Expected $expectedUsed `
        -Description "Selection '$id' extensionsUsed"
    Assert-StringSequence `
        -Actual $required `
        -Expected $expectedRequired `
        -Description "Selection '$id' extensionsRequired"

    $typed = Get-StringArray $selection.extensions.typed
    $raw = Get-StringArray $selection.extensions.raw
    $declaredModes = @($typed + $raw | Sort-Object -Unique)
    $sortedUsed = @($used | Sort-Object -Unique)
    Assert-StringSequence `
        -Actual $declaredModes `
        -Expected $sortedUsed `
        -Description "Selection '$id' typed/raw extension partition"
    foreach ($extension in $typed) {
        Assert-Condition ($typedHandlers -ccontains $extension) `
            "Selection '$id' marks unsupported '$extension' as typed"
    }
    foreach ($extension in $raw) {
        Assert-Condition (-not ($typedHandlers -ccontains $extension)) `
            "Selection '$id' marks typed '$extension' as raw"
    }
    foreach ($extension in $required) {
        Assert-Condition ($used -ccontains $extension) `
            "Selection '$id' requires undeclared extension '$extension'"
    }
}

Write-Host (
    "Validated {0} corpus selections and {1} pinned files at {2}" -f
    @($manifest.selection).Count,
    @($manifest.files).Count,
    $manifest.commit)
