# Task 2.2: Implement strict parser and BOM policy

## Goal

Provide one string/stream parser with duplicate, UTF-8, and 256-depth enforcement.

## Requirements addressed

REQ-PARSE-2, REQ-PARSE-3, REQ-PARSE-4, REQ-PARSE-5, REQ-PARSE-6, REQ-PARSE-7, REQ-TST-2

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Task 2.1 provides parse declarations and ordered ownership.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `GLTFSDK/Source/Internal/Json.h` — Finalize policy declarations/depth constant.
- `GLTFSDK/Source/Json.cpp` — Implement shared bytes, BOM, duplicate/depth callback, UTF-8, exceptions.
- `GLTFSDK.Test/Source/JsonTests.cpp` — Add strict matrix.
- `GLTFSDK.Test/Source/GLTFTests.cpp` — Add compact-BOM regression.

## Implementation steps

1. Read streams once and call the same byte parser as strings.
2. Reject leading EF BB BF without IgnoreByteOrderMark; with flag remove exactly one leading BOM; never allow UTF-16/32 or embedded BOM.
3. Use strict nlohmann grammar with exceptions/comments disabled and per-object duplicate key sets.
4. Count root container as level 1; accept 256 and reject 257; reuse for schemas/extensions/extras.
5. Translate every vendor failure to stable SDK exceptions.

## Targeted validation commands

- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer --config Debug --target GLTFSDK.Test`
- `E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer\GLTFSDK.Test\Debug\GLTFSDK.Test.exe --gtest_filter=JsonTests.Strict*:GLTFTests.UnicodeByteOrderMark*`

## Acceptance criteria

- [ ] String/stream matrices agree for bytes and BOM flags.
- [ ] Malformed/comments/trailing/non-finite/duplicate/UTF-8/depth failures are SDK exceptions.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Revert parser/tests together; do not switch public Deserialize.

## Expected artifacts

- Strict parser and complete policy tests.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
