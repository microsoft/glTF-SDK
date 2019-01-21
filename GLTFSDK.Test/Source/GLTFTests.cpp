// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/Constants.h>
#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/ExtensionsKHR.h>
#include <GLTFSDK/GLBResourceReader.h>
#include <GLTFSDK/GLBResourceWriter.h>
#include <GLTFSDK/MeshPrimitiveUtils.h>
#include <GLTFSDK/Serialize.h>

#include "TestResources.h"
#include "TestUtils.h"

#include <locale>
#include <string>
#include <memory>
#include <mutex>
#include <string>

using namespace glTF::UnitTest;

namespace
{
    using namespace Microsoft::glTF;
    using namespace Microsoft::glTF::Test;

    Document ImportAndParseGLB(std::shared_ptr<IStreamReader> streamReader, const std::shared_ptr<std::istream>& glbStream)
    {
        GLBResourceReader resourceReader(streamReader, glbStream);
        auto json = resourceReader.GetJson();
        auto doc = Deserialize(json);
        return doc;
    }

    Document ImportAndParseGLTF(std::shared_ptr<IStreamReader> streamReader, const std::shared_ptr<std::istream>& stream)
    {
        GLTFResourceReader resourceReader(streamReader);
        auto json = std::string(std::istreambuf_iterator<char>(*stream), std::istreambuf_iterator<char>());
        auto doc = Deserialize(json);
        return doc;
    }

    void TestBufferViewOffsetAlignment(const char* data,
        size_t indicesBufferViewLength,
        size_t positionsBufferViewOffset,
        size_t positionsBufferViewLength,
        size_t normalsBufferViewOffset,
        size_t normalsBufferViewLength)
    {
        auto input = std::make_shared<std::stringstream>(data);
        auto streamWriter = std::make_shared<StreamReaderWriter>();

        auto doc = ImportAndParseGLTF(streamWriter, input);

        // We're only checking the offsets for mesh 0 for the purpose of this test. Feel free to add support
        // for multiple meshes if necessary.
        auto mesh = doc.meshes[0];
        auto primitive = mesh.primitives[0];

        auto indicesAccessor = doc.accessors.Get(primitive.indicesAccessorId);
        auto positionsAccessor = doc.accessors.Get(primitive.attributes.at(ACCESSOR_POSITION));
        auto normalsAccessor = doc.accessors.Get(primitive.attributes.at(ACCESSOR_NORMAL));

        auto indicesBufferView = doc.bufferViews.Get(indicesAccessor.bufferViewId);
        auto positionsBufferView = doc.bufferViews.Get(positionsAccessor.bufferViewId);
        auto normalsBufferView = doc.bufferViews.Get(normalsAccessor.bufferViewId);

        Assert::AreEqual(indicesBufferView.byteOffset, size_t(0));
        Assert::AreEqual(indicesBufferView.byteLength, indicesBufferViewLength);
        Assert::AreEqual(positionsBufferView.byteOffset, positionsBufferViewOffset);
        Assert::AreEqual(positionsBufferView.byteLength, positionsBufferViewLength);
        Assert::AreEqual(normalsBufferView.byteOffset, normalsBufferViewOffset);
        Assert::AreEqual(normalsBufferView.byteLength, normalsBufferViewLength);
    }

    void TestGLTFRoundTrip(const std::string& dataStr)
    {
        // Deserialize JSON string -> Document
        auto originalDoc = Deserialize(dataStr);

        // Serialize Document -> JSON string
        auto reserializedJson = Serialize(originalDoc);

        // Deserialize JSON string -> Document
        auto roundtrippedDoc = Deserialize(reserializedJson);

        // Compare input and output Documents
        Assert::IsTrue(originalDoc == roundtrippedDoc, L"Input gltf and output gltf are not equal");
    }

    Document TestDeserializeValidGLTFFile(const char* resourcePath)
    {
        auto input = ReadLocalAsset(resourcePath);
        auto readwriter = std::make_shared<StreamReaderWriter>();
        auto doc = ImportAndParseGLTF(readwriter, input);
        Validation::Validate(doc);
        return doc;
    }

    void TestDeserializeInvalidGLTF(std::shared_ptr<std::istream> input)
    {
        Assert::ExpectException<GLTFException>([&input]
        {
            auto readwriter = std::make_shared<StreamReaderWriter>();
            ImportAndParseGLTF(readwriter, input);
        }, L"Expected exception was not thrown");
    }

    void TestDeserializeInvalidGLTF(const char* data)
    {
        auto stream = std::make_shared<std::stringstream>(data);
        TestDeserializeInvalidGLTF(stream);
    }

    void TestDeserializeInvalidGLBFile(const char* resourcePath)
    {
        Assert::ExpectException<GLTFException>([&resourcePath]
        {
            auto input = ReadLocalAsset(resourcePath);
            auto readwriter = std::make_shared<StreamReaderWriter>();
            ImportAndParseGLB(readwriter, input);
        }, L"Expected exception was not thrown");
    }

    Document TestDeserializeValidGLBFile(const char* resourcePath)
    {
        auto input = ReadLocalAsset(resourcePath);
        auto readwriter = std::make_shared<StreamReaderWriter>();
        auto doc = ImportAndParseGLB(readwriter, input);
        Validation::Validate(doc);
        return doc;
    }

    // Manifest violates the schema as the version number has three parts
    constexpr static const char asset_invalid_version[] = R"(
{
    "asset": {
        "version": "2.0.0",
        "generator": "glTF SDK Unit Tests"
    }
})";

    // Manifest violates the schema as the children array is empty
    constexpr static const char node_invalid_children[] = R"(
{
    "asset": {
        "version": "2.0",
        "generator": "glTF SDK Unit Tests"
    },
    "nodes": [
        {
            "children": []
        }
    ]
})";
}

namespace Microsoft
{
    namespace  glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(GLTFTests)
            {
                BEGIN_TEST_CLASS_ATTRIBUTE()
                    TEST_CLASS_ATTRIBUTE(L"Priority", L"1")
                    TEST_CLASS_ATTRIBUTE(L"Category", L"Unit-Integration")
                    END_TEST_CLASS_ATTRIBUTE()

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_Deserialize_Valid)
                {
                    TestDeserializeValidGLBFile(c_glbSampleBoxInterleaved);
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_Deserialize_Valid_NoBuffer)
                {
                    TestDeserializeValidGLBFile(c_glbCubeNoBuffer);
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLTF_RoundTrip_ValidCamera)
                {
                    TestGLTFRoundTrip(ReadLocalJson(c_validCameraJson));
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLTF_RoundTrip_ValidCameraWithExtensions)
                {
                    TestGLTFRoundTrip(ReadLocalJson(c_cameraWithExtensions));
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLTF_RoundTrip_ReciprocatingSaw)
                {
                    TestGLTFRoundTrip(ReadLocalJson(c_reciprocatingSaw));
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLTF_CameraMissingProperty)
                {
                    TestDeserializeInvalidGLTF(ReadLocalJson(c_cameraMissingProperties).c_str());
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLTF_CameraInvalidPerspective)
                {
                    TestDeserializeInvalidGLTF(ReadLocalJson(c_cameraInvalidPerspective).c_str());
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLTF_CameraInvalidProjection)
                {
                    TestDeserializeInvalidGLTF(ReadLocalJson(c_cameraInvalidProjectionJson).c_str());
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_RoundTrip_SingleTriangle)
                {
                    TestGLTFRoundTrip(ReadLocalJson(c_singleTriangleJson));
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_RoundTrip_Cube)
                {
                    TestGLTFRoundTrip(ReadLocalJson(c_cubeJson));
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_RoundTrip_CubeWithLOD)
                {
                    TestGLTFRoundTrip(ReadLocalJson(c_cubeWithLODJson));
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_RoundTrip_MultipleMeshes)
                {
                    TestGLTFRoundTrip(ReadLocalJson(c_doubleTriangleJson));
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_RoundTrip_SingleTriangleTextured)
                {
                    TestGLTFRoundTrip(ReadLocalJson(c_singleTriangleWithTextureJson));
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_RoundTrip_SinglePolygonNormals)
                {
                    TestGLTFRoundTrip(ReadLocalJson(c_singlePolyWithNormalsJson));
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_RoundTrip_MultipleNodes)
                {
                    TestGLTFRoundTrip(ReadLocalJson(c_doubleNodesJson));
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_RoundTrip_Transforms)
                {
                    TestGLTFRoundTrip(ReadLocalJson(c_transformsJson));
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_RoundTrip_ComplexTexture)
                {
                    TestGLTFRoundTrip(ReadLocalJson(c_cartoonCurse01FbxJson));
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_RoundTrip_AnimatedTriangle)
                {
                    // Node animation test
                    TestGLTFRoundTrip(ReadLocalJson(c_animatedTriangleJson));
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_RoundTrip_SimpleSkin)
                {
                    // Skinned animation test
                    TestGLTFRoundTrip(ReadLocalJson(c_riggedSimpleJson));
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_RoundTrip_TriangleWithoutIndices)
                {
                    TestGLTFRoundTrip(ReadLocalJson(c_validTriangleWithoutIndices));
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_RoundTrip_TriangleTRS)
                {
                    TestGLTFRoundTrip(ReadLocalJson(c_validTriangleTRS));
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_RoundTrip_TriangleMatrix)
                {
                    TestGLTFRoundTrip(ReadLocalJson(c_validTriangleMatrix));
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_RoundTrip_MorphTarget)
                {
                    TestGLTFRoundTrip(ReadLocalJson(c_validMorphTarget));
                }

                // following test cases are only checked while deserializing to GLTF
                GLTFSDK_TEST_METHOD(GLTFTests, GLB_MissingDefaultSceneReference)
                {
                    TestDeserializeInvalidGLTF(ReadLocalJson(c_missingDefaultSceneJson).c_str());
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_MissingMeshReference)
                {
                    TestDeserializeInvalidGLTF(ReadLocalJson(c_missingMeshRefJson).c_str());
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_MissingNodeReference)
                {
                    TestDeserializeInvalidGLTF(ReadLocalJson(c_missingNodeRefJson).c_str());
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_CircularNodeReference)
                {
                    TestDeserializeInvalidGLTF(ReadLocalJson(c_duplicateNodesJson).c_str());
                }

                // following test cases are stored in files because right now as material and textures create fairly complex files
                GLTFSDK_TEST_METHOD(GLTFTests, GLB_MissingMaterialReference)
                {
                    TestDeserializeInvalidGLBFile(c_glbDuckMissingMaterialRef);
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_MissingTextureReference)
                {
                    TestDeserializeInvalidGLBFile(c_glbCubeMissingTextureRef);
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_InvalidAccessorByteLength)
                {
                    TestDeserializeInvalidGLBFile(c_glbCubeInvalidAccessorByteLength);
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_InvalidAccessorByteOffset)
                {
                    TestDeserializeInvalidGLBFile(c_glbCubeInvalidAccessorByteOffset);
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_InvalidBufferViewLength)
                {
                    TestDeserializeInvalidGLBFile(c_glbCubeInvalidBufferViewLength);
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_AccessorMinMax)
                {
                    std::string assetName = "test";
                    std::string filename = assetName + "." + GLB_EXTENSION;
                    auto input = std::make_shared<std::stringstream>(ReadLocalJson(c_cubeJson).c_str());
                    auto readwriter = std::make_shared<StreamReaderWriter>();

                    auto doc = ImportAndParseGLTF(readwriter, input);

                    auto mesh = doc.meshes[0];
                    auto primitive = mesh.primitives[0];
                    auto indicesAccessor = doc.accessors.Get(primitive.indicesAccessorId);
                    auto positionsAccessor = doc.accessors.Get(primitive.attributes.at(ACCESSOR_POSITION));
                    auto normalsAccessor = doc.accessors.Get(primitive.attributes.at(ACCESSOR_NORMAL));

                    Assert::AreEqual(0.f, indicesAccessor.min[0]);
                    Assert::AreEqual(23.f, indicesAccessor.max[0]);

                    // use IsTrue because AreEqual doesn't support comparing vectors
                    Assert::IsTrue(std::vector<float> {0.f, 0.f, 0.f} == positionsAccessor.min);
                    Assert::IsTrue(std::vector<float> {1.f, 1.f, 1.f} == positionsAccessor.max);

                    Assert::IsTrue(std::vector<float> {-1.f, -1.f, -1.f } == normalsAccessor.min);
                    Assert::IsTrue(std::vector<float> {1.f, 1.f, 0.f} == normalsAccessor.max);
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_OffsetAlignment_SingleTriangle)
                {
                    //              offset  length
                    //indices       0       6
                    //positions     8       36
                    //normals       44      36
                    TestBufferViewOffsetAlignment(ReadLocalJson(c_singleTriangleJson).c_str(), 6, 6, 38, 44, 36);
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_OffsetAlignment_TwoTriangles)
                {
                    //              offset  length
                    //indices_1     0       6
                    //indices_2     6       6
                    //positions_1   12      36
                    //positions_2   48      36
                    //normals_1     84      36
                    //normals_2     120     36

                    // Only testing mesh1 which is sufficient for the purpose of this test.
                    // Besides, if the offsets in mesh2 are wrong, some of the offsets in mesh1 would also be wrong.
                    TestBufferViewOffsetAlignment(ReadLocalJson(c_doubleTriangleJson).c_str(), 12, 12, 72, 84, 72);
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_WrongReportedLength)
                {
                    TestDeserializeInvalidGLBFile(c_glbWrongReportedLength);
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_WrongJsonLength)
                {
                    TestDeserializeInvalidGLBFile(c_glbWrongJsonLength);
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_WrongBinHeaderLength)
                {
                    TestDeserializeInvalidGLBFile(c_glbWrongBinHeaderLength);
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLB_TextureComparison)
                {
                    Material::OcclusionTextureInfo occ1;
                    occ1.textureId = "foo1";

                    Material::OcclusionTextureInfo occ2;
                    occ2.textureId = "foo2";

                    Assert::IsFalse(occ1 == occ2);
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLTF_TestNoDefaultScene)
                {
                    // Verify that trying to access the default scene in a document
                    // which has no scenes, throws the expected exception.
                    auto GetScene = [] { Document().GetDefaultScene(); };
                    Assert::ExpectException<DocumentException>( GetScene, L"Expected DocumentException was not thrown" );
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLTF_DeserializeTexCoord1)
                {
                    auto doc = TestDeserializeValidGLTFFile(c_meshPrimitivesUV04);

                    // Check for expected values in document
                    Assert::AreEqual<size_t>(doc.materials[0].metallicRoughness.baseColorTexture.texCoord, 1);
                    Assert::AreEqual<size_t>(doc.materials[0].normalTexture.texCoord, 1);

                    Assert::AreEqual<std::string>(doc.meshes[0].primitives[0].attributes.at(ACCESSOR_TEXCOORD_1), "5");
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLTF_DeserializeExtensionsUsed)
                {
                    auto doc = TestDeserializeValidGLTFFile(c_cubeJson);

                    // Check for expected values in document
                    Assert::IsTrue(doc.IsExtensionUsed(KHR::Materials::PBRSPECULARGLOSSINESS_NAME));
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLTF_RoundTripTexCoord1)
                {
                    TestGLTFRoundTrip(ReadLocalJson(c_meshPrimitivesUV04));
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLTF_DeserializeNodeAnimation)
                {
                    auto doc = TestDeserializeValidGLTFFile(c_animatedTriangleJson);

                    // Check for expected values in document
                    Assert::AreEqual<std::string>(doc.animations.Get("0").channels[0].samplerId, "0");
                    Assert::AreEqual<std::string>(doc.animations.Get("0").channels[0].target.nodeId, "0");
                    Assert::IsTrue(doc.animations.Get("0").channels[0].target.path == TARGET_ROTATION);

                    Assert::AreEqual<std::string>(doc.animations.Get("0").samplers[0].inputAccessorId, "2");
                    Assert::IsTrue(doc.animations.Get("0").samplers[0].interpolation == INTERPOLATION_LINEAR);
                    Assert::AreEqual<std::string>(doc.animations.Get("0").samplers[0].outputAccessorId, "3");
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLTF_DeserializeSkinnedAnimation)
                {
                    auto doc = TestDeserializeValidGLTFFile(c_riggedSimpleJson);

                    // Check for expected values in document
                    Assert::AreEqual<std::string>(doc.skins.Get("0").inverseBindMatricesAccessorId, "13");
                    Assert::AreEqual<std::string>(doc.skins.Get("0").skeletonId, "2");
                    Assert::IsTrue(doc.skins.Get("0").jointIds.size() == 2);
                    Assert::AreEqual<std::string>(doc.skins.Get("0").jointIds[0], "2");
                    Assert::AreEqual<std::string>(doc.skins.Get("0").jointIds[1], "3");


                    Assert::AreEqual<std::string>(doc.animations.Get("0").channels[0].samplerId, "0");
                    Assert::AreEqual<std::string>(doc.animations.Get("0").channels[0].target.nodeId, "2");
                    Assert::IsTrue(doc.animations.Get("0").channels[0].target.path == TARGET_TRANSLATION);

                    Assert::AreEqual<std::string>(doc.animations.Get("1").channels[2].samplerId, "2");
                    Assert::AreEqual<std::string>(doc.animations.Get("1").channels[2].target.nodeId, "3");
                    Assert::IsTrue(doc.animations.Get("1").channels[2].target.path == TARGET_SCALE);

                    Assert::AreEqual<std::string>(doc.animations.Get("1").samplers[1].inputAccessorId, "9");
                    Assert::IsTrue(doc.animations.Get("0").samplers[0].interpolation == INTERPOLATION_LINEAR);
                    Assert::AreEqual<std::string>(doc.animations.Get("1").samplers[1].outputAccessorId, "11");

                    Assert::AreEqual<std::string>(doc.meshes.Get("0").primitives[0].attributes.at(ACCESSOR_JOINTS_0), "1");
                    Assert::AreEqual<std::string>(doc.meshes.Get("0").primitives[0].attributes.at(ACCESSOR_WEIGHTS_0), "4");
                }

                GLTFSDK_TEST_METHOD(GLTFTests, GLTF_Deserialize_Positions_Vec3_Float_Interleaved)
                {
                    auto input = ReadLocalAsset(c_glbSampleBoxInterleaved);
                    auto readwriter = std::make_shared<StreamReaderWriter>();

                    GLBResourceReader resourceReader(readwriter, input);
                    auto json = resourceReader.GetJson();

                    auto doc = Deserialize(json);
                    auto output = MeshPrimitiveUtils::GetPositions(doc, resourceReader, doc.accessors.Get("2"));

                    std::vector<float> expected = {
                        -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,
                        0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,
                        0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f,  0.5f,  0.5f,
                        0.5f, -0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,
                        -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f,
                        0.5f,  0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,
                        -0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
                        -0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,
                    };

                    AreEqual(expected, output);
                }

                GLTFSDK_TEST_METHOD(GLTFTests, SerializeSparseAccessorRoundTrip)
                {
                    TestGLTFRoundTrip(ReadLocalJson(c_simpleSparseAccessor));
                }
                
                GLTFSDK_TEST_METHOD(GLTFTests, Verify_Extensions_In_ExtensionsUsed)
                {
                    // Add an extension to extensions and add it to extensionsUsed
                    Document doc;
                    doc.extensions.emplace("MyExtension", "{}");
                    doc.extensionsUsed.emplace("MyExtension");
                    auto reserializedJson = Serialize(doc);

                    // Add an extension to extensions without adding it to extensionsUsed
                    Assert::ExpectException<GLTFException>(
                        []()
                    {
                        Document doc;
                        doc.extensions.emplace("MyExtension", "{}");
                        auto reserializedJson = Serialize(doc);
                    }, L"missing extensionsUsed value should have thrown an exception.");
                }

                GLTFSDK_TEST_METHOD(GLTFTests, Verify_ExtensionsRequired_In_ExtensionsUsed)
                {
                    // Add an extension to extensionsRequired and add it to extensionsUsed
                    Document doc;
                    doc.extensions.emplace("MyExtension", "{}");
                    doc.extensionsUsed.emplace("MyExtension");
                    doc.extensionsRequired.emplace("MyExtension");
                    auto reserializedJson = Serialize(doc);

                    // Add an extension to extensionsRequired without adding it to extensionsUsed
                    Assert::ExpectException<GLTFException>(
                        []()
                    {
                        Document doc;
                        doc.extensions.emplace("MyExtension", "{}");
                        doc.extensionsRequired.emplace("MyExtension");
                        auto reserializedJson = Serialize(doc);
                    }, L"missing extensionsUsed value should have thrown an exception.");
                }

                GLTFSDK_TEST_METHOD(GLTFTests, Verify_MeshPrimitive_Attributes_RoundTrip)
                {
                    Document doc;

                    MeshPrimitive primitive;
                    primitive.mode = MESH_TRIANGLES;
                    primitive.attributes["EXTRA_ATTRIBUTE"] = "0";
                    primitive.attributes[ACCESSOR_POSITION] = "1";
                    Mesh mesh;
                    mesh.id = "0";
                    mesh.primitives.push_back(primitive);
                    doc.meshes.Append(mesh);

                    Accessor accessor0;
                    accessor0.id = "0";
                    accessor0.type = TYPE_SCALAR;
                    accessor0.componentType = COMPONENT_FLOAT;
                    accessor0.count = 1;
                    doc.accessors.Append(accessor0);
                    Accessor accessor1;
                    accessor1.id = "1";
                    accessor1.type = TYPE_SCALAR;
                    accessor1.componentType = COMPONENT_FLOAT;
                    accessor1.count = 1;
                    doc.accessors.Append(accessor1);

                    auto serializedJson = Serialize(doc);

                    Document doc2 = Deserialize(serializedJson);

                    Assert::AreEqual<size_t>(doc2.meshes.Size(), 1);
                    Assert::AreEqual<size_t>(doc2.meshes[0].primitives.size(), 1);
                    Assert::AreEqual<size_t>(doc2.meshes[0].primitives[0].attributes.size(), 2);
                    Assert::AreEqual<std::string>(doc2.meshes[0].primitives[0].attributes.at("EXTRA_ATTRIBUTE"), "0");
                    Assert::AreEqual<std::string>(doc2.meshes[0].primitives[0].attributes.at(ACCESSOR_POSITION), "1");

                    Assert::IsTrue(doc == doc2, L"Input gltf and output gltf are not equal");
                }

                GLTFSDK_TEST_METHOD(GLTFTests, UnicodeByteOrderMark)
                {
                    constexpr static const char assetBom[] = "\xEF\xBB\xBF";
                    constexpr static const char asset[] = R"(
{
    "asset": {
        "version": "2.0",
        "generator": "glTF SDK Unit Tests"
    }
})";

                    // Test the overload of Deserialize that accepts a string
                    {
                        std::stringstream ss;

                        ss << assetBom;
                        ss << asset;

                        auto documentWithBom = Deserialize(ss.str(), DeserializeFlags::IgnoreByteOrderMark);
                        auto documentWithoutBom = Deserialize(asset);

                        Assert::IsTrue(documentWithBom == documentWithoutBom, L"Deserialized asset with utf8 BOM doesn't match asset without utf8 BOM");
                    }

                    // Test the overload of Deserialize that accepts a stream
                    {
                        std::stringstream ss;

                        ss << assetBom;
                        ss << asset;

                        auto documentWithBom = Deserialize(ss, DeserializeFlags::IgnoreByteOrderMark); // Note that the stringstream was passed to Deserialize
                        auto documentWithoutBom = Deserialize(asset);

                        Assert::IsTrue(documentWithBom == documentWithoutBom, L"Deserialized asset with utf8 BOM doesn't match asset without utf8 BOM");
                    }

                    // Test the overload of Deserialize that accepts a string
                    Assert::ExpectException<GLTFException>([]
                    {
                        std::stringstream ss;

                        ss << assetBom;
                        ss << asset;

                        // If the IgnoreByteOrderMark flag isn't specified then a BOM should result in Deserialize throwing an exception
                        Deserialize(ss.str(), DeserializeFlags::None);
                    });

                    // Test the overload of Deserialize that accepts a stream
                    Assert::ExpectException<GLTFException>([]
                    {
                        std::stringstream ss;

                        ss << assetBom;
                        ss << asset;

                        // If the IgnoreByteOrderMark flag isn't specified then a BOM should result in Deserialize throwing an exception
                        Deserialize(ss, DeserializeFlags::None);
                    });
                }

                GLTFSDK_TEST_METHOD(GLTFTests, SchemaFlagsNone)
                {
                    Assert::ExpectException<ValidationException>([json = asset_invalid_version]()
                    {
                        Deserialize(json, DeserializeFlags::None, SchemaFlags::None);
                    });

                    Assert::ExpectException<ValidationException>([json = node_invalid_children]()
                    {
                        Deserialize(json, DeserializeFlags::None, SchemaFlags::None);
                    });
                }

                GLTFSDK_TEST_METHOD(GLTFTests, SchemaFlagsDisableSchema)
                {
                    // SchemaFlags::DisableSchemaRoot - disables all schema validation
                    auto document = Deserialize(asset_invalid_version, DeserializeFlags::None, SchemaFlags::DisableSchemaRoot);

                    Assert::AreEqual(document.asset.version.c_str(), "2.0.0"); // Assert that the invalid version string was deserialized correctly
                }

                GLTFSDK_TEST_METHOD(GLTFTests, SchemaFlagsDisableSchemaAsset)
                {
                    // SchemaFlags::DisableSchemaAsset - disables asset schema validation only
                    auto document = Deserialize(asset_invalid_version, DeserializeFlags::None, SchemaFlags::DisableSchemaAsset);

                    Assert::AreEqual(document.asset.version.c_str(), "2.0.0"); // Assert that the invalid version string was deserialized correctly
                }

                GLTFSDK_TEST_METHOD(GLTFTests, SchemaFlagsDisableSchemaNode)
                {
                    // SchemaFlags::DisableSchemaAsset - disables asset schema validation only
                    auto document = Deserialize(node_invalid_children, DeserializeFlags::None, SchemaFlags::DisableSchemaNode);

                    Assert::IsTrue(document.nodes.Size() == 1U);
                    Assert::IsTrue(document.nodes.Front().children.empty()); // Assert that the node has no children
                }
            };
        }
    }
}
