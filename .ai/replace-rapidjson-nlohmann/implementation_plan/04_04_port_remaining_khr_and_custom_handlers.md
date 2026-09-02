# Task 4.4: Port remaining KHR paths and custom handlers

## Goal

Complete ExtensionsKHR migration and remove test-side vendor DOM use.

## Requirements addressed

REQ-API-1, REQ-API-4, REQ-PARSE-11, REQ-SER-3, REQ-SER-6, REQ-TST-7

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Task 4.3 ports shared helpers/material extensions.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `GLTFSDK/Source/ExtensionsKHR.cpp` — Port Draco, mesh GPU instancing, texture transform; finish removal.
- `GLTFSDK.Test/Source/GLTFExtensionsTests.cpp` — Replace vendor construction/parsing and use string validator.

## Implementation steps

1. Port Draco, MeshGPUInstancing, and TextureTransform Serialize/Deserialize pairs with checked object/array/numeric helpers.
2. Preserve texture sizes/texCoord, Draco attributes, instancing attributes, and handler registration.
3. Rewrite SerializeTestExtension without SDK vendor APIs; DeserializeTestExtension calls public string validation and vendor-neutral test parsing.
4. Run every extension test including locators, handlers, malformed input, collisions, nested extras, round trips.
5. Verify implementation/test contain no RapidJSON references.

## Targeted validation commands

- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer --config Debug --target GLTFSDK.Test`
- `E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer\GLTFSDK.Test\Debug\GLTFSDK.Test.exe --gtest_filter=ExtensionsTests.*`
- `rg -n "rapidjson|RapidJsonUtils" E:\Base3D\glTF-SDK\GLTFSDK\Source\ExtensionsKHR.cpp E:\Base3D\glTF-SDK\GLTFSDK.Test\Source\GLTFExtensionsTests.cpp`

## Acceptance criteria

- [ ] All KHR/custom tests pass; both files RapidJSON-free.
- [ ] Extension locators reference bundled schemas through new API.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Revert remaining extension/test slice; retain material slice only if independently buildable.

## Expected artifacts

- Fully migrated ExtensionsKHR and tests.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
