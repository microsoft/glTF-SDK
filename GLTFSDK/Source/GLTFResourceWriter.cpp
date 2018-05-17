// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/GLTFResourceWriter.h>

#include <GLTFSDK/StreamCacheLRU.h>

using namespace Microsoft::glTF;

GLTFResourceWriter::GLTFResourceWriter(std::shared_ptr<const IStreamWriter> streamWriter)
    : GLTFResourceWriter(MakeStreamWriterCache<StreamWriterCacheLRU>(std::move(streamWriter), 16U))
{
}

GLTFResourceWriter::GLTFResourceWriter(std::unique_ptr<IStreamWriterCache> streamCache)
    : ResourceWriter(std::move(streamCache)),
    m_streamOffsets()
{
}

std::string GLTFResourceWriter::GenerateBufferUri(const std::string& bufferId) const
{
    return m_uriPrefix + bufferId + "." + Microsoft::glTF::BUFFER_EXTENSION;
}

void GLTFResourceWriter::SetUriPrefix(std::string uriPrefix)
{
    m_uriPrefix = std::move(uriPrefix);
}

std::ostream* GLTFResourceWriter::GetBufferStream(const std::string& bufferId)
{
    return m_streamWriterCache->Get(GenerateBufferUri(bufferId)).get();
}

std::streamoff GLTFResourceWriter::GetBufferOffset(const std::string& bufferId)
{
    // Default constructs an offset value (i.e. zero) if there was no entry in the map
    return m_streamOffsets[bufferId];
}

void GLTFResourceWriter::SetBufferOffset(const std::string& bufferId, std::streamoff offset)
{
    m_streamOffsets[bufferId] = offset;
}
