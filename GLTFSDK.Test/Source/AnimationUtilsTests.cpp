// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"
#include <typeinfo>
#include <map>

#include <GLTFSDK/AnimationUtils.h>
#include <GLTFSDK/BufferBuilder.h>
#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLTFResourceReader.h>
#include <GLTFSDK/GLTFResourceWriter.h>

#include "TestUtils.h"

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            namespace
            {
                const std::map<const std::type_index, ComponentType> kComponentTypeMap =
                {
                    { std::type_index(typeid(float)),    COMPONENT_FLOAT },
                    { std::type_index(typeid(int8_t)),   COMPONENT_BYTE },
                    { std::type_index(typeid(uint8_t)),  COMPONENT_UNSIGNED_BYTE },
                    { std::type_index(typeid(int16_t)),  COMPONENT_SHORT },
                    { std::type_index(typeid(uint16_t)), COMPONENT_UNSIGNED_SHORT }
                };

                // Utility for verifying GetMorphWeights
                template<typename T>
                void VerifyGetMorphWeights()
                {
                    std::vector<float> testValues = { 0.0f, 0.11f, 0.22f, 0.33f, 0.44f, 0.55f, 1.0f };

                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<T> input;
                    std::vector<float> expectedOutput;
                    for (auto& v : testValues)
                    {
                        auto c = AnimationUtils::FloatToComponent<T>(v);
                        input.push_back(c);
                        expectedOutput.push_back(AnimationUtils::ComponentToFloat(c));
                    }

                    auto componentType = kComponentTypeMap.find(std::type_index(typeid(T)));
                    Assert::IsTrue(componentType != kComponentTypeMap.end(), L"ComponentType not found");
                    auto accessor = bufferBuilder.AddAccessor(input, { TYPE_SCALAR, componentType->second });

                    Document doc;
                    bufferBuilder.Output(doc);

                    // Verify that we read back what's expected
                    std::stringstream ss;
                    ss << "Error extracting weights for component type " << typeid(T).name();
                    std::string s = ss.str();
                    std::wstring msg(s.begin(), s.end());

                    // Accessor
                    GLTFResourceReader reader(readerWriter);
                    auto output = AnimationUtils::GetMorphWeights(doc, reader, accessor);
                    AreEqual(expectedOutput, output, msg.c_str());

                    // Sampler
                    AnimationSampler animationSampler;
                    animationSampler.outputAccessorId = accessor.id;
                    output = AnimationUtils::GetMorphWeights(doc, reader, animationSampler);
                    AreEqual(expectedOutput, output, msg.c_str());
                }

                // Utility for verifying GetRotations
                template<typename T>
                void VerifyGetRotations()
                {
                    std::vector<float> testValues = { 0.213941514f, 0.963860869f, -0.158749819f, 0.204712942f };

                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<T> input;
                    std::vector<float> expectedOutput;
                    for (auto& v : testValues)
                    {
                        auto c = AnimationUtils::FloatToComponent<T>(v);
                        input.push_back(c);
                        expectedOutput.push_back(AnimationUtils::ComponentToFloat(c));
                    }

                    auto componentType = kComponentTypeMap.find(std::type_index(typeid(T)));
                    Assert::IsTrue(componentType != kComponentTypeMap.end(), L"ComponentType not found");
                    auto accessor = bufferBuilder.AddAccessor(input, { TYPE_VEC4, componentType->second });

                    Document doc;
                    bufferBuilder.Output(doc);

                    // Verify that we read back what's expected
                    std::stringstream ss;
                    ss << "Error extracting rotations for component type " << typeid(T).name();
                    std::string s = ss.str();
                    std::wstring msg(s.begin(), s.end());

                    // Accessor
                    GLTFResourceReader reader(readerWriter);
                    auto output = AnimationUtils::GetRotations(doc, reader, accessor);
                    AreEqual(expectedOutput, output, msg.c_str());

                    // Sampler
                    AnimationSampler animationSampler;
                    animationSampler.outputAccessorId = accessor.id;
                    output = AnimationUtils::GetRotations(doc, reader, animationSampler);
                    AreEqual(expectedOutput, output, msg.c_str());
                }
            }

            GLTFSDK_TEST_CLASS(AnimationUtilsTests)
            {
                GLTFSDK_TEST_METHOD(AnimationUtilsTests, AnimationUtils_Test_GetKeyframeTimes_Scalar_Float)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<float> input = { 0.000f, 0.100f, 0.200f, 0.300f };
                    auto accessor = bufferBuilder.AddAccessor(input, { TYPE_SCALAR, COMPONENT_FLOAT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = AnimationUtils::GetKeyframeTimes(doc, reader, accessor);

                    AreEqual(input, output);
                }

                GLTFSDK_TEST_METHOD(AnimationUtilsTests, AnimationUtils_Test_GetInverseBindMatrices_Mat4_Float)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<float> input = {
                         0.213941514f,    0.963860869f, -0.158749819f,  0.000000000f,
                         0.0374440104f,  -0.170484781f, -0.984648883f,  0.000000000f,
                        -0.976128876f,    0.204712942f, -0.0725645721f, 0.000000000f,
                       -10.2514353f,    -38.3263512f,   89.1614075f,    1.00000000f
                    };
                    auto accessor = bufferBuilder.AddAccessor(input, { TYPE_MAT4, COMPONENT_FLOAT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    GLTFResourceReader reader(readerWriter);
                    auto output = AnimationUtils::GetInverseBindMatrices(doc, reader, accessor);

                    AreEqual(input, output);
                }

                GLTFSDK_TEST_METHOD(AnimationUtilsTests, AnimationUtils_Test_GetTranslations_Vec3_Float)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<float> input = {
                        0.213941514f, 0.963860869f, -0.158749819f
                    };
                    auto accessor = bufferBuilder.AddAccessor(input, { TYPE_VEC3, COMPONENT_FLOAT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    // Accessor

                    GLTFResourceReader reader(readerWriter);
                    auto output = AnimationUtils::GetTranslations(doc, reader, accessor);

                    AreEqual(input, output);

                    // Sampler

                    AnimationSampler animationSampler;
                    animationSampler.outputAccessorId = accessor.id;
                    output = AnimationUtils::GetTranslations(doc, reader, animationSampler);

                    AreEqual(input, output);
                }

                GLTFSDK_TEST_METHOD(AnimationUtilsTests, AnimationUtils_Test_GetScales_Vec3_Float)
                {
                    auto readerWriter = std::make_shared<const StreamReaderWriter>();
                    auto bufferBuilder = BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));

                    bufferBuilder.AddBuffer();
                    bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

                    std::vector<float> input = {
                        0.213941514f, 0.963860869f, 0.204712942f
                    };
                    auto accessor = bufferBuilder.AddAccessor(input, { TYPE_VEC3, COMPONENT_FLOAT });

                    Document doc;
                    bufferBuilder.Output(doc);

                    // Accessor

                    GLTFResourceReader reader(readerWriter);
                    auto output = AnimationUtils::GetScales(doc, reader, accessor);

                    AreEqual(input, output);

                    // Sampler

                    AnimationSampler animationSampler;
                    animationSampler.outputAccessorId = accessor.id;
                    output = AnimationUtils::GetScales(doc, reader, animationSampler);

                    AreEqual(input, output);
                }

                // Verify GetWeights for all possible component types
                GLTFSDK_TEST_METHOD(AnimationUtilsTests, AnimationUtils_Test_GetMorphWeights)
                {
                    VerifyGetMorphWeights<float>();
                    VerifyGetMorphWeights<int8_t>();
                    VerifyGetMorphWeights<uint8_t>();
                    VerifyGetMorphWeights<int16_t>();
                    VerifyGetMorphWeights<uint16_t>();
                }

                // Verify GetRotations for all possible component types
                GLTFSDK_TEST_METHOD(AnimationUtilsTests, AnimationUtils_Test_GetRotations)
                {
                    VerifyGetRotations<float>();
                    VerifyGetRotations<int8_t>();
                    VerifyGetRotations<uint8_t>();
                    VerifyGetRotations<int16_t>();
                    VerifyGetRotations<uint16_t>();
                }
            };
        }
    }
}
