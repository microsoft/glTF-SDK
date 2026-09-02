# Task 2.6: Replace ExtrasDocument with move-only vendor-neutral PImpl

## Goal

Remove public DOM while preserving typed root/member/pointer behavior.

## Requirements addressed

REQ-API-1, REQ-API-2, REQ-API-6, REQ-API-7, REQ-API-8, REQ-API-9, REQ-PTR-1, REQ-PTR-2, REQ-PTR-3, REQ-TST-5

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Tasks 2.2-2.5 provide all private operations.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `GLTFSDK/Inc/GLTFSDK/ExtrasDocument.h` — Declare PImpl/lifetime/ToJson/HasMember/vendor-free dispatch.
- `GLTFSDK/Source/ExtrasDocument.cpp` — Own JsonValue and define exported operations.
- `GLTFSDK.Test/Source/ExtrasDocumentTests.cpp` — Migrate/add API/type/move/pointer tests.
- `GLTFSDK/CMakeLists.txt` — Build/install source/header normally.

## Implementation steps

1. Store only unique_ptr<Impl>; exported out-of-line destructor/noexcept moves; delete copies; support moved-from defaults/lazy recreation.
2. Add strict const char*/string constructors and null C-string error.
3. Implement ToJson and HasMember; non-object/missing false, null name error.
4. Use C++14 traits/tag dispatch for supported types and const char*/literal setters without size_t duplicate-specialization bugs.
5. Preserve numeric assignment category, member order, defaults, pointers; unsupported templates fail clearly.
6. Replace test Serialize(GetDocument()) calls with ToJson().

## Targeted validation commands

- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer --config Debug --target GLTFSDK.Test`
- `E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer\GLTFSDK.Test\Debug\GLTFSDK.Test.exe --gtest_filter=GLTFExtrasDocumentTests.*`

## Acceptance criteria

- [ ] Installed header is vendor-free and GetDocument absent.
- [ ] All types/moves/ToJson/HasMember/default/category/pointer tests pass.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Revert header/source/tests as one public API unit.

## Expected artifacts

- Vendor-neutral ExtrasDocument API/implementation/tests.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
