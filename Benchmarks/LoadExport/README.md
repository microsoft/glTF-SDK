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
SHA-256 values, model licenses, and these matched variants:

- Box glTF + GLB: small/core tier.
- Avocado glTF + GLB: texture-heavy tier with roughly 8.1 MB of encoded image
  and geometry data.

No model binaries are committed. Fetch and verify them explicitly:

```powershell
powershell -ExecutionPolicy Bypass -File .\Benchmarks\LoadExport\FetchAssets.ps1
```

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

## Matched run

The runner performs at least five warm-up cycles and 30 measured cycles per
asset/format/operation. It alternates implementation order (AB, BA, AB, ...)
and gives both executables the same deterministic shuffled work order in each
cycle. All output goes to disk.

```powershell
powershell -ExecutionPolicy Bypass -File .\Benchmarks\LoadExport\RunMatchedBenchmarks.ps1 `
  -BaselineBuildDir E:\Base3D\glTF-SDK-perf-1.9.5\Built\Int\load-export `
  -CandidateBuildDir E:\Base3D\glTF-SDK\Built\Int\load-export `
  -Warmups 5 -Samples 100
```

The script rejects toolchain/configuration mismatches and writes raw CSV,
environment JSON, canonical output hashes, summary JSON, and a Markdown
comparison under
`docs/release-evidence/replace-rapidjson-nlohmann/load-export/`.

Each timed output is reloaded outside the timer. The reloaded `Document`, every
buffer byte, and every encoded image byte must match the source. Peak working
set is sampled for the whole benchmark process and therefore includes untimed
validation; it is a coarse matched metric, not a per-operation memory peak.
