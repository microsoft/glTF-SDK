# Task 1.1: Verify branch, base, remotes, and user changes

## Goal

Establish non-destructive release preflight evidence before implementation.

## Requirements addressed

REQ-REL-1, REQ-REL-3

## Scope

This is the C++14 glTF-SDK 2.0 migration from public RapidJSON coupling to private `nlohmann::ordered_json` 3.12.0 and corrected private Valijson 1.0.6 Draft-04 validation. Before editing, run `git -C E:\Base3D\glTF-SDK status --short --branch`, inspect modified, staged, and untracked paths, and preserve them. Never use destructive `reset`, `clean`, checkout/restore-overwrite, blanket staging, or any operation that can discard user work. Touch only the listed files and narrowly required evidence; do not modify adjacent repositories.

The task must end as a buildable, independently reviewable checkpoint.

## Dependencies

None.

## Background

The architecture requires one authoritative ordered DOM, strict duplicate and
UTF-8 rejection, a 256-container limit, deterministic compact/four-space
pretty output, vendor-neutral installed headers, complete Draft-04 external
reference behavior, and no final dual-engine fallback. Existing integration
hotspots are the root and GLTFSDK CMake files, `Deserialize.cpp`,
`Serialize.cpp`, `ExtensionsKHR.cpp`, `SchemaValidation.cpp`,
`ExtrasDocument.h`, schema locator/flags, tests, workflows, and notices.

## Exact files and symbols

- `.ai/replace-rapidjson-nlohmann/evidence/preflight.md` — Create branch/base/remotes/worktree inventory and preservation rules.

## Implementation steps

1. Confirm active branch is Release/2.0.0 without switching, rebasing, or resetting.
2. Record HEAD, upstream/Release/1.9.5, merge-base, and ahead/behind; require ancestry through 3193f83265a70585093f13d651167b763979ade1.
3. Record origin/upstream URLs and flag mismatches without changing remotes.
4. Inventory staged, tracked, and untracked changes separately, explicitly preserving .ai and later user edits.
5. Define explicit-path staging and pre/post-status procedure for all later tasks.

## Targeted validation commands

- `git -C E:\Base3D\glTF-SDK status --short --branch`
- `git -C E:\Base3D\glTF-SDK merge-base HEAD upstream/Release/1.9.5`
- `git -C E:\Base3D\glTF-SDK rev-list --left-right --count upstream/Release/1.9.5...HEAD`
- `git -C E:\Base3D\glTF-SDK remote -v`

## Acceptance criteria

- [ ] Evidence records approved base, branch, and exact remotes.
- [ ] No branch, index, remote, source, or user-owned file changed.
- [ ] Pre/post status and scoped diff review show no overwritten or accidentally staged user changes.

## Rollback notes

Delete only the new evidence file; never roll back repository state.

## Expected artifacts

- Preflight evidence document.

## Verification checklist

- [ ] Available targeted commands pass; unavailable platform commands have CI evidence.
- [ ] The listed requirement IDs map to changed files and durable tests/evidence.
- [ ] No nlohmann/json or Valijson type leaks through an installed public declaration.
- [ ] The task is independently revertible at the rollback boundary above.
