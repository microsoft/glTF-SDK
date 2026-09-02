# Known downstream source breaks for glTF-SDK 2.0

Adjacent repositories were inspected for documentation only and were not
modified.

## Replacement patterns

| Removed use | 2.0 replacement |
| --- | --- |
| `Serialize(extras.GetDocument())` | `extras.ToJson()` |
| `extras.GetDocument().HasMember(name)` | `extras.HasMember(name)` |
| DOM-taking `ValidateDocumentAgainstSchema` | pass the serialized JSON string |
| direct `GLTFSDK/RapidJsonUtils.h` extension construction | create serialized extension JSON with consumer-owned code and store the string |

## Confirmed paths

- `CoreUtils/Shared/CoreUtils/cpp/Source/Gltf.cpp`
- `Engine/Products/Components/NativeRenderer/cpp/Source/Engine/Serializing/GLTFSerializerWriter.cpp`
- `Transcoders/Shared/Transcoders.Test/Source/TranscoderGLTF/TranscoderGLTFTests.cpp`
- `Transcoders/Shared/Transcoders/Source/ExporterGLTF/Asset3DToGLTFConverter.cpp`

`Asset3DToGLTFConverter.cpp` contains several direct SDK/RapidJSON extension
builders and will require the broadest source update.

These paths are intentionally not edited from this repository. Downstream
owners should migrate atomically to the 2.0 headers/library and remove any SDK
RapidJSON include/package assumptions.
