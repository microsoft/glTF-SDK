[![Build Status](https://github.com/microsoft/glTF-SDK/actions/workflows/ci.yml/badge.svg)](https://github.com/microsoft/glTF-SDK/actions/workflows/ci.yml)

> **2.0 migration:** This development line contains intentional source and ABI
> breaks from 1.9.5. See the
> [2.0 migration guide](docs/MigrationGuide-2.0.md) and
> [release notes](docs/ReleaseNotes/2.0.0.md).

# Microsoft glTF SDK - A C++ Deserializer/Serializer for glTF

* Windows, Linux, macOS, iOS, and Android support
* glTF and GLB support, including embedded Base64 resources
* Strongly typed extension support
* Built-in JSON Schema Draft-04 validation
* C++14 baseline

## JSON implementation in 2.0

The SDK uses a private, vendored nlohmann/json 3.12.0 ordered DOM and a
corrected private Valijson 1.0.6 Draft-04 validator. Neither dependency appears
in installed public headers or needs to be installed by SDK consumers.

2.0 removes the public `GLTFSDK/RapidJsonUtils.h` surface,
`ExtrasDocument::GetDocument()`, and the DOM-taking schema-validation overload.
Use `ExtrasDocument::ToJson()`, `ExtrasDocument::HasMember()`, its typed and
JSON Pointer operations, and the string-based `ValidateDocumentAgainstSchema`
API instead.

Parsing now rejects duplicate object names, invalid UTF-8, comments, trailing
commas, non-finite spellings, and inputs deeper than 256 object/array
containers. String and stream overloads share the same parser. A UTF-8 BOM is
accepted only with `DeserializeFlags::IgnoreByteOrderMark`, including compact
stream input.

## Configure and build

Required tools are Git, CMake, a supported C++14 toolchain, and PowerShell
(`pwsh` or Windows PowerShell) for schema-header generation.

```powershell
git clone https://github.com/microsoft/glTF-SDK.git
cd glTF-SDK
cmake -S . -B Built
cmake --build Built --target install --config Debug
```

The build is offline with respect to the JSON dependencies: their exact
headers are committed under `External/`. Unit-test configuration may still use
the repository's existing GoogleTest fallback when no installed GoogleTest
package is found.

Installed artifacts are written to
`Built/Out/<platform>/<configuration>/GLTFSDK`. The package contains the SDK
library, public headers, and required notices, but no nlohmann/json, Valijson,
or RapidJSON headers or CMake targets.

Optional targets:

```powershell
# Dependency-free JSON benchmark harness
cmake -S . -B Built/Bench -DENABLE_UNIT_TESTS=OFF -DENABLE_SAMPLES=OFF -DENABLE_BENCHMARKS=ON
cmake --build Built/Bench --config RelWithDebInfo --target GLTFSDK.JsonBenchmarks

# Configure the installed-only public consumer independently
cmake -S GLTFSDK.PublicConsumer.Test -B Built/PublicConsumer `
  -DGLTFSDK_ROOT=Built/Out/windows_x64/Debug/GLTFSDK
cmake --build Built/PublicConsumer --config Debug
```

## Running tests

From the installed test folder:

```powershell
.\GLTFSDK.Test.exe --gtest_output=xml:GLTFSDK.Test.log
```

The CI matrix covers Windows x64/Win32/ARM64, Linux, macOS, iOS
device/simulator, three Android ABIs, and Linux ASAN/UBSAN.

## Documentation

* [2.0.0 release notes](docs/ReleaseNotes/2.0.0.md)
* [2.0 migration guide](docs/MigrationGuide-2.0.md)
* [Dependency and provenance details](docs/Dependencies.md)
* [Known downstream source breaks](docs/DownstreamBreaks-2.0.md)
* [Release evidence](docs/release-evidence/replace-rapidjson-nlohmann)

# Trademarks

glTF is a trademark of The Khronos Group Inc.

# Contributing

This project welcomes contributions and suggestions. Most contributions
require a Contributor License Agreement. See
https://cla.microsoft.com and the
[Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
