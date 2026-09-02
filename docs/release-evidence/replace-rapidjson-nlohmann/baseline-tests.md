# RapidJSON behavioral and test baseline

Date: 2026-09-01
Source commit: `3193f83265a70585093f13d651167b763979ade1`

## Toolchain

- Host: Windows x64
- CMake: 3.31.8
- Generator used: Visual Studio 17 2022, x64
- Visual Studio: Enterprise 2022 17.14.39
- MSVC C/C++ compiler: 19.44.35228.0
- MSVC tools: 14.44.35207
- Windows SDK: 10.0.26100.0
- Configurations: Debug and RelWithDebInfo

The planned `Visual Studio 18 2026` generator was not listed by the installed
CMake 3.31.8 even though Visual Studio 18 is installed. The baseline therefore
used the available supported Visual Studio 17 generator. Visual Studio 18
coverage remains an explicit platform/CI gate in Task 5.2.

## Build and test commands

```powershell
cmake -S E:\Base3D\glTF-SDK `
  -B E:\Base3D\glTF-SDK\Built\Int\cmake_baseline_x64 `
  -G "Visual Studio 17 2022" -A x64

cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_baseline_x64 `
  --target install --config Debug --parallel 8

cmake --build E:\Base3D\glTF-SDK\Built\Int\cmake_baseline_x64 `
  --target install --config RelWithDebInfo --parallel 8
```

Both builds and installs passed.

Full Debug:

```text
404 tests from 22 test suites ran.
404 passed.
```

Full RelWithDebInfo:

```text
404 tests from 22 test suites ran.
404 passed.
```

Focused migration suites:

```powershell
GLTFSDK.Test.exe --gtest_filter=GLTFTests.*:DeserializeTests.*:
  SerializeTests.*:GLTFExtrasDocumentTests.*:ExtensionsTests.*:
  JsonBaselineTests.*
```

```text
177 tests from 6 test suites ran.
177 passed.
```

The XML results are retained in the ignored baseline build tree:

- `Built/Int/cmake_baseline_x64/baseline-debug-with-fixtures.xml`
- `Built/Int/cmake_baseline_x64/baseline-relwithdebinfo-with-fixtures.xml`
- `Built/Int/cmake_baseline_x64/baseline-focused-with-fixtures.xml`

## Byte-exact differential fixtures

`GLTFSDK.Test/Source/JsonBaselineTests.cpp` round-trips each compact fixture,
then compares compact and four-space pretty output byte-for-byte. The
extension fixture uses the KHR extension handlers.

| Fixture | Compact SHA-256 | Pretty SHA-256 |
| --- | --- | --- |
| default | `F74CABC3532658AF0AA3E7ABDD6939D22AA5CD78849DC06E3817248FE1C3788D` | `4D0E84404F646606BC168C9CE4A1648186B62C17A5E3A9743B50E492D6BA09AC` |
| extension-heavy | `6746106A1B7AAC2E9D9F9E0720A51B42C5397EA9AC1A6846934FC14957AC57B1` | `78B41A7F10677E5A14CFFD0841A48000636E1A6D493136C11A1AB51430F6C323` |
| extras-heavy | `A246213DE670D84071E8A93683F4CEB5F5884553AFACB8CAD5D70796DAE114FF` | `5975DB378BE893A8BE1FB228146030789FF0B304E4781778F8E3B6AF99FE6203` |
| numeric | `44C9A4E03F479A42CBAFE34B44966DD70BAD918664651DC15CB0D65A24EE57ED` | `0716C2E823249C45D32A79F04FD88BD4F6752B6225B03D4B89DF9B54C0F0C8CC` |

The two generator labels were renamed from a legacy-vendor-specific baseline
label to `JSON migration baseline` during final removal. No structural,
numeric, ordering, or formatting expectation changed.

## Exact schema diagnostics

The baseline captured and the existing tests assert these stable diagnostics:

```text
Schema violation at #/accessors/0/byteOffset due to minimum
Schema violation at #/accessors/0/count due to minimum
Schema violation at #/accessors/0 due to dependencies
Schema violation at #/buffers/0/byteLength due to minimum
Schema violation at #/bufferViews/0/byteOffset due to minimum
Schema violation at #/bufferViews/0/byteLength due to minimum
Schema violation at # due to dependencies
```

Malformed typed-member regressions also produce these exact deserializer
messages before throwing:

```text
baseColorFactor must be an array of 4 numeric elements
emissiveFactor must be an array of 3 numeric elements
A node must have a matrix transform with 16 numeric elements
A node must have a scale with 3 numeric elements
A node must have a translation with 3 numeric elements
A node must have a rotation with 4 numeric elements
MeshPrimitive attributes must be a JSON object
Camera perspective must be a JSON object
Camera orthographic must be a JSON object
pbrMetallicRoughness must be a JSON object
Node children must be a JSON array
Node children array elements must be unsigned integers
The weights member must be a JSON array
The weights array elements must be JSON numbers
```

Malformed JSON categories such as trailing content and unterminated strings
are rejected with:

```text
The document is invalid due to bad JSON formatting
```

Passing a JSON array as the root with schema validation disabled reaches a
RapidJSON `IsObject()` debug assertion instead of an SDK exception. This is a
known baseline defect, not a compatibility requirement.

## BOM, duplicate, UTF-8, and flag behavior

Empirical compact-input results:

| Case | RapidJSON baseline |
| --- | --- |
| String overload, UTF-8 BOM, `IgnoreByteOrderMark` | accepted |
| Stream overload, UTF-8 BOM directly followed by `{`, `IgnoreByteOrderMark` | rejected with bad JSON formatting |
| String or stream overload, BOM without ignore flag | rejected with bad JSON formatting |
| Duplicate `asset.generator` keys (`first`, then `second`) | accepted; first value observed |
| Invalid UTF-8 bytes `C3 28` in `asset.generator` | accepted and preserved |

The compact-stream BOM rejection is the known encoded-stream wiring defect:
the encoded wrapper consumes the BOM but parsing is performed on the
underlying stream wrapper, losing the first JSON character. Existing tests
used whitespace after the BOM and therefore did not expose it.

Duplicate-key acceptance, invalid UTF-8 acceptance, and compact-stream BOM
loss are approved migration deltas. They are recorded here but deliberately
are not compatibility goldens.

The sparse schema flag values are:

```text
DisableSchemaAccessorSparse        = 0x80
DisableSchemaAccessorSparseValues  = 0x80
DisableSchemaAccessorSparseIndices = 0x100
```

The alias between the first two values is frozen as baseline evidence for the
later schema-flag regression tests.
