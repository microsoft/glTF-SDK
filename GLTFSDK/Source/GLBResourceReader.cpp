// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/GLBResourceReader.h>

#include <GLTFSDK/Constants.h>

#include <memory>

using namespace Microsoft::glTF;

namespace
{
    bool ParseChunkType(const char* expectedChunkType, std::istream& stream)
    {
        char buffer[GLB_CHUNK_TYPE_SIZE];
        stream.read(buffer, GLB_CHUNK_TYPE_SIZE);
        if (stream.fail())
        {
            throw InvalidGLTFException("Cannot read the chunk type: " + std::string(expectedChunkType));
        }
        auto cmp = memcmp(&buffer[0], expectedChunkType, GLB_CHUNK_TYPE_SIZE);
        return cmp == 0;
    }

    std::string ReadJson(std::istream& stream, size_t jsonLength)
    {
        stream.seekg(GLB_HEADER_BYTE_SIZE);
        std::string json(jsonLength, '\0');
        StreamUtils::ReadBinary(stream, &json[0], jsonLength);
        if (stream.fail())
        {
            throw InvalidGLTFException("Cannot read the json from the GLB file");
        }
        return json;
    }
}

GLBResourceReader::GLBResourceReader(std::shared_ptr<const IStreamReader> streamReader, std::shared_ptr<std::istream> glbStream)
    : GLTFResourceReader(std::move(streamReader)),
    m_buffer(std::move(glbStream)),
    m_bufferOffset()
{
    Init();
}

GLBResourceReader::GLBResourceReader(std::unique_ptr<IStreamReaderCache> streamCache, std::shared_ptr<std::istream> glbStream)
    : GLTFResourceReader(std::move(streamCache)),
    m_buffer(std::move(glbStream)),
    m_bufferOffset()
{
    Init();
}

std::shared_ptr<std::istream> GLBResourceReader::GetBinaryStream(const Buffer& buffer) const
{
    std::shared_ptr<std::istream> stream;

    // We allow "uri": "data:," to refer to a GLB buffer
    if (buffer.uri.empty() || buffer.uri == EMPTY_URI)
    {
        stream = m_buffer;
    }
    else
    {
        stream = GLTFResourceReader::GetBinaryStream(buffer);
    }

    return stream;
}

std::streampos GLBResourceReader::GetBinaryStreamPos(const Buffer& buffer) const
{
    std::streampos streamPos;

    // We allow "uri": "data:," to refer to a GLB buffer
    if (buffer.uri.empty() || buffer.uri == EMPTY_URI)
    {
        streamPos = m_bufferOffset;
    }
    else
    {
        streamPos = GLTFResourceReader::GetBinaryStreamPos(buffer);
    }

    return streamPos;
}

const std::string& GLBResourceReader::GetJson() const
{
    return m_json;
}

void GLBResourceReader::Init()
{
    // Get the length of the stream before reading anything, to validate against later
    // NOTE: The approach used below with seekg to the end and then tellg may be problematic since
    // seekg is not guaranteed to give the number of bytes from the start of the file:
    // see http://stackoverflow.com/a/22986486. This is used elsewhere in the code though,
    // and seems to Do The Right Thing as of right now. TODO: discuss fixing this problem.
    const auto curPos = m_buffer->tellg();
    m_buffer->seekg(0, std::ios::end);
    uint32_t trueStreamLength = static_cast<uint32_t>(m_buffer->tellg()); // in bytes (i.e. sizeof(char))
    m_buffer->seekg(curPos); // reset the stream pointer where it was

    char magic[GLB_HEADER_MAGIC_STRING_SIZE];
    m_buffer->read(magic, sizeof(magic));
    if (m_buffer->fail())
    {
        throw InvalidGLTFException("Cannot read the magic number");
    }

    const uint32_t version = StreamUtils::ReadBinary<uint32_t>(*m_buffer);
    const uint32_t length = StreamUtils::ReadBinary<uint32_t>(*m_buffer);

    // Verify that the length we just read actually matches the length of the stream
    if (trueStreamLength != length)
    {
        throw InvalidGLTFException("File-reported file length does not match actual file length");
    }

    const uint32_t jsonChunkLength = StreamUtils::ReadBinary<uint32_t>(*m_buffer);

    if (!ParseChunkType(GLB_CHUNK_TYPE_JSON, *m_buffer))
    {
        throw InvalidGLTFException("JSON chunk should appear first");
    }

    // validate header
    if (strncmp(magic, GLB_HEADER_MAGIC_STRING, GLB_HEADER_MAGIC_STRING_SIZE) != 0)
    {
        throw InvalidGLTFException("Cannot find GLB magic bytes");
    }

    if (version != GLB_HEADER_VERSION_2)
    {
        throw InvalidGLTFException("Unsupported GLB Version: " + std::to_string(version));
    }

    // Length has been validated as the actual length of the file, but make sure to include the header bytes in this check
    if (length < (GLB_HEADER_BYTE_SIZE + jsonChunkLength))
    {
        throw InvalidGLTFException("File length " + std::to_string(length) + " less than content length " + std::to_string(jsonChunkLength) + 
            " plus header length " + std::to_string(GLB_HEADER_BYTE_SIZE));
    }

    m_json = ReadJson(*m_buffer, jsonChunkLength);

    // If length is exactly equal to the json chunk length, plus the header, it means there is no binary buffer chunk
    if (length == (GLB_HEADER_BYTE_SIZE + jsonChunkLength))
    {
        return;
    }

    // Read the length of the binary buffer chunk
    const uint32_t bufferChunkLength = StreamUtils::ReadBinary<uint32_t>(*m_buffer);

    if (!ParseChunkType(GLB_CHUNK_TYPE_BIN, *m_buffer))
    {
        throw InvalidGLTFException("Binary chunk should appear second");
    }

    // Verify that the sum of the sizes of the chunks (plus the headers) matches the size of the file
    const uint32_t chunkSizeSum = GLB_HEADER_BYTE_SIZE + jsonChunkLength + sizeof(bufferChunkLength) + GLB_CHUNK_TYPE_SIZE +  bufferChunkLength;

    if (chunkSizeSum != length)
    {
        throw InvalidGLTFException("File length does not match sum of length of component chunks");
    }

    m_bufferOffset = m_buffer->tellg();
}
