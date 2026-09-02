# Task 6.2: Create clean commits, push Release/2.0.0, and verify remote CI

## Goal

Organize buildable commits, preserve user changes, push approved branch, and prove remote ref/CI contains final result.

## Requirements addressed

REQ-REL-1, REQ-REL-3, REQ-REL-4, REQ-REL-5, REQ-PERF-6

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

Task 6.1 fully passes; benchmark deltas explicitly approved.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `Git history on Release/2.0.0` — Create buildable dependency, private API, validation/deserializer, serializer/extensions, removal/docs/CI, and evidence commits.
- `docs/release-evidence/replace-rapidjson-nlohmann/final-validation.md` — Append commit IDs, remote ref, push result, CI links/status.

## Implementation steps

1. Reverify branch/base 3193f83, approved origin, and intentional worktree inventory.
2. Review all diffs and stage explicit paths only; exclude unrelated changes and Built output; preserve untracked .ai unless intentionally included.
3. Create coherent buildable commits in migration order. Include trailers Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com> and Copilot-Session: 1fa03cb7-25c6-4b24-8341-a9230b92e1e2.
4. Before push rerun status, focused smoke, vendor scan, benchmark-review check.
5. Push only Release/2.0.0 to origin; no force push, remote/tag/other-branch change.
6. Verify ls-remote equals local HEAD and contains approved base; record/wait for all required CI gates.

## Targeted validation commands

- `git -C E:\Base3D\glTF-SDK status --short --branch`
- `git -C E:\Base3D\glTF-SDK merge-base --is-ancestor 3193f83265a70585093f13d651167b763979ade1 HEAD`
- `git -C E:\Base3D\glTF-SDK log --oneline --decorate upstream/Release/1.9.5..HEAD`
- `git -C E:\Base3D\glTF-SDK diff --cached --check`
- `git -C E:\Base3D\glTF-SDK push origin Release/2.0.0`
- `git -C E:\Base3D\glTF-SDK ls-remote origin refs/heads/Release/2.0.0`

## Acceptance criteria

- [ ] Local/remote refs match, contain approved base, and no force push occurred.
- [ ] Commits exclude unrelated/generated changes, include trailers, and required CI passes/links.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Before push, unstage explicit paths non-destructively if composition is wrong. After normal push, fix forward with reviewed commits; never force rewrite without approval.

## Expected artifacts

- Buildable commit series, pushed branch, matching remote ref, passing CI evidence.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
