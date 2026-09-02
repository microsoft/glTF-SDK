# Task 5.4: Update release, migration, dependency, downstream, and notice docs

## Goal

Document 2.0 breaks, replacements, behavior changes, dependencies, provenance, and adjacent impacts.

## Requirements addressed

REQ-API-12, REQ-DEP-10, REQ-DOC-1, REQ-DOC-2, REQ-DOC-3, REQ-DOC-4, REQ-DOC-5, REQ-DOC-6

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Public APIs, versions, lexical differences, benchmark disposition known.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `README.md` — Identify 2.0/private dependencies/build/package boundary.
- `docs/ReleaseNotes/2.0.0.md` — Describe breaks/strict changes/BOM/flags/lexical notes.
- `docs/MigrationGuide-2.0.md` — Explain extras/pointers/validation/extensions replacements.
- `docs/Dependencies.md` — Document versions/licenses/private targets/patch/update.
- `docs/DownstreamBreaks-2.0.md` — List known adjacent files/usages only.
- `thirdPartyNotices.txt` — Remove obsolete RapidJSON-only and add nlohmann/Valijson while retaining applicable notices.
- `External/Valijson/UPSTREAM.md` — Finalize patch/update instructions.

## Implementation steps

1. State 2.0 source/ABI break and removals: RapidJsonUtils.h, ExtrasDocument::GetDocument, DOM validator.
2. Document ToJson, HasMember, typed/pointer methods, string validation, serialized extension construction.
3. Document duplicate/invalid UTF-8 rejection, depth 256, compact-stream BOM fix, sparse 0x80 alias, reviewed lexical differences.
4. List architecture-confirmed downstream paths without editing adjacent repositories.
5. Audit notices line-by-line; retain schema/test/other required material and add exact dependency licenses.
6. Document dependency update/repatch verification and no-public-vendor policy.

## Targeted validation commands

- `rg -n "GetDocument|RapidJsonUtils|ValidateDocumentAgainstSchema|nlohmann|Valijson|duplicate|UTF-8|ByteOrderMark|0x80" E:\Base3D\glTF-SDK\README.md E:\Base3D\glTF-SDK\docs E:\Base3D\glTF-SDK\thirdPartyNotices.txt`
- `git -C E:\Base3D\glTF-SDK diff --check -- README.md docs thirdPartyNotices.txt External/Valijson/UPSTREAM.md`

## Acceptance criteria

- [ ] Every removed API/behavior change has migration/release guidance.
- [ ] Notices/provenance are complete without removing still-required material.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Revert docs only if technical decisions change; never restore obsolete notice after removal.

## Expected artifacts

- Release/migration/dependency/downstream docs and notices.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
