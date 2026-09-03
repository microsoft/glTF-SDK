# Valijson repository patches

The private Valijson 1.0.6 import is modified only by the following patches,
applied in this exact order:

| Patch | Purpose | SHA-256 | Imported tree after patch |
| --- | --- | --- | --- |
| `0001-ordered-nlohmann-adapter.patch` | Use the authoritative insertion-ordered nlohmann DOM. | `F0E70DFB0174C216D6083BB04BB7907B8DC1E64AC40C57BCF5155A0B4CC674C5` | `25AB86694E194FB96517D1FB83E19C18F077850D416C39EFD645A34350152D23` |
| `0002-draft04-uri-and-reference-resolution.patch` | Correct Draft-04 URI scopes, references, caching, and cycles. | `E75B6C5AD75BC3B5409A5A175B5EC69B44CEF866BB6077DF8A71FA851C49F1C1` | `D7E82196CC4C5158E1F11598995780619F25467D81CFDE2E2BE394726FE3E5D0` |
| `0003-structured-validation-keywords.patch` | Retain structured diagnostic keywords and paths. | `2C95D14243C5E157A73BF2FCCC94E50D80CB7676805FABB475DF7609717E99AB` | `8099487F6B16F6E2C75CF98495A69F1E0D008456447844B2619C41AE454D3CBE` |
| `0004-replace-subschema-metadata-optionals.patch` | Avoid the optimized C++14 compatibility-optional lifetime failure. | `DB84BDB68CDA077CAE5D8BB14AEED0AD693EC2F82447811A60619AADEEC0C28F` | `19A7B2F4CE1E26C518DB0EFFBCDDF5ED275F6191D192A317BF8687B594766C41` |
| `0005-optimize-scalar-unique-items.patch` | Give diagnostics-free `uniqueItems` validation an average-linear scalar path. | `C2506247BDC75128A6FF64C976614A48CDDFF4FEC37A0783592C29E15C18602F` | `4A2CE6EE33E8D01E2F6C3188625024A3794ECA90BB9995D2547F57B8B3D06E73` |

Patch 0005 keeps null, boolean, string, and finite numeric categories separate.
Numbers use Valijson's existing strict `getNumber()` equality, so signed zero
and mathematically equal integer/floating values retain existing behavior.
Non-finite numbers and any structured value fall back to the original
recursive pairwise comparison. Results-producing validation always uses the
original path, preserving public diagnostics.

After the documented parser-adapter pruning, the shipped 48-file dependency
tree SHA-256 is
`098816AE043B63A5DB3E3000407B572CAC02C42A9C8F1CC523DC388CA53F785D`.

These repository modifications remain covered by Valijson's bundled
BSD-2-Clause license. The C++14 compatibility optional remains covered by its
bundled Boost Software License. No new third-party code or license is added.

See `UPSTREAM.md` for the pinned source, exact import, reapplication, pruning,
tree-hash, and validation procedure.
