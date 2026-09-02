# Exploration Notes

## Repository state

- Repository: `E:\Base3D\glTF-SDK`.
- Current branch: `Release/2.0.0`.
- `HEAD`, `upstream/Release/1.9.5`, and their merge base are commit `3193f83265a70585093f13d651167b763979ade1`; ahead/behind is `0/0`.
- `origin` is `git@github.com:SergioRZMasson/glTF-SDK.git`; `upstream` is `https://github.com/microsoft/glTF-SDK.git`.
- The current worktree has untracked `.ai/` planning material. Executors must preserve it and any later user changes; they must not use destructive `reset`, `clean`, checkout-overwrite, or blanket staging.
- Local nlohmann source exists at `C:\Users\sergioze\Downloads\include\single_include\nlohmann\json.hpp`, declares 3.12.0, and has SHA-256 `AAF127C04CB31C406E5B04A63F1AE89369FCCDE6D8FA7CDDA1ED4F32DFC5DE63`.
- Jsonnet's current `third_party/json` contains only `BUILD`, `LICENSE`, and `nlohmann/json.hpp`; the required SDK mirror replaces `BUILD` with `CMakeLists.txt` and deliberately omits `json_fwd.hpp`.

## Build and packaging

- Root `CMakeLists.txt:36-56,76` probes/downloads RapidJSON, publishes its include path, and installs its headers to `Built/Out/RapidJSON`.
- `External/RapidJSON/CMakeLists.txt` performs configure-time download/build and creates the `RapidJSON` interface target.
- `GLTFSDK/CMakeLists.txt` globs `Source/*.cpp`, generates `SchemaJson.h`, creates `GLTFSDK`, publishes only `Inc`, and uses `CreateGLTFInstallTargets`.
- `Build/CMake/Modules/GLTFPlatform.cmake` defines platform output names and install destinations.
- `GLTFSDK.Test/CMakeLists.txt` globs test sources/resources and links `GLTFSDK`, `GLTFSDK.TestUtils`, and `GTest::gtest_main`.
- `.github/workflows/ci.yml` gates `Release/1.9.5`; reusable workflows cover Windows, Linux, macOS, iOS device, and sanitizer. Android and iOS simulator workflows are absent. `.github/workflows/windows.yml:17-23` contains the RapidJSON-only CMake policy workaround.
- Existing validation pattern: configure with `cmake -B Built/...`, build target `install`, then run installed or build-tree `GLTFSDK.Test` with Google Test filters such as `--gtest_filter=DeserializeTests.*`.

## Public API and internal JSON hotspots

- `GLTFSDK/Inc/GLTFSDK/RapidJsonUtils.h` is the public catch-all vendor boundary. It defines parse helpers, typed access, ordered construction helpers, writer helpers, and the compact-stream BOM defect in `CreateDocumentFromEncodedStream`.
- `GLTFSDK/Inc/GLTFSDK/ExtrasDocument.h` directly owns `rapidjson::Document`, exposes `GetDocument()`, and implements typed root/member/pointer operations inline.
- `GLTFSDK/Inc/GLTFSDK/SchemaValidation.h` includes `RapidJsonUtils.h` and exposes the DOM-taking validator.
- `GLTFSDK/Inc/GLTFSDK/Deserialize.h` and `Serialize.h` are already vendor-neutral and must retain their overloads and flags.
- `GLTFSDK/Inc/GLTFSDK/Schema.h` contains all schema URI constants and the intentionally duplicated `0x80` sparse/sparse-values flags.
- Planned private files are `GLTFSDK/Source/Internal/Json.h`, `GLTFSDK/Source/Internal/JsonSchema.h`, `GLTFSDK/Source/Json.cpp`, and `GLTFSDK/Source/ExtrasDocument.cpp`.

## Production integration points

- `GLTFSDK/Source/Deserialize.cpp:59-894` parses properties and every glTF object from RapidJSON values. `DeserializeInternal` at line 857 validates and then traverses the same DOM. Public string/stream entry points at lines 902-927 currently choose separate RapidJSON parse helpers.
- `GLTFSDK/Source/Serialize.cpp:85-858` builds the document in stable call order. `SerializePropertyExtensions` and `SerializePropertyExtras` parse raw JSON strings and copy their subtrees. `CreateJsonDocument` at line 831 controls root member order; public writing at lines 867-889 selects compact or pretty writers.
- `GLTFSDK/Source/ExtensionsKHR.cpp` repeats parse/property/texture helpers and implements serializers/deserializers for PBR specular-glossiness, unlit, clearcoat, volume, iridescence, transmission, sheen, specular, Draco, mesh GPU instancing, and texture transform.
- `GLTFSDK/Source/SchemaValidation.cpp` owns a per-call RapidJSON remote provider and preserves the diagnostic shape `Schema violation at <pointer> due to <keyword>`.
- `GLTFSDK/Source/Schema.cpp` maps all 33 bundled schema URIs to flags and returns `{}` for disabled schemas.

## Valijson evidence

- Required source is Valijson tag `v1.0.6`, commit `4edda758546436462da479bb8c8514f8a95c35ad`.
- `include/valijson/adapters/nlohmann_json_adapter.hpp` hard-codes `nlohmann::json` in iterator, value, frozen-value, singleton, constructor, and `AdapterTraits::DocumentType` declarations.
- `include/valijson/schema_parser.hpp` defaults to Draft-07 unless constructed with `SchemaParser::kDraft4`; its document fetch/free callbacks and local caches are the main integration points.
- `include/valijson/internal/uri.hpp::resolveRelativeUri` is only `resolutionScope + relativeUri`.
- Upstream `tests/test_validator.cpp` comments out Draft-04 `ref` and `refRemote`.
- `ValidationResults::Error` currently carries only `context` and English `description`, so a private keyword field must be threaded from validation visitors/constraints.

## Existing tests to retain and extend

- `GLTFSDK.Test/Source/GLTFTests.cpp:595-655` covers BOM behavior but uses whitespace after the BOM; compact `{` coverage must be added.
- `DeserializeTests.cpp` contains exact schema diagnostics, missing-dependent-property CWE regressions, and schema-disabled semantic type/array checks.
- `ExtrasDocumentTests.cpp` covers current typed operations and relies on `Serialize(extrasDoc.GetDocument())`; it is the migration point for `ToJson`, `HasMember`, move-only behavior, types, and pointer semantics.
- `GLTFExtensionsTests.cpp:124-146` directly builds/parses RapidJSON in custom handlers; lines 713-869 cover custom handlers and extension schema locators.
- `SerializeTests.cpp` and `GLTFExtensionsTests.cpp` contain byte-exact pretty output and round-trip gates.
- New focused files should separate JSON core/parser/writer tests, schema integration/conformance tests, and an out-of-tree installed-package consumer.

## Documentation and notices

- `README.md` identifies the branch as `Release/1.9.5` and documents current build/test commands.
- `thirdPartyNotices.txt` contains RapidJSON and material bundled with it; the audit must remove only obsolete entries while retaining JSON Schema Test Suite and other still-used notices.
- No release-note or dependency-documentation directory currently exists; the implementation plan establishes explicit `docs/` paths.
