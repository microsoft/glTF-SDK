# Requirement trace

Recorded: 2026-09-01

`Passed` denotes durable implementation plus local and, where required,
remote evidence. GitHub Actions run
[`33579694425`](https://github.com/SergioRZMasson/glTF-SDK/actions/runs/33579694425)
passed all 21 required jobs.

## Dependency and public API

| Requirements | Implementation and evidence | Status |
| --- | --- | --- |
| REQ-DEP-1, REQ-DEP-2, REQ-DEP-3, REQ-DEP-4 | `External/json` contains the approved 3.12.0 single header, license, and private CMake target; header SHA-256 is recorded in `docs/Dependencies.md`. | Passed |
| REQ-DEP-5 | Six final invalid-proxy Windows package builds configured and built without JSON dependency network access. | Passed |
| REQ-DEP-6 | No installed-package selection path is enabled; the mandatory vendored 3.12.0 target is authoritative. | Passed |
| REQ-DEP-7, REQ-DEP-8 | `External/Valijson/UPSTREAM.md`, ordered adapter, forced Draft-04 session, four retained correction patches, and schema suites pin and verify 1.0.6. | Passed |
| REQ-DEP-9 | `GLTFSDK/CMakeLists.txt` links both JSON targets privately; six installed-only consumers compile without vendor include/link paths. | Passed |
| REQ-DEP-10 | `UPSTREAM.md`, `docs/Dependencies.md`, licenses, update steps, and `thirdPartyNotices.txt` retain exact provenance and notices. | Passed |
| REQ-DEP-11, REQ-DEP-12, REQ-DEP-13 | `rapidjson-removal.md`; old external tree/header/build/install/workaround removed; shipped and six-package scans clean; no dependency header package exists. | Passed |
| REQ-API-1, REQ-API-2 | Public old-vendor API/header removed; all installed-header vendor-leak scans report zero matches. | Passed |
| REQ-API-3 | Existing string/stream deserialize and document serialize surfaces remain and pass the full and public-consumer suites. | Passed |
| REQ-API-4, REQ-API-5 | `SchemaValidation.h/.cpp` expose vendor-neutral serialized JSON plus owned `ISchemaLocator`; schema/API tests pass. | Passed |
| REQ-API-6, REQ-API-7, REQ-API-8, REQ-API-9 | `ExtrasDocument.h/.cpp` provide move-only PImpl lifetime, `ToJson`, `HasMember`, typed root/member/pointer access, and compile-time type restrictions; extras tests pass. | Passed |
| REQ-API-10 | Existing schema names/values and the shared `0x80` sparse alias are asserted across all flags. | Passed |
| REQ-API-11 | Six x64/Win32/ARM64, Debug/RelWithDebInfo installed-only consumer builds pass; four host executables run. | Passed |
| REQ-API-12 | `MigrationGuide-2.0.md`, `DownstreamBreaks-2.0.md`, and release notes identify removals and replacements. | Passed |

## Parsing, serialization, and pointers

| Requirements | Implementation and evidence | Status |
| --- | --- | --- |
| REQ-PARSE-1 | `Internal/Json.h`, `Json.cpp`, deserializer, extensions, extras, and schema code use one ordered nlohmann DOM. | Passed |
| REQ-PARSE-2 | Unified byte parser serves string and stream paths; parity and public deserializer tests pass. | Passed |
| REQ-PARSE-3, REQ-PARSE-4 | Strict syntax/comment/trailing/non-finite/duplicate/UTF-8 tests include schema-disabled paths. | Passed |
| REQ-PARSE-5 | Compact/whitespace BOM matrices for string and stream enforce the opt-in flag. | Passed |
| REQ-PARSE-6 | `Json.cpp` and public boundaries translate vendor failures to SDK exceptions; error tests pass. | Passed |
| REQ-PARSE-7 | The common parser/writer enforces the documented 256-container boundary; 256/257 tests pass. | Passed |
| REQ-PARSE-8, REQ-PARSE-9 | Checked scalar access enforces category, integrality, and exact ranges; numeric boundary/overflow tests pass. | Passed |
| REQ-PARSE-10 | Defensive object/array/member/fixed-array/numeric regressions pass with schema root disabled. | Passed |
| REQ-PARSE-11 | Extension/extras unique-member semantic round trips pass. | Passed |
| REQ-SER-1, REQ-SER-3 | Ordered construction plus lexical ordering of SDK unordered collections preserves deterministic member order; goldens pass. | Passed |
| REQ-SER-2 | Compact and four-space pretty fixtures pass byte-for-byte. | Passed |
| REQ-SER-4 | Integer bounds, `1.0`, negative zero, exponent, float/double, and reviewed shortest-decimal tests/evidence pass. | Passed |
| REQ-SER-5 | Strict UTF-8 and escaping tests pass; invalid/non-finite output is rejected. | Passed |
| REQ-SER-6 | Strictly parsed raw extension/extras subtrees retain structure/order and compact whitespace. | Passed |
| REQ-SER-7 | Serialization failures are SDK exceptions in focused error tests. | Passed |
| REQ-SER-8 | Repeatability tests produce byte-identical compact and pretty output. | Passed |
| REQ-PTR-1, REQ-PTR-2, REQ-PTR-3 | Extras defaults, category adoption/reassignment, object initialization/update ordering, and wrong-root tests pass. | Passed |
| REQ-PTR-4, REQ-PTR-5, REQ-PTR-6 | RFC 6901 lookup/create, escaping, array growth, invalid syntax/index/traversal, atomicity, and depth tests pass. | Passed |

## Draft-04 schema validation

| Requirements | Implementation and evidence | Status |
| --- | --- | --- |
| REQ-SCH-1 | `Draft4ValidationSession` forces Valijson Draft-04; adapter/session tests pass. | Passed |
| REQ-SCH-2 | All 33 bundled schemas and complete root graph compile and validate known-valid/invalid corpus. | Passed |
| REQ-SCH-3 | Bundled keyword matrix plus selected official suite covers every listed assertion/format behavior. | Passed |
| REQ-SCH-4, REQ-SCH-5 | Locator-backed nested, parent, fragment, id-scope, and extension-to-core references pass. | Passed |
| REQ-SCH-6, REQ-SCH-7 | Legal recursion, illegal cycles, repeated refs, and per-session locator isolation tests pass. | Passed |
| REQ-SCH-8, REQ-SCH-9 | Every flag substitution, Draft-04 `{}` behavior, shared sparse alias, strict parse, and semantic checks pass. | Passed |
| REQ-SCH-10 | Null locator, malformed document/schema, missing URI/fragment, locator, and validation error categories are tested. | Passed |
| REQ-SCH-11 | Structured keyword/pointer diagnostics retain exact asserted messages. | Passed |
| REQ-SCH-12 | CWE and malformed dependency regressions pass locally without crash; sanitizer confirmation is tracked under REQ-BLD-11. | Passed |
| REQ-SCH-13 | 329 selected official Draft-04 cases, including all 40 reference cases, pass without skips. | Passed |
| REQ-SCH-14 | Three C++14 Valijson correction patches and complete reference tests retain validation strength. | Passed |
| REQ-SCH-15 | Public deserialization parses once into the authoritative DOM for validation and typed traversal. | Passed |

## Build, tests, and platforms

| Requirements | Implementation and evidence | Status |
| --- | --- | --- |
| REQ-BLD-1, REQ-BLD-2 | Root/consumer CMake require C++14; minimum CMake, Apple 10.11 deployment, and existing toolchain baselines are unchanged. | Passed |
| REQ-BLD-3 | Final x64, Win32, and ARM64 Debug/RelWithDebInfo build/install matrix passes. | Passed |
| REQ-BLD-4 | Linux/macOS workflows build, test, install, scan, and run consumers in both configurations; all jobs pass in run `33579694425`. | Passed |
| REQ-BLD-5 | iOS device and 64-bit simulator workflows build/install both configurations; all jobs pass in run `33579694425`. | Passed |
| REQ-BLD-6 | Android three-ABI workflow builds/installs both configurations; all jobs pass in run `33579694425`. | Passed |
| REQ-BLD-7 | x64/Win32/Linux/macOS host tests pass; ARM64/iOS/Android configure, compile, link, and install gates pass. | Passed |
| REQ-BLD-8 | Old Windows policy workaround is removed; six final Windows configurations pass. | Passed |
| REQ-BLD-9 | Final invalid-proxy package matrix proves no RapidJSON, nlohmann/json, or Valijson download. | Passed |
| REQ-BLD-10 | Six 44-file packages contain SDK artifacts/notices/licenses and no private dependency headers/targets. | Passed |
| REQ-BLD-11 | Linux Clang ASAN/UBSAN full plus malformed/deep focused suites pass in run `33579694425`. | Passed |
| REQ-BLD-12 | Seven branch/PR-gated workflows define and passed the complete required matrix in run `33579694425`. | Passed |
| REQ-TST-1 | x64 and Win32 Debug/RelWithDebInfo full runs each pass 505/505; optimized Clang passes 505/505 and CTest registration passes 1/1. | Passed |
| REQ-TST-2 | `JsonTests` and deserializer regressions cover every strict syntax, UTF-8, BOM, stream, and depth case. | Passed |
| REQ-TST-3 | JSON/deserializer/serializer numeric suites cover all listed ranges/categories/precision forms. | Passed |
| REQ-TST-4 | Golden, ordering, escaping, repeatability, and semantic round-trip suites pass. | Passed |
| REQ-TST-5 | Extras move/type/member/default/pointer suite passes. | Passed |
| REQ-TST-6 | Bundled, flags, locator/reference, diagnostics, official, and CWE schema suites pass. | Passed |
| REQ-TST-7 | All 48 production KHR/custom extension tests pass within the full suite. | Passed |
| REQ-TST-8 | Six installed-only consumers build/link without vendor dependencies; host variants run. | Passed |
| REQ-TST-9 | Shipped source, workflow, notice, public-header, filename, and package scans are clean. | Passed |
| REQ-TST-10 | Local XML/matrix results and all 21 remote job conclusions are recorded in `final-validation.md`. | Passed |

## Performance, documentation, and release

| Requirements | Implementation and evidence | Status |
| --- | --- | --- |
| REQ-PERF-1, REQ-PERF-2 | Equivalent baseline/candidate harness, six required assets, and 33 operation/workload groups are recorded. | Passed |
| REQ-PERF-3, REQ-PERF-4 | Comparison records absolute and percentage timing, memory, binary, clean/incremental build values with parse/validate/write/end-to-end separation. | Passed |
| REQ-PERF-5 | Commands, environment, assets, 30-run aggregation, and two 990-row raw datasets are retained. | Passed |
| REQ-PERF-6 | `benchmark-comparison.md` records the accepted review disposition and visible regressions before push. | Passed |
| REQ-DOC-1 | README, dependency guide, provenance, licenses, and update procedure identify both private dependencies. | Passed |
| REQ-DOC-2, REQ-DOC-3 | 2.0 release/migration docs identify the ABI/source break and replacement extras/pointer/schema usage. | Passed |
| REQ-DOC-4 | `DownstreamBreaks-2.0.md` records adjacent source impacts without changing adjacent repositories. | Passed |
| REQ-DOC-5 | Release notes cover duplicate/UTF-8/BOM/sparse-alias/lexical behavior. | Passed |
| REQ-DOC-6 | Current repository and installed notices contain all retained/new material and no obsolete dependency notice. | Passed |
| REQ-REL-1 | Preflight records approved base `3193f83265a70585093f13d651167b763979ade1` and Microsoft Release/1.9.5 ancestry. | Passed |
| REQ-REL-2 | This trace covers every approved requirement, and all local/remote gates pass. | Passed |
| REQ-REL-3 | Explicit, generated-output-free commit series with required trailers is on `Release/2.0.0`. | Passed |
| REQ-REL-4 | Normal, non-force push to `git@github.com:SergioRZMasson/glTF-SDK.git` succeeded. | Passed |
| REQ-REL-5 | Remote implementation ref matched `d28ffe6`; run `33579694425` completed successfully with 21/21 jobs. | Passed |
