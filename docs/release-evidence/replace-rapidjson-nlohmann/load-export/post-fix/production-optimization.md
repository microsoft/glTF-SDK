# Production JSON performance optimization

Generated from the retained T-38 corpus and the clean T-40 matched run.

## Acceptance result

| Implementation | N | LOAD mean / median / p95 |
| --- | ---: | ---: |
| Retained RapidJSON evidence | 30 | 1919.17 / 1883.58 / 2091.95 ms |
| Retained pre-fix Release/2.0.0 | 30 | 15769.76 / 15535.31 / 16866.38 ms |
| Fresh RapidJSON control | 30 | 1990.66 / 1950.91 / 2150.51 ms |
| Production post-fix Release/2.0.0 | 30 | **2019.59 / 1993.40 / 2132.92 ms** |

Post-fix mean LOAD is 87.19% lower than the retained pre-fix result and recovers 99.27% of the retained RapidJSON regression gap. It is +1.45% versus the fresh matched RapidJSON control.

## All 16 LOAD cases

Times are mean / median / p95 in milliseconds.

| Case | N | Retained pre-fix | Fresh RapidJSON | Post-fix | Post vs pre mean | Post vs fresh Rapid mean |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| ABeautifulGame-draco-glb | 30 | 38.29 / 34.94 / 49.23 | 34.46 / 33.03 / 41.88 | 39.18 / 35.78 / 44.86 | +2.35% | +13.72% |
| ABeautifulGame-glb | 30 | 86.17 / 78.91 / 124.87 | 81.62 / 76.73 / 95.71 | 85.02 / 79.53 / 99.89 | -1.34% | +4.17% |
| Avocado-glb | 100 | 24.87 / 22.65 / 31.64 | 25.63 / 20.91 / 31.59 | 25.82 / 23.42 / 30.09 | +3.80% | +0.72% |
| Avocado-gltf | 100 | 20.57 / 18.49 / 25.41 | 19.89 / 17.29 / 23.66 | 23.24 / 19.74 / 29.36 | +13.02% | +16.87% |
| Box-glb | 100 | 7.08 / 4.98 / 8.04 | 4.06 / 2.85 / 4.61 | 5.59 / 5.09 / 8.10 | -21.07% | +37.85% |
| Box-gltf | 100 | 6.59 / 5.24 / 8.44 | 4.00 / 3.16 / 4.41 | 5.89 / 5.45 / 8.33 | -10.59% | +47.46% |
| EmissiveStrengthTest-glb | 100 | 6.77 / 5.72 / 9.09 | 3.88 / 3.49 / 5.25 | 6.35 / 5.87 / 9.01 | -6.28% | +63.69% |
| IridescenceSuzanne-glb | 100 | 7.61 / 6.29 / 9.76 | 4.30 / 4.07 / 5.77 | 8.23 / 6.48 / 9.18 | +8.16% | +91.34% |
| MaterialsVariantsShoe-glb | 100 | 24.37 / 21.90 / 32.81 | 21.98 / 20.11 / 27.41 | 24.52 / 22.70 / 30.77 | +0.62% | +11.55% |
| MorphStressTest-glb | 100 | 8.48 / 7.19 / 11.78 | 7.47 / 5.17 / 7.76 | 9.36 / 7.14 / 10.40 | +10.33% | +25.30% |
| NodePerformanceTest-glb | 30 | 15769.76 / 15535.31 / 16866.38 | 1990.66 / 1950.91 / 2150.51 | 2019.59 / 1993.40 / 2132.92 | -87.19% | +1.45% |
| SheenTestGrid-glb | 100 | 12.62 / 10.50 / 14.37 | 9.73 / 8.02 / 11.59 | 13.00 / 10.38 / 16.74 | +3.00% | +33.60% |
| SimpleInstancing-glb | 100 | 5.55 / 5.06 / 8.07 | 3.22 / 2.94 / 4.87 | 7.65 / 5.30 / 8.12 | +37.79% | +137.41% |
| SpecularTest-glb | 100 | 8.92 / 7.31 / 11.83 | 5.47 / 4.55 / 6.36 | 9.21 / 7.45 / 11.23 | +3.18% | +68.24% |
| TextureTransformMultiTest-glb | 100 | 12.24 / 9.34 / 17.49 | 8.54 / 6.55 / 10.09 | 11.28 / 9.34 / 15.36 | -7.84% | +31.99% |
| XmpMetadataRoundedCube-glb | 100 | 6.79 / 5.16 / 8.21 | 3.29 / 2.99 / 4.48 | 6.43 / 5.36 / 7.84 | -5.27% | +95.46% |

## Aggregate timing

Summed per-case means retain equal case weighting within each group.

| Group | Operation | Cases | Retained pre-fix mean total | Fresh RapidJSON mean total | Post-fix mean total | Post vs pre |
| --- | --- | ---: | ---: | ---: | ---: | ---: |
| overall:all-cases | load | 16 | 16046.69 ms | 2228.20 ms | 2300.36 ms | -85.66% |
| overall:all-cases | export | 16 | 1374.18 ms | 840.57 ms | 1422.25 ms | +3.50% |
| overall:all-cases | roundtrip | 16 | 17366.93 ms | 3021.36 ms | 3718.18 ms | -78.59% |
| complexityClass:large | load | 3 | 15894.22 ms | 2106.74 ms | 2143.79 ms | -86.51% |
| complexityClass:large | export | 3 | 1224.00 ms | 687.51 ms | 1265.59 ms | +3.40% |
| complexityClass:large | roundtrip | 3 | 17066.70 ms | 2743.66 ms | 3404.48 ms | -80.05% |
| complexityClass:medium | load | 7 | 110.75 ms | 97.54 ms | 115.44 ms | +4.24% |
| complexityClass:medium | export | 7 | 139.82 ms | 146.07 ms | 148.16 ms | +5.96% |
| complexityClass:medium | roundtrip | 7 | 254.06 ms | 247.57 ms | 262.66 ms | +3.39% |
| complexityClass:small | load | 6 | 41.72 ms | 23.92 ms | 41.13 ms | -1.42% |
| complexityClass:small | export | 6 | 10.36 ms | 6.99 ms | 8.50 ms | -17.96% |
| complexityClass:small | roundtrip | 6 | 46.18 ms | 30.12 ms | 51.05 ms | +10.54% |

Per-tier, extension-coverage, and exact-extension mean/median/p95 aggregates are retained in `load-export-three-way-summary.json`; every case/operation triplet is in `load-export-three-way-aggregates.csv`.

## Process memory, outputs, and wall time

| Implementation | Cohort | N | Peak working set median / maximum | Process duration median / p95 |
| --- | --- | ---: | ---: | ---: |
| retainedRapidJson | large-enabled | 30 | 380.52 / 400.98 MiB | 12.766 / 13.451 s |
| retainedRapidJson | standard-only | 70 | 40.50 / 41.12 MiB | 1.188 / 1.261 s |
| retainedPreFix | large-enabled | 30 | 431.51 / 433.52 MiB | 83.180 / 90.621 s |
| retainedPreFix | standard-only | 70 | 40.87 / 41.03 MiB | 1.360 / 1.465 s |
| freshRapidJson | large-enabled | 30 | 380.62 / 401.34 MiB | 13.091 / 16.561 s |
| freshRapidJson | standard-only | 70 | 40.49 / 41.38 MiB | 1.258 / 1.374 s |
| postFix | large-enabled | 30 | 431.45 / 433.51 MiB | 15.460 / 16.547 s |
| postFix | standard-only | 70 | 40.85 / 41.00 MiB | 1.437 / 1.564 s |

- A same-toolchain current A/B check against exact pre-fix `20dbbc9` matched all 32 export/round-trip output sets byte-for-byte: `true`.
- Historical retained-versus-fresh canonical hashes match 15/16 nlohmann cases and 15/16 RapidJSON cases; every output size matches. Only Avocado glTF changed across independent rebuilds on both branches, so it is not a parser/validator delta.
- One deterministic post-fix export set across 16 cases contains 21 files and 120831322 bytes.
- Retained matched-run wall time: 3662.669 s; clean post-fix matched-run wall time: 1356.441 s (-62.97%).

## Integrity

- All four raw datasets contain 4,170 successful timing rows.
- Five alternating warm-ups, 100 standard samples, and 30 large samples were used.
- Semantic/resource/extension verification and output hashing remained outside timers.
- The post-fix run uses Sample Assets `9429648735279342b4c32b8745f7904196607379`, RapidJSON `743b7ecd749a53e8d848db56839752a65205e915`, and production candidate `dc9c9f78dc33b9df5201e28dea2eeddcb5aa7b2e`.
- The first pilot was stopped and deleted because unrelated local validation work overlapped it; only the subsequent idle-machine clean run is retained.
