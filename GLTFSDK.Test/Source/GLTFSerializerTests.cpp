// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/Serialize.h>
#include <GLTFSDK/Validation.h>

#include <memory>

using namespace glTF::UnitTest;

namespace
{
    using namespace Microsoft::glTF;

    void TestBadGLTFSerializeToJson(const Document& doc)
    {
        Assert::ExpectException<GLTFException>([&doc]
        {
            Serialize(doc);
        }, L"Expected exception was not thrown");
    }

    void TestBadGLTFDeserializeToDocument(const char* data)
    {
        Assert::ExpectException<GLTFException>([&data]
        {
            Deserialize(data);
        }, L"Expected exception was not thrown");
    }

    void TestDocumentValidationFail(const char* data)
    {
        Assert::ExpectException<ValidationException>([&data]
        {
            auto doc = Deserialize(data);
            Validation::Validate(doc);
        }, L"Expected exception was not thrown");
    }

    const char* c_invalidPrimitiveAccessorComponentType = R"({
    "scenes": [{"nodes": [0]}],
    "nodes": [{"mesh": 0}],
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
            "uri": "triangleWithoutIndices.bin",
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
}
)";

    const char* c_validPrimitiveNoIndices = R"({
    "scenes": [{"nodes": [0]}],
    "nodes": [{"mesh": 0}],
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
}
)";

    const char* c_invalidPrimitiveAccessorType = R"({
    "scenes": [{"nodes": [0]}],
    "nodes": [{"mesh": 0}],
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
            "type": "VEC2"
        }
    ],
    "asset": {"version": "2.0"}
}
)";

    const char* c_negativeAccessorOffset = R"({
    "accessors": {
        "accessor_21": {
            "bufferView": "bufferView_29",
            "byteOffset": -10,
            "componentType": 5123,
            "count": 12636,
            "type": "SCALAR"
        }
    }
}
)";

    const char* c_negativeAccessorCount = R"({
    "accessors": {
        "accessor_21": {
            "bufferView": "bufferView_29",
            "byteOffset": 10,
            "componentType": 5123,
            "count": -12636,
            "type": "SCALAR"
        }
    }
}
)";

    const char* c_negativeBufferLength = R"({
    "buffers": {
        "Duck": {
            "byteLength": -102040,
            "type": "arraybuffer",
            "uri": "Duck.bin"
        }
    }
}
)";

    const char* c_negativeBufferViewOffset = R"({
    "bufferViews": {
        "bufferView_29": {
            "buffer": "Duck",
            "byteLength": 25272,
            "byteOffset": -10,
            "target": 34963
        }
    }
}
)";

    const char* c_negativeBufferViewLength = R"({
    "bufferViews": {
        "bufferView_29": {
            "buffer": "Duck",
            "byteLength": -25272,
            "byteOffset": 10,
            "target": 34963
        }
    }
}
)";

    const char* c_invalidAccessorComponentType = R"({
    "accessors": {
        "accessor_21": {
            "bufferView": "bufferView_29",
            "byteOffset": 0,
            "componentType": 1337,
            "count": 12636,
            "type": "SCALAR"
        }
    }
}
)";

    const char* c_extraFieldsJson = R"({
    "accessors": {},
    "accessors-extra": {}
}
)";

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
}

namespace Microsoft
{
    namespace  glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(SerializerGLTFTests)
            {
                BEGIN_TEST_CLASS_ATTRIBUTE()
                    TEST_CLASS_ATTRIBUTE(L"Priority", L"1")
                    TEST_CLASS_ATTRIBUTE(L"Category", L"Unit-Integration")
                END_TEST_CLASS_ATTRIBUTE()

                GLTFSDK_TEST_METHOD(SerializerGLTFTests, SerializerGLTFTests_ValidPrimitiveNoIndices)
                {
                    auto doc = Deserialize(c_validPrimitiveNoIndices);
                    Validation::Validate(doc);
                }

                GLTFSDK_TEST_METHOD(SerializerGLTFTests, SerializerGLTFTests_InvalidPrimitiveAccessorComponentType)
                {
                    TestDocumentValidationFail(c_invalidPrimitiveAccessorComponentType);
                }

                GLTFSDK_TEST_METHOD(SerializerGLTFTests, SerializerGLTFTests_InvalidPrimitiveAccessorType)
                {
                    TestDocumentValidationFail(c_invalidPrimitiveAccessorType);
                }

                // following test cases are only checked while deserializing to gltfdocument
                GLTFSDK_TEST_METHOD(SerializerGLTFTests, SerializerGLTFTests_NegativeAccessorOffset)
                {
                    TestBadGLTFDeserializeToDocument(c_negativeAccessorOffset);
                }

                GLTFSDK_TEST_METHOD(SerializerGLTFTests, SerializerGLTFTests_NegativeBufferViewOffset)
                {
                    TestBadGLTFDeserializeToDocument(c_negativeBufferViewOffset);
                }

                GLTFSDK_TEST_METHOD(SerializerGLTFTests, SerializerGLTFTests_NegativeBufferLength)
                {
                    TestBadGLTFDeserializeToDocument(c_negativeBufferLength);
                }

                GLTFSDK_TEST_METHOD(SerializerGLTFTests, SerializerGLTFTests_NegativeBufferViewLength)
                {
                    TestBadGLTFDeserializeToDocument(c_negativeBufferViewLength);
                }

                GLTFSDK_TEST_METHOD(SerializerGLTFTests, SerializerGLTFTests_NegativeAccessorCount)
                {
                    TestBadGLTFDeserializeToDocument(c_negativeAccessorCount);
                }

                // TODO: Add NegativeBufferViewByteStride test

                // TODO: Add TooLargeBufferViewByteStride test

                GLTFSDK_TEST_METHOD(SerializerGLTFTests, SerializerGLTFTests_InvalidAccessorComponentType)
                {
                    TestBadGLTFDeserializeToDocument(c_invalidAccessorComponentType);
                }

                GLTFSDK_TEST_METHOD(SerializerGLTFTests, SerializerGLTFTests_ExtraRootFields)
                {
                    TestBadGLTFDeserializeToDocument(c_extraFieldsJson);
                }

                GLTFSDK_TEST_METHOD(SerializerGLTFTests, SerializerGLTFTests_DefaultDocument)
                {
                    Document doc;

                    const auto output = Serialize(doc, SerializeFlags::Pretty);
                    Assert::AreEqual(output.c_str(), c_expectedDefaultDocument);
                }

                GLTFSDK_TEST_METHOD(SerializerGLTFTests, SerializerGLTFTests_DefaultDocumentAndScene)
                {
                    Document doc;
                    doc.scenes.Append(Scene(), AppendIdPolicy::GenerateOnEmpty);

                    const auto output = Serialize(doc, SerializeFlags::Pretty);
                    Assert::AreEqual(output.c_str(), c_expectedDefaultDocumentAndScene);
                }

                GLTFSDK_TEST_METHOD(SerializerGLTFTests, SerializerGLTFTests_DefaultDocumentAndSceneAsDefault)
                {
                    Document doc;
                    doc.SetDefaultScene(Scene(), AppendIdPolicy::GenerateOnEmpty);

                    const auto output = Serialize(doc, SerializeFlags::Pretty);
                    Assert::AreEqual(output.c_str(), c_expectedDefaultDocumentAndSceneAsDefault);
                }

                GLTFSDK_TEST_METHOD(SerializerGLTFTests, SerializerGLTFTests_DefaultDocumentAndNonDefaultScene)
                {
                    Document doc;
                    Scene scene;
                    scene.id = "foo";
                    doc.scenes.Append(std::move(scene));

                    const auto output = Serialize(doc, SerializeFlags::Pretty);
                    Assert::AreEqual(output.c_str(), c_expectedDefaultDocumentAndNonDefaultScene);
                }

                GLTFSDK_TEST_METHOD(SerializerGLTFTests, SerializerGLTFTests_DefaultDocumentAndNonDefaultSceneAsDefault)
                {
                    Document doc;
                    Scene scene;
                    scene.id = "foo";
                    doc.SetDefaultScene(std::move(scene));

                    const auto output = Serialize(doc, SerializeFlags::Pretty);
                    Assert::AreEqual(output.c_str(), c_expectedDefaultDocumentAndNonDefaultSceneAsDefault);
                }

                GLTFSDK_TEST_METHOD(SerializerGLTFTests, SerializerGLTFTests_InvalidDefaultScene)
                {
                    Document doc;
                    Scene scene;
                    scene.id = "foo";
                    doc.scenes.Append(std::move(scene));
                    doc.defaultSceneId = "bar";

                    TestBadGLTFSerializeToJson(doc);
                }
            };
        }
    }
}
