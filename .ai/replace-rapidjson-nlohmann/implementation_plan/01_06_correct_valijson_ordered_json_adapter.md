# Task 1.6: Correct Valijson for ordered_json

## Goal

Make Valijson use the authoritative ordered DOM without conversion.

## Requirements addressed

REQ-DEP-8, REQ-PARSE-1, REQ-SCH-15

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Tasks 1.4-1.5 provide exact sources.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `External/Valijson/include/valijson/adapters/nlohmann_json_adapter.hpp` — Replace hard-coded default json consistently.
- `External/Valijson/patches/0001-ordered-nlohmann-adapter.patch` — Record reproducible correction.
- `GLTFSDK.Test/Source/JsonSchemaAdapterTests.cpp` — Test order/categories/no conversion.

## Implementation steps

1. Define one adapter-local ordered document type and use it for iterators, containers, values, frozen values, singletons, constructors, and AdapterTraits.
2. Ensure unsigned values remain representable without signed/double loss; correct adapter accessors if required.
3. Test insertion-order iteration and use of the same ordered type for parsing/validation.
4. Generate patch against pristine v1.0.6 and document reapplication in UPSTREAM.md.

## Targeted validation commands

- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_valijson_probe --config Debug --target GLTFSDK.Test`
- `E:\Base3D\glTF-SDK\Built\Int\cmake_valijson_probe\GLTFSDK.Test\Debug\GLTFSDK.Test.exe --gtest_filter=JsonSchemaAdapterTests.*`

## Acceptance criteria

- [ ] Valijson directly accepts ordered_json preserving order/categories.
- [ ] No production/test conversion to default nlohmann::json remains.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Reverse patch 0001 and remove focused tests; pristine source stays.

## Expected artifacts

- Applied correction, patch, adapter tests.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
