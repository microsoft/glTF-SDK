# RapidJSON Replacement: Current State and Constraints

Status: planning input
Assessment date: 2026-09-01
Repository state reviewed: `59bbf36dfeee4ee83335db792c9ea7c6f7289975` on
`sergioze/deserialize-sample-extension-schema-options`

## Feasibility conclusion

Removing RapidJSON is feasible, but it is not a package-name or include-name
replacement. The SDK uses RapidJSON as four components:

1. A strict JSON parser for strings and streams.
2. A mutable, insertion-ordered DOM for serialization and extension/extras
   composition.
3. A JSON Pointer implementation for reading and creating values.
4. A Draft-04 JSON Schema validator with externally resolved `$ref` documents.

RapidJSON types also appear in public headers. A complete removal therefore
requires source migration, schema-validator migration, public API decisions,
build-system changes, and downstream consumer updates.

## Current dependency integration

| Area | Current behavior | Replacement impact |
|---|---|---|
| Discovery | Root `CMakeLists.txt` tries `find_package(RapidJSON CONFIG)` first. | Preserve installed-package-first behavior where the selected library supports it. |
| Fallback | `External/RapidJSON` downloads commit `232389d4f1012dddec4ef84861face2d2ba85709` during configure. | Pin an audited release/commit of the selected library and expose one CMake target. |
| Local patches | Three configure-time patches address allocator null dereference, a cast diagnostic, and writer size overflow. | Delete these patches after equivalent regression coverage passes against the replacement. |
| Include propagation | RapidJSON include directories are `PUBLIC` because public SDK headers expose its types. | Make the replacement `PRIVATE` if the new API boundary hides vendor types. |
| Installation | RapidJSON headers are copied to `Built/Out/RapidJSON`. | Stop installing JSON-library headers if the dependency is private. |
| CI | Windows currently sets a CMake policy workaround specifically for vendored RapidJSON. | Remove the workaround after confirming no other dependency needs it. |
| Notices | `thirdPartyNotices.txt` includes RapidJSON and material bundled with it. | Re-audit and replace only notices that cease to apply. |

Primary files:

- `CMakeLists.txt:36-56,76`
- `External/RapidJSON/CMakeLists.txt`
- `External/RapidJSON/CMakeRapidJSONDownload.txt.in`
- `External/RapidJSON/patches/*`
- `.github/workflows/windows.yml:17-23`
- `thirdPartyNotices.txt`

## Code footprint

The migration is concentrated, despite the high number of individual API
calls.

| Surface | RapidJSON role | Migration size |
|---|---|---|
| `GLTFSDK/Source/Deserialize.cpp` | DOM traversal, typed numeric/string access, arrays, objects | High: 958 lines; 89 RapidJSON-related lines |
| `GLTFSDK/Source/Serialize.cpp` | Mutable DOM creation, ordered members, arrays, compact/pretty writers | High: 919 lines; 168 RapidJSON-related lines |
| `GLTFSDK/Source/ExtensionsKHR.cpp` | Repeats both parse and serialization patterns for KHR extensions | High: 1,355 lines; 124 RapidJSON-related lines |
| `GLTFSDK/Source/SchemaValidation.cpp` | Draft-04 validation, schema cache, external `$ref` resolver, diagnostics | High-risk: 85 lines; behavior is more important than size |
| `GLTFSDK/Inc/GLTFSDK/RapidJsonUtils.h` | Public catch-all include and approximately 30 helper overloads | Replace with private/vendor-neutral helpers |
| `GLTFSDK/Inc/GLTFSDK/ExtrasDocument.h` | Public DOM ownership, JSON Pointer get/create, typed values | Public source/ABI decision required |
| `GLTFSDK/Inc/GLTFSDK/SchemaValidation.h` | Public validator accepts `rapidjson::Document` | Public source/ABI decision required |
| `GLTFSDK.Test/Source/GLTFExtensionsTests.cpp` | Test extension directly constructs and validates a RapidJSON DOM | Migrate to the new public or internal API |
| `GLTFSDK.Test/Source/ExtrasDocumentTests.cpp` | Serializes the vendor DOM returned by `GetDocument()` | Migrate to vendor-neutral `ExtrasDocument` operations |

Common operations include approximately 138 member insertions, 93 member
lookups, 77 end-iterator comparisons, 29 array appends, 34 allocator accesses,
and numerous explicit integer/float type checks. An adapter should cover
repeated operations, but it should not attempt to emulate the entire RapidJSON
API.

## Public API and downstream impact

### SDK public surface

`RapidJsonUtils.h` lives in the SDK's public `Inc/GLTFSDK` include surface and
is consumed directly by known downstream code. It puts RapidJSON in the
`Microsoft::glTF::rapidjson` namespace and exposes `Document`, `Value`,
allocator, writer, pointer, and schema types.

`ExtrasDocument::GetDocument()` returns `const rapidjson::Document&`.
`ValidateDocumentAgainstSchema` accepts `const rapidjson::Document&`.

Replacing those signatures with another vendor's DOM would remove RapidJSON
but preserve the architectural problem. The preferred end state is an
SDK-owned, vendor-neutral boundary.

### Known consumers in this workspace

The adjacent `bigpark.canvas.native.lib` workspace proves that the public
coupling is in use:

- `Transcoders/Shared/Transcoders/Source/ExporterGLTF/Asset3DToGLTFConverter.cpp`
  includes `GLTFSDK/RapidJsonUtils.h`, directly builds RapidJSON documents, and
  contains a warning about the version/namespace coupling.
- `CoreUtils/Shared/CoreUtils/cpp/Source/Gltf.cpp` and
  `Engine/Products/Components/NativeRenderer/cpp/Source/Engine/Serializing/GLTFSerializerWriter.cpp`
  serialize `ExtrasDocument::GetDocument()`.
- `Transcoders/Shared/Transcoders.Test/Source/TranscoderGLTF/TranscoderGLTFTests.cpp`
  calls `GetDocument().HasMember(...)`.

These consumers will need a coordinated migration or a staged deprecation
period. Public source compatibility cannot be maintained while also removing
all RapidJSON headers unless the old APIs are removed or isolated in a
separate compatibility package.

## Behavioral compatibility contract

The selected implementation and migration tests must explicitly define and
preserve the following behavior.

### Parsing

- C++14 support remains mandatory.
- Windows x64/Win32/ARM64, macOS, Linux, iOS, and Android remain supported.
- JSON is strict by default: comments, trailing commas, and non-finite numbers
  must not become accepted accidentally.
- The intended post-migration behavior is that a UTF-8 BOM is accepted only
  when `DeserializeFlags::IgnoreByteOrderMark` is supplied; without the flag
  it must fail.
- String and stream overloads must become behaviorally equivalent.
- Nesting must be bounded or processed without unbounded call-stack recursion.
  The current RapidJSON configuration uses iterative parsing specifically to
  avoid stack overflow.
- Malformed JSON continues to become an SDK `GLTFException`, not a
  vendor-specific exception.

Known baseline defects must not be mistaken for compatibility requirements:

- `CreateDocumentFromEncodedStream` constructs an `EncodedInputStream` but
  passes the underlying `IStreamWrapper` to `ParseStream`. On compact input,
  construction of the encoded wrapper can consume/buffer the first JSON
  character and the parser can start after it. Existing BOM tests begin with a
  newline and do not cover this case.
- The default RapidJSON flags enable iterative parsing but not
  `kParseValidateEncodingFlag`. Invalid UTF-8 is therefore not intentionally
  validated today. The migration must explicitly choose whether to preserve
  that behavior or reject invalid UTF-8 as an approved correctness/security
  change; rejection is the recommended contract.

### DOM and typed access

- Objects used for output preserve insertion order.
- Signed, unsigned, 32-bit, 64-bit, and floating values receive explicit range
  and integrality checks equivalent to current `IsUint`, `GetUint`, and related
  use.
- Numeric conversion must not silently turn an out-of-range integer into a
  floating value accepted by a typed glTF field.
- Duplicate object member names and invalid UTF-8 are rejected under the
  migration plan's recommended strict-input policy. Both are intentional
  changes from behavior that RapidJSON does not currently reject.
- Maximum nesting behavior must be characterized in a proof-of-concept and then
  locked with tests.
- Extension and extras JSON with unique member names remains structurally
  round-trippable.

### Serialization

- Compact and four-space pretty output remain available.
- Member order remains stable where tests and downstream snapshots rely on it.
- Floating-point formatting is compared against current outputs, including
  preserving a floating representation for values such as `1.0f`.
- Escaping, UTF-8 handling, negative zero, exponent formatting, and large
  integer output are covered by compatibility tests.

### JSON Pointer

- Lookup supports array and object paths used by
  `ExtrasDocument::GetPointerValueOrDefault`.
- Set/create behavior supports intermediate object/array construction used by
  `ExtrasDocument::SetPointerValue`.
- Invalid pointer syntax and incompatible existing value types map to stable
  SDK exceptions.

### JSON Schema

- All 33 bundled schemas declare JSON Schema Draft-04.
- External relative `$ref` documents are resolved through `ISchemaLocator`.
- Per-schema `SchemaFlags` continue to work by substituting the permissive
  `{}` schema.
- Extension schemas can provide their own locator and reference core schemas.
- Invalid schemas, unknown schema URIs, and validation failures remain distinct
  failures.
- Validation diagnostics retain at least the failing document pointer and
  keyword. Exact existing text such as
  `Schema violation at #/accessors/0/count due to minimum` should be preserved
  where practical.
- The missing-dependent-property regressions that motivated the current
  RapidJSON patch must remain non-crashing.

`SchemaFlags::DisableSchemaAccessorSparse` and
`SchemaFlags::DisableSchemaAccessorSparseValues` currently share the numeric
value `0x80`, so either symbolic flag disables both schemas. This migration
will preserve and test that legacy behavior rather than silently alter a public
enum. Correcting the alias should be a separately approved API change.

## Planning assumptions and decisions still required

The migration plan assumes an intentional source/ABI break in one versioned
SDK release. A staged deprecation would necessarily retain RapidJSON for at
least one release and should be planned separately if that assumption is not
acceptable.

1. Select which replacement candidate to take through the mandatory
   proof-of-concept and, if necessary, its separate schema validator.
2. Decide whether byte-for-byte JSON formatting is a compatibility guarantee
   or whether semantic equivalence is enough outside existing documented
   pretty-output tests.
3. Set acceptable parse, write, peak-memory, binary-size, and compile-time
   regression thresholds using representative glTF assets.
