# Task 1.8: Add structured validation keywords to Valijson

## Goal

Carry failing keywords through results for deterministic SDK diagnostics.

## Requirements addressed

REQ-SCH-10, REQ-SCH-11, REQ-SCH-12

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Tasks 1.6-1.7 provide adapter/resolver.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `External/Valijson/include/valijson/validation_results.hpp` — Add structured keyword field.
- `External/Valijson/include/valijson/validation_visitor.hpp` — Pass keywords at failures.
- `External/Valijson/include/valijson/constraints/**` — Supply originating keyword names.
- `External/Valijson/patches/0003-structured-validation-keywords.patch` — Record correction.
- `GLTFSDK.Test/Source/JsonSchemaDiagnosticsTests.cpp` — Assert context/keyword/CWE safety.

## Implementation steps

1. Extend private Error/pushError without public leakage.
2. Thread concrete minimum/dependencies/required/type/composition keywords instead of parsing English text.
3. Retain descriptions for debugging but make structured keyword authoritative.
4. Test deterministic deepest concrete errors and missing-dependent-property non-crash behavior.
5. Generate patch and update reproducibility instructions.

## Targeted validation commands

- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_valijson_probe --config Debug --target GLTFSDK.Test`
- `E:\Base3D\glTF-SDK\Built\Int\cmake_valijson_probe\GLTFSDK.Test\Debug\GLTFSDK.Test.exe --gtest_filter=JsonSchemaDiagnosticsTests.*`

## Acceptance criteria

- [ ] Errors used by SDK diagnostics carry explicit keyword/context.
- [ ] CWE dependency cases fail cleanly.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Reverse patch 0003/tests; retain previous gates.

## Expected artifacts

- Diagnostic correction, patch, focused tests.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
