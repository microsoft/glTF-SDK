// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/GLTFResourceReader.h>

#include "TestUtils.h"

using namespace glTF::UnitTest;

namespace
{
    static const char test_json[] = R"(
{
    "asset":
    {
        "version": "2.0"
    },
    "buffers": [
        {
            "byteLength": 8,
            "uri": "buffer.bin"
        }
    ],
    "bufferViews": [
        {
            "buffer": 0,
            "byteLength": 8,
            "byteOffset": 0
        }
    ],
    "accessors": [
        {
            "bufferView": 0,
            "byteOffset": 0,
            "componentType": 5126,
            "count": 1,
            "type": "VEC2",
            "max": [100.0, 100.0],
            "min": [0.0, 0.0]
        }
    ]
}
)";

    static const char base64_json[] = R"(
{
    "asset":
    {
        "version": "2.0"
    },
    "buffers": [
        {
            "byteLength": 18,
            "uri": "data:application/octet-stream;base64,abcdagyhubcd+bzdtbcdab+d"
        }
    ],
    "bufferViews": [
        {
            "buffer": 0,
            "byteLength": 8,
            "byteOffset": 0
        },
        {
            "buffer": 0,
            "byteLength": 4,
            "byteOffset": 12
        }
    ],
    "images": [
        {
            "bufferView": 1,
            "mimeType": "image/jpeg"
        },
        {
            "bufferView": 0,
            "mimeType": "image/png"
        }
    ]
}
)";

    static const char sparse_json_uint8[] = R"(
{
    "asset":
    {
        "version": "2.0"
    },
    "buffers": [
        {
            "byteLength": 16,
            "uri": "buffer.bin"
        }
    ],
    "bufferViews": [
        {
            "buffer": 0,
            "byteLength": 4
        },
        {
            "buffer": 0,
            "byteLength": 2,
            "byteOffset": 4
        },
        {
            "buffer": 0,
            "byteLength": 10,
            "byteOffset": 6
        }
    ],
    "accessors": [
        {
            "bufferView": 2,
            "componentType": 5121,
            "count": 5,
            "type": "VEC2",
            "max": [100.0, 100.0],
            "min": [0.0, 0.0],
            "sparse": {
                "count": 2,
                "indices": {
                    "bufferView": 1,
                    "componentType": 5121
                },
                "values": {
                    "bufferView": 0,
                    "byteOffset": 0
                }
            }
        }
    ]
}
)";

    static const char sparse_json_uint16[] = R"(
{
    "asset":
    {
        "version": "2.0"
    },
    "buffers": [
        {
            "byteLength": 32,
            "uri": "buffer.bin"
        }
    ],
    "bufferViews": [
        {
            "buffer": 0,
            "byteLength": 8
        },
        {
            "buffer": 0,
            "byteLength": 4,
            "byteOffset": 8
        },
        {
            "buffer": 0,
            "byteLength": 20,
            "byteOffset": 12
        }
    ],
    "accessors": [
        {
            "bufferView": 2,
            "componentType": 5123,
            "count": 5,
            "type": "VEC2",
            "max": [100.0, 100.0],
            "min": [0.0, 0.0],
            "sparse": {
                "count": 2,
                "indices": {
                    "bufferView": 1,
                    "componentType": 5123
                },
                "values": {
                    "bufferView": 0
                }
            }
        }
    ]
}
)";

    static const char sparse_json_uint32[] = R"(
{
    "asset":
    {
        "version": "2.0"
    },
    "buffers": [
        {
            "byteLength": 64,
            "uri": "buffer.bin"
        }
    ],
    "bufferViews": [
        {
            "buffer": 0,
            "byteLength": 16
        },
        {
            "buffer": 0,
            "byteLength": 8,
            "byteOffset": 16
        },
        {
            "buffer": 0,
            "byteLength": 40,
            "byteOffset": 24
        }
    ],
    "accessors": [
        {
            "bufferView": 2,
            "componentType": 5125,
            "count": 5,
            "type": "VEC2",
            "max": [100.0, 100.0],
            "min": [0.0, 0.0],
            "sparse": {
                "count": 2,
                "indices": {
                    "bufferView": 1,
                    "componentType": 5125
                },
                "values": {
                    "bufferView": 0
                }
            }
        }
    ]
}
)";

    static const char sparse_json_float[] = R"(
{
    "asset":
    {
        "version": "2.0"
    },
    "buffers": [
        {
            "byteLength": 64,
            "uri": "buffer.bin"
        }
    ],
    "bufferViews": [
        {
            "buffer": 0,
            "byteLength": 16
        },
        {
            "buffer": 0,
            "byteLength": 8,
            "byteOffset": 16
        },
        {
            "buffer": 0,
            "byteLength": 40,
            "byteOffset": 24
        }
    ],
    "accessors": [
        {
            "bufferView": 2,
            "componentType": 5126,
            "count": 5,
            "type": "VEC2",
            "max": [100.0, 100.0],
            "min": [0.0, 0.0],
            "sparse": {
                "count": 2,
                "indices": {
                    "bufferView": 1,
                    "componentType": 5125
                },
                "values": {
                    "bufferView": 0
                }
            }
        }
    ]
}
)";

    static const char sparse_json_interleaved[] = R"(
{
    "asset":
    {
        "version": "2.0"
    },
    "buffers": [
        {
            "byteLength": 32,
            "uri": "buffer.bin"
        }
    ],
    "bufferViews": [
        {
            "buffer": 0,
            "byteLength": 8,
            "byteStride": 4
        },
        {
            "buffer": 0,
            "byteLength": 8,
            "byteOffset": 8,
            "byteStride": 4
        },
        {
            "buffer": 0,
            "byteLength": 16,
            "byteOffset": 16,
            "byteStride": 4
        }
    ],
    "accessors": [
        {
            "bufferView": 2,
            "componentType": 5121,
            "count": 4,
            "type": "VEC2",
            "max": [100.0, 100.0],
            "min": [0.0, 0.0],
            "sparse": {
                "count": 2,
                "indices": {
                    "bufferView": 1,
                    "componentType": 5121
                },
                "values": {
                    "bufferView": 0,
                    "byteOffset": 0
                }
            }
        }
    ]
}
)";

    static const char sparse_emptybufferview_json[] = R"(
{
    "asset":
    {
        "version": "2.0"
    },
    "buffers": [
        {
            "byteLength": 6,
            "uri": "buffer.bin"
        }
    ],
    "bufferViews": [
        {
            "buffer": 0,
            "byteLength": 4
        },
        {
            "buffer": 0,
            "byteLength": 2,
            "byteOffset": 4
        }
    ],
    "accessors": [
        {
            "componentType": 5121,
            "count": 5,
            "type": "VEC2",
            "max": [100.0, 100.0],
            "min": [0.0, 0.0],
            "sparse": {
                "count": 2,
                "indices": {
                    "bufferView": 1,
                    "componentType": 5121
                },
                "values": {
                    "bufferView": 0,
                    "byteOffset": 0
                }
            }
        }
    ]
}
)";
}

namespace Microsoft
{
    namespace  glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(GLTFResourceReaderTests)
            {
                GLTFSDK_TEST_METHOD(GLTFResourceReaderTests, TestReadBinaryDataAccessor)
                {
                    float f1 = 1.0f, f2 = 10.0f;

                    auto stream = std::make_shared<StreamReaderWriter>();
                    auto streamOutput = stream->GetOutputStream("buffer.bin");

                    streamOutput->write(reinterpret_cast<char*>(&f1), sizeof(f1));
                    streamOutput->write(reinterpret_cast<char*>(&f2), sizeof(f2));

                    Document gltfDoc = Deserialize(test_json);

                    auto gltfResourceReader = std::make_unique<GLTFResourceReader>(stream);

                    auto accessor = gltfDoc.accessors.Get("0");
                    auto accessorData = gltfResourceReader->ReadBinaryData<float>(gltfDoc, accessor);

                    Assert::AreEqual<size_t>(2U, accessorData.size());
                    Assert::AreEqual<float>(f1, accessorData[0]);
                    Assert::AreEqual<float>(f2, accessorData[1]);
                }

                GLTFSDK_TEST_METHOD(GLTFResourceReaderTests, TestReadBase64Image)
                {
                    auto stream = std::make_shared<StreamReaderWriter>();
                    Document gltfDoc = Deserialize(base64_json);
                    auto gltfResourceReader = std::make_unique<GLTFResourceReader>(stream);

                    auto img1 = gltfResourceReader->ReadBinaryData(gltfDoc, gltfDoc.images.Get("0"));
                    auto img2 = gltfResourceReader->ReadBinaryData(gltfDoc, gltfDoc.images.Get("1"));

                    Assert::IsTrue(img1 == std::vector<uint8_t>{181, 183, 29, 105});
                    Assert::IsTrue(img2 == std::vector<uint8_t>{105, 183, 29, 106, 12, 161, 185, 183});
                }

                GLTFSDK_TEST_METHOD(GLTFResourceReaderTests, TestReadSparseAccessorUint8)
                {
                    uint8_t inputBuffer[16] = { 3U, 3U, 3U, 3U, // the sparse values
                                                1U, 3U, // the sparse indices
                                                1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U }; // base bufferview

                    // expected sparse replacement output
                    std::vector<uint8_t> expectedReadOutput = { 1U, 1U, 3U, 3U, 1U, 1U, 3U, 3U, 1U, 1U };

                    auto stream = std::make_shared<StreamReaderWriter>();
                    auto streamOutput = stream->GetOutputStream("buffer.bin");

                    streamOutput->write(reinterpret_cast<char*>(&inputBuffer), 16);

                    Document gltfDoc = Deserialize(sparse_json_uint8);

                    auto gltfResourceReader = std::make_unique<GLTFResourceReader>(stream);

                    auto accessor = gltfDoc.accessors.Get("0");
                    auto output = gltfResourceReader->ReadBinaryData<uint8_t>(gltfDoc, accessor);

                    Assert::IsTrue(output == expectedReadOutput);
                }

                GLTFSDK_TEST_METHOD(GLTFResourceReaderTests, TestReadSparseAccessorUint16)
                {
                    uint16_t inputBuffer[16] = { 3U, 3U, 3U, 3U, // the sparse values
                                                 1U, 3U, // the sparse indices
                                                 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U }; // base bufferview

                    // expected sparse replacement output
                    std::vector<uint16_t> expectedReadOutput = { 1U, 1U, 3U, 3U, 1U, 1U, 3U, 3U, 1U, 1U };;

                    auto streamReaderWriter = std::make_shared<StreamReaderWriter>();
                    auto streamOutput = streamReaderWriter->GetOutputStream("buffer.bin");

                    streamOutput->write(reinterpret_cast<char*>(&inputBuffer), 32);

                    Document gltfDoc = Deserialize(sparse_json_uint16);

                    auto gltfResourceReader = std::make_unique<GLTFResourceReader>(streamReaderWriter);

                    auto accessor = gltfDoc.accessors.Get("0");
                    auto output = gltfResourceReader->ReadBinaryData<uint16_t>(gltfDoc, accessor);

                    Assert::IsTrue(output == expectedReadOutput);
                }

                GLTFSDK_TEST_METHOD(GLTFResourceReaderTests, TestReadSparseAccessorUint32)
                {
                    uint32_t inputBuffer[16] = { 3U, 3U, 3U, 3U, // the sparse values
                                                1U, 3U, // the sparse indices
                                                1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U }; // base bufferview

                    std::vector<uint32_t> expectedReadOutput = { 1U, 1U, 3U, 3U, 1U, 1U, 3U, 3U, 1U, 1U };;

                    auto stream = std::make_shared<StreamReaderWriter>();
                    auto streamOutput = stream->GetOutputStream("buffer.bin");

                    streamOutput->write(reinterpret_cast<char*>(&inputBuffer), 64);

                    Document gltfDoc = Deserialize(sparse_json_uint32);

                    auto gltfResourceReader = std::make_unique<GLTFResourceReader>(stream);

                    auto accessor = gltfDoc.accessors.Get("0");
                    auto output = gltfResourceReader->ReadBinaryData<uint32_t>(gltfDoc, accessor);

                    Assert::IsTrue(output == expectedReadOutput);
                }

                GLTFSDK_TEST_METHOD(GLTFResourceReaderTests, TestReadSparseAccessorFloat)
                {
                    auto stream = std::make_shared<StreamReaderWriter>();
                    auto streamOutput = stream->GetOutputStream("buffer.bin");

                    float valuesBuffer[4] = { 3.0, 3.0, 3.0, 3.0,}; // the sparse indices
                    streamOutput->write(reinterpret_cast<char*>(&valuesBuffer), 16);

                    uint32_t indicesBuffer[2] = {1U, 3U }; // the sparse indices
                    streamOutput->write(reinterpret_cast<char*>(&indicesBuffer), 8);

                    float floatInputBuffer[10] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };
                    streamOutput->write(reinterpret_cast<char*>(&floatInputBuffer), 40);

                    // expected sparse replacement output
                    std::vector<float> expectedReadOutput = { 1.0, 1.0, 3.0, 3.0, 1.0, 1.0, 3.0, 3.0, 1.0, 1.0 };

                    Document gltfDoc = Deserialize(sparse_json_float);

                    auto gltfResourceReader = std::make_unique<GLTFResourceReader>(stream);

                    auto accessor = gltfDoc.accessors.Get("0");
                    auto output = gltfResourceReader->ReadBinaryData<float>(gltfDoc, accessor);

                    Assert::IsTrue(output == expectedReadOutput);
                }

                GLTFSDK_TEST_METHOD(GLTFResourceReaderTests, TestReadSparseAccessorInterleaved)
                {
                    uint8_t inputBuffer[32] = { 3U, 3U, 0U, 0U, 3U, 3U, 0U, 0U,// the sparse values
                                                1U, 0U, 0U, 0U, 3U, 0U, 0U, 0U,// the sparse indices
                                                1U, 1U, 0U, 0U, 1U, 1U, 0U, 0U, 1U, 1U, 0U, 0U, 1U, 1U, 0U, 0U, }; // base bufferview

                    // expected sparse replacement output
                    std::vector<uint8_t> expectedReadOutput = { 1U, 1U, 3U, 3U, 1U, 1U, 3U, 3U};

                    auto stream = std::make_shared<StreamReaderWriter>();
                    auto streamOutput = stream->GetOutputStream("buffer.bin");

                    streamOutput->write(reinterpret_cast<char*>(&inputBuffer), 32);

                    Document gltfDoc = Deserialize(sparse_json_interleaved);

                    auto gltfResourceReader = std::make_unique<GLTFResourceReader>(stream);

                    auto accessor = gltfDoc.accessors.Get("0");
                    auto output = gltfResourceReader->ReadBinaryData<uint8_t>(gltfDoc, accessor);

                    Assert::IsTrue(output == expectedReadOutput);
                }

                GLTFSDK_TEST_METHOD(GLTFResourceReaderTests, TestReadSparseEmptyBufferViewAccessor)
                {
                    uint8_t inputBuffer[6] = { 3U, 3U, 0U, 1U, // the sparse values
                                               1U, 3U }; // the sparse indices

                    // expected sparse replacement output
                    std::vector<uint8_t> expectedReadOutput = { 0U, 0U, 3U, 3U, 0U, 0U, 0U, 1U, 0U, 0U };

                    auto stream = std::make_shared<StreamReaderWriter>();
                    auto streamOutput = stream->GetOutputStream("buffer.bin");

                    streamOutput->write(reinterpret_cast<char*>(&inputBuffer), 6);

                    Document gltfDoc = Deserialize(sparse_emptybufferview_json);

                    auto gltfResourceReader = std::make_unique<GLTFResourceReader>(stream);

                    auto accessor = gltfDoc.accessors.Get("0");
                    auto output = gltfResourceReader->ReadBinaryData<uint8_t>(gltfDoc, accessor);

                    Assert::IsTrue(output == expectedReadOutput);
                }

                GLTFSDK_TEST_METHOD(GLTFResourceReaderTests, TestReadFloatData)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<float> values = { 0.f, 1.f, -0.125f };
                    auto accessor = bufferBuilder.AddAccessor(values, { TYPE_SCALAR, COMPONENT_FLOAT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto data = reader.ReadFloatData(doc, accessor);

                    Assert::AreEqual<size_t>(3U, data.size());
                    Assert::AreEqual<float>(data[0], 0.f);
                    Assert::AreEqual<float>(data[1], 1.f);
                    Assert::AreEqual<float>(data[2], -0.125f);
                }

                GLTFSDK_TEST_METHOD(GLTFResourceReaderTests, TestReadFloatData_U8)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint8_t> values = { 0, 1, 255 };
                    auto accessor = bufferBuilder.AddAccessor(values, { TYPE_SCALAR, COMPONENT_UNSIGNED_BYTE });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto data = reader.ReadFloatData(doc, accessor);

                    Assert::AreEqual<size_t>(3U, data.size());
                    Assert::AreEqual<float>(data[0], 0.f);
                    Assert::AreEqual<float>(data[1], 1.f);
                    Assert::AreEqual<float>(data[2], 255.f);
                }

                GLTFSDK_TEST_METHOD(GLTFResourceReaderTests, TestReadFloatData_U8N)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint8_t> values = { 0, 1, 255 };
                    auto accessor = bufferBuilder.AddAccessor(values, { TYPE_SCALAR, COMPONENT_UNSIGNED_BYTE, true });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto data = reader.ReadFloatData(doc, accessor);

                    Assert::AreEqual<size_t>(3U, data.size());
                    Assert::AreEqual<float>(data[0], 0.f);
                    Assert::AreEqual<float>(data[1], 1.f / 255.f);
                    Assert::AreEqual<float>(data[2], 1.f);
                }

                GLTFSDK_TEST_METHOD(GLTFResourceReaderTests, TestReadFloatData_S8)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<int8_t> values = { 0, 1, -1, 127, -127, -128 };
                    auto accessor = bufferBuilder.AddAccessor(values, { TYPE_SCALAR, COMPONENT_BYTE });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto data = reader.ReadFloatData(doc, accessor);

                    Assert::AreEqual<size_t>(6U, data.size());
                    Assert::AreEqual<float>(data[0], 0.f);
                    Assert::AreEqual<float>(data[1], 1.f);
                    Assert::AreEqual<float>(data[2], -1.f);
                    Assert::AreEqual<float>(data[3], 127.f);
                    Assert::AreEqual<float>(data[4], -127.f);
                    Assert::AreEqual<float>(data[5], -128.f);
                }

                GLTFSDK_TEST_METHOD(GLTFResourceReaderTests, TestReadFloatData_S8N)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<int8_t> values = { 0, 1, -1, 127, -127, -128 };
                    auto accessor = bufferBuilder.AddAccessor(values, { TYPE_SCALAR, COMPONENT_BYTE, true });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto data = reader.ReadFloatData(doc, accessor);

                    Assert::AreEqual<size_t>(6U, data.size());
                    Assert::AreEqual<float>(data[0], 0.f);
                    Assert::AreEqual<float>(data[1], 1.f / 127.f);
                    Assert::AreEqual<float>(data[2], -1.f / 127.f);
                    Assert::AreEqual<float>(data[3], 1.f);
                    Assert::AreEqual<float>(data[4], -1.f);
                    Assert::AreEqual<float>(data[5], -1.f);
                }

                GLTFSDK_TEST_METHOD(GLTFResourceReaderTests, TestReadFloatData_U16)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint16_t> values = { 0, 1, 255, 65535 };
                    auto accessor = bufferBuilder.AddAccessor(values, { TYPE_SCALAR, COMPONENT_UNSIGNED_SHORT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto data = reader.ReadFloatData(doc, accessor);

                    Assert::AreEqual<size_t>(4U, data.size());
                    Assert::AreEqual<float>(data[0], 0.f);
                    Assert::AreEqual<float>(data[1], 1.f);
                    Assert::AreEqual<float>(data[2], 255.f);
                    Assert::AreEqual<float>(data[3], 65535.f);
                }

                GLTFSDK_TEST_METHOD(GLTFResourceReaderTests, TestReadFloatData_U16N)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint16_t> values = { 0, 1, 255, 65535 };
                    auto accessor = bufferBuilder.AddAccessor(values, { TYPE_SCALAR, COMPONENT_UNSIGNED_SHORT, true });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto data = reader.ReadFloatData(doc, accessor);

                    Assert::AreEqual<size_t>(4U, data.size());
                    Assert::AreEqual<float>(data[0], 0.f);
                    Assert::AreEqual<float>(data[1], 1.f / 65535.f);
                    Assert::AreEqual<float>(data[2], 255.f / 65535.f);
                    Assert::AreEqual<float>(data[3], 1.f);
                }

                GLTFSDK_TEST_METHOD(GLTFResourceReaderTests, TestReadFloatData_S16)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<int16_t> values = { 0, 1, -1, 32767, -32767, -32768 };
                    auto accessor = bufferBuilder.AddAccessor(values, { TYPE_SCALAR, COMPONENT_SHORT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto data = reader.ReadFloatData(doc, accessor);

                    Assert::AreEqual<size_t>(6U, data.size());
                    Assert::AreEqual<float>(data[0], 0.f);
                    Assert::AreEqual<float>(data[1], 1.f);
                    Assert::AreEqual<float>(data[2], -1.f);
                    Assert::AreEqual<float>(data[3], 32767.f);
                    Assert::AreEqual<float>(data[4], -32767.f);
                    Assert::AreEqual<float>(data[5], -32768.f);
                }

                GLTFSDK_TEST_METHOD(GLTFResourceReaderTests, TestReadFloatData_S16N)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<int16_t> values = { 0, 1, -1, 32767, -32767, -32768 };
                    auto accessor = bufferBuilder.AddAccessor(values, { TYPE_SCALAR, COMPONENT_SHORT, true });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto data = reader.ReadFloatData(doc, accessor);

                    Assert::AreEqual<size_t>(6U, data.size());
                    Assert::AreEqual<float>(data[0], 0.f);
                    Assert::AreEqual<float>(data[1], 1.f / 32767.f);
                    Assert::AreEqual<float>(data[2], -1.f / 32767.f);
                    Assert::AreEqual<float>(data[3], 1.f);
                    Assert::AreEqual<float>(data[4], -1.f);
                    Assert::AreEqual<float>(data[5], -1.f);
                }

                // Negative regression: an accessor whose declared element
                // count, when multiplied by its type component count, is not
                // representable as size_t must be rejected with a
                // GLTFException rather than silently allocating an
                // undersized buffer that the sparse-write loop would then
                // overrun.
                GLTFSDK_TEST_METHOD(GLTFResourceReaderTests, TestSparseAccessor_RejectsUnrepresentableElementCount)
                {
                    Accessor accessor;
                    accessor.id = "0";
                    accessor.componentType = COMPONENT_FLOAT;
                    accessor.type = TYPE_VEC3;
                    // VEC3 has typeCount = 3. Pick a count whose product
                    // with 3 overflows size_t (max()/3 + 1) * 3 wraps to a
                    // small number.
                    accessor.count = (std::numeric_limits<size_t>::max)() / 3U + 1U;
                    accessor.sparse.count = 1U;
                    accessor.sparse.indicesComponentType = COMPONENT_UNSIGNED_INT;
                    accessor.sparse.indicesBufferViewId = "0";
                    accessor.sparse.valuesBufferViewId = "1";

                    BufferView indicesView;
                    indicesView.id = "0";
                    indicesView.bufferId = "0";
                    indicesView.byteLength = sizeof(uint32_t);
                    BufferView valuesView;
                    valuesView.id = "1";
                    valuesView.bufferId = "0";
                    valuesView.byteOffset = sizeof(uint32_t);
                    valuesView.byteLength = 3U * sizeof(float);

                    Buffer buffer;
                    buffer.id = "0";
                    buffer.uri = "buffer.bin";
                    buffer.byteLength = indicesView.byteLength + valuesView.byteLength;

                    Document doc;
                    doc.buffers.Append(buffer);
                    doc.bufferViews.Append(indicesView);
                    doc.bufferViews.Append(valuesView);
                    doc.accessors.Append(accessor);

                    auto stream = std::make_shared<StreamReaderWriter>();
                    auto streamOutput = stream->GetOutputStream("buffer.bin");
                    uint32_t idx = 0U;
                    float values[3] = { 0.f, 0.f, 0.f };
                    streamOutput->write(reinterpret_cast<char*>(&idx), sizeof(idx));
                    streamOutput->write(reinterpret_cast<char*>(values), sizeof(values));

                    GLTFResourceReader reader(stream);
                    Assert::ExpectException<GLTFException>([&]()
                    {
                        reader.ReadBinaryData<float>(doc, accessor);
                    });
                }
            };
        }
    }
}
