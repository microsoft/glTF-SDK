# Task 1.4: Vendor nlohmann/json 3.12.0 in Jsonnet layout

## Goal

Add the exact approved header, license, and hermetic target while RapidJSON remains active.

## Requirements addressed

REQ-DEP-1, REQ-DEP-2, REQ-DEP-3, REQ-DEP-4, REQ-DEP-5, REQ-DEP-6

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Task 1.1 preflight; approved local header must retain its hash.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `External/json/nlohmann/json.hpp` — Copy approved local header byte-for-byte.
- `External/json/LICENSE` — Add applicable MIT license.
- `External/json/CMakeLists.txt` — Declare 3.12.0 INTERFACE target and alias.
- `CMakeLists.txt` — Add vendored subdirectory without replacing RapidJSON.

## Implementation steps

1. Verify Jsonnet third_party/json shape is BUILD, LICENSE, nlohmann/json.hpp; replace only BUILD with CMakeLists.txt.
2. Copy only json.hpp from C:\Users\sergioze\Downloads\include\single_include\nlohmann; omit json_fwd.hpp and BUILD.
3. Verify SHA-256 AAF127C04CB31C406E5B04A63F1AE89369FCCDE6D8FA7CDDA1ED4F32DFC5DE63 and version macros 3.12.0.
4. Create CMake 3.5 no-language project, real INTERFACE nlohmann_json, alias nlohmann_json::nlohmann_json, SYSTEM build include, no install/network rules.
5. Prove configure/build performs no fetch.

## Targeted validation commands

- `Get-FileHash C:\Users\sergioze\Downloads\include\single_include\nlohmann\json.hpp -Algorithm SHA256`
- `Get-FileHash E:\Base3D\glTF-SDK\External\json\nlohmann\json.hpp -Algorithm SHA256`
- `cmake -S E:\Base3D\glTF-SDK -B E:\Base3D\glTF-SDK\Built\Int\cmake_nlohmann_probe -DENABLE_UNIT_TESTS=OFF -DENABLE_SAMPLES=OFF`
- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_nlohmann_probe --config Debug --target nlohmann_json`

## Acceptance criteria

- [ ] Exact three-file mirrored layout exists without BUILD/json_fwd.hpp.
- [ ] Target version/alias/include/offline/no-install behavior is correct.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Remove External/json and its root add_subdirectory; leave RapidJSON untouched.

## Expected artifacts

- Vendored header, license, and interface target.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
