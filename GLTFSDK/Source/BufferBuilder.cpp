// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/BufferBuilder.h>

#include <GLTFSDK/ResourceWriter.h>

using namespace Microsoft::glTF;

namespace
{
    size_t GetPadding(size_t offset, size_t alignment)
    {
        const auto padAlign = offset % alignment;
        const auto pad = padAlign ? alignment - padAlign : 0U;

        return pad;
    }

    size_t GetPadding(size_t offset, ComponentType componentType)
    {
        return GetPadding(offset, Accessor::GetComponentTypeSize(componentType));
    }

    size_t GetAlignment(const AccessorDesc& desc)
    {
        return Accessor::GetComponentTypeSize(desc.componentType);
    }
}

BufferBuilder::BufferBuilder(std::unique_ptr<ResourceWriter>&& resourceWriter) : BufferBuilder(std::move(resourceWriter), {}, {}, {})
{
}

BufferBuilder::BufferBuilder(std::unique_ptr<ResourceWriter>&& resourceWriter,
    FnGenId fnGenBufferId,
    FnGenId fnGenBufferViewId,
    FnGenId fnGenAccessorId) : m_resourceWriter(std::move(resourceWriter)),
    m_fnGenBufferId(std::move(fnGenBufferId)),
    m_fnGenBufferViewId(std::move(fnGenBufferViewId)),
    m_fnGenAccessorId(std::move(fnGenAccessorId))
{
}

const Buffer& BufferBuilder::AddBuffer(const char* bufferId)
{
    Buffer buffer;

    if (bufferId)
    {
        buffer.id = bufferId;
    }
    else if (m_fnGenBufferId)
    {
        buffer.id = m_fnGenBufferId(*this);
    }

    buffer.byteLength = 0U;// The buffer's length is updated whenever an Accessor or BufferView is added (and data is written to the underlying buffer)
    auto& bufferRef = m_buffers.Append(std::move(buffer), AppendIdPolicy::GenerateOnEmpty);
    bufferRef.uri = m_resourceWriter->GenerateBufferUri(bufferRef.id);

    return bufferRef;
}

const BufferView& BufferBuilder::AddBufferView(Optional<BufferViewTarget> target)
{
    Buffer& buffer = m_buffers.Back();
    BufferView bufferView;

    if (m_fnGenBufferViewId)
    {
        bufferView.id = m_fnGenBufferViewId(*this);
    }

    bufferView.bufferId = buffer.id;
    bufferView.byteOffset = buffer.byteLength;
    bufferView.byteLength = 0U;// The BufferView's length is updated whenever an Accessor is added (and data is written to the underlying buffer)
    bufferView.target = target;

    return m_bufferViews.Append(std::move(bufferView), AppendIdPolicy::GenerateOnEmpty);
}

const BufferView& BufferBuilder::AddBufferView(const void* data, size_t byteLength, Optional<size_t> byteStride, Optional<BufferViewTarget> target)
{
    Buffer& buffer = m_buffers.Back();
    BufferView bufferView;

    if (m_fnGenBufferViewId)
    {
        bufferView.id = m_fnGenBufferViewId(*this);
    }

    bufferView.bufferId = buffer.id;
    bufferView.byteOffset = buffer.byteLength;
    bufferView.byteLength = byteLength;
    bufferView.byteStride = byteStride;
    bufferView.target = target;

    buffer.byteLength = bufferView.byteOffset + bufferView.byteLength;

    if (m_resourceWriter)
    {
        m_resourceWriter->Write(bufferView, data);
    }

    return m_bufferViews.Append(std::move(bufferView), AppendIdPolicy::GenerateOnEmpty);
}

const Accessor& BufferBuilder::AddAccessor(const void* data, size_t count, AccessorDesc desc)
{
    Buffer& buffer = m_buffers.Back();
    BufferView& bufferView = m_bufferViews.Back();

    // If the bufferView has not yet been written to then ensure it is correctly aligned for this accessor's component type
    if (bufferView.byteLength == 0U)
    {
        bufferView.byteOffset += ::GetPadding(bufferView.byteOffset, desc.componentType);
    }

    desc.byteOffset = bufferView.byteLength;
    const Accessor& accessor = AddAccessor(count, std::move(desc));

    bufferView.byteLength += accessor.GetByteLength();
    buffer.byteLength = bufferView.byteOffset + bufferView.byteLength;

    if (m_resourceWriter)
    {
        m_resourceWriter->Write(bufferView, data, accessor);
    }

    return accessor;
}

void BufferBuilder::AddAccessors(const void* data, size_t count, size_t byteStride, const AccessorDesc* pDescs, size_t descCount, std::string* pOutIds)
{
    Buffer& buffer = m_buffers.Back();
    BufferView& bufferView = m_bufferViews.Back();

    if (count == 0 || pDescs == nullptr || descCount == 0)
    {
        throw InvalidGLTFException("invalid parameters specified");
    }

    for (size_t i = 0; i < descCount; ++i)
    {
        if (!pDescs[i].IsValid())
        {
            throw InvalidGLTFException("invalid AccessorDesc specified in pDescs");
        }
    }

    if (bufferView.byteLength != 0U)
    {
        throw InvalidGLTFException("current buffer view already has written data - this interface doesn't support appending to an existing buffer view");
    }

    size_t extent;

    if (byteStride == 0)
    {
        if (descCount > 1)
        {
            throw InvalidGLTFException("glTF 2.0 specification denotes that byte stride must be >= 4 when a buffer view is accessed by more than one accessor");
        }

        extent = count * Accessor::GetComponentTypeSize(pDescs[0].componentType) * Accessor::GetTypeCount(pDescs[0].accessorType);
    }
    else
    {
        extent = count * byteStride;

        // Ensure all accessors fit within the buffer view's extent.
        const size_t lastElement = (count - 1) * (bufferView.byteStride ? bufferView.byteStride.Get() : 0U);

        for (size_t i = 0; i < descCount; ++i)
        {
            const size_t accessorSize = Accessor::GetTypeCount(pDescs[i].accessorType) * Accessor::GetComponentTypeSize(pDescs[i].componentType);
            const size_t accessorEnd = lastElement + pDescs[i].byteOffset + accessorSize;

            if (extent < accessorEnd)
            {
                throw InvalidGLTFException("specified accessor does not fit within the currently defined buffer view");
            }
        }
    }

    // Calculate the max alignment.
    size_t alignment = 1;
    for (size_t i = 0; i < descCount; ++i)
    {
        alignment = std::max(alignment, GetAlignment(pDescs[i]));
    }

    bufferView.byteStride = byteStride;
    bufferView.byteLength = extent;
    bufferView.byteOffset += ::GetPadding(bufferView.byteOffset, alignment);

    buffer.byteLength = bufferView.byteOffset + bufferView.byteLength;

    for (size_t i = 0; i < descCount; ++i)
    {
        AddAccessor(count, pDescs[i]);

        if (pOutIds != nullptr)
        {
            pOutIds[i] = GetCurrentAccessor().id;
        }
    }

    if (m_resourceWriter)
    {
        m_resourceWriter->Write(bufferView, data);
    }
}

void BufferBuilder::Output(Document& gltfDocument)
{
    for (auto& buffer : m_buffers.Elements())
    {
        gltfDocument.buffers.Append(std::move(buffer), AppendIdPolicy::ThrowOnEmpty);
    }

    m_buffers.Clear();

    for (auto& bufferView : m_bufferViews.Elements())
    {
        gltfDocument.bufferViews.Append(std::move(bufferView), AppendIdPolicy::ThrowOnEmpty);
    }

    m_bufferViews.Clear();

    for (auto& accessor : m_accessors.Elements())
    {
        gltfDocument.accessors.Append(std::move(accessor), AppendIdPolicy::ThrowOnEmpty);
    }

    m_accessors.Clear();
}

const Buffer& BufferBuilder::GetCurrentBuffer() const
{
    return m_buffers.Back();
}

const BufferView& BufferBuilder::GetCurrentBufferView() const
{
    return m_bufferViews.Back();
}

const Accessor& BufferBuilder::GetCurrentAccessor() const
{
    return m_accessors.Back();
}

size_t BufferBuilder::GetBufferCount() const
{
    return m_buffers.Size();
}

size_t BufferBuilder::GetBufferViewCount() const
{
    return m_bufferViews.Size();
}

size_t BufferBuilder::GetAccessorCount() const
{
    return m_accessors.Size();
}

ResourceWriter& BufferBuilder::GetResourceWriter()
{
    return *m_resourceWriter;
}

const ResourceWriter& BufferBuilder::GetResourceWriter() const
{
    return *m_resourceWriter;
}

const Accessor& BufferBuilder::AddAccessor(size_t count, AccessorDesc desc)
{
    Buffer& buffer = m_buffers.Back();
    BufferView& bufferView = m_bufferViews.Back();

    if (buffer.id != bufferView.bufferId)
    {
        throw InvalidGLTFException("bufferView.bufferId does not match buffer.id");
    }

    if (count == 0)
    {
        throw GLTFException("Invalid accessor count: 0");
    }

    if (desc.accessorType == TYPE_UNKNOWN)
    {
        throw GLTFException("Invalid accessorType: TYPE_UNKNOWN");
    }

    if (desc.componentType == COMPONENT_UNKNOWN)
    {
        throw GLTFException("Invalid componentType: COMPONENT_UNKNOWN");
    }

    const auto accessorTypeSize = Accessor::GetTypeCount(desc.accessorType);
    size_t componentTypeSize = Accessor::GetComponentTypeSize(desc.componentType);

    // Only check for a valid number of min and max values if they exist
    if ((!desc.minValues.empty() || !desc.maxValues.empty()) &&
        ((desc.minValues.size() != accessorTypeSize) || (desc.maxValues.size() != accessorTypeSize)))
    {
        throw InvalidGLTFException("the number of min and max values must be equal to the number of elements to be stored in the accessor");
    }

    if (desc.byteOffset % componentTypeSize != 0)
    {
        throw InvalidGLTFException("asccessor offset within buffer view must be a multiple of the component size");
    }

    if ((desc.byteOffset + bufferView.byteOffset) % componentTypeSize != 0)
    {
        throw InvalidGLTFException("accessor offset within buffer must be a multiple of the component size");
    }

    Accessor accessor;

    if (m_fnGenAccessorId)
    {
        accessor.id = m_fnGenAccessorId(*this);
    }

    accessor.bufferViewId = bufferView.id;
    accessor.count = count;
    accessor.byteOffset = desc.byteOffset;
    accessor.type = desc.accessorType;
    accessor.componentType = desc.componentType;
    accessor.normalized = desc.normalized;

    // TODO: make accessor min & max members be vectors of doubles
    accessor.min = desc.minValues;
    accessor.max = desc.maxValues;

    return m_accessors.Append(std::move(accessor), AppendIdPolicy::GenerateOnEmpty);
}
