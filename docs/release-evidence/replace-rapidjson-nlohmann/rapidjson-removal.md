# RapidJSON removal evidence

Recorded: 2026-09-01

## Removed artifacts

- `External/RapidJSON/**`, including download logic and local patches
- `GLTFSDK/Inc/GLTFSDK/RapidJsonUtils.h`
- root dependency discovery, public include propagation, and install wiring
- the Windows-only RapidJSON CMake policy workaround
- obsolete vendor-specific test and Valijson adapter/utility content

The Valijson pruning retained all applicable author credit with
parser-neutral wording. The final 48-file shipped subset has deterministic
tree SHA-256
`13FA74916D7D9D424841C64569F905CC95FFFF4327874032D6C227C197395AC9`.
All four retained correction patches applied cleanly, and their SHA-256
values matched `External/Valijson/UPSTREAM.md`.

## Clean build and tests

The following clean build used Visual Studio 2022 17.14, MSVC 19.44, and the
x64 Windows SDK target:

```text
cmake -S . -B Built/Int/cmake_no_rapidjson_x64
  -G "Visual Studio 17 2022" -A x64 -DENABLE_SAMPLES=OFF
cmake --build Built/Int/cmake_no_rapidjson_x64
  --target install --config Debug --parallel 8
```

Configure, build, and install passed without locating or downloading
RapidJSON. Focused migration coverage passed 218/218 tests across JSON,
schema, deserialization, serialization, extensions, and extras. The full
clean Debug suite then passed 504/504 tests.

## Shipped-tree and package scans

Case-insensitive content and filename scans found no `RapidJSON` or
`RapidJsonUtils` remnants in:

- `GLTFSDK/**`
- `External/**`
- `GLTFSDK.Test/**`
- `GLTFSDK.TestUtils/**`
- current samples and build metadata
- `.github/workflows/**`
- `thirdPartyNotices.txt`
- `Built/Out/windows_x64/Debug/GLTFSDK/**`

The installed public headers also contain no nlohmann/json or Valijson names,
and no JSON dependency header directory is installed. Seven workflow YAML
files parse successfully. Remaining old-vendor names are limited to approved
historical planning, migration, and benchmark-baseline evidence.
