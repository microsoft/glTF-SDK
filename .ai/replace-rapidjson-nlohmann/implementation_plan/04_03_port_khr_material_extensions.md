# Task 4.3: Port KHR material extension JSON paths

## Goal

Migrate material extension serializers/deserializers and nested property/extras handling.

## Requirements addressed

REQ-PARSE-11, REQ-SER-3, REQ-SER-6, REQ-TST-7

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Tasks 3.4 and 4.1 provide shared ordered parse/write.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `GLTFSDK/Source/ExtensionsKHR.cpp` — Port shared helpers and material extension pairs.
- `GLTFSDK.Test/Source/GLTFExtensionsTests.cpp` — Migrate/run material round-trip and malformed cases.

## Implementation steps

1. Port RequireFixedSizeNumericArray, ParseExtensions/Extras/Property/TextureInfo and serializer counterparts to narrow helpers.
2. Port PBR specular-glossiness, unlit, clearcoat, volume, iridescence, transmission, sheen, and specular Serialize/Deserialize pairs.
3. Preserve default omission, member order, finite numeric checks, nested properties/extras, existing error categories.
4. Strictly parse handler strings and emit deterministic compact strings.
5. Retain fixed-size array/texture object defenses; keep remaining RapidJSON buildable until Task 4.4.

## Targeted validation commands

- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer --config Debug --target GLTFSDK.Test`
- `E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer\GLTFSDK.Test\Debug\GLTFSDK.Test.exe --gtest_filter=ExtensionsTests.*SpecGloss*:ExtensionsTests.*Clearcoat*:ExtensionsTests.*Volume*:ExtensionsTests.*Transmission*:ExtensionsTests.*Iridescence*:ExtensionsTests.*Sheen*:ExtensionsTests.*Specular*:ExtensionsTests.*DiffuseFactor*`

## Acceptance criteria

- [ ] All material extension direct/round-trip tests pass.
- [ ] Order/extras/registered extension behavior preserved.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Revert material/shared-helper portion coherently; no half-ported pairs.

## Expected artifacts

- Migrated KHR material paths/tests.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
