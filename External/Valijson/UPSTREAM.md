# Valijson provenance and update procedure

The headers in this directory are a private, repository-controlled import of
Valijson 1.0.6. They are not acquired or updated by CMake.

## Upstream identity

- Repository: `https://github.com/tristanpenman/valijson.git`
- Release tag: `v1.0.6`
- Commit: `4edda758546436462da479bb8c8514f8a95c35ad`
- Archive:
  `https://github.com/tristanpenman/valijson/archive/refs/tags/v1.0.6.tar.gz`
- Archive SHA-256:
  `BF0839DE19510FF7792D8A8ACA94EA11A288775726B36C4C9A2662651870F8DA`

The tag was verified with:

```powershell
git ls-remote https://github.com/tristanpenman/valijson.git `
  refs/tags/v1.0.6 'refs/tags/v1.0.6^{}'
```

## Imported pristine subset

Only these upstream paths are imported:

- `include/valijson/**`
- `include/compat/optional.hpp`
- `LICENSE`
- `Authors`

The imported pristine subset contains 50 files: 48 headers plus the license
and author list. `include/compat/optional.hpp` is mandatory for Valijson's
C++14 path: `include/valijson/internal/optional.hpp` includes it whenever the
language mode is earlier than C++17.

Its deterministic tree SHA-256 is:

```text
C021A46604FBA44CBC2DAA487C38F5629331B40C7BBD2A52A0E3B578D9E076F2
```

The tree hash is calculated by sorting imported files by forward-slash
relative path and hashing, for each file, UTF-8 path bytes, a NUL byte, the
exact file bytes, and a trailing NUL byte. The extracted source and copied
destination produced the same hash.

Upstream tests, examples, documentation, inspector, CMake files, submodules,
`thirdparty/` dependencies, bundled parser sources, and all other repository
content are deliberately excluded.

## Reproducible import

```powershell
$scratch = 'Built\Int\valijson_source'
curl.exe -L --fail `
  https://github.com/tristanpenman/valijson/archive/refs/tags/v1.0.6.tar.gz `
  -o "$scratch\valijson-v1.0.6.tar.gz"
tar -xf "$scratch\valijson-v1.0.6.tar.gz" -C $scratch

$source = "$scratch\valijson-1.0.6"
$dest = 'External\Valijson'
Copy-Item "$source\include\valijson" "$dest\include\valijson" -Recurse
Copy-Item "$source\include\compat\optional.hpp" "$dest\include\compat\optional.hpp"
Copy-Item "$source\LICENSE" "$dest\LICENSE"
Copy-Item "$source\Authors" "$dest\Authors"
```

The commands are maintenance-time operations. The checked-in CMake target is
offline and has no download or update behavior.

## Repository corrections

The following patches are applied in order and retained as reviewable
maintenance artifacts:

1. `patches/0001-ordered-nlohmann-adapter.patch`
   - SHA-256:
     `F0E70DFB0174C216D6083BB04BB7907B8DC1E64AC40C57BCF5155A0B4CC674C5`
   - Imported-tree hash after application:
     `25AB86694E194FB96517D1FB83E19C18F077850D416C39EFD645A34350152D23`
2. `patches/0002-draft04-uri-and-reference-resolution.patch`
   - SHA-256:
     `E75B6C5AD75BC3B5409A5A175B5EC69B44CEF866BB6077DF8A71FA851C49F1C1`
   - Imported-tree hash after application:
     `BFF781CCFA117877288DBACF614AA411548D2BBAE83BE008DAE72538E96CBD32`
3. `patches/0003-structured-validation-keywords.patch`
   - SHA-256:
     `2C95D14243C5E157A73BF2FCCC94E50D80CB7676805FABB475DF7609717E99AB`
   - Final imported-tree hash after all patches:
     `85D599BC6A0F5253F39E92DB652F0CD677F6D60A26C74C6A03C14849D40493DC`

From the repository root, reapply the first patch to a pristine import with:

```powershell
git apply --directory=External/Valijson `
  External/Valijson/patches/0001-ordered-nlohmann-adapter.patch
git apply --directory=External/Valijson `
  External/Valijson/patches/0002-draft04-uri-and-reference-resolution.patch
git apply --directory=External/Valijson `
  External/Valijson/patches/0003-structured-validation-keywords.patch
```

Verify each patch before application with the same command plus
`--check`. Regenerate a patch by comparing the prior patch state with the
edited header tree using `git diff --no-index`, then normalize the paths to
`a/include/...` and `b/include/...`. Recalculate the deterministic imported
tree hash using the path/NUL/content/NUL algorithm above and update every
affected patch/intermediate/final hash in this file.

After any update, run:

```powershell
cmake --build Built\Int\cmake_json_layer --config Debug --target GLTFSDK.Test
Built\Int\cmake_json_layer\GLTFSDK.Test\Debug\GLTFSDK.Test.exe `
  --gtest_filter=JsonSchemaAdapterTests.*:JsonSchemaReferenceTests.*:JsonSchemaDiagnosticsTests.*:JsonSchemaConformanceTests.*
```

Then repeat the complete test, platform/package, installed-consumer, and
benchmark gates. No failing reference case may be skipped or weakened.

## Final dependency pruning

The upstream legacy-parser adapter and its utility header are not used by the
ordered nlohmann integration and are removed from the shipped subset. The
associated author credit is retained with parser-neutral wording, and one
validator documentation comment is likewise parser-neutral.

The final shipped subset contains 48 files: 46 headers plus `LICENSE` and
`Authors`. Its deterministic tree SHA-256 is:

```text
CCD9F7F326B8EC97939B9BD3DC29B425A7DD8748347F7CCAE68C467258EC49BD
```

After applying the three patches, reproduce the pruning from the repository
root with:

```powershell
$legacy = 'rapid' + 'json'
Remove-Item "External\Valijson\include\valijson\adapters\${legacy}_adapter.hpp"
Remove-Item "External\Valijson\include\valijson\utils\${legacy}_utils.hpp"

$authors = Get-Content External\Valijson\Authors -Raw
$authors = $authors.Replace(
  ("Memory management improvements for " + ('Rapid' + 'Json') +
   " parser library"),
  "Memory management improvements for parser adapters")
[IO.File]::WriteAllText(
  "External\Valijson\Authors", $authors,
  (New-Object Text.UTF8Encoding($false)))
```

The parser-neutral validator comment is a non-functional documentation edit.
Recalculate and verify the final tree hash after pruning.
