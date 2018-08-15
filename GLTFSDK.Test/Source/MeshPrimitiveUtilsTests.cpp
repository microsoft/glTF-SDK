// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/BufferBuilder.h>
#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLTFResourceReader.h>
#include <GLTFSDK/GLTFResourceWriter.h>
#include <GLTFSDK/IStreamWriter.h>
#include <GLTFSDK/MeshPrimitiveUtils.h>

#include "TestUtils.h"

using namespace glTF::UnitTest;

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(MeshPrimitiveUtilsTests)
            {
                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetIndices16_UnsignedByte)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint8_t> indices = { 0, 1, 2, 3, 4, 5, 6, UINT8_MAX };
                    auto accessor = bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_BYTE });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetIndices16(doc, reader, accessor);

                    std::vector<uint16_t> expected = { 0, 1, 2, 3, 4, 5, 6, UINT8_MAX };
                    AreEqual(expected, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetIndices16_UnsignedShort)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint16_t> indices = { 0, 1, 2, 3, 4, 5, UINT8_MAX, UINT16_MAX };
                    auto accessor = bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_SHORT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetIndices16(doc, reader, accessor);

                    std::vector<uint16_t> expected = { 0, 1, 2, 3, 4, 5, UINT8_MAX, UINT16_MAX };
                    AreEqual(expected, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetIndices16_UnsignedInt)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint32_t> indices = { 0, 1, 2, 3, 4, UINT8_MAX, UINT16_MAX, UINT32_MAX};
                    auto accessor = bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_INT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    Assert::ExpectException<GLTFException>([&doc, &reader, &accessor]()
                    {
                        MeshPrimitiveUtils::GetIndices16(doc, reader, accessor);
                    });
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetIndices32_UnsignedByte)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint8_t> indices = { 0, 1, 2, 3, 4, 5, 6, UINT8_MAX };
                    auto accessor = bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_BYTE });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetIndices32(doc, reader, accessor);

                    std::vector<uint32_t> expected = { 0, 1, 2, 3, 4, 5, 6, UINT8_MAX };
                    AreEqual(expected, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetIndices32_UnsignedShort)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint16_t> indices = { 0, 1, 2, 3, 4, 5, UINT8_MAX, UINT16_MAX };
                    auto accessor = bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_SHORT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetIndices32(doc, reader, accessor);

                    std::vector<uint32_t> expected = { 0, 1, 2, 3, 4, 5, UINT8_MAX, UINT16_MAX };
                    AreEqual(expected, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetIndices32_UnsignedInt)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint32_t> indices = { 0, 1, 2, 3, 4, UINT8_MAX, UINT16_MAX, UINT32_MAX };
                    auto accessor = bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_INT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetIndices32(doc, reader, accessor);

                    std::vector<uint32_t> expected = { 0, 1, 2, 3, 4, UINT8_MAX, UINT16_MAX, UINT32_MAX };
                    AreEqual(expected, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetPositions_Vec3_Float)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<float> positions = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f };
                    auto accessor = bufferBuilder.AddAccessor(positions, { TYPE_VEC3, COMPONENT_FLOAT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetPositions(doc, reader, accessor);

                    AreEqual(positions, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetMorphPositions_Vec3_Float)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<float> positions = {
                        0.1f, 0.2f, 0.3f,
                        0.4f, 0.5f, 0.6f
                    };
                    auto accessor = bufferBuilder.AddAccessor(positions, { TYPE_VEC3, COMPONENT_FLOAT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    MorphTarget target;
                    target.positionsAccessorId = accessor.id;
                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetPositions(doc, reader, target);

                    AreEqual(positions, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetNormals_Vec3_Float)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<float> normals = {
                        0.1f, 0.2f, 0.3f,
                        0.4f, 0.5f, 0.6f,
                        0.7f, 0.8f, 0.9f
                    };
                    auto accessor = bufferBuilder.AddAccessor(normals, { TYPE_VEC3, COMPONENT_FLOAT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetNormals(doc, reader, accessor);

                    AreEqual(normals, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetMorphNormals_Vec3_Float)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<float> normals = {
                        0.1f, 0.2f, 0.3f,
                        0.4f, 0.5f, 0.6f
                    };
                    auto accessor = bufferBuilder.AddAccessor(normals, { TYPE_VEC3, COMPONENT_FLOAT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    MorphTarget target;
                    target.normalsAccessorId = accessor.id;
                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetNormals(doc, reader, target);

                    AreEqual(normals, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetTangents_Vec4_Float)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<float> tangents = {
                        0.1f, 0.2f, 0.3f, 0.4f,
                        0.5f, 0.6f, 0.7f, 0.8f
                    };
                    auto accessor = bufferBuilder.AddAccessor(tangents, { TYPE_VEC4, COMPONENT_FLOAT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetTangents(doc, reader, accessor);

                    AreEqual(tangents, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetMorphTangents_Vec3_Float)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    // Morph tangents have no w component so are VEC3
                    std::vector<float> tangents = {
                        0.1f, 0.2f, 0.3f,
                        0.4f, 0.5f, 0.6f
                    };
                    auto accessor = bufferBuilder.AddAccessor(tangents, { TYPE_VEC3, COMPONENT_FLOAT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    MorphTarget target;
                    target.tangentsAccessorId = accessor.id;
                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetTangents(doc, reader, target);

                    AreEqual(tangents, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetTexcoords_Vec2_Float)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<float> texcoords = {
                        0.1f, 0.2f,
                        0.3f, 0.4f,
                        0.5f, 0.6f,
                        0.7f, 0.8f
                    };
                    auto accessor = bufferBuilder.AddAccessor(texcoords, { TYPE_VEC2, COMPONENT_FLOAT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetTexCoords(doc, reader, accessor);

                    AreEqual(texcoords, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetTexcoords_Vec2_Unsigned_Byte)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint8_t> texcoords = {
                        25, 50,
                        75, 100,
                        125, 150,
                        175, 200
                    };
                    auto accessor = bufferBuilder.AddAccessor(texcoords, { TYPE_VEC2, COMPONENT_UNSIGNED_BYTE, true });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetTexCoords(doc, reader, accessor);

                    std::vector<float> expected = {
                        0.0980392173f,
                        0.196078435f,
                        0.294117659f,
                        0.392156869f,
                        0.490196079f,
                        0.588235319f,
                        0.686274529f,
                        0.784313738f
                    };
                    AreEqual(expected, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetTexcoords_Vec2_Unsigned_Short)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint16_t> texcoords = {
                        6500, 13000,
                        19500, 26000,
                        32500, 39000,
                        45500, 52000
                    };
                    auto accessor = bufferBuilder.AddAccessor(texcoords, { TYPE_VEC2, COMPONENT_UNSIGNED_SHORT, true });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetTexCoords(doc, reader, accessor);

                    std::vector<float> expected = {
                        0.0991836414f,
                        0.198367283f,
                        0.297550917f,
                        0.396734565f,
                        0.495918214f,
                        0.595101833f,
                        0.694285512f,
                        0.793469131f
                    };
                    AreEqual(expected, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetColors_Vec3_Float)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<float> colors = {
                        0.1f, 0.2f, 0.3f,
                        0.4f, 0.5f, 0.6f,
                        0.7f, 0.8f, 0.9f
                    };
                    auto accessor = bufferBuilder.AddAccessor(colors, { TYPE_VEC3, COMPONENT_FLOAT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetColors(doc, reader, accessor);

                    std::vector<uint32_t> expected = {
                        4283249434,
                        4288249958,
                        4293315763
                    };
                    AreEqual(expected, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetColors_Vec4_Float)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<float> colors = {
                        0.1f, 0.2f, 0.3f, 1.0f,
                        0.4f, 0.5f, 0.6f, 1.0f,
                        0.7f, 0.8f, 0.9f, 1.0f
                    };
                    auto accessor = bufferBuilder.AddAccessor(colors, { TYPE_VEC4, COMPONENT_FLOAT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetColors(doc, reader, accessor);

                    std::vector<uint32_t> expected = {
                        4283249434,
                        4288249958,
                        4293315763
                    };
                    AreEqual(expected, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetColors_Vec3_Unsigned_Byte)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint8_t> colors = {
                        25, 50, 75,
                        100, 125, 150,
                        175, 200, 225
                    };
                    auto accessor = bufferBuilder.AddAccessor(colors, { TYPE_VEC3, COMPONENT_UNSIGNED_BYTE, true });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetColors(doc, reader, accessor);

                    std::vector<uint32_t> expected = {
                        4283118105,
                        4288052580,
                        4292987055
                    };
                    AreEqual(expected, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetColors_Vec4_Unsigned_Byte)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint8_t> colors = {
                        25, 50, 75, 255,
                        100, 125, 150, 255,
                        175, 200, 225, 255
                    };
                    auto accessor = bufferBuilder.AddAccessor(colors, { TYPE_VEC4, COMPONENT_UNSIGNED_BYTE, true });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetColors(doc, reader, accessor);

                    std::vector<uint32_t> expected = {
                        4283118105,
                        4288052580,
                        4292987055
                    };
                    AreEqual(expected, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetColors_Vec3_Unsigned_Short)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint16_t> colors = {
                        6500, 13000, 19500,
                        26000, 32500, 39000,
                        45500, 52000, 58500
                    };
                    auto accessor = bufferBuilder.AddAccessor(colors, { TYPE_VEC3, COMPONENT_UNSIGNED_SHORT, true });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetColors(doc, reader, accessor);

                    std::vector<uint32_t> expected = {
                        4283183897,
                        4288183909,
                        4293184177
                    };
                    AreEqual(expected, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetColors_Vec4_Unsigned_Short)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint16_t> colors = {
                        6500, 13000, 19500, 65535,
                        26000, 32500, 39000, 65535,
                        45500, 52000, 58500, 65535
                    };
                    auto accessor = bufferBuilder.AddAccessor(colors, { TYPE_VEC4, COMPONENT_UNSIGNED_SHORT, true });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetColors(doc, reader, accessor);

                    std::vector<uint32_t> expected = {
                        4283183897,
                        4288183909,
                        4293184177
                    };
                    AreEqual(expected, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetJointIndices32_Vec4_Unsigned_Byte)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint8_t> indices = {
                        0, 15, 0, 0,
                        15, 0, 20, 0,
                    };
                    auto accessor = bufferBuilder.AddAccessor(indices, { TYPE_VEC4, COMPONENT_UNSIGNED_BYTE });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetJointIndices32(doc, reader, accessor);

                    std::vector<uint32_t> expected = {
                        3840,
                        1310735
                    };
                    AreEqual(expected, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetJointIndices32_Vec4_Unsigned_Short)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint16_t> indices = {
                        0, 65535, 0, 0,
                        15, 0, 20, 0
                    };
                    auto accessor = bufferBuilder.AddAccessor(indices, { TYPE_VEC4, COMPONENT_UNSIGNED_SHORT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    Assert::ExpectException<GLTFException>([&doc, &reader, &accessor]()
                    {
                        MeshPrimitiveUtils::GetJointIndices32(doc, reader, accessor);
                    });
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetJointIndices64_Vec4_Unsigned_Byte)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint8_t> indices = {
                        0, 15, 0, 0,
                        15, 0, 20, 0,
                    };
                    auto accessor = bufferBuilder.AddAccessor(indices, { TYPE_VEC4, COMPONENT_UNSIGNED_BYTE });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetJointIndices64(doc, reader, accessor);

                    std::vector<uint64_t> expected = {
                        983040,
                        85899345935
                    };
                    AreEqual(expected, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetJointIndices64_Vec4_Unsigned_Short)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint16_t> indices = {
                        0, 65535, 0, 0,
                        15, 0, 20, 0
                    };
                    auto accessor = bufferBuilder.AddAccessor(indices, { TYPE_VEC4, COMPONENT_UNSIGNED_SHORT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetJointIndices64(doc, reader, accessor);

                    std::vector<uint64_t> expected = {
                        4294901760,
                        85899345935
                    };
                    AreEqual(expected, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetJointWeights32_Vec4_Float)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<float> weights = {
                        1.0f,          0.0f,           0.0f,          0.0f,
                        0.9254902005f, 0.7294117808f,  0.4980392158f, 0.003921568859f,
                        0.4941176474f, 0.3529411852f,  0.1529411823f, 0.0f,
                        0.9254902005f, 0.07450980693f, 0.0f,          0.0f
                    };
                    auto accessor = bufferBuilder.AddAccessor(weights, { TYPE_VEC4, COMPONENT_FLOAT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetJointWeights32(doc, reader, accessor);

                    std::vector<uint32_t> expected = {
                        255,
                        25148140,
                        2579070,
                        5100
                    };
                    AreEqual(expected, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetJointWeights32_Vec4_Unsigned_Byte)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint8_t> weights = {
                        255, 0, 0, 0,
                        236, 186, 127, 1,
                        126, 90, 39, 0,
                        236, 19, 0, 0
                    };
                    auto accessor = bufferBuilder.AddAccessor(weights, { TYPE_VEC4, COMPONENT_UNSIGNED_BYTE, true });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetJointWeights32(doc, reader, accessor);

                    std::vector<uint32_t> expected = {
                        255,
                        25148140,
                        2579070,
                        5100
                    };
                    AreEqual(expected, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetJointWeights32_Vec4_Unsigned_Short)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint16_t> weights = {
                        65535,     0,     0,     0,
                        60652, 47802, 32639,   257,
                        32382, 23130, 10023,     0,
                        60652,  4883,     0,     0
                    };
                    auto accessor = bufferBuilder.AddAccessor(weights, { TYPE_VEC4, COMPONENT_UNSIGNED_SHORT, true });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = MeshPrimitiveUtils::GetJointWeights32(doc, reader, accessor);

                    std::vector<uint32_t> expected = {
                        255,
                        25148140,
                        2579070,
                        5100
                    };
                    AreEqual(expected, output);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetTriangulatedIndices16_TriangleStrip_NoIndices)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<float> positions = {
                        0.0f, 0.0f, 0.0f,
                        1.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f,
                        1.0f, 1.0f, 0.0f,
                        2.0f, 0.0f, 0.0f
                    };
                    auto accessor = bufferBuilder.AddAccessor(positions, { TYPE_VEC3, COMPONENT_FLOAT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    MeshPrimitive meshPrimitive;
                    meshPrimitive.attributes[ACCESSOR_POSITION] = accessor.id;
                    meshPrimitive.mode = MESH_TRIANGLE_STRIP;

                    GLTFResourceReader reader(readerWriter);

                    auto outputPositions = MeshPrimitiveUtils::GetPositions(doc, reader, meshPrimitive);
                    AreEqual(positions, outputPositions);

                    std::vector<uint16_t> indices =
                    {
                        0, 1, 2,
                        1, 3, 2,
                        2, 3, 4
                    };

                    auto outputIndices = MeshPrimitiveUtils::GetTriangulatedIndices16(doc, reader, meshPrimitive);
                    AreEqual(indices, outputIndices);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetTriangulatedIndices16_TriangleFan_NoIndices)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<float> positions = {
                        0.0f, 0.0f, 0.0f,
                        1.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f,
                        1.0f, 1.0f, 0.0f,
                        2.0f, 0.0f, 0.0f
                    };
                    auto accessor = bufferBuilder.AddAccessor(positions, { TYPE_VEC3, COMPONENT_FLOAT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    MeshPrimitive meshPrimitive;
                    meshPrimitive.attributes[ACCESSOR_POSITION] = accessor.id;
                    meshPrimitive.mode = MESH_TRIANGLE_FAN;

                    GLTFResourceReader reader(readerWriter);

                    auto outputPositions = MeshPrimitiveUtils::GetPositions(doc, reader, meshPrimitive);
                    AreEqual(positions, outputPositions);

                    std::vector<uint16_t> indices =
                    {
                        0, 1, 2,
                        0, 2, 3,
                        0, 3, 4
                    };

                    auto outputIndices = MeshPrimitiveUtils::GetTriangulatedIndices16(doc, reader, meshPrimitive);
                    AreEqual(indices, outputIndices);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetTriangulatedIndices16_TriangleStrip_Indices)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<float> positions = {
                        0.0f, 0.0f, 0.0f,
                        1.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f,
                        1.0f, 1.0f, 0.0f
                    };
                    auto positionsAccessor = bufferBuilder.AddAccessor(positions, { TYPE_VEC3, COMPONENT_FLOAT });

                    std::vector<uint16_t> indices = {
                        0U, 3U, 1U, 2U
                    };
                    auto indicesAccessor = bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_SHORT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    MeshPrimitive meshPrimitive;
                    meshPrimitive.indicesAccessorId = indicesAccessor.id;
                    meshPrimitive.attributes[ACCESSOR_POSITION] = positionsAccessor.id;
                    meshPrimitive.mode = MESH_TRIANGLE_STRIP;

                    GLTFResourceReader reader(readerWriter);

                    auto outputPositions = MeshPrimitiveUtils::GetPositions(doc, reader, meshPrimitive);
                    AreEqual(positions, outputPositions);

                    std::vector<uint16_t> triangulatedIndices =
                    {
                        0, 3, 1,
                        3, 2, 1
                    };

                    auto outputIndices = MeshPrimitiveUtils::GetTriangulatedIndices16(doc, reader, meshPrimitive);
                    AreEqual(triangulatedIndices, outputIndices);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetTriangulatedIndices16_TriangleFan_Indices)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<float> positions = {
                        0.0f, 0.0f, 0.0f,
                        1.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f,
                        1.0f, 1.0f, 0.0f
                    };
                    auto positionsAccessor = bufferBuilder.AddAccessor(positions, { TYPE_VEC3, COMPONENT_FLOAT });

                    std::vector<uint16_t> indices = {
                        0U, 3U, 1U, 2U
                    };
                    auto indicesAccessor = bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_SHORT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    MeshPrimitive meshPrimitive;
                    meshPrimitive.indicesAccessorId = indicesAccessor.id;
                    meshPrimitive.attributes[ACCESSOR_POSITION] = positionsAccessor.id;
                    meshPrimitive.mode = MESH_TRIANGLE_FAN;

                    GLTFResourceReader reader(readerWriter);

                    auto outputPositions = MeshPrimitiveUtils::GetPositions(doc, reader, meshPrimitive);
                    AreEqual(positions, outputPositions);

                    std::vector<uint16_t> triangulatedIndices =
                    {
                        0, 3, 1,
                        0, 1, 2
                    };

                    auto outputIndices = MeshPrimitiveUtils::GetTriangulatedIndices16(doc, reader, meshPrimitive);
                    AreEqual(triangulatedIndices, outputIndices);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetTriangulatedIndices32_TriangleStrip_NoIndices)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    const auto numVertices = UINT16_MAX * 2;

                    std::vector<float> positions(numVertices * 3); // Multiply by 3 since they're VEC3 positions
                    for (uint32_t i = 0; i < positions.size(); i+=3)
                    {
                        if (i % 2 == 0)
                        {
                            positions[i]     = 0.0f;
                            positions[i + 1] = i * 1.0f;
                            positions[i + 2] = 0.0f;
                        }
                        else
                        {
                            positions[i]     = i * 1.0f;;
                            positions[i + 1] = 0.0f;
                            positions[i + 2] = 0.0f;
                        }
                    }

                    auto accessor = bufferBuilder.AddAccessor(positions, { TYPE_VEC3, COMPONENT_FLOAT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    MeshPrimitive meshPrimitive;
                    meshPrimitive.attributes[ACCESSOR_POSITION] = accessor.id;
                    meshPrimitive.mode = MESH_TRIANGLE_STRIP;

                    GLTFResourceReader reader(readerWriter);

                    auto outputPositions = MeshPrimitiveUtils::GetPositions(doc, reader, meshPrimitive);
                    AreEqual(positions, outputPositions);

                    auto outputIndices = MeshPrimitiveUtils::GetTriangulatedIndices32(doc, reader, meshPrimitive);

                    const size_t expectedIndexCount = (numVertices - 2) * 3; // Two less triangles than the number of verts, 3 indices per triangle
                    Assert::AreEqual(outputIndices.size(), expectedIndexCount);

                    // 0,1,2 type triangle (1st, 3rd, etc)
                    Assert::AreEqual(outputIndices[393198], 131066U);
                    Assert::AreEqual(outputIndices[393199], 131067U);
                    Assert::AreEqual(outputIndices[393200], 131068U);

                    // 1,3,2 type triangle (2nd, 4th, etc)
                    Assert::AreEqual(outputIndices[393201], 131067U);
                    Assert::AreEqual(outputIndices[393202], 131069U);
                    Assert::AreEqual(outputIndices[393203], 131068U);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetTriangulatedIndices32_TriangleFan_NoIndices)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    const auto numVertices = UINT16_MAX * 2;

                    std::vector<float> positions(numVertices * 3); // Multiply by 3 since they're VEC3 positions
                    for (uint32_t i = 0; i < positions.size(); i += 3)
                    {
                        if (i % 2 == 0)
                        {
                            positions[i]     = 0.0f;
                            positions[i + 1] = i * 1.0f;
                            positions[i + 2] = 0.0f;
                        }
                        else
                        {
                            positions[i]     = i * 1.0f;;
                            positions[i + 1] = 0.0f;
                            positions[i + 2] = 0.0f;
                        }
                    }

                    auto accessor = bufferBuilder.AddAccessor(positions, { TYPE_VEC3, COMPONENT_FLOAT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    MeshPrimitive meshPrimitive;
                    meshPrimitive.attributes[ACCESSOR_POSITION] = accessor.id;
                    meshPrimitive.mode = MESH_TRIANGLE_FAN;

                    GLTFResourceReader reader(readerWriter);

                    auto outputPositions = MeshPrimitiveUtils::GetPositions(doc, reader, meshPrimitive);
                    AreEqual(positions, outputPositions);

                    auto outputIndices = MeshPrimitiveUtils::GetTriangulatedIndices32(doc, reader, meshPrimitive);

                    const size_t expectedIndexCount = (numVertices - 2) * 3; // Two less triangles than the number of verts, 3 indices per triangle
                    Assert::AreEqual(outputIndices.size(), expectedIndexCount);

                    // 131067th triangle - 0, n, n+1
                    Assert::AreEqual(outputIndices[393198], 0U);
                    Assert::AreEqual(outputIndices[393199], 131067U);
                    Assert::AreEqual(outputIndices[393200], 131068U);

                    // 131068th triangle - 0, n, n+1
                    Assert::AreEqual(outputIndices[393201], 0U);
                    Assert::AreEqual(outputIndices[393202], 131068U);
                    Assert::AreEqual(outputIndices[393203], 131069U);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetSegmentedIndices16_LineStrip_NoIndices)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<float> positions = {
                        0.0f, 0.0f, 0.0f,
                        1.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f,
                        1.0f, 1.0f, 0.0f
                    };
                    auto accessor = bufferBuilder.AddAccessor(positions, { TYPE_VEC3, COMPONENT_FLOAT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    MeshPrimitive meshPrimitive;
                    meshPrimitive.attributes[ACCESSOR_POSITION] = accessor.id;
                    meshPrimitive.mode = MESH_LINE_STRIP;

                    GLTFResourceReader reader(readerWriter);

                    auto outputPositions = MeshPrimitiveUtils::GetPositions(doc, reader, meshPrimitive);
                    AreEqual(positions, outputPositions);

                    std::vector<uint16_t> expectedIndices =
                    {
                        0, 1,
                        1, 2,
                        2, 3
                    };

                    auto outputIndices = MeshPrimitiveUtils::GetSegmentedIndices16(doc, reader, meshPrimitive);
                    AreEqual(expectedIndices, outputIndices);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetSegmentedIndices16_LineLoop_NoIndices)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<float> positions = {
                        0.0f, 0.0f, 0.0f,
                        1.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f,
                        1.0f, 1.0f, 0.0f
                    };
                    auto accessor = bufferBuilder.AddAccessor(positions, { TYPE_VEC3, COMPONENT_FLOAT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    MeshPrimitive meshPrimitive;
                    meshPrimitive.attributes[ACCESSOR_POSITION] = accessor.id;
                    meshPrimitive.mode = MESH_LINE_LOOP;

                    GLTFResourceReader reader(readerWriter);

                    auto outputPositions = MeshPrimitiveUtils::GetPositions(doc, reader, meshPrimitive);
                    AreEqual(positions, outputPositions);

                    std::vector<uint16_t> expectedIndices =
                    {
                        0, 1,
                        1, 2,
                        2, 3,
                        3, 0
                    };

                    auto outputIndices = MeshPrimitiveUtils::GetSegmentedIndices16(doc, reader, meshPrimitive);
                    AreEqual(expectedIndices, outputIndices);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetSegmentedIndices16_LineStrip_Indices)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<float> positions = {
                        0.0f, 0.0f, 0.0f,
                        1.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f,
                        1.0f, 1.0f, 0.0f
                    };
                    auto positionsAccessor = bufferBuilder.AddAccessor(positions, { TYPE_VEC3, COMPONENT_FLOAT });

                    std::vector<uint16_t> indices = {
                        0U, 3U, 1U, 2U
                    };
                    auto indicesAccessor = bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_SHORT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    MeshPrimitive meshPrimitive;
                    meshPrimitive.indicesAccessorId = indicesAccessor.id;
                    meshPrimitive.attributes[ACCESSOR_POSITION] = positionsAccessor.id;
                    meshPrimitive.mode = MESH_LINE_STRIP;

                    GLTFResourceReader reader(readerWriter);

                    auto outputPositions = MeshPrimitiveUtils::GetPositions(doc, reader, meshPrimitive);
                    AreEqual(positions, outputPositions);

                    std::vector<uint16_t> segmentedIndices =
                    {
                        0, 3,
                        3, 1,
                        1, 2
                    };

                    auto outputIndices = MeshPrimitiveUtils::GetSegmentedIndices16(doc, reader, meshPrimitive);
                    AreEqual(segmentedIndices, outputIndices);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetSegmentedIndices16_LineLoop_Indices)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<float> positions = {
                        0.0f, 0.0f, 0.0f,
                        1.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f,
                        1.0f, 1.0f, 0.0f
                    };
                    auto positionsAccessor = bufferBuilder.AddAccessor(positions, { TYPE_VEC3, COMPONENT_FLOAT });

                    std::vector<uint16_t> indices = {
                        0U, 3U, 1U, 2U
                    };
                    auto indicesAccessor = bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_SHORT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    MeshPrimitive meshPrimitive;
                    meshPrimitive.indicesAccessorId = indicesAccessor.id;
                    meshPrimitive.attributes[ACCESSOR_POSITION] = positionsAccessor.id;
                    meshPrimitive.mode = MESH_LINE_LOOP;

                    GLTFResourceReader reader(readerWriter);

                    auto outputPositions = MeshPrimitiveUtils::GetPositions(doc, reader, meshPrimitive);
                    AreEqual(positions, outputPositions);

                    std::vector<uint16_t> segmentedIndices =
                    {
                        0, 3,
                        3, 1,
                        1, 2,
                        2, 0
                    };

                    auto outputIndices = MeshPrimitiveUtils::GetSegmentedIndices16(doc, reader, meshPrimitive);
                    AreEqual(segmentedIndices, outputIndices);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetSegmentedIndices32_LineStrip_NoIndices)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    const auto numVertices = UINT16_MAX * 2;

                    std::vector<float> positions(numVertices * 3); // Multiply by 3 since they're VEC3 positions
                    for (uint32_t i = 0; i < positions.size(); i += 3)
                    {
                        if (i % 2 == 0)
                        {
                            positions[i] = 0.0f;
                            positions[i + 1] = i * 1.0f;
                            positions[i + 2] = 0.0f;
                        }
                        else
                        {
                            positions[i] = i * 1.0f;;
                            positions[i + 1] = 0.0f;
                            positions[i + 2] = 0.0f;
                        }
                    }

                    auto accessor = bufferBuilder.AddAccessor(positions, { TYPE_VEC3, COMPONENT_FLOAT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    MeshPrimitive meshPrimitive;
                    meshPrimitive.attributes[ACCESSOR_POSITION] = accessor.id;
                    meshPrimitive.mode = MESH_LINE_STRIP;

                    GLTFResourceReader reader(readerWriter);

                    auto outputPositions = MeshPrimitiveUtils::GetPositions(doc, reader, meshPrimitive);
                    AreEqual(positions, outputPositions);

                    auto outputIndices = MeshPrimitiveUtils::GetSegmentedIndices32(doc, reader, meshPrimitive);

                    const size_t expectedIndexCount = (numVertices - 1) * 2; // One less line than the number of verts, 2 indices per segment
                    Assert::AreEqual(outputIndices.size(), expectedIndexCount);

                    // 131067th segment - n-1, n
                    Assert::AreEqual(outputIndices[262132], 131066U);
                    Assert::AreEqual(outputIndices[262133], 131067U);

                    // 131068th segment - n-1, n
                    Assert::AreEqual(outputIndices[262134], 131067U);
                    Assert::AreEqual(outputIndices[262135], 131068U);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_GetSegmentedIndices32_LineLoop_NoIndices)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    const auto numVertices = UINT16_MAX * 2;

                    std::vector<float> positions(numVertices * 3); // Multiply by 3 since they're VEC3 positions
                    for (uint32_t i = 0; i < positions.size(); i += 3)
                    {
                        if (i % 2 == 0)
                        {
                            positions[i] = 0.0f;
                            positions[i + 1] = i * 1.0f;
                            positions[i + 2] = 0.0f;
                        }
                        else
                        {
                            positions[i] = i * 1.0f;;
                            positions[i + 1] = 0.0f;
                            positions[i + 2] = 0.0f;
                        }
                    }

                    auto accessor = bufferBuilder.AddAccessor(positions, { TYPE_VEC3, COMPONENT_FLOAT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    MeshPrimitive meshPrimitive;
                    meshPrimitive.attributes[ACCESSOR_POSITION] = accessor.id;
                    meshPrimitive.mode = MESH_LINE_LOOP;

                    GLTFResourceReader reader(readerWriter);

                    auto outputPositions = MeshPrimitiveUtils::GetPositions(doc, reader, meshPrimitive);
                    AreEqual(positions, outputPositions);

                    auto outputIndices = MeshPrimitiveUtils::GetSegmentedIndices32(doc, reader, meshPrimitive);

                    const size_t expectedIndexCount = numVertices * 2; // Same number of segments as verts, 2 indices per segment
                    Assert::AreEqual(outputIndices.size(), expectedIndexCount);

                    // 131067th segment - n-1, n
                    Assert::AreEqual(outputIndices[262132], 131066U);
                    Assert::AreEqual(outputIndices[262133], 131067U);

                    // 131068th segment - n-1, n
                    Assert::AreEqual(outputIndices[262134], 131067U);
                    Assert::AreEqual(outputIndices[262135], 131068U);

                    // 131070th segment, last - n-1, 0
                    Assert::AreEqual(outputIndices[262138], 131069U);
                    Assert::AreEqual(outputIndices[262139], 0U);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_SerializeTriangulatedIndices16_TriangleStrip)
                {
                    std::vector<uint16_t> triangulatedIndices =
                    {
                        0, 3, 1,
                        3, 2, 1,
                        1, 2, 4,
                        2, 5, 4
                    };

                    auto outputIndices = MeshPrimitiveUtils::ReverseTriangulateIndices16(triangulatedIndices, MESH_TRIANGLE_STRIP);

                    std::vector<uint16_t> expectedIndices =
                    {
                        0, 3, 1, 2, 4, 5
                    };

                    AreEqual(expectedIndices, outputIndices);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_SerializeTriangulatedIndices16_TriangleFan)
                {
                    std::vector<uint16_t> triangulatedIndices =
                    {
                        5, 2, 0,
                        5, 0, 1,
                        5, 1, 4,
                        5, 4, 3
                    };

                    auto outputIndices = MeshPrimitiveUtils::ReverseTriangulateIndices16(triangulatedIndices, MESH_TRIANGLE_FAN);

                    std::vector<uint16_t> expectedIndices =
                    {
                        5, 2, 0, 1, 4, 3
                    };

                    AreEqual(expectedIndices, outputIndices);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_SerializeTriangulatedIndices32_TriangleStrip)
                {
                    std::vector<uint32_t> triangulatedIndices =
                    {
                        0, 3, 1,
                        3, 2, 1,
                        1, 2, 4,
                        2, 5, 4
                    };

                    auto outputIndices = MeshPrimitiveUtils::ReverseTriangulateIndices32(triangulatedIndices, MESH_TRIANGLE_STRIP);

                    std::vector<uint32_t> expectedIndices =
                    {
                        0, 3, 1, 2, 4, 5
                    };

                    AreEqual(expectedIndices, outputIndices);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_SerializeTriangulatedIndices32_TriangleFan)
                {
                    std::vector<uint32_t> triangulatedIndices =
                    {
                        5, 2, 0,
                        5, 0, 1,
                        5, 1, 4,
                        5, 4, 3
                    };

                    auto outputIndices = MeshPrimitiveUtils::ReverseTriangulateIndices32(triangulatedIndices, MESH_TRIANGLE_FAN);

                    std::vector<uint32_t> expectedIndices =
                    {
                        5, 2, 0, 1, 4, 3
                    };

                    AreEqual(expectedIndices, outputIndices);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_SerializeSegmentedIndices16_LineStrip)
                {
                    std::vector<uint16_t> segmentedIndices =
                    {
                        4, 2,
                        2, 1,
                        1, 3,
                        3, 0
                    };

                    auto outputIndices = MeshPrimitiveUtils::ReverseSegmentIndices16(segmentedIndices, MESH_LINE_STRIP);

                    std::vector<uint16_t> expectedIndices =
                    {
                        4, 2, 1, 3, 0
                    };

                    AreEqual(expectedIndices, outputIndices);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_SerializeSegmentedIndices16_LineLoop)
                {
                    std::vector<uint16_t> segmentedIndices =
                    {
                        4, 2,
                        2, 1,
                        1, 3,
                        3, 0,
                        0, 4
                    };

                    auto outputIndices = MeshPrimitiveUtils::ReverseSegmentIndices16(segmentedIndices, MESH_LINE_LOOP);

                    std::vector<uint16_t> expectedIndices =
                    {
                        4, 2, 1, 3, 0
                    };

                    AreEqual(expectedIndices, outputIndices);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_SerializeSegmentedIndices32_LineStrip)
                {
                    std::vector<uint32_t> segmentedIndices =
                    {
                        4, 2,
                        2, 1,
                        1, 3,
                        3, 0
                    };

                    auto outputIndices = MeshPrimitiveUtils::ReverseSegmentIndices32(segmentedIndices, MESH_LINE_STRIP);

                    std::vector<uint32_t> expectedIndices =
                    {
                        4, 2, 1, 3, 0
                    };

                    AreEqual(expectedIndices, outputIndices);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_SerializeSegmentedIndices32_LineLoop)
                {
                    std::vector<uint32_t> segmentedIndices =
                    {
                        4, 2,
                        2, 1,
                        1, 3,
                        3, 0,
                        0, 4
                    };

                    auto outputIndices = MeshPrimitiveUtils::ReverseSegmentIndices32(segmentedIndices, MESH_LINE_LOOP);

                    std::vector<uint32_t> expectedIndices =
                    {
                        4, 2, 1, 3, 0
                    };

                    AreEqual(expectedIndices, outputIndices);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_TriangulatedIndices16Roundtrip_TriangleStrip)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint16_t> indices = {
                        0U, 3U, 1U, 2U
                    };
                    auto indicesAccessor = bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_SHORT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    MeshPrimitive meshPrimitive;
                    meshPrimitive.indicesAccessorId = indicesAccessor.id;
                    meshPrimitive.mode = MESH_TRIANGLE_STRIP;

                    GLTFResourceReader reader(readerWriter);

                    auto triangulatedIndices = MeshPrimitiveUtils::GetTriangulatedIndices16(doc, reader, meshPrimitive);
                    auto outputIndices = MeshPrimitiveUtils::ReverseTriangulateIndices16(triangulatedIndices, meshPrimitive.mode);

                    AreEqual(outputIndices, indices);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_TriangulatedIndices16Roundtrip_TriangleFan)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint16_t> indices = {
                        0U, 3U, 1U, 2U
                    };
                    auto indicesAccessor = bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_SHORT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    MeshPrimitive meshPrimitive;
                    meshPrimitive.indicesAccessorId = indicesAccessor.id;
                    meshPrimitive.mode = MESH_TRIANGLE_FAN;

                    GLTFResourceReader reader(readerWriter);

                    auto triangulatedIndices = MeshPrimitiveUtils::GetTriangulatedIndices16(doc, reader, meshPrimitive);
                    auto outputIndices = MeshPrimitiveUtils::ReverseTriangulateIndices16(triangulatedIndices, meshPrimitive.mode);

                    AreEqual(outputIndices, indices);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_TriangulatedIndices32Roundtrip_TriangleStrip)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint32_t> indices = {
                        0U, 3U, 1U, 2U
                    };
                    auto indicesAccessor = bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_INT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    MeshPrimitive meshPrimitive;
                    meshPrimitive.indicesAccessorId = indicesAccessor.id;
                    meshPrimitive.mode = MESH_TRIANGLE_STRIP;

                    GLTFResourceReader reader(readerWriter);

                    auto triangulatedIndices = MeshPrimitiveUtils::GetTriangulatedIndices32(doc, reader, meshPrimitive);
                    auto outputIndices = MeshPrimitiveUtils::ReverseTriangulateIndices32(triangulatedIndices, meshPrimitive.mode);

                    AreEqual(outputIndices, indices);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_TriangulatedIndices32Roundtrip_TriangleFan)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint32_t> indices = {
                        0U, 3U, 1U, 2U
                    };
                    auto indicesAccessor = bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_INT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    MeshPrimitive meshPrimitive;
                    meshPrimitive.indicesAccessorId = indicesAccessor.id;
                    meshPrimitive.mode = MESH_TRIANGLE_FAN;

                    GLTFResourceReader reader(readerWriter);

                    auto triangulatedIndices = MeshPrimitiveUtils::GetTriangulatedIndices32(doc, reader, meshPrimitive);
                    auto outputIndices = MeshPrimitiveUtils::ReverseTriangulateIndices32(triangulatedIndices, meshPrimitive.mode);

                    AreEqual(outputIndices, indices);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_SegmentedIndices16Roundtrip_LineStrip)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint16_t> indices = {
                        0U, 3U, 1U, 2U
                    };
                    auto indicesAccessor = bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_SHORT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    MeshPrimitive meshPrimitive;
                    meshPrimitive.indicesAccessorId = indicesAccessor.id;
                    meshPrimitive.mode = MESH_LINE_STRIP;

                    GLTFResourceReader reader(readerWriter);

                    auto segmentedIndices = MeshPrimitiveUtils::GetSegmentedIndices16(doc, reader, meshPrimitive);
                    auto outputIndices = MeshPrimitiveUtils::ReverseSegmentIndices16(segmentedIndices, meshPrimitive.mode);

                    AreEqual(outputIndices, indices);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_SegmentedIndices16Roundtrip_LineLoop)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint16_t> indices = {
                        0U, 3U, 1U, 2U
                    };
                    auto indicesAccessor = bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_SHORT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    MeshPrimitive meshPrimitive;
                    meshPrimitive.indicesAccessorId = indicesAccessor.id;
                    meshPrimitive.mode = MESH_LINE_LOOP;

                    GLTFResourceReader reader(readerWriter);

                    auto segmentedIndices = MeshPrimitiveUtils::GetSegmentedIndices16(doc, reader, meshPrimitive);
                    auto outputIndices = MeshPrimitiveUtils::ReverseSegmentIndices16(segmentedIndices, meshPrimitive.mode);

                    AreEqual(outputIndices, indices);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_SegmentedIndices32Roundtrip_LineStrip)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint32_t> indices = {
                        0U, 3U, 1U, 2U
                    };
                    auto indicesAccessor = bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_INT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    MeshPrimitive meshPrimitive;
                    meshPrimitive.indicesAccessorId = indicesAccessor.id;
                    meshPrimitive.mode = MESH_LINE_STRIP;

                    GLTFResourceReader reader(readerWriter);

                    auto segmentedIndices = MeshPrimitiveUtils::GetSegmentedIndices32(doc, reader, meshPrimitive);
                    auto outputIndices = MeshPrimitiveUtils::ReverseSegmentIndices32(segmentedIndices, meshPrimitive.mode);

                    AreEqual(outputIndices, indices);
                }

                GLTFSDK_TEST_METHOD(MeshPrimitiveUtilsTests, MeshPrimitiveUtils_Test_SegmentedIndices32Roundtrip_LineLoop)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<uint32_t> indices = {
                        0U, 3U, 1U, 2U
                    };
                    auto indicesAccessor = bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_INT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    MeshPrimitive meshPrimitive;
                    meshPrimitive.indicesAccessorId = indicesAccessor.id;
                    meshPrimitive.mode = MESH_LINE_LOOP;

                    GLTFResourceReader reader(readerWriter);

                    auto segmentedIndices = MeshPrimitiveUtils::GetSegmentedIndices32(doc, reader, meshPrimitive);
                    auto outputIndices = MeshPrimitiveUtils::ReverseSegmentIndices32(segmentedIndices, meshPrimitive.mode);

                    AreEqual(outputIndices, indices);
                }
            };
        }
    }
}
