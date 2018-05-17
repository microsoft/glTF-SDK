// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/BufferBuilder.h>
#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLTFResourceWriter.h>
#include <GLTFSDK/MeshPrimitiveUtils.h>
#include <GLTFSDK/Serialize.h>

using namespace glTF::UnitTest;

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(SerializeUnitTest)
            {
                GLTFSDK_TEST_METHOD(SerializeUnitTest, SerializeNodeMatrixTransform)
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

                GLTFSDK_TEST_METHOD(SerializeUnitTest, SerializeNodeTRSTransform)
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

                GLTFSDK_TEST_METHOD(SerializeUnitTest, SerializeNodeInvalidTransform)
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

                GLTFSDK_TEST_METHOD(SerializeUnitTest, MatrixNodeTest)
                {
                    Node matrixNode;
                    std::array<float, 16> matrixData; std::fill(matrixData.begin(), matrixData.end(), 1.0f);
                    Matrix4 mat4; mat4.values = matrixData;
                    matrixNode.matrix = mat4;
                    Assert::IsTrue(matrixNode.GetTransformationType() == TransformationType::TRANSFORMATION_MATRIX);
                }

                GLTFSDK_TEST_METHOD(SerializeUnitTest, NoTransformTest)
                {
                    Node defaultNode;
                    Assert::IsTrue(defaultNode.GetTransformationType() == TransformationType::TRANSFORMATION_IDENTITY);
                }

                GLTFSDK_TEST_METHOD(SerializeUnitTest, TRSNodeTest)
                {
                    Node trsNode;
                    Vector3 scale = { 2.0f, 1.1f, 4.0f };
                    trsNode.scale = scale;
                    Assert::IsTrue(trsNode.GetTransformationType() == TransformationType::TRANSFORMATION_TRS);
                }

                GLTFSDK_TEST_METHOD(SerializeUnitTest, InvalidNodeTest)
                {
                    Node badNode;
                    std::array<float, 16> matrixData; std::fill(matrixData.begin(), matrixData.end(), 1.0f);
                    Matrix4 mat4; mat4.values = matrixData;
                    Vector3 scale = { 2.0f, 1.1f, 4.0f };
                    badNode.matrix = mat4;
                    badNode.scale = scale;
                    Assert::IsFalse(badNode.HasValidTransformType());
                }

                GLTFSDK_TEST_METHOD(SerializeUnitTest, ValidNodeTest)
                {
                    Node node;
                    Assert::IsTrue(node.HasValidTransformType());
                }

                GLTFSDK_TEST_METHOD(SerializeUnitTest, PerspectiveCameraTest)
                {
                    Camera cam("0", "", std::make_unique<Perspective>(0.1f, 10.0f, 1.2f, 0.5f));
                    Assert::IsTrue(cam.projection->GetProjectionType() == ProjectionType::PERSPECTIVE);
                }
            };
        }
    }
}
