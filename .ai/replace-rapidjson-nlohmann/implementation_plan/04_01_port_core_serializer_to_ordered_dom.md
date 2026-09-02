# Task 4.1: Port core serializer to ordered_json

## Goal

Replace RapidJSON construction/writing while preserving root/member order and raw subtree structure.

## Requirements addressed

REQ-API-3, REQ-SER-1, REQ-SER-2, REQ-SER-3, REQ-SER-4, REQ-SER-5, REQ-SER-6, REQ-SER-7, REQ-SER-8

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Tasks 2.3-2.4 provide construction/writer; Task 3.4 stores strict extension/extras strings.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `GLTFSDK/Source/Serialize.cpp` — Replace all RapidJSON construction/allocation/copy/writer calls.
- `GLTFSDK/Source/Internal/Json.h` — Add only narrow missing serializer helpers.

## Implementation steps

1. Port SerializePropertyExtensions/Extras, SerializeIndexedContainer, every Serialize* function, SerializeStringSet, and CreateJsonDocument.
2. Preserve exact call order that determines output member order; updating members must not move position.
3. Strictly parse registered/unregistered extension/extras strings into ordered subtrees and move them into parent; no maps/default json.
4. Use checked index conversions, finite floats, and one final private writer selected by SerializeFlags.
5. Remove RapidJSON from Serialize.cpp only after focused outputs pass.

## Targeted validation commands

- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer --config Debug --target GLTFSDK.Test`
- `E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer\GLTFSDK.Test\Debug\GLTFSDK.Test.exe --gtest_filter=SerializeTests.*:GLTFTests.*RoundTrip*`
- `rg -n "rapidjson|RapidJsonUtils" E:\Base3D\glTF-SDK\GLTFSDK\Source\Serialize.cpp`

## Acceptance criteria

- [ ] Core serializer is RapidJSON-free and preserves call/member order.
- [ ] Existing compact/pretty/round-trip tests pass.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Revert Serialize.cpp as one slice; validation/deserialization may remain migrated.

## Expected artifacts

- Ordered-DOM core serializer.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
