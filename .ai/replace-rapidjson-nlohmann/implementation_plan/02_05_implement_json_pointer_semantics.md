# Task 2.5: Implement JSON Pointer lookup and set/create

## Goal

Provide vendor-neutral RFC 6901 behavior with strict indices, creation, category checks, and depth.

## Requirements addressed

REQ-PTR-1, REQ-PTR-2, REQ-PTR-3, REQ-PTR-4, REQ-PTR-5, REQ-PTR-6, REQ-TST-5

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Tasks 2.2-2.4 provide values, mutation, depth, writer.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `GLTFSDK/Source/Internal/Json.h` — Declare pointer operations.
- `GLTFSDK/Source/Json.cpp` — Implement tokenization/lookup/create.
- `GLTFSDK.Test/Source/JsonTests.cpp` — Add pointer matrix.

## Implementation steps

1. Treat empty pointer as root; decode only ~0/~1 and reject other tilde escapes.
2. Accept canonical decimal array indices only; reject signs, leading zero, overflow, and dash.
3. Lookup never creates and reports missing.
4. Set/create chooses array for next numeric token, otherwise object; grow arrays with null placeholders.
5. Allow null adoption, reject incompatible traversal/replacement, preserve values, enforce depth 256.

## Targeted validation commands

- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer --config Debug --target GLTFSDK.Test`
- `E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer\GLTFSDK.Test\Debug\GLTFSDK.Test.exe --gtest_filter=JsonTests.Pointer*`

## Acceptance criteria

- [ ] Root/object/array/escaped/missing/create/growth cases pass.
- [ ] Invalid syntax/index/type/depth throws SDK exceptions.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Revert pointer helpers/tests only.

## Expected artifacts

- Pointer implementation and tests.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
