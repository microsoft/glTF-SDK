# Replace RapidJSON with nlohmann/json

## Goal

Create a `Release/2.0.0` branch from Microsoft's `Release/1.9.5` branch and
completely replace glTF-SDK's RapidJSON dependency with nlohmann/json 3.12.0.

## Required outcomes

- Base `Release/2.0.0` on
  `https://github.com/microsoft/glTF-SDK.git` branch `Release/1.9.5`.
- Remove `External/RapidJSON`.
- Add `External/json` with the same source layout as
  `https://github.com/google/jsonnet/tree/master/third_party/json`, using the
  local nlohmann/json 3.12.0 single-header source at
  `C:\Users\sergioze\Downloads\include\single_include\nlohmann`.
- Replace Jsonnet's `BUILD` file with `CMakeLists.txt`.
- Have `External/json/CMakeLists.txt` declare version 3.12.0 and provide a
  header-only CMake target for nlohmann/json.
- Replace every glTF-SDK production and test use of RapidJSON with
  nlohmann/json.
- Treat nlohmann/json 3.12.0 as the selected core JSON library.
- Treat `Release/2.0.0` as an intentional source/ABI-breaking release: remove
  public RapidJSON DOM APIs and replace them with SDK-owned string/typed APIs
  rather than exposing nlohmann/json types.
- Preserve glTF parsing, serialization, extension/extras, JSON Pointer, schema
  validation, supported-platform, and error behavior except for explicitly
  approved changes below.
- Keep C++14 and use Valijson 1.0.6 through its nlohmann/json adapter for
  Draft-04 validation. Require the complete bundled schema graph and external
  `$ref` test suite to pass, fixing the integration as needed rather than
  weakening validation or silently raising the language baseline.
- Remove RapidJSON-specific build logic, patches, installed artifacts, notices,
  and CI workarounds.
- Update tests for nlohmann/json and add regression coverage for migration
  compatibility risks.
- Make the complete test suite pass.
- Limit implementation changes to glTF-SDK. Document known downstream source
  breaks in adjacent repositories without modifying those repositories.
- Commit the completed work and push `Release/2.0.0` to
  `git@github.com:SergioRZMasson/glTF-SDK.git`.

## Quality constraints

- Keep the SDK at C++14 unless a hard, documented blocker requires explicit
  user approval.
- Do not expose nlohmann/json or Valijson types from public APIs.
- Preserve supported Windows, Linux, macOS, iOS, and Android builds.
- Reject duplicate JSON object member names and invalid UTF-8.
- Fix the compact-stream BOM first-character bug while preserving the intended
  `DeserializeFlags::IgnoreByteOrderMark` contract.
- Preserve existing compact/pretty output formatting tests.
- Validate Draft-04 external `$ref` behavior against the bundled glTF schemas
  and extension-schema tests.
- Measure and report parse, validation, write, memory, binary-size, and compile
  deltas before finalizing.

## Dependency layout

- Mirror the source structure of
  `https://github.com/google/jsonnet/tree/master/third_party/json` under
  `External/json`.
- Replace Jsonnet's `BUILD` file with `CMakeLists.txt`.
- Declare nlohmann/json version 3.12.0 in that CMake project.
- Expose the conventional header-only target
  `nlohmann_json::nlohmann_json`.
- Use the local 3.12.0 headers from
  `C:\Users\sergioze\Downloads\include\single_include\nlohmann`.
- Include all required upstream license and notice material.
- Integrate Valijson 1.0.6 as a private validation dependency using a
  repository-consistent pinned external layout.

## Existing planning inputs

- `.ai/00-rapidjson-current-state.md`
- `.ai/01-json-library-evaluation.md`
- `.ai/02-library-agnostic-migration-plan.md`
