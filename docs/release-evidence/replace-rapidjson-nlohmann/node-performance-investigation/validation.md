# T-39 validation record

## Revisions and toolchain

- Release branch/base: `Release/2.0.0` at
  `15e30d85b320669527d49071be17847335a2f661`.
- RapidJSON comparison: `perf/Release-1.9.5-load-export` at
  `743b7ecd749a53e8d848db56839752a65205e915`.
- Sample Assets:
  `9429648735279342b4c32b8745f7904196607379`.
- Visual Studio 17 2022, MSVC `19.44.35228.0`, x64 Release,
  `/MD /O2 /Ob2 /DNDEBUG`.

All instrumentation was confined to
`E:\Base3D\glTF-SDK-T39-profile`. The clean proposed spike was independently
recreated without instrumentation in
`E:\Base3D\glTF-SDK-T39-spike`. Neither worktree was committed or pushed.

## Builds and tests

The instrumented harness and clean spike both configured and built with the
same generator, compiler, architecture, and Release flags as the retained
load/export evidence.

The clean spike passed:

- focused parser/schema/deserializer/extension/serializer tests:
  **148/148**;
- complete Release suite: **505/505**;
- the complete 16-case, 21-file pinned load/export corpus validation,
  including typed and raw extension representation, all buffer bytes, all
  SDK-addressable encoded image bytes, export, reload, and document equality.

The complete suite covers strict duplicate-member, malformed grammar, invalid
UTF-8, UTF-8/UTF-16/UTF-32 BOM policy, depth 256/257, numeric boundaries,
Draft-04 references and external fragments, flags, deterministic diagnostics,
official conformance cases, extension/extras preservation, serialization
order, and deterministic round trips.

## Public API benchmark confirmation

The clean, non-instrumented spike ran one warm-up and ten measured cycles
through the unchanged `GLTFSDK.LoadExportBenchmarks` public API boundary for:

- `NodePerformanceTest.glb`;
- `ABeautifulGame.glb`;
- `ABeautifulGame` Draco/BasisU;
- `Avocado.glb`;
- `SpecularTest.glb`.

Every measured LOAD, EXPORT, and ROUNDTRIP semantic check passed. Each of the
ten export and round-trip outputs per case was byte-deterministic. The complete
hash list is in `public-clean-spike-output-hashes.csv`.

## Non-instrumented controls

The original Release/2.0.0 executable was built before instrumentation and
rerun after the investigation. Fresh three-sample NodePerformanceTest means
were 18,090.05 ms before and 15,269.32 ms after; the retained 30-sample mean is
15,769.76 ms. The spread is why conclusions use matched phase profiles,
operation counts, medians, and the retained 30-sample baseline rather than a
single wall-clock observation.

Fresh RapidJSON reproduction at the current comparison head produced a
1,824.76 ms NodePerformanceTest mean over three samples, consistent with the
retained 30-sample 1,919.17 ms mean.

## Sampling-profiler limitation

Windows Performance Recorder, `xperf`, and WPAExporter were installed.
Starting `CPU.Verbose.File` failed with error `0xc5585011`:
`Failed to enable the policy to profile system performance`. Visual Studio
sampling collection commands and another installed native sampler were not
available. The investigation therefore used low-frequency scoped timers plus
runtime-gated exact operation counters. Counted runs were not used as timing
samples.

## Cleanup

The Release/2.0.0 worktree contains no parser, schema, Valijson, adapter, or
profiling behavior change. The proposed implementation is retained only as
`strongest-clean-spike.patch`; full temporary instrumentation is retained as
`temporary-profiling-instrumentation.patch`.
