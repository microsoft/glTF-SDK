# Task 2.3: Implement checked scalar access and ordered construction

## Goal

Centralize safe extraction and mutation for deserialization, serialization, and extras.

## Requirements addressed

REQ-PARSE-8, REQ-PARSE-9, REQ-PARSE-10, REQ-SER-1, REQ-SER-4, REQ-SER-7, REQ-TST-3

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Task 2.2 supplies trusted values.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `GLTFSDK/Source/Internal/Json.h` — Declare predicates/member/array/scalar/construction helpers.
- `GLTFSDK/Source/Json.cpp` — Implement categories/ranges/integrality/finite insertion.
- `GLTFSDK.Test/Source/JsonTests.cpp` — Test numeric/type/order behavior.

## Implementation steps

1. Implement nullable and required member lookup, requiring object/array before traversal.
2. Support bool, signed/unsigned 32/64, size_t, float, double, string with exact architecture rules.
3. Reject floating-to-integer, negative-to-unsigned, overflow, and non-finite programmatic values.
4. Create/append ordered objects/arrays and update members without moving position.
5. Never route 64-bit integers through double; retain floating category for 1.0f.

## Targeted validation commands

- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer --config Debug --target GLTFSDK.Test`
- `E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer\GLTFSDK.Test\Debug\GLTFSDK.Test.exe --gtest_filter=JsonTests.Numeric*:JsonTests.Access*:JsonTests.Construction*`

## Acceptance criteria

- [ ] All scalar boundaries/wrong categories/defaults are tested.
- [ ] Defensive traversal and stable update order pass.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Revert access/construction helpers/tests; retain parser.

## Expected artifacts

- Checked access/mutation layer and numeric tests.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
