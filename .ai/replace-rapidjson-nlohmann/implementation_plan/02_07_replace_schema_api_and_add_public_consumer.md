# Task 2.7: Replace schema API and add installed consumer

## Goal

Expose string validation and prove installed consumers need no JSON dependency.

## Requirements addressed

REQ-API-4, REQ-API-5, REQ-API-10, REQ-API-11, REQ-TST-8, REQ-BLD-10

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Task 2.6 removes the other public DOM surface; corrected Valijson exists.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `GLTFSDK/Inc/GLTFSDK/SchemaValidation.h` — Declare vendor-free string validator.
- `GLTFSDK/Source/Internal/JsonSchema.h` — Declare private ordered-DOM validation boundary.
- `GLTFSDK/Source/SchemaValidation.cpp` — Add public parse/delegate boundary.
- `GLTFSDK.PublicConsumer.Test/CMakeLists.txt` — Configure against installed SDK only.
- `GLTFSDK.PublicConsumer.Test/Source/main.cpp` — Exercise changed public APIs.
- `CMakeLists.txt` — Register opt-in/post-install consumer path.

## Implementation steps

1. Declare ValidateDocumentAgainstSchema(string documentJson, string schemaUri, unique_ptr locator) and export it.
2. Keep ISchemaLocator, SchemaFlags values, GetDefaultSchemaLocator unchanged/vendor-neutral.
3. Public call parses once then delegates to private ordered-DOM overload; normal Deserialize will use private overload later.
4. Consumer includes Deserialize/Serialize/ExtrasDocument/SchemaValidation and links using only installed roots.
5. Scan installed headers for rapidjson, nlohmann, valijson.

## Targeted validation commands

- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_json_layer --config Debug --target install`
- `cmake -S E:\Base3D\glTF-SDK\GLTFSDK.PublicConsumer.Test -B E:\Base3D\glTF-SDK\Built\Int\public_consumer -DGLTFSDK_ROOT=E:\Base3D\glTF-SDK\Built\Out\windows_x64\Debug\GLTFSDK`
- `cmake --build E:\Base3D\glTF-SDK\Built\Int\public_consumer --config Debug`
- `rg -n "rapidjson|nlohmann|valijson" E:\Base3D\glTF-SDK\Built\Out\windows_x64\Debug\GLTFSDK\Inc`

## Acceptance criteria

- [ ] Old DOM validator absent; string API links from installed-only consumer.
- [ ] No public header/consumer dependency names a vendor.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Revert schema boundary/consumer together; do not leave dual public overloads.

## Expected artifacts

- String API, private boundary, installed consumer.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
