# End-to-end glTF/GLB load-export benchmark

Generated: 2026-09-03T13:40:24.5908660Z
Baseline: rapidjson-1.9.5 on perf/Release-1.9.5-load-export at 743b7ecd749a53e8d848db56839752a65205e915
Candidate: nlohmann-2.0.0 on Release/2.0.0 at dc9c9f78dc33b9df5201e28dea2eeddcb5aa7b2e
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
| ABeautifulGame-draco-glb | large/compressed-material | export | 30 | 73.380 | 73.907 | 0.526 | 0.72% | 146.758 | 145.007 | -1.19% |
| ABeautifulGame-glb | large/material-texture | export | 30 | 257.943 | 262.623 | 4.680 | 1.81% | 445.348 | 371.019 | -16.69% |
| ABeautifulGame-glb | large/material-texture | load | 30 | 76.728 | 79.528 | 2.800 | 3.65% | 95.710 | 99.886 | 4.36% |
| ABeautifulGame-draco-glb | large/compressed-material | load | 30 | 33.026 | 35.775 | 2.750 | 8.33% | 41.880 | 44.863 | 7.12% |
| ABeautifulGame-draco-glb | large/compressed-material | roundtrip | 30 | 108.154 | 110.610 | 2.456 | 2.27% | 166.076 | 134.907 | -18.77% |
| ABeautifulGame-glb | large/material-texture | roundtrip | 30 | 346.043 | 342.765 | -3.278 | -0.95% | 414.230 | 406.387 | -1.89% |
| Avocado-glb | medium/texture-heavy | export | 100 | 49.899 | 49.586 | -0.313 | -0.63% | 113.620 | 115.186 | 1.38% |
| Avocado-glb | medium/texture-heavy | load | 100 | 20.908 | 23.417 | 2.510 | 12.00% | 31.587 | 30.086 | -4.75% |
| Avocado-glb | medium/texture-heavy | roundtrip | 100 | 71.428 | 73.026 | 1.598 | 2.24% | 153.887 | 153.057 | -0.54% |
| Avocado-gltf | medium/texture-heavy | export | 100 | 5.080 | 5.157 | 0.077 | 1.52% | 6.581 | 6.738 | 2.39% |
| Avocado-gltf | medium/texture-heavy | load | 100 | 17.292 | 19.743 | 2.452 | 14.18% | 23.656 | 29.361 | 24.12% |
| Avocado-gltf | medium/texture-heavy | roundtrip | 100 | 23.306 | 25.235 | 1.930 | 8.28% | 32.035 | 32.186 | 0.47% |
| Box-glb | small/core | export | 100 | 0.581 | 0.662 | 0.081 | 13.95% | 0.758 | 0.827 | 9.09% |
| Box-glb | small/core | load | 100 | 2.848 | 5.090 | 2.242 | 78.74% | 4.613 | 8.105 | 75.70% |
| Box-glb | small/core | roundtrip | 100 | 3.435 | 5.674 | 2.239 | 65.18% | 4.790 | 8.433 | 76.06% |
| Box-gltf | small/core | export | 100 | 0.884 | 0.927 | 0.043 | 4.88% | 1.208 | 1.244 | 3.02% |
| Box-gltf | small/core | load | 100 | 3.164 | 5.450 | 2.286 | 72.24% | 4.412 | 8.326 | 88.70% |
| Box-gltf | small/core | roundtrip | 100 | 3.961 | 6.373 | 2.412 | 60.89% | 5.776 | 8.857 | 53.35% |
| EmissiveStrengthTest-glb | small/material-grid | export | 100 | 0.721 | 0.922 | 0.201 | 27.88% | 0.921 | 1.409 | 53.06% |
| EmissiveStrengthTest-glb | small/material-grid | load | 100 | 3.492 | 5.873 | 2.381 | 68.19% | 5.252 | 9.010 | 71.54% |
| EmissiveStrengthTest-glb | small/material-grid | roundtrip | 100 | 4.300 | 6.870 | 2.570 | 59.77% | 6.289 | 9.355 | 48.74% |
| IridescenceSuzanne-glb | medium/material-scene | export | 100 | 3.838 | 4.093 | 0.255 | 6.63% | 5.546 | 6.160 | 11.05% |
| IridescenceSuzanne-glb | medium/material-scene | load | 100 | 4.066 | 6.478 | 2.412 | 59.31% | 5.773 | 9.179 | 58.99% |
| IridescenceSuzanne-glb | medium/material-scene | roundtrip | 100 | 7.837 | 10.448 | 2.611 | 33.32% | 11.506 | 14.732 | 28.04% |
| MaterialsVariantsShoe-glb | medium/material-texture | export | 100 | 49.158 | 48.770 | -0.388 | -0.79% | 120.106 | 115.009 | -4.24% |
| MaterialsVariantsShoe-glb | medium/material-texture | load | 100 | 20.108 | 22.700 | 2.592 | 12.89% | 27.408 | 30.773 | 12.28% |
| MaterialsVariantsShoe-glb | medium/material-texture | roundtrip | 100 | 70.021 | 71.389 | 1.368 | 1.95% | 150.199 | 150.061 | -0.09% |
| MorphStressTest-glb | medium/geometry-animation | export | 100 | 4.487 | 4.971 | 0.484 | 10.77% | 7.068 | 7.388 | 4.53% |
| MorphStressTest-glb | medium/geometry-animation | load | 100 | 5.170 | 7.144 | 1.974 | 38.17% | 7.765 | 10.402 | 33.96% |
| MorphStressTest-glb | medium/geometry-animation | roundtrip | 100 | 9.699 | 11.917 | 2.218 | 22.87% | 13.908 | 14.408 | 3.60% |
| NodePerformanceTest-glb | large/scene-structure | export | 30 | 310.472 | 894.672 | 584.200 | 188.16% | 354.138 | 967.013 | 173.06% |
| NodePerformanceTest-glb | large/scene-structure | load | 30 | 1950.907 | 1993.404 | 42.496 | 2.18% | 2150.515 | 2132.923 | -0.82% |
| NodePerformanceTest-glb | large/scene-structure | roundtrip | 30 | 2223.146 | 2925.352 | 702.206 | 31.59% | 2359.943 | 3095.595 | 31.17% |
| SheenTestGrid-glb | medium/material-grid | export | 100 | 12.117 | 13.216 | 1.098 | 9.06% | 15.378 | 18.368 | 19.45% |
| SheenTestGrid-glb | medium/material-grid | load | 100 | 8.022 | 10.376 | 2.353 | 29.33% | 11.590 | 16.738 | 44.42% |
| SheenTestGrid-glb | medium/material-grid | roundtrip | 100 | 20.014 | 23.844 | 3.830 | 19.14% | 25.787 | 29.366 | 13.88% |
| SimpleInstancing-glb | small/scene-instancing | export | 100 | 0.665 | 0.744 | 0.078 | 11.79% | 0.941 | 0.978 | 3.89% |
| SimpleInstancing-glb | small/scene-instancing | load | 100 | 2.937 | 5.301 | 2.364 | 80.48% | 4.868 | 8.123 | 66.87% |
| SimpleInstancing-glb | small/scene-instancing | roundtrip | 100 | 3.685 | 5.890 | 2.206 | 59.87% | 5.982 | 7.600 | 27.05% |
| SpecularTest-glb | small/material-grid | export | 100 | 2.198 | 2.732 | 0.534 | 24.29% | 3.425 | 3.953 | 15.40% |
| SpecularTest-glb | small/material-grid | load | 100 | 4.554 | 7.454 | 2.899 | 63.66% | 6.361 | 11.230 | 76.53% |
| SpecularTest-glb | small/material-grid | roundtrip | 100 | 6.811 | 10.271 | 3.459 | 50.79% | 9.288 | 14.990 | 61.39% |
| TextureTransformMultiTest-glb | medium/material-grid | export | 100 | 3.282 | 4.496 | 1.214 | 36.97% | 4.376 | 6.053 | 38.34% |
| TextureTransformMultiTest-glb | medium/material-grid | load | 100 | 6.554 | 9.342 | 2.788 | 42.54% | 10.094 | 15.359 | 52.16% |
| TextureTransformMultiTest-glb | medium/material-grid | roundtrip | 100 | 9.711 | 13.815 | 4.103 | 42.25% | 14.996 | 22.226 | 48.21% |
| XmpMetadataRoundedCube-glb | small/metadata | export | 100 | 1.405 | 1.467 | 0.062 | 4.38% | 2.092 | 1.991 | -4.84% |
| XmpMetadataRoundedCube-glb | small/metadata | load | 100 | 2.994 | 5.360 | 2.366 | 79.00% | 4.480 | 7.842 | 75.07% |
| XmpMetadataRoundedCube-glb | small/metadata | roundtrip | 100 | 4.323 | 6.760 | 2.438 | 56.39% | 6.000 | 10.800 | 80.01% |

## Aggregate views

Summed medians weight cases by observed runtime. The geometric delta is also weighted by each case's RapidJSON median, so tiny assets cannot dominate the aggregate. These are descriptive summaries, not inferential statistics.

### Overall

| Group | Operation | Cases | RapidJSON summed median (ms) | nlohmann summed median (ms) | Delta (ms) | Aggregate delta | Baseline-time-weighted geometric delta | Aggregate p95 delta |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| all-cases | export | 16 | 776.112 | 1368.944 | 592.832 | 76.38% | 54.34% | 43.97% |
| all-cases | load | 16 | 2162.771 | 2242.435 | 79.664 | 3.68% | 3.45% | 1.49% |
| all-cases | roundtrip | 16 | 2915.873 | 3650.240 | 734.367 | 25.19% | 24.43% | 21.66% |

### Complexity class

| Group | Operation | Cases | RapidJSON summed median (ms) | nlohmann summed median (ms) | Delta (ms) | Aggregate delta | Baseline-time-weighted geometric delta | Aggregate p95 delta |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| large | export | 3 | 641.795 | 1231.202 | 589.406 | 91.84% | 68.21% | 56.73% |
| large | load | 3 | 2060.661 | 2108.706 | 48.046 | 2.33% | 2.33% | -0.46% |
| large | roundtrip | 3 | 2677.343 | 3378.727 | 701.384 | 26.20% | 25.56% | 23.69% |
| medium | export | 7 | 127.862 | 130.288 | 2.426 | 1.90% | 1.71% | 0.82% |
| medium | load | 7 | 82.121 | 99.200 | 17.080 | 20.80% | 20.12% | 20.38% |
| medium | roundtrip | 7 | 212.015 | 229.674 | 17.659 | 8.33% | 7.82% | 3.41% |
| small | export | 6 | 6.455 | 7.454 | 0.999 | 15.48% | 15.10% | 11.31% |
| small | load | 6 | 19.990 | 34.528 | 14.538 | 72.73% | 72.60% | 75.53% |
| small | roundtrip | 6 | 26.515 | 41.839 | 15.324 | 57.79% | 57.72% | 57.47% |

### Detailed complexity tier

| Group | Operation | Cases | RapidJSON summed median (ms) | nlohmann summed median (ms) | Delta (ms) | Aggregate delta | Baseline-time-weighted geometric delta | Aggregate p95 delta |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| large/compressed-material | export | 1 | 73.380 | 73.907 | 0.526 | 0.72% | 0.72% | -1.19% |
| large/compressed-material | load | 1 | 33.026 | 35.775 | 2.750 | 8.33% | 8.33% | 7.12% |
| large/compressed-material | roundtrip | 1 | 108.154 | 110.610 | 2.456 | 2.27% | 2.27% | -18.77% |
| large/material-texture | export | 1 | 257.943 | 262.623 | 4.680 | 1.81% | 1.81% | -16.69% |
| large/material-texture | load | 1 | 76.728 | 79.528 | 2.800 | 3.65% | 3.65% | 4.36% |
| large/material-texture | roundtrip | 1 | 346.043 | 342.765 | -3.278 | -0.95% | -0.95% | -1.89% |
| large/scene-structure | export | 1 | 310.472 | 894.672 | 584.200 | 188.16% | 188.16% | 173.06% |
| large/scene-structure | load | 1 | 1950.907 | 1993.404 | 42.496 | 2.18% | 2.18% | -0.82% |
| large/scene-structure | roundtrip | 1 | 2223.146 | 2925.352 | 702.206 | 31.59% | 31.59% | 31.17% |
| medium/geometry-animation | export | 1 | 4.487 | 4.971 | 0.484 | 10.77% | 10.77% | 4.53% |
| medium/geometry-animation | load | 1 | 5.170 | 7.144 | 1.974 | 38.17% | 38.17% | 33.96% |
| medium/geometry-animation | roundtrip | 1 | 9.699 | 11.917 | 2.218 | 22.87% | 22.87% | 3.60% |
| medium/material-grid | export | 2 | 15.399 | 17.711 | 2.312 | 15.01% | 14.49% | 23.63% |
| medium/material-grid | load | 2 | 14.576 | 19.717 | 5.141 | 35.27% | 35.11% | 48.02% |
| medium/material-grid | roundtrip | 2 | 29.725 | 37.659 | 7.934 | 26.69% | 26.24% | 26.50% |
| medium/material-scene | export | 1 | 3.838 | 4.093 | 0.255 | 6.63% | 6.63% | 11.05% |
| medium/material-scene | load | 1 | 4.066 | 6.478 | 2.412 | 59.31% | 59.31% | 58.99% |
| medium/material-scene | roundtrip | 1 | 7.837 | 10.448 | 2.611 | 33.32% | 33.32% | 28.04% |
| medium/material-texture | export | 1 | 49.158 | 48.770 | -0.388 | -0.79% | -0.79% | -4.24% |
| medium/material-texture | load | 1 | 20.108 | 22.700 | 2.592 | 12.89% | 12.89% | 12.28% |
| medium/material-texture | roundtrip | 1 | 70.021 | 71.389 | 1.368 | 1.95% | 1.95% | -0.09% |
| medium/texture-heavy | export | 2 | 54.979 | 54.743 | -0.236 | -0.43% | -0.43% | 1.43% |
| medium/texture-heavy | load | 2 | 38.199 | 43.161 | 4.961 | 12.99% | 12.98% | 7.61% |
| medium/texture-heavy | roundtrip | 2 | 94.733 | 98.261 | 3.528 | 3.72% | 3.69% | -0.37% |
| small/core | export | 2 | 1.465 | 1.589 | 0.124 | 8.48% | 8.39% | 5.36% |
| small/core | load | 2 | 6.012 | 10.540 | 4.528 | 75.32% | 75.29% | 82.06% |
| small/core | roundtrip | 2 | 7.397 | 12.048 | 4.651 | 62.88% | 62.87% | 63.64% |
| small/material-grid | export | 2 | 2.919 | 3.655 | 0.735 | 25.18% | 25.17% | 23.38% |
| small/material-grid | load | 2 | 8.046 | 13.327 | 5.280 | 65.63% | 65.61% | 74.28% |
| small/material-grid | roundtrip | 2 | 11.112 | 17.141 | 6.030 | 54.26% | 54.20% | 56.28% |
| small/metadata | export | 1 | 1.405 | 1.467 | 0.062 | 4.38% | 4.38% | -4.84% |
| small/metadata | load | 1 | 2.994 | 5.360 | 2.366 | 79.00% | 79.00% | 75.07% |
| small/metadata | roundtrip | 1 | 4.323 | 6.760 | 2.438 | 56.39% | 56.39% | 80.01% |
| small/scene-instancing | export | 1 | 0.665 | 0.744 | 0.078 | 11.79% | 11.79% | 3.89% |
| small/scene-instancing | load | 1 | 2.937 | 5.301 | 2.364 | 80.48% | 80.48% | 66.87% |
| small/scene-instancing | roundtrip | 1 | 3.685 | 5.890 | 2.206 | 59.87% | 59.87% | 27.05% |

### Extension execution mode

| Group | Operation | Cases | RapidJSON summed median (ms) | nlohmann summed median (ms) | Delta (ms) | Aggregate delta | Baseline-time-weighted geometric delta | Aggregate p95 delta |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| core-only | export | 5 | 60.931 | 61.303 | 0.372 | 0.61% | 0.56% | 1.66% |
| core-only | load | 5 | 49.382 | 60.845 | 11.463 | 23.21% | 21.73% | 19.78% |
| core-only | roundtrip | 5 | 111.829 | 122.226 | 10.397 | 9.30% | 8.42% | 3.11% |
| mixed-typed-raw | export | 2 | 77.219 | 77.999 | 0.781 | 1.01% | 1.00% | -0.75% |
| mixed-typed-raw | load | 2 | 37.092 | 42.253 | 5.161 | 13.91% | 13.00% | 13.41% |
| mixed-typed-raw | roundtrip | 2 | 115.991 | 121.058 | 5.067 | 4.37% | 4.12% | -15.74% |
| raw-only | export | 4 | 361.757 | 945.831 | 584.075 | 161.46% | 147.91% | 127.43% |
| raw-only | load | 4 | 1977.502 | 2027.337 | 49.835 | 2.52% | 2.46% | -0.32% |
| raw-only | roundtrip | 4 | 2301.790 | 3010.371 | 708.581 | 30.78% | 30.66% | 29.47% |
| typed-only | export | 5 | 276.206 | 283.810 | 7.604 | 2.75% | 2.67% | -14.72% |
| typed-only | load | 5 | 98.796 | 112.000 | 13.204 | 13.37% | 11.91% | 17.66% |
| typed-only | roundtrip | 5 | 386.264 | 396.585 | 10.321 | 2.67% | 2.14% | 2.19% |

### Exact extension coverage

| Group | Operation | Cases | RapidJSON summed median (ms) | nlohmann summed median (ms) | Delta (ms) | Aggregate delta | Baseline-time-weighted geometric delta | Aggregate p95 delta |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| EXT_mesh_gpu_instancing | export | 1 | 0.665 | 0.744 | 0.078 | 11.79% | 11.79% | 3.89% |
| EXT_mesh_gpu_instancing | load | 1 | 2.937 | 5.301 | 2.364 | 80.48% | 80.48% | 66.87% |
| EXT_mesh_gpu_instancing | roundtrip | 1 | 3.685 | 5.890 | 2.206 | 59.87% | 59.87% | 27.05% |
| KHR_draco_mesh_compression | export | 1 | 73.380 | 73.907 | 0.526 | 0.72% | 0.72% | -1.19% |
| KHR_draco_mesh_compression | load | 1 | 33.026 | 35.775 | 2.750 | 8.33% | 8.33% | 7.12% |
| KHR_draco_mesh_compression | roundtrip | 1 | 108.154 | 110.610 | 2.456 | 2.27% | 2.27% | -18.77% |
| KHR_lights_punctual | export | 2 | 314.310 | 898.765 | 584.455 | 185.95% | 184.69% | 170.56% |
| KHR_lights_punctual | load | 2 | 1954.974 | 1999.882 | 44.908 | 2.30% | 2.27% | -0.66% |
| KHR_lights_punctual | roundtrip | 2 | 2230.983 | 2935.800 | 704.817 | 31.59% | 31.59% | 31.16% |
| KHR_materials_clearcoat | export | 1 | 3.282 | 4.496 | 1.214 | 36.97% | 36.97% | 38.34% |
| KHR_materials_clearcoat | load | 1 | 6.554 | 9.342 | 2.788 | 42.54% | 42.54% | 52.16% |
| KHR_materials_clearcoat | roundtrip | 1 | 9.711 | 13.815 | 4.103 | 42.25% | 42.25% | 48.21% |
| KHR_materials_emissive_strength | export | 1 | 0.721 | 0.922 | 0.201 | 27.88% | 27.88% | 53.06% |
| KHR_materials_emissive_strength | load | 1 | 3.492 | 5.873 | 2.381 | 68.19% | 68.19% | 71.54% |
| KHR_materials_emissive_strength | roundtrip | 1 | 4.300 | 6.870 | 2.570 | 59.77% | 59.77% | 48.74% |
| KHR_materials_ior | export | 1 | 3.838 | 4.093 | 0.255 | 6.63% | 6.63% | 11.05% |
| KHR_materials_ior | load | 1 | 4.066 | 6.478 | 2.412 | 59.31% | 59.31% | 58.99% |
| KHR_materials_ior | roundtrip | 1 | 7.837 | 10.448 | 2.611 | 33.32% | 33.32% | 28.04% |
| KHR_materials_iridescence | export | 1 | 3.838 | 4.093 | 0.255 | 6.63% | 6.63% | 11.05% |
| KHR_materials_iridescence | load | 1 | 4.066 | 6.478 | 2.412 | 59.31% | 59.31% | 58.99% |
| KHR_materials_iridescence | roundtrip | 1 | 7.837 | 10.448 | 2.611 | 33.32% | 33.32% | 28.04% |
| KHR_materials_sheen | export | 1 | 12.117 | 13.216 | 1.098 | 9.06% | 9.06% | 19.45% |
| KHR_materials_sheen | load | 1 | 8.022 | 10.376 | 2.353 | 29.33% | 29.33% | 44.42% |
| KHR_materials_sheen | roundtrip | 1 | 20.014 | 23.844 | 3.830 | 19.14% | 19.14% | 13.88% |
| KHR_materials_specular | export | 1 | 2.198 | 2.732 | 0.534 | 24.29% | 24.29% | 15.40% |
| KHR_materials_specular | load | 1 | 4.554 | 7.454 | 2.899 | 63.66% | 63.66% | 76.53% |
| KHR_materials_specular | roundtrip | 1 | 6.811 | 10.271 | 3.459 | 50.79% | 50.79% | 61.39% |
| KHR_materials_transmission | export | 3 | 335.161 | 340.622 | 5.461 | 1.63% | 1.63% | -12.63% |
| KHR_materials_transmission | load | 3 | 113.820 | 121.781 | 7.961 | 6.99% | 6.61% | 7.37% |
| KHR_materials_transmission | roundtrip | 3 | 462.033 | 463.823 | 1.790 | 0.39% | 0.30% | -6.05% |
| KHR_materials_unlit | export | 1 | 3.282 | 4.496 | 1.214 | 36.97% | 36.97% | 38.34% |
| KHR_materials_unlit | load | 1 | 6.554 | 9.342 | 2.788 | 42.54% | 42.54% | 52.16% |
| KHR_materials_unlit | roundtrip | 1 | 9.711 | 13.815 | 4.103 | 42.25% | 42.25% | 48.21% |
| KHR_materials_variants | export | 1 | 49.158 | 48.770 | -0.388 | -0.79% | -0.79% | -4.24% |
| KHR_materials_variants | load | 1 | 20.108 | 22.700 | 2.592 | 12.89% | 12.89% | 12.28% |
| KHR_materials_variants | roundtrip | 1 | 70.021 | 71.389 | 1.368 | 1.95% | 1.95% | -0.09% |
| KHR_materials_volume | export | 3 | 335.161 | 340.622 | 5.461 | 1.63% | 1.63% | -12.63% |
| KHR_materials_volume | load | 3 | 113.820 | 121.781 | 7.961 | 6.99% | 6.61% | 7.37% |
| KHR_materials_volume | roundtrip | 3 | 462.033 | 463.823 | 1.790 | 0.39% | 0.30% | -6.05% |
| KHR_texture_basisu | export | 1 | 73.380 | 73.907 | 0.526 | 0.72% | 0.72% | -1.19% |
| KHR_texture_basisu | load | 1 | 33.026 | 35.775 | 2.750 | 8.33% | 8.33% | 7.12% |
| KHR_texture_basisu | roundtrip | 1 | 108.154 | 110.610 | 2.456 | 2.27% | 2.27% | -18.77% |
| KHR_texture_transform | export | 1 | 3.282 | 4.496 | 1.214 | 36.97% | 36.97% | 38.34% |
| KHR_texture_transform | load | 1 | 6.554 | 9.342 | 2.788 | 42.54% | 42.54% | 52.16% |
| KHR_texture_transform | roundtrip | 1 | 9.711 | 13.815 | 4.103 | 42.25% | 42.25% | 48.21% |
| KHR_xmp_json_ld | export | 1 | 1.405 | 1.467 | 0.062 | 4.38% | 4.38% | -4.84% |
| KHR_xmp_json_ld | load | 1 | 2.994 | 5.360 | 2.366 | 79.00% | 79.00% | 75.07% |
| KHR_xmp_json_ld | roundtrip | 1 | 4.323 | 6.760 | 2.438 | 56.39% | 56.39% | 80.01% |

## Input and output size

| Asset | Format | Input bytes | RapidJSON output bytes | nlohmann output bytes | Delta bytes | Delta |
| --- | --- | ---: | ---: | ---: | ---: | ---: |
| Box | gltf | 3546 | 1618 | 1618 | 0 | 0.00% |
| Box | glb | 1664 | 1632 | 1632 | 0 | 0.00% |
| Avocado | gltf | 8110895 | 8109906 | 8109904 | -2 | 0.00% |
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
| NodePerformanceTest | glb | 37986536 | 38031536 | 38031532 | -4 | 0.00% |

Per-output canonical SHA-256 values are in load-export-output-hashes.csv. Different byte hashes are permitted only when both SDK reloads produce the same source Document, buffer bytes, and encoded image bytes.
Across all cases, RapidJSON output sets total 120831328 bytes and nlohmann output sets total 120831322 bytes.

## Peak working set

| Implementation | Cohort | Measured processes | Median process peak (MiB) | Maximum process peak (MiB) |
| --- | --- | ---: | ---: | ---: |
| rapidjson-1.9.5 | all | 100 | 40.54 | 401.34 |
| rapidjson-1.9.5 | large-enabled | 30 | 380.63 | 401.34 |
| rapidjson-1.9.5 | standard-only | 70 | 40.49 | 41.38 |
| nlohmann-2.0.0 | all | 100 | 40.90 | 433.51 |
| nlohmann-2.0.0 | large-enabled | 30 | 431.45 | 433.51 |
| nlohmann-2.0.0 | standard-only | 70 | 40.85 | 41.00 |

## Interpretation

- **Fixed overhead versus scaling:** small-class LOAD summed medians change by 14.538 ms (72.73%); large-class LOAD changes by 48.046 ms (2.33%). Read percentage and absolute changes together: tiny manifests magnify fixed setup/validation costs, while the large cases expose scaling.
- **Parse/validation versus serialization:** overall LOAD is 3.68%, EXPORT is 76.38%, and ROUNDTRIP is 25.19% on summed medians. LOAD includes schema validation and SDK object construction; EXPORT isolates serialization/resource writing from that parse path.
- **Tails:** the largest p95/median ratio is 2.44x for rapidjson-1.9.5 MaterialsVariantsShoe-glb/export. 9 of 48 case/operation rows reverse delta direction between median and p95, so scheduler/filesystem outliers should not be treated as parser behavior.
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
