# Task 3.5: Expand strict, numeric, and semantic deserializer regressions

## Goal

Lock the complete migration risk matrix through public APIs and schema-disabled paths.

## Requirements addressed

REQ-PARSE-3, REQ-PARSE-4, REQ-PARSE-5, REQ-PARSE-7, REQ-PARSE-8, REQ-PARSE-9, REQ-PARSE-10, REQ-TST-2, REQ-TST-3

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Task 3.4 switches production Deserialize.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `GLTFSDK.Test/Source/DeserializeTests.cpp` — Add semantic/type/range regressions.
- `GLTFSDK.Test/Source/GLTFTests.cpp` — Complete public strict/BOM matrix.
- `GLTFSDK.Test/Source/JsonTests.cpp` — Retain isolated parser cases.

## Implementation steps

1. Test comments, trailing commas, malformed, duplicates, invalid UTF-8, non-finite, depths 256/257, compact/whitespace BOM, both public forms.
2. Repeat duplicate/UTF-8/depth/type cases with DisableSchemaRoot.
3. Cover signed/unsigned 32/64, size_t, fractional integer, overflow, float/double boundaries, exponent, negative zero where fields permit.
4. Retain all existing wrong-object/array/fixed-length/non-numeric regressions and exact contracted exceptions.
5. Add representative unique-member extension/extras round trips.

## Targeted validation commands

- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer --config Debug --target GLTFSDK.Test`
- `E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer\GLTFSDK.Test\Debug\GLTFSDK.Test.exe --gtest_filter=DeserializeTests.*:GLTFTests.UnicodeByteOrderMark*:JsonTests.Strict*:JsonTests.Numeric*`

## Acceptance criteria

- [ ] Full strict/public/semantic/numeric matrix passes for string/stream.
- [ ] Schema-disabled paths remain safe; no vendor exception leaks.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Remove only new regressions if production slice reverts; keep pre-existing tests.

## Expected artifacts

- Expanded public deserializer suite.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
