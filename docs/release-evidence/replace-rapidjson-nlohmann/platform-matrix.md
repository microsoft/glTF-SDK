# Platform and CI matrix evidence

Date: 2026-09-01

## Local Windows evidence

Visual Studio 2022/MSVC 19.44 clean invalid-proxy package builds passed for:

| Architecture | SDK configure/build/install | Installed consumer |
| --- | --- | --- |
| x64 | passed | built, linked, and ran |
| Win32 | passed | built, linked, and ran |
| ARM64 | passed | built and linked (cross-built executable not run) |

All configurations use C++14 and the installed consumer sees only the
configuration-specific SDK library and installed public headers. The local
CMake 3.31.8 does not provide the Visual Studio 18 generator, so Visual Studio
18 remains a remote Windows CI gate.

## Release/2.0.0 workflows

The branch-gated workflow now defines:

- Windows x64, Win32, and ARM64 in Debug and RelWithDebInfo;
- x64/Win32 unit tests and runnable installed consumers;
- ARM64 SDK/package and installed-consumer compile/link;
- Linux and macOS Debug/RelWithDebInfo tests and installed consumers;
- iOS device and 64-bit simulator Debug/RelWithDebInfo cross-build/install;
- Android armeabi-v7a, arm64-v8a, and x86_64
  Debug/RelWithDebInfo cross-build/install; and
- Linux Clang ASAN/UBSAN full-suite plus malformed/deep focused runs.

Every reusable platform workflow performs an invalid-proxy offline dependency
configure. Desktop workflows verify installed headers/package contents and
upload test XML plus package manifests. Mobile workflows upload package
manifests; sanitizer uploads both XML results.

All seven workflow YAML files parse successfully with PyYAML 6.0.3, and
`git diff --check -- .github/workflows` reports no whitespace errors.
