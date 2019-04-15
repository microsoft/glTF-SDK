// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/BufferBuilder.h>
#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/GLTFResourceWriter.h>
#include <GLTFSDK/MeshPrimitiveUtils.h>
#include <GLTFSDK/Serialize.h>

#include "TestUtils.h"

#include <map>

using namespace glTF::UnitTest;

namespace
{
    using namespace Microsoft::glTF;

    template <typename T, size_t N>
    static constexpr size_t ArrayCount(T(&)[N]) { return N; }

    struct NullStreamBuf : std::streambuf
    {
        int overflow(int ch) override
        {
            ++m_size; return traits_type::not_eof(ch);
        }

        size_t m_size = {};
    };

    struct NullStream : std::ostream
    {
        NullStream() : std::ostream(&m_streamBuf) {}
        NullStreamBuf m_streamBuf;
    };

    struct TestStreamWriter : IStreamWriter
    {
        std::shared_ptr<std::ostream> GetOutputStream(const std::string& uri) const override
        {
            return m_streamMap.emplace(uri, std::make_shared<NullStream>()).first->second;
        }

        size_t GetBufferCount() const
        {
            return m_streamMap.size();
        }

        size_t GetBufferLength(size_t idx) const
        {
            auto mapIt = std::next(m_streamMap.begin(), idx);
            return mapIt->second->m_streamBuf.m_size;
        }

        const std::string& GetBufferUri(size_t idx) const
        {
            auto mapIt = std::next(m_streamMap.begin(), idx);

            return mapIt->first;
        }

        mutable std::map<std::string, std::shared_ptr<NullStream>> m_streamMap;
    };

    static const char expectedBufferBuilder[] =
R"({
    "asset": {
        "version": "2.0"
    },
    "accessors": [
        {
            "bufferView": 0,
            "componentType": 5123,
            "count": 3,
            "type": "SCALAR"
        },
        {
            "bufferView": 1,
            "componentType": 5126,
            "count": 3,
            "type": "VEC3"
        },
        {
            "bufferView": 1,
            "byteOffset": 36,
            "componentType": 5126,
            "count": 2,
            "type": "VEC2"
        }
    ],
    "bufferViews": [
        {
            "buffer": 0,
            "byteOffset": 0,
            "byteLength": 6,
            "target": 34963
        },
        {
            "buffer": 0,
            "byteOffset": 8,
            "byteLength": 52,
            "target": 34962
        }
    ],
    "buffers": [
        {
            "byteLength": 60,
            "uri": "0.bin"
        }
    ]
})";

    static const char expectedBufferBuilderMultiple[] =
R"({
    "asset": {
        "version": "2.0"
    },
    "bufferViews": [
        {
            "buffer": 0,
            "byteOffset": 0,
            "byteLength": 4
        },
        {
            "buffer": 1,
            "byteOffset": 0,
            "byteLength": 4
        }
    ],
    "buffers": [
        {
            "byteLength": 4,
            "uri": "0.bin"
        },
        {
            "byteLength": 4,
            "uri": "1.bin"
        }
    ]
})";

static const char expectedBufferBuilderMultipleAccessor[] =
R"({
    "asset": {
        "version": "2.0"
    },
    "accessors": [
        {
            "bufferView": 0,
            "componentType": 5121,
            "count": 6,
            "type": "SCALAR",
            "max": [
                3.0
            ],
            "min": [
                0.0
            ]
        },
        {
            "bufferView": 1,
            "componentType": 5126,
            "count": 4,
            "type": "VEC3",
            "max": [
                1.0,
                1.0,
                0.0
            ],
            "min": [
                -1.0,
                -1.0,
                0.0
            ]
        },
        {
            "bufferView": 1,
            "byteOffset": 12,
            "componentType": 5126,
            "count": 4,
            "type": "VEC3",
            "max": [
                0.0,
                0.0,
                -1.0
            ],
            "min": [
                0.0,
                0.0,
                -1.0
            ]
        },
        {
            "bufferView": 1,
            "byteOffset": 24,
            "componentType": 5126,
            "count": 4,
            "type": "VEC2",
            "max": [
                1.0,
                1.0
            ],
            "min": [
                0.0,
                0.0
            ]
        }
    ],
    "bufferViews": [
        {
            "buffer": 0,
            "byteOffset": 0,
            "byteLength": 6,
            "target": 34963
        },
        {
            "buffer": 0,
            "byteOffset": 8,
            "byteLength": 128,
            "byteStride": 32,
            "target": 34962
        }
    ],
    "buffers": [
        {
            "byteLength": 136,
            "uri": "0.bin"
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
            GLTFSDK_TEST_CLASS(GLTFResourceWriterTests)
            {
                GLTFSDK_TEST_METHOD(GLTFResourceWriterTests, WriteBufferView)
                {
                    auto streamWriter = std::make_shared<const TestStreamWriter>();
                    GLTFResourceWriter writer(streamWriter);

                    std::vector<uint32_t> data = { 0U, 1U, 2U, 3U };

                    BufferView bufferView;
                    bufferView.id = "0";
                    bufferView.bufferId = "0";
                    bufferView.byteOffset = 0;
                    bufferView.byteLength = data.size() * sizeof(uint32_t);

                    writer.Write(bufferView, data.data());

                    bufferView.id = "1";
                    bufferView.byteOffset = 16U;

                    writer.Write(bufferView, data.data());

                    Assert::AreEqual(static_cast<size_t>(1), streamWriter->GetBufferCount(), L"Unexpected number of buffers");
                    Assert::AreEqual(static_cast<size_t>(32), streamWriter->GetBufferLength(0U), L"Unexpected number of bytes written to buffer");
                    Assert::AreEqual("0.bin", streamWriter->GetBufferUri(0U).c_str(), L"Unexpected buffer uri");
                }

                GLTFSDK_TEST_METHOD(GLTFResourceWriterTests, WriteBufferViewWithOffset)
                {
                    auto streamWriter = std::make_shared<const TestStreamWriter>();
                    GLTFResourceWriter writer(streamWriter);

                    std::vector<uint32_t> data(4, 0);

                    BufferView bufferView;
                    bufferView.id = "0";
                    bufferView.bufferId = "0";
                    bufferView.byteOffset = 0;
                    bufferView.byteLength = data.size() * sizeof(uint32_t);

                    writer.Write(bufferView, data.data());

                    bufferView.id = "1";
                    bufferView.byteOffset = 16U + 8U;// Add an 8-byte offset so the GLTFResourceWriter must seek forward

                    writer.Write(bufferView, data.data());

                    Assert::AreEqual(static_cast<size_t>(1), streamWriter->GetBufferCount(), L"Unexpected number of buffers");
                    Assert::AreEqual(static_cast<size_t>(40), streamWriter->GetBufferLength(0U), L"Unexpected number of bytes written to buffer");
                    Assert::AreEqual("0.bin", streamWriter->GetBufferUri(0U).c_str(), L"Unexpected buffer uri");
                }

                GLTFSDK_TEST_METHOD(GLTFResourceWriterTests, WriteBufferViewInvalidOffset)
                {
                    auto streamWriter = std::make_shared<const TestStreamWriter>();
                    GLTFResourceWriter writer(streamWriter);

                    std::vector<uint32_t> data(4, 0);

                    BufferView bufferView;
                    bufferView.id = "0";
                    bufferView.bufferId = "0";
                    bufferView.byteOffset = 0;
                    bufferView.byteLength = data.size() * sizeof(uint32_t);

                    writer.Write(bufferView, data.data());

                    bufferView.id = "1";
                    bufferView.byteOffset = 0U;// Invalid offset- should be 16 (or greater)

                    Assert::ExpectException<InvalidGLTFException>([&]()
                    {
                        writer.Write(bufferView, data.data());
                    });
                }

                GLTFSDK_TEST_METHOD(GLTFResourceWriterTests, WriteBufferViewMultipleBuffers)
                {
                    auto streamWriter = std::make_shared<const TestStreamWriter>();
                    GLTFResourceWriter writer(streamWriter);

                    std::vector<uint32_t> data1(4, 0);
                    std::vector<uint32_t> data2(8, 0);

                    BufferView bufferView;
                    bufferView.id = "0";
                    bufferView.bufferId = "0";
                    bufferView.byteOffset = 0;
                    bufferView.byteLength = data1.size() * sizeof(uint32_t);

                    writer.Write(bufferView, data1.data());

                    bufferView.id = "1";
                    bufferView.bufferId = "1";
                    bufferView.byteLength = data2.size() * sizeof(uint32_t);

                    writer.Write(bufferView, data2.data());

                    Assert::AreEqual(static_cast<size_t>(2), streamWriter->GetBufferCount(), L"Unexpected number of buffers");
                    Assert::AreEqual(static_cast<size_t>(16), streamWriter->GetBufferLength(0U), L"Unexpected number of bytes written to buffer");
                    Assert::AreEqual(static_cast<size_t>(32), streamWriter->GetBufferLength(1U), L"Unexpected number of bytes written to buffer");
                    Assert::AreEqual("0.bin", streamWriter->GetBufferUri(0U).c_str(), L"Unexpected buffer uri");
                    Assert::AreEqual("1.bin", streamWriter->GetBufferUri(1U).c_str(), L"Unexpected buffer uri");
                }

                GLTFSDK_TEST_METHOD(GLTFResourceWriterTests, WriteAccessor)
                {
                    auto streamWriter = std::make_shared<const TestStreamWriter>();
                    GLTFResourceWriter writer(streamWriter);

                    std::vector<float> data(4, 0.0);

                    BufferView bufferView;
                    bufferView.id = "0";
                    bufferView.bufferId = "0";
                    bufferView.byteOffset = 0;
                    bufferView.byteLength = data.size() * sizeof(float);

                    Accessor accessor;
                    accessor.id = "0";
                    accessor.bufferViewId = "0";
                    accessor.byteOffset = 0;
                    accessor.componentType = ComponentType::COMPONENT_FLOAT;
                    accessor.type = AccessorType::TYPE_VEC4;
                    accessor.count = 1U;

                    writer.Write(bufferView, data.data(), accessor);

                    bufferView.id = "1";
                    bufferView.bufferId = "0";
                    bufferView.byteOffset = 16U;

                    accessor.id = "1";
                    accessor.bufferViewId = "1";

                    writer.Write(bufferView, data.data(), accessor);

                    Assert::AreEqual(static_cast<size_t>(1), streamWriter->GetBufferCount(), L"Unexpected number of buffers");
                    Assert::AreEqual(static_cast<size_t>(32), streamWriter->GetBufferLength(0U), L"Unexpected number of bytes written to buffer");
                    Assert::AreEqual("0.bin", streamWriter->GetBufferUri(0U).c_str(), L"Unexpected buffer uri");
                }

                GLTFSDK_TEST_METHOD(GLTFResourceWriterTests, WriteAccessorWithOffset)
                {
                    auto streamWriter = std::make_shared<const TestStreamWriter>();
                    GLTFResourceWriter writer(streamWriter);

                    std::vector<float> data(4, 0.0);

                    BufferView bufferView;
                    bufferView.id = "0";
                    bufferView.bufferId = "0";
                    bufferView.byteOffset = 0;
                    bufferView.byteLength = data.size() * sizeof(float);

                    Accessor accessor;
                    accessor.id = "0";
                    accessor.bufferViewId = "0";
                    accessor.byteOffset = 0;
                    accessor.componentType = ComponentType::COMPONENT_FLOAT;
                    accessor.type = AccessorType::TYPE_VEC2;
                    accessor.count = 1U;

                    writer.Write(bufferView, data.data(), accessor);

                    accessor.id = "1";
                    accessor.byteOffset = accessor.GetByteLength();// Offset the 2nd accessor by the size of the 1st

                    writer.Write(bufferView, data.data(), accessor);

                    Assert::AreEqual(static_cast<size_t>(1), streamWriter->GetBufferCount(), L"Unexpected number of buffers");
                    Assert::AreEqual(static_cast<size_t>(16), streamWriter->GetBufferLength(0U), L"Unexpected number of bytes written to buffer");
                    Assert::AreEqual("0.bin", streamWriter->GetBufferUri(0U).c_str(), L"Unexpected buffer uri");
                }

                GLTFSDK_TEST_METHOD(GLTFResourceWriterTests, WriteAccessorWithAlignment)
                {
                    auto streamWriter = std::make_shared<const TestStreamWriter>();
                    GLTFResourceWriter writer(streamWriter);

                    std::vector<uint8_t>  data1(3, 0);//  3 bytes - no alignment requirements
                    std::vector<uint16_t> data2(3, 0);//  6 bytes - must be 2-byte aligned -> 1 byte of padding needed  (2 - (3 % 2) == 1)
                    std::vector<uint32_t> data3(3, 0);// 12 bytes - must be 4-byte aligned -> 2 bytes of padding needed (4 - ((3 + 6 + 1) % 4) == 2)

                    BufferView bufferView;
                    bufferView.id = "0";
                    bufferView.bufferId = "0";
                    bufferView.byteOffset = 0;
                    bufferView.byteLength = data1.size() * sizeof(uint8_t);

                    Accessor accessor;
                    accessor.id = "0";
                    accessor.bufferViewId = "0";
                    accessor.byteOffset = 0;
                    accessor.componentType = ComponentType::COMPONENT_UNSIGNED_BYTE;
                    accessor.type = AccessorType::TYPE_VEC3;
                    accessor.count = 1U;

                    writer.Write(bufferView, data1.data(), accessor);

                    bufferView.id = "1";
                    bufferView.bufferId = "0";
                    bufferView.byteOffset = bufferView.byteOffset + bufferView.byteLength + 1U;// Add 1 byte of padding to ensure 2-byte alignment
                    bufferView.byteLength = data2.size() * sizeof(uint16_t);

                    accessor.id = "1";
                    accessor.bufferViewId = "1";
                    accessor.componentType = ComponentType::COMPONENT_UNSIGNED_SHORT;

                    writer.Write(bufferView, data2.data(), accessor);

                    bufferView.id = "2";
                    bufferView.bufferId = "0";
                    bufferView.byteOffset = bufferView.byteOffset + bufferView.byteLength + 2U;// Add 2 bytes of padding to ensure 4-byte alignment
                    bufferView.byteLength = data3.size() * sizeof(uint32_t);

                    accessor.id = "2";
                    accessor.bufferViewId = "2";
                    accessor.componentType = ComponentType::COMPONENT_UNSIGNED_INT;

                    writer.Write(bufferView, data3.data(), accessor);

                    Assert::AreEqual(static_cast<size_t>(1), streamWriter->GetBufferCount(), L"Unexpected number of buffers");
                    Assert::AreEqual(static_cast<size_t>(24), streamWriter->GetBufferLength(0U), L"Unexpected number of bytes written to buffer");
                    Assert::AreEqual("0.bin", streamWriter->GetBufferUri(0U).c_str(), L"Unexpected buffer uri");
                }

                GLTFSDK_TEST_METHOD(GLTFResourceWriterTests, WriteAccessorWithOffsetAndAlignment)
                {
                    auto streamWriter = std::make_shared<const TestStreamWriter>();
                    GLTFResourceWriter writer(streamWriter);

                    std::vector<uint8_t>  data1(2, 0);//  2 bytes - no alignment requirements
                    std::vector<uint32_t> data2(4, 0);// 16 bytes - must be 4-byte aligned -> 2 bytes of padding needed (4 - (2 % 4) == 2)
                    std::vector<uint32_t> data3(4, 0);// 16 bytes - must be 4-byte aligned -> 0 bytes of padding needed (4 - ((2 + 16 + 2) % 4) == 0)

                    BufferView bufferView;
                    bufferView.id = "0";
                    bufferView.bufferId = "0";
                    bufferView.byteOffset = 0;
                    bufferView.byteLength = data1.size() * sizeof(uint8_t);

                    Accessor accessor;
                    accessor.id = "0";
                    accessor.bufferViewId = "0";
                    accessor.byteOffset = 0;
                    accessor.componentType = ComponentType::COMPONENT_UNSIGNED_BYTE;
                    accessor.type = AccessorType::TYPE_VEC2;
                    accessor.count = 1U;

                    writer.Write(bufferView, data1.data(), accessor);

                    bufferView.id = "1";
                    bufferView.bufferId = "0";
                    bufferView.byteOffset = bufferView.byteOffset + bufferView.byteLength + 2U;// Add 2 bytes of padding to ensure 4-byte alignment
                    bufferView.byteLength = (data2.size() * sizeof(uint32_t)) + (data3.size() * sizeof(uint32_t));// Pack accessors '1' & '2' into buffer view '1'

                    accessor.id = "1";
                    accessor.bufferViewId = "1";
                    accessor.componentType = ComponentType::COMPONENT_UNSIGNED_INT;
                    accessor.type = AccessorType::TYPE_VEC4;

                    writer.Write(bufferView, data2.data(), accessor);

                    accessor.id = "2";
                    accessor.bufferViewId = "1";
                    accessor.byteOffset = accessor.GetByteLength();// Offset the 2nd accessor by the size of the 1st

                    writer.Write(bufferView, data3.data(), accessor);

                    Assert::AreEqual(static_cast<size_t>(1), streamWriter->GetBufferCount(), L"Unexpected number of buffers");
                    Assert::AreEqual(static_cast<size_t>(36), streamWriter->GetBufferLength(0U), L"Unexpected number of bytes written to buffer");
                    Assert::AreEqual("0.bin", streamWriter->GetBufferUri(0U).c_str(), L"Unexpected buffer uri");
                }

                GLTFSDK_TEST_METHOD(GLTFResourceWriterTests, WriteAccessorInvalidOffset)
                {
                    auto streamWriter = std::make_shared<const TestStreamWriter>();
                    GLTFResourceWriter writer(streamWriter);

                    std::vector<uint32_t> data(4, 0);

                    BufferView bufferView;
                    bufferView.id = "0";
                    bufferView.bufferId = "0";
                    bufferView.byteOffset = 0;
                    bufferView.byteLength = data.size() * sizeof(uint32_t) + 1U;// Add an additional byte as the accessor's byteOffset is 1;

                    Accessor accessor;
                    accessor.id = "0";
                    accessor.bufferViewId = "0";
                    accessor.byteOffset = 1U;
                    accessor.componentType = ComponentType::COMPONENT_UNSIGNED_INT;
                    accessor.type = AccessorType::TYPE_SCALAR;
                    accessor.count = data.size();

                    Assert::ExpectException<InvalidGLTFException>([&]()
                    {
                        writer.Write(bufferView, data.data(), accessor);
                    });
                }

                GLTFSDK_TEST_METHOD(GLTFResourceWriterTests, WriteAccessorInvalidTotalOffset)
                {
                    auto streamWriter = std::make_shared<const TestStreamWriter>();
                    GLTFResourceWriter writer(streamWriter);

                    std::vector<uint32_t> data(4, 0);

                    BufferView bufferView;
                    bufferView.id = "0";
                    bufferView.bufferId = "0";
                    bufferView.byteOffset = 1U;
                    bufferView.byteLength = data.size() * sizeof(uint32_t) + 5U;// Add an additional 5 bytes as the bufferView and accessor's byteOffsets are 1 and 4 respectively;

                    Accessor accessor;
                    accessor.id = "0";
                    accessor.bufferViewId = "0";
                    accessor.byteOffset = sizeof(uint32_t);
                    accessor.componentType = ComponentType::COMPONENT_UNSIGNED_INT;
                    accessor.type = AccessorType::TYPE_SCALAR;
                    accessor.count = data.size();

                    Assert::ExpectException<InvalidGLTFException>([&]()
                    {
                        writer.Write(bufferView, data.data(), accessor);
                    });
                }

                GLTFSDK_TEST_METHOD(GLTFResourceWriterTests, BufferBuilderMultiple)
                {
                    Document gltfDocument;

                    {
                        std::vector<char> data(4, '!');

                        BufferBuilder bufferBuilder(std::make_unique<GLTFResourceWriter>(std::make_unique<TestStreamWriter>()),
                            [&gltfDocument](const BufferBuilder& builder) { return std::to_string(gltfDocument.buffers.Size() + builder.GetBufferCount()); },
                            [&gltfDocument](const BufferBuilder& builder) { return std::to_string(gltfDocument.bufferViews.Size() + builder.GetBufferViewCount()); },
                            [&gltfDocument](const BufferBuilder& builder) { return std::to_string(gltfDocument.accessors.Size() + builder.GetAccessorCount()); });

                        bufferBuilder.AddBuffer();
                        bufferBuilder.AddBufferView(data);
                        bufferBuilder.Output(gltfDocument);
                    }

                    {
                        std::vector<char> data(4, '?');

                        BufferBuilder bufferBuilder(std::make_unique<GLTFResourceWriter>(std::make_unique<TestStreamWriter>()),
                            [&gltfDocument](const BufferBuilder& builder) { return std::to_string(gltfDocument.buffers.Size() + builder.GetBufferCount()); },
                            [&gltfDocument](const BufferBuilder& builder) { return std::to_string(gltfDocument.bufferViews.Size() + builder.GetBufferViewCount()); },
                            [&gltfDocument](const BufferBuilder& builder) { return std::to_string(gltfDocument.accessors.Size() + builder.GetAccessorCount()); });

                        bufferBuilder.AddBuffer();
                        bufferBuilder.AddBufferView(data);
                        bufferBuilder.Output(gltfDocument);
                    }

                    const std::string gltfManifest = Serialize(gltfDocument, SerializeFlags::Pretty);

                    Assert::AreEqual(expectedBufferBuilderMultiple, gltfManifest.c_str());
                }

                GLTFSDK_TEST_METHOD(GLTFResourceWriterTests, BufferBuilderAccessor)
                {
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(std::make_shared<const StreamReaderWriter>()));

                    std::vector<uint16_t> indices = { 0, 1, 2 };

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ELEMENT_ARRAY_BUFFER);

                    bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_SHORT });

                    std::vector<float> positions = { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f };
                    std::vector<float> texCoords = { 0.0f, 0.0f, 1.0f, 1.0f };

                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    bufferBuilder.AddAccessor(positions.data(), positions.size() / 3U, { TYPE_VEC3, COMPONENT_FLOAT });
                    bufferBuilder.AddAccessor(texCoords.data(), texCoords.size() / 2U, { TYPE_VEC2, COMPONENT_FLOAT });

                    Document gltfDocument;
                    bufferBuilder.Output(gltfDocument);

                    const std::string gltfManifest = Serialize(gltfDocument, SerializeFlags::Pretty);

                    Assert::AreEqual(expectedBufferBuilder, gltfManifest.c_str());
                }

                GLTFSDK_TEST_METHOD(GLTFResourceWriterTests, BufferBuilderAccessorUriPrefix)
                {
                    auto resourceWriter = std::make_unique<GLTFResourceWriter>(std::make_unique<TestStreamWriter>());
                    resourceWriter->SetUriPrefix("foo");
                    BufferBuilder bufferBuilder(std::move(resourceWriter));

                    std::vector<uint16_t> indices = { 0, 1, 2 };

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ELEMENT_ARRAY_BUFFER);

                    bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_SHORT });

                    std::vector<float> positions = { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f };
                    std::vector<float> texCoords = { 0.0f, 0.0f, 1.0f, 1.0f };

                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    bufferBuilder.AddAccessor(positions.data(), positions.size() / 3U, { TYPE_VEC3, COMPONENT_FLOAT });
                    bufferBuilder.AddAccessor(texCoords.data(), texCoords.size() / 2U, { TYPE_VEC2, COMPONENT_FLOAT });

                    Document gltfDocument;
                    bufferBuilder.Output(gltfDocument);

                    Assert::AreEqual(gltfDocument.buffers[0].uri.c_str(), "foo0.bin");
                }

                GLTFSDK_TEST_METHOD(GLTFResourceWriterTests, InvalidMaxMinBufferBuilderAccessor)
                {
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(std::make_shared<const StreamReaderWriter>()));

                    BufferView bufferView;
                    bufferView.id = "0";
                    bufferView.bufferId = "0";

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ELEMENT_ARRAY_BUFFER);

                    std::vector<float> keyframeTimes = { 0.0f , 0.0f, 0.0f, 0.0f, 0.0f };
                    std::vector<float> minValues = { 0.0f, 0.0f, 0.0f, 0.0f };
                    std::vector<float> maxValues = { 0.0f, 0.0f, 0.0f };

                    Assert::ExpectException<InvalidGLTFException>([&]()
                    {
                        bufferBuilder.AddAccessor(keyframeTimes, { TYPE_SCALAR, COMPONENT_FLOAT, false, minValues, maxValues });
                    });
                }

                GLTFSDK_TEST_METHOD(GLTFResourceWriterTests, BufferBuilderMultipleAccessor)
                {
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(std::make_shared<const StreamReaderWriter>()));

                    std::vector<uint8_t> indices ={ 0, 1, 2, 3, 2, 1 };

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ELEMENT_ARRAY_BUFFER);

                    bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_BYTE, false, { 0 }, { 3 } });

                    std::vector<float> vertices ={
                        -1.0f, 1.0f, 0.0f, // pos0
                        0.0f, 0.0f, -1.0f, // norm0
                        0.0f, 0.0f,        // uv0

                        1.0f, 1.0f, 0.0f,  // pos1
                        0.0f, 0.0f, -1.0f, // norm1
                        1.0f, 0.0f,        // uv1

                        -1.0f, -1.0f, 0.0f, // pos2
                        0.0f, 0.0f, -1.0f,  // norm2
                        0.0f, 1.0f,         // uv2

                        1.0f, -1.0f, 0.0f, // pos3
                        0.0f, 0.0f, -1.0f, // norm3
                        1.0f, 1.0f,        // uv3 
                    };

                    const size_t componentSize = sizeof(decltype(vertices)::value_type);
                    const size_t stride = (3 + 3 + 2) * componentSize;
                    const size_t count = vertices.size() * componentSize / stride;

                    AccessorDesc descs[3] = 
                    {
                        { TYPE_VEC3, COMPONENT_FLOAT, false, { -1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f }, 0 },
                        { TYPE_VEC3, COMPONENT_FLOAT, false, { 0.0f, 0.0f, -1.0f },{ 0.0f, 0.0f, -1.0f }, 12 },
                        { TYPE_VEC2, COMPONENT_FLOAT, false, { 0.0f, 0.0f },{ 1.0f, 1.0f }, 24 },
                    };

                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);
                    bufferBuilder.AddAccessors(vertices.data(), count, stride, descs, ArrayCount(descs), nullptr);

                    Document gltfDocument;
                    bufferBuilder.Output(gltfDocument);

                    const std::string gltfManifest = Serialize(gltfDocument, SerializeFlags::Pretty);

                    Assert::AreEqual(expectedBufferBuilderMultipleAccessor, gltfManifest.c_str());
                }

                GLTFSDK_TEST_METHOD(GLTFResourceWriterTests, BufferBuilderSharedReadWriter)
                {
                    auto streamReaderWriter = std::make_shared<const StreamReaderWriter>();

                    const std::string filename = "foo.gltf";

                    {
                        BufferBuilder bufferBuilder(std::make_unique<GLTFResourceWriter>(streamReaderWriter));
                        bufferBuilder.AddBuffer();

                        bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                        std::vector<uint8_t> indices = { 0, 1, 2, 3, 4, 5, 6, UINT8_MAX };
                        auto accessor = bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_BYTE });

                        Document doc;
                        bufferBuilder.Output(doc);
                        
                        const auto gltfManifest = Serialize(doc);
                        bufferBuilder.GetResourceWriter().WriteExternal(filename, gltfManifest.c_str(), gltfManifest.length());
                    }

                    {
                        auto inputStream = std::dynamic_pointer_cast<std::stringstream>(streamReaderWriter->GetInputStream(filename));

                        auto resourceReader = GLTFResourceReader(streamReaderWriter);
                        auto doc = Deserialize(inputStream->str());

                        auto output = MeshPrimitiveUtils::GetIndices16(doc, resourceReader, doc.accessors[0]);

                        std::vector<uint16_t> expected = { 0, 1, 2, 3, 4, 5, 6, UINT8_MAX };
                        AreEqual(expected, output);
                    }
                }
            };
        }
    }
}
