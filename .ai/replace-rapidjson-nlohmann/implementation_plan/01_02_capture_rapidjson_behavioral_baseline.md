# Task 1.2: Capture RapidJSON behavioral and test baseline

## Goal

Freeze current outputs, failures, and test results before replacement.

## Requirements addressed

REQ-TST-1, REQ-TST-4, REQ-SCH-11, REQ-REL-2

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Task 1.1 confirms the untouched upstream base.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `docs/release-evidence/replace-rapidjson-nlohmann/baseline-tests.md` — Record commands, toolchains, outcomes, exact diagnostics, and known defects.
- `GLTFSDK.Test/Resources/JsonMigrationBaseline/` — Store stable compact/pretty and round-trip fixtures.
- `GLTFSDK.Test/Source/JsonBaselineTests.cpp` — Assert differential fixtures while RapidJSON remains active.

## Implementation steps

1. Build the baseline in Debug and RelWithDebInfo and record generator/compiler/CMake identity.
2. Capture byte-exact compact/pretty outputs for default, extension-heavy, extras-heavy, and numeric documents.
3. Record exact schema diagnostics, BOM behavior including compact-stream loss, malformed categories, and sparse flag alias behavior.
4. Run the full suite and focused GLTFTests, DeserializeTests, SerializeTests, GLTFExtrasDocumentTests, and ExtensionsTests.
5. Label duplicate acceptance, invalid UTF-8 behavior, and compact-stream BOM loss as approved deltas, not compatibility goldens.

## Targeted validation commands

- `cmake -S E:\Base3D\glTF-SDK -B E:\Base3D\glTF-SDK\Built\Int\cmake_baseline_x64 -G "Visual Studio 18 2026" -A x64`
- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_baseline_x64 --target install --config Debug`
- `E:\Base3D\glTF-SDK\Built\Out\windows_x64\Debug\GLTFSDK.Test\GLTFSDK.Test.exe --gtest_output=xml:baseline.xml`
- `E:\Base3D\glTF-SDK\Built\Out\windows_x64\Debug\GLTFSDK.Test\GLTFSDK.Test.exe --gtest_filter=GLTFTests.*:DeserializeTests.*:SerializeTests.*:GLTFExtrasDocumentTests.*:ExtensionsTests.*`

## Acceptance criteria

- [ ] Byte-exact goldens and exact asserted diagnostics are recorded.
- [ ] Baseline full/focused results exist before production migration.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Remove only new fixtures/tests/evidence; do not alter baseline implementation.

## Expected artifacts

- Baseline report and differential fixtures.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
