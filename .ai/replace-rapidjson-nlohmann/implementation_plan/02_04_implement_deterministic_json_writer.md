# Task 2.4: Implement deterministic compact and pretty JSON writer

## Goal

Define one ordered writer with strict UTF-8 and SDK exception translation.

## Requirements addressed

REQ-SER-2, REQ-SER-3, REQ-SER-4, REQ-SER-5, REQ-SER-7, REQ-SER-8, REQ-TST-4

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Tasks 2.2-2.3 provide values/construction.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `GLTFSDK/Source/Internal/Json.h` — Declare write operations.
- `GLTFSDK/Source/Json.cpp` — Implement formatting/scalar policy.
- `GLTFSDK.Test/Source/JsonTests.cpp` — Test formatting/order/numbers/UTF-8/repeatability.

## Implementation steps

1. Emit compact with no insignificant whitespace and pretty with four spaces, LF, one colon space, no trailing whitespace.
2. Traverse ordered containers deterministically and require repeated byte identity.
3. Use strict UTF-8 scalar serialization with ensure_ascii=false and reject invalid output strings.
4. Lock 1.0, integer bounds, negative zero, exponent/precision, and escaping behavior.
5. Translate serialization exceptions to GLTFException.

## Targeted validation commands

- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer --config Debug --target GLTFSDK.Test`
- `E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer\GLTFSDK.Test\Debug\GLTFSDK.Test.exe --gtest_filter=JsonTests.Writer*`

## Acceptance criteria

- [ ] Compact/pretty policy and approved goldens are byte-exact.
- [ ] Numeric, UTF-8, escaping, repeatability pass.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Revert writer/tests; retain parser/access.

## Expected artifacts

- Deterministic writer and compatibility tests.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
