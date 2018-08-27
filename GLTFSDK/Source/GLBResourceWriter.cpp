// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/GLBResourceWriter.h>

#include <sstream>

using namespace Microsoft::glTF;

namespace
{
    uint32_t CalculatePadding(size_t byteLength)
    {
        const uint32_t alignmentSize = GLB_CHUNK_ALIGNMENT_SIZE;

        const auto padAlign = byteLength % alignmentSize;
        const auto pad = padAlign ? alignmentSize - padAlign : 0U;

        return static_cast<uint32_t>(pad);
    }
}

GLBResourceWriter::GLBResourceWriter(std::shared_ptr<const IStreamWriter> streamWriter)
    : GLBResourceWriter(std::move(streamWriter), std::make_unique<std::stringstream>())
{
}

GLBResourceWriter::GLBResourceWriter(std::shared_ptr<const IStreamWriter> streamWriter, std::unique_ptr<std::iostream> tempBufferStream)
    : GLTFResourceWriter(std::move(streamWriter)),
    m_stream(std::move(tempBufferStream))
{
}

GLBResourceWriter::GLBResourceWriter(std::unique_ptr<IStreamWriterCache> streamCache)
    : GLBResourceWriter(std::move(streamCache), std::make_unique<std::stringstream>())
{
}

GLBResourceWriter::GLBResourceWriter(std::unique_ptr<IStreamWriterCache> streamCache, std::unique_ptr<std::iostream> tempBufferStream)
    : GLTFResourceWriter(std::move(streamCache)),
    m_stream(std::move(tempBufferStream))
{
}

void GLBResourceWriter::Flush(const std::string& manifest, const std::string& uri)
{
    uint32_t jsonChunkLength = static_cast<uint32_t>(manifest.length());
    const uint32_t jsonPaddingLength = ::CalculatePadding(jsonChunkLength);

    jsonChunkLength += jsonPaddingLength;

    uint32_t binaryChunkLength = static_cast<uint32_t>(GetBufferOffset(GLB_BUFFER_ID));
    const uint32_t binaryPaddingLength = ::CalculatePadding(binaryChunkLength);

    binaryChunkLength += binaryPaddingLength;

    const uint32_t length = GLB_HEADER_BYTE_SIZE // 12 bytes (GLB header) + 8 bytes (JSON header)
        + jsonChunkLength
        + sizeof(binaryChunkLength) + GLB_CHUNK_TYPE_SIZE // 8 bytes (BIN header)
        + binaryChunkLength;

    auto stream = m_streamWriterCache->Get(uri);

    // Write GLB header (12 bytes)
    StreamUtils::WriteBinary(*stream, GLB_HEADER_MAGIC_STRING, GLB_HEADER_MAGIC_STRING_SIZE);
    StreamUtils::WriteBinary(*stream, GLB_HEADER_VERSION_2);
    StreamUtils::WriteBinary(*stream, length);

    // Write JSON header (8 bytes)
    StreamUtils::WriteBinary(*stream, jsonChunkLength);
    StreamUtils::WriteBinary(*stream, GLB_CHUNK_TYPE_JSON, GLB_CHUNK_TYPE_SIZE);

    // Write JSON (indeterminate length)
    StreamUtils::WriteBinary(*stream, manifest);

    if (jsonPaddingLength > 0)
    {
        // GLB spec requires the JSON chunk to be padded with trailing space characters (0x20) to satisfy alignment requirements
        StreamUtils::WriteBinary(*stream, std::string(jsonPaddingLength, ' '));
    }

    // Write BIN header (8 bytes)
    StreamUtils::WriteBinary(*stream, binaryChunkLength);
    StreamUtils::WriteBinary(*stream, GLB_CHUNK_TYPE_BIN, GLB_CHUNK_TYPE_SIZE);

    // Write BIN contents (indeterminate length) - copy the temporary buffer's contents to the output stream
    if (binaryChunkLength > 0)
    {
        *stream << m_stream->rdbuf();
    }

    if (binaryPaddingLength > 0)
    {
        // GLB spec requires the BIN chunk to be padded with trailing zeros (0x00) to satisfy alignment requirements
        StreamUtils::WriteBinary(*stream, std::vector<uint8_t>(binaryPaddingLength, 0));
    }
}

std::string GLBResourceWriter::GenerateBufferUri(const std::string& bufferId) const
{
    std::string bufferUri;

    // Return an empty uri string when passed the GLB buffer id
    if (bufferId != GLB_BUFFER_ID)
    {
        bufferUri = GLTFResourceWriter::GenerateBufferUri(bufferId);
    }

    return bufferUri;
}

std::ostream* GLBResourceWriter::GetBufferStream(const std::string& bufferId)
{
    std::ostream* stream = m_stream.get();

    if (bufferId != GLB_BUFFER_ID)
    {
        stream = GLTFResourceWriter::GetBufferStream(bufferId);
    }

    return stream;
}