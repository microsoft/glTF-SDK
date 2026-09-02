# Task 3.3: Add official Draft-04 conformance suite

## Goal

Vendor and run official cases for glTF-used keywords and reference behavior.

## Requirements addressed

REQ-SCH-3, REQ-SCH-13, REQ-SCH-14, REQ-TST-6

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Tasks 3.1-3.2 pass SDK-focused validation tests.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `GLTFSDK.Test/Resources/JSON-Schema-Test-Suite/UPSTREAM.md` — Pin source commit/files/hashes/license/update.
- `GLTFSDK.Test/Resources/JSON-Schema-Test-Suite/tests/draft4/` — Copy relevant cases.
- `GLTFSDK.Test/Resources/JSON-Schema-Test-Suite/remotes/` — Copy referenced remotes.
- `GLTFSDK.Test/Source/JsonSchemaConformanceTests.cpp` — Execute cases through SDK session.
- `thirdPartyNotices.txt` — Confirm suite notice remains accurate.

## Implementation steps

1. Pin immutable official JSON-Schema-Test-Suite commit and copy complete relevant Draft-04 keyword/ref files/remotes.
2. Add focused nested path, ../, external fragment, id scope, repeat, legal recursion, malformed cycle fixtures if official coverage is insufficient.
3. Run schema/data/valid expectations with Draft-04 strong typing and readable case descriptions.
4. Do not skip, disable, reinterpret, or raise C++ standard for failing refs.
5. Record provenance/license/update steps.

## Targeted validation commands

- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer --config Debug --target GLTFSDK.Test`
- `E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer\GLTFSDK.Test\Debug\GLTFSDK.Test.exe --gtest_filter=JsonSchemaConformanceTests.*`

## Acceptance criteria

- [ ] Every selected official/ref case passes.
- [ ] Fixture provenance and license are reproducible.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Remove conformance source/resources/provenance together; never weaken cases.

## Expected artifacts

- Pinned fixtures, runner, passing evidence.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
