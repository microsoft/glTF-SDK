# Dependency and provenance policy

## nlohmann/json

- Version: 3.12.0
- Source file: `External/json/nlohmann/json.hpp`
- Approved SHA-256:
  `AAF127C04CB31C406E5B04A63F1AE89369FCCDE6D8FA7CDDA1ED4F32DFC5DE63`
- License: MIT, `External/json/LICENSE`
- CMake target: `nlohmann_json::nlohmann_json`
- Visibility/install: private build-only interface; no install/export rules

The repository intentionally mirrors only `LICENSE`, `CMakeLists.txt`, and the
single `nlohmann/json.hpp` header. Updates require an approved local
single-header source, exact version-macro verification, hash review, C++14
compile/run probe, strict parser/writer tests, full tests, and benchmark review.

## Valijson

- Version/tag: 1.0.6 / `v1.0.6`
- Commit: `4edda758546436462da479bb8c8514f8a95c35ad`
- Archive SHA-256:
  `BF0839DE19510FF7792D8A8ACA94EA11A288775726B36C4C9A2662651870F8DA`
- License: BSD-2-Clause, `External/Valijson/LICENSE`
- CMake target: `ValiJSON::valijson`
- Visibility/install: private build-only interface; no install/export rules

The imported C++14 subset is `include/valijson/**`,
`include/compat/optional.hpp`, `LICENSE`, and `Authors`.
`compat/optional.hpp` is required because Valijson's C++14 path includes it and
is covered by the Boost Software License.

Repository patches, in order:

1. `0001-ordered-nlohmann-adapter.patch`
2. `0002-draft04-uri-and-reference-resolution.patch`
3. `0003-structured-validation-keywords.patch`
4. `0004-replace-subschema-metadata-optionals.patch`

Exact patch hashes, intermediate/final tree hashes, import commands, and
reapplication instructions are maintained in
`External/Valijson/UPSTREAM.md`. An update must start from a pristine import,
reapply or regenerate each patch, verify the resulting tree hash, run all
adapter/reference/diagnostic tests, run the selected official Draft-04 suite,
and repeat platform/package/benchmark gates.

## JSON Schema Test Suite

The selected Draft-04 fixtures are pinned to commit
`8c3d56df71754e6b1fd4c5e48e93e4047840bbe5`, the suite gitlink used by
Valijson 1.0.6. Provenance, archive/tree hashes, selection, license, and update
commands are in
`GLTFSDK.Test/Resources/JSON-Schema-Test-Suite/UPSTREAM.md`.

## GoogleTest

Unit tests retain the repository's existing GoogleTest package lookup and
fallback. GoogleTest is not a public SDK dependency. Package-only offline
validation configures with `ENABLE_UNIT_TESTS=OFF`; normal test builds may use
the fallback when an installed package is unavailable.

## Public boundary

No installed public header or exported declaration names nlohmann/json or
Valijson. Installed consumers link only GLTFSDK. CI scans installed headers and
package contents on desktop and mobile gates.
