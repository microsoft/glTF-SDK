// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/Serialize.h>
#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/ExtensionHandlers.h>

#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>

using namespace glTF::UnitTest;

namespace
{
    const char* c_expectedDefaultDocument = R"({
    "asset": {
        "version": "2.0"
    }
})";

    const char* c_expectedDefaultDocumentAndScene = R"({
    "asset": {
        "version": "2.0"
    },
    "scenes": [
        {}
    ]
})";

    const char* c_expectedDefaultDocumentAndSceneAsDefault = R"({
    "asset": {
        "version": "2.0"
    },
    "scenes": [
        {}
    ],
    "scene": 0
})";

    const char* c_expectedDefaultDocumentAndNonDefaultScene = R"({
    "asset": {
        "version": "2.0"
    },
    "scenes": [
        {}
    ]
})";

    const char* c_expectedDefaultDocumentAndNonDefaultSceneAsDefault = R"({
    "asset": {
        "version": "2.0"
    },
    "scenes": [
        {}
    ],
    "scene": 0
})";

    struct AlphaRegisteredExtension : Microsoft::glTF::Extension
    {
        std::unique_ptr<Microsoft::glTF::Extension> Clone() const override
        {
            return std::make_unique<AlphaRegisteredExtension>(*this);
        }

        bool IsEqual(
            const Microsoft::glTF::Extension& rhs) const override
        {
            return dynamic_cast<
                const AlphaRegisteredExtension*>(&rhs) != nullptr;
        }
    };

    struct ZuluRegisteredExtension : Microsoft::glTF::Extension
    {
        std::unique_ptr<Microsoft::glTF::Extension> Clone() const override
        {
            return std::make_unique<ZuluRegisteredExtension>(*this);
        }

        bool IsEqual(
            const Microsoft::glTF::Extension& rhs) const override
        {
            return dynamic_cast<
                const ZuluRegisteredExtension*>(&rhs) != nullptr;
        }
    };
}

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(SerializeTests)
            {
                GLTFSDK_TEST_METHOD(SerializeTests, SerializeNodeMatrixTransform)
                {
                    Document originalDoc;
                    Scene sc; sc.id = "0";
                    sc.nodes = { "0" };
                    originalDoc.SetDefaultScene(std::move(sc));
                    std::array<float, 16> matrixData; std::fill(matrixData.begin(), matrixData.end(), 1.0f);
                    Matrix4 mat4; mat4.values = matrixData;
                    Node matrixNode; matrixNode.id = "0"; matrixNode.name = "matrixNode";
                    matrixNode.matrix = mat4;
                    originalDoc.nodes.Append(std::move(matrixNode));
                    auto outputString = Serialize(originalDoc);
                    auto twoPassDoc = Deserialize(outputString);
                    Assert::IsTrue(twoPassDoc == originalDoc);
                }

                GLTFSDK_TEST_METHOD(SerializeTests, SerializeNodeTRSTransform)
                {
                    Document originalDoc;
                    Scene sc; sc.id = "0";
                    sc.nodes = { "0" };
                    originalDoc.SetDefaultScene(std::move(sc));
                    Vector3 translation = { 1.0f, 1.0f, 1.0f };
                    Vector3 scaling = { 0.1f, 0.42f, 0.133f };
                    Node trsNode; trsNode.id = "0"; trsNode.name = "trsNode";
                    trsNode.translation = translation;
                    trsNode.scale = scaling;
                    originalDoc.nodes.Append(std::move(trsNode));
                    auto outputString = Serialize(originalDoc);
                    auto twoPassDoc = Deserialize(outputString);
                    Assert::IsTrue(twoPassDoc == originalDoc);
                }

                GLTFSDK_TEST_METHOD(SerializeTests, SerializeNodeInvalidTransform)
                {
                    Assert::ExpectException<DocumentException>([]()
                    {
                        Document originalDoc;
                        Scene sc; sc.id = "0";
                        sc.nodes = { "0" };
                        originalDoc.SetDefaultScene(std::move(sc));
                        Vector3 translation = { 1.0f, 1.0f, 1.0f };
                        Vector3 scaling = { 0.1f, 0.42f, 0.133f };
                        std::array<float, 16> matrixData; std::fill(matrixData.begin(), matrixData.end(), 1.0f);
                        Matrix4 mat4; mat4.values = matrixData;
                        Node badNode; badNode.id = "0"; badNode.name = "badNode";
                        badNode.translation = translation;
                        badNode.scale = scaling;
                        badNode.matrix = mat4;
                        originalDoc.nodes.Append(std::move(badNode));
                        auto outputString = Serialize(originalDoc);
                    });
                }

                GLTFSDK_TEST_METHOD(SerializeTests, MatrixNodeTest)
                {
                    Node matrixNode;
                    std::array<float, 16> matrixData; std::fill(matrixData.begin(), matrixData.end(), 1.0f);
                    Matrix4 mat4; mat4.values = matrixData;
                    matrixNode.matrix = mat4;
                    Assert::IsTrue(matrixNode.GetTransformationType() == TransformationType::TRANSFORMATION_MATRIX);
                }

                GLTFSDK_TEST_METHOD(SerializeTests, NoTransformTest)
                {
                    Node defaultNode;
                    Assert::IsTrue(defaultNode.GetTransformationType() == TransformationType::TRANSFORMATION_IDENTITY);
                }

                GLTFSDK_TEST_METHOD(SerializeTests, TRSNodeTest)
                {
                    Node trsNode;
                    Vector3 scale = { 2.0f, 1.1f, 4.0f };
                    trsNode.scale = scale;
                    Assert::IsTrue(trsNode.GetTransformationType() == TransformationType::TRANSFORMATION_TRS);
                }

                GLTFSDK_TEST_METHOD(SerializeTests, InvalidNodeTest)
                {
                    Node badNode;
                    std::array<float, 16> matrixData; std::fill(matrixData.begin(), matrixData.end(), 1.0f);
                    Matrix4 mat4; mat4.values = matrixData;
                    Vector3 scale = { 2.0f, 1.1f, 4.0f };
                    badNode.matrix = mat4;
                    badNode.scale = scale;
                    Assert::IsFalse(badNode.HasValidTransformType());
                }

                GLTFSDK_TEST_METHOD(SerializeTests, ValidNodeTest)
                {
                    Node node;
                    Assert::IsTrue(node.HasValidTransformType());
                }

                GLTFSDK_TEST_METHOD(SerializeTests, PerspectiveCameraTest)
                {
                    Camera cam("0", "", std::make_unique<Perspective>(0.1f, 10.0f, 1.2f, 0.5f));
                    Assert::IsTrue(cam.projection->GetProjectionType() == PROJECTION_PERSPECTIVE);
                }

                GLTFSDK_TEST_METHOD(SerializeTests, DefaultDocument)
                {
                    Document doc;

                    const auto output = Serialize(doc, SerializeFlags::Pretty);
                    Assert::AreEqual(output.c_str(), c_expectedDefaultDocument);
                }

                GLTFSDK_TEST_METHOD(SerializeTests, CompactDefaultDocument)
                {
                    Document document;
                    Assert::AreEqual(
                        R"({"asset":{"version":"2.0"}})",
                        Serialize(document).c_str());
                }

                GLTFSDK_TEST_METHOD(SerializeTests, DefaultDocumentAndScene)
                {
                    Document doc;
                    doc.scenes.Append(Scene(), AppendIdPolicy::GenerateOnEmpty);

                    const auto output = Serialize(doc, SerializeFlags::Pretty);
                    Assert::AreEqual(output.c_str(), c_expectedDefaultDocumentAndScene);
                }

                GLTFSDK_TEST_METHOD(SerializeTests, DefaultDocumentAndSceneAsDefault)
                {
                    Document doc;
                    doc.SetDefaultScene(Scene(), AppendIdPolicy::GenerateOnEmpty);

                    const auto output = Serialize(doc, SerializeFlags::Pretty);
                    Assert::AreEqual(output.c_str(), c_expectedDefaultDocumentAndSceneAsDefault);
                }

                GLTFSDK_TEST_METHOD(SerializeTests, DefaultDocumentAndNonDefaultScene)
                {
                    Document doc;
                    Scene scene;
                    scene.id = "foo";
                    doc.scenes.Append(std::move(scene));

                    const auto output = Serialize(doc, SerializeFlags::Pretty);
                    Assert::AreEqual(output.c_str(), c_expectedDefaultDocumentAndNonDefaultScene);
                }

                GLTFSDK_TEST_METHOD(SerializeTests, DefaultDocumentAndNonDefaultSceneAsDefault)
                {
                    Document doc;
                    Scene scene;
                    scene.id = "foo";
                    doc.SetDefaultScene(std::move(scene));

                    const auto output = Serialize(doc, SerializeFlags::Pretty);
                    Assert::AreEqual(output.c_str(), c_expectedDefaultDocumentAndNonDefaultSceneAsDefault);
                }

                GLTFSDK_TEST_METHOD(SerializeTests, DeterministicCoreExtensionAndExtrasOrder)
                {
                    Document document;
                    document.asset.extras =
                        R"({"second":2,"first":1})";
                    document.extras =
                        R"({"rootSecond":2,"rootFirst":1})";

                    Accessor position;
                    position.id = "0";
                    position.componentType = COMPONENT_FLOAT;
                    position.count = 1U;
                    position.type = TYPE_SCALAR;
                    document.accessors.Append(std::move(position));

                    Accessor normal;
                    normal.id = "1";
                    normal.componentType = COMPONENT_FLOAT;
                    normal.count = 1U;
                    normal.type = TYPE_SCALAR;
                    document.accessors.Append(std::move(normal));

                    MeshPrimitive primitive;
                    primitive.attributes[ACCESSOR_POSITION] = "0";
                    primitive.attributes[ACCESSOR_NORMAL] = "1";
                    Mesh mesh;
                    mesh.id = "0";
                    mesh.primitives.push_back(std::move(primitive));
                    document.meshes.Append(std::move(mesh));

                    document.SetExtension<ZuluRegisteredExtension>();
                    document.SetExtension<AlphaRegisteredExtension>();
                    document.extensions.emplace(
                        "Z_unregistered",
                        R"({"second":2,"first":1})");
                    document.extensions.emplace(
                        "A_unregistered",
                        R"({"second":2,"first":1})");
                    document.extensionsUsed = {
                        "Z_unregistered",
                        "A_unregistered",
                        "Z_registered",
                        "A_registered"
                    };

                    ExtensionSerializer serializer;
                    serializer.AddHandler<AlphaRegisteredExtension>(
                        "A_registered",
                        [](const AlphaRegisteredExtension&,
                           const Document&,
                           const ExtensionSerializer&)
                        {
                            return std::string(
                                R"({"second":2,"first":1})");
                        });
                    serializer.AddHandler<ZuluRegisteredExtension>(
                        "Z_registered",
                        [](const ZuluRegisteredExtension&,
                           const Document&,
                           const ExtensionSerializer&)
                        {
                            return std::string(
                                R"({"second":2,"first":1})");
                        });

                    const auto output =
                        Serialize(document, serializer);
                    const auto alphaRegistered =
                        output.find("\"A_registered\"");
                    const auto zuluRegistered =
                        output.find("\"Z_registered\"");
                    const auto alphaUnregistered =
                        output.find("\"A_unregistered\"");
                    const auto zuluUnregistered =
                        output.find("\"Z_unregistered\"");

                    Assert::IsTrue(
                        alphaRegistered < zuluRegistered);
                    Assert::IsTrue(
                        zuluRegistered < alphaUnregistered);
                    Assert::IsTrue(
                        alphaUnregistered < zuluUnregistered);
                    Assert::IsTrue(
                        output.find(
                            R"("extras":{"second":2,"first":1})") !=
                        std::string::npos);
                    Assert::IsTrue(
                        output.find(
                            R"("extras":{"rootSecond":2,"rootFirst":1})") !=
                        std::string::npos);
                    Assert::IsTrue(
                        output.find(
                            R"("attributes":{"NORMAL":1,"POSITION":0})") !=
                        std::string::npos);
                    Assert::IsTrue(
                        output.find(
                            R"("extensionsUsed":["A_registered","A_unregistered","Z_registered","Z_unregistered"])") !=
                        std::string::npos);

                    for (int iteration = 0; iteration < 20; ++iteration)
                    {
                        Assert::AreEqual(
                            output.c_str(),
                            Serialize(document, serializer).c_str());
                    }
                }

                GLTFSDK_TEST_METHOD(SerializeTests, NumericUtf8AndRoundTrip)
                {
                    Document document;
                    document.asset.generator =
                        std::string("quote\"\n") + "\xC3\xA9";
                    document.asset.extras =
                        R"({"one":1.0,"signed":-9223372036854775808,"unsigned":18446744073709551615,"negativeZero":-0.0,"exponent":1e-7})";

                    Buffer buffer;
                    buffer.id = "0";
                    buffer.byteLength =
                        std::numeric_limits<std::size_t>::max();
                    document.buffers.Append(std::move(buffer));

                    Node node;
                    node.id = "0";
                    node.scale = Vector3(2.0F, -0.0F, 1.0F);
                    document.nodes.Append(std::move(node));

                    const auto output = Serialize(document);
                    Assert::IsTrue(
                        output.find(
                            "\"byteLength\":" +
                            std::to_string(
                                std::numeric_limits<std::size_t>::max())) !=
                        std::string::npos);
                    Assert::IsTrue(
                        output.find(
                            R"("signed":-9223372036854775808)") !=
                        std::string::npos);
                    Assert::IsTrue(
                        output.find(
                            R"("unsigned":18446744073709551615)") !=
                        std::string::npos);
                    Assert::IsTrue(
                        output.find(R"("one":1.0)") !=
                        std::string::npos);
                    Assert::IsTrue(
                        output.find(R"("negativeZero":-0.0)") !=
                        std::string::npos);
                    Assert::IsTrue(
                        output.find(R"("exponent":1e-7)") !=
                        std::string::npos);
                    Assert::IsTrue(
                        output.find(R"("scale":[2.0,-0.0,1.0])") !=
                        std::string::npos);
                    Assert::IsTrue(
                        output.find("quote\\\"\\n\xC3\xA9") !=
                        std::string::npos);

                    const auto roundTrip = Deserialize(
                        output,
                        DeserializeFlags::None,
                        SchemaFlags::DisableSchemaRoot);
                    Assert::IsTrue(document == roundTrip);
                }

                GLTFSDK_TEST_METHOD(SerializeTests, RejectsNonFiniteAndInvalidUtf8)
                {
                    Assert::ExpectException<GLTFException>([]()
                    {
                        Document document;
                        Node node;
                        node.id = "0";
                        node.scale = Vector3(
                            std::numeric_limits<float>::infinity(),
                            1.0F,
                            1.0F);
                        document.nodes.Append(std::move(node));
                        Serialize(document);
                    });

                    Assert::ExpectException<GLTFException>([]()
                    {
                        Document document;
                        document.asset.generator =
                            std::string(1U, static_cast<char>(0xC3)) +
                            std::string(1U, static_cast<char>(0x28));
                        Serialize(document);
                    });

                    Assert::ExpectException<GLTFException>([]()
                    {
                        Document document;
                        document.asset.extras =
                            R"({"value":1,"value":2})";
                        Serialize(document);
                    });
                }

                GLTFSDK_TEST_METHOD(SerializeTests, InvalidDefaultScene)
                {
                    Scene scene;
                    scene.id = "foo";

                    Document doc;
                    doc.scenes.Append(std::move(scene));
                    doc.defaultSceneId = "bar";

                    Assert::ExpectException<GLTFException>([&doc]
                    {
                        try
                        {
                            Serialize(doc);
                        }
                        catch (const GLTFException& ex)
                        {
                            Assert::AreEqual("key bar not in container", ex.what());
                            throw;
                        }
                    }, L"Expected exception was not thrown");
                }
            };
        }
    }
}
