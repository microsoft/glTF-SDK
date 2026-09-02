# Task 5.5: Remove RapidJSON and scan the shipped tree

## Goal

Delete the old vendor only after all replacement gates pass and prove the final tree/package is clean.

## Requirements addressed

REQ-DEP-11, REQ-DEP-12, REQ-DEP-13, REQ-API-1, REQ-BLD-8, REQ-BLD-9, REQ-TST-9

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Production/tests migrated; build/CI/benchmark review/docs ready.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `External/RapidJSON/**` — Delete downloader/templates/patches/directory.
- `GLTFSDK/Inc/GLTFSDK/RapidJsonUtils.h` — Delete public header.
- `CMakeLists.txt` — Delete discovery/fallback/public includes/install.
- `.github/workflows/windows.yml` — Delete RapidJSON-only policy workaround.
- `GLTFSDK.Test/Source/**/*.cpp` — Remove remaining current-code dependency references.
- `thirdPartyNotices.txt` — Confirm obsolete entries gone.

## Implementation steps

1. Resolve all production/test/build/current-notice uses before deleting dependency/header.
2. Delete all three patches/download logic/public include propagation/Built Out install.
3. Remove Windows policy workaround now and re-run Windows.
4. Delete only known generated Built\Out\RapidJSON directory; never git clean.
5. Scan excluding .git, Built, historical .ai; current docs may name RapidJSON only for migration/removal.
6. Verify no shipped source/public header/build/workflow/test/package/current notice depends on vendor.

## Targeted validation commands

- `rg -n -i "rapidjson|RapidJsonUtils" E:\Base3D\glTF-SDK\CMakeLists.txt E:\Base3D\glTF-SDK\External E:\Base3D\glTF-SDK\GLTFSDK E:\Base3D\glTF-SDK\GLTFSDK.Test E:\Base3D\glTF-SDK\GLTFSDK.Samples E:\Base3D\glTF-SDK\.github E:\Base3D\glTF-SDK\thirdPartyNotices.txt`
- `cmake -S E:\Base3D\glTF-SDK -B E:\Base3D\glTF-SDK\Built\Int\cmake_no_rapidjson_x64 -G "Visual Studio 18 2026" -A x64`
- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_no_rapidjson_x64 --target install --config Debug`
- `E:\Base3D\glTF-SDK\Built\Out\windows_x64\Debug\GLTFSDK.Test\GLTFSDK.Test.exe --gtest_filter=JsonTests.*:JsonSchemaTests.*:DeserializeTests.*:SerializeTests.*:ExtensionsTests.*:GLTFExtrasDocumentTests.*`

## Acceptance criteria

- [ ] Dependency/header/build/install/workaround removed; focused tests pass clean-configure.
- [ ] Shipped/package/current notices contain no dependency/reference.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Revert the single removal commit if late gate fails; no runtime fallback.

## Expected artifacts

- RapidJSON-free tree/package/scan evidence.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
