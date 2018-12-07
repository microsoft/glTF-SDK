// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/Validation.h>
#include <GLTFSDK/Deserialize.h>

#include "TestResources.h"
#include "TestUtils.h"

#include <cmath>

using namespace glTF::UnitTest;

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            namespace
            {
                Document ReadAsset(const char* path)
                {
                    const auto inputJson = ReadLocalJson(path);
                    auto doc = Deserialize(inputJson);
                    return doc;
                }

                void ReadAndValidate(const char* path)
                {
                    auto doc = ReadAsset(path);
                    Validation::Validate(doc);
                }

                void ExpectValidationFail(const Document& doc)
                {
                    Assert::ExpectException<ValidationException>([&doc]{
                        Validation::Validate(doc);
                    });
                }
            }

            GLTFSDK_TEST_CLASS(ValidationUnitTests)
            {
                GLTFSDK_TEST_METHOD(ValidationUnitTests, TestAddition_size_t_NoOverflow)
                {
                    size_t a = std::numeric_limits<size_t>::max() - 500;
                    size_t b = 42;
                    size_t t;
                    Assert::IsTrue(Validation::SafeAddition(a, b, t) && t == (a + b));
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, TestAddition_size_t_MaxNoOverflow)
                {
                    size_t a = std::numeric_limits<size_t>::max() - 1;
                    size_t b = 1;
                    size_t t;
                    Assert::IsTrue(Validation::SafeAddition(a, b, t) && t == (a + b));
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, TestAddition_size_t_MinOverflow)
                {
                    size_t a = std::numeric_limits<size_t>::max();
                    size_t b = 1;
                    size_t t;
                    Assert::IsFalse(Validation::SafeAddition(a, b, t));
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, TestAddition_size_t_Overflow)
                {
                    size_t a = std::numeric_limits<size_t>::max();
                    size_t b = 42;
                    size_t t;
                    Assert::IsFalse(Validation::SafeAddition(a, b, t));
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, TestMultiplication_size_t_NoOverflow)
                {
                    size_t a = 42;
                    size_t b = 42;
                    size_t t;
                    Assert::IsTrue(Validation::SafeMultiplication(a, b, t) && t == a * b);
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, TestMultiplication_size_t_MaxNoOverflow)
                {
                    size_t a = std::numeric_limits<size_t>::max() >> 1;
                    size_t b = 2;
                    size_t t;
                    Assert::IsTrue(Validation::SafeMultiplication(a, b, t) && t == (a * b));
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, TestMultiplication_size_t_MinOverflow)
                {
                    size_t a = std::numeric_limits<size_t>::max() >> (sizeof(size_t) / 2);
                    size_t b = a;
                    size_t t;
                    Assert::IsFalse(Validation::SafeMultiplication(a, b, t));
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, TestMultiplication_size_t_Overflow)
                {
                    size_t a = std::numeric_limits<size_t>::max();
                    size_t b = std::numeric_limits<size_t>::max();
                    size_t t;
                    Assert::IsFalse(Validation::SafeMultiplication(a, b, t));
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, TestDraco_Validation)
                {
                    const auto inputJson = ReadLocalJson(c_dracoBox);
                    auto doc = Deserialize(inputJson);

                    Assert::AreEqual(doc.buffers.Size(), size_t(1));
                    Assert::AreEqual(doc.bufferViews.Size(), size_t(1));
                    Assert::AreEqual(doc.accessors.Size(), size_t(3));
                    for (const auto& accessor : doc.accessors.Elements())
                    {
                        Assert::IsTrue(accessor.bufferViewId.empty());
                    }

                    Validation::Validate(doc);
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, Validate_MeshPrimitive_00)
                {
                    ReadAndValidate(c_meshPrimitiveMode_00);
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, Validate_MeshPrimitive_01)
                {
                    ReadAndValidate(c_meshPrimitiveMode_01);
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, Validate_MeshPrimitive_02)
                {
                    ReadAndValidate(c_meshPrimitiveMode_02);
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, Validate_MeshPrimitive_03)
                {
                    ReadAndValidate(c_meshPrimitiveMode_03);
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, Validate_MeshPrimitive_04)
                {
                    ReadAndValidate(c_meshPrimitiveMode_04);
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, Validate_MeshPrimitive_05)
                {
                    ReadAndValidate(c_meshPrimitiveMode_05);
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, Validate_MeshPrimitive_06)
                {
                    ReadAndValidate(c_meshPrimitiveMode_06);
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, Validate_MeshPrimitive_07)
                {
                    ReadAndValidate(c_meshPrimitiveMode_07);
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, Validate_MeshPrimitive_08)
                {
                    ReadAndValidate(c_meshPrimitiveMode_08);
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, Validate_MeshPrimitive_09)
                {
                    ReadAndValidate(c_meshPrimitiveMode_09);
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, Validate_MeshPrimitive_10)
                {
                    ReadAndValidate(c_meshPrimitiveMode_10);
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, Validate_MeshPrimitive_11)
                {
                    ReadAndValidate(c_meshPrimitiveMode_11);
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, Validate_MeshPrimitive_12)
                {
                    ReadAndValidate(c_meshPrimitiveMode_12);
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, Validate_MeshPrimitive_13)
                {
                    ReadAndValidate(c_meshPrimitiveMode_13);
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, Validate_MeshPrimitive_14)
                {
                    ReadAndValidate(c_meshPrimitiveMode_14);
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, Validate_MeshPrimitive_15)
                {
                    ReadAndValidate(c_meshPrimitiveMode_15);
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, Validate_Invalid_Unindexed_Lines)
                {
                    auto doc = ReadAsset(c_meshPrimitiveMode_01);
                    auto accessor = doc.accessors[doc.meshes.Front().primitives.front().GetAttributeAccessorId(ACCESSOR_POSITION)];

                    accessor.count = 1;
                    doc.accessors.Replace(accessor);
                    ExpectValidationFail(doc);

                    accessor.count = 3;
                    doc.accessors.Replace(accessor);
                    ExpectValidationFail(doc);
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, Validate_Invalid_Unindexed_Line_Loop)
                {
                    auto doc = ReadAsset(c_meshPrimitiveMode_02);
                    auto accessor = doc.accessors[doc.meshes.Front().primitives.front().GetAttributeAccessorId(ACCESSOR_POSITION)];

                    accessor.count = 1;
                    doc.accessors.Replace(accessor);
                    ExpectValidationFail(doc);
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, Validate_Invalid_Unindexed_Line_Strip)
                {
                    auto doc = ReadAsset(c_meshPrimitiveMode_03);
                    auto accessor = doc.accessors[doc.meshes.Front().primitives.front().GetAttributeAccessorId(ACCESSOR_POSITION)];

                    accessor.count = 1;
                    doc.accessors.Replace(accessor);
                    ExpectValidationFail(doc);
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, Validate_Invalid_Indexed_Lines)
                {
                    auto doc = ReadAsset(c_meshPrimitiveMode_08);
                    auto accessor = doc.accessors[doc.meshes.Front().primitives.front().indicesAccessorId];

                    accessor.count = 1;
                    doc.accessors.Replace(accessor);
                    ExpectValidationFail(doc);

                    accessor.count = 3;
                    doc.accessors.Replace(accessor);
                    ExpectValidationFail(doc);
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, Validate_Invalid_Indexed_Line_Loop)
                {
                    auto doc = ReadAsset(c_meshPrimitiveMode_09);
                    auto accessor = doc.accessors[doc.meshes.Front().primitives.front().indicesAccessorId];

                    accessor.count = 1;
                    doc.accessors.Replace(accessor);
                    ExpectValidationFail(doc);
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, Validate_Invalid_Indexed_Line_Strip)
                {
                    auto doc = ReadAsset(c_meshPrimitiveMode_10);
                    auto accessor = doc.accessors[doc.meshes.Front().primitives.front().indicesAccessorId];

                    accessor.count = 1;
                    doc.accessors.Replace(accessor);
                    ExpectValidationFail(doc);
                }
            };
        }
    }
}
