# Task 1.3: Add benchmark harness and capture RapidJSON baseline

## Goal

Create reproducible dependency-free workloads and baseline measurements.

## Requirements addressed

REQ-PERF-1, REQ-PERF-2, REQ-PERF-3, REQ-PERF-4, REQ-PERF-5

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Task 1.2 establishes baseline fixtures and a known-good build.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `Benchmarks/CMakeLists.txt` — Define opt-in benchmark executable.
- `Benchmarks/Source/JsonBenchmarks.cpp` — Measure parse, validation, deserialize, write, serialize, and end-to-end.
- `Benchmarks/Assets/` — Store or manifest required workloads.
- `Benchmarks/RunBenchmarks.ps1` — Automate samples, memory, size, and build timing.
- `docs/release-evidence/replace-rapidjson-nlohmann/benchmark-baseline.md` — Record reproducible baseline data.

## Implementation steps

1. Keep ENABLE_BENCHMARKS off by default and add no benchmark dependency.
2. Separate parse, Draft-04 compile/validate, Deserialize schema on/off, write, public Serialize, and end-to-end workloads.
3. Cover small, large, extension-heavy, float-heavy, malformed, deep, and repeated operations with at least 30 timed samples after warm-up.
4. Record median, p95, peak resident memory, binary size, clean/incremental compile time, allocation counts when reproducible, asset hashes, and machine identity.
5. Use one controlled optimized configuration and retain raw/summarized results.

## Targeted validation commands

- `cmake -S E:\Base3D\glTF-SDK -B E:\Base3D\glTF-SDK\Built\Int\cmake_benchmark_baseline -DENABLE_UNIT_TESTS=OFF -DENABLE_SAMPLES=OFF -DENABLE_BENCHMARKS=ON`
- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_benchmark_baseline --config RelWithDebInfo --target GLTFSDK.JsonBenchmarks`
- `powershell -ExecutionPolicy Bypass -File E:\Base3D\glTF-SDK\Benchmarks\RunBenchmarks.ps1 -BuildDir E:\Base3D\glTF-SDK\Built\Int\cmake_benchmark_baseline -Label rapidjson-1.9.5`

## Acceptance criteria

- [ ] Every required workload and metric has reproducible baseline data.
- [ ] Normal builds are unchanged when benchmarks are disabled.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Remove benchmark target/assets/evidence as one unit; production behavior stays untouched.

## Expected artifacts

- Harness, assets, runner, and baseline report.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
