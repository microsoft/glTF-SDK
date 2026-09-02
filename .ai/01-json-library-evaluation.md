# RapidJSON Replacement: Library Evaluation

Status: ready for preferred-candidate decision; final adoption requires a proof-of-concept
Assessment date: 2026-09-01
Current SDK baseline: C++14 on Windows, Linux, macOS, iOS, and Android

## Executive conclusion

RapidJSON can be completely removed.

**jsoncons is the preferred candidate** because it is the only evaluated,
actively maintained library whose documented feature set satisfies the full
current requirement set in one library family: C++14 compatibility, mutable
and insertion-ordered DOM, parse/write APIs, JSON Pointer get/create
operations, and first-party JSON Schema Draft-04 validation with
external-reference resolution.

This is not yet a final dependency approval. The exact pinned jsoncons release
must pass the selection proof-of-concept across the SDK's actual language,
schema, build, architecture, and performance constraints. Upstream support
claims do not prove the SDK's Win32, Windows ARM64, iOS, or Android
configurations.

The strongest alternatives are conditional:

- **Boost.JSON + Valijson `v1.0.x`** offers the closest RapidJSON-class core
  engine and the best allocator/performance profile, but Valijson's maintained
  C++14 branch disables failing Draft-04 `$ref` conformance tests. This pairing
  is not acceptable without a successful proof-of-concept.
- **nlohmann/json + Valijson `v1.0.x`** is the easiest DOM API to work with but
  carries the same schema risk and weaker performance/allocation behavior.
- **yyjson** is an excellent speed-oriented C engine, but no direct,
  actively-maintained, proven C++14 schema companion covers the SDK's current
  workflow.

simdjson is a specialized parser rather than a full replacement. Glaze is not
compatible with the current C++14/platform baseline.

## Selection requirements

The evaluation gives highest priority to:

1. Active upstream maintenance and an acceptable open-source license.
2. C++14 and all existing SDK target platforms, including Win32 and mobile.
3. Strict JSON parsing from strings and streams.
4. A mutable DOM that preserves insertion order for deterministic output.
5. Checked integer and floating-point handling.
6. Compact and pretty serialization.
7. JSON Pointer lookup and path creation.
8. JSON Schema Draft-04 with relative external `$ref` resolution.
9. Bounded or iterative nesting behavior.
10. A migration that can hide vendor types from public SDK headers.

Performance matters, but a fast parser that cannot replace serialization,
schema validation, and `ExtrasDocument` is not a complete solution.

## Maintenance and licensing

All six primary projects showed recent upstream activity at the assessment
date.

| Project | Latest release by 2026-09-01 | Recent default-branch activity | License |
|---|---|---|---|
| [jsoncons](https://github.com/danielaparker/jsoncons) | [v1.9.0, 2026-08-07](https://github.com/danielaparker/jsoncons/releases/tag/v1.9.0) | [2026-08-14](https://github.com/danielaparker/jsoncons/commit/af497a1c1ea9043f4ad8d799617b7d00d6676153) | [BSL-1.0](https://github.com/danielaparker/jsoncons/blob/master/LICENSE) |
| [Boost.JSON](https://github.com/boostorg/json) | [Boost 1.92.0, 2026-08-12](https://github.com/boostorg/boost/releases/tag/boost-1.92.0) | [2026-08-30](https://github.com/boostorg/json/commit/82398f9a19de39d672b621cc6021995d1edbe8c7) | [BSL-1.0](https://github.com/boostorg/json/blob/develop/LICENSE_1_0.txt) |
| [nlohmann/json](https://github.com/nlohmann/json) | [v3.12.0, 2025-04-11](https://github.com/nlohmann/json/releases/tag/v3.12.0) | [2026-08-31](https://github.com/nlohmann/json/commit/c864bb36da3d35e406ea7cf98fae0f8a13a27383) | [MIT](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT) |
| [yyjson](https://github.com/ibireme/yyjson) | [v0.12.0, 2025-08-18](https://github.com/ibireme/yyjson/releases/tag/0.12.0) | [2026-08-26](https://github.com/ibireme/yyjson/commit/757305bfaf4b8cb3e52c105926dc9105fb785101) | [MIT](https://github.com/ibireme/yyjson/blob/master/LICENSE) |
| [simdjson](https://github.com/simdjson/simdjson) | [v4.6.9, 2026-08-27](https://github.com/simdjson/simdjson/releases/tag/v4.6.9) | [2026-09-01](https://github.com/simdjson/simdjson/commit/06856ec929ebb5668c6e70e3927824c27fedeeff) | [Apache-2.0 or MIT](https://github.com/simdjson/simdjson/blob/master/README.md#L262-L271) |
| [Glaze](https://github.com/stephenberry/glaze) | [v8.3.0, 2026-08-29](https://github.com/stephenberry/glaze/releases/tag/v8.3.0) | [2026-08-31](https://github.com/stephenberry/glaze/commit/a5f515c1216a7172c41b2c15761705c58b0fc616) | [MIT](https://github.com/stephenberry/glaze/blob/main/LICENSE) |

Release recency is not used alone: nlohmann/json and yyjson have older release
tags but recent repository activity.

## Capability matrix

| Candidate | C++14/platform fit | Mutable ordered DOM and writer | Pointer get/create | Draft-04 + external `$ref` | Expected performance fit | Disposition |
|---|---|---|---|---|---|---|
| **jsoncons `ojson`** | Documented C++11 support; SDK target matrix still requires proof | Yes; `ojson` preserves insertion order | Yes, with SDK wrapper for type rules | **Yes, first-party** | Below RapidJSON/Boost in available large-DOM tests; must benchmark glTF | **Preferred pending proof-of-concept** |
| **Boost.JSON + Valijson `v1.0.x`** | Core is C++11 and broadly portable; validator platform coverage is incomplete | Yes; insertion order and strong allocator support | Yes in current Boost.JSON | Conditional: Draft-04 mode and resolver exist, but `$ref` tests are disabled as broken | Close to RapidJSON for core DOM workloads | Conditional proof-of-concept |
| **nlohmann/json `ordered_json` + Valijson `v1.0.x`** | Yes for core; same validator coverage caveat | Yes, but ordered objects use linear lookup | Yes | Same conditional validator risk | Expected material regression versus RapidJSON | Conditional fallback |
| **yyjson + custom/second validator** | C89 core is highly portable | Separate immutable parse and mutable construction DOMs | Available C-style operations; wrapper required | No proven direct companion | Strongest speed-oriented candidate | Architecture experiment only |
| **simdjson** | C++11, but 64-bit-oriented and not a safe Win32 assumption | No mutable DOM equivalent; writer is low-level | No equivalent complete workflow | No | Excellent parsing | Not a complete replacement |
| **Glaze** | **No: C++23 and little-endian only** | Generic DOM exists; primary design is typed serialization | Not enough to close all gaps | Generates schemas; not a replacement validator | Excellent typed-path claims | Disqualified for current baseline |

## Option 1: jsoncons

### Pros

- It is the only evaluated one-library solution for the complete RapidJSON
  removal.
- Header-only and supports C++11, preserving the SDK's C++14 baseline.
- `jsoncons::ojson` preserves insertion order, unlike the default sorted
  `jsoncons::json`.
- The same ordered JSON type can be used directly with the templated JSON Schema
  compiler; no conversion to a second DOM type is required.
- First-party JSON Schema support includes Drafts 4, 6, 7, 2019-09, and 2020-12
  and accepts a user resolver for external schemas.
- JSON Pointer supports mutable/const lookup, create-if-missing, add, and
  replace.
- Supports SAX-like events, cursors, incremental parsing, standard allocators,
  PMR, and separate result/temporary allocators.
- Stores signed 64-bit, unsigned 64-bit, and double values and offers stronger
  lossless-number options if needed.
- Configurable maximum nesting depth; the parser uses heap-backed parser state
  and reports depth exhaustion instead of relying on recursive descent.

### Cons

- Available large-DOM benchmarks place it below RapidJSON and Boost.JSON.
- Comments are accepted and ignored by default, so the SDK must explicitly set
  `allow_comments(false)` to retain strict parsing.
- `ojson` accepts duplicate keys and retains the first occurrence. If duplicate
  rejection follows the migration plan, the SDK needs an event-level check or
  prepass.
- JSON Pointer automatic creation of missing arrays is not fully established by
  the documentation and needs a proof-of-concept.
- The SDK's existing rule that a pointer/member value cannot change JSON type
  must be implemented in the wrapper; it is not native JSON Pointer behavior.
- No-exception configurations terminate for some failures rather than
  consistently returning error objects. This is not a blocker while the SDK
  itself uses exceptions.
- The project is less ubiquitous than nlohmann/json or Boost, so team
  familiarity may be lower.
- Its upstream CMake project requires CMake 3.15 while the SDK declares 3.5.
  The fallback should initially expose pinned headers through an SDK-owned
  interface target rather than unconditionally adding jsoncons's top-level
  CMake project.
- Upstream CI does not prove every SDK target architecture. C++14 compile/link
  and mobile/Windows architecture coverage are mandatory selection gates.

### Fit assessment

This option has the lowest documented architectural risk. Most work is still a
source port, but every current RapidJSON capability has a credible
destination. Mandatory spike areas are C++14 and all target architectures,
output/float compatibility, missing-array pointer creation, exact schema
diagnostics, the complete Draft-04 schema graph, and glTF-specific performance.

Primary sources:

- [Core capabilities and platform CI](https://github.com/danielaparker/jsoncons/blob/master/README.md#L1-L23)
- [Upstream CMake minimum](https://github.com/danielaparker/jsoncons/blob/master/CMakeLists.txt#L1-L8)
- [`ojson` insertion ordering](https://github.com/danielaparker/jsoncons/blob/master/doc/ref/corelib/ojson.md#L6-L17)
- [JSON Schema drafts and resolvers](https://github.com/danielaparker/jsoncons/blob/master/doc/ref/jsonschema/jsonschema.md#L124-L151)
- [`ojson` schema example](https://github.com/danielaparker/jsoncons/blob/master/doc/ref/jsonschema/jsonschema.md#L265-L290)
- [JSON Pointer get/create](https://github.com/danielaparker/jsoncons/blob/master/doc/ref/jsonpointer/get.md#L1-L56)
- [Parser options and nesting](https://github.com/danielaparker/jsoncons/blob/master/doc/ref/corelib/basic_json_options.md#L17-L30)

## Option 2: Boost.JSON plus Valijson `v1.0.x`

### Pros

- Boost.JSON is the closest core replacement for RapidJSON's combination of
  mutable DOM, parsing, streaming/SAX APIs, writing, insertion order, and
  performance.
- C++11 core with wide GCC, Clang, MSVC, little-endian, and big-endian support.
- Strongest allocator story in the evaluated set: memory resources, monotonic
  and static storage, recursive propagation, reusable parser storage, and
  allocation-free configurations.
- Supports exception and error-code APIs.
- Current Boost.JSON has JSON Pointer get/set operations.
- Strict comments/trailing-comma behavior by default and a configurable parser
  depth limit using an explicit parser stack.
- Valijson has native Boost.JSON and nlohmann/json adapters, a Draft-04 mode,
  and callbacks for fetching/freeing external schema documents.

### Cons

- Boost.JSON has no comparable first-party schema validator.
- The maintained C++14 Valijson branch explicitly disables its Draft-04 `ref`
  and `refRemote` conformance tests as broken. External `$ref` is central to all
  33 bundled glTF schemas, so this is a release-blocking risk.
- Current Valijson has stronger tests but requires C++17; adopting it would
  expand the task into a language-baseline migration.
- Valijson `v1.0.x` CI covers Ubuntu only. MSVC, Apple platforms, iOS, and
  Android support are assumptions requiring direct testing.
- This is a two-library solution with two maintenance/security update streams.
- Boost.JSON brings multiple Boost component dependencies even in its
  one-translation-unit mode.
- The default float parser favors speed; `number_precision::precise` should be
  enabled and benchmarked for float-heavy glTF data.
- Duplicate keys are accepted and the last value is retained, unlike
  jsoncons's first-value behavior. A deliberate SDK policy needs custom parser
  handling.
- The default depth limit is 32 and may be too low for arbitrary nested
  `extras`.

### Fit assessment

Choose this only if RapidJSON-class core performance/allocator control is more
important than minimizing dependency and schema risk. Approval should be
conditional on passing the complete Draft-04 schema corpus, external-reference
tests, and every target toolchain.

Primary sources:

- [Boost.JSON compiler/platform support](https://github.com/boostorg/json/blob/develop/README.adoc#L110-L143)
- [Boost.JSON allocators](https://github.com/boostorg/json/blob/develop/doc/pages/allocators/storage_ptr.adoc#L11-L35)
- [Boost.JSON parser options](https://github.com/boostorg/json/blob/develop/include/boost/json/parse_options.hpp#L19-L139)
- [Valijson C++14 branch policy](https://github.com/tristanpenman/valijson/blob/master/README.md#L11-L23)
- [Valijson Boost.JSON adapter](https://github.com/tristanpenman/valijson/blob/master/include/valijson/adapters/boost_json_adapter.hpp#L1-L35)
- [Valijson Draft-04 resolver API](https://github.com/tristanpenman/valijson/blob/v1.0.x/include/valijson/schema_parser.hpp#L30-L124)
- [Disabled Draft-04 reference tests](https://github.com/tristanpenman/valijson/blob/v1.0.x/tests/test_validator.cpp#L414-L425)

## Option 3: nlohmann/json plus Valijson `v1.0.x`

### Pros

- Single-header core library with no core dependencies.
- C++11 and exceptionally broad compiler/platform use.
- The most familiar and ergonomic DOM API in the evaluated set.
- Mutable values, convenient conversions, compact/pretty writing, SAX parsing,
  and JSON Pointer support are readily available.
- Valijson has a native nlohmann/json adapter.
- Comments and trailing commas are rejected by default unless explicitly
  enabled.
- Parsing uses an iterative state stack, avoiding nesting-proportional C++ call
  recursion.

### Cons

- It has no built-in JSON Schema validator and inherits the same Valijson
  Draft-04 `$ref` and platform risks as the Boost option.
- The other common nlohmann schema validator supports Draft-07 in its active
  version. Its Draft-04 branch is from 2021 and fails the active-maintenance
  requirement.
- nlohmann/json explicitly prioritizes ease of use over speed and memory
  efficiency; current common benchmarks show a substantial regression from
  RapidJSON.
- Default objects sort keys. `ordered_json` preserves insertion order but uses
  linear lookup and can make repeated object construction quadratic.
- Its allocator template is not equivalent to RapidJSON document arenas or
  Boost memory resources.
- There is no built-in maximum nesting limit; a parser callback must impose one.
- Duplicate-key retention is documented as unspecified unless a callback
  rejects duplicates.
- Disabling exceptions generally turns failures into process aborts.

### Fit assessment

This is the lowest API-learning-cost option, not the lowest project-risk
option. It is reasonable only if implementation ergonomics and ecosystem
familiarity outweigh performance/allocator concerns and the Valijson
proof-of-concept succeeds.

Primary sources:

- [nlohmann/json design and baseline](https://github.com/nlohmann/json/blob/develop/README.md#L60-L72)
- [Object ordering tradeoff](https://github.com/nlohmann/json/blob/develop/docs/mkdocs/docs/features/object_order.md#L7-L59)
- [Number behavior](https://github.com/nlohmann/json/blob/develop/docs/mkdocs/docs/features/types/number_handling.md#L59-L100)
- [Duplicate-key rejection callback](https://github.com/nlohmann/json/blob/develop/docs/mkdocs/docs/features/parsing/parser_callbacks.md#L93-L122)
- [Active schema validator is Draft-07](https://github.com/pboettch/json-schema-validator/blob/main/README.md#L20-L28)

## Option 4: yyjson plus a separate schema strategy

### Pros

- Very small integration surface: one C header and one C source file.
- C89 and tested across a broad set of compilers, CPU architectures, and WASM.
- Strong parsing/writing performance; the best speed-oriented candidate in the
  evaluated DOM group.
- Strict by default, with explicit optional extensions.
- Correctly rounded 64-bit signed/unsigned/double number handling.
- Supports custom, pool, and dynamic allocators.
- Explicit error structures avoid leaking vendor exceptions across the SDK.

### Cons

- It is a C API, so the SDK needs a carefully owned RAII wrapper.
- Parsed documents are immutable; construction/modification uses a separate
  mutable document API. This increases porting complexity.
- No native, actively maintained, verified schema companion meets the current
  constraints.
- Valijson has no yyjson adapter. Creating one is a separate project with its
  own conformance and ownership risks.
- Re-parsing every document with jsoncons for schema validation is feasible but
  creates two DOMs/engines and erodes the performance/memory rationale.
- Indexed/key access patterns differ from RapidJSON; iterator-oriented code may
  be required for performance.
- Team debugging and extension authoring would cross a C/C++ boundary.

### Fit assessment

This is viable only as a deliberate two-engine or custom-adapter architecture.
It should not be selected for a straightforward dependency replacement.

Primary sources:

- [Build and platform coverage](https://github.com/ibireme/yyjson/blob/master/doc/BuildAndTest.md#L7-L10)
- [Immutable/mutable API split](https://github.com/ibireme/yyjson/blob/master/doc/API.md#L13-L35)
- [Allocator API](https://github.com/ibireme/yyjson/blob/master/src/yyjson.h#L660-L732)
- [Valijson adapter list, no yyjson](https://github.com/tristanpenman/valijson/blob/master/CMakeLists.txt#L159-L201)

## Specialized/non-viable candidates

### simdjson

**Pros:** exceptionally fast strict parsing, UTF-8 validation, exact numeric
classification, exception or error-result APIs, and reusable parser capacity.

**Cons:** immutable DOM/on-demand model, no equivalent mutable DOM serializer,
no complete JSON Pointer create workflow, no schema validator, and a
64-bit-oriented platform story that must not be assumed to cover Win32.

**Decision:** do not use as the RapidJSON replacement. It could be evaluated
later as a deserialization accelerator only after the main migration, but that
would add a second JSON engine.

### Glaze

**Pros:** very active, header-only, excellent typed-serialization performance,
generic JSON support, and error-result APIs that work without exceptions.

**Cons:** requires C++23, supports little-endian systems only, its top-level
CMake currently requires 3.31, its generic DOM lacks a uniform document
allocator, and its JSON Schema feature generates schemas rather than validating
the external glTF Draft-04 corpus.

**Decision:** disqualify for the current SDK. Reconsider only as part of an
explicit C++23 typed-serialization redesign.

## Performance evidence and caveat

Published results are workload-specific and must not decide the migration by
themselves.

- simdjson reports roughly four times RapidJSON parsing throughput in its own
  reproducible benchmarks.
- yyjson reports substantial parse/write gains, but its headline DOM charts
  were last updated in 2020.
- A maintained common Windows/MSVC large-document benchmark places Boost.JSON
  close to RapidJSON, yyjson ahead, jsoncons below them, and nlohmann/json well
  behind. The harness is maintained by jsoncons's author, so it is transparent
  but not vendor-neutral.
- Glaze's headline results use its typed path and cannot be projected onto a
  generic-DOM port.

Selection must include a repository-specific benchmark of small, large,
extension-heavy, and float-heavy glTF manifests. Relevant sources:

- [simdjson performance and methodology](https://github.com/simdjson/simdjson/blob/master/README.md#L10-L21)
- [Common integer-DOM benchmark](https://github.com/danielaparker/json_benchmarks/blob/master/report/big.text.integer.md#L5-L36)
- [Common floating-point DOM benchmark](https://github.com/danielaparker/json_benchmarks/blob/master/report/big.text.floating_point.md#L5-L36)

## Cross-candidate behavior requiring explicit tests

| Behavior | jsoncons `ojson` | Boost.JSON | nlohmann `ordered_json` |
|---|---|---|---|
| Comments by default | Accepted/ignored; must disable | Rejected | Rejected |
| Trailing commas by default | Rejected | Rejected | Rejected |
| Duplicate keys | First retained | Last retained | Retained value unspecified |
| Nesting control | Explicit heap-backed parser state; default max 1024 | Explicit parser stack; default max 32 | Iterative state stack; no built-in max |
| Output order | Insertion order | Insertion order unless object removals perturb it | Insertion order with linear lookup |
| Schema | First-party Draft-04 | Conditional Valijson companion | Conditional Valijson companion |

The plan assumes an intentional SDK-level policy of rejecting duplicate object
member names in manifests, extensions, and extras. This avoids silently
inheriting different first/last/unspecified behaviors and removes parser
differential ambiguity. It is a behavior change from RapidJSON, which retains
duplicates and normally finds the first member, so it must be called out in
release notes and locked with tests. If preserving duplicate members is a hard
compatibility requirement, the preferred candidate and DOM strategy must be
re-evaluated before selection.

## Recommendation and decision choices

1. **Prototype jsoncons first** if the objective is the lowest-risk complete
   RapidJSON removal while preserving C++14 and Draft-04. This is the
   recommendation and preferred-candidate decision.
2. **Prototype Boost.JSON + Valijson `v1.0.x`** if maintaining
   RapidJSON-class core performance and allocator control justifies a second
   dependency and possible validator remediation.
3. **Prototype nlohmann/json + Valijson `v1.0.x`** if API familiarity and
   implementation ergonomics outweigh likely runtime/memory regressions.
4. **Prototype yyjson + a separate schema architecture** only if maximizing
   JSON speed is important enough to fund a C++ wrapper and custom/two-engine
   validation design.

Regardless of selection, do not replace public `rapidjson::Document` signatures
with the new vendor's DOM type. Follow the vendor-neutral boundary in the
migration plan so this dependency can be changed without another public API
break.
