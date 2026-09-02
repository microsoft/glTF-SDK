# Task 3.4: Port core deserializer to the single ordered DOM

## Goal

Atomically switch parsing, validation, and typed traversal without reparsing/fallback.

## Requirements addressed

REQ-API-3, REQ-PARSE-1, REQ-PARSE-2, REQ-PARSE-6, REQ-PARSE-8, REQ-PARSE-10, REQ-PARSE-11, REQ-SCH-15, REQ-TST-1

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Tasks 2.2-2.4 provide JSON operations; Tasks 3.1-3.3 prove validation.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `GLTFSDK/Source/Deserialize.cpp` — Replace RapidJSON in every Parse* and entry point.
- `GLTFSDK/Source/Internal/Json.h` — Add only narrow missing operations.
- `GLTFSDK/Source/SchemaValidation.cpp` — Expose private DOM validation to Deserialize.

## Implementation steps

1. Change DeserializeInternal, all Parse* functions, and DeserializeToIndexedContainer to const JsonValue& and checked helpers.
2. Both public string/stream overloads call shared strict parse, private validation, then traverse that exact DOM.
3. Write extension/extras subtrees to existing string storage without creating a second validation/traversal DOM.
4. Preserve explicit object/array/fixed-size/required/numeric/enum/index checks with schema disabled.
5. Remove RapidJSON from Deserialize.cpp only when the atomic slice passes; add no fallback.

## Targeted validation commands

- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer --config Debug --target GLTFSDK.Test`
- `E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer\GLTFSDK.Test\Debug\GLTFSDK.Test.exe --gtest_filter=DeserializeTests.*:GLTFTests.UnicodeByteOrderMark*:GLTFTests.SchemaFlags*`
- `rg -n "rapidjson|RapidJsonUtils" E:\Base3D\glTF-SDK\GLTFSDK\Source\Deserialize.cpp`

## Acceptance criteria

- [ ] Deserialize parses once and shares DOM across validation/traversal.
- [ ] Focused tests pass and Deserialize.cpp is RapidJSON-free.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Revert Deserialize/private bridge atomically; never leave different validation/traversal DOMs.

## Expected artifacts

- Ordered-DOM deserializer vertical slice.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
