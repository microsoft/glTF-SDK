# T-40 production validation

Validation date: 2026-09-03

## Revisions and toolchain

- Production parser commit: `9e40a61`
- Production validation commit: `8a5c498`
- Dependency/provenance commit: `dc9c9f78dc33b9df5201e28dea2eeddcb5aa7b2e`
- Exact pre-fix reference: `20dbbc9db365cb50ea64cebf36559e2b18e60b99`
- RapidJSON comparison:
  `743b7ecd749a53e8d848db56839752a65205e915`
- Sample Assets:
  `9429648735279342b4c32b8745f7904196607379`
- Visual Studio 17 2022, MSVC `19.44.35228.0`, Windows SDK
  `10.0.26100.0`, CMake 3.31.8, C++14
- Matched performance build: x64 Release `/MD /O2 /Ob2 /DNDEBUG`

## Production behavior

- `Internal::ParseJson` now builds the insertion-ordered nlohmann DOM directly
  from strict SAX events. The builder retains decoded-key duplicate rejection,
  signed/unsigned/floating callbacks, UTF-8 checking, compact BOM policy,
  strict comments/trailing-input behavior, 256-container depth, string/stream
  equivalence, and SDK exception translation.
- Draft-04 validation first runs without `ValidationResults`. A failure reruns
  the unchanged exhaustive results-producing path before selecting the public
  diagnostic.
- Valijson patch 0005 uses separate scalar sets for null, boolean, string, and
  finite `getNumber()` values. Structured or non-finite input falls back to
  recursive pairwise equality. Results-producing validation always remains
  pairwise.

## Focused and complete tests

| Gate | Result |
| --- | ---: |
| Focused private JSON, schema, deserializer, extension, ExtrasDocument, serializer, and official-schema classes; x64 Debug | 224/224 passed |
| Official Draft-04 wrappers | 3/3 passed, executing 329 selected upstream cases |
| x64 Debug complete suite | 514/514 passed |
| x64 RelWithDebInfo complete suite | 514/514 passed |
| x64 Release complete suite | 514/514 passed |
| Win32 Debug complete suite | 514/514 passed |
| Win32 RelWithDebInfo complete suite | 514/514 passed |
| Optimized clang-cl RelWithDebInfo complete suite | 514/514 passed |
| x64 Debug / RelWithDebInfo CTest | 1/1 and 1/1 passed |
| Win32 Debug / RelWithDebInfo CTest | 1/1 and 1/1 passed |

The new regressions cover root scalar/object/array values, empty and partial
SAX input, escaped and nested duplicate keys, parsed insertion order, depth
255/256/257, six malformed UTF-8 forms, UTF-8 and unsupported BOMs for string
and stream input, comments, trailing commas/tokens, finite numeric categories
and boundaries, repeated error cleanup, every requested scalar uniqueness
category, signed zero, integer/float equality, values around and above
2^53, int64/uint64 boundaries, non-finite fallback, mixed scalars, structured
objects/arrays, nested arrays, and exact `uniqueItems`/`minimum` paths.

## Platform, package, and consumer gates

- ARM64 Debug and RelWithDebInfo SDK compile/link/install passed; both
  installed-only consumers compiled and linked.
- x64 and Win32 Debug and RelWithDebInfo SDK install plus installed-only
  consumer build/link/run passed.
- Invalid-proxy offline configure/build/install passed for x64, Win32, and
  ARM64 in Debug and RelWithDebInfo.
- All six installed packages contain 44 files and 38 public headers. No
  package contains `json.hpp`, `json_fwd.hpp`, or `RapidJsonUtils.h`; installed
  headers contain zero RapidJSON, nlohmann, or Valijson references.
- Benchmark-disabled Release configure/build gates passed on both comparison
  branches and produced no load-export executable.

## Corpus, outputs, and benchmark

- `FetchAssets.ps1 -VerifyOnly` validated all 21 pinned files and all 16
  manifest selections.
- `TestCorpus.ps1` passed on both RapidJSON and production branches, including
  every typed/raw extension, required-extension set, buffer byte, encoded
  image byte, export, reload, and document comparison.
- A current same-toolchain A/B run against exact pre-fix `20dbbc9` passed all
  48 semantic rows and matched all 32 export/round-trip output sets
  byte-for-byte. Details are in
  `load-export/post-fix/pre-post-output-equivalence.json`.
- The clean retained benchmark produced 4,170 successful rows per
  implementation after five warm-ups, with 100 standard and 30 large samples.
  NodePerformanceTest production LOAD is
  **2,019.59 ms mean / 1,993.40 ms median / 2,132.92 ms p95**. This is within
  4.31% of the T-39 clean spike, 87.19% below the retained pre-fix mean, and
  1.45% above the fresh matched RapidJSON mean.
- Detailed raw data, environment, memory, hashes, per-case mean/median/p95,
  tier/extension aggregates, three-way comparisons, and wall time are under
  `load-export/post-fix/`.

## Patch, hash, scan, and formatting gates

- All five Valijson patches passed `git apply --check`, applied in documented
  order to a pristine import, and reproduced every intermediate hash.
- Patch 0005 SHA-256:
  `C2506247BDC75128A6FF64C976614A48CDDFF4FEC37A0783592C29E15C18602F`
- Imported tree after patch 0005:
  `4A2CE6EE33E8D01E2F6C3188625024A3794ECA90BB9995D2547F57B8B3D06E73`
- Final pruned 48-file Valijson tree:
  `098816AE043B63A5DB3E3000407B572CAC02C42A9C8F1CC523DC388CA53F785D`
- nlohmann header:
  `AAF127C04CB31C406E5B04A63F1AE89369FCCDE6D8FA7CDDA1ED4F32DFC5DE63`
- Official fixture tree:
  `8A2103D0C76F5CB5536B82E07A8EE08B611260245DFDFCE934D9C68F59BF3B7C`
- Shipped code/workflow scans found zero RapidJSON remnants; public headers
  found zero private-vendor names.
- All seven workflow YAML files parsed successfully. `git diff --check`
  passed.

## Sanitizer note

The repository-defined sanitizer gate is Linux Clang ASAN/UBSAN in CI. An
additional local Windows clang-cl ASAN probe successfully instrumented and ran
all valid parser/uniqueness cases, but clang-cl's manually linked dynamic ASAN
runtime converted every expected C++ exception into the same `0xc0000005` SEH
failure, including unrelated established writer and JSON Pointer tests. It is
not a usable repository gate.

## Remote CI

Implementation/evidence head
`538ba17d60b519631a776300dcff33b8cfe1257a` was pushed normally to
`origin/Release/2.0.0`. GitHub Actions run
[`33763176875`](https://github.com/SergioRZMasson/glTF-SDK/actions/runs/33763176875)
completed successfully with all 21 jobs passing. This includes:

- Linux and macOS Debug/RelWithDebInfo tests, package scans, and consumers;
- Windows x64/Win32/ARM64 Debug/RelWithDebInfo;
- iOS device/simulator Debug/RelWithDebInfo;
- Android three-ABI Debug/RelWithDebInfo; and
- the complete Linux Clang ASAN/UBSAN full and malformed/deep suites.

All temporary probes, output-equivalence worktrees, benchmark run directories,
and profiling artifacts were removed. The retained RapidJSON comparison
worktree remains at its required branch head.
