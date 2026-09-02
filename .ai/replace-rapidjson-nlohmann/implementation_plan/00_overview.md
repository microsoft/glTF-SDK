# Replace RapidJSON with nlohmann/json - Implementation Plan

## Summary

Replace RapidJSON throughout glTF-SDK with a private, strict `nlohmann::ordered_json` 3.12.0 layer and corrected private Valijson 1.0.6 Draft-04 validation. The plan preserves C++14/platform behavior, introduces vendor-neutral breaking APIs for 2.0.0, verifies performance and packaging, removes RapidJSON completely, and ends with clean commit/push/CI evidence.

## Phases

- **Phase 1: Baseline and dependencies** — Verify release state, preserve user work, capture baseline evidence, and integrate exact offline dependencies with corrected Valijson.
- **Phase 2: Private JSON and public boundaries** — Build strict ordered JSON primitives, writer/pointer behavior, and vendor-neutral ExtrasDocument/schema APIs.
- **Phase 3: Validation and deserialization** — Prove Draft-04 conformance and atomically switch parse, validation, and typed traversal to one DOM.
- **Phase 4: Serialization and extensions** — Port deterministic core writing and every KHR/custom extension path.
- **Phase 5: Packaging, platforms, evidence, and removal** — Harden install/CI, review benchmarks, document migration, and delete RapidJSON remnants.
- **Phase 6: Release** — Run full validation, create clean commits, push, and verify remote CI.

## Phase Rationale

RapidJSON stays active until exact dependencies, Valijson reference conformance, strict private JSON behavior, and public replacement boundaries are proven. Validation and deserialization switch atomically before serialization; extensions then reuse the common layer. Removal occurs only after platform, benchmark-review, documentation, package, and consumer gates are ready, keeping checkpoints buildable and rollback-safe.

## Execution Rules

- Execute tasks in order and satisfy each named dependency.
- Preserve user changes: inspect status before/after, stage explicit paths only, and never use destructive reset/clean/checkout-overwrite.
- Do not remove RapidJSON before Task 5.5 and do not push before Task 6.2.
- Keep C++14 and existing CMake/toolchain/deployment baselines unless separately approved.
- Historical `.ai` docs may name RapidJSON; shipped/current source, build metadata, package output, CI, and notices must be clean at completion.

## Task Index

| File | Task | Phase | Requirements |
| --- | --- | --- | --- |
| `01_01_verify_branch_base_and_user_changes.md` | Verify branch, base, remotes, and user changes | 1 | REQ-REL-1, REQ-REL-3 |
| `01_02_capture_rapidjson_behavioral_baseline.md` | Capture RapidJSON behavioral and test baseline | 1 | REQ-TST-1, REQ-TST-4, REQ-SCH-11, REQ-REL-2 |
| `01_03_add_benchmark_harness_and_capture_baseline.md` | Add benchmark harness and capture RapidJSON baseline | 1 | REQ-PERF-1, REQ-PERF-2, REQ-PERF-3, REQ-PERF-4, REQ-PERF-5 |
| `01_04_vendor_nlohmann_json_3_12_0.md` | Vendor nlohmann/json 3.12.0 in Jsonnet layout | 1 | REQ-DEP-1, REQ-DEP-2, REQ-DEP-3, REQ-DEP-4, REQ-DEP-5, REQ-DEP-6 |
| `01_05_vendor_pristine_valijson_1_0_6.md` | Vendor pristine Valijson 1.0.6 privately | 1 | REQ-DEP-7, REQ-DEP-9, REQ-DEP-10, REQ-BLD-1, REQ-BLD-2, REQ-BLD-9 |
| `01_06_correct_valijson_ordered_json_adapter.md` | Correct Valijson for ordered_json | 1 | REQ-DEP-8, REQ-PARSE-1, REQ-SCH-15 |
| `01_07_correct_valijson_draft04_refs.md` | Correct Valijson Draft-04 URI and reference resolution | 1 | REQ-SCH-1, REQ-SCH-4, REQ-SCH-5, REQ-SCH-6, REQ-SCH-7, REQ-SCH-13, REQ-SCH-14 |
| `01_08_add_structured_valijson_keywords.md` | Add structured validation keywords to Valijson | 1 | REQ-SCH-10, REQ-SCH-11, REQ-SCH-12 |
| `02_01_create_private_json_layer_and_build_wiring.md` | Create private JSON layer and build wiring | 2 | REQ-DEP-9, REQ-API-2, REQ-PARSE-1, REQ-BLD-1, REQ-BLD-10 |
| `02_02_implement_strict_parser_and_bom_policy.md` | Implement strict parser and BOM policy | 2 | REQ-PARSE-2, REQ-PARSE-3, REQ-PARSE-4, REQ-PARSE-5, REQ-PARSE-6, REQ-PARSE-7, REQ-TST-2 |
| `02_03_implement_checked_access_and_construction.md` | Implement checked scalar access and ordered construction | 2 | REQ-PARSE-8, REQ-PARSE-9, REQ-PARSE-10, REQ-SER-1, REQ-SER-4, REQ-SER-7, REQ-TST-3 |
| `02_04_implement_deterministic_json_writer.md` | Implement deterministic compact and pretty JSON writer | 2 | REQ-SER-2, REQ-SER-3, REQ-SER-4, REQ-SER-5, REQ-SER-7, REQ-SER-8, REQ-TST-4 |
| `02_05_implement_json_pointer_semantics.md` | Implement JSON Pointer lookup and set/create | 2 | REQ-PTR-1, REQ-PTR-2, REQ-PTR-3, REQ-PTR-4, REQ-PTR-5, REQ-PTR-6, REQ-TST-5 |
| `02_06_replace_extras_document_public_api.md` | Replace ExtrasDocument with move-only vendor-neutral PImpl | 2 | REQ-API-1, REQ-API-2, REQ-API-6, REQ-API-7, REQ-API-8, REQ-API-9, REQ-PTR-1, REQ-PTR-2, REQ-PTR-3, REQ-TST-5 |
| `02_07_replace_schema_api_and_add_public_consumer.md` | Replace schema API and add installed consumer | 2 | REQ-API-4, REQ-API-5, REQ-API-10, REQ-API-11, REQ-TST-8, REQ-BLD-10 |
| `03_01_implement_draft04_validation_session.md` | Implement Draft-04 validation session and resolver | 3 | REQ-SCH-1, REQ-SCH-4, REQ-SCH-5, REQ-SCH-6, REQ-SCH-7, REQ-SCH-10, REQ-SCH-14, REQ-SCH-15 |
| `03_02_validate_bundled_schema_graph_flags_and_diagnostics.md` | Validate bundled graph, flags, diagnostics, and CWE regressions | 3 | REQ-API-10, REQ-SCH-2, REQ-SCH-3, REQ-SCH-8, REQ-SCH-9, REQ-SCH-10, REQ-SCH-11, REQ-SCH-12, REQ-TST-6 |
| `03_03_add_official_draft04_conformance_suite.md` | Add official Draft-04 conformance suite | 3 | REQ-SCH-3, REQ-SCH-13, REQ-SCH-14, REQ-TST-6 |
| `03_04_port_core_deserializer_to_ordered_dom.md` | Port core deserializer to the single ordered DOM | 3 | REQ-API-3, REQ-PARSE-1, REQ-PARSE-2, REQ-PARSE-6, REQ-PARSE-8, REQ-PARSE-10, REQ-PARSE-11, REQ-SCH-15, REQ-TST-1 |
| `03_05_expand_deserializer_strict_and_numeric_tests.md` | Expand strict, numeric, and semantic deserializer regressions | 3 | REQ-PARSE-3, REQ-PARSE-4, REQ-PARSE-5, REQ-PARSE-7, REQ-PARSE-8, REQ-PARSE-9, REQ-PARSE-10, REQ-TST-2, REQ-TST-3 |
| `04_01_port_core_serializer_to_ordered_dom.md` | Port core serializer to ordered_json | 4 | REQ-API-3, REQ-SER-1, REQ-SER-2, REQ-SER-3, REQ-SER-4, REQ-SER-5, REQ-SER-6, REQ-SER-7, REQ-SER-8 |
| `04_02_expand_serializer_golden_numeric_and_utf8_tests.md` | Expand serializer golden, numeric, UTF-8, and determinism tests | 4 | REQ-SER-2, REQ-SER-3, REQ-SER-4, REQ-SER-5, REQ-SER-6, REQ-SER-8, REQ-TST-4 |
| `04_03_port_khr_material_extensions.md` | Port KHR material extension JSON paths | 4 | REQ-PARSE-11, REQ-SER-3, REQ-SER-6, REQ-TST-7 |
| `04_04_port_remaining_khr_and_custom_handlers.md` | Port remaining KHR paths and custom handlers | 4 | REQ-API-1, REQ-API-4, REQ-PARSE-11, REQ-SER-3, REQ-SER-6, REQ-TST-7 |
| `05_01_harden_build_install_and_offline_consumer.md` | Harden build, install, offline configure, and public consumer | 5 | REQ-DEP-5, REQ-DEP-9, REQ-DEP-13, REQ-API-11, REQ-BLD-1, REQ-BLD-2, REQ-BLD-9, REQ-BLD-10, REQ-TST-8 |
| `05_02_expand_platform_and_ci_matrix.md` | Expand platform and CI matrix for Release/2.0.0 | 5 | REQ-BLD-3, REQ-BLD-4, REQ-BLD-5, REQ-BLD-6, REQ-BLD-7, REQ-BLD-8, REQ-BLD-11, REQ-BLD-12, REQ-TST-10 |
| `05_03_measure_candidate_and_review_benchmarks.md` | Measure candidate and review benchmark deltas | 5 | REQ-PERF-1, REQ-PERF-2, REQ-PERF-3, REQ-PERF-4, REQ-PERF-5, REQ-PERF-6, REQ-REL-2 |
| `05_04_update_docs_migration_notices_and_provenance.md` | Update release, migration, dependency, downstream, and notice docs | 5 | REQ-API-12, REQ-DEP-10, REQ-DOC-1, REQ-DOC-2, REQ-DOC-3, REQ-DOC-4, REQ-DOC-5, REQ-DOC-6 |
| `05_05_remove_rapidjson_and_scan_shipped_tree.md` | Remove RapidJSON and scan the shipped tree | 5 | REQ-DEP-11, REQ-DEP-12, REQ-DEP-13, REQ-API-1, REQ-BLD-8, REQ-BLD-9, REQ-TST-9 |
| `06_01_run_final_full_validation_and_record_evidence.md` | Run final full validation and record release evidence | 6 | REQ-REL-2, REQ-BLD-3, REQ-BLD-4, REQ-BLD-5, REQ-BLD-6, REQ-BLD-7, REQ-BLD-11, REQ-TST-1, REQ-TST-6, REQ-TST-7, REQ-TST-8, REQ-TST-9, REQ-TST-10 |
| `06_02_create_commits_push_and_verify_remote_ci.md` | Create clean commits, push Release/2.0.0, and verify remote CI | 6 | REQ-REL-1, REQ-REL-3, REQ-REL-4, REQ-REL-5, REQ-PERF-6 |
