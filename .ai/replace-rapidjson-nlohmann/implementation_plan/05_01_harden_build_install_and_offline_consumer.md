# Task 5.1: Harden build, install, offline configure, and public consumer

## Goal

Make private dependencies hermetic and prove package outputs expose only SDK artifacts.

## Requirements addressed

REQ-DEP-5, REQ-DEP-9, REQ-DEP-13, REQ-API-11, REQ-BLD-1, REQ-BLD-2, REQ-BLD-9, REQ-BLD-10, REQ-TST-8

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

All production JSON paths use private dependencies.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `CMakeLists.txt` — Finalize dependency subdirectories/options and remove public propagation/install behavior, retaining RapidJSON until Task 5.5.
- `GLTFSDK/CMakeLists.txt` — Finalize private links/includes/source-private headers.
- `GLTFSDK.PublicConsumer.Test/CMakeLists.txt` — Support installed paths/configurations.
- `GLTFSDK.PublicConsumer.Test/Source/main.cpp` — Exercise changed public boundaries.
- `Build/CMake/Modules/GLTFPlatform.cmake` — Adjust public header/notice install only if required.

## Implementation steps

1. Ensure dependencies are local only: no find_package/FetchContent/ExternalProject/download/install/export.
2. Ensure GLTFSDK links both PRIVATE and package contains no dependency headers/targets.
3. Repair public header installation if current install helper does not copy Inc/GLTFSDK; verify actual output.
4. Clean-configure with network unavailable/monitored.
5. Build installed-only consumer for x64/Win32; ARM64 compile/link evidence later.

## Targeted validation commands

- `cmake -S E:\Base3D\glTF-SDK -B E:\Base3D\glTF-SDK\Built\Int\cmake_offline_x64 -G "Visual Studio 18 2026" -A x64 -DENABLE_SAMPLES=OFF`
- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_offline_x64 --target install --config Debug`
- `cmake -S E:\Base3D\glTF-SDK\GLTFSDK.PublicConsumer.Test -B E:\Base3D\glTF-SDK\Built\Int\public_consumer_offline -DGLTFSDK_ROOT=E:\Base3D\glTF-SDK\Built\Out\windows_x64\Debug\GLTFSDK`
- `cmake --build E:\Base3D\glTF-SDK\Built\Int\public_consumer_offline --config Debug`
- `Get-ChildItem -Recurse E:\Base3D\glTF-SDK\Built\Out | Select-String -Pattern 'RapidJSON|nlohmann|valijson'`

## Acceptance criteria

- [ ] Clean offline configure/build/install succeeds as C++14 with no downloads.
- [ ] Installed consumer links and package exposes no JSON dependency.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Revert build/install/consumer hardening; retain migrations/dependencies.

## Expected artifacts

- Hermetic package model and consumer evidence.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
