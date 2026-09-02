# Task 2.1: Create private JSON layer and build wiring

## Goal

Introduce one private ordered DOM boundary while both vendors remain buildable.

## Requirements addressed

REQ-DEP-9, REQ-API-2, REQ-PARSE-1, REQ-BLD-1, REQ-BLD-10

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Tasks 1.4-1.8 provide corrected private targets.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `GLTFSDK/Source/Internal/Json.h` — Declare JsonValue and narrow operations.
- `GLTFSDK/Source/Json.cpp` — Define operations and 3.12.0 assertions.
- `GLTFSDK/CMakeLists.txt` — Link dependencies privately.
- `GLTFSDK.Test/Source/JsonTests.cpp` — Add foundational tests.

## Implementation steps

1. Include nlohmann only in private source files and assert version 3.12.0.
2. Declare parse, predicate, member, scalar, ordered construction, subtree, pointer, and writer families without emulating vendor iterators/allocators.
3. Shape APIs to avoid unchecked operator[] insertion, implicit conversion, and unchecked get<T>() in production callers.
4. Link nlohmann_json::nlohmann_json and ValiJSON::valijson PRIVATE; install/export neither.
5. Keep RapidJSON active until vertical slices pass.

## Targeted validation commands

- `cmake -S E:\Base3D\glTF-SDK -B E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer -DENABLE_SAMPLES=OFF`
- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer --config Debug --target GLTFSDK.Test`
- `E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer\GLTFSDK.Test\Debug\GLTFSDK.Test.exe --gtest_filter=JsonTests.*`

## Acceptance criteria

- [ ] Only private files name nlohmann/Valijson and links are private.
- [ ] Project still builds with RapidJSON active.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Remove private layer/tests/links; leave dependencies and RapidJSON.

## Expected artifacts

- Private JSON header/source, CMake wiring, tests.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
