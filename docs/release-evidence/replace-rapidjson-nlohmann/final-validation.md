# Final validation

Validation date: 2026-09-01

## Status

All local and remote gates pass. The final implementation SHA
`d28ffe663fe9aaabdd6b34fc19fb834727d938c2` was pushed normally to
`origin/Release/2.0.0`, the remote ref matched, and GitHub Actions run
[`33579694425`](https://github.com/SergioRZMasson/glTF-SDK/actions/runs/33579694425)
completed successfully with all 21 required jobs passing.

## Source and toolchain

- Branch: `Release/2.0.0`
- Approved base and starting HEAD:
  `3193f83265a70585093f13d651167b763979ade1`
- CMake: 3.31.8
- Local generator: Visual Studio 2022 17.14
- Compiler: MSVC 19.44
- Windows SDK: 10.0.26100.0
- Language baseline: C++14
- CMake minimum remains 3.5; Apple deployment baseline remains 10.11

The installed CMake does not expose the planned Visual Studio 18 generator,
so the local matrix uses Visual Studio 17. The Windows workflow retains the
newer remote toolchain gate.

Before validation, no path was staged. The worktree contained only the
feature implementation and its pre-existing `.ai` planning inputs. No reset,
clean, force operation, blanket staging, or adjacent-repository edit was
performed.

## Local Windows matrix

Fresh `cmake_final_*` build trees produced:

| Architecture | Configuration | SDK build/install | Full executable | CTest | Installed consumer |
| --- | --- | --- | --- | --- | --- |
| x64 | Debug | passed | 505/505 | 1/1 | built, linked, ran |
| x64 | RelWithDebInfo | passed | 505/505 | 1/1 | built, linked, ran |
| Win32 | Debug | passed | 505/505 | 1/1 | built, linked, ran |
| Win32 | RelWithDebInfo | passed | 505/505 | 1/1 | built, linked, ran |
| ARM64 | Debug | passed | cross-build only | n/a | built and linked |
| ARM64 | RelWithDebInfo | passed | cross-build only | n/a | built and linked |

The four host test runs generated XML under the corresponding ignored
`Built/Int/cmake_final_*` trees. Each XML root reports 505 tests, zero
failures, and zero disabled tests.

The final focused JSON/schema/deserializer/serializer/extensions/extras run
passed 219/219. The full suite includes all 329 selected official Draft-04
cases, all 48 extension tests, strict malformed/UTF-8/BOM/depth regressions,
pointer and extras behavior, deterministic serialization, and legacy CWE
regressions.

The first required CTest probe exposed that the executable had never been
registered and returned `No tests were found`. U-04 added the missing
`add_test` registration. Reconfigured x64 and Win32 Debug/RelWithDebInfo
CTest runs then each passed 1/1, executing the complete unit-test binary.

Remote optimized Linux diagnostics subsequently reproduced upstream Valijson
issue 124 while destroying the shared empty `Subschema`: its default
constructor did not explicitly disengage three C++14 compatibility optionals.
Patch `0004-replace-subschema-metadata-optionals.patch` removes the vulnerable
compatibility optionals from `Subschema` metadata while preserving behavior,
and a patterned-allocation regression test locks the boundary. After the
correction, full MSVC Debug,
MSVC RelWithDebInfo, and optimized Clang suites each pass 505/505 locally.

## Offline and package gates

Fresh `cmake_final_offline_*` trees configured, built, and installed x64,
Win32, and ARM64 in Debug and RelWithDebInfo while `HTTP_PROXY`,
`HTTPS_PROXY`, and `ALL_PROXY` pointed at unreachable `127.0.0.1:9`.
No JSON dependency was located or downloaded.

Each of the six installed SDK packages contains:

- 44 files;
- 38 public headers;
- the configuration-specific library and PDB;
- the SDK license, current third-party notice, and separately named
  nlohmann/json and Valijson licenses.

All six package scans report zero `RapidJsonUtils.h`, `json.hpp`, or
`json_fwd.hpp` filenames and zero RapidJSON, nlohmann/json, or Valijson names
in public headers. No private JSON dependency headers or CMake targets are
installed.

## Source, workflow, and formatting gates

- Shipped source/build/test/workflow scan: zero RapidJSON or
  `RapidJsonUtils` matches.
- `External/RapidJSON`: absent.
- Seven workflow YAML files: parsed successfully with PyYAML.
- `git diff --check`: passed for the complete worktree and workflows.
- Benchmark review: accepted and documented in `benchmark-comparison.md`;
  baseline and candidate each contain 990 successful samples.

Historical planning, migration, and benchmark-baseline evidence may retain
the old vendor name as explicitly allowed by REQ-DEP-12.

## CI-only matrix

The committed workflows define Debug and RelWithDebInfo gates for:

- Linux and macOS tests, package scans, and installed consumers;
- iOS device and 64-bit simulator build/install;
- Android `armeabi-v7a`, `arm64-v8a`, and `x86_64` build/install;
- Linux Clang ASAN/UBSAN full and malformed/deep focused suites; and
- Windows x64, Win32, and ARM64 package/consumer coverage.

Remote run:
[`33579694425`](https://github.com/SergioRZMasson/glTF-SDK/actions/runs/33579694425),
`completed/success`, 21/21 jobs.

## Commit and remote evidence

- `1516cd6` — Vendor private JSON dependencies
- `c9e8b63` — Add JSON migration baselines
- `d56dceb` — Replace RapidJSON with ordered JSON
- `bec4ce5` — Expand release validation matrix
- `0009ca0` — Document the 2.0 JSON migration
- `a8f1ee7` — Fix cross-platform validation failures
- `8e6c8a0` — Capture optimized Linux crash diagnostics
- `aff2ab2` — Initialize Valijson subschema optionals
- `3d1c3ee` — Construct Valijson optionals disengaged
- `d28ffe6` — Replace vulnerable Valijson metadata optionals

All commits contain the required Copilot trailers. The fix-forward history is
retained because release instructions prohibit history rewriting.

- Local implementation SHA:
  `d28ffe663fe9aaabdd6b34fc19fb834727d938c2`
- Verified remote implementation SHA:
  `d28ffe663fe9aaabdd6b34fc19fb834727d938c2`
- Push: normal, no force, only `Release/2.0.0`
- Required CI: 21/21 passed in run `33579694425`
- Final task-board/evidence closure: this docs-only commit; its pushed ref and
  CI conclusion are verified in the execution report.
