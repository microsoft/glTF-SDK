# Task 3.1: Implement Draft-04 validation session and resolver

## Goal

Replace RapidJSON schema compilation with per-call Valijson ordered-json validation and stable failures.

## Requirements addressed

REQ-SCH-1, REQ-SCH-4, REQ-SCH-5, REQ-SCH-6, REQ-SCH-7, REQ-SCH-10, REQ-SCH-14, REQ-SCH-15

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Tasks 1.6-1.8 corrected Valijson; Task 2.7 established validation boundaries.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `GLTFSDK/Source/Internal/JsonSchema.h` — Define private session operations.
- `GLTFSDK/Source/SchemaValidation.cpp` — Implement Draft4ValidationSession/caches/resolution/translation.
- `GLTFSDK.Test/Source/JsonSchemaTests.cpp` — Test session/locator/cache/URI/cycle/failures.

## Implementation steps

1. Create fresh per-call session owning locator, root URI, parsed schema documents, canonical caches, and resolving states; no static cache.
2. Copy locator content immediately, reject null locator/content, parse schemas with strict parser, fetch each canonical URI once.
3. Compile using SchemaParser::kDraft4, corrected ordered adapter, exceptions, strong types, strict date/time.
4. Resolve fragments inside cached documents and distinguish locator, missing URI, malformed JSON/schema, invalid ref/fragment/cycle, and invalid instance.
5. Accept already-parsed manifest privately so normal Deserialize will not reparse.

## Targeted validation commands

- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer --config Debug --target GLTFSDK.Test`
- `E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer\GLTFSDK.Test\Debug\GLTFSDK.Test.exe --gtest_filter=JsonSchemaTests.Session*:JsonSchemaTests.Resolver*:JsonSchemaTests.Errors*`

## Acceptance criteria

- [ ] Sessions are locator-isolated and terminate safely for repeats/cycles.
- [ ] Vendor failures map to stable SDK categories; private path uses original DOM.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Revert session/tests atomically; do not mix validators.

## Expected artifacts

- Draft4ValidationSession and resolver/error tests.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
