# NodePerformanceTest load investigation

## Decision summary

The 8x Release/2.0.0 LOAD regression is caused by two independent quadratic
algorithms. It is not explained by file I/O, schema compilation, ordinary
`ordered_json` object lookup, resource reads, or SDK typed construction.

1. **The nlohmann parse-callback implementation is quadratic for arrays of
   objects.** `Internal::ParseJson` uses a callback to enforce duplicate-member
   and depth policy. On every object close,
   `json_sax_dom_callback_parser::end_object` scans the entire parent container
   looking for a discarded value even though the SDK callback never discards
   valid values. NodePerformanceTest caused **1,803,590,934 parent elements**
   to be inspected. A direct strict SAX DOM builder reduced the profiled parse
   phase from **13,323.95 ms to 504.05 ms** while retaining duplicate, UTF-8,
   BOM, depth, numeric-category, and insertion-order behavior.
2. **Valijson implements `uniqueItems` with pairwise comparison.**
   `scene.nodes` contains 10,002 distinct integer indices and its schema has
   `uniqueItems: true`, producing exactly **50,015,001 comparisons** and
   **102,802,780 adapter constructions**. A scalar hash-set fast path on an
   initial no-diagnostics validation pass reduced validation from
   **3,474.44 ms to 840.82 ms**. Invalid documents rerun the original
   exhaustive path, preserving diagnostics.

A clean, non-instrumented spike combining those two changes loaded
NodePerformanceTest through the public benchmark boundary in **1,936.12 ms
mean / 1,923.51 ms median / 2,003.42 ms p95** over ten samples. Relative to
the retained Release/2.0.0 result, this recovers **13,833.64 ms (87.72%)** of
mean LOAD time and is within **0.88%** of the retained RapidJSON mean.

No optimization is committed by T-39. The exact tested spike is retained as
`strongest-clean-spike.patch`.

## Reproduction

All measurements used Visual Studio 17 2022, MSVC `19.44.35228.0`, x64
Release `/MD /O2 /Ob2 /DNDEBUG`, and Sample Assets commit
`9429648735279342b4c32b8745f7904196607379`.

| Implementation/run | Samples | Mean LOAD | Median | p95 |
| --- | ---: | ---: | ---: | ---: |
| RapidJSON retained T-38 | 30 | 1,919.17 ms | 1,889.31 ms | 2,091.95 ms |
| Release/2.0.0 retained T-38 | 30 | 15,769.76 ms | 15,539.54 ms | 16,866.38 ms |
| RapidJSON fresh T-39 confirmation | 3 | 1,824.76 ms | 1,818.71 ms | 1,843.19 ms |
| Release/2.0.0 fresh post-profile control | 3 | 15,269.32 ms | 15,227.12 ms | 15,378.36 ms |
| Clean strongest spike, public API | 10 | **1,936.12 ms** | **1,923.51 ms** | **2,003.42 ms** |

The original executable was also measured before instrumentation
(18,090.05 ms mean over three samples). The before/after spread reflects
machine state, not a source change: that binary was built before profiling and
never rebuilt. The retained 30-sample result remains the decision baseline.

## Method

- Instrumentation lived only in
  `E:\Base3D\glTF-SDK-T39-profile`.
- The strongest change was independently recreated from
  `15e30d85b320669527d49071be17847335a2f661` in
  `E:\Base3D\glTF-SDK-T39-spike`, without profiling code.
- Long NodePerformanceTest variants used documented smaller sample counts;
  promising fast variants used 10-15 measured samples after 2-3 warm-ups.
- Controls were additionally alternated AB/BA for 20 pairs.
- Counted runs were separate from timing runs.
- Cached and uncached variants used the same parsed schemas and validators;
  only schema lifetime differed.
- Derived 100, 1,000, and 5,000 item GLBs retained valid references, camera
  and light nodes, all images, the original binary chunk, and required
  `KHR_lights_punctual`. Exact hashes are in `scaling-subsets.json`.
- OS file cache was not forcibly cleared. The first sample and repeated warm
  samples were retained, while default per-load schema construction versus
  explicit schema reuse was controlled directly. File/JSON input was only
  about 15 ms on the large case.

Windows Performance Recorder was installed, but CPU sampling failed with
`0xc5585011` because the environment could not enable the system-performance
policy. No other native sampler was installed. Low-frequency phase timers and
runtime-gated exact counters were therefore used. See `validation.md`.

## Current phase breakdown

These values are means from the seven-sample instrumented current build.
Percentages use that build's 17,946.73 ms mean, so they are internally
additive. The stable non-instrumented public mean is lower, but the dominance
and operation counts are unchanged.

| LOAD phase | Mean | Percent |
| --- | ---: | ---: |
| Open file/measure stream | 0.34 ms | <0.01% |
| Read and validate GLB header + JSON chunk | 15.22 ms | 0.08% |
| Strict nlohmann parse + ordered DOM | **13,323.95 ms** | **74.24%** |
| Default schema compile, total | 4.30 ms | 0.02% |
| Valijson document validation | **3,886.26 ms** | **21.65%** |
| SDK typed construction | **681.90 ms** | **3.80%** |
| Complete buffer reads | 32.84 ms | 0.18% |
| Complete encoded-image reads | 1.90 ms | 0.01% |
| **Total** | **17,946.73 ms** | **100%** |

Schema compilation performed 33 locator calls, parsed 33 documents totaling
71,456 bytes, and populated one Valijson schema. Locator, schema-parse, and
population sub-timers overlap because external-reference loading occurs
inside `populateSchema`; the non-overlapping total is 4.30 ms.

With both viable changes enabled, the comparable phase means were:

| LOAD phase | Mean | Percent of 2,066.65 ms |
| --- | ---: | ---: |
| File + GLB JSON chunk | 15.34 ms | 0.74% |
| Strict SAX ordered DOM | 521.53 ms | 25.24% |
| Schema compile | 4.01 ms | 0.19% |
| Full validation | 840.82 ms | 40.69% |
| SDK typed construction | 652.08 ms | 31.55% |
| Complete buffers + images | 32.83 ms | 1.59% |

### Typed construction detail

`Deserialize.cpp` remained roughly linear. Its largest mean subphases were:

| Top-level array | Mean | Share of typed construction |
| --- | ---: | ---: |
| accessors (39,993) | 176.43 ms | 27.1% |
| materials (10,000) | 171.86 ms | 26.4% |
| bufferViews (40,093) | 126.93 ms | 19.5% |
| meshes (10,000) | 89.52 ms | 13.7% |
| nodes (10,002) | 58.29 ms | 8.9% |
| textures (10,000) | 27.46 ms | 4.2% |

Typed construction made 1,351,340 ordered-object lookups and inspected
4,373,460 members, averaging 3.24 inspected members per lookup. That is real
work, but it accounts for about 0.65 seconds, not the 13.8-second regression.

## Hot call paths and operation counts

| Path/operation | Current | Strong combined spike | Interpretation |
| --- | ---: | ---: | --- |
| `ParseJson` -> callback SAX `end_object` parent-scan loops | 160,593 | 394 | Remaining spike count is bundled-schema parsing, not the document |
| Parent elements inspected by callback cleanup | **1,803,590,934** | 1,474 | Primary parser hot path |
| Valijson `uniqueItems` pair comparisons | **50,015,001** | 0 | Exactly `10,002 * 10,001 / 2` |
| Scalar uniqueness inserts | 0 | 10,004 | 10,002 scene indices plus extension-name arrays |
| Nlohmann adapter constructions during validation | **102,802,780** | 2,782,782 | Pairwise equality creates two adapters per comparison |
| Validation constraint evaluations | 3,173,429 | 3,173,429 | Validation breadth was not skipped |
| Validation subschema traversals | 2,682,877 | 2,682,877 | Validation breadth was not skipped |
| `allOf` / `anyOf` / `oneOf` | 430,577 / 130,084 / 100 | same | Composition remains intact |
| Valijson ordered-object finds / inspected members | 2,062,496 / 6,726,303 | same | Average lookup scans only 3.26 members |

The parser path is:

`Internal::ParseJson` -> `ordered_json::parse(callback)` ->
`json_sax_dom_callback_parser::end_object` -> linear scan of the parent
container for a discarded child.

The validation path is:

`Draft4ValidationSession::Validate` -> `Validator::validate` ->
`ValidationVisitor::visit(UniqueItemsConstraint)` -> nested outer/inner
iterator loops -> `Adapter::equalTo`.

The equal constraint/traversal counts before and after the uniqueness change
are important: the speedup is from changing the uniqueness algorithm, not
from silently omitting schema branches.

## Scaling and complexity

Medians below use valid deterministic subsets. The binary/resource portion is
held constant, so parse, validation, and typed trends reflect JSON structure.

| Item groups | JSON chunk | Current parse | Current validation | Current total | Strong parse | Strong validation | Strong total |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 100 | 0.127 MiB | 7.74 ms | 11.24 ms | 63.19 ms | 5.90 ms | 10.51 ms | 63.79 ms |
| 1,000 | 1.173 MiB | 207.28 ms | 141.49 ms | 464.43 ms | 53.24 ms | 101.47 ms | 278.10 ms |
| 5,000 | 5.882 MiB | 4,076.83 ms | 1,379.79 ms | 5,726.30 ms | 281.39 ms | 464.46 ms | 1,251.39 ms |
| 10,000 | 11.795 MiB | 13,174.01 ms | 3,881.24 ms | 17,780.77 ms | 501.16 ms | 839.17 ms | 2,018.80 ms |

Exact callback parent-scan counts were 232,992; 18,425,592; 451,961,298;
and 1,803,590,934. They scale as approximately 18 times the square of the
item count. The scene-node uniqueness comparisons scale exactly as
`n(n-1)/2`: 5,151; 501,501; 12,507,501; and 50,015,001.

**Conclusion:** both dominant paths are quadratic. The direct SAX builder and
scalar hash-set uniqueness path are average-case linear. Remaining schema
constraint counts, SDK member lookups, and typed construction scale roughly
linearly.

## Optimization experiments

The main table uses the controlled profiling pipeline. Improvement is versus
the instrumented current mean of 17,946.73 ms. The clean public result is
reported separately because it is the production-quality confirmation.

| Strategy | Mean / median / p95 LOAD | Controlled improvement | Peak working set | Result |
| --- | --- | ---: | ---: | --- |
| Current Release/2.0.0 | 17,946.73 / 17,780.77 / 19,039.25 ms | baseline | 248.1 MiB | Two quadratic paths |
| No-results first pass + diagnostic fallback only | 18,518.65 / 17,744.91 / 20,581.06 ms | -3.19% | 248.1 MiB | No material benefit alone; operation counts were unchanged |
| Plain ordered parse, diagnostic only | 5,827.81 / 5,658.45 / 7,337.68 ms | 67.53% | 247.9 MiB | Proves callback cost, but unsafe because duplicate/depth policy is omitted |
| **Direct strict SAX ordered DOM** | **4,645.72 / 4,636.11 / 4,708.87 ms** | **74.11%** | 248.4 MiB | Viable; parser phase falls to about 0.50 s |
| Strict SAX + scalar `uniqueItems` fast path | 2,066.65 / 2,018.80 / 2,304.82 ms | 88.48% | 248.9 MiB | Strongest uncached strategy |
| Same + cached compiled default schema | 1,950.00 / 1,938.72 / 2,023.47 ms | 89.13% apparent | 249.0 MiB | Cache setup was only 4.43 ms; most row difference is run-to-run variation |
| Strict SAX + default-`json` validation DOM | 4,854.20 / 4,845.00 / 4,900.16 ms | 72.95% | **334.6 MiB** | Validation only 1.3% faster; second parse adds 274 ms and about 86 MiB |
| Default validation DOM + scalar uniqueness | 2,238.43 / 2,236.19 / 2,260.93 ms | 87.53% | **334.6 MiB** | 8.3% slower than ordered single-DOM combined strategy |
| Strict SAX + transient adapter indexes | 5,083.83 / 4,966.35 / 5,703.81 ms | 71.67% | 248.6 MiB | Worse: tiny objects average only 3.26 linear comparisons per find |
| Indexed adapter + scalar uniqueness | 2,466.58 / 2,469.14 / 2,734.71 ms | 86.26% | 248.9 MiB | 42% slower validation than non-indexed combined strategy |
| `DisableSchemaRoot`, diagnostic only | 1,143.15 / 1,139.12 / 1,168.80 ms | 93.63% | 248.5 MiB | Not an acceptable fix; confirms residual non-schema cost |

### Schema reuse

Compiling all 33 bundled schemas costs about 4.0-4.4 ms on this asset. Reuse
is safe only for immutable bundled schemas keyed by the exact `SchemaFlags`;
custom locators and arbitrary schema roots must remain per-session. A
thread-safe bounded cache would matter for repeated small-document loads, but
it is not a meaningful NodePerformanceTest optimization and should not be
bundled into the primary fix.

### Default `nlohmann::json`

Using a second default-`json` DOM for validation did not support the original
hypothesis that `ordered_json` lookup was the main problem. Default-map
validation was only about 45 ms faster before the uniqueness fix and about
13 ms faster afterward, while constructing the second DOM cost about 274 ms
and raised peak working set by roughly 35%. A global default-`json` switch
would additionally break insertion-order behavior and was not considered a
compatible fix.

### Adapter/property indexes

The transient index spike built hash indexes for ordered objects before
Valijson property access. It was slower both before and after uniqueness
optimization. The measured 6.7 million member inspections sound large, but
each find scans only 3.26 members on average; index construction, allocations,
and hashing cost more than those short scans.

## Memory and controls

The clean public-API spike had a **427.04 MiB median / 429.86 MiB maximum**
process peak. Retained current Release/2.0.0 large-enabled runs were
431.51 MiB median / 433.52 MiB maximum. RapidJSON was 380.52 MiB median /
400.98 MiB maximum. The proposed fix therefore did not increase the current
nlohmann peak, although it remains above RapidJSON.

Twenty-pair alternating controls showed only small absolute changes:

| Control LOAD | Current median | Combined spike median | Delta |
| --- | ---: | ---: | ---: |
| Avocado GLB | 24.849 ms | 24.610 ms | -0.97% |
| SpecularTest GLB | 8.508 ms | 8.340 ms | -2.00% |
| ABeautifulGame GLB | 85.221 ms | 86.862 ms | +1.92% (+1.64 ms) |
| ABeautifulGame Draco/BasisU | 37.819 ms | 38.802 ms | +2.59% (+0.98 ms) |

No control showed a decision-relevant regression. The clean public benchmark
also produced means of 24.90, 7.72, 82.38, and 36.49 ms respectively.

## Serializer and round-trip context

Serialization was not changed by any viable load spike. On
NodePerformanceTest, median instrumented export subphases were:

| Export phase | Median |
| --- | ---: |
| SDK object -> ordered JSON DOM | 628.32 ms |
| JSON DOM -> UTF-8 text | 148.90 ms |
| GLB/resource writing and flush | 174.05 ms |

The 951 ms instrumented sum is close to the retained Release/2.0.0 EXPORT
median of 878.71 ms; the difference is profiling and run variance. RapidJSON's
retained EXPORT median is 303.92 ms, so serializer work remains a separate
future opportunity, not the cause of the LOAD regression.

ABeautifulGame's export was instead dominated by its 43 MB resource write
(260.38 ms median); its serializer DOM and text phases were 1.22 and 0.34 ms.

## Compatibility and correctness

The clean proposed spike passed:

- focused parser/schema/deserializer/extension/serializer tests: **148/148**;
- complete Release suite: **505/505**;
- complete 16-case pinned corpus validation;
- ten public LOAD/EXPORT/ROUNDTRIP samples for NodePerformanceTest and four
  controls, with all semantic reload checks passing;
- deterministic output: every case had one hash across all ten export and
  round-trip outputs.

This covers strict duplicate/UTF-8/BOM/depth behavior, Draft-04 external
references, flags and deterministic diagnostics, official schema
conformance, typed/raw extension preservation, resource equivalence,
serialization order, and output repeatability.

The uniqueness fast path is used only when validation is not collecting
diagnostics. It handles null, boolean, numeric, and string arrays. If a
structured value is encountered it falls back to the original recursive
pairwise algorithm. If any fast validation fails, the complete original
validation is rerun with `ValidationResults`, so public diagnostics remain
unchanged. Numeric hashing uses Valijson's existing `getNumber()` semantics;
the implementation task should add explicit large-integer and mixed numeric
regressions before adoption.

## Ranked recommendation

### Low-risk production optimizations

1. **Implement the direct strict SAX ordered-DOM parser.** This is the largest
   fix and removes the 1.8-billion-element callback cleanup scan without
   weakening strictness or order.
2. **Implement fast-valid validation plus scalar `uniqueItems`.** First
   validate without a results collector; on failure rerun unchanged exhaustive
   validation for diagnostics. Use hash sets for scalar arrays and preserve
   pairwise fallback for structured values. This removes the 50,015,001
   scene-node comparisons while retaining all 3,173,429 constraint
   evaluations on valid input.
3. **Do not prioritize schema caching for this issue.** It can be considered
   separately for repeated small-document workloads, with exact flags,
   immutable default schemas, bounded lifetime, and thread-safety tests.

### Architectural alternatives

4. If more validation reduction is required, compile a flattened validation
   plan for the bundled glTF schema graph or evaluate a validator with
   linear-time `uniqueItems` and lower adapter churn. This targets the
   remaining approximately 0.8-second validation phase but has a much larger
   compatibility surface.
5. Optimize SDK typed construction only after the parser/validator fixes.
   Reserving indexed containers and reducing the measured 1.35 million member
   lookups may recover part of the remaining approximately 0.6 seconds.
6. Treat serializer optimization as a separate task. Its ordered DOM
   construction is measurable on NodePerformanceTest, but it does not affect
   LOAD.

### Rejected as primary fixes

- disabling schema validation;
- plain parsing without duplicate/depth enforcement;
- switching public behavior to default `nlohmann::json`;
- a sequential default validation DOM;
- transient ordered-object indexes;
- schema caching presented as a large-asset fix.

## Recoverable time

The clean public spike reduced mean LOAD from 15,769.76 to 1,936.12 ms:
**13,833.64 ms recovered (87.72%)**. Median recovery was 13,616.03 ms
(87.62%), and p95 recovery was 14,862.96 ms (88.12%).

The controlled phase deltas attribute about 12.82 seconds to the callback
parser and 2.63 seconds to pairwise uniqueness. Those figures overlap with
machine variance and should not be added to a different baseline run, but they
correctly rank the recoverable work. Schema compilation contributes only
about 4 ms.

## Exact next implementation steps

1. Start from Release/2.0.0 and apply the strategy in
   `strongest-clean-spike.patch`; do not copy profiling hooks.
2. Move the strict SAX builder into the private JSON layer, keep the current
   BOM checks and exception translation, and add focused tests for root
   scalars, empty input, duplicate nesting, depth 256/257, invalid UTF-8,
   comments, trailing input, and numeric categories.
3. Change schema validation to a no-results first pass followed by the current
   exhaustive diagnostic pass only on failure.
4. Add a new maintained Valijson patch for scalar `uniqueItems`, update
   `PATCHES.md`, provenance, and the checkout-stable Valijson tree hash.
5. Add regressions for duplicate null/bool/string/number values, `-0.0`,
   integer/float equality, integers around and above 2^53, mixed scalar and
   structured arrays, duplicate objects/arrays, and exact diagnostic paths.
6. Run the focused 148 tests, official Draft-04 suite, full 505 tests,
   16-case corpus, x64/Win32/ARM64 and Linux/macOS/sanitizer gates.
7. Rerun the matched RapidJSON/nlohmann public benchmark with at least five
   warm-ups and 30 large samples before merging.

## Evidence

- `profile-raw.csv` / `profile-aggregates.csv`: 294 raw phase rows and 900
  aggregates.
- `fresh-reproduction-raw.csv` / `fresh-reproduction-summary.csv`.
- `paired-controls-raw.csv` / `paired-controls-summary.csv`.
- `public-clean-spike-raw.csv`, process memory, and output hashes.
- `scaling-subsets.json`.
- `environment.json`.
- `strongest-clean-spike.patch`.
- `temporary-profiling-instrumentation.patch`.
- `tools/`: opt-in subset generator, matrix runner, and result aggregator.
- Existing retained baseline:
  `../load-export/load-export-comparison.md` and its raw CSV/JSON files.
