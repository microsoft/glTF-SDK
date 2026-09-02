# Offline package and public-consumer evidence

Date: 2026-09-01

## Offline configuration

Clean x64 and Win32 Visual Studio 2022 build trees were configured with
`HTTP_PROXY`, `HTTPS_PROXY`, and `ALL_PROXY` directed to the unreachable local
endpoint `127.0.0.1:9`. Unit tests, samples, and benchmarks were disabled for
the package-only configure.

Both configurations completed without a network operation:

```powershell
cmake -S E:\Base3D\glTF-SDK `
  -B E:\Base3D\glTF-SDK\Built\Int\cmake_offline_x64 `
  -G "Visual Studio 17 2022" -A x64 `
  -DENABLE_UNIT_TESTS=OFF -DENABLE_SAMPLES=OFF -DENABLE_BENCHMARKS=OFF

cmake -S E:\Base3D\glTF-SDK `
  -B E:\Base3D\glTF-SDK\Built\Int\cmake_offline_Win32 `
  -G "Visual Studio 17 2022" -A Win32 `
  -DENABLE_UNIT_TESTS=OFF -DENABLE_SAMPLES=OFF -DENABLE_BENCHMARKS=OFF
```

The installed CMake 3.31.8 does not expose a Visual Studio 18 generator, so
local evidence uses Visual Studio 17/MSVC 19.44. Visual Studio 18 is retained
as a later CI/platform gate.

## Build and install

Both commands passed without warnings after removing an invalid constructor
calling-convention annotation:

```powershell
cmake --build Built\Int\cmake_offline_x64 --target install --config Debug
cmake --build Built\Int\cmake_offline_Win32 --target install --config Debug
```

Each installed package contains 44 files: the static library/PDB, 38 public
SDK headers, repository license, third-party notice, and separately named
nlohmann/json and Valijson licenses.

No installed package contains `json.hpp`, `json_fwd.hpp`, a Valijson header,
`RapidJsonUtils.h`, RapidJSON content, or a dependency CMake target. Scanning
both installed `Inc` trees for `rapidjson|nlohmann|valijson` returned no
matches. Dependency names occur only in the required notice/license files.

## Installed-only consumers

Independent x64 and Win32 consumer trees were configured using only:

- the configuration-specific installed `GLTFSDK.lib`; and
- the installed `Inc` directory.

Both consumers built, linked, and ran successfully. The program includes and
exercises `Deserialize`, `Serialize`, the move-only `ExtrasDocument` boundary,
and string-based schema validation. It has no source-tree include path and no
JSON-vendor include or link dependency.
