# JSON benchmark: nlohmann-valijson-2.0.0

Date: 2026-09-01
Source commit: `3193f83265a70585093f13d651167b763979ade1`
Configuration: RelWithDebInfo
Samples per workload: 30 after 5 warm-up samples
Aggregation: median and nearest-rank p95 of microseconds per operation

## Machine and toolchain

- Computer: LENOVO 30BFS9C91U
- CPU: Intel(R) Xeon(R) W-2235 CPU @ 3.80GHz
- Logical processors: 12
- Installed memory: 63.59 GiB
- OS: Microsoft Windows 11 Enterprise Insider Preview 10.0.26310
- cmake version 3.31.8
- Generator: Visual Studio 17 2022 x64
- C++ compiler: C:/Program Files/Microsoft Visual Studio/2022/Enterprise/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/cl.exe
- Compiler version: 19.44.35228.0
- CMAKE_CXX_FLAGS: `/DWIN32 /D_WINDOWS /W3 /GR /EHsc`
- RelWithDebInfo flags: `/MD /Zi /O2 /Ob1 /DNDEBUG`

## Workload timing

| Operation | Workload | Samples | Median (us/op) | p95 (us/op) |
| --- | --- | ---: | ---: | ---: |
| deserialize_reject | malformed | 30 | 14.793 | 19.224 |
| deserialize_schema_off | extension-heavy | 30 | 40.890 | 46.390 |
| deserialize_schema_off | float-heavy | 30 | 1183.550 | 1412.600 |
| deserialize_schema_off | large | 30 | 7552.200 | 8515.500 |
| deserialize_schema_off | small | 30 | 26.376 | 45.900 |
| deserialize_schema_on | extension-heavy | 30 | 4154.450 | 5273.480 |
| deserialize_schema_on | float-heavy | 30 | 6539.750 | 8310.950 |
| deserialize_schema_on | large | 30 | 17771.700 | 20120.500 |
| deserialize_schema_on | small | 30 | 4286.740 | 4575.388 |
| end_to_end | extension-heavy | 30 | 4203.750 | 5144.150 |
| end_to_end | float-heavy | 30 | 7449.600 | 10012.600 |
| end_to_end | large | 30 | 20893.500 | 27033.900 |
| end_to_end | small | 30 | 4428.050 | 6070.312 |
| parse | deep | 30 | 66.676 | 80.368 |
| parse | extension-heavy | 30 | 12.400 | 14.290 |
| parse | float-heavy | 30 | 1158.820 | 1648.150 |
| parse | large | 30 | 5213.450 | 7847.350 |
| parse | small | 30 | 9.126 | 16.725 |
| parse_reject | malformed | 30 | 13.406 | 21.915 |
| public_serialize | extension-heavy | 30 | 12.650 | 13.910 |
| public_serialize | float-heavy | 30 | 966.500 | 1125.200 |
| public_serialize | large | 30 | 2858.800 | 3177.700 |
| public_serialize | small | 30 | 7.336 | 8.308 |
| schema_compile | gltf-root | 30 | 4191.600 | 4510.400 |
| schema_validate | extension-heavy | 30 | 12.110 | 13.760 |
| schema_validate | float-heavy | 30 | 1105.200 | 1155.150 |
| schema_validate | large | 30 | 4741.900 | 5585.500 |
| schema_validate | small | 30 | 13.164 | 17.094 |
| write | deep | 30 | 13.708 | 14.172 |
| write | extension-heavy | 30 | 2.853 | 3.093 |
| write | float-heavy | 30 | 486.290 | 791.460 |
| write | large | 30 | 644.500 | 924.800 |
| write | small | 30 | 1.861 | 2.129 |

## Process, binary, and build metrics

- Peak benchmark-process working set: 11890688 bytes (11.34 MiB)
- Benchmark executable size: 1725440 bytes
- Clean benchmark-target build: 30.365 seconds
- No-op incremental benchmark-target build: 1.088 seconds
- Allocation count: omitted; this Windows/MSVC environment has no dependency-free, reproducible per-process allocation counter.

The clean-build timer starts after configuration and dependency acquisition,
then builds the SDK and benchmark target in a script-owned empty build tree.
The incremental value is an unchanged no-op target build.

## Assets

| Asset | Bytes | SHA-256 |
| --- | ---: | --- |
| deep.json | 513 | `9CD13E3D387F21922247827F547EC757907DDBD9ABEDAF6C5EFA817381626D0B` |
| extension-heavy.gltf | 247 | `CAB8F050DB13B6214063F6D1AE3A5A499CCD5ACF1C41D0C30184145606D5C21B` |
| float-heavy.gltf | 41131 | `3140D6CA033E9B0EB9DE8D3B6F14CEB9250A11102DCDBEA9D90085608E657D0E` |
| large.gltf | 194096 | `D7F508DAE8A12550E861B45E78D836B21E14B27A1B1D64E42732ABCA5E111492` |
| malformed.json | 36 | `A8655EA37B812A5F0CECC5F17AFDCC89FBB520E69BE016004311EA24502A6180` |
| small.gltf | 126 | `500420FBECD7960CBCE709A66E650205856C6B967BCC1310FFA8827D84A7B0ED` |

## Reproduction

```powershell
cmake -S E:\Base3D\glTF-SDK -B E:\Base3D\glTF-SDK\Built\Int\cmake_benchmark_candidate -DENABLE_UNIT_TESTS=OFF -DENABLE_SAMPLES=OFF -DENABLE_BENCHMARKS=ON
cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_benchmark_candidate --config RelWithDebInfo --target GLTFSDK.JsonBenchmarks
powershell -ExecutionPolicy Bypass -File E:\Base3D\glTF-SDK\Benchmarks\RunBenchmarks.ps1 -BuildDir E:\Base3D\glTF-SDK\Built\Int\cmake_benchmark_candidate -Label nlohmann-valijson-2.0.0
```

Raw samples: `benchmark-nlohmann-valijson-2.0.0-raw.csv`

Comparison baseline requested: `E:\Base3D\glTF-SDK\docs\release-evidence\replace-rapidjson-nlohmann\benchmark-baseline.md`
