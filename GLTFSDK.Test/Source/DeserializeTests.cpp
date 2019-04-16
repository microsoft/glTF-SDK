// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/Validation.h>

using namespace glTF::UnitTest;

namespace
{
    const char* c_validPrimitiveNoIndices = R"({
    "meshes": [
        {
            "primitives": [
                {
                    "attributes": {
                        "POSITION": 0
                    }
                }
            ]
        }
    ],
    "buffers": [
        {
            "byteLength": 36
        }
    ],
    "bufferViews": [
        {
            "buffer": 0,
            "byteOffset": 0,
            "byteLength": 36,
            "target": 34962
        }
    ],
    "accessors": [
        {
            "bufferView": 0,
            "byteOffset": 0,
            "componentType": 5126,
            "count": 3,
            "type": "VEC3"
        }
    ],
    "asset": {"version": "2.0"}
})";

    // Color data must be FLOAT, UNSIGNED_SHORT or UNSIGNED_BYTE (not BYTE)
    const char* c_invalidPrimitiveAccessorComponentType = R"({
    "meshes": [
        {
            "primitives": [
                {
                    "attributes": {
                        "COLOR_0": 0,
                        "POSITION": 1
                    }
                }
            ]
        }
    ],
    "buffers": [
        {
            "byteLength": 72
        }
    ],
    "bufferViews": [
        {
            "buffer": 0,
            "byteOffset": 0,
            "byteLength": 72,
            "target": 34962
        }
    ],
    "accessors": [
        {
            "bufferView": 0,
            "byteOffset": 0,
            "componentType": 5120,
            "count": 3,
            "type": "VEC3"
        },
        {
            "bufferView": 0,
            "byteOffset": 36,
            "componentType": 5126,
            "count": 3,
            "type": "VEC3"
        }
    ],
    "asset": {"version": "2.0"}
})";

    // Position data must be VEC3 (not VEC2)
    const char* c_invalidPrimitiveAccessorType = R"({
    "meshes": [
        {
            "primitives": [
                {
                    "attributes": {
                        "POSITION": 0
                    }
                }
            ]
        }
    ],
    "buffers": [
        {
            "uri": "triangleWithoutIndices.bin",
            "byteLength": 24
        }
    ],
    "bufferViews": [
        {
            "buffer": 0,
            "byteLength": 24,
            "target": 34962
        }
    ],
    "accessors": [
        {
            "bufferView": 0,
            "componentType": 5126,
            "count": 3,
            "type": "VEC2"
        }
    ],
    "asset": {"version": "2.0"}
})";

    const char* c_validAccessor = R"({
    "accessors": [
        {
            "componentType": 5123,
            "count": 12636,
            "type": "SCALAR"
        }
    ],
    "asset": {"version": "2.0"}
})";

    const char* c_negativeAccessorOffset = R"({
    "buffers": [
        {
            "byteLength": 12
        }
    ],
    "bufferViews": [
        {
            "buffer": 0,
            "byteLength": 12
        }
    ],
    "accessors": [
        {
            "bufferView": 0,
            "byteOffset": -10,
            "componentType": 5123,
            "count": 1,
            "type": "SCALAR"
        }
    ],
    "asset": {"version": "2.0"}
})";

    const char* c_negativeAccessorCount = R"({
    "accessors": [
        {
            "componentType": 5123,
            "count": -1,
            "type": "SCALAR"
        }
    ],
    "asset": {"version": "2.0"}
})";

    // When byteOffset property is present an accessor must reference a bufferView
    const char* c_invalidAccessorDependency = R"({
    "accessors": [
        {
            "byteOffset": 0,
            "componentType": 5123,
            "count": 1,
            "type": "SCALAR"
        }
    ],
    "asset": {"version": "2.0"}
})";

    // '1337' is not an valid accessor enum value (5120, 5121, 5122, 5123, 5125, 5126)
    const char* c_invalidAccessorComponentType = R"({
    "buffers": [
        {
            "byteLength": 4
        }
    ],
    "bufferViews": [
        {
            "buffer": 0,
            "byteLength": 4
        }
    ],
    "accessors": [
        {
            "bufferView": 0,
            "componentType": 1337,
            "count": 1,
            "type": "SCALAR"
        }
    ],
    "asset": {"version": "2.0"}
}
)";

    const char* c_negativeBufferLength = R"({
    "buffers": [
        {
            "byteLength": -1
        }
    ],
    "asset": {"version": "2.0"}
})";

    const char* c_negativeBufferViewOffset = R"({
    "buffers": [
        {
            "byteLength": 25282
        }
    ],
    "bufferViews": [
        {
            "buffer": 0,
            "byteLength": 25272,
            "byteOffset": -10,
            "target": 34963
        }
    ],
    "asset": {"version": "2.0"}
})";

    const char* c_negativeBufferViewLength = R"({
    "buffers": [
        {
            "byteLength": 25282
        }
    ],
    "bufferViews": [
        {
            "buffer": 0,
            "byteLength": -25272,
            "byteOffset": 10,
            "target": 34963
        }
    ],
    "asset": {"version": "2.0"}
})";

    const char* c_extraFieldsJson = R"({
    "asset": {"version": "2.0"},
    "assetExtra": {}
})";

    const char* c_validSamplerDocument = R"({
    "samplers": [
        {
            "minFilter": 9728,
            "magFilter": 9729
        },
        {
            "wrapS": 33648,
            "wrapT": 33071
        }
    ],
    "asset": {"version": "2.0"}
})";
}

namespace Microsoft
{
    namespace  glTF
    {
        std::wstring ToString(WrapMode wrapMode)
        {
            switch (wrapMode)
            {
            case Wrap_REPEAT:
                return L"REPEAT";
            case Wrap_CLAMP_TO_EDGE:
                return L"CLAMP_TO_EDGE";
            case Wrap_MIRRORED_REPEAT:
                return L"MIRRORED_REPEAT";
            }

            return {};
        }

        std::wstring ToString(MinFilterMode minFilterMode)
        {
            switch (minFilterMode)
            {
            case MinFilter_NEAREST:
                return L"NEAREST";
            case MinFilter_NEAREST_MIPMAP_LINEAR:
                return L"NEAREST_MIPMAP_LINEAR";
            case MinFilter_NEAREST_MIPMAP_NEAREST:
                return L"NEAREST_MIPMAP_NEAREST";
            case MinFilter_LINEAR:
                return L"LINEAR";
            case MinFilter_LINEAR_MIPMAP_LINEAR:
                return L"LINEAR_MIPMAP_LINEAR";
            case MinFilter_LINEAR_MIPMAP_NEAREST:
                return L"LINEAR_MIPMAP_NEAREST";
            }

            return {};
        }

        std::wstring ToString(MagFilterMode magFilterMode)
        {
            switch (magFilterMode)
            {
            case MagFilter_NEAREST:
                return L"NEAREST";
            case MagFilter_LINEAR:
                return L"LINEAR";
            }

            return {};
        }

        namespace Test
        {
            GLTFSDK_TEST_CLASS(DeserializeTests)
            {
                GLTFSDK_TEST_METHOD(DeserializeTests, ValidationSuccess_ValidPrimitiveNoIndices)
                {
                    auto doc = Deserialize(c_validPrimitiveNoIndices);

                    Validation::Validate(doc);

                    Assert::AreEqual(size_t(1), doc.meshes.Size());
                    Assert::AreEqual(size_t(1), doc.meshes.Front().primitives.size());

                    const auto& accessor = doc.meshes.Front().primitives.front();

                    Assert::IsTrue(accessor.HasAttribute(glTF::ACCESSOR_POSITION));
                    Assert::IsTrue(accessor.indicesAccessorId.empty());
                }

                GLTFSDK_TEST_METHOD(DeserializeTests, ValidationFail_InvalidPrimitiveAccessorComponentType)
                {
                    auto doc = Deserialize(c_invalidPrimitiveAccessorComponentType);

                    Assert::ExpectException<ValidationException>([&doc]()
                    {
                        try
                        {
                            Validation::Validate(doc);
                        }
                        catch (const ValidationException& ex)
                        {
                            Assert::AreEqual("Accessor 0 COLOR_0 componentType must be: [UNSIGNED_BYTE, UNSIGNED_SHORT, FLOAT]", ex.what());
                            throw;
                        }
                    });
                }

                GLTFSDK_TEST_METHOD(DeserializeTests, ValidationFail_InvalidPrimitiveAccessorType)
                {
                    auto doc = Deserialize(c_invalidPrimitiveAccessorType);

                    Assert::ExpectException<ValidationException>([&doc]()
                    {
                        try
                        {
                            Validation::Validate(doc);
                        }
                        catch (const ValidationException& ex)
                        {
                            Assert::AreEqual("Accessor 0 POSITION type must be: [VEC3]", ex.what());
                            throw;
                        }
                    });
                }

                GLTFSDK_TEST_METHOD(DeserializeTests, DeserializeSuccess_ExtraRootFields)
                {
                    auto doc = Deserialize(c_extraFieldsJson);

                    Assert::AreEqual(GLTF_VERSION_2_0, doc.asset.version.c_str());
                }

                GLTFSDK_TEST_METHOD(DeserializeTests, DeserializeSuccess_ValidAccessor)
                {
                    auto doc = Deserialize(c_validAccessor);

                    Assert::AreEqual(size_t(1), doc.accessors.Size());

                    const auto& accessor = doc.accessors.Front();

                    Assert::IsTrue(accessor.bufferViewId.empty());
                    Assert::AreEqual(size_t(0), accessor.byteOffset);
                    Assert::AreEqual(size_t(25272), accessor.GetByteLength());
                }

                GLTFSDK_TEST_METHOD(DeserializeTests, DeserializeFail_NegativeAccessorOffset)
                {
                    Assert::ExpectException<ValidationException>([]()
                    {
                        try
                        {
                            Deserialize(c_negativeAccessorOffset);
                        }
                        catch (const ValidationException& ex)
                        {
                            Assert::AreEqual("Schema violation at #/accessors/0/byteOffset due to minimum", ex.what());
                            throw;
                        }
                    });
                }

                GLTFSDK_TEST_METHOD(DeserializeTests, DeserializeFail_NegativeAccessorCount)
                {
                    Assert::ExpectException<ValidationException>([]()
                    {
                        try
                        {
                            Deserialize(c_negativeAccessorCount);
                        }
                        catch (const ValidationException& ex)
                        {
                            Assert::AreEqual("Schema violation at #/accessors/0/count due to minimum", ex.what());
                            throw;
                        }
                    });
                }

                GLTFSDK_TEST_METHOD(DeserializeTests, DeserializeFail_InvalidAccessorDependency)
                {
                    Assert::ExpectException<ValidationException>([]()
                    {
                        try
                        {
                            Deserialize(c_invalidAccessorDependency);
                        }
                        catch (const ValidationException& ex)
                        {
                            Assert::AreEqual("Schema violation at #/accessors/0 due to dependencies", ex.what());
                            throw;
                        }
                    });
                }

                GLTFSDK_TEST_METHOD(DeserializeTests, DeserializeFail_InvalidAccessorComponentType)
                {
                    auto doc = Deserialize(c_invalidAccessorComponentType);

                    Assert::ExpectException<GLTFException>([&doc]()
                    {
                        try
                        {
                            Validation::Validate(doc);
                        }
                        catch (const GLTFException& ex)
                        {
                            Assert::AreEqual("Unknown componentType 0", ex.what());
                            throw;
                        }
                    });
                }

                GLTFSDK_TEST_METHOD(DeserializeTests, DeserializeFail_NegativeBufferLength)
                {
                    Assert::ExpectException<ValidationException>([]()
                    {
                        try
                        {
                            Deserialize(c_negativeBufferLength);
                        }
                        catch (const ValidationException& ex)
                        {
                            Assert::AreEqual("Schema violation at #/buffers/0/byteLength due to minimum", ex.what());
                            throw;
                        }
                    });
                }

                GLTFSDK_TEST_METHOD(DeserializeTests, DeserializeFail_NegativeBufferViewOffset)
                {
                    Assert::ExpectException<ValidationException>([]()
                    {
                        try
                        {
                            Deserialize(c_negativeBufferViewOffset);
                        }
                        catch (const ValidationException& ex)
                        {
                            Assert::AreEqual("Schema violation at #/bufferViews/0/byteOffset due to minimum", ex.what());
                            throw;
                        }
                    });
                }

                GLTFSDK_TEST_METHOD(DeserializeTests, DeserializeFail_NegativeBufferViewLength)
                {
                    Assert::ExpectException<ValidationException>([]()
                    {
                        try
                        {
                            Deserialize(c_negativeBufferViewLength);
                        }
                        catch (const ValidationException& ex)
                        {
                            Assert::AreEqual("Schema violation at #/bufferViews/0/byteLength due to minimum", ex.what());
                            throw;
                        }
                    });
                }

                // TODO: Add NegativeBufferViewByteStride test
                // TODO: Add TooLargeBufferViewByteStride test

                GLTFSDK_TEST_METHOD(DeserializeTests, DeserializeSuccess_DeserializeSampler)
                {
                    auto doc = Deserialize(c_validSamplerDocument);

                    Assert::AreEqual(doc.samplers.Size(), size_t(2U), L"Unexpected number of samplers after deserializing manifest");

                    Assert::AreEqual(doc.samplers[0].minFilter.Get(), MinFilter_NEAREST, L"Sampler minification filter was not deserialized correctly");
                    Assert::AreEqual(doc.samplers[0].magFilter.Get(), MagFilter_LINEAR, L"Sampler magnification filter was not deserialized correctly");

                    Assert::AreEqual(doc.samplers[0].wrapS, Wrap_REPEAT, L"Sampler default wrapS property was not deserialized correctly");
                    Assert::AreEqual(doc.samplers[0].wrapT, Wrap_REPEAT, L"Sampler default wrapT property was not deserialized correctly");

                    Assert::IsFalse(doc.samplers[1].minFilter.HasValue(), L"Sampler default minification filter was not unspecified");
                    Assert::IsFalse(doc.samplers[1].magFilter.HasValue(), L"Sampler default magnification filter was not unspecified");

                    Assert::AreEqual(doc.samplers[1].wrapS, Wrap_MIRRORED_REPEAT, L"Sampler wrapS property was not deserialized correctly");
                    Assert::AreEqual(doc.samplers[1].wrapT, Wrap_CLAMP_TO_EDGE, L"Sampler wrapT property was not deserialized correctly");
                }
            };
        }
    }
}
