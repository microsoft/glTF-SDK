# JSON benchmark: rapidjson-1.9.5

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
| deserialize_reject | malformed | 30 | 9.325 | 10.742 |
| deserialize_schema_off | extension-heavy | 30 | 37.810 | 68.290 |
| deserialize_schema_off | float-heavy | 30 | 1012.500 | 2442.850 |
| deserialize_schema_off | large | 30 | 8015.400 | 14208.400 |
| deserialize_schema_off | small | 30 | 24.078 | 49.754 |
| deserialize_schema_on | extension-heavy | 30 | 4419.390 | 5848.610 |
| deserialize_schema_on | float-heavy | 30 | 7234.900 | 12859.350 |
| deserialize_schema_on | large | 30 | 33791.500 | 57375.200 |
| deserialize_schema_on | small | 30 | 3341.866 | 3667.566 |
| end_to_end | extension-heavy | 30 | 4289.850 | 5246.470 |
| end_to_end | float-heavy | 30 | 9398.050 | 13827.850 |
| end_to_end | large | 30 | 35069.700 | 44710.400 |
| end_to_end | small | 30 | 4416.806 | 6418.010 |
| parse | deep | 30 | 16.516 | 32.832 |
| parse | extension-heavy | 30 | 2.840 | 3.215 |
| parse | float-heavy | 30 | 290.150 | 302.980 |
| parse | large | 30 | 747.950 | 895.900 |
| parse | small | 30 | 2.543 | 8.044 |
| parse_reject | malformed | 30 | 7.007 | 8.242 |
| public_serialize | extension-heavy | 30 | 6.870 | 24.700 |
| public_serialize | float-heavy | 30 | 409.650 | 1460.050 |
| public_serialize | large | 30 | 695.900 | 2498.500 |
| public_serialize | small | 30 | 3.576 | 18.090 |
| schema_compile | gltf-root | 30 | 3786.400 | 4636.200 |
| schema_validate | extension-heavy | 30 | 8.700 | 11.300 |
| schema_validate | float-heavy | 30 | 1526.500 | 1929.850 |
| schema_validate | large | 30 | 14897.400 | 17356.200 |
| schema_validate | small | 30 | 19.738 | 29.520 |
| write | deep | 30 | 7.604 | 10.334 |
| write | extension-heavy | 30 | 1.470 | 1.915 |
| write | float-heavy | 30 | 333.240 | 388.680 |
| write | large | 30 | 333.700 | 412.250 |
| write | small | 30 | 0.721 | 1.018 |

## Process, binary, and build metrics

- Peak benchmark-process working set: 10600448 bytes (10.11 MiB)
- Benchmark executable size: 1197056 bytes
- Clean benchmark-target build: 29.556 seconds
- No-op incremental benchmark-target build: 2.269 seconds
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
cmake -S E:\Base3D\glTF-SDK -B E:\Base3D\glTF-SDK\Built\Int\cmake_benchmark_baseline -DENABLE_UNIT_TESTS=OFF -DENABLE_SAMPLES=OFF -DENABLE_BENCHMARKS=ON
cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_benchmark_baseline --config RelWithDebInfo --target GLTFSDK.JsonBenchmarks
powershell -ExecutionPolicy Bypass -File E:\Base3D\glTF-SDK\Benchmarks\RunBenchmarks.ps1 -BuildDir E:\Base3D\glTF-SDK\Built\Int\cmake_benchmark_baseline -Label rapidjson-1.9.5
```

Raw samples: `benchmark-rapidjson-1.9.5-raw.csv`
