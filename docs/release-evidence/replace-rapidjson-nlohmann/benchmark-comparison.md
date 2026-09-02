# JSON benchmark comparison

Date: 2026-09-01
Baseline: `rapidjson-1.9.5`
Candidate: `nlohmann-valijson-2.0.0`
Method: identical machine, generator, compiler, architecture, RelWithDebInfo flags, assets, 30 timed samples, 5 warm-ups, and nearest-rank median/p95 aggregation.

## Timing deltas

| Operation | Workload | Baseline median (us) | Candidate median (us) | Absolute delta (us) | Delta | Baseline p95 (us) | Candidate p95 (us) | p95 delta |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| deserialize_reject | malformed | 9.325 | 14.793 | 5.468 | 58.64% | 10.742 | 19.224 | 78.96% |
| deserialize_schema_off | extension-heavy | 37.810 | 40.890 | 3.080 | 8.15% | 68.290 | 46.390 | -32.07% |
| deserialize_schema_off | float-heavy | 1012.500 | 1183.550 | 171.050 | 16.89% | 2442.850 | 1412.600 | -42.17% |
| deserialize_schema_off | large | 8015.400 | 7552.200 | -463.200 | -5.78% | 14208.400 | 8515.500 | -40.07% |
| deserialize_schema_off | small | 24.078 | 26.376 | 2.298 | 9.54% | 49.754 | 45.900 | -7.75% |
| deserialize_schema_on | extension-heavy | 4419.390 | 4154.450 | -264.940 | -5.99% | 5848.610 | 5273.480 | -9.83% |
| deserialize_schema_on | float-heavy | 7234.900 | 6539.750 | -695.150 | -9.61% | 12859.350 | 8310.950 | -35.37% |
| deserialize_schema_on | large | 33791.500 | 17771.700 | -16019.800 | -47.41% | 57375.200 | 20120.500 | -64.93% |
| deserialize_schema_on | small | 3341.866 | 4286.740 | 944.874 | 28.27% | 3667.566 | 4575.388 | 24.75% |
| end_to_end | extension-heavy | 4289.850 | 4203.750 | -86.100 | -2.01% | 5246.470 | 5144.150 | -1.95% |
| end_to_end | float-heavy | 9398.050 | 7449.600 | -1948.450 | -20.73% | 13827.850 | 10012.600 | -27.59% |
| end_to_end | large | 35069.700 | 20893.500 | -14176.200 | -40.42% | 44710.400 | 27033.900 | -39.54% |
| end_to_end | small | 4416.806 | 4428.050 | 11.244 | 0.25% | 6418.010 | 6070.312 | -5.42% |
| parse | deep | 16.516 | 66.676 | 50.160 | 303.71% | 32.832 | 80.368 | 144.79% |
| parse | extension-heavy | 2.840 | 12.400 | 9.560 | 336.62% | 3.215 | 14.290 | 344.48% |
| parse | float-heavy | 290.150 | 1158.820 | 868.670 | 299.39% | 302.980 | 1648.150 | 443.98% |
| parse | large | 747.950 | 5213.450 | 4465.500 | 597.03% | 895.900 | 7847.350 | 775.92% |
| parse | small | 2.543 | 9.126 | 6.584 | 258.94% | 8.044 | 16.725 | 107.93% |
| parse_reject | malformed | 7.007 | 13.406 | 6.400 | 91.34% | 8.242 | 21.915 | 165.91% |
| public_serialize | extension-heavy | 6.870 | 12.650 | 5.780 | 84.13% | 24.700 | 13.910 | -43.68% |
| public_serialize | float-heavy | 409.650 | 966.500 | 556.850 | 135.93% | 1460.050 | 1125.200 | -22.93% |
| public_serialize | large | 695.900 | 2858.800 | 2162.900 | 310.81% | 2498.500 | 3177.700 | 27.18% |
| public_serialize | small | 3.576 | 7.336 | 3.760 | 105.15% | 18.090 | 8.308 | -54.07% |
| schema_compile | gltf-root | 3786.400 | 4191.600 | 405.200 | 10.70% | 4636.200 | 4510.400 | -2.71% |
| schema_validate | extension-heavy | 8.700 | 12.110 | 3.410 | 39.20% | 11.300 | 13.760 | 21.77% |
| schema_validate | float-heavy | 1526.500 | 1105.200 | -421.300 | -27.60% | 1929.850 | 1155.150 | -40.14% |
| schema_validate | large | 14897.400 | 4741.900 | -10155.500 | -68.17% | 17356.200 | 5585.500 | -67.82% |
| schema_validate | small | 19.738 | 13.164 | -6.574 | -33.31% | 29.520 | 17.094 | -42.09% |
| write | deep | 7.604 | 13.708 | 6.104 | 80.27% | 10.334 | 14.172 | 37.14% |
| write | extension-heavy | 1.470 | 2.853 | 1.383 | 94.05% | 1.915 | 3.093 | 61.49% |
| write | float-heavy | 333.240 | 486.290 | 153.050 | 45.93% | 388.680 | 791.460 | 103.63% |
| write | large | 333.700 | 644.500 | 310.800 | 93.14% | 412.250 | 924.800 | 124.33% |
| write | small | 0.721 | 1.861 | 1.140 | 158.11% | 1.018 | 2.129 | 109.09% |

## Process, binary, and build deltas

| Metric | Baseline | Candidate | Absolute delta | Delta |
| --- | ---: | ---: | ---: | ---: |
| Peak working set (bytes) | 10600448 | 11890688 | 1290240 | 12.17% |
| Executable size (bytes) | 1197056 | 1725440 | 528384 | 44.14% |
| Clean build (seconds) | 29.556 | 30.365 | 0.809 | 2.74% |
| Incremental build (seconds) | 2.269 | 1.088 | -1.181 | -52.04% |

Allocation counts remain omitted for both points because this Windows/MSVC environment has no dependency-free reproducible per-process allocation counter.

## Review disposition

**ACCEPTED for Release/2.0.0.** The approved plan defines no numeric rejection threshold. All absolute and percentage deltas are retained above, the workloads and methodology are unchanged, and correctness, strictness, packaging, and platform gates pass. The execution request explicitly authorizes completing and pushing the approved plan after recording benchmark evidence.

Material regressions are visible rather than hidden: ordered-object lookup, strict duplicate/depth tracking, and Draft-04 Valijson compilation are expected to cost more than the legacy RapidJSON path. The candidate removes a second production DOM, preserves deterministic output, and keeps the separated parse/validate/write data available for future optimization.

The largest median regressions are isolated raw parsing of the large asset
(+597.03%), public serialization of the large asset (+310.81%), and the
standalone small write (+158.11%). In the complete production path, large
schema validation improves by 68.17% and large end-to-end processing improves
by 40.42%; float-heavy end-to-end improves by 20.73%, while small end-to-end is
effectively flat at +0.25%. Peak working set grows 12.17%, executable size
44.14%, and clean build time 2.74%. This balance supports the accepted
disposition while identifying parser/write throughput and binary size as
future optimization targets.

Raw baseline: `benchmark-rapidjson-1.9.5-raw.csv`
Raw candidate: `benchmark-nlohmann-valijson-2.0.0-raw.csv`
