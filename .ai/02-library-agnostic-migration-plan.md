# RapidJSON Replacement: Library-Agnostic Migration Plan

Status: ready for preferred-candidate selection; production work is blocked on the selection proof-of-concept
Related documents:

- `.ai/00-rapidjson-current-state.md`
- `.ai/01-json-library-evaluation.md`

## Goal

Remove RapidJSON source, headers, build logic, patches, public types, and
notices from glTF-SDK while preserving supported platforms, C++14, glTF
serialization/deserialization behavior, Draft-04 schema validation, extension
handling, and `ExtrasDocument` functionality.

## Recommended architecture

Do not expose the replacement library's DOM in installed SDK headers.

Create a small private JSON layer owned by glTF-SDK. It should provide only the
operations the SDK uses:

- strict parse from string and stream;
- an insertion-ordered mutable value/document type;
- checked object lookup and typed numeric/string access;
- object/array construction helpers;
- compact and pretty serialization;
- JSON Pointer get and set/create;
- Draft-04 schema compilation/validation through `ISchemaLocator`; and
- translation of vendor failures into SDK exceptions.

This is not a generic JSON facade. Keep it narrow enough that changing
libraries again affects the adapter, schema implementation, and exceptional
candidate-specific code rather than hundreds of call sites.

## Phase 0: API decision and selection proof-of-concept

The user decision chooses which candidate to prototype first, not an
irrevocable production dependency. Before committing production migration to
that candidate, resolve the public boundary and build a throwaway C++14 spike
against the exact pinned release.

### Public API decision assumed by this plan

Complete RapidJSON removal in one release requires an intentional source and
ABI break. This plan assumes that path:

- Version the package/ABI as a breaking SDK release.
- Remove `ExtrasDocument::GetDocument()` and the DOM-taking public schema
  validator rather than retaining a RapidJSON compatibility overload.
- Preserve `ExtrasDocument` as move-only with an out-of-line destructor, move
  constructor, and move assignment if a private implementation is used. Export
  its out-of-line public methods with the SDK's normal `GLTFSDK_API` mechanism.
- Define and test the supported scalar contract explicitly: `bool`, signed and
  unsigned 32/64-bit integers, `size_t`, `float`, `double`, `std::string`, and
  `const char*` setters. Unsupported template types should fail clearly at
  compile time.
- Add `ExtrasDocument::ToJson()` (or `Serialize()`), `HasMember()`, and the
  existing typed member/pointer operations without exposing a vendor DOM.
- Replace the public DOM-taking validation function with a string-based
  function. Keep a selected-library DOM overload private to avoid reparsing in
  normal deserialization.
- Add consumer-facing public-header compile tests using the actual package
  include layout before porting large source files.

If compatibility must span releases, the overall removal must span releases
too, or the old APIs must move to a separately shipped RapidJSON compatibility
package. That alternative is outside the one-release plan.

The spike must demonstrate:

1. Configure and compile as C++14 with supported minimum compiler/toolchain
   versions where they are defined.
2. Strict string and stream parsing.
3. Explicit BOM accept/reject behavior on compact JSON for all string/stream
   and flag combinations, without reproducing the current encoded-stream
   first-character defect.
4. Insertion-ordered object output and four-space pretty formatting.
5. Exact checked conversion boundaries for every integer type used by glTF.
6. Representative float parsing and writing.
7. JSON Pointer lookup and intermediate-path creation.
8. Draft-04 validation of the root glTF schema.
9. Relative external `$ref` resolution through an `ISchemaLocator`-like
   callback.
10. Validation error extraction with document pointer and keyword.
11. Nesting-depth handling without stack exhaustion.
12. Rejection of duplicate member names and invalid UTF-8.
13. Cross-compile/link on Windows x64/Win32/ARM64, Linux, macOS, iOS device and
    simulator, and Android `armeabi-v7a`, `arm64-v8a`, and `x86_64`.
14. Consumer-facing public-header compilation on each practical
    architecture.
15. Builds using the SDK's deployment/API levels and normal exception/RTTI
    settings.
16. Baseline and candidate parse, schema, write, peak-memory, binary-size, and
    clean-build measurements on representative glTF workloads.

Exit criterion: no unresolved capability gap. Any intentional behavior change
is documented and approved before production code is ported. Minimum
acceptable performance/size thresholds are set before the candidate is
approved, not after the mechanical port.

## Phase 1: Dependency and build integration

1. Add exact or explicitly bounded compatible-version installed-package
   discovery for the selected dependency where useful.
2. Add a fallback pinned to an audited release or immutable commit.
3. Normalize both paths to one namespaced CMake target.
4. Keep the selected dependency private unless a companion schema package
   forces otherwise.
5. Verify offline/reproducible builds from an already populated source cache.
6. Add the selected license and required notices.
7. Run a no-code-change configure/build on every existing CI platform and the
   additional Android/architecture gates established in Phase 0 before
   beginning source migration.

Do not assume the dependency's top-level CMake minimum is compatible with the
SDK's declared CMake 3.5. Either raise and document the SDK minimum as an
approved change or consume pinned headers/sources behind an SDK-created target.

Do not remove RapidJSON in this phase. A short dual-dependency transition keeps
each later change reviewable and allows direct differential tests.

## Phase 2: Final public boundary, internal JSON layer, and differential tests

1. Implement the final vendor-neutral `ExtrasDocument` and public string-based
   schema-validation boundary decided in Phase 0.
2. Introduce private `JsonValue`/`JsonDocument` aliases or wrappers and narrow
   helpers under a non-installed internal include path.
3. Centralize parse options, maximum nesting, numeric conversion, serialization
   options, JSON Pointer behavior, and exception translation.
4. Add a differential test fixture that runs carefully chosen JSON through the
   RapidJSON path and the new path.
5. Capture behavior for malformed input, BOMs, duplicate keys, UTF-8, numbers,
   nesting, compact output, and pretty output.
6. Preserve the current `SchemaFlags::DisableSchemaAccessorSparse` /
   `DisableSchemaAccessorSparseValues` numeric alias and coupled behavior, with
   a regression test. Treat any future correction as a separate API change.

Exit criterion: consumer-facing public headers compile without either JSON
vendor, the adapter contract is stable, and candidate-specific code is not
spreading into public headers.

## Phase 3: Parsing, schema validation, and deserialization

Schema validation and deserialization must migrate as one coherent vertical
slice. This avoids serializing/reparsing between a RapidJSON DOM and the
replacement DOM or running two parsers with different number/duplicate-key
semantics.

1. Make the replacement DOM the authoritative parse for string and stream
   overloads.
2. Implement schema parsing and per-validation caching with the selected
   validator. Do not globally cache by URI: different `ISchemaLocator`
   instances and `SchemaFlags` can define different schema universes for the
   same URI.
3. Adapt `ISchemaLocator` to the validator's external-reference callback.
4. Confirm Draft-04 is selected from the schema dialect, not silently treated
   as a newer draft; explicitly force Draft-04 for schemas such as substituted
   `{}` that omit `$schema`.
5. Preserve the current `{}` substitution used by every `SchemaFlags` value.
6. Translate validation results to `ValidationException`, preserving document
   pointer and failing keyword.
7. Port `GLTFSDK/Source/Deserialize.cpp` to the same authoritative DOM.
8. Replace iterator-shaped helpers with explicit optional member access; do not
   emulate unsafe end iterators.
9. Keep explicit object/array/number checks even when schema validation normally
   catches invalid values, because schema validation can be disabled.
10. Preserve string and stream overloads and all legacy `SchemaFlags` behavior.
11. Port extension-schema tests, nested `$ref` tests, missing dependency tests,
    unknown URI tests, and malformed schema tests.
12. Add conformance cases from the Draft-04 JSON Schema Test Suite for every
   keyword used by the bundled glTF schemas.
13. Add resolver cases for nested paths and `../`, external fragments, Draft-04
    `id` scope changes, repeated/cyclic references, absent `$schema`, locator
    failures, and two locators returning different content for the same URI.
14. Differentially test representative valid and invalid glTF documents.

Exit criterion: parsing, schema validation, and core deserialization have no
RapidJSON references and produce the approved SDK objects, validation results,
and failure categories.

## Phase 4: Serialization and extension handlers

1. Port `GLTFSDK/Source/Serialize.cpp`.
2. Use the insertion-ordered DOM type/configuration selected in Phase 0.
3. Parse and attach raw extension/extras JSON without changing its structure.
4. Port `GLTFSDK/Source/ExtensionsKHR.cpp` in groups, running focused extension
   round-trip tests after each group.
5. Preserve compact/pretty modes, deterministic member order, and checked index
   conversion.

Exit criterion: all serializer and KHR extension tests pass, and approved
formatting compatibility is met.

## Phase 5: Downstream migration

Coordinate at least the known `bigpark.canvas.native.lib` consumers:

1. Replace direct `RapidJsonUtils.h` and DOM use in
   `Asset3DToGLTFConverter.cpp`.
2. Replace `Serialize(extras.GetDocument())` with the new
   `ExtrasDocument` serialization API.
3. Replace `GetDocument().HasMember(...)` with `ExtrasDocument::HasMember(...)`.
4. Build and test downstream binaries against the SDK package exactly as they
   consume it, not only against in-tree headers.

If arbitrary extension construction is a common downstream need, decide
whether to add a small SDK-owned JSON builder or require consumers to supply
the already-supported serialized extension string. Do not expose the selected
vendor DOM solely for this use case.

## Phase 6: Remove RapidJSON and harden

1. Delete `External/RapidJSON`, all three patches, and RapidJSON CMake logic.
2. Remove the Windows CI policy workaround if no longer required.
3. Remove RapidJSON-specific notices and install output.
4. Search source, build scripts, tests, documentation, and generated package
   metadata for remaining references.
5. Run sanitizer tests and malformed-input regression tests.
6. Run the final performance, memory, binary-size, and compile-time comparison.
7. Document dependency acquisition, selected version, update procedure, and
   security-monitoring ownership.

## Validation matrix

| Dimension | Required coverage |
|---|---|
| Platforms | Windows x64/Win32/ARM64, Linux, macOS, iOS device/simulator, Android `armeabi-v7a`/`arm64-v8a`/`x86_64` |
| Build modes | Debug and RelWithDebInfo; installed dependency and fallback dependency paths |
| Parse inputs | String, stream, BOM/no BOM, malformed syntax, invalid UTF-8, duplicate keys, excessive nesting |
| Numbers | Signed/unsigned bounds, fractional-to-integer rejection, large integers, float/double, exponent, negative zero, non-finite rejection |
| DOM | Missing members, wrong types, arrays, insertion order, copying/moving parsed extension and extras values |
| Pointer | Get, missing path, escaped tokens, arrays, create path, incompatible existing type |
| Schema | Draft-04, all used keywords, external relative/nested/fragment `$ref`, Draft-04 `id` scopes, cycles, distinct locators, disabled schemas, invalid schema, missing URI, stable diagnostics |
| Output | Compact, four-space pretty, escaping, float formatting, deterministic ordering, semantic round trip |
| Robustness | Existing CWE regressions, ASAN/UBSAN, large manifests, deep input, allocation failure where testable |
| Downstream | Package install layout, public-header compilation, known native consumers |

## Performance gate

Use representative assets rather than library-published headline benchmarks:

- small common manifests;
- large manifests;
- extension-heavy manifests;
- float-heavy manifests;
- malformed/deep adversarial inputs; and
- repeated parse/serialize workloads that expose allocator reuse.

Measure parse time, schema-validation time, write time, peak resident memory,
allocation count if available, binary size, and clean/incremental compile time.
Record the RapidJSON baseline and set candidate thresholds in Phase 0. Repeat
measurements after parsing/schema/deserialization and serialization are ported;
the final phase confirms rather than first discovers the result.

## Completion criteria

- No `rapidjson`, `RapidJSON`, or `RapidJsonUtils` reference remains in shipped
  source, public headers, build logic, package output, or applicable notices.
- No public SDK type exposes the replacement library.
- All supported platforms build with C++14.
- Existing behavior tests and the new compatibility/security tests pass.
- Draft-04 schemas and external extension schemas validate correctly.
- Known downstream consumers compile and pass their focused tests.
- Approved performance and output-compatibility thresholds are met.
- The selected dependency is pinned, licensed, documented, and updateable.
