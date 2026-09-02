# Task 4.2: Expand serializer golden, numeric, UTF-8, and determinism tests

## Goal

Prove byte formatting and semantic edge compatibility after serializer port.

## Requirements addressed

REQ-SER-2, REQ-SER-3, REQ-SER-4, REQ-SER-5, REQ-SER-6, REQ-SER-8, REQ-TST-4

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Task 4.1 switches core serialization.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `GLTFSDK.Test/Source/SerializeTests.cpp` — Add output/order/numeric/UTF-8/repeatability cases.
- `GLTFSDK.Test/Resources/JsonMigrationBaseline/` — Retain/update only approved goldens.
- `docs/release-evidence/replace-rapidjson-nlohmann/serialization-differences.md` — Record approved non-golden lexical differences.

## Implementation steps

1. Assert existing pretty goldens byte-for-byte and add representative compact gates.
2. Test order for core, registered/unregistered extensions, extras, and repeated writes.
3. Cover 1.0, signed/unsigned 64-bit bounds, negative zero, exponent, precision, non-finite rejection.
4. Cover escaping, Unicode output, invalid UTF-8 failure.
5. Round-trip output through public Deserialize; document only reviewed differences outside established goldens.

## Targeted validation commands

- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer --config Debug --target GLTFSDK.Test`
- `E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer\GLTFSDK.Test\Debug\GLTFSDK.Test.exe --gtest_filter=SerializeTests.*:JsonTests.Writer*`

## Acceptance criteria

- [ ] Established goldens remain byte-exact; repeated output identical.
- [ ] Numeric/UTF-8/escaping edges pass or have reviewed lexical note.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Revert new tests/evidence; never rewrite goldens to hide defects.

## Expected artifacts

- Expanded serializer suite and lexical evidence.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
