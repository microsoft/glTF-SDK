# Requirements: Replace RapidJSON with nlohmann/json

Status: requirements complete
Feature: `replace-rapidjson-nlohmann`
Target release: `Release/2.0.0`
Approved baseline: Microsoft `Release/1.9.5`

## 1. Purpose and scope

This feature replaces every production and test dependency on RapidJSON in
glTF-SDK with nlohmann/json 3.12.0, while retaining C++14 and the SDK's
supported platform set. Valijson 1.0.6 is the private JSON Schema Draft-04
validator. The release is intentionally source- and ABI-breaking where the
existing public API exposes RapidJSON.

These requirements are derived from:

- `.ai/replace-rapidjson-nlohmann/goals.md`
- `.ai/00-rapidjson-current-state.md`
- `.ai/01-json-library-evaluation.md`
- `.ai/02-library-agnostic-migration-plan.md`

### 1.1 Approved-goal trace keys

| Goal | Approved outcome |
|---|---|
| G-01 | Base `Release/2.0.0` on Microsoft `Release/1.9.5`. |
| G-02 | Select nlohmann/json 3.12.0 and remove RapidJSON completely. |
| G-03 | Use the prescribed `External/json` layout and CMake target. |
| G-04 | Use Valijson 1.0.6 privately for complete Draft-04 validation. |
| G-05 | Make an intentional source/ABI break without exposing a replacement vendor DOM. |
| G-06 | Preserve glTF parse, deserialize, extension, extras, pointer, and error behavior except approved changes. |
| G-07 | Preserve deterministic compact and four-space pretty serialization. |
| G-08 | Reject duplicate object names and invalid UTF-8. |
| G-09 | Fix compact-stream BOM handling while preserving `IgnoreByteOrderMark`. |
| G-10 | Preserve C++14 and Windows, Linux, macOS, iOS, and Android support. |
| G-11 | Remove RapidJSON build, patch, package, notice, and CI artifacts. |
| G-12 | Update tests and pass the complete suite, including schema regressions. |
| G-13 | Measure and report performance, memory, size, and compile-time deltas. |
| G-14 | Limit implementation changes to glTF-SDK and document adjacent-repository breaks. |
| G-15 | Include licenses, notices, dependency provenance, and update information. |
| G-16 | Commit and push the completed `Release/2.0.0` branch to the specified fork. |

## 2. Compatibility decisions

1. `Release/2.0.0` does not provide source or ABI compatibility for public
   RapidJSON APIs. A compatibility package or deprecation release is not part
   of this feature.
2. Public APIs expose SDK-owned strings and typed values, never
   `nlohmann::json`, `nlohmann::ordered_json`, Valijson types, or equivalent
   vendor types.
3. Existing non-vendor public `Deserialize`, `Serialize`, `ISchemaLocator`,
   `SchemaFlags`, and exception types remain supported unless a requirement
   below explicitly changes their behavior.
4. Existing golden compact/pretty formatting tests are byte-for-byte
   compatibility gates. For JSON not covered by a golden test, deterministic
   member ordering and semantic equivalence are required; harmless lexical
   differences must be documented rather than treated as universal
   byte-for-byte compatibility.
5. Duplicate object member names and invalid UTF-8 are intentionally rejected
   in 2.0.0, even where RapidJSON previously accepted them.
6. The shared numeric value and coupled behavior of
   `SchemaFlags::DisableSchemaAccessorSparse` and
   `SchemaFlags::DisableSchemaAccessorSparseValues` are intentionally
   preserved.
7. No numeric performance-regression threshold has been approved. Completion
   therefore requires reproducible measurements, percentage deltas, and
   explicit review before push rather than an invented threshold.

## 3. Assumptions

- The files in
  `C:\Users\sergioze\Downloads\include\single_include\nlohmann` are the approved
  local source for nlohmann/json and declare version 3.12.0.
- The repository continues to use exceptions and RTTI as in `Release/1.9.5`.
- The root CMake and C++ language baselines are not raised without separate
  approval.
- A finite maximum JSON nesting depth will be selected during architecture or
  implementation, documented, and tested at and beyond its boundary.
- Adjacent repositories may be inspected only to describe migration impact;
  they are not implementation or release-validation targets for this feature.

## 4. Dependency and repository layout

### nlohmann/json

- **REQ-DEP-1:** The repository MUST contain `External/json` with the same
  source layout as `google/jsonnet` `third_party/json`: `LICENSE`,
  `nlohmann/json.hpp`, and a build-definition file, except that Jsonnet's
  `BUILD` file MUST be replaced by `CMakeLists.txt`. Trace: G-02, G-03, G-15.
- **REQ-DEP-2:** `External/json/nlohmann/json.hpp` MUST be sourced from
  `C:\Users\sergioze\Downloads\include\single_include\nlohmann\json.hpp` and
  MUST identify itself as nlohmann/json 3.12.0. Trace: G-02, G-03.
- **REQ-DEP-3:** `External/json` MUST NOT contain Jsonnet's `BUILD` file.
  Trace: G-03.
- **REQ-DEP-4:** `External/json/CMakeLists.txt` MUST declare version 3.12.0 and
  provide a header-only target consumable as
  `nlohmann_json::nlohmann_json`. Trace: G-02, G-03.
- **REQ-DEP-5:** The repository-vendored dependency path MUST build without
  network access during CMake configure or build. Trace: G-03, G-10.
- **REQ-DEP-6:** If an installed nlohmann/json package is supported, only exact
  version 3.12.0 MAY be selected, and installed and vendored paths MUST present
  the same target and behavior to GLTFSDK. The vendored path remains mandatory.
  Trace: G-02, G-03.

### Valijson

- **REQ-DEP-7:** Valijson MUST be pinned to release 1.0.6 in a dedicated
  repository-controlled external layout under `External/Valijson` (or an
  equivalently explicit `External` directory approved by architecture).
  Trace: G-04, G-15.
- **REQ-DEP-8:** GLTFSDK MUST consume Valijson through its nlohmann/json adapter
  in Draft-04 mode. Trace: G-04.
- **REQ-DEP-9:** nlohmann/json and Valijson MUST be private implementation
  dependencies of GLTFSDK; their include directories and types MUST NOT be
  required by an installed SDK consumer. Trace: G-04, G-05.
- **REQ-DEP-10:** Required nlohmann/json and Valijson licenses, copyright
  notices, exact versions, source provenance, and update procedure MUST be
  retained in the repository and applicable distribution notices. Trace:
  G-15.

### RapidJSON removal

- **REQ-DEP-11:** `External/RapidJSON`, its download logic, and all three local
  RapidJSON patches MUST be removed. Trace: G-02, G-11.
- **REQ-DEP-12:** Shipped source, installed public headers, CMake/package
  metadata, CI workarounds, install output, and applicable notices MUST contain
  no RapidJSON dependency or `RapidJsonUtils` reference. Historical planning
  and migration documentation MAY name RapidJSON. Trace: G-02, G-11.
- **REQ-DEP-13:** JSON dependency headers MUST NOT be installed into a public
  SDK dependency folder comparable to `Built/Out/RapidJSON`. Trace: G-05,
  G-11.

## 5. Public API and ABI behavior

- **REQ-API-1:** `Release/2.0.0` MUST remove the public
  `GLTFSDK/RapidJsonUtils.h` header and every public overload, return type,
  parameter, namespace alias, or template contract that exposes RapidJSON.
  Trace: G-02, G-05.
- **REQ-API-2:** No installed public header or exported declaration MUST
  include or name nlohmann/json or Valijson. Trace: G-05.
- **REQ-API-3:** Existing public `Deserialize` string and stream overloads,
  `Serialize(const Document&, ...)` overloads, flags, and SDK exception
  hierarchy MUST remain source-compatible except for the approved strict-input
  changes. Trace: G-05, G-06.
- **REQ-API-4:** The public DOM-taking
  `ValidateDocumentAgainstSchema(const rapidjson::Document&, ...)` API MUST be
  removed and replaced by a public string-based validation API that accepts
  serialized JSON, a schema URI, and ownership of an `ISchemaLocator`. Trace:
  G-04, G-05.
- **REQ-API-5:** `ISchemaLocator` and its URI-to-schema-content role MUST remain
  public and vendor-neutral. Trace: G-04, G-05, G-06.
- **REQ-API-6:** `ExtrasDocument::GetDocument()` MUST be removed. An
  `ExtrasDocument` MUST provide an SDK-owned `ToJson()` operation returning
  deterministic compact JSON and a `HasMember()` operation for object-member
  queries. Trace: G-05, G-06, G-14.
- **REQ-API-7:** `ExtrasDocument` MUST retain construction from serialized JSON
  and its typed root, member, and JSON Pointer get/set capabilities without
  exposing a DOM. Trace: G-05, G-06.
- **REQ-API-8:** `ExtrasDocument` MUST be move-constructible and move-assignable,
  MUST NOT be copyable, and MUST keep implementation-specific state out of its
  public API through exported out-of-line lifetime operations or an equivalent
  stable vendor-neutral boundary. Trace: G-05.
- **REQ-API-9:** The supported typed `ExtrasDocument` scalar contract MUST
  include `bool`, signed and unsigned 32-bit and 64-bit integers, `size_t`,
  `float`, `double`, `std::string`, and `const char*` setter input. Unsupported
  template types MUST fail clearly at compile time. Trace: G-05, G-06.
- **REQ-API-10:** Existing `SchemaFlags` names and numeric values MUST remain
  unchanged, including the `0x80` alias and coupled disabling behavior for
  sparse and sparse-values schemas. Trace: G-06.
- **REQ-API-11:** Public-header consumer tests MUST compile using only the
  installed GLTFSDK include/library package and MUST require no JSON-vendor
  include path or link target. Trace: G-05, G-10.
- **REQ-API-12:** Release documentation MUST identify removed declarations,
  replacement APIs, and representative before/after migration examples.
  Trace: G-05, G-14.

## 6. JSON parsing and typed access

- **REQ-PARSE-1:** nlohmann/json 3.12.0 MUST be the authoritative parser and
  mutable JSON representation for production glTF, extension, extras, and
  schema JSON paths. Trace: G-02, G-04, G-06.
- **REQ-PARSE-2:** String and stream deserialization MUST apply identical parse
  policy and produce equivalent SDK documents or equivalent SDK exception
  categories for the same bytes. Trace: G-06, G-09.
- **REQ-PARSE-3:** Parsing MUST reject comments, trailing commas, malformed
  syntax, non-finite JSON number spellings, duplicate object member names, and
  invalid UTF-8. Trace: G-06, G-08.
- **REQ-PARSE-4:** Duplicate-member and UTF-8 rejection MUST apply even when
  `SchemaFlags::DisableSchemaRoot` disables schema validation. Trace: G-08.
- **REQ-PARSE-5:** A UTF-8 BOM MUST be accepted only when
  `DeserializeFlags::IgnoreByteOrderMark` is supplied; otherwise it MUST cause
  `GLTFException`. This behavior MUST hold for string and stream overloads,
  including compact JSON whose first `{` immediately follows the BOM. Trace:
  G-06, G-09.
- **REQ-PARSE-6:** Malformed JSON and JSON-vendor failures visible to consumers
  MUST be translated to SDK exception types and MUST NOT leak
  `nlohmann::json` exception types. Trace: G-05, G-06.
- **REQ-PARSE-7:** The parser MUST enforce one documented finite maximum
  nesting depth without stack exhaustion; inputs at the supported boundary and
  inputs exceeding it MUST have tests. Trace: G-06, G-10, G-12.
- **REQ-PARSE-8:** Typed extraction MUST distinguish signed integers, unsigned
  integers, floating values, and booleans and MUST enforce the exact range and
  integrality required by each glTF field. Trace: G-06.
- **REQ-PARSE-9:** A fractional number or an integer outside a destination
  type's range MUST NOT be silently accepted as that integer type. Trace:
  G-06.
- **REQ-PARSE-10:** Deserialization with schema validation disabled MUST still
  perform the existing defensive object, array, required-member, fixed-length
  array, and numeric-element checks and throw stable SDK exception categories
  rather than invoking undefined behavior. Trace: G-06, G-12.
- **REQ-PARSE-11:** Valid extension and extras JSON with unique member names
  MUST remain structurally and semantically round-trippable. Trace: G-06,
  G-07.

## 7. Serialization and ordering

- **REQ-SER-1:** Production object construction MUST preserve insertion order
  so existing deterministic output ordering remains stable. Trace: G-06,
  G-07.
- **REQ-SER-2:** `SerializeFlags::None` MUST produce compact JSON and
  `SerializeFlags::Pretty` MUST produce four-space-indented JSON matching all
  existing golden formatting tests. Trace: G-07.
- **REQ-SER-3:** Existing output-member order for core documents, KHR
  extensions, unregistered extensions, and extras MUST be preserved where
  asserted by tests or documented snapshots. Trace: G-06, G-07.
- **REQ-SER-4:** Serialization MUST preserve JSON number category and required
  precision, including floating output for values such as `1.0f`, signed and
  unsigned 64-bit bounds, negative zero, exponent-form inputs, and values near
  float/double precision boundaries. Trace: G-06, G-07.
- **REQ-SER-5:** String escaping and UTF-8 output MUST produce valid JSON and
  MUST not emit or silently replace invalid UTF-8. Trace: G-07, G-08.
- **REQ-SER-6:** Parsing and attaching serialized extension or extras content
  MUST preserve its JSON structure and insertion order, subject to compacting
  whitespace and the approved strict-input policy. Trace: G-06, G-07, G-08.
- **REQ-SER-7:** Serialization errors MUST be reported through SDK exception
  types and MUST NOT expose vendor exception types. Trace: G-05, G-06.
- **REQ-SER-8:** Repeated serialization of the same SDK document in the same
  mode MUST produce byte-identical output. Trace: G-07.

## 8. `ExtrasDocument` and JSON Pointer behavior

- **REQ-PTR-1:** Missing root/member/pointer values and wrong scalar types MUST
  return the caller-provided default, including member lookup on a non-object
  root. Trace: G-06.
- **REQ-PTR-2:** A null/unassigned `ExtrasDocument` value MAY adopt the first
  assigned JSON type; later assignments MAY replace values only within the
  same JSON type category and MUST throw `GLTFException` for an incompatible
  category. Trace: G-06.
- **REQ-PTR-3:** Member setters MUST initialize a null root as an object,
  update existing members without changing their insertion position, and
  reject a non-object root. Trace: G-06, G-07.
- **REQ-PTR-4:** JSON Pointer lookup MUST support object keys, array indices,
  escaped `~0` and `~1` tokens, the root pointer, and missing paths. Trace:
  G-06.
- **REQ-PTR-5:** JSON Pointer set/create MUST create required intermediate
  objects and arrays for supported paths and preserve existing compatible
  values. Trace: G-06.
- **REQ-PTR-6:** Invalid pointer syntax, invalid array indices, or traversal
  through an incompatible existing value MUST produce a stable
  `GLTFException`, not a vendor exception or process termination. Trace:
  G-05, G-06.

## 9. JSON Schema Draft-04 validation

- **REQ-SCH-1:** Valijson 1.0.6 MUST validate schemas as JSON Schema Draft-04;
  no schema may be silently evaluated as Draft-06, Draft-07, or a later
  dialect. Trace: G-04.
- **REQ-SCH-2:** All 33 bundled schemas MUST parse and compile, MUST retain
  their Draft-04 declarations, and the complete root `glTF.schema.json`
  dependency graph MUST validate known-valid and known-invalid glTF documents.
  Trace: G-04, G-06, G-12.
- **REQ-SCH-3:** Validation MUST correctly implement every assertion keyword
  used by the bundled graph, including `$ref`, `allOf`, `anyOf`, `oneOf`,
  `not`, `type`, `properties`, `additionalProperties`, `required`,
  `dependencies`, `items`, `minItems`, `maxItems`, `uniqueItems`,
  `minProperties`, `minimum`, `maximum`, `exclusiveMinimum`, `multipleOf`,
  `pattern`, `enum`, and applicable Draft-04 `format` behavior. Trace: G-04,
  G-12.
- **REQ-SCH-4:** Relative external `$ref` URIs MUST resolve through the
  caller-provided `ISchemaLocator`, including nested paths, `../`, external
  fragments, and Draft-04 `id` scope changes. Trace: G-04, G-06.
- **REQ-SCH-5:** Extension schemas MUST be able to use their own locator and
  reference bundled core glTF schemas. Trace: G-04, G-06.
- **REQ-SCH-6:** Repeated and cyclic references MUST terminate safely and
  produce the correct validation result or a defined SDK exception. Trace:
  G-04, G-12.
- **REQ-SCH-7:** Schema caches MUST be scoped so two different
  `ISchemaLocator` instances may return different schema content for the same
  URI without cross-contamination. Trace: G-04, G-06.
- **REQ-SCH-8:** Every existing `SchemaFlags` value MUST continue to substitute
  a permissive `{}` schema for its associated URI, and the substituted schema
  MUST be treated as Draft-04 despite omitting `$schema`. Trace: G-04, G-06.
- **REQ-SCH-9:** `SchemaFlags::DisableSchemaRoot` MUST disable schema
  assertions but MUST NOT disable strict JSON parsing or semantic
  deserialization checks. Trace: G-06, G-08.
- **REQ-SCH-10:** A null schema locator MUST throw `GLTFException`; malformed
  document JSON, malformed schema JSON, unknown/missing schema URIs, locator
  failures, and schema-validation failures MUST remain distinguishable by
  stable SDK exception type and message category. Trace: G-04, G-06.
- **REQ-SCH-11:** A schema-validation failure MUST throw
  `ValidationException` and report at least the failing document JSON Pointer
  and failing keyword. Existing asserted diagnostics such as
  `Schema violation at #/accessors/0/count due to minimum` MUST remain exact.
  Trace: G-04, G-06, G-12.
- **REQ-SCH-12:** Missing dependent properties and other malformed inputs
  covered by the existing RapidJSON CWE regressions MUST fail without crash,
  null dereference, sanitizer finding, or undefined behavior. Trace: G-11,
  G-12.
- **REQ-SCH-13:** The relevant official Draft-04 JSON Schema Test Suite cases
  for every keyword and reference behavior used by glTF-SDK MUST pass.
  Disabled or weakened upstream Valijson reference tests do not satisfy this
  requirement. Trace: G-04, G-12.
- **REQ-SCH-14:** Any SDK-side integration correction required for Valijson
  1.0.6 external-reference conformance MUST retain full validation strength
  and C++14; skipping failing cases or raising the language standard is not an
  acceptable resolution. Trace: G-04, G-10, G-12.
- **REQ-SCH-15:** Normal deserialization MUST parse a manifest once for
  validation and typed deserialization; the new public string validation API
  MAY parse independently when called directly. Trace: G-04, G-06, G-13.

## 10. Build and platform requirements

- **REQ-BLD-1:** All production and test code MUST compile as C++14; the
  migration MUST NOT require C++17 or later. Trace: G-10.
- **REQ-BLD-2:** The root CMake minimum and supported toolchain/deployment
  baselines MUST not be raised without explicit separate approval. Trace:
  G-10.
- **REQ-BLD-3:** GLTFSDK and its install target MUST build in Debug and
  RelWithDebInfo on Windows x64, Win32, and ARM64. Trace: G-10.
- **REQ-BLD-4:** GLTFSDK and its install target MUST build in Debug and
  RelWithDebInfo on Linux and macOS. Trace: G-10.
- **REQ-BLD-5:** GLTFSDK MUST cross-compile and link for iOS device and
  simulator using the repository toolchain and deployment settings. Trace:
  G-10.
- **REQ-BLD-6:** GLTFSDK MUST cross-compile and link for Android
  `armeabi-v7a`, `arm64-v8a`, and `x86_64` using the repository's supported
  Android configuration. Trace: G-10.
- **REQ-BLD-7:** Unit tests MUST execute on every host architecture currently
  capable of executing them; cross-compile-only targets MUST at least complete
  configure, compile, link, and install. Trace: G-10, G-12.
- **REQ-BLD-8:** The Windows CMake policy workaround whose sole purpose is
  vendored RapidJSON MUST be removed, and all Windows configurations MUST
  still pass. Trace: G-11.
- **REQ-BLD-9:** The configured build MUST not download RapidJSON,
  nlohmann/json, or Valijson at configure or build time. Trace: G-03, G-04,
  G-11.
- **REQ-BLD-10:** The installed package MUST contain GLTFSDK public artifacts
  and required notices but no private JSON-library headers or public
  transitive JSON-library dependency. Trace: G-05, G-11, G-15.
- **REQ-BLD-11:** Linux ASAN/UBSAN configuration and the malformed/deep-input
  regression suite MUST complete without sanitizer findings. Trace: G-12.
- **REQ-BLD-12:** CI MUST gate `Release/2.0.0` pushes and pull requests on the
  complete required Windows, Linux, macOS, iOS, Android, and sanitizer matrix.
  Trace: G-10, G-12, G-16.

## 11. Test requirements

- **REQ-TST-1:** All existing GLTFSDK unit tests MUST pass after migration,
  with tests changed only where the approved public API or strict-input policy
  requires it. Trace: G-06, G-12.
- **REQ-TST-2:** Tests MUST cover strict parsing for string and stream inputs,
  comments, trailing commas, malformed syntax, duplicate names, invalid UTF-8,
  non-finite numbers, excessive nesting, and all BOM/flag combinations. Trace:
  G-08, G-09, G-12.
- **REQ-TST-3:** Tests MUST cover signed/unsigned 32/64-bit boundaries,
  `size_t`, fractional-to-integer rejection, overflow, float/double precision,
  exponent forms, negative zero, and large-integer serialization. Trace: G-06,
  G-12.
- **REQ-TST-4:** Tests MUST cover compact and pretty golden output, stable
  member order, escaping, repeated deterministic serialization, and semantic
  round trips. Trace: G-07, G-12.
- **REQ-TST-5:** Tests MUST cover every public `ExtrasDocument` supported type,
  move behavior, `ToJson`, `HasMember`, defaults, incompatible reassignment,
  and JSON Pointer lookup/create/error behavior. Trace: G-05, G-06, G-12.
- **REQ-TST-6:** Tests MUST cover core schema validation, all `SchemaFlags`,
  the legacy sparse flag alias, extension locators, external references,
  invalid schemas, unknown URIs, stable diagnostics, and CWE regressions.
  Trace: G-04, G-06, G-12.
- **REQ-TST-7:** Tests MUST cover all production KHR extension parse,
  serialization, schema, and round-trip paths migrated from RapidJSON. Trace:
  G-06, G-12.
- **REQ-TST-8:** A public-consumer compile/link test MUST use installed headers
  and libraries and MUST demonstrate that no RapidJSON, nlohmann/json, or
  Valijson dependency is exposed. Trace: G-05, G-10, G-12.
- **REQ-TST-9:** A repository-wide completion scan MUST verify that no shipped
  source, public header, build file, package output, or current notice retains
  a RapidJSON dependency. Trace: G-02, G-11, G-12.
- **REQ-TST-10:** Test results for every required platform/configuration gate
  MUST be recorded or linked in the completion evidence. Trace: G-10, G-12,
  G-16.

## 12. Benchmark and size requirements

- **REQ-PERF-1:** A RapidJSON baseline from Microsoft `Release/1.9.5` and a
  nlohmann/json result from `Release/2.0.0` MUST be measured with equivalent
  toolchains, configurations, compiler flags, and workloads. Trace: G-01,
  G-13.
- **REQ-PERF-2:** Workloads MUST include small common manifests, large
  manifests, extension-heavy manifests, float-heavy manifests, malformed/deep
  adversarial inputs, and repeated parse/validate/serialize operations. Trace:
  G-13.
- **REQ-PERF-3:** The report MUST include absolute values and percentage deltas
  for parse time, Draft-04 validation time, write time, peak resident memory,
  produced binary size, clean compile time, and incremental compile time;
  allocation counts SHOULD be included where the platform provides a
  reproducible method. Trace: G-13.
- **REQ-PERF-4:** Benchmarks MUST distinguish parsing, validation, writing, and
  combined end-to-end workloads so Valijson and ordered-object costs are
  visible. Trace: G-04, G-13.
- **REQ-PERF-5:** Benchmark commands, assets, machine/toolchain identity, run
  count, aggregation method, and raw or summarized results MUST be recorded
  sufficiently for repetition. Trace: G-13.
- **REQ-PERF-6:** Final push MUST not occur until the measured deltas have been
  reviewed and any accepted regressions are documented in release evidence.
  Trace: G-13, G-16.

## 13. Documentation and downstream impact

- **REQ-DOC-1:** Build and dependency documentation MUST identify
  nlohmann/json 3.12.0 and Valijson 1.0.6, their private status, source
  locations, licenses, and update process. Trace: G-02, G-04, G-15.
- **REQ-DOC-2:** Release/migration documentation MUST state that 2.0.0 is
  intentionally source- and ABI-breaking and list the removal of
  `RapidJsonUtils.h`, `ExtrasDocument::GetDocument()`, and the DOM-taking schema
  validator. Trace: G-05, G-14.
- **REQ-DOC-3:** Migration documentation MUST describe replacement usage for
  serializing extras, checking extras members, typed extras access, JSON
  Pointer access, and string-based schema validation. Trace: G-05, G-14.
- **REQ-DOC-4:** Known adjacent-repository source breaks, including direct
  `RapidJsonUtils.h` use, `Serialize(extras.GetDocument())`, and
  `GetDocument().HasMember(...)`, MUST be documented without modifying those
  repositories. Trace: G-14.
- **REQ-DOC-5:** Release notes MUST call out duplicate-name rejection,
  invalid-UTF-8 rejection, corrected compact-stream BOM behavior, retained
  sparse schema-flag alias, and any approved lexical serialization differences.
  Trace: G-06, G-08, G-09.
- **REQ-DOC-6:** `thirdPartyNotices.txt` and equivalent distribution notices
  MUST remove RapidJSON-only material that no longer applies and add all
  required nlohmann/json and Valijson material without removing notices still
  required by schemas or tests. Trace: G-11, G-15.

## 14. Release completion

- **REQ-REL-1:** `Release/2.0.0` MUST be based on the commit represented by
  Microsoft `Release/1.9.5`, with that upstream relationship recorded in
  release evidence. Trace: G-01.
- **REQ-REL-2:** All dependency, API, parser, serializer, pointer, schema,
  platform, test, documentation, and benchmark requirements above MUST have
  completion evidence before release. Trace: G-02 through G-15.
- **REQ-REL-3:** The completed changes MUST be committed on
  `Release/2.0.0`; the completion commit MUST contain no unrelated adjacent
  repository changes or generated build output. Trace: G-14, G-16.
- **REQ-REL-4:** `Release/2.0.0` MUST be pushed successfully to
  `git@github.com:SergioRZMasson/glTF-SDK.git`. Trace: G-16.
- **REQ-REL-5:** Completion evidence MUST verify that the remote
  `Release/2.0.0` ref contains the completion commit and that all required CI
  gates are passing. Trace: G-12, G-16.

## 15. Explicit non-goals

1. Modifying, committing, branching, or pushing any adjacent repository.
2. Preserving source or ABI compatibility for public RapidJSON APIs.
3. Shipping a RapidJSON compatibility layer or a staged deprecation release.
4. Exposing nlohmann/json or Valijson as part of the SDK public API.
5. Raising the language standard above C++14.
6. Correcting the duplicate `0x80` `SchemaFlags` values.
7. Changing the glTF 2.0 data model or bundled schema semantics.
8. Replacing Valijson with a Draft-07-only validator or weakening Draft-04
   external-reference validation.
9. Broad serializer redesign, unrelated performance work, or unrelated defect
   repair.
10. Guaranteeing byte-for-byte output compatibility for JSON not covered by an
    approved golden-output contract.

## 16. Acceptance criteria summary

| Area | Acceptance evidence | Requirements |
|---|---|---|
| Dependencies | Exact vendored layouts, versions, targets, licenses, offline configure | REQ-DEP-1..13 |
| Public API/ABI | Vendor-free installed headers, replacement APIs, consumer compile test | REQ-API-1..12 |
| Parsing | Strict-input, BOM, depth, numeric, and exception tests pass | REQ-PARSE-1..11 |
| Serialization | Golden compact/pretty output and deterministic round trips pass | REQ-SER-1..8 |
| Extras/pointers | Typed, move, member, pointer, and error tests pass | REQ-PTR-1..6 |
| Draft-04 schema | Bundled graph, official keyword subset, external refs, diagnostics, CWE tests pass | REQ-SCH-1..15 |
| Platforms/build | C++14 Debug/RelWithDebInfo matrix, mobile cross-builds, sanitizer, install pass | REQ-BLD-1..12 |
| Tests | Complete existing and migration regression suites pass with recorded results | REQ-TST-1..10 |
| Benchmarks | Reproducible baseline/candidate report reviewed before push | REQ-PERF-1..6 |
| Documentation | Migration, downstream breaks, licenses/notices, behavior changes documented | REQ-DOC-1..6 |
| Release | Correct base, clean completion commit, remote push, passing CI verified | REQ-REL-1..5 |

## 17. Requirement blockers and deferred values

There is no blocker to architecture or implementation planning. Two numeric
values are intentionally not invented by this requirements phase:

1. the finite maximum nesting depth required by `REQ-PARSE-7`; and
2. quantitative acceptable-regression thresholds for `REQ-PERF-6`.

Architecture/implementation MUST select and document the nesting limit.
Benchmark deltas MUST be measured and reviewed before push because the approved
goals require reporting but do not approve numeric pass/fail thresholds.
