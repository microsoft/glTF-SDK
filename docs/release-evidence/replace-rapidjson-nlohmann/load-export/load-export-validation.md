# Expanded load-export validation

Validated 2026-09-02 on the same Windows x64 machine used for the retained
benchmark run.

## Revisions and toolchain

- RapidJSON: `perf/Release-1.9.5-load-export` at
  `695b90de1f044a1081e00049d5c33c69152159e6`.
- nlohmann: `Release/2.0.0` at
  `47c456f0f817eba6fe42af1561d016e767c2453e`.
- Visual Studio 17 2022, MSVC `19.44.35228.0`, x64 Release
  `/MD /O2 /Ob2 /DNDEBUG`.
- The eight files under `Benchmarks/LoadExport` used by the matched run were
  byte-identical between branches. Exact hashes are in
  `load-export-environment.json`.

## Offline and corpus gates

- Separate `ENABLE_BENCHMARKS=OFF`, `ENABLE_UNIT_TESTS=OFF`,
  `ENABLE_SAMPLES=OFF` configure/builds completed for both branches, and no
  load-export executable was present in either disabled build tree.
- `FetchAssets.ps1 -VerifyOnly` validated all 21 cached files from pinned
  glTF-Sample-Assets commit
  `9429648735279342b4c32b8745f7904196607379`.
- `ValidateAssets.ps1` reparsed all 16 entrypoints and matched byte lengths,
  SHA-256 values, structural counts, `extensionsUsed`,
  `extensionsRequired`, immutable source/license metadata, and the typed/raw
  partition.
- `TestCorpus.ps1` passed on both committed harness revisions. Each branch
  loaded, exported, reparsed, and compared all 16 cases, including required
  extension sets, SDK-typed versus raw extension representation, complete
  buffer bytes, and encoded image bytes.

Normal builds/tests do not invoke the fetch or validation scripts and remain
network-independent.

## Unit tests

| Branch | Targeted resource/extension tests | Complete Release suite |
| --- | ---: | ---: |
| RapidJSON 1.9.5 | 84/84 passed | 400/400 passed |
| nlohmann 2.0.0 | 84/84 passed | 505/505 passed |

The targeted filter was
`GLBResourceWriterTests.*:GLTFResourceReaderTests.*:GLTFResourceWriterTests.*:ExtensionsTests.*`.

## Retained benchmark integrity

- Five alternating warm-up cycles completed per implementation.
- Thirteen standard cases produced 100 samples per operation; three large
  cases produced 30 per operation.
- Each implementation produced 4,170 valid raw timing rows (8,340 total),
  across LOAD, EXPORT, and ROUNDTRIP.
- `semantic_status` is `ok` in every row.
- All export and round-trip output sets were deterministic within each
  implementation and case.
- The runner recorded 105 matched order rows and 210 process rows.
- Total retained wall time was 1:01:02.669. Median whole-process duration was
  12.776 seconds (RapidJSON) versus 83.237 seconds (nlohmann) for
  large-enabled cycles, and 1.188 versus 1.361 seconds for standard-only
  cycles.
