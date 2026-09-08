# End-to-end load/export benchmark

`GLTFSDK.LoadExportBenchmarks` is an opt-in, dependency-free benchmark for a
complete SDK round trip. It is intentionally excluded from normal builds,
unit tests, and CI unless `ENABLE_BENCHMARKS=ON` is supplied.

The matched comparison uses the identical harness source on:

- `perf/Release-1.9.5-load-export`, based exactly on upstream
  `3193f83265a70585093f13d651167b763979ade1` (RapidJSON).
- `Release/2.0.0` (nlohmann JSON).

## Workload

`assets.json` pins KhronosGroup/glTF-Sample-Assets commit
`9429648735279342b4c32b8745f7904196607379`, immutable raw URLs, byte lengths,
SHA-256 values, model licenses, structural counts, exact
`extensionsUsed`/`extensionsRequired` arrays, and the typed-versus-raw
extension mode for every case.

The original Box and Avocado glTF+GLB cases remain for continuity. The
expanded corpus adds 12 GLBs spanning:

- 43 MB ABeautifulGame and 38 MB NodePerformanceTest large tiers; the latter
  has 10,002 nodes and 10,000 meshes/materials.
- Morph/animation, material-grid, texture-heavy, metadata, instancing,
  compressed-resource, and large scene-structure tiers.
- SDK-typed `KHR_materials_clearcoat`, transmission, volume, sheen, specular,
  iridescence, unlit, `KHR_texture_transform`,
  `KHR_draco_mesh_compression`, and `EXT_mesh_gpu_instancing`.
- Raw-preserved `KHR_materials_ior`, `KHR_lights_punctual`,
  `KHR_texture_basisu`, `KHR_materials_emissive_strength`,
  `KHR_materials_variants`, and `KHR_xmp_json_ld`.

The same `KHR::GetKHRExtensionDeserializer` and serializer are enabled on both
branches. Unsupported extensions stay in the SDK raw extension map.

No model binaries are committed. Fetch and verify them explicitly:

```powershell
powershell -ExecutionPolicy Bypass -File .\Benchmarks\LoadExport\FetchAssets.ps1
```

`FetchAssets.ps1` downloads only on explicit invocation, caches under
`Built\Int`, checks every byte length and SHA-256, then reparses each manifest
with `ValidateAssets.ps1` to verify counts and exact extension metadata. An
offline integrity-only check is available with `-VerifyOnly`; normal
builds/tests/CI never fetch this corpus.

## Timing boundaries

- **LOAD:** starts before the source file is opened. It includes reading the
  glTF manifest or GLB container, `Deserialize` with normal schema validation,
  complete reads of every `Buffer`, and reads of every SDK-addressable encoded
  `Image`. It ends after reader/input-stream destruction closes files.
- **EXPORT:** starts with the loaded `Document` and retained resource bytes. It
  includes `Serialize`, all SDK `ResourceWriter` writes, GLB `Flush` or the
  glTF manifest write, and writer/output-stream destruction so files are
  flushed and closed.
- **ROUNDTRIP:** LOAD immediately followed by EXPORT in one measured interval.

Directory preparation, source hash checks, semantic reload/compare, output
SHA-256 calculation, and cleanup occur outside the timed intervals.

glTF-SDK reads encoded PNG/JPEG bytes; it does not decode images, create or
upload GPU textures, compile shaders, or render. Those activities are not part
of this benchmark.

## Build

Use the same MSVC generator/toolset, x64 architecture, and Release
configuration in both worktrees:

```powershell
cmake -S E:\Base3D\glTF-SDK-perf-1.9.5 -B E:\Base3D\glTF-SDK-perf-1.9.5\Built\Int\load-export -A x64 -DCMAKE_FIND_USE_PACKAGE_REGISTRY=FALSE -DCMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY=FALSE -DENABLE_UNIT_TESTS=ON -DENABLE_SAMPLES=OFF -DENABLE_BENCHMARKS=ON
cmake --build E:\Base3D\glTF-SDK-perf-1.9.5\Built\Int\load-export --config Release --target GLTFSDK.LoadExportBenchmarks GLTFSDK.Test --parallel

cmake -S E:\Base3D\glTF-SDK -B E:\Base3D\glTF-SDK\Built\Int\load-export -A x64 -DCMAKE_FIND_USE_PACKAGE_REGISTRY=FALSE -DCMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY=FALSE -DENABLE_UNIT_TESTS=ON -DENABLE_SAMPLES=OFF -DENABLE_BENCHMARKS=ON
cmake --build E:\Base3D\glTF-SDK\Built\Int\load-export --config Release --target GLTFSDK.LoadExportBenchmarks GLTFSDK.Test --parallel
```

Validate corpus integrity and one complete untimed load/export/reload pass,
including typed/raw extension representation and resource bytes:

```powershell
powershell -ExecutionPolicy Bypass -File E:\Base3D\glTF-SDK-perf-1.9.5\Benchmarks\LoadExport\TestCorpus.ps1 -BuildDir E:\Base3D\glTF-SDK-perf-1.9.5\Built\Int\load-export
powershell -ExecutionPolicy Bypass -File E:\Base3D\glTF-SDK\Benchmarks\LoadExport\TestCorpus.ps1 -BuildDir E:\Base3D\glTF-SDK\Built\Int\load-export
```

## Matched run

The runner performs at least five warm-up cycles, 100 measured samples for
small/medium cases, and 30 for the three genuinely large cases. It alternates
implementation order (AB, BA, AB, ...) and gives both executables the same
deterministic shuffled work order in each cycle. All output goes to disk.

```powershell
powershell -ExecutionPolicy Bypass -File .\Benchmarks\LoadExport\RunMatchedBenchmarks.ps1 `
  -BaselineBuildDir E:\Base3D\glTF-SDK-perf-1.9.5\Built\Int\load-export `
  -CandidateBuildDir E:\Base3D\glTF-SDK\Built\Int\load-export `
  -Warmups 5 -Samples 100 -LargeSamples 30
```

The script rejects toolchain/configuration or matched-file mismatches and
writes raw CSV, environment JSON, canonical output hashes, per-tier and
per-extension aggregates, summary JSON, and a Markdown comparison under
`Built/BenchmarkResults/LoadExport/`.

Each timed output is reloaded outside the timer. The reloaded `Document`, every
buffer byte, and every encoded image byte must match the source. Peak working
set is sampled for the whole benchmark process and therefore includes untimed
validation; it is a coarse matched metric, not a per-operation memory peak.
