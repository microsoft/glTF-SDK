# End-to-end glTF/GLB load-export benchmark

Generated: 2026-09-02T15:40:19.0813963Z
Baseline: rapidjson-1.9.5 on perf/Release-1.9.5-load-export at 695b90de1f044a1081e00049d5c33c69152159e6
Candidate: nlohmann-2.0.0 on Release/2.0.0 at 47c456f0f817eba6fe42af1561d016e767c2453e
Exact upstream Release/1.9.5 base: 3193f83265a70585093f13d651167b763979ade1

## Method

- Optimized MSVC x64 Release builds use the same generator, compiler, flags, machine, disk, harness source, assets, and output medium.
- Each implementation receives 5 alternating warm-up cycles. Small/medium cases use 100 measured samples and the three genuinely large cases use 30. Odd cycles run RapidJSON first; even cycles run nlohmann first.
- The 30-sample large-case count is justified by a preliminary complete cycle: about 13 seconds for RapidJSON and 82-91 seconds for nlohmann, dominated by NodePerformanceTest schema/object construction plus the required untimed output reload checks. The reduction is identical on both branches.
- Each cycle uses the same deterministic shuffled asset/operation order in both implementations.
- All eight benchmark/corpus files are byte-identical between branches; their SHA-256 values are recorded in load-export-environment.json.
- The shared KHR::GetKHRExtensionDeserializer/Serializer set is enabled globally. Supported extensions take the SDK-typed path and unsupported extensions stay in the raw glTFProperty::extensions path, exactly as listed per case below.
- LOAD starts before opening the source file and ends after Deserialize plus complete SDK reads of every buffer and every encoded image resource.
- EXPORT starts with the loaded representation and ends after Serialize, resource writes, GLB Flush or glTF manifest write, and output stream flush/close by destruction.
- ROUNDTRIP measures those LOAD and EXPORT boundaries back-to-back. Hashing, semantic validation, directory setup, and cleanup are outside all timers.
- The process peak-working-set metric covers a whole cycle, including untimed validation; it is not an operation-specific peak. Large-enabled and standard-only process cohorts are reported separately.

glTF-SDK reads encoded image bytes but does not decode pixels, create GPU textures, upload resources, compile shaders, or render. Those activities are excluded. Encoded image file I/O is included.

## Corpus and provenance

Repository: https://github.com/KhronosGroup/glTF-Sample-Assets
Pinned commit: [9429648735279342b4c32b8745f7904196607379](https://github.com/KhronosGroup/glTF-Sample-Assets/commit/9429648735279342b4c32b8745f7904196607379)

| Case | Tier | Samples | Format | Bytes | Scenes / meshes / primitives / nodes / materials / accessors | Typed handlers | Raw-preserved | Required | License |
| --- | --- | ---: | --- | ---: | --- | --- | --- | --- | --- |
| Box-gltf | small/core | 100 | gltf | 3546 | 1 / 1 / 1 / 2 / 1 / 3 | none | none | none | [CC-BY-4.0](https://github.com/KhronosGroup/glTF-Sample-Assets/blob/9429648735279342b4c32b8745f7904196607379/Models/Box/LICENSE.md) |
| Box-glb | small/core | 100 | glb | 1664 | 1 / 1 / 1 / 2 / 1 / 3 | none | none | none | [CC-BY-4.0](https://github.com/KhronosGroup/glTF-Sample-Assets/blob/9429648735279342b4c32b8745f7904196607379/Models/Box/LICENSE.md) |
| Avocado-gltf | medium/texture-heavy | 100 | gltf | 8110895 | 1 / 1 / 1 / 1 / 1 / 5 | none | none | none | [CC0-1.0](https://github.com/KhronosGroup/glTF-Sample-Assets/blob/9429648735279342b4c32b8745f7904196607379/Models/Avocado/LICENSE.md) |
| Avocado-glb | medium/texture-heavy | 100 | glb | 8110040 | 1 / 1 / 1 / 1 / 1 / 5 | none | none | none | [CC0-1.0](https://github.com/KhronosGroup/glTF-Sample-Assets/blob/9429648735279342b4c32b8745f7904196607379/Models/Avocado/LICENSE.md) |
| MorphStressTest-glb | medium/geometry-animation | 100 | glb | 575900 | 1 / 1 / 2 / 1 / 2 / 48 | none | none | none | [CC-BY-4.0](https://github.com/KhronosGroup/glTF-Sample-Assets/blob/9429648735279342b4c32b8745f7904196607379/Models/MorphStressTest/LICENSE.md) |
| SpecularTest-glb | small/material-grid | 100 | glb | 223376 | 1 / 24 / 24 / 25 / 24 / 11 | KHR_materials_specular | none | none | [CC-BY-4.0](https://github.com/KhronosGroup/glTF-Sample-Assets/blob/9429648735279342b4c32b8745f7904196607379/Models/SpecularTest/LICENSE.md) |
| EmissiveStrengthTest-glb | small/material-grid | 100 | glb | 10668 | 1 / 6 / 6 / 6 / 6 / 15 | none | KHR_materials_emissive_strength | none | [CC-BY-4.0](https://github.com/KhronosGroup/glTF-Sample-Assets/blob/9429648735279342b4c32b8745f7904196607379/Models/EmissiveStrengthTest/LICENSE.md) |
| SimpleInstancing-glb | small/scene-instancing | 100 | glb | 7356 | 1 / 1 / 1 / 1 / 0 / 6 | EXT_mesh_gpu_instancing | none | none | [CC0-1.0](https://github.com/KhronosGroup/glTF-Sample-Assets/blob/9429648735279342b4c32b8745f7904196607379/Models/SimpleInstancing/LICENSE.md) |
| XmpMetadataRoundedCube-glb | small/metadata | 100 | glb | 105936 | 1 / 1 / 1 / 1 / 1 / 3 | none | KHR_xmp_json_ld | none | [CC-BY-4.0](https://github.com/KhronosGroup/glTF-Sample-Assets/blob/9429648735279342b4c32b8745f7904196607379/Models/XmpMetadataRoundedCube/LICENSE.md) |
| TextureTransformMultiTest-glb | medium/material-grid | 100 | glb | 388264 | 1 / 29 / 29 / 29 / 29 / 35 | KHR_materials_clearcoat, KHR_materials_unlit, KHR_texture_transform | none | KHR_texture_transform | [CC-BY-4.0](https://github.com/KhronosGroup/glTF-Sample-Assets/blob/9429648735279342b4c32b8745f7904196607379/Models/TextureTransformMultiTest/LICENSE.md) |
| IridescenceSuzanne-glb | medium/material-scene | 100 | glb | 507608 | 1 / 3 / 3 / 4 / 3 / 4 | KHR_materials_iridescence, KHR_materials_transmission, KHR_materials_volume | KHR_lights_punctual, KHR_materials_ior | KHR_materials_iridescence | [CC0-1.0](https://github.com/KhronosGroup/glTF-Sample-Assets/blob/9429648735279342b4c32b8745f7904196607379/Models/IridescenceSuzanne/LICENSE.md) |
| SheenTestGrid-glb | medium/material-grid | 100 | glb | 1841064 | 1 / 20 / 20 / 20 / 19 / 61 | KHR_materials_sheen | none | KHR_materials_sheen | [CC0-1.0](https://github.com/KhronosGroup/glTF-Sample-Assets/blob/9429648735279342b4c32b8745f7904196607379/Models/SheenTestGrid/LICENSE.md) |
| MaterialsVariantsShoe-glb | medium/material-texture | 100 | glb | 7833592 | 1 / 1 / 1 / 3 / 3 / 4 | none | KHR_materials_variants | none | [CC-BY-4.0](https://github.com/KhronosGroup/glTF-Sample-Assets/blob/9429648735279342b4c32b8745f7904196607379/Models/MaterialsVariantsShoe/LICENSE.md) |
| ABeautifulGame-glb | large/material-texture | 30 | glb | 42977928 | 1 / 15 / 15 / 49 / 15 / 75 | KHR_materials_transmission, KHR_materials_volume | none | none | [CC-BY-4.0](https://github.com/KhronosGroup/glTF-Sample-Assets/blob/9429648735279342b4c32b8745f7904196607379/Models/ABeautifulGame/LICENSE.md) |
| ABeautifulGame-draco-glb | large/compressed-material | 30 | glb | 12105252 | 1 / 15 / 15 / 49 / 15 / 32 | KHR_draco_mesh_compression, KHR_materials_transmission, KHR_materials_volume | KHR_texture_basisu | KHR_draco_mesh_compression, KHR_texture_basisu | [CC-BY-4.0](https://github.com/KhronosGroup/glTF-Sample-Assets/blob/9429648735279342b4c32b8745f7904196607379/Models/ABeautifulGame/LICENSE.md) |
| NodePerformanceTest-glb | large/scene-structure | 30 | glb | 37986536 | 1 / 10000 / 10000 / 10002 / 10000 / 39993 | none | KHR_lights_punctual | KHR_lights_punctual | [CC0-1.0](https://github.com/KhronosGroup/glTF-Sample-Assets/blob/9429648735279342b4c32b8745f7904196607379/Models/NodePerformanceTest/LICENSE.md) |

The regular ABeautifulGame GLB (42,977,928 bytes) and NodePerformanceTest GLB (37,986,536 bytes; 10,002 nodes, 10,000 meshes/materials) are both materially larger and structurally more complex than Avocado. The compressed ABeautifulGame variant adds typed Draco plus raw required BasisU coverage.

| Pinned source file | Bytes | SHA-256 | Immutable URL |
| --- | ---: | --- | --- |
| Models/Box/glTF/Box.gltf | 2898 | 4A0D69EECFCE0672A50B71DC218CBACEC6C53FE2445040C235C6314B1B2C41B9 | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/Box/glTF/Box.gltf) |
| Models/Box/glTF/Box0.bin | 648 | 3266A8E39B9F425B3341CBE5EEC7849F44310256BFA651E6B8B40C85CE0CCAFB | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/Box/glTF/Box0.bin) |
| Models/Box/glTF-Binary/Box.glb | 1664 | ED52F7192B8311D700AC0CE80644E3852CD01537E4D62241B9ACBA023DA3D54E | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/Box/glTF-Binary/Box.glb) |
| Models/Avocado/glTF/Avocado.gltf | 2413 | C9BDE1D2F09514DAD0A16971DF2222BEB582FAD5DFC86203DE1D56389D52FEDA | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/Avocado/glTF/Avocado.gltf) |
| Models/Avocado/glTF/Avocado.bin | 23580 | 3BF7CE0EC994562EABF0CB72A52BBC7976818D630D62609A966B53A5FF9B9180 | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/Avocado/glTF/Avocado.bin) |
| Models/Avocado/glTF/Avocado_baseColor.png | 3158729 | 385DCE948C3B9E8BC93E1C930D796E72B02CBD474B121A160D577DE95EBBB48F | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/Avocado/glTF/Avocado_baseColor.png) |
| Models/Avocado/glTF/Avocado_normal.png | 3271114 | 25783C211761B7E32497FDD3E2490F3BAC05B738E5E2BC62FC5B78A270BB4E50 | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/Avocado/glTF/Avocado_normal.png) |
| Models/Avocado/glTF/Avocado_roughnessMetallic.png | 1655059 | 33EE80F7CCFD36825ACFBD5EF0A51326BEA49E33EFED852DCA44798798BD6901 | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/Avocado/glTF/Avocado_roughnessMetallic.png) |
| Models/Avocado/glTF-Binary/Avocado.glb | 8110040 | CCC9C3CE56423720B09399C2351537207CD5A65F859F9E6E2F30922762F3ABD4 | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/Avocado/glTF-Binary/Avocado.glb) |
| Models/MorphStressTest/glTF-Binary/MorphStressTest.glb | 575900 | 005F1D9DD938C553A506D6BF3D21830FC3F2EC3A199D9332E9A625BAF7342BEF | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/MorphStressTest/glTF-Binary/MorphStressTest.glb) |
| Models/SpecularTest/glTF-Binary/SpecularTest.glb | 223376 | CF789C68C3AB4B74877DA3C5992C612C213F9C901ED4FAFBF9BE362F434E48D9 | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/SpecularTest/glTF-Binary/SpecularTest.glb) |
| Models/EmissiveStrengthTest/glTF-Binary/EmissiveStrengthTest.glb | 10668 | 074898EC4C3636230FAFFE0CACAC7403C3F6135DB61F943DA28FFAA88CA2A39F | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/EmissiveStrengthTest/glTF-Binary/EmissiveStrengthTest.glb) |
| Models/SimpleInstancing/glTF-Binary/SimpleInstancing.glb | 7356 | 1C9425627481346A8C226118F7000DB7CD7F80198E9F54331F8B664D16726F3F | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/SimpleInstancing/glTF-Binary/SimpleInstancing.glb) |
| Models/XmpMetadataRoundedCube/glTF-Binary/XmpMetadataRoundedCube.glb | 105936 | 92A22170651513274577F81A3AF1CF548C2049E31CFEDE85A5E96170C583E451 | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/XmpMetadataRoundedCube/glTF-Binary/XmpMetadataRoundedCube.glb) |
| Models/TextureTransformMultiTest/glTF-Binary/TextureTransformMultiTest.glb | 388264 | 569AEDB53822D5721E7E06AF5983348683D4B2FFB1D469338AD4F02BF6A74911 | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/TextureTransformMultiTest/glTF-Binary/TextureTransformMultiTest.glb) |
| Models/IridescenceSuzanne/glTF-Binary/IridescenceSuzanne.glb | 507608 | 866762602D60A0942B7608BFABF4E8686A29033636224F4B55C5E56ABE8ABF8F | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/IridescenceSuzanne/glTF-Binary/IridescenceSuzanne.glb) |
| Models/SheenTestGrid/glTF-Binary/SheenTestGrid.glb | 1841064 | B3D82DDE0AE6B93BEF1A8085EC05540B21139F9917BFE771135C5AED9BA1C101 | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/SheenTestGrid/glTF-Binary/SheenTestGrid.glb) |
| Models/MaterialsVariantsShoe/glTF-Binary/MaterialsVariantsShoe.glb | 7833592 | E1D7CB190382111E5A5B37B51E9A7F007F7EB2AB1B6185E0188E8D0A0D1265A7 | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/MaterialsVariantsShoe/glTF-Binary/MaterialsVariantsShoe.glb) |
| Models/ABeautifulGame/glTF-Binary/ABeautifulGame.glb | 42977928 | BD7133B4B322AAE97C589B8839DAE8155AD2546ACB35AE32A127E722A959D007 | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/ABeautifulGame/glTF-Binary/ABeautifulGame.glb) |
| Models/ABeautifulGame/glTF-Binary-KTX-ETC1S-Draco/ABeautifulGame.glb | 12105252 | D950162D41E0064BE9DFAC0C09D80C7A23719A37AF37F08CD79385B8AF0AF0C1 | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/ABeautifulGame/glTF-Binary-KTX-ETC1S-Draco/ABeautifulGame.glb) |
| Models/NodePerformanceTest/glTF-Binary/NodePerformanceTest.glb | 37986536 | 81EEC3B14B8ED25068448FFDC824528D03F1E844B20E017F5C3AAB3F076B1FB8 | [raw](https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/9429648735279342b4c32b8745f7904196607379/Models/NodePerformanceTest/glTF-Binary/NodePerformanceTest.glb) |

## Timing comparison

Positive deltas mean nlohmann Release/2.0.0 is slower; negative deltas mean it is faster.

| Case | Tier | Operation | N | RapidJSON median (ms) | nlohmann median (ms) | Median delta (ms) | Delta | RapidJSON p95 (ms) | nlohmann p95 (ms) | p95 delta |
| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| ABeautifulGame-draco-glb | large/compressed-material | export | 30 | 71.615 | 72.048 | 0.433 | 0.60% | 129.289 | 133.475 | 3.24% |
| ABeautifulGame-glb | large/material-texture | export | 30 | 251.953 | 256.986 | 5.033 | 2.00% | 328.954 | 287.526 | -12.59% |
| ABeautifulGame-draco-glb | large/compressed-material | load | 30 | 31.697 | 34.937 | 3.239 | 10.22% | 40.824 | 49.235 | 20.60% |
| ABeautifulGame-glb | large/material-texture | load | 30 | 75.329 | 78.912 | 3.582 | 4.76% | 92.389 | 124.865 | 35.15% |
| ABeautifulGame-draco-glb | large/compressed-material | roundtrip | 30 | 103.262 | 108.064 | 4.801 | 4.65% | 113.405 | 131.955 | 16.36% |
| ABeautifulGame-glb | large/material-texture | roundtrip | 30 | 331.815 | 338.086 | 6.271 | 1.89% | 398.998 | 458.764 | 14.98% |
| Avocado-glb | medium/texture-heavy | export | 100 | 48.032 | 47.674 | -0.358 | -0.75% | 103.578 | 96.496 | -6.84% |
| Avocado-glb | medium/texture-heavy | load | 100 | 20.266 | 22.647 | 2.381 | 11.75% | 27.768 | 31.636 | 13.93% |
| Avocado-glb | medium/texture-heavy | roundtrip | 100 | 69.397 | 70.387 | 0.989 | 1.43% | 136.404 | 132.672 | -2.74% |
| Avocado-gltf | medium/texture-heavy | export | 100 | 4.789 | 4.904 | 0.115 | 2.41% | 6.080 | 6.347 | 4.39% |
| Avocado-gltf | medium/texture-heavy | load | 100 | 16.274 | 18.494 | 2.220 | 13.64% | 20.708 | 25.411 | 22.71% |
| Avocado-gltf | medium/texture-heavy | roundtrip | 100 | 21.196 | 23.584 | 2.388 | 11.27% | 27.920 | 69.193 | 147.82% |
| Box-glb | small/core | export | 100 | 0.552 | 0.616 | 0.064 | 11.62% | 0.705 | 0.748 | 6.05% |
| Box-glb | small/core | load | 100 | 2.757 | 4.975 | 2.218 | 80.45% | 4.321 | 8.040 | 86.07% |
| Box-glb | small/core | roundtrip | 100 | 3.248 | 5.602 | 2.353 | 72.44% | 4.646 | 7.995 | 72.09% |
| Box-gltf | small/core | export | 100 | 0.823 | 0.885 | 0.062 | 7.60% | 1.126 | 1.176 | 4.42% |
| Box-gltf | small/core | load | 100 | 2.974 | 5.242 | 2.268 | 76.28% | 4.282 | 8.443 | 97.19% |
| Box-gltf | small/core | roundtrip | 100 | 3.780 | 6.152 | 2.372 | 62.76% | 5.769 | 9.579 | 66.04% |
| EmissiveStrengthTest-glb | small/material-grid | export | 100 | 0.682 | 0.887 | 0.205 | 30.12% | 0.865 | 1.159 | 34.03% |
| EmissiveStrengthTest-glb | small/material-grid | load | 100 | 3.321 | 5.718 | 2.397 | 72.17% | 4.570 | 9.088 | 98.87% |
| EmissiveStrengthTest-glb | small/material-grid | roundtrip | 100 | 4.049 | 6.645 | 2.596 | 64.10% | 5.961 | 9.185 | 54.09% |
| IridescenceSuzanne-glb | medium/material-scene | export | 100 | 3.651 | 3.895 | 0.244 | 6.68% | 5.722 | 6.212 | 8.57% |
| IridescenceSuzanne-glb | medium/material-scene | load | 100 | 3.852 | 6.293 | 2.441 | 63.35% | 5.330 | 9.756 | 83.05% |
| IridescenceSuzanne-glb | medium/material-scene | roundtrip | 100 | 7.575 | 10.111 | 2.536 | 33.47% | 12.014 | 17.335 | 44.30% |
| MaterialsVariantsShoe-glb | medium/material-texture | export | 100 | 46.405 | 46.889 | 0.483 | 1.04% | 99.116 | 95.925 | -3.22% |
| MaterialsVariantsShoe-glb | medium/material-texture | load | 100 | 19.264 | 21.904 | 2.640 | 13.71% | 24.463 | 32.809 | 34.12% |
| MaterialsVariantsShoe-glb | medium/material-texture | roundtrip | 100 | 66.906 | 69.403 | 2.497 | 3.73% | 135.926 | 135.900 | -0.02% |
| MorphStressTest-glb | medium/geometry-animation | export | 100 | 4.197 | 4.660 | 0.464 | 11.05% | 5.790 | 6.164 | 6.46% |
| MorphStressTest-glb | medium/geometry-animation | load | 100 | 4.939 | 7.186 | 2.247 | 45.50% | 6.605 | 11.777 | 78.32% |
| MorphStressTest-glb | medium/geometry-animation | roundtrip | 100 | 9.115 | 11.822 | 2.707 | 29.70% | 12.774 | 15.902 | 24.49% |
| NodePerformanceTest-glb | large/scene-structure | export | 30 | 303.394 | 877.492 | 574.098 | 189.23% | 337.618 | 932.843 | 176.30% |
| NodePerformanceTest-glb | large/scene-structure | load | 30 | 1883.577 | 15535.308 | 13651.731 | 724.78% | 2091.950 | 16866.381 | 706.25% |
| NodePerformanceTest-glb | large/scene-structure | roundtrip | 30 | 2150.800 | 16419.031 | 14268.231 | 663.39% | 2404.986 | 16990.397 | 606.47% |
| SheenTestGrid-glb | medium/material-grid | export | 100 | 11.723 | 12.544 | 0.821 | 7.01% | 14.398 | 19.802 | 37.53% |
| SheenTestGrid-glb | medium/material-grid | load | 100 | 7.598 | 10.502 | 2.904 | 38.23% | 11.105 | 14.372 | 29.42% |
| SheenTestGrid-glb | medium/material-grid | roundtrip | 100 | 19.107 | 23.120 | 4.012 | 21.00% | 24.922 | 31.052 | 24.60% |
| SimpleInstancing-glb | small/scene-instancing | export | 100 | 0.637 | 0.709 | 0.071 | 11.15% | 0.815 | 0.989 | 21.34% |
| SimpleInstancing-glb | small/scene-instancing | load | 100 | 2.869 | 5.064 | 2.195 | 76.51% | 4.239 | 8.073 | 90.44% |
| SimpleInstancing-glb | small/scene-instancing | roundtrip | 100 | 3.438 | 5.821 | 2.383 | 69.33% | 5.291 | 7.810 | 47.60% |
| SpecularTest-glb | small/material-grid | export | 100 | 2.080 | 2.644 | 0.564 | 27.11% | 3.066 | 3.841 | 25.31% |
| SpecularTest-glb | small/material-grid | load | 100 | 4.425 | 7.311 | 2.886 | 65.22% | 6.679 | 11.834 | 77.19% |
| SpecularTest-glb | small/material-grid | roundtrip | 100 | 6.511 | 9.936 | 3.425 | 52.61% | 10.051 | 12.817 | 27.52% |
| TextureTransformMultiTest-glb | medium/material-grid | export | 100 | 3.175 | 4.275 | 1.100 | 34.66% | 4.121 | 5.832 | 41.52% |
| TextureTransformMultiTest-glb | medium/material-grid | load | 100 | 6.202 | 9.345 | 3.142 | 50.66% | 9.138 | 17.485 | 91.34% |
| TextureTransformMultiTest-glb | medium/material-grid | roundtrip | 100 | 9.421 | 13.813 | 4.392 | 46.62% | 15.381 | 18.879 | 22.74% |
| XmpMetadataRoundedCube-glb | small/metadata | export | 100 | 1.281 | 1.433 | 0.151 | 11.80% | 1.799 | 2.225 | 23.71% |
| XmpMetadataRoundedCube-glb | small/metadata | load | 100 | 2.900 | 5.160 | 2.260 | 77.91% | 4.364 | 8.214 | 88.21% |
| XmpMetadataRoundedCube-glb | small/metadata | roundtrip | 100 | 4.200 | 6.611 | 2.411 | 57.41% | 5.774 | 10.170 | 76.14% |

## Aggregate views

Summed medians weight cases by observed runtime. The geometric delta is also weighted by each case's RapidJSON median, so tiny assets cannot dominate the aggregate. These are descriptive summaries, not inferential statistics.

### Overall

| Group | Operation | Cases | RapidJSON summed median (ms) | nlohmann summed median (ms) | Delta (ms) | Aggregate delta | Baseline-time-weighted geometric delta | Aggregate p95 delta |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| all-cases | export | 16 | 754.987 | 1338.539 | 583.552 | 77.29% | 55.08% | 53.47% |
| all-cases | load | 16 | 2088.244 | 15778.995 | 13690.751 | 655.61% | 581.31% | 630.79% |
| all-cases | roundtrip | 16 | 2813.820 | 17128.186 | 14314.366 | 508.72% | 379.73% | 443.93% |

### Complexity class

| Group | Operation | Cases | RapidJSON summed median (ms) | nlohmann summed median (ms) | Delta (ms) | Aggregate delta | Baseline-time-weighted geometric delta | Aggregate p95 delta |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| large | export | 3 | 626.962 | 1206.525 | 579.563 | 92.44% | 68.64% | 70.11% |
| large | load | 3 | 1990.604 | 15649.156 | 13658.553 | 686.15% | 638.76% | 665.81% |
| large | roundtrip | 3 | 2585.877 | 16865.180 | 14279.303 | 552.20% | 444.57% | 502.63% |
| medium | export | 7 | 121.970 | 124.840 | 2.870 | 2.35% | 2.20% | -0.85% |
| medium | load | 7 | 78.394 | 96.369 | 17.975 | 22.93% | 21.94% | 36.28% |
| medium | roundtrip | 7 | 202.717 | 222.239 | 19.522 | 9.63% | 9.02% | 15.22% |
| small | export | 6 | 6.055 | 7.173 | 1.118 | 18.47% | 18.15% | 21.05% |
| small | load | 6 | 19.246 | 33.470 | 14.224 | 73.91% | 73.82% | 88.69% |
| small | roundtrip | 6 | 25.226 | 40.767 | 15.541 | 61.61% | 61.46% | 53.52% |

### Detailed complexity tier

| Group | Operation | Cases | RapidJSON summed median (ms) | nlohmann summed median (ms) | Delta (ms) | Aggregate delta | Baseline-time-weighted geometric delta | Aggregate p95 delta |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| large/compressed-material | export | 1 | 71.615 | 72.048 | 0.433 | 0.60% | 0.60% | 3.24% |
| large/compressed-material | load | 1 | 31.697 | 34.937 | 3.239 | 10.22% | 10.22% | 20.60% |
| large/compressed-material | roundtrip | 1 | 103.262 | 108.064 | 4.801 | 4.65% | 4.65% | 16.36% |
| large/material-texture | export | 1 | 251.953 | 256.986 | 5.033 | 2.00% | 2.00% | -12.59% |
| large/material-texture | load | 1 | 75.329 | 78.912 | 3.582 | 4.76% | 4.76% | 35.15% |
| large/material-texture | roundtrip | 1 | 331.815 | 338.086 | 6.271 | 1.89% | 1.89% | 14.98% |
| large/scene-structure | export | 1 | 303.394 | 877.492 | 574.098 | 189.23% | 189.23% | 176.30% |
| large/scene-structure | load | 1 | 1883.577 | 15535.308 | 13651.731 | 724.78% | 724.78% | 706.25% |
| large/scene-structure | roundtrip | 1 | 2150.800 | 16419.031 | 14268.231 | 663.39% | 663.39% | 606.47% |
| medium/geometry-animation | export | 1 | 4.197 | 4.660 | 0.464 | 11.05% | 11.05% | 6.46% |
| medium/geometry-animation | load | 1 | 4.939 | 7.186 | 2.247 | 45.50% | 45.50% | 78.32% |
| medium/geometry-animation | roundtrip | 1 | 9.115 | 11.822 | 2.707 | 29.70% | 29.70% | 24.49% |
| medium/material-grid | export | 2 | 14.897 | 16.819 | 1.921 | 12.90% | 12.38% | 38.42% |
| medium/material-grid | load | 2 | 13.800 | 19.846 | 6.046 | 43.82% | 43.68% | 57.37% |
| medium/material-grid | roundtrip | 2 | 28.528 | 36.933 | 8.405 | 29.46% | 28.92% | 23.89% |
| medium/material-scene | export | 1 | 3.651 | 3.895 | 0.244 | 6.68% | 6.68% | 8.57% |
| medium/material-scene | load | 1 | 3.852 | 6.293 | 2.441 | 63.35% | 63.35% | 83.05% |
| medium/material-scene | roundtrip | 1 | 7.575 | 10.111 | 2.536 | 33.47% | 33.47% | 44.30% |
| medium/material-texture | export | 1 | 46.405 | 46.889 | 0.483 | 1.04% | 1.04% | -3.22% |
| medium/material-texture | load | 1 | 19.264 | 21.904 | 2.640 | 13.71% | 13.71% | 34.12% |
| medium/material-texture | roundtrip | 1 | 66.906 | 69.403 | 2.497 | 3.73% | 3.73% | -0.02% |
| medium/texture-heavy | export | 2 | 52.821 | 52.578 | -0.243 | -0.46% | -0.46% | -6.22% |
| medium/texture-heavy | load | 2 | 36.540 | 41.140 | 4.600 | 12.59% | 12.59% | 17.68% |
| medium/texture-heavy | roundtrip | 2 | 90.593 | 93.971 | 3.377 | 3.73% | 3.65% | 22.85% |
| small/core | export | 2 | 1.374 | 1.501 | 0.127 | 9.21% | 9.19% | 5.05% |
| small/core | load | 2 | 5.731 | 10.217 | 4.486 | 78.28% | 78.27% | 91.61% |
| small/core | roundtrip | 2 | 7.028 | 11.754 | 4.725 | 67.23% | 67.17% | 68.74% |
| small/material-grid | export | 2 | 2.762 | 3.531 | 0.769 | 27.86% | 27.85% | 27.23% |
| small/material-grid | load | 2 | 7.746 | 13.029 | 5.283 | 68.20% | 68.17% | 86.00% |
| small/material-grid | roundtrip | 2 | 10.560 | 16.581 | 6.021 | 57.02% | 56.92% | 37.41% |
| small/metadata | export | 1 | 1.281 | 1.433 | 0.151 | 11.80% | 11.80% | 23.71% |
| small/metadata | load | 1 | 2.900 | 5.160 | 2.260 | 77.91% | 77.91% | 88.21% |
| small/metadata | roundtrip | 1 | 4.200 | 6.611 | 2.411 | 57.41% | 57.41% | 76.14% |
| small/scene-instancing | export | 1 | 0.637 | 0.709 | 0.071 | 11.15% | 11.15% | 21.34% |
| small/scene-instancing | load | 1 | 2.869 | 5.064 | 2.195 | 76.51% | 76.51% | 90.44% |
| small/scene-instancing | roundtrip | 1 | 3.438 | 5.821 | 2.383 | 69.33% | 69.33% | 47.60% |

### Extension execution mode

| Group | Operation | Cases | RapidJSON summed median (ms) | nlohmann summed median (ms) | Delta (ms) | Aggregate delta | Baseline-time-weighted geometric delta | Aggregate p95 delta |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| core-only | export | 5 | 58.391 | 58.739 | 0.348 | 0.60% | 0.54% | -5.41% |
| core-only | load | 5 | 47.209 | 58.543 | 11.334 | 24.01% | 22.28% | 33.96% |
| core-only | roundtrip | 5 | 106.737 | 117.546 | 10.810 | 10.13% | 9.03% | 25.51% |
| mixed-typed-raw | export | 2 | 75.266 | 75.942 | 0.677 | 0.90% | 0.89% | 3.46% |
| mixed-typed-raw | load | 2 | 35.549 | 41.229 | 5.680 | 15.98% | 15.02% | 27.81% |
| mixed-typed-raw | roundtrip | 2 | 110.837 | 118.174 | 7.337 | 6.62% | 6.40% | 19.03% |
| raw-only | export | 4 | 351.762 | 926.700 | 574.938 | 163.45% | 150.50% | 134.90% |
| raw-only | load | 4 | 1909.062 | 15568.090 | 13659.028 | 715.48% | 704.37% | 695.94% |
| raw-only | roundtrip | 4 | 2225.955 | 16501.690 | 14275.735 | 641.33% | 614.80% | 571.68% |
| typed-only | export | 5 | 269.568 | 277.157 | 7.589 | 2.82% | 2.74% | -9.50% |
| typed-only | load | 5 | 96.423 | 111.133 | 14.710 | 15.26% | 13.67% | 42.96% |
| typed-only | roundtrip | 5 | 370.291 | 390.775 | 20.484 | 5.53% | 4.99% | 16.43% |

### Exact extension coverage

| Group | Operation | Cases | RapidJSON summed median (ms) | nlohmann summed median (ms) | Delta (ms) | Aggregate delta | Baseline-time-weighted geometric delta | Aggregate p95 delta |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| EXT_mesh_gpu_instancing | export | 1 | 0.637 | 0.709 | 0.071 | 11.15% | 11.15% | 21.34% |
| EXT_mesh_gpu_instancing | load | 1 | 2.869 | 5.064 | 2.195 | 76.51% | 76.51% | 90.44% |
| EXT_mesh_gpu_instancing | roundtrip | 1 | 3.438 | 5.821 | 2.383 | 69.33% | 69.33% | 47.60% |
| KHR_draco_mesh_compression | export | 1 | 71.615 | 72.048 | 0.433 | 0.60% | 0.60% | 3.24% |
| KHR_draco_mesh_compression | load | 1 | 31.697 | 34.937 | 3.239 | 10.22% | 10.22% | 20.60% |
| KHR_draco_mesh_compression | roundtrip | 1 | 103.262 | 108.064 | 4.801 | 4.65% | 4.65% | 16.36% |
| KHR_lights_punctual | export | 2 | 307.045 | 881.387 | 574.342 | 187.05% | 185.82% | 173.51% |
| KHR_lights_punctual | load | 2 | 1887.430 | 15541.601 | 13654.172 | 723.43% | 722.06% | 704.67% |
| KHR_lights_punctual | roundtrip | 2 | 2158.375 | 16429.141 | 14270.766 | 661.18% | 658.73% | 603.67% |
| KHR_materials_clearcoat | export | 1 | 3.175 | 4.275 | 1.100 | 34.66% | 34.66% | 41.52% |
| KHR_materials_clearcoat | load | 1 | 6.202 | 9.345 | 3.142 | 50.66% | 50.66% | 91.34% |
| KHR_materials_clearcoat | roundtrip | 1 | 9.421 | 13.813 | 4.392 | 46.62% | 46.62% | 22.74% |
| KHR_materials_emissive_strength | export | 1 | 0.682 | 0.887 | 0.205 | 30.12% | 30.12% | 34.03% |
| KHR_materials_emissive_strength | load | 1 | 3.321 | 5.718 | 2.397 | 72.17% | 72.17% | 98.87% |
| KHR_materials_emissive_strength | roundtrip | 1 | 4.049 | 6.645 | 2.596 | 64.10% | 64.10% | 54.09% |
| KHR_materials_ior | export | 1 | 3.651 | 3.895 | 0.244 | 6.68% | 6.68% | 8.57% |
| KHR_materials_ior | load | 1 | 3.852 | 6.293 | 2.441 | 63.35% | 63.35% | 83.05% |
| KHR_materials_ior | roundtrip | 1 | 7.575 | 10.111 | 2.536 | 33.47% | 33.47% | 44.30% |
| KHR_materials_iridescence | export | 1 | 3.651 | 3.895 | 0.244 | 6.68% | 6.68% | 8.57% |
| KHR_materials_iridescence | load | 1 | 3.852 | 6.293 | 2.441 | 63.35% | 63.35% | 83.05% |
| KHR_materials_iridescence | roundtrip | 1 | 7.575 | 10.111 | 2.536 | 33.47% | 33.47% | 44.30% |
| KHR_materials_sheen | export | 1 | 11.723 | 12.544 | 0.821 | 7.01% | 7.01% | 37.53% |
| KHR_materials_sheen | load | 1 | 7.598 | 10.502 | 2.904 | 38.23% | 38.23% | 29.42% |
| KHR_materials_sheen | roundtrip | 1 | 19.107 | 23.120 | 4.012 | 21.00% | 21.00% | 24.60% |
| KHR_materials_specular | export | 1 | 2.080 | 2.644 | 0.564 | 27.11% | 27.11% | 25.31% |
| KHR_materials_specular | load | 1 | 4.425 | 7.311 | 2.886 | 65.22% | 65.22% | 77.19% |
| KHR_materials_specular | roundtrip | 1 | 6.511 | 9.936 | 3.425 | 52.61% | 52.61% | 27.52% |
| KHR_materials_transmission | export | 3 | 327.219 | 332.928 | 5.709 | 1.74% | 1.74% | -7.92% |
| KHR_materials_transmission | load | 3 | 110.879 | 120.141 | 9.262 | 8.35% | 7.94% | 32.71% |
| KHR_materials_transmission | roundtrip | 3 | 442.652 | 456.260 | 13.608 | 3.07% | 3.00% | 15.95% |
| KHR_materials_unlit | export | 1 | 3.175 | 4.275 | 1.100 | 34.66% | 34.66% | 41.52% |
| KHR_materials_unlit | load | 1 | 6.202 | 9.345 | 3.142 | 50.66% | 50.66% | 91.34% |
| KHR_materials_unlit | roundtrip | 1 | 9.421 | 13.813 | 4.392 | 46.62% | 46.62% | 22.74% |
| KHR_materials_variants | export | 1 | 46.405 | 46.889 | 0.483 | 1.04% | 1.04% | -3.22% |
| KHR_materials_variants | load | 1 | 19.264 | 21.904 | 2.640 | 13.71% | 13.71% | 34.12% |
| KHR_materials_variants | roundtrip | 1 | 66.906 | 69.403 | 2.497 | 3.73% | 3.73% | -0.02% |
| KHR_materials_volume | export | 3 | 327.219 | 332.928 | 5.709 | 1.74% | 1.74% | -7.92% |
| KHR_materials_volume | load | 3 | 110.879 | 120.141 | 9.262 | 8.35% | 7.94% | 32.71% |
| KHR_materials_volume | roundtrip | 3 | 442.652 | 456.260 | 13.608 | 3.07% | 3.00% | 15.95% |
| KHR_texture_basisu | export | 1 | 71.615 | 72.048 | 0.433 | 0.60% | 0.60% | 3.24% |
| KHR_texture_basisu | load | 1 | 31.697 | 34.937 | 3.239 | 10.22% | 10.22% | 20.60% |
| KHR_texture_basisu | roundtrip | 1 | 103.262 | 108.064 | 4.801 | 4.65% | 4.65% | 16.36% |
| KHR_texture_transform | export | 1 | 3.175 | 4.275 | 1.100 | 34.66% | 34.66% | 41.52% |
| KHR_texture_transform | load | 1 | 6.202 | 9.345 | 3.142 | 50.66% | 50.66% | 91.34% |
| KHR_texture_transform | roundtrip | 1 | 9.421 | 13.813 | 4.392 | 46.62% | 46.62% | 22.74% |
| KHR_xmp_json_ld | export | 1 | 1.281 | 1.433 | 0.151 | 11.80% | 11.80% | 23.71% |
| KHR_xmp_json_ld | load | 1 | 2.900 | 5.160 | 2.260 | 77.91% | 77.91% | 88.21% |
| KHR_xmp_json_ld | roundtrip | 1 | 4.200 | 6.611 | 2.411 | 57.41% | 57.41% | 76.14% |

## Input and output size

| Asset | Format | Input bytes | RapidJSON output bytes | nlohmann output bytes | Delta bytes | Delta |
| --- | --- | ---: | ---: | ---: | ---: | ---: |
| Box | gltf | 3546 | 1618 | 1618 | 0 | 0.00% |
| Box | glb | 1664 | 1632 | 1632 | 0 | 0.00% |
| Avocado | gltf | 8110895 | 8109906 | 8109904 | -2 | -0.00% |
| Avocado | glb | 8110040 | 8110104 | 8110104 | 0 | 0.00% |
| MorphStressTest | glb | 575900 | 576088 | 576088 | 0 | 0.00% |
| SpecularTest | glb | 223376 | 224024 | 224024 | 0 | 0.00% |
| EmissiveStrengthTest | glb | 10668 | 10908 | 10908 | 0 | 0.00% |
| SimpleInstancing | glb | 7356 | 7288 | 7288 | 0 | 0.00% |
| XmpMetadataRoundedCube | glb | 105936 | 105908 | 105908 | 0 | 0.00% |
| TextureTransformMultiTest | glb | 388264 | 387988 | 387988 | 0 | 0.00% |
| IridescenceSuzanne | glb | 507608 | 507344 | 507344 | 0 | 0.00% |
| SheenTestGrid | glb | 1841064 | 1840956 | 1840956 | 0 | 0.00% |
| MaterialsVariantsShoe | glb | 7833592 | 7832868 | 7832868 | 0 | 0.00% |
| ABeautifulGame | glb | 42977928 | 42978040 | 42978040 | 0 | 0.00% |
| ABeautifulGame | glb | 12105252 | 12105120 | 12105120 | 0 | 0.00% |
| NodePerformanceTest | glb | 37986536 | 38031536 | 38031532 | -4 | -0.00% |

Per-output canonical SHA-256 values are in load-export-output-hashes.csv. Different byte hashes are permitted only when both SDK reloads produce the same source Document, buffer bytes, and encoded image bytes.
Across all cases, RapidJSON output sets total 120831328 bytes and nlohmann output sets total 120831322 bytes.

## Peak working set

| Implementation | Cohort | Measured processes | Median process peak (MiB) | Maximum process peak (MiB) |
| --- | --- | ---: | ---: | ---: |
| rapidjson-1.9.5 | all | 100 | 40.54 | 400.98 |
| rapidjson-1.9.5 | large-enabled | 30 | 380.52 | 400.98 |
| rapidjson-1.9.5 | standard-only | 70 | 40.50 | 41.12 |
| nlohmann-2.0.0 | all | 100 | 40.92 | 433.52 |
| nlohmann-2.0.0 | large-enabled | 30 | 431.51 | 433.52 |
| nlohmann-2.0.0 | standard-only | 70 | 40.87 | 41.03 |

## Interpretation

- **Fixed overhead versus scaling:** small-class LOAD summed medians change by 14.224 ms (73.91%); large-class LOAD changes by 13658.553 ms (686.15%). Read percentage and absolute changes together: tiny manifests magnify fixed setup/validation costs, while the large cases expose scaling.
- **Parse/validation versus serialization:** overall LOAD is 655.61%, EXPORT is 77.29%, and ROUNDTRIP is 508.72% on summed medians. LOAD includes schema validation and SDK object construction; EXPORT isolates serialization/resource writing from that parse path.
- **Tails:** the largest p95/median ratio is 2.93x for nlohmann-2.0.0 Avocado-gltf/roundtrip. 4 of 48 case/operation rows reverse delta direction between median and p95, so scheduler/filesystem outliers should not be treated as parser behavior.
- **Memory:** large-enabled median process peak rises from 380.52 MiB to 431.51 MiB (+50.99 MiB, +13.40%). Standard-only median peak rises from 40.50 MiB to 40.87 MiB (+0.37 MiB, +0.91%). This coarse metric includes untimed correctness reloads.
- **Output size:** aggregate output is six bytes smaller with nlohmann (120,831,322 versus 120,831,328 bytes): -2 bytes for Avocado glTF and -4 bytes for NodePerformanceTest GLB; all other output sizes match and every output passed semantic/resource validation.
- Extension-mode and exact-extension tables are corpus associations, not causal isolation: assets differ in size and structure as well as extensions.
- No confidence intervals or hypothesis tests were computed. These single-machine matched observations do not establish statistical significance.

## Correctness and caveats

- FetchAssets.ps1 verifies every pinned source file's byte length and SHA-256. ValidateAssets.ps1 reparses every glTF/GLB manifest and checks counts, exact extensionsUsed/extensionsRequired arrays, typed/raw partition, immutable URLs, and licenses before running.
- After every timed LOAD, resource counts, complete buffer lengths, exact extension sets, and typed-versus-raw representation are checked outside the interval.
- After every timed EXPORT/ROUNDTRIP, the output is reparsed with the same typed handlers and compared with the source Document, required-extension sets, every buffer byte, and every encoded image byte.
- Output files are SHA-256 hashed only after the timer stops; every output was deterministic across its configured 100- or 30-sample series.
- Results are single-machine, hot-cache nearest-rank medians/p95s, not confidence intervals. Filesystem cache, antivirus, thermals, and background activity can affect tails.
- Peak memory is sampled at process level and includes untimed correctness work, so it is useful only as a coarse matched comparison.
- Assets are fetched only by explicit benchmark commands into Built/Int. Normal builds, tests, and CI remain offline and network-independent with ENABLE_BENCHMARKS=OFF.

## Build and test validation

See load-export-validation.md for benchmark-disabled builds, corpus integrity/extension round-trip checks, targeted tests, and complete suites on both branches.

## Raw evidence

- load-export-rapidjson-1.9.5-raw.csv
- load-export-nlohmann-2.0.0-raw.csv
- load-export-processes.csv
- load-export-order.csv
- load-export-output-hashes.csv
- load-export-aggregates.csv
- load-export-environment.json
- load-export-summary.json
- load-export-validation.md
