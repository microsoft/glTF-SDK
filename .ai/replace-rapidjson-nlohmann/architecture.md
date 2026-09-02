# Architecture: Replace RapidJSON with nlohmann/json

Status: architecture complete
Feature: `replace-rapidjson-nlohmann`
Target branch: `Release/2.0.0`
Inspected source baseline: `3193f83`, also `upstream/Release/1.9.5`
Language baseline: C++14

## 1. Executive summary

`Release/2.0.0` will replace RapidJSON with one private, authoritative
`nlohmann::ordered_json` DOM and Valijson 1.0.6 for Draft-04 validation. JSON
vendor types will disappear from installed SDK headers. A narrow internal JSON
layer will own strict parsing, checked conversion, ordered construction,
serialization, JSON Pointer behavior, and vendor-exception translation.

Normal deserialization will parse a manifest exactly once. The same ordered DOM
will be passed to the Draft-04 validator through a Valijson adapter and then to
typed glTF deserialization. There will be no RapidJSON/nlohmann conversion and
no default-`json`/`ordered_json` conversion in that path.

| Concern | Before, `Release/1.9.5` | After, `Release/2.0.0` |
|---|---|---|
| Core JSON | RapidJSON DOM exposed through a public catch-all header | Private nlohmann/json 3.12.0 `ordered_json` |
| Ordering | RapidJSON insertion order | `ordered_json` insertion order |
| Schema | RapidJSON Draft-04 schema validator | Private Valijson 1.0.6, forced Draft-04 |
| Public extras API | Returns `rapidjson::Document` | Move-only PImpl, typed operations, `ToJson`, `HasMember` |
| Public validation API | Accepts `rapidjson::Document` | Accepts serialized JSON |
| Parsing | Separate string/stream helpers; stream BOM defect; duplicates and UTF-8 not intentionally rejected | One byte-ingestion and strict-parse policy; duplicate and UTF-8 rejection; corrected BOM behavior |
| Depth | Iterative parse, no SDK limit | Maximum 256 nested containers |
| Build | Installed-package probe plus configure-time RapidJSON download and patches; public include propagation | Offline vendored headers; private interface targets; no dependency installation |
| Output | RapidJSON compact/pretty writer | SDK formatting policy over ordered nlohmann DOM |
| ABI | RapidJSON appears in public declarations and object layout | Intentional 2.0 ABI break; vendor-neutral exported boundary |

This architecture satisfies the approved one-release breaking-change decision.
It does not provide a compatibility shim. (REQ-API-1..12, REQ-PARSE-1..11,
REQ-SER-1..8)

## 2. Evidence and constraints

The inspected tree has 33 bundled schemas, all declaring
`http://json-schema.org/draft-04/schema`. The root graph contains 85 `$ref`
occurrences. Across the bundled schemas and checked-in `.gltf` test resources,
the deepest observed JSON container nesting is 9.

The approved local `json.hpp`:

- is nlohmann/json 3.12.0;
- has SHA-256
  `AAF127C04CB31C406E5B04A63F1AE89369FCCDE6D8FA7CDDA1ED4F32DFC5DE63`;
- provides `ordered_json` using an insertion-ordered vector-backed map;
- distinguishes signed integer, unsigned integer, and floating categories;
- validates UTF-8 while parsing strings;
- offers strict UTF-8 serialization; and
- supports C++11 and therefore the required C++14 build.

Valijson 1.0.6 is tag `v1.0.6`, commit
`4edda758546436462da479bb8c8514f8a95c35ad`. Its upstream CMake project requires
CMake 3.10 and probes/selects newer language modes, so it cannot be added
unchanged beneath this repository's CMake 3.5 project. Its nlohmann adapter is
hard-coded to `nlohmann::json`, and its Draft-04 `ref` and `refRemote`
conformance tests are disabled. Its URI helper also concatenates relative URIs
rather than performing path resolution. The private integration therefore
requires documented, test-driven corrections; merely adding the upstream
target is not sufficient. (REQ-DEP-7..10, REQ-SCH-1..14, REQ-BLD-1..2)

## 3. Current architecture

```text
Deserialize(string/istream)
          |
          v
 Public RapidJsonUtils.h
  - parse helpers
  - DOM helpers
  - numeric helpers
          |
          v
 rapidjson::Document -------------------------+
          |                                   |
          v                                   v
 RapidJSON SchemaValidator             Deserialize.cpp
   + ISchemaLocator                    ExtensionsKHR.cpp
          |
          v
 typed Microsoft::glTF::Document

typed Document -> Serialize.cpp -> RapidJSON ordered DOM -> Writer/PrettyWriter

ExtrasDocument(public)
  directly owns rapidjson::Document
  exposes GetDocument()
```

The dependency is discovered or downloaded in the root `CMakeLists.txt`, its
include directory is PUBLIC, and its headers are copied to
`Built/Out/RapidJSON`. Production use is concentrated in `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`RapidJsonUtils.h`, and `ExtrasDocument.h`. Tests directly use the vendor DOM
for extension construction and extras serialization.

The current schema provider caches `rapidjson::SchemaDocument` by URI within a
single validation call. It does not model RFC-compliant URI scope explicitly.
The current compact-stream BOM path constructs an encoded stream but parses
the underlying stream wrapper, which can lose the first JSON character.

## 4. Target architecture

```text
                  public SDK boundary
 +-------------------------------------------------------+
 | Deserialize(string/istream)  Serialize(Document)      |
 | ExtrasDocument (PImpl)       Validate...(string, ...) |
 | ISchemaLocator, flags, SDK exceptions                 |
 +--------------------------+----------------------------+
                            |
                            v
             private GLTFSDK JSON subsystem
 +-------------------------------------------------------+
 | ByteInput/BOM policy                                  |
 | StrictParsePolicy: duplicate, UTF-8, depth=256        |
 | JsonValue = nlohmann::ordered_json                    |
 | checked typed access and ordered construction         |
 | JSON Pointer tokenize/get/set                         |
 | compact/pretty deterministic writer                   |
 | vendor exception -> SDK exception translation         |
 +----------------------+----------------+---------------+
                        |                |
                        v                v
              typed deserializer    Draft4ValidationSession
              and serializers       + ordered-json adapter
                                         |
                                         v
                              Valijson 1.0.6 + local
                              conformance corrections
                                         |
                                         v
                                   ISchemaLocator
```

Only private source headers may name `nlohmann` or `valijson`. Production call
sites use SDK-owned `JsonValue` and narrow helper operations. The internal layer
is not a general-purpose facade and will not reproduce vendor iterator or
allocator APIs. (REQ-DEP-9, REQ-API-2, REQ-PARSE-1, REQ-API-11)

## 5. Dependency and CMake model

### 5.1 nlohmann/json layout

The committed layout exactly mirrors Jsonnet's `third_party/json` shape, with
its Bazel file replaced by CMake:

```text
External/json/
|-- CMakeLists.txt
|-- LICENSE
`-- nlohmann/
    `-- json.hpp
```

`json_fwd.hpp` is deliberately not copied: Jsonnet's mirrored target contains
only `nlohmann/json.hpp`, and the approved single header is self-contained.
`LICENSE` is the nlohmann MIT license. `BUILD` must not exist.

`External/json/CMakeLists.txt` has this target model:

| Property | Decision |
|---|---|
| Minimum CMake | 3.5 |
| Project | `nlohmann_json`, version exactly `3.12.0`, no compiled language |
| Real target | `nlohmann_json`, INTERFACE |
| Alias | `nlohmann_json::nlohmann_json` |
| Include path | SYSTEM, build interface only, rooted at `External/json` |
| Install interface/rules | None |
| Network behavior | None |

The root build always adds this vendored directory. It does not call
`find_package(nlohmann_json)`: using only the approved committed header is the
smallest hermetic model and avoids installed-package drift. A compile-time
version assertion in the private JSON implementation verifies 3.12.0 in
addition to the CMake project version. (REQ-DEP-1..6)

### 5.2 Valijson acquisition and target

The repository-controlled layout is:

```text
External/Valijson/
|-- CMakeLists.txt
|-- LICENSE
|-- Authors
|-- UPSTREAM.md
|-- patches/
|   |-- 0001-ordered-nlohmann-adapter.patch
|   |-- 0002-draft04-uri-and-reference-resolution.patch
|   |-- 0003-structured-validation-keywords.patch
|   `-- 0004-replace-subschema-metadata-optionals.patch
`-- include/
    |-- compat/
    |   `-- optional.hpp
    `-- valijson/
        `-- ... headers from v1.0.6 ...
```

Acquisition is an implementation-time maintenance operation, never a CMake
operation: obtain tag `v1.0.6` at commit
`4edda758546436462da479bb8c8514f8a95c35ad`, copy `include/valijson`,
the C++14-required `include/compat/optional.hpp`, `LICENSE`, and `Authors`,
apply the three tracked patches, prune the unused legacy-parser adapter and
utility, and record the source
URL, tag, commit, archive hash, commands, and resulting-tree hash in
`UPSTREAM.md`, and commit the resulting headers. Valijson's examples, tests,
inspector, third-party submodules, bundled nlohmann copy, and upstream
top-level CMake file are not imported.

The patches have narrowly defined purposes:

1. Bind Valijson's nlohmann adapter to `nlohmann::ordered_json`, preserving a
   single DOM.
2. replace placeholder URI concatenation, propagate the external document URI
   as the new scope, support relative paths, `../`, fragments, Draft-04 `id`,
   repeated references, and safe cycle handling;
3. attach the originating JSON Schema keyword to private validation-result
   records so SDK diagnostics are not inferred from English text.

`External/Valijson/CMakeLists.txt` has this target model:

| Property | Decision |
|---|---|
| Minimum CMake | 3.5 |
| Project | `valijson`, version exactly `1.0.6`, no compiled language |
| Real target | `valijson`, INTERFACE |
| Alias | `ValiJSON::valijson` |
| Include path | SYSTEM, build interface only, `External/Valijson/include` |
| Compile definition | `VALIJSON_USE_EXCEPTIONS=1` |
| Test/example options | Not present |
| Install interface/rules | None |
| Network behavior | None |

`GLTFSDK` links both interface targets PRIVATE. Neither target is exported,
installed, copied to `Built/Out`, or added to a public link interface. The
existing library kind and `GLTFSDK_API` definition remain unchanged.
(REQ-DEP-7..10, REQ-BLD-9..10)

### 5.3 SDK install boundary

The GLTFSDK install step will install the SDK library/PDB as today, the public
`Inc/GLTFSDK` headers under the GLTFSDK output's `Inc/GLTFSDK` directory, and
applicable license/notice files. It will not install `External/json`,
`External/Valijson`, private JSON headers, CMake dependency targets, or an
equivalent of `Built/Out/RapidJSON`. The public-consumer test will compile from
that installed tree rather than source-tree include paths. (REQ-DEP-13,
REQ-API-11, REQ-BLD-10)

## 6. Private JSON subsystem

Private headers live below `GLTFSDK/Source/Internal` and are not installed.
They define one `JsonValue` alias for `nlohmann::ordered_json` and these narrow
families of operations:

- strict parse from a byte string and from an input stream;
- type predicates, required/optional member lookup, and checked scalar access;
- ordered object/array creation, member insertion/update, and append;
- compact and pretty serialization;
- raw extension/extras subtree parse and move;
- JSON Pointer tokenization, lookup, and set/create; and
- conversion of all vendor exceptions to SDK exceptions.

Call sites do not use vendor allocators, iterators, pointers, `operator[]`
insertion, implicit conversions, or unchecked `get<T>()`. Member lookup returns
a nullable `JsonValue` pointer; required lookup throws `InvalidGLTFException`.
Construction helpers preserve the existing source call order. This keeps the
adapter small and prevents a new vendor-shaped public or internal API.
(REQ-PARSE-6, REQ-PARSE-8..10, REQ-SER-1, REQ-SER-7)

### 6.1 Authoritative DOM

`nlohmann::ordered_json` is used for manifests, schemas, extension payloads,
extras, serializer construction, and `ExtrasDocument::Impl`. Default
`nlohmann::json` is not used in production. The Valijson adapter correction
accepts the same ordered type. Consequently:

```text
manifest bytes -> ordered_json (one parse)
                       |             |
                       v             v
                  Valijson      typed deserializer
```

This avoids sorted-key output, a validation-only conversion DOM, differing
duplicate semantics, and number-category drift. (REQ-PARSE-1,
REQ-SCH-15, REQ-SER-1)

### 6.2 Unified byte and BOM policy

Both string and stream overloads feed the same byte-oriented parser. The stream
overload reads all bytes once, checks stream failure, and then calls the same
function as the string overload. This intentionally favors behavioral identity
over a second streaming policy.

Before nlohmann parsing:

1. if the first three bytes are UTF-8 BOM `EF BB BF`, reject unless
   `IgnoreByteOrderMark` is present;
2. with the flag, remove exactly that leading UTF-8 BOM;
3. never remove a BOM elsewhere and never treat the flag as permission for
   UTF-16/UTF-32 input; and
4. parse the remaining byte range without a terminating-NUL assumption.

This prevents nlohmann's own BOM tolerance from weakening the SDK contract and
fixes compact input where `{` immediately follows the BOM. (REQ-PARSE-2,
REQ-PARSE-5)

### 6.3 Strict parse policy

nlohmann parsing uses exceptions enabled, comments disabled, and the default
strict grammar. A parser callback maintains a stack of object-key sets and:

- rejects a key already present in the current object;
- counts object and array containers;
- rejects a container whose one-based nesting level would exceed 256; and
- otherwise retains every value.

nlohmann's lexer rejects malformed UTF-8 in JSON strings. Comments, trailing
commas, malformed syntax, and non-finite spellings remain rejected. The same
policy parses manifests, schemas, extension strings, and extras, including when
root schema validation is disabled. (REQ-PARSE-3..4, REQ-PARSE-7)

The maximum is **256 nested object/array containers**, with the root container
counted as level 1. It is high enough to be 28 times the deepest checked-in
schema/asset depth (9) and eight times Boost.JSON's evaluated default (32),
while bounding parser bookkeeping and the recursive serializer/validator work
on mobile and Win32 stacks. Level 256 succeeds; level 257 throws
`GLTFException`. Pointer-created and programmatically serialized values are
subject to the same limit. This is a documented 2.0 safety limit, not a glTF
data-model limit.

### 6.4 Numeric and scalar policy

The internal DOM preserves nlohmann's three number categories. Checked getters
apply these rules:

| Destination | Accepted JSON category and check |
|---|---|
| `bool` | Boolean only |
| signed 32/64 | Signed integer in range, or unsigned integer no greater than destination max |
| unsigned 32/64 | Unsigned integer in range, or non-negative signed integer in range |
| `size_t` | Integer, non-negative, no greater than `SIZE_MAX` |
| `float` | Any finite JSON number representable as finite `float` |
| `double` | Any finite JSON number representable as finite `double` |
| `std::string` | String only |

Floating-category values, including `1.0`, are not accepted as integer fields.
No out-of-range integer is routed through `double`. All required object, array,
array-length, and element checks remain in typed deserialization even when
schema validation is disabled. (REQ-PARSE-8..10)

## 7. Public API and ABI

### 7.1 `ExtrasDocument`

`ExtrasDocument` remains in `GLTFSDK/ExtrasDocument.h`, but the public header
includes only SDK and standard-library types. Its exact public operations are:

| Operation | Contract |
|---|---|
| default constructor | Creates an unassigned/null extras value |
| explicit constructor from `const char*` | Strictly parses serialized JSON; null pointer is an error |
| explicit constructor from `const std::string&` | Strictly parses serialized JSON |
| destructor | Exported and defined out of line |
| move constructor | Exported, out of line, `noexcept` |
| move assignment | Exported, out of line, `noexcept` |
| copy constructor/assignment | Explicitly deleted |
| `ToJson() const` | Exported; deterministic compact JSON |
| `HasMember(const char*) const` | Exported; false for missing member or non-object root; null name is an error |
| existing typed root getters/setters | Retained without vendor types |
| existing typed member getters/setters | Retained without vendor types |
| existing typed pointer getters/setters | Retained without vendor types |

The object contains only `std::unique_ptr<Impl>`. `Impl` owns the ordered DOM.
All lifetime operations and all non-template dispatch methods are out of line
and marked with the existing `GLTFSDK_API` mechanism. The move operation may
leave the source with a null PImpl; getters then return defaults,
`HasMember` returns false, `ToJson` returns `null`, and a later setter lazily
recreates the implementation. This keeps move operations non-throwing.

The supported getter types are `bool`, `int32_t`, `uint32_t`, `int64_t`,
`uint64_t`, `size_t`, `float`, `double`, and `std::string`. Setter input adds
`const char*` and string literals. Public templates use C++14 traits and tag
dispatch to exported primitive operations; they do not instantiate vendor
code. Unsupported types trigger a targeted compile-time assertion. The trait
design must account for platforms where `size_t` aliases a supported unsigned
integer and must not rely on duplicate explicit specializations.

All numbers belong to one JSON compatibility category for assignment, matching
the former RapidJSON `kNumberType` rule. A null value may adopt any first
assigned category. Later assignment may replace only the same category:
number, boolean, string, object, or array. (REQ-API-6..9, REQ-PTR-1..3,
REQ-TST-5)

### 7.2 JSON Pointer semantics

The SDK tokenizes RFC 6901 pointers rather than exposing
`nlohmann::json_pointer` exceptions:

- empty string denotes the root;
- tokens decode `~0` and `~1`; every other `~` escape is invalid;
- object tokens are literal keys;
- array tokens are canonical decimal indices (`0` or a non-zero digit followed
  by digits); signs, overflow, leading zeroes, and `-` are rejected;
- lookup never creates values and returns the caller default when absent;
- set/create chooses an array for a missing intermediate node when the next
  token is an array index, otherwise an object;
- arrays grow with null placeholders as required;
- existing non-null incompatible traversal throws `GLTFException`; and
- replacing an existing scalar obeys the extras category rule.

Object member updates do not change insertion position. Pointer depth is
included in the 256-level limit. (REQ-PTR-4..6)

### 7.3 Schema-validation API

The removed public overload is the one accepting
`const rapidjson::Document&`. Its exact public replacement is a `void`
`ValidateDocumentAgainstSchema` operation taking, in order:

1. `const std::string& documentJson`;
2. `const std::string& schemaUri`; and
3. `std::unique_ptr<const ISchemaLocator> schemaLocator` by value.

It is exported with `GLTFSDK_API`. `ISchemaLocator`, its
`GetSchemaContent(const std::string&) -> const char*` role, `SchemaFlags`, and
`GetDefaultSchemaLocator` remain public and vendor-neutral. The public function
strictly parses its document once for that direct call. Normal `Deserialize`
uses a private ordered-DOM overload so it does not reparse. (REQ-API-4..5,
REQ-SCH-15)

## 8. Draft-04 validation architecture

### 8.1 Validation session

Each validation call creates a `Draft4ValidationSession` owning:

- the consumed locator;
- the root schema URI;
- a document cache keyed by canonical document URI without a fragment;
- a schema/subschema cache keyed by canonical document URI plus fragment;
- resolution state (`unseen`, `resolving`, `resolved`); and
- parsed ordered schema documents for the life of schema compilation.

Caches are never static or shared across calls. Two locator instances may
therefore return different content for the same URI. `SchemaFlags` remain
implemented by the existing locator's `{}` substitution, including the shared
`0x80` sparse/sparse-values value. Valijson's parser is always constructed as
Draft-04, so substituted `{}` schemas are Draft-04 despite lacking `$schema`.
(REQ-API-10, REQ-SCH-1, REQ-SCH-7..9)

### 8.2 URI and external-reference resolution

Resolution follows this sequence:

```text
current document URI + current Draft-04 id scope
                 |
                 v
 split $ref into URI and fragment
                 |
                 v
 RFC 3986 merge + dot-segment normalization
                 |
       +---------+---------+
       |                   |
 document URI          fragment
 (locator/cache key)   (RFC 6901 lookup)
```

The root `schemaUri` establishes the initial document URI even when the schema
has no `id`. Relative references merge against the current document's
directory; `.` and `..` are normalized. Absolute URIs and URNs replace the
base. A Draft-04 `id` changes scope after resolving relative to the current
scope. Fragments are not passed to `ISchemaLocator`; they select within the
fetched document.

The fetch callback calls the locator once per canonical document URI, rejects a
null content pointer, copies the returned text immediately, and parses it with
the same duplicate/UTF-8/depth policy as a manifest. The cache owns the parsed
document independently of the locator's returned pointer lifetime.

Repeated references reuse cache entries. A subschema is registered as
`resolving` before its children are followed; a legal recursive reference
reuses that allocated subschema. A cycle that cannot resolve to an allocated
schema, a missing fragment, or an invalid pointer is a schema error, not
unbounded recursion. The Valijson reference patch must pass nested path,
`../`, external-fragment, `id`-scope, repeated, and recursive official cases
before production migration. (REQ-SCH-4..6, REQ-SCH-13..14)

### 8.3 Compilation, validation, and diagnostics

Schema compilation uses the corrected ordered-json Valijson adapter,
`SchemaParser::kDraft4`, and exceptions. Validation uses strong types and
strict date/time behavior. The complete 33-schema graph and extension locator
path use this same mechanism. (REQ-SCH-2..3, REQ-SCH-5)

The structured-keyword patch records the constraint keyword with each
`ValidationResults` error. The SDK converts Valijson's context vector to a URI
fragment JSON Pointer by removing `<root>`, converting object/array components
to tokens, and escaping `~` and `/`. The primary diagnostic is the first
emitted error at greatest context depth that has a concrete keyword. This
preserves deterministic leaf failures such as:

`Schema violation at #/accessors/0/count due to minimum`

Failure categories are:

| Failure | SDK result |
|---|---|
| null locator | `GLTFException`: locator must not be null |
| malformed document JSON | `GLTFException`: document JSON parse category |
| locator throws | `GLTFException`: locator failure with URI |
| locator returns null/unknown URI | `GLTFException`: schema could not be located |
| malformed schema JSON | `GLTFException`: schema JSON parse category with URI |
| invalid schema, unresolved ref, invalid fragment, illegal cycle | `GLTFException`: invalid schema category with URI |
| valid schema, invalid instance | `ValidationException` with pointer and keyword |

No nlohmann or Valijson exception crosses the boundary. Exact messages already
asserted by tests remain exact; new category prefixes are stable and tested.
(REQ-SCH-10..12)

## 9. Serialization and extension/extras preservation

The serializer builds `ordered_json` objects in the existing call order. Raw
registered-extension, unregistered-extension, and extras strings are strictly
parsed to ordered subtrees and moved into the parent DOM. They are not converted
through maps, default `json`, or text after parsing. Updating an object member
keeps its original position. (REQ-SER-1, REQ-SER-3, REQ-SER-6)

One SDK-owned formatting entry point defines both modes:

| Mode | Policy |
|---|---|
| compact | no insignificant whitespace |
| pretty | four spaces, `\n` line endings, one space after `:`, no trailing whitespace |

Object and array traversal is ordered and deterministic. Scalar string escaping
and number emission use nlohmann 3.12.0's serializer with UTF-8 strict error
handling and `ensure_ascii=false`; the SDK wrapper controls container
whitespace and translates failures. Existing golden files are the byte-level
authority.

Integer helpers insert signed or unsigned values without a floating
intermediate. Float and double helpers insert the floating category even for
integral-valued values, so `1.0f` remains floating output. Non-finite
programmatic values are rejected before insertion, preventing nlohmann's
default non-finite-to-null behavior. Tests lock negative zero, exponent
normalization, 64-bit limits, float/double boundaries, escaping, and repeated
byte-identical output. Lexical differences outside approved goldens are
documented if semantically harmless. (REQ-SER-2, REQ-SER-4..5, REQ-SER-8)

## 10. Migration sequencing

The production path must never validate one DOM and deserialize another. The
sequence is:

1. **Baseline evidence:** on unmodified `upstream/Release/1.9.5`, capture
   golden outputs, strict/error behavior, tests, sanitizer results, and
   benchmark baselines.
2. **Vendored dependencies:** add the two offline interface targets, notices,
   version checks, and platform compile probes. RapidJSON remains active.
3. **Private JSON and public boundary:** add strict parser/helpers, PImpl
   `ExtrasDocument`, string validation API, public-consumer test, and focused
   tests. Complete all callers of each changed public API in the same commit.
4. **Atomic parse/schema/deserialization vertical slice:** switch string and
   stream parsing, Valijson validation, core deserialization, and extension
   deserialization together to the single ordered DOM. No serialize/reparse or
   RapidJSON fallback is permitted in this slice.
5. **Serialization vertical slice:** switch core serialization, raw
   extension/extras attachment, KHR serialization, and golden tests.
6. **Removal and hardening:** delete RapidJSON and its patches/download/install
   logic, remove the Windows workaround, update notices/docs, run completion
   scans, all platforms, sanitizers, and final benchmarks.
7. **Release:** review measured deltas, create clean completion commits, push,
   and verify the remote ref and CI.

Temporary dual dependencies are allowed only between buildable commits.
Individual production operations always have one authoritative DOM, and the
final tree contains no RapidJSON. Differential coverage is captured as fixtures
and baseline results rather than retained as a final dual-engine test.
(REQ-SCH-15, REQ-TST-1, REQ-REL-2..3)

## 11. Testing and validation strategy

### 11.1 Unit and conformance tests

- strict parse matrix for string/stream, compact BOM, no BOM, flags, malformed
  JSON, comments, trailing commas, duplicate names, invalid UTF-8, non-finite
  spellings, level 256, and level 257;
- signed/unsigned 32/64, `size_t`, fractional integer rejection, overflow,
  float/double boundaries, exponent forms, and negative zero;
- compact/pretty goldens, member order, escaping, raw extension/extras order,
  repeat determinism, and semantic round trips;
- every `ExtrasDocument` supported type, compile-fail unsupported types, PImpl
  move behavior, moved-from behavior, `ToJson`, `HasMember`, defaults, and
  category errors;
- pointer root, objects, arrays, escaped tokens, missing values, intermediate
  creation, sparse array growth, invalid syntax/index, incompatible traversal,
  and depth;
- all existing KHR extension parse/serialize/round-trip paths;
- all 33 bundled schemas, all `SchemaFlags`, shared `0x80` behavior, extension
  locators, malformed schemas, missing URIs, locator exceptions, stable
  diagnostics, and existing CWE regressions;
- relevant official Draft-04 cases for every used keyword plus `ref`,
  `refRemote`, relative/nested/`../` paths, fragments, `id` scopes, repeated
  and recursive refs; and
- a repository/package scan for vendor leakage and RapidJSON remnants.

No disabled external-reference test counts as evidence. (REQ-TST-1..7,
REQ-TST-9, REQ-SCH-13..14)

### 11.2 Public consumer test

A small out-of-tree CMake consumer is configured after `install`. Its only
include root and library path are the GLTFSDK installed output. It includes
`Deserialize.h`, `Serialize.h`, `ExtrasDocument.h`, and `SchemaValidation.h`,
constructs/moves extras, and links a call to the public validation API. It has
no `External` or source-tree path and verifies that no public header names a
JSON vendor. (REQ-API-11, REQ-TST-8)

### 11.3 Platform matrix

| Platform | Required evidence |
|---|---|
| Windows x64, Win32 | Debug/RelWithDebInfo build, install, unit tests, consumer test |
| Windows ARM64 | Debug/RelWithDebInfo configure/build/link/install and consumer compile/link |
| Linux | Debug/RelWithDebInfo build/install/tests/consumer |
| macOS | Debug/RelWithDebInfo build/install/tests/consumer |
| iOS device and simulator | repository toolchain configure/build/link/install |
| Android `armeabi-v7a`, `arm64-v8a`, `x86_64` | supported NDK configure/build/link/install |
| Linux Clang ASAN/UBSAN | complete tests, malformed/deep corpus, no findings |

CI changes the branch gate to `Release/2.0.0`, adds Android and iOS simulator
coverage, retains host-executable tests, and removes only the RapidJSON-specific
Windows policy environment variable. Offline configure is tested because
neither dependency may download. (REQ-BLD-3..12, REQ-TST-10)

## 12. Benchmark strategy

An opt-in benchmark target and scripts use no new benchmark dependency. Both
the upstream baseline and candidate are built with the same machine,
generator, compiler, architecture, configuration, flags, and asset copies.

Workloads include a small common manifest, a large manifest, an
extension-heavy manifest, a float-heavy manifest, malformed/deep adversarial
inputs, and repeated operations. Measurements separate:

- core JSON parse;
- Draft-04 compile/validate;
- typed `Deserialize` with schema off and on;
- core JSON write;
- public `Serialize`; and
- end-to-end deserialize/serialize.

After warm-up, each timing workload runs at least 30 samples; the report records
median, p95, absolute values, and percentage deltas. Peak resident memory uses
platform process metrics; allocation counts are included where a stable
instrument exists. Binary size records the static library and a representative
linked consumer. Clean and one-file incremental builds are timed from scripted
commands. Raw command lines, assets, hashes, machine/toolchain identity, and
results are retained as release evidence.

No numeric threshold is invented. Performance deltas are a mandatory human
review gate before push; unacceptable results roll back to the last dependency
or vertical-slice checkpoint for targeted redesign. (REQ-PERF-1..6)

## 13. Downstream migration documentation

Adjacent repositories are documented, not changed. The migration guide must
cover these confirmed source breaks:

| Old use | 2.0 replacement |
|---|---|
| `Serialize(extras.GetDocument())` | `extras.ToJson()` |
| `extras.GetDocument().HasMember(name)` | `extras.HasMember(name)` |
| DOM-taking `ValidateDocumentAgainstSchema` | pass the serialized JSON string |
| direct `GLTFSDK/RapidJsonUtils.h` extension construction | construct serialized extension JSON with consumer-owned facilities and store the string in the existing extension map |

Known files include
`CoreUtils/Shared/CoreUtils/cpp/Source/Gltf.cpp`,
`Engine/Products/Components/NativeRenderer/cpp/Source/Engine/Serializing/GLTFSerializerWriter.cpp`,
`Transcoders/Shared/Transcoders.Test/Source/TranscoderGLTF/TranscoderGLTFTests.cpp`,
and
`Transcoders/Shared/Transcoders/Source/ExporterGLTF/Asset3DToGLTFConverter.cpp`.
The last file contains several direct SDK-RapidJSON extension builders.

No public generic JSON builder is added: existing extension storage already
accepts serialized JSON, and exposing another generic builder would recreate
the dependency boundary this release removes. (REQ-DOC-2..5)

## 14. Removal, commits, push, and rollback

### 14.1 Removal plan

Delete:

- `External/RapidJSON` including all three patches and download files;
- `GLTFSDK/Inc/GLTFSDK/RapidJsonUtils.h`;
- root RapidJSON discovery, include propagation, download, and header install;
- vendor-DOM public declarations and test code;
- the Windows RapidJSON CMake policy workaround;
- RapidJSON-only notice text and generated/install artifacts.

Retain notices still required by schemas, GoogleTest, or other code; add
nlohmann and Valijson notices and provenance/update instructions. Final scans
exclude historical `.ai` planning and migration documentation but fail on
shipped source, public headers, build metadata, package output, current
notices, or CI references. (REQ-DEP-11..13, REQ-DOC-1, REQ-DOC-6)

### 14.2 Commit and push model

Commits are organized by the seven migration stages and remain buildable. The
completion commit contains only glTF-SDK changes and no generated `Built`
output. Before push:

1. verify `Release/2.0.0` ancestry contains `3193f83` /
   `upstream/Release/1.9.5`;
2. record all platform, conformance, sanitizer, scan, and benchmark evidence;
3. obtain review of performance and documented lexical differences;
4. verify a clean index/worktree except intentional documentation; and
5. push `Release/2.0.0` to
   `git@github.com:SergioRZMasson/glTF-SDK.git`, then verify the remote ref and
   required CI.

This architecture phase itself performs none of those repository operations.
(REQ-REL-1..5)

### 14.3 Rollback points

| Point | Gate | Rollback action |
|---|---|---|
| R0 | upstream baseline captured | reset feature work to branch base |
| R1 | both private dependencies compile everywhere | remove dependency commits; RapidJSON path is untouched |
| R2 | ordered adapter and full Draft-04 refs conform | do not begin production parse migration if it fails |
| R3 | parse/schema/deserialization vertical slice passes | revert the whole slice, never ship a mixed validator/deserializer |
| R4 | serializer/extensions pass goldens | revert the serializer slice without changing R3 |
| R5 | RapidJSON removed and all gates pass | restore removal commit only if late package/CI evidence fails |

There is no runtime fallback or dual-engine release mode.

## 15. Risks and mitigations

| Risk | Mitigation |
|---|---|
| Valijson 1.0.6 has disabled Draft-04 reference tests and an upstream C++14 optional-initialization defect | Treat the four private patches and full official ref suite as R2 release blockers; retain RapidJSON until they pass |
| `ordered_json` has linear object lookup | Keep lookups explicit, avoid repeated construction-time searches, and benchmark extension-heavy/large manifests |
| nlohmann parse/memory/compile regression | Separate benchmarks, private single-header inclusion limited to internal translation units, and review gate before push |
| Header-only dependencies increase compile cost | Include vendor headers only from private JSON/schema translation units where practical; PImpl prevents public fan-out |
| Float lexical differences | Preserve floating category, use one formatter, lock existing goldens and numeric edge cases, document only approved non-golden differences |
| Invalid UTF-8 or non-finite output could be replaced | Strict writer mode and pre-insertion finite checks; translate failure rather than replace/null |
| Recursion in serializer/schema validation | Shared 256-level limit, boundary tests, mobile/Win32 builds, ASAN/UBSAN deep-input tests |
| Valijson patch maintenance | Keep pristine provenance, reproducible patches, exact commit/hash, isolated target, and conformance tests |
| `size_t` aliases another unsigned type | C++14 trait/tag design avoids duplicate specializations and tests every target width |
| ABI leakage through templates/PImpl lifetime | Vendor-free template wrappers, exported out-of-line lifetime/dispatch methods, installed-header consumer test |

## 16. Files to create, modify, and delete

### Create

- `External/json/CMakeLists.txt`
- `External/json/LICENSE`
- `External/json/nlohmann/json.hpp`
- `External/Valijson/CMakeLists.txt`
- `External/Valijson/LICENSE`
- `External/Valijson/Authors`
- `External/Valijson/UPSTREAM.md`
- `External/Valijson/patches/0001-ordered-nlohmann-adapter.patch`
- `External/Valijson/patches/0002-draft04-uri-and-reference-resolution.patch`
- `External/Valijson/patches/0003-structured-validation-keywords.patch`
- `External/Valijson/patches/0004-replace-subschema-metadata-optionals.patch`
- `External/Valijson/include/valijson/**`
- `GLTFSDK/Source/Internal/Json.h`
- `GLTFSDK/Source/Internal/JsonSchema.h`
- `GLTFSDK/Source/Json.cpp`
- `GLTFSDK/Source/ExtrasDocument.cpp`
- focused JSON/schema/public-consumer/benchmark test files and CMake directories
- Android CI workflow and release/migration documentation

### Modify

- root `CMakeLists.txt`
- `GLTFSDK/CMakeLists.txt`
- `GLTFSDK/Inc/GLTFSDK/ExtrasDocument.h`
- `GLTFSDK/Inc/GLTFSDK/SchemaValidation.h`
- `GLTFSDK/Source/Deserialize.cpp`
- `GLTFSDK/Source/Serialize.cpp`
- `GLTFSDK/Source/ExtensionsKHR.cpp`
- `GLTFSDK/Source/SchemaValidation.cpp`
- relevant GLTFSDK unit tests and workflow files
- `README.md`, release notes, dependency documentation, and
  `thirdPartyNotices.txt`

### Delete

- `External/RapidJSON/**`
- `GLTFSDK/Inc/GLTFSDK/RapidJsonUtils.h`
- RapidJSON-specific generated/install output before release packaging

## 17. Requirement traceability

| Architecture area | Requirements |
|---|---|
| Exact nlohmann layout, version, target, offline and no install | REQ-DEP-1..6 |
| Exact Valijson source, private target, patches, provenance and notices | REQ-DEP-7..10 |
| RapidJSON removal and no dependency artifacts | REQ-DEP-11..13 |
| Breaking vendor-neutral public boundary | REQ-API-1..5 |
| `ExtrasDocument` API, ABI, move and type contract | REQ-API-6..9 |
| Flags, installed consumer and migration docs | REQ-API-10..12 |
| One ordered DOM, unified strict parse, BOM, depth and exceptions | REQ-PARSE-1..7 |
| Numeric checks, defensive deserialization and round trips | REQ-PARSE-8..11 |
| Ordered deterministic serializer and numeric/UTF-8 strategy | REQ-SER-1..8 |
| Extras defaults, category rules and JSON Pointer | REQ-PTR-1..6 |
| Forced Draft-04 and complete bundled graph/keywords | REQ-SCH-1..3 |
| External refs, extension locators, cycles and scoped caches | REQ-SCH-4..7 |
| Disabled schemas, strict parse, errors and diagnostics | REQ-SCH-8..12 |
| Official conformance, integration fixes and parse-once path | REQ-SCH-13..15 |
| C++14 and unchanged CMake/toolchain baseline | REQ-BLD-1..2 |
| Desktop/mobile build matrix | REQ-BLD-3..7 |
| RapidJSON workaround removal, offline/private install and CI | REQ-BLD-8..12 |
| Existing, regression, extension, consumer and scan tests | REQ-TST-1..9 |
| Recorded platform evidence | REQ-TST-10 |
| Equivalent benchmark method and review gate | REQ-PERF-1..6 |
| Dependency, migration, downstream, release and notice docs | REQ-DOC-1..6 |
| Correct base, evidence, clean commits, push and remote verification | REQ-REL-1..5 |

## 18. Decisions and unresolved release blockers

Architecture decisions are complete:

- use `nlohmann::ordered_json` everywhere internally;
- use Valijson 1.0.6 through a corrected ordered nlohmann adapter;
- enforce a 256-container nesting limit;
- use only offline vendored targets with no installed dependency path;
- make `ExtrasDocument` a move-only PImpl with typed templates;
- replace public DOM validation with serialized-string validation; and
- preserve a single-DOM parse/validate/deserialize path.

There is no blocker to implementation planning. Two release gates remain
deliberately unresolved by architecture:

1. the Valijson reference/URI/cycle corrections must prove full relevant
   Draft-04 conformance on the target platform matrix; and
2. benchmark deltas must be measured and reviewed because no numeric
   acceptance threshold was approved.

RapidJSON must not be removed and the branch must not be pushed until both
gates are closed.
