# Task 3.2: Validate bundled graph, flags, diagnostics, and CWE regressions

## Goal

Prove all 33 schemas and existing public schema behavior through the new session.

## Requirements addressed

REQ-API-10, REQ-SCH-2, REQ-SCH-3, REQ-SCH-8, REQ-SCH-9, REQ-SCH-10, REQ-SCH-11, REQ-SCH-12, REQ-TST-6

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Task 3.1 provides production session.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `GLTFSDK/Source/SchemaValidation.cpp` — Finalize deterministic pointer/keyword selection.
- `GLTFSDK/Source/Schema.cpp` — Retain URI/flag substitution integration.
- `GLTFSDK.Test/Source/JsonSchemaTests.cpp` — Add all-schema/root/flag/alias/diagnostic/error/CWE tests.
- `GLTFSDK.Test/Source/DeserializeTests.cpp` — Retain exact diagnostic/CWE assertions.

## Implementation steps

1. Enumerate GetDefaultSchemaUriMap and strictly parse/compile all 33 Draft-04 schemas.
2. Validate known-valid/invalid root documents and every graph-used assertion keyword.
3. Test every SchemaFlags substitution, shared 0x80 sparse alias, and Draft-04 treatment of {}.
4. Convert context to escaped URI-fragment JSON Pointer and select first deepest concrete-keyword error deterministically.
5. Preserve exact asserted minimum/dependencies messages and non-crashing missing-dependent-property cases.
6. Confirm DisableSchemaRoot never bypasses strict parse/semantic checks.

## Targeted validation commands

- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer --config Debug --target GLTFSDK.Test`
- `E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer\GLTFSDK.Test\Debug\GLTFSDK.Test.exe --gtest_filter=JsonSchemaTests.Bundled*:JsonSchemaTests.Flags*:JsonSchemaTests.Diagnostics*:DeserializeTests.DeserializeFail_Negative*:DeserializeTests.DeserializeFail_MissingDependent*`

## Acceptance criteria

- [ ] All bundled schemas/root graph pass expected corpus.
- [ ] Flags, exact diagnostics, failures, CWE cases pass.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Revert graph integration/tests as one gate; retain underlying session.

## Expected artifacts

- Bundled schema/flag/diagnostic tests/evidence.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
