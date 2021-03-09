// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/Document.h>

#include <functional>

namespace Microsoft
{
    namespace glTF
    {
        class Document;
        class ResourceWriter;

        struct AccessorDesc
        {
            AccessorDesc() = default;

            AccessorDesc(AccessorType accessorType, ComponentType componentType, bool normalized = false, std::vector<float> minValues = {}, std::vector<float> maxValues = {}, size_t byteOffset = 0)
                : accessorType(accessorType), componentType(componentType), normalized(normalized),
                byteOffset(byteOffset), minValues(std::move(minValues)), maxValues(std::move(maxValues))
            { }

            bool IsValid() const { return accessorType != TYPE_UNKNOWN && componentType != COMPONENT_UNKNOWN; }

            AccessorType accessorType;
            ComponentType componentType;
            bool normalized;
            size_t byteOffset;
            std::vector<float> minValues;
            std::vector<float> maxValues;
        };

        class BufferBuilder final
        {
            typedef std::function<std::string(const BufferBuilder&)> FnGenId;

        public:
            BufferBuilder(std::unique_ptr<ResourceWriter>&& resourceWriter);
            BufferBuilder(std::unique_ptr<ResourceWriter>&& resourceWriter,
                FnGenId fnGenBufferId,
                FnGenId fnGenBufferViewId,
                FnGenId fnGenAccessorId);

            const Buffer& AddBuffer(const char* bufferId = nullptr);

            const BufferView& AddBufferView(Optional<BufferViewTarget> target = {});
            const BufferView& AddBufferView(const void* data, size_t byteLength, Optional<size_t> byteStride = {}, Optional<BufferViewTarget> target = {});

            template<typename T>
            const BufferView& AddBufferView(const std::vector<T>& data, Optional<size_t> byteStride = {}, Optional<BufferViewTarget> target = {})
            {
                return AddBufferView(data.data(), data.size() * sizeof(T), byteStride, target);
            }

            const Accessor& AddAccessor(const void* data, size_t count, AccessorDesc accessorDesc);

            template<typename T>
            const Accessor& AddAccessor(const std::vector<T>& data, AccessorDesc accessorDesc)
            {
                const auto accessorTypeSize = Accessor::GetTypeCount(accessorDesc.accessorType);

                if (data.size() % accessorTypeSize)
                {
                    throw InvalidGLTFException("vector size is not a multiple of accessor type size");
                }

                return AddAccessor(data.data(), data.size() / accessorTypeSize, std::move(accessorDesc));
            }

            void AddAccessors(const void* data, size_t count, size_t byteStride, const AccessorDesc* pDescs, size_t descCount, std::string* pOutIds = nullptr);

            // This method moved from the .cpp to the header because
            // When this library is built with VS2017 and used in an executable built with VS2019
            // an unordered_map issue ( see https://docs.microsoft.com/en-us/cpp/overview/cpp-conformance-improvements?view=msvc-160 )
            // makes the destruction to perform an underflow.
            // To fix the issue, the code here is inlined so it gets compiled and linked with VS2019.
            // So only 1 version of std::unordered_map binary code is generated.
            void Output(Document& gltfDocument)
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

            const Buffer&     GetCurrentBuffer() const;
            const BufferView& GetCurrentBufferView() const;
            const Accessor&   GetCurrentAccessor() const;

            size_t GetBufferCount() const;
            size_t GetBufferViewCount() const;
            size_t GetAccessorCount() const;

            ResourceWriter& GetResourceWriter();
            const ResourceWriter& GetResourceWriter() const;

        private:
            const Accessor& AddAccessor(size_t count, AccessorDesc desc);

            std::unique_ptr<ResourceWriter> m_resourceWriter;

            IndexedContainer<Buffer>     m_buffers;
            IndexedContainer<BufferView> m_bufferViews;
            IndexedContainer<Accessor>   m_accessors;

            FnGenId m_fnGenBufferId;
            FnGenId m_fnGenBufferViewId;
            FnGenId m_fnGenAccessorId;
        };
    }
}
