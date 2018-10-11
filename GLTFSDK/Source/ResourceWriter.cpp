// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/ResourceWriter.h>

using namespace Microsoft::glTF;

ResourceWriter::ResourceWriter(std::unique_ptr<IStreamWriterCache> streamWriterCache) : m_streamWriterCache(std::move(streamWriterCache))
{
}

ResourceWriter::~ResourceWriter() = default;

void ResourceWriter::Write(const BufferView& bufferView, const void* data)
{
    WriteImpl(bufferView, data, bufferView.byteOffset, bufferView.byteLength);
}

void ResourceWriter::Write(const BufferView& bufferView, const void* data, const Accessor& accessor)
{
    if (accessor.bufferViewId != bufferView.id)
    {
        throw InvalidGLTFException("accessor.bufferViewId does not match bufferView.id");
    }

    const auto componentTypeSize = Accessor::GetComponentTypeSize(accessor.componentType);

    // From the glTF 2.0 spec: the offset of an accessor into a bufferView (accessor.byteOffset) must be a multiple of the size of the accessor's component type
    if (accessor.byteOffset % componentTypeSize)
    {
        throw InvalidGLTFException("accessor.byteOffset must be a multiple of the accessor's component type size");
    }

    // From the glTF 2.0 spec: the offset of an accessor into a buffer (accessor.byteOffset + bufferView.byteOffset) must be a multiple of the size of the accessor's component type
    if ((accessor.byteOffset + bufferView.byteOffset) % componentTypeSize)
    {
        throw InvalidGLTFException("accessor.byteOffset + bufferView.byteOffset must be a multiple of the accessor's component type size");
    }

    const auto accessorByteLength = accessor.GetByteLength();

    // Ensure there is enough room in the BufferView for the accessor's data
    if (bufferView.byteLength < accessor.byteOffset + accessorByteLength)
    {
        throw InvalidGLTFException("accessor offset and byte length exceed the buffer view's byte length");
    }

    WriteImpl(bufferView, data, bufferView.byteOffset + accessor.byteOffset, accessorByteLength);
}

void ResourceWriter::WriteExternal(const std::string& uri, const void* data, size_t byteLength) const
{
    if (auto stream = m_streamWriterCache->Get(uri))
    {
        StreamUtils::WriteBinary(*stream, data, byteLength);
    }
}

void ResourceWriter::WriteExternal(const std::string& uri, const std::string& data) const
{
    WriteExternal(uri, data.c_str(), data.length());
}

void ResourceWriter::WriteImpl(const BufferView& bufferView, const void* data, std::streamoff totalOffset, size_t totalByteLength)
{
    // TODO: vertex attributes must be aligned to 4-byte boundaries inside a bufferView (accessor.byteOffset and bufferView.byteStride must be multiples of 4)

    if (auto bufferStream = GetBufferStream(bufferView.bufferId))
    {
        const auto bufferOffset = GetBufferOffset(bufferView.bufferId);

        if (totalOffset < bufferOffset)
        {
            throw InvalidGLTFException("Stream 'put' pointer is already ahead of specified offset");
        }
        else if (totalOffset > bufferOffset)
        {
            const auto padSize = static_cast<size_t>(totalOffset - bufferOffset);
            const auto padData = std::make_unique<char[]>(padSize);

            StreamUtils::WriteBinary(*bufferStream, padData.get(), padSize);
        }

        SetBufferOffset(bufferView.bufferId, totalOffset);

        if (StreamUtils::WriteBinary(*bufferStream, data, totalByteLength) != totalByteLength)
        {
            throw InvalidGLTFException("An unexpected number of bytes were output to the stream");
        }

        SetBufferOffset(bufferView.bufferId, totalOffset + totalByteLength);
    }
}
