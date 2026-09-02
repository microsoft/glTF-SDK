# Task 1.7: Correct Valijson Draft-04 URI and reference resolution

## Goal

Implement RFC-aware refs, fragments, id scopes, caching, and safe cycles before migration.

## Requirements addressed

REQ-SCH-1, REQ-SCH-4, REQ-SCH-5, REQ-SCH-6, REQ-SCH-7, REQ-SCH-13, REQ-SCH-14

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Task 1.6 ordered adapter; RapidJSON remains until this gate passes.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `External/Valijson/include/valijson/internal/uri.hpp` — Replace placeholder URI logic.
- `External/Valijson/include/valijson/schema_parser.hpp` — Propagate scope/fragments/id/repeats/cycles.
- `External/Valijson/include/valijson/internal/json_reference.hpp` — Adjust splitting/canonicalization if tests require.
- `External/Valijson/patches/0002-draft04-uri-and-reference-resolution.patch` — Record corrections.
- `GLTFSDK.Test/Source/JsonSchemaReferenceTests.cpp` — Test nested/parent/fragment/id/repeat/cycle/isolation.

## Implementation steps

1. Split refs into canonical document URI and RFC 6901 fragment; never send fragment to locator.
2. Resolve relative paths against current document directory with dot-segment normalization; absolute URI/URN replaces base.
3. Propagate fetched document URI and Draft-04 id scopes.
4. Cache per operation; register resolving subschema before children; reuse legal recursion and reject illegal cycles/missing fragments.
5. Pass nested, ../, fragment, id, repeat, recursive, and distinct-locator tests without skips.

## Targeted validation commands

- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_valijson_probe --config Debug --target GLTFSDK.Test`
- `E:\Base3D\glTF-SDK\Built\Int\cmake_valijson_probe\GLTFSDK.Test\Debug\GLTFSDK.Test.exe --gtest_filter=JsonSchemaReferenceTests.*`

## Acceptance criteria

- [ ] All required external-reference behaviors pass.
- [ ] No disabled/weakened reference test counts as evidence.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Reverse patch 0002/tests; do not begin production validation migration.

## Expected artifacts

- Resolver correction, patch, conformance tests.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
