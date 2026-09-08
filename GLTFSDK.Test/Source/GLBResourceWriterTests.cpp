// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"
#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/Exceptions.h>
#include <GLTFSDK/GLBResourceReader.h>
#include <GLTFSDK/GLBResourceWriter.h>
#include <GLTFSDK/Serialize.h>
#include "TestUtils.h"

#include <sstream>

using namespace glTF::UnitTest;

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(GLBResourceWriterTests)
            {
                GLTFSDK_TEST_METHOD(GLBResourceWriterTests, WriteBufferView_Empty_Bin)
                {
                    auto streamWriter = std::make_shared<const StreamReaderWriter>();
                    GLBResourceWriter writer(streamWriter);
                    std::string uri = "foo.glb";

                    // Serialize Default Document -> Json string -> Stream
                    Document doc;
                    const auto serialiedJson = Serialize(doc, SerializeFlags::None);
                    writer.Flush(serialiedJson, uri);
                    auto stream = streamWriter->GetInputStream(uri);

                    // Deserialize Stream -> Document
                    GLBResourceReader resourceReader(streamWriter, stream);
                    Document roundTrippedDoc = Deserialize(resourceReader.GetJson());

                    Assert::IsFalse(stream->fail());
                    Assert::IsTrue(doc == roundTrippedDoc);
                }

                GLTFSDK_TEST_METHOD(GLBResourceWriterTests, FlushStream_Empty_Bin)
                {
                    auto streamWriter = std::make_shared<const StreamReaderWriter>();
                    GLBResourceWriter writer(streamWriter);
                    Document doc;
                    const auto serializedJson = Serialize(doc, SerializeFlags::None);
                    std::stringstream output(
                        std::ios::in | std::ios::out | std::ios::binary);

                    writer.FlushStream(serializedJson, &output);

                    auto stream = std::make_shared<std::stringstream>(
                        output.str(),
                        std::ios::in | std::ios::out | std::ios::binary);
                    GLBResourceReader resourceReader(streamWriter, stream);
                    const Document roundTrippedDoc =
                        Deserialize(resourceReader.GetJson());

                    Assert::IsFalse(stream->fail());
                    Assert::IsTrue(doc == roundTrippedDoc);
                }

                GLTFSDK_TEST_METHOD(GLBResourceWriterTests, GLBReader_RejectsOverflowingJsonChunkLength)
                {
                    // A GLB whose JSON chunk length is 0xFFFFFFFF previously passed the header-size check in
                    // GLBResourceReader::Init because (GLB_HEADER_BYTE_SIZE + jsonChunkLength) was computed in
                    // 32-bit and wrapped below the file length. The addition is now performed in 64-bit, so the
                    // invalid chunk length is rejected.
                    const std::string json = "{\"asset\":{\"version\":\"2.0\"}}";

                    auto writeU32 = [](std::string& s, uint32_t v)
                    {
                        s.push_back(static_cast<char>(v & 0xFF));
                        s.push_back(static_cast<char>((v >> 8) & 0xFF));
                        s.push_back(static_cast<char>((v >> 16) & 0xFF));
                        s.push_back(static_cast<char>((v >> 24) & 0xFF));
                    };

                    std::string glb;
                    glb += "glTF";                                                    // magic
                    writeU32(glb, 2);                                                  // version
                    writeU32(glb, static_cast<uint32_t>(20 + json.size()));           // total length (matches actual stream length)
                    writeU32(glb, 0xFFFFFFFFu);                                        // JSON chunk length (overflowing)
                    glb += "JSON";                                                    // JSON chunk type
                    glb += json;                                                      // JSON payload

                    auto streamReader = std::make_shared<const StreamReaderWriter>();
                    auto glbStream = std::make_shared<std::stringstream>(glb, std::ios::in | std::ios::out | std::ios::binary);

                    Assert::ExpectException<InvalidGLTFException>([&streamReader, &glbStream]()
                    {
                        GLBResourceReader resourceReader(streamReader, glbStream);
                    });
                }
            };
        }
    }
}
