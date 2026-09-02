# End-to-end glTF/GLB load-export benchmark

Generated: 2026-09-02T13:27:33.8036217Z
Baseline: rapidjson-1.9.5 on perf/Release-1.9.5-load-export at 2b410117c0e9e67da52afea5845610d05ad3f825
Candidate: nlohmann-2.0.0 on Release/2.0.0 at e213c56f29a06d286edb48c6db9a7f05cd573bb2
Exact upstream Release/1.9.5 base: 3193f83265a70585093f13d651167b763979ade1

## Method

- Optimized MSVC x64 Release builds use the same generator, compiler, flags, machine, disk, harness source, assets, and output medium.
- Each implementation receives 5 alternating warm-up cycles and 100 measured cycles. Odd cycles run RapidJSON first; even cycles run nlohmann first.
- Each cycle uses the same deterministic shuffled asset/operation order in both implementations.
- The byte-identical benchmark workload source SHA-256 on both branches is `6B3D5B70D7A9B52664F24CBE4E5951A30ADBE8204516DAF325ACC2467035B8B1`.
- LOAD starts before opening the source file and ends after Deserialize plus complete SDK reads of every buffer and every encoded image resource.
- EXPORT starts with the loaded representation and ends after Serialize, resource writes, GLB Flush or glTF manifest write, and output stream flush/close by destruction.
- ROUNDTRIP measures those LOAD and EXPORT boundaries back-to-back. Hashing, semantic validation, directory setup, and cleanup are outside all timers.
- The process peak-working-set metric covers the whole 12-operation cycle, including untimed validation; it is not an operation-specific peak.

glTF-SDK reads encoded PNG/JPEG bytes but does not decode pixels, create GPU textures, upload resources, compile shaders, or render. Those activities are excluded. Encoded image file I/O is included.

## Assets and provenance

Repository: https://github.com/KhronosGroup/glTF-Sample-Assets
Pinned commit: [9429648735279342b4c32b8745f7904196607379](https://github.com/KhronosGroup/glTF-Sample-Assets/commit/9429648735279342b4c32b8745f7904196607379)

| Case | Tier | Format | Source bytes | License |
| --- | --- | --- | ---: | --- |
| Box | small/core | gltf | 3546 | [CC-BY-4.0](https://github.com/KhronosGroup/glTF-Sample-Assets/blob/9429648735279342b4c32b8745f7904196607379/Models/Box/LICENSE.md) |
| Box | small/core | glb | 1664 | [CC-BY-4.0](https://github.com/KhronosGroup/glTF-Sample-Assets/blob/9429648735279342b4c32b8745f7904196607379/Models/Box/LICENSE.md) |
| Avocado | texture-heavy | gltf | 8110895 | [CC0-1.0](https://github.com/KhronosGroup/glTF-Sample-Assets/blob/9429648735279342b4c32b8745f7904196607379/Models/Avocado/LICENSE.md) |
| Avocado | texture-heavy | glb | 8110040 | [CC0-1.0](https://github.com/KhronosGroup/glTF-Sample-Assets/blob/9429648735279342b4c32b8745f7904196607379/Models/Avocado/LICENSE.md) |

| Pinned source file | Bytes | SHA-256 | Immutable URL |
| --- | ---: | --- | --- |
| Models/Box/glTF/Box.gltf | 2898 | 4A0D69EECFCE0672A50B71DC218CBACEC6C53FE2445040C235C6314B1B2C41B9 | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/Box/glTF/Box.gltf) |
| Models/Box/glTF/Box0.bin | 648 | 3266A8E39B9F425B3341CBE5EEC7849F44310256BFA651E6B8B40C85CE0CCAFB | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/Box/glTF/Box0.bin) |
| Models/Box/glTF-Binary/Box.glb | 1664 | ED52F7192B8311D700AC0CE80644E3852CD01537E4D62241B9ACBA023DA3D54E | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/Box/glTF-Binary/Box.glb) |
| Models/Avocado/glTF/Avocado.gltf | 2413 | C9BDE1D2F09514DAD0A16971DF2222BEB582FAD5DFC86203DE1D56389D52FEDA | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/Avocado/glTF/Avocado.gltf) |
| Models/Avocado/glTF/Avocado.bin | 23580 | 3BF7CE0EC994562EABF0CB72A52BBC7976818D630D62609A966B53A5FF9B9180 | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/Avocado/glTF/Avocado.bin) |
| Models/Avocado/glTF/Avocado_baseColor.png | 3158729 | 385DCE948C3B9E8BC93E1C930D796E72B02CBD474B121A160D577DE95EBBB48F | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/Avocado/glTF/Avocado_baseColor.png) |
| Models/Avocado/glTF/Avocado_normal.png | 3271114 | 25783C211761B7E32497FDD3E2490F3BAC05B738E5E2BC62FC5B78A270BB4E50 | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/Avocado/glTF/Avocado_normal.png) |
| Models/Avocado/glTF/Avocado_roughnessMetallic.png | 1655059 | 33EE80F7CCFD36825ACFBD5EF0A51326BEA49E33EFED852DCA44798798BD6901 | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/Avocado/glTF/Avocado_roughnessMetallic.png) |
| Models/Avocado/glTF-Binary/Avocado.glb | 8110040 | CCC9C3CE56423720B09399C2351537207CD5A65F859F9E6E2F30922762F3ABD4 | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/Avocado/glTF-Binary/Avocado.glb) |

## Timing comparison

Positive deltas mean nlohmann Release/2.0.0 is slower; negative deltas mean it is faster.

| Asset | Format | Operation | RapidJSON median (ms) | nlohmann median (ms) | Median delta (ms) | Delta | RapidJSON p95 (ms) | nlohmann p95 (ms) | p95 delta |
| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Avocado | glb | export | 48.040 | 48.108 | 0.068 | 0.14% | 117.875 | 112.911 | -4.21% |
| Avocado | glb | load | 20.295 | 22.589 | 2.294 | 11.30% | 74.821 | 76.387 | 2.09% |
| Avocado | glb | roundtrip | 73.911 | 74.494 | 0.583 | 0.79% | 156.059 | 142.684 | -8.57% |
| Avocado | gltf | export | 5.007 | 5.046 | 0.039 | 0.79% | 6.776 | 55.581 | 720.31% |
| Avocado | gltf | load | 16.867 | 18.723 | 1.856 | 11.00% | 67.088 | 73.417 | 9.43% |
| Avocado | gltf | roundtrip | 21.789 | 23.876 | 2.088 | 9.58% | 71.532 | 73.773 | 3.13% |
| Box | glb | export | 0.542 | 0.632 | 0.090 | 16.65% | 0.751 | 0.810 | 7.94% |
| Box | glb | load | 2.962 | 5.220 | 2.258 | 76.22% | 4.807 | 8.200 | 70.59% |
| Box | glb | roundtrip | 3.500 | 5.779 | 2.279 | 65.10% | 5.378 | 8.822 | 64.05% |
| Box | gltf | export | 0.792 | 0.882 | 0.089 | 11.26% | 1.139 | 1.156 | 1.51% |
| Box | gltf | load | 3.270 | 5.825 | 2.555 | 78.15% | 6.168 | 8.907 | 44.40% |
| Box | gltf | roundtrip | 3.973 | 6.329 | 2.356 | 59.29% | 54.063 | 10.789 | -80.04% |

## Interpretation

- On the small Box workload, nlohmann Release/2.0.0 adds about 2.0-2.6 ms to LOAD and ROUNDTRIP medians. Large percentages reflect the small absolute baseline.
- Avocado glTF LOAD is 11.00% slower and ROUNDTRIP is 9.58% slower at the median. Avocado GLB LOAD is 11.30% slower, while EXPORT is effectively flat (+0.14%) and the combined ROUNDTRIP is +0.79%.
- Output sizes are equal except for a two-byte reduction in the nlohmann Avocado glTF output. Byte hashes differ because serializer byte representations differ, but every output passed the SDK semantic/resource-byte round-trip comparison.
- Median process peak working set rises from 40.13 MiB to 40.45 MiB (about 0.8%). This coarse process metric includes untimed validation.
- This end-to-end result does not replace or reuse the earlier JSON microbenchmark conclusion; file opening, complete encoded-resource I/O, SDK document construction, serialization, resource writing, and file close are all included here.

## Input and output size

| Asset | Format | Input bytes | RapidJSON output bytes | nlohmann output bytes | Delta bytes | Delta |
| --- | --- | ---: | ---: | ---: | ---: | ---: |
| Box | gltf | 3546 | 1618 | 1618 | 0 | 0.00% |
| Box | glb | 1664 | 1632 | 1632 | 0 | 0.00% |
| Avocado | gltf | 8110895 | 8109906 | 8109904 | -2 | 0.00% |
| Avocado | glb | 8110040 | 8110104 | 8110104 | 0 | 0.00% |

Per-output canonical SHA-256 values are in load-export-output-hashes.csv. Different byte hashes are permitted only when both SDK reloads produce the same source Document, buffer bytes, and encoded image bytes.

## Peak working set

| Implementation | Measured processes | Median process peak (MiB) | Maximum process peak (MiB) |
| --- | ---: | ---: | ---: |
| rapidjson-1.9.5 | 100 | 40.13 | 40.25 |
| nlohmann-2.0.0 | 100 | 40.45 | 41.22 |

## Correctness and caveats

- FetchAssets.ps1 verifies every pinned source file's byte length and SHA-256 before running.
- After every timed LOAD, resource counts and complete buffer lengths are checked outside the interval.
- After every timed EXPORT/ROUNDTRIP, the output is reloaded with the same SDK and compared with the source Document, all buffer bytes, and all encoded image bytes.
- Output files are SHA-256 hashed only after the timer stops; all measured outputs were deterministic across 100 samples.
- Results are single-machine, hot-cache observations with 100-point nearest-rank p95 values, not confidence intervals. Filesystem cache, antivirus, thermals, and background activity can affect tails.
- A preliminary 30-sample run showed that Windows filesystem/scheduling stalls could dominate the second-highest observation. The retained run therefore uses 100 measured samples. Some p95 values still contain roughly 50 ms system stalls and should not be interpreted as parser-only latency.
- Peak memory is sampled at process level and includes untimed correctness work, so it is useful only as a coarse matched comparison.

## Build and test validation

- Both benchmark/test trees configured with Visual Studio 17 2022, MSVC 19.44.35228.0, x64, and Release flags `/MD /O2 /Ob2 /DNDEBUG`.
- RapidJSON benchmark branch resource-reader/writer targeted tests: 91/91 passed.
- nlohmann Release/2.0.0 resource-reader/writer targeted tests: 95/95 passed.
- RapidJSON benchmark branch complete existing suite: 400/400 passed.
- nlohmann Release/2.0.0 complete existing suite: 505/505 passed.
- Separate x64 Release SDK-only configure/builds with `ENABLE_BENCHMARKS=OFF` passed on both branches, confirming the network fetch and benchmark target remain opt-in.
- The retained benchmark run produced 1,200 valid timing rows per implementation (four asset/format cases × three operations × 100 samples), and every timed export/roundtrip passed untimed semantic and resource-byte validation.

## Raw evidence

- load-export-rapidjson-1.9.5-raw.csv
- load-export-nlohmann-2.0.0-raw.csv
- load-export-processes.csv
- load-export-order.csv
- load-export-output-hashes.csv
- load-export-environment.json
- load-export-summary.json
