# Task 1.5: Vendor pristine Valijson 1.0.6 privately

## Goal

Import the pinned header subset, provenance, patch paths, and compatible target.

## Requirements addressed

REQ-DEP-7, REQ-DEP-9, REQ-DEP-10, REQ-BLD-1, REQ-BLD-2, REQ-BLD-9

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Task 1.4 provides nlohmann target.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `External/Valijson/include/valijson/**` — Copy v1.0.6 headers.
- `External/Valijson/LICENSE` — Copy license.
- `External/Valijson/Authors` — Copy authors.
- `External/Valijson/UPSTREAM.md` — Record source, tag, commit, hashes, commands, exclusions.
- `External/Valijson/CMakeLists.txt` — Create private interface target.
- `External/Valijson/patches/0001-ordered-nlohmann-adapter.patch` — Track adapter correction.
- `External/Valijson/patches/0002-draft04-uri-and-reference-resolution.patch` — Track resolver correction.
- `External/Valijson/patches/0003-structured-validation-keywords.patch` — Track diagnostics correction.
- `CMakeLists.txt` — Add vendored subdirectory while RapidJSON remains.

## Implementation steps

1. Acquire tag v1.0.6 commit 4edda758546436462da479bb8c8514f8a95c35ad and verify archive identity.
2. Copy only include/valijson, LICENSE, Authors; exclude tests/examples/inspector/submodules/bundled JSON/upstream CMake.
3. Record pristine archive/result tree hashes and reproducible extraction/update commands.
4. Create CMake 3.5 no-language valijson INTERFACE target, ValiJSON::valijson alias, SYSTEM build-only include, VALIJSON_USE_EXCEPTIONS=1, no install/network.
5. Create tracked patch files to be populated by Tasks 1.6-1.8.

## Targeted validation commands

- `cmake -S E:\Base3D\glTF-SDK -B E:\Base3D\glTF-SDK\Built\Int\cmake_valijson_probe -DENABLE_UNIT_TESTS=OFF -DENABLE_SAMPLES=OFF`
- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_valijson_probe --config Debug --target valijson`
- `git -C E:\Base3D\glTF-SDK diff -- External/Valijson`

## Acceptance criteria

- [ ] Tree is traceable to exact tag/commit and contains only approved files.
- [ ] Target is C++14/CMake-3.5 compatible, offline, private, and not installed.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Delete External/Valijson and root registration; keep nlohmann/RapidJSON.

## Expected artifacts

- Pinned tree, provenance, patch paths, target.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
