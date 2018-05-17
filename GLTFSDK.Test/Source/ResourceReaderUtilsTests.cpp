// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLTFResourceReader.h>
#include <GLTFSDK/ResourceReaderUtils.h>

#include "TestUtils.h"

#include <memory>
#include <string>

using namespace glTF::UnitTest;

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(ResourceReaderUtilsTest)
            {
                GLTFSDK_TEST_METHOD(ResourceReaderUtilsTest, TestValidBase64UriRanges)
                {
                    static const std::pair<std::vector<uint8_t>, std::string> tests[] = {
                        { {}, "" },// 0 bytes -> empty string

                        { { 0x0 }, "AA" },// 1 byte (0 blocks + 8 bits) -> 2 chars (0 blocks + 12 bits)
                        { { 0x0 }, "AA==" },
                        { { 0x0, 0x1 }, "AAE" },// 2 bytes (0 blocks + 16 bits) -> 3 chars (0 blocks + 18 bits)
                        { { 0x0, 0x1 }, "AAE=" },
                        { { 0x0, 0x1, 0x2 }, "AAEC" },// 3 bytes (1 block + 0 bits) -> 4 chars (1 block + 0 bits)
                        //{ { 0x0, 0x1, 0x2 }, "AAEC" },
                        { { 0x0, 0x1, 0x2, 0x3 }, "AAECAw" },// 4 bytes (1 block + 8 bits) -> 6 chars (1 block + 12 bits)
                        { { 0x0, 0x1, 0x2, 0x3 }, "AAECAw==" },
                        { { 0x0, 0x1, 0x2, 0x3, 0x4 }, "AAECAwQ" },// 5 bytes (1 block + 16 bits) -> 7 chars (1 block + 18 bits)
                        { { 0x0, 0x1, 0x2, 0x3, 0x4 }, "AAECAwQ=" },
                        { { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5 }, "AAECAwQF" },// 6 bytes (2 blocks + 0 bits) -> 8 chars (2 blocks + 0 bits)
                        //{ { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5 }, "AAECAwQF" },
                        { { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6 }, "AAECAwQFBg"},// 7 bytes (2 blocks + 8 bits) -> 10 chars (2 blocks + 12 bits)
                        { { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6 }, "AAECAwQFBg=="},
                        { { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7 }, "AAECAwQFBgc" },// 8 bytes (2 blocks + 16 bits) -> 11 chars (2 blocks + 18 bits)
                        { { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7 }, "AAECAwQFBgc=" },
                        { { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8 }, "AAECAwQFBgcI" },// 9 bytes (3 blocks + 0 bits) -> 12 chars (3 blocks + 0 bits)
                        //{ { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8 }, "AAECAwQFBgcI" },
                        { { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9 }, "AAECAwQFBgcICQ" },// 10 bytes (3 blocks + 8 bits) -> 14 chars (3 blocks + 12 bits)
                        { { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9 }, "AAECAwQFBgcICQ==" },

                        { { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, "////////////////" },// 12 bytes (4 blocks + 0 bits) -> 16 chars (4 blocks + 0 bits)
                        { { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, "/////////////////w" },// 13 bytes (4 blocks + 8 bits) -> 18 chars (4 blocks + 12 bits)
                        { { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, "//////////////////8" }// 14 bytes (4 blocks + 16 bits) -> 19 chars (4 blocks + 18 bits)
                    };

                    GLTFResourceReader gltfResourceReader(std::make_shared<StreamReaderWriter>());

                    for (auto& test : tests)
                    {
                        Microsoft::glTF::Buffer buffer;

                        buffer.id = "buffer";
                        buffer.uri = "data:application/octet-stream;base64," + test.second;
                        buffer.byteLength = test.first.size();

                        std::string::const_iterator itBegin, itEnd;

                        Assert::IsTrue(IsUriBase64(buffer.uri, itBegin, itEnd), L"Data uri was not recognized as such");
                        Assert::IsTrue(Base64Decode(test.second) == test.first, L"Decoded data uri doesn't match expected values");

                        Document gltfDocument;
                        gltfDocument.buffers.Append(buffer);

                        for (size_t i = 0; i < test.first.size(); ++i)
                        {
                            for (size_t j = i + 1U; j <= test.first.size(); ++j)
                            {
                                Microsoft::glTF::BufferView bufferView;

                                bufferView.bufferId = buffer.id;
                                bufferView.byteOffset = i;
                                bufferView.byteLength = j - i;

                                auto resultExpected = std::vector<uint8_t>(test.first.begin() + i, test.first.begin() + j);
                                auto resultActual = gltfResourceReader.ReadBinaryData<uint8_t>(gltfDocument, bufferView);

                                Assert::IsTrue(resultExpected == resultActual, L"Decoded data uri range doesn't match expected values");
                            }
                        }
                    }
                }

                GLTFSDK_TEST_METHOD(ResourceReaderUtilsTest, TestValidBase64UriFinal2Chars)
                {
                    const auto data = Base64Decode("YW55IGNhcm5hbCBwbGVhcw");

                    const std::string decodedString(data.begin(), data.end());
                    const std::string encodedString = "any carnal pleas";

                    Assert::AreEqual(encodedString.c_str(), decodedString.c_str(), L"Incorrect decoded string");
                }

                GLTFSDK_TEST_METHOD(ResourceReaderUtilsTest, TestValidBase64UriFinal3Chars)
                {
                    const auto data = Base64Decode("YW55IGNhcm5hbCBwbGVhc3U");

                    const std::string decodedString(data.begin(), data.end());
                    const std::string encodedString = "any carnal pleasu";

                    Assert::AreEqual(encodedString.c_str(), decodedString.c_str(), L"Incorrect decoded string");
                }

                GLTFSDK_TEST_METHOD(ResourceReaderUtilsTest, TestBase64UriInterleaved)
                {
                    Buffer buffer;
                    buffer.id = "buffer1";
                    buffer.uri = "data:application/octet-stream;base64,MTIzNDEyMzQxMjM0MTIzNA==";// Data uri stores the ASCII string: "1234123412341234"
                    buffer.byteLength = 16;

                    BufferView bufferView;
                    bufferView.id = "bufferView1";
                    bufferView.bufferId = buffer.id;
                    bufferView.byteLength = buffer.byteLength;
                    bufferView.byteStride = 4;

                    Accessor accessor1;
                    accessor1.id = "accessor1";
                    accessor1.bufferViewId = bufferView.id;
                    accessor1.byteOffset = 0;
                    accessor1.count = 4;
                    accessor1.componentType = ComponentType::COMPONENT_BYTE;
                    accessor1.type = AccessorType::TYPE_SCALAR;

                    Accessor accessor2;
                    accessor2.id = "accessor2";
                    accessor2.bufferViewId = bufferView.id;
                    accessor2.byteOffset = 1;
                    accessor2.count = 4;
                    accessor2.componentType = ComponentType::COMPONENT_BYTE;
                    accessor2.type = AccessorType::TYPE_SCALAR;

                    Accessor accessor3;
                    accessor3.id = "accessor3";
                    accessor3.bufferViewId = bufferView.id;
                    accessor3.byteOffset = 2;
                    accessor3.count = 4;
                    accessor3.componentType = ComponentType::COMPONENT_BYTE;
                    accessor3.type = AccessorType::TYPE_SCALAR;

                    Accessor accessor4;
                    accessor4.id = "accessor4";
                    accessor4.bufferViewId = bufferView.id;
                    accessor4.byteOffset = 3;
                    accessor4.count = 4;
                    accessor4.componentType = ComponentType::COMPONENT_BYTE;
                    accessor4.type = AccessorType::TYPE_SCALAR;

                    Document gltfDocument;

                    gltfDocument.buffers.Append(buffer);
                    gltfDocument.bufferViews.Append(bufferView);

                    gltfDocument.accessors.Append(accessor1);
                    gltfDocument.accessors.Append(accessor2);
                    gltfDocument.accessors.Append(accessor3);
                    gltfDocument.accessors.Append(accessor4);

                    GLTFResourceReader resourceReader(std::make_shared<StreamReaderWriter>());

                    auto a1Actual = resourceReader.ReadBinaryData<int8_t>(gltfDocument, accessor1);
                    auto a1Expected = std::vector<int8_t>{ '1', '1', '1', '1' };

                    Assert::IsTrue(a1Expected == a1Actual, L"Unexpected result reading interleaved accessor data from base64 encoded data uri");

                    auto a2Actual = resourceReader.ReadBinaryData<int8_t>(gltfDocument, accessor2);
                    auto a2Expected = std::vector<int8_t>{ '2', '2', '2', '2' };

                    Assert::IsTrue(a2Expected == a2Actual, L"Unexpected result reading interleaved accessor data from base64 encoded data uri");

                    auto a3Actual = resourceReader.ReadBinaryData<int8_t>(gltfDocument, accessor3);
                    auto a3Expected = std::vector<int8_t>{ '3', '3', '3', '3' };

                    Assert::IsTrue(a3Expected == a3Actual, L"Unexpected result reading interleaved accessor data from base64 encoded data uri");

                    auto a4Actual = resourceReader.ReadBinaryData<int8_t>(gltfDocument, accessor4);
                    auto a4Expected = std::vector<int8_t>{ '4', '4', '4', '4' };

                    Assert::IsTrue(a4Expected == a4Actual, L"Unexpected result reading interleaved accessor data from base64 encoded data uri");
                }

                GLTFSDK_TEST_METHOD(ResourceReaderUtilsTest, TestValidBase64UriFinal4Chars)
                {
                    const auto data = Base64Decode("YW55IGNhcm5hbCBwbGVhc3Vy");

                    const std::string decodedString(data.begin(), data.end());
                    const std::string encodedString = "any carnal pleasur";

                    Assert::AreEqual(encodedString.c_str(), decodedString.c_str(), L"");
                }

                GLTFSDK_TEST_METHOD(ResourceReaderUtilsTest, TestValidBase64Uri_noPadding1)
                {
                    std::string encoded_string = "a9TA";
                    Assert::IsTrue(std::vector<uint8_t>{ (uint8_t)0x6B, (uint8_t)0xD4, (uint8_t)0xC0 } == Base64Decode(encoded_string));
                }
        
                GLTFSDK_TEST_METHOD(ResourceReaderUtilsTest, TestValidBase64Uri_onePadding1)
                {
                    const std::string encoded_string = "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlz"
                        "IHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2Yg"
                        "dGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGlu"
                        "dWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRo"
                        "ZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=";
                    const std::string decoded_string = "Man is distinguished, not only by his reason, but by this singular passion from "
                        "other animals, which is a lust of the mind, that by a perseverance of delight "
                        "in the continued and indefatigable generation of knowledge, exceeds the short "
                        "vehemence of any carnal pleasure.";
                    Assert::IsTrue(std::vector<uint8_t>(decoded_string.begin(), decoded_string.end()) == Base64Decode(encoded_string));
                }
        
                GLTFSDK_TEST_METHOD(ResourceReaderUtilsTest, TestValidBase64Uri_twoPadding1)
                {
                    const std::string encoded_string = "/+==";
                    Assert::IsTrue(std::vector<uint8_t>{ 0xFF } == Base64Decode(encoded_string));
                }
        
                // Randomly generated strings tested against other decoders
                GLTFSDK_TEST_METHOD(ResourceReaderUtilsTest, TestValidBase64Uri_noPadding2)
                {
                    const std::string encoded_string = "FyMP";
                    std::string expected = "\x17\x23\x0f";
                    Assert::IsTrue(std::vector<uint8_t>(expected.begin(), expected.end()) == Base64Decode(encoded_string));
                }

                GLTFSDK_TEST_METHOD(ResourceReaderUtilsTest, TestValidBase64Uri_twoPadding2)
                {
                    const std::string encoded_string = "UpRSREKIOvh9DUlSc8PvywTI7d1f99eKJ0v3l4VtK1eVQwL4mqmKGHVoovwe21QsB3oKyFZpDFA8vVT3mzsWGakiHukw1a4qk4lRfx9Dhlw6INlWGeKcaxo+/i6dj2/MaAOXUFqEMeWYDeqdt1njqUIF3SZtmPMaLXKh5IHyt4ZdIRKVD+szeL==";
                    std::string expected = "\x52\x94\x52\x44\x42\x88\x3a\xf8\x7d\x0d\x49\x52\x73\xc3\xef\xcb\x04\xc8\xed\xdd\x5f\xf7\xd7\x8a\x27\x4b\xf7\x97\x85\x6d\x2b\x57\x95\x43\x02\xf8\x9a\xa9\x8a\x18\x75\x68\xa2\xfc\x1e\xdb\x54\x2c\x07\x7a\x0a\xc8\x56\x69\x0c\x50\x3c\xbd\x54\xf7\x9b\x3b\x16\x19\xa9\x22\x1e\xe9\x30\xd5\xae\x2a\x93\x89\x51\x7f\x1f\x43\x86\x5c\x3a\x20\xd9\x56\x19\xe2\x9c\x6b\x1a\x3e\xfe\x2e\x9d\x8f\x6f\xcc\x68\x03\x97\x50\x5a\x84\x31\xe5\x98\x0d\xea\x9d\xb7\x59\xe3\xa9\x42\x05\xdd\x26\x6d\x98\xf3\x1a\x2d\x72\xa1\xe4\x81\xf2\xb7\x86\x5d\x21\x12\x95\x0f\xeb\x33\x78";
                    Assert::IsTrue(std::vector<uint8_t>(expected.begin(), expected.end()) == Base64Decode(encoded_string));
                }

                GLTFSDK_TEST_METHOD(ResourceReaderUtilsTest, TestValidBase64Uri_noPadding3)
                {
                    const std::string encoded_string = "+ZHqpntIdgEB52E9Xq6iS/usFvCAUed9xMJVYOabc/Rcmz/z7suY9R851bJMPSUjm4gBCEdsfREDxYDSkcakokFYtub3";
                    std::string expected = "\xf9\x91\xea\xa6\x7b\x48\x76\x01\x01\xe7\x61\x3d\x5e\xae\xa2\x4b\xfb\xac\x16\xf0\x80\x51\xe7\x7d\xc4\xc2\x55\x60\xe6\x9b\x73\xf4\x5c\x9b\x3f\xf3\xee\xcb\x98\xf5\x1f\x39\xd5\xb2\x4c\x3d\x25\x23\x9b\x88\x01\x08\x47\x6c\x7d\x11\x03\xc5\x80\xd2\x91\xc6\xa4\xa2\x41\x58\xb6\xe6\xf7";
                    Assert::IsTrue(std::vector<uint8_t>(expected.begin(), expected.end()) == Base64Decode(encoded_string));
                }

                GLTFSDK_TEST_METHOD(ResourceReaderUtilsTest, TestValidBase64Uri_onePadding2)
                {
                    const std::string encoded_string = "i2FmFUWwv9jv/mdgNWBcgHtbhPy2Q/qx2MM4bs9p4DpTv/T+6Apri9ccxjvXp/No2dflixe1I3mTLXMQHLyXIDZ16J2=";
                    std::string expected = "\x8b\x61\x66\x15\x45\xb0\xbf\xd8\xef\xfe\x67\x60\x35\x60\x5c\x80\x7b\x5b\x84\xfc\xb6\x43\xfa\xb1\xd8\xc3\x38\x6e\xcf\x69\xe0\x3a\x53\xbf\xf4\xfe\xe8\x0a\x6b\x8b\xd7\x1c\xc6\x3b\xd7\xa7\xf3\x68\xd9\xd7\xe5\x8b\x17\xb5\x23\x79\x93\x2d\x73\x10\x1c\xbc\x97\x20\x36\x75\xe8\x9d";
                    Assert::IsTrue(std::vector<uint8_t>(expected.begin(), expected.end()) == Base64Decode(encoded_string));
                }

                GLTFSDK_TEST_METHOD(ResourceReaderUtilsTest, TestValidBase64Uri_twoPadding3)
                {
                    const std::string encoded_string = "hhHYLzn0CsgdGhB461xgd9Dq8jLumIvChuBJbUMtjisoZIjJjjq1igFsljaNcqDdPtiEPJ1Yteqer20OwneXA6fjqMvcE1avUoTQQaK+JnBIjxbQIK2PdU6Z1myiFybCZl71FP0mdEZtoCAWTTZkj1+Vt5LoJpvdEtD8VwbYT+bVkxMo5Mve4nSg6Mg13i9I+I==";
                    std::string expected = "\x86\x11\xd8\x2f\x39\xf4\x0a\xc8\x1d\x1a\x10\x78\xeb\x5c\x60\x77\xd0\xea\xf2\x32\xee\x98\x8b\xc2\x86\xe0\x49\x6d\x43\x2d\x8e\x2b\x28\x64\x88\xc9\x8e\x3a\xb5\x8a\x01\x6c\x96\x36\x8d\x72\xa0\xdd\x3e\xd8\x84\x3c\x9d\x58\xb5\xea\x9e\xaf\x6d\x0e\xc2\x77\x97\x03\xa7\xe3\xa8\xcb\xdc\x13\x56\xaf\x52\x84\xd0\x41\xa2\xbe\x26\x70\x48\x8f\x16\xd0\x20\xad\x8f\x75\x4e\x99\xd6\x6c\xa2\x17\x26\xc2\x66\x5e\xf5\x14\xfd\x26\x74\x46\x6d\xa0\x20\x16\x4d\x36\x64\x8f\x5f\x95\xb7\x92\xe8\x26\x9b\xdd\x12\xd0\xfc\x57\x06\xd8\x4f\xe6\xd5\x93\x13\x28\xe4\xcb\xde\xe2\x74\xa0\xe8\xc8\x35\xde\x2f\x48\xf8";
                    Assert::IsTrue(std::vector<uint8_t>(expected.begin(), expected.end()) == Base64Decode(encoded_string));
                }

                GLTFSDK_TEST_METHOD(ResourceReaderUtilsTest, TestValidBase64Uri_Empty)
                {
                    const std::string encoded_string = "";
                    Assert::IsTrue(Base64Decode(encoded_string) == std::vector<uint8_t>());
                }

                GLTFSDK_TEST_METHOD(ResourceReaderUtilsTest, TestInvalidBase64Uri_SpecialChar1)
                {
                    Assert::ExpectException<GLTFException>([]()
                    {
                        const std::string encoded_string = "aaa\t";
                        Base64Decode(encoded_string);
                    });
                }

                GLTFSDK_TEST_METHOD(ResourceReaderUtilsTest, TestInvalidBase64Uri_SpecialChar2)
                {
                    Assert::ExpectException<GLTFException>([]()
                    {
                        const std::string encoded_string = "aa/\\";
                        Base64Decode(encoded_string);
                    });
                }

                GLTFSDK_TEST_METHOD(ResourceReaderUtilsTest, TestInvalidBase64Uri_BadPadding)
                {
                    Assert::ExpectException<GLTFException>([]()
                    {
                        const std::string encoded_string = "lfjoi=a=";
                        Base64Decode(encoded_string);
                    });
                }

                GLTFSDK_TEST_METHOD(ResourceReaderUtilsTest, TestAllAsciiChar)
                {
                    const std::string base64_chars =
                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                        "abcdefghijklmnopqrstuvwxyz"
                        "0123456789+/";
                    constexpr uint8_t NUM_ASCII_CHARS = 128;
                    for (uint8_t i = 0; i < NUM_ASCII_CHARS; i++)
                    {
                        char c = static_cast<char>(i);
                        std::string encoded_string = { c, 'A', '=', '=' };
                        if (base64_chars.find(c) == std::string::npos)
                        {
                            Assert::ExpectException<GLTFException>([encoded_string]()
                            {
                                Base64Decode(encoded_string);
                            });
                        }
                        else
                        {
                            int decodedByte = (int)base64_chars.find(c) << 2;
                            auto decoded_string = Base64Decode(encoded_string);
                            Assert::IsTrue(decoded_string == std::vector<uint8_t>{ (uint8_t)decodedByte });
                        }
                    }
                }

                GLTFSDK_TEST_METHOD(ResourceReaderUtilsTest, TestIsUriBase64)
                {
                    std::string::const_iterator itBegin;
                    std::string::const_iterator itEnd;

                    Assert::IsTrue(IsUriBase64("data:image/png;base64,/+==", itBegin, itEnd));
                }
            };
        }
    }
}