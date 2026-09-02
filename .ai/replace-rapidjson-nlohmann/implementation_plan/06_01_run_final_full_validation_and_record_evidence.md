# Task 6.1: Run final full validation and record release evidence

## Goal

Execute every host, conformance, sanitizer, install, consumer, scan, and cross-build gate on the RapidJSON-free tree.

## Requirements addressed

REQ-REL-2, REQ-BLD-3, REQ-BLD-4, REQ-BLD-5, REQ-BLD-6, REQ-BLD-7, REQ-BLD-11, REQ-TST-1, REQ-TST-6, REQ-TST-7, REQ-TST-8, REQ-TST-9, REQ-TST-10

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Task 5.5 removed RapidJSON; benchmark review/docs complete.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `docs/release-evidence/replace-rapidjson-nlohmann/final-validation.md` — Record commands/commits/toolchains/configs/results/artifacts.
- `docs/release-evidence/replace-rapidjson-nlohmann/requirement-trace.md` — Map every REQ to code/tests/build/docs/evidence.

## Implementation steps

1. Use fresh named build directories without deleting unrelated builds/user files.
2. Run Debug/RelWithDebInfo Windows x64/Win32 tests/install/consumer; ARM64 build/install/consumer compile-link.
3. Run/link CI evidence for Linux/macOS both configs, iOS device/simulator, Android three ABIs, Linux ASAN/UBSAN.
4. Run full suite plus parser/schema conformance/extensions/extras/malformed/deep/public consumer/offline/package scans.
5. Verify C++14 and unchanged CMake/toolchain/deployment baselines.
6. Map every requirement; leave failing gates open and do not proceed to push.

## Targeted validation commands

- `cmake -S E:\Base3D\glTF-SDK -B E:\Base3D\glTF-SDK\Built\Int\cmake_final_x64 -G "Visual Studio 18 2026" -A x64`
- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_final_x64 --target install --config Debug`
- `cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_final_x64 --target install --config RelWithDebInfo`
- `E:\Base3D\glTF-SDK\Built\Out\windows_x64\Debug\GLTFSDK.Test\GLTFSDK.Test.exe --gtest_output=xml:final-debug.xml`
- `E:\Base3D\glTF-SDK\Built\Out\windows_x64\RelWithDebInfo\GLTFSDK.Test\GLTFSDK.Test.exe --gtest_output=xml:final-relwithdebinfo.xml`
- `ctest --test-dir E:\Base3D\glTF-SDK\Built\Int\cmake_final_x64 -C Debug --output-on-failure`
- `git -C E:\Base3D\glTF-SDK diff --check`

## Acceptance criteria

- [ ] Every required platform/config gate has passing recorded result/CI link.
- [ ] Every REQ traced; no sanitizer/scan/consumer/schema/full-suite failure.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Rollback to smallest buildable checkpoint for a failed gate; never falsify evidence.

## Expected artifacts

- Final validation report, requirement trace, XML/CI links, scans.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
