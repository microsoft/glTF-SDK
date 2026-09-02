# JSON Schema Test Suite provenance

This directory contains the immutable Draft-04 conformance fixtures selected
for the glTF-SDK JSON migration.

## Source identity

- Repository:
  `https://github.com/json-schema-org/JSON-Schema-Test-Suite.git`
- Commit:
  `8c3d56df71754e6b1fd4c5e48e93e4047840bbe5`
- Selection rationale: this is the exact JSON-Schema-Test-Suite gitlink pinned
  by Valijson tag `v1.0.6` at commit
  `4edda758546436462da479bb8c8514f8a95c35ad`.
- Archive:
  `https://github.com/json-schema-org/JSON-Schema-Test-Suite/archive/8c3d56df71754e6b1fd4c5e48e93e4047840bbe5.tar.gz`
- Archive SHA-256:
  `F41B26DEF5E3A95A883E020145A24125426DD8DDB5A04580FF6950F9C2FF29BD`
- License: MIT, copied verbatim to `LICENSE`.

The existing `JSON Schema Test Suite` section in
`thirdPartyNotices.txt` contains the same copyright and MIT terms and remains
accurate.

## Selected files

All 28 standard `tests/draft4/*.json` files are copied without modification,
including the complete `ref.json` and `refRemote.json` files. The complete
`tests/draft4/optional/format.json` fixture is also copied. The runner executes
its complete `date-time` group because the SDK configures Valijson for strict
date/time validation; the other optional format names are not used by the
bundled glTF schema graph.

All four official `remotes/**` files are copied without modification. The
Draft-04 meta-schema needed by the official `ref.json` remote case is copied
from the same pinned Valijson 1.0.6 source at
`doc/schema/draft-04.json`; its SHA-256 is
`7353FF13FAA979027813B95C8B35F992057822696A70C43C79B3BFB7249E76B4`.

The deterministic selected-tree SHA-256, covering `LICENSE`, all selected
tests, and all remotes except this provenance file, is:

```text
8A2103D0C76F5CB5536B82E07A8EE08B611260245DFDFCE934D9C68F59BF3B7C
```

The tree hash sorts files by forward-slash relative path and hashes, for each
file, the UTF-8 path, a NUL byte, exact file bytes, and a trailing NUL byte.

## Supplemental coverage

The official reference files are executed without skips or weakened
expectations. `JsonSchemaReferenceTests.cpp` additionally covers normalized
nested paths, `../`, external and root fragments, Draft-04 named and scoped
`id`, repeated fetches, legal recursion, malformed reference-only cycles,
missing fragments, and per-locator cache isolation.

## Reproduction

```powershell
$commit = '8c3d56df71754e6b1fd4c5e48e93e4047840bbe5'
$archive = "Built\Int\json_schema_test_suite\suite-$commit.tar.gz"
curl.exe -L --fail `
  "https://github.com/json-schema-org/JSON-Schema-Test-Suite/archive/$commit.tar.gz" `
  -o $archive
tar -xf $archive -C Built\Int\json_schema_test_suite

$source = "Built\Int\json_schema_test_suite\JSON-Schema-Test-Suite-$commit"
$dest = "GLTFSDK.Test\Resources\JSON-Schema-Test-Suite"
Copy-Item "$source\tests\draft4\*.json" "$dest\tests\draft4"
Copy-Item "$source\tests\draft4\optional\format.json" `
  "$dest\tests\draft4\optional\format.json"
Copy-Item "$source\remotes\*" "$dest\remotes" -Recurse
Copy-Item "$source\LICENSE" "$dest\LICENSE"
```
