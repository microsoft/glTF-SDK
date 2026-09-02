# Task 5.3: Measure candidate and review benchmark deltas

## Goal

Run the baseline-equivalent harness, report all deltas, and close the human performance gate.

## Requirements addressed

REQ-PERF-1, REQ-PERF-2, REQ-PERF-3, REQ-PERF-4, REQ-PERF-5, REQ-PERF-6, REQ-REL-2

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Task 1.3 baseline; production optimized build complete.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `Benchmarks/Source/JsonBenchmarks.cpp` — Update implementation hooks without changing workloads.
- `Benchmarks/RunBenchmarks.ps1` — Run candidate/calculate deltas.
- `docs/release-evidence/replace-rapidjson-nlohmann/benchmark-candidate.md` — Record candidate data.
- `docs/release-evidence/replace-rapidjson-nlohmann/benchmark-comparison.md` — Record all deltas/review disposition.

## Implementation steps

1. Use identical machine/generator/compiler/architecture/configuration/flags/assets/samples/aggregation as baseline.
2. Measure separated and end-to-end workloads including malformed/deep/extension/float-heavy.
3. Calculate absolute/percentage deltas for parse, validation, write, memory, binary, clean/incremental compile; explain allocation omissions.
4. Investigate material regressions without changing workloads; rerun both points if methodology changes.
5. Record explicit human acceptance; no numeric threshold exists, so unreviewed report blocks push.

## Targeted validation commands

- `cmake -S E:\Base3D\glTF-SDK -B E:\Base3D\glTF-SDK\Built\Int\cmake_benchmark_candidate -DENABLE_UNIT_TESTS=OFF -DENABLE_SAMPLES=OFF -DENABLE_BENCHMARKS=ON`
- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_benchmark_candidate --config RelWithDebInfo --target GLTFSDK.JsonBenchmarks`
- `powershell -ExecutionPolicy Bypass -File E:\Base3D\glTF-SDK\Benchmarks\RunBenchmarks.ps1 -BuildDir E:\Base3D\glTF-SDK\Built\Int\cmake_benchmark_candidate -Label nlohmann-valijson-2.0.0 -Baseline E:\Base3D\glTF-SDK\docs\release-evidence\replace-rapidjson-nlohmann\benchmark-baseline.md`

## Acceptance criteria

- [ ] All required values/deltas recorded reproducibly.
- [ ] Named review disposition exists before removal/push.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

If unacceptable, roll back relevant dependency/validation/serializer checkpoint; never hide regressions.

## Expected artifacts

- Candidate results, comparison, review outcome.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
