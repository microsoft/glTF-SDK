# Task 5.2: Expand platform and CI matrix for Release/2.0.0

## Goal

Gate desktop, mobile, sanitizer, offline, and consumer builds on the release branch.

## Requirements addressed

REQ-BLD-3, REQ-BLD-4, REQ-BLD-5, REQ-BLD-6, REQ-BLD-7, REQ-BLD-8, REQ-BLD-11, REQ-BLD-12, REQ-TST-10

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Task 5.1 provides hermetic commands.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `.github/workflows/ci.yml` — Gate Release/2.0.0 complete matrix.
- `.github/workflows/windows.yml` — Windows x64/Win32/ARM64 both configs/consumer.
- `.github/workflows/linux.yml` — Linux both configs/tests/consumer.
- `.github/workflows/macos.yml` — macOS both configs/tests/consumer.
- `.github/workflows/ios.yml` — Device and simulator cross-build/install.
- `.github/workflows/android.yml` — Create three-ABI NDK workflow.
- `.github/workflows/sanitizer.yml` — Full and malformed/deep ASAN/UBSAN tests.

## Implementation steps

1. Change branch gates from Release/1.9.5 to Release/2.0.0 only.
2. Run Windows x64/Win32 tests/consumer both configs; ARM64 build/install/consumer compile-link.
3. Run Linux/macOS tests/consumer; add iOS simulator alongside device.
4. Add Android armeabi-v7a, arm64-v8a, x86_64 configure/build/link/install.
5. Add offline assertion and upload evidence for every gate.
6. Keep Windows RapidJSON workaround until deletion Task 5.5, then remove/retest there.

## Targeted validation commands

- `cmake -S E:\Base3D\glTF-SDK -B E:\Base3D\glTF-SDK\Built\Int\cmake_ci_x64 -G "Visual Studio 18 2026" -A x64`
- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_ci_x64 --target install --config Debug`
- `E:\Base3D\glTF-SDK\Built\Out\windows_x64\Debug\GLTFSDK.Test\GLTFSDK.Test.exe --gtest_output=xml:ci-local.xml`
- `git -C E:\Base3D\glTF-SDK diff --check -- .github/workflows`

## Acceptance criteria

- [ ] Workflow syntax and representative local Windows gate pass.
- [ ] CI defines every required platform/config/consumer/sanitizer/evidence gate.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Revert workflow changes only.

## Expected artifacts

- Release/2.0.0 CI matrix with Android/iOS simulator.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
