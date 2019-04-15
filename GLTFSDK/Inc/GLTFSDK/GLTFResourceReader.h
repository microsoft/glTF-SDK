// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <GLTFSDK/Document.h>
#include <GLTFSDK/IStreamReader.h>
#include <GLTFSDK/ResourceReaderUtils.h>
#include <GLTFSDK/StreamCacheLRU.h>
#include <GLTFSDK/StreamUtils.h>
#include <GLTFSDK/Validation.h>

#include <cassert>

namespace Microsoft
{
    namespace glTF
    {
        class GLTFResourceReader
        {
        public:
            GLTFResourceReader(std::shared_ptr<const IStreamReader> streamReader)
                : GLTFResourceReader(MakeStreamReaderCache<StreamReaderCacheLRU>(std::move(streamReader), 16U))
            {
            }

            GLTFResourceReader(std::unique_ptr<IStreamReaderCache> streamCache)
                : m_streamReaderCache(std::move(streamCache))
            {
            }

            GLTFResourceReader(GLTFResourceReader&&) = default;

            virtual ~GLTFResourceReader() = default;

            // TODO: return mimeType of image
            std::vector<uint8_t> ReadBinaryData(const Document& document, const Image& image) const
            {
                std::vector<uint8_t> data;

                std::string::const_iterator itBegin;
                std::string::const_iterator itEnd;

                if (image.uri.empty())
                {
                    if (image.bufferViewId.empty())
                    {
                        throw GLTFException("Invalid image, both uri and bufferView are unspecified");
                    }

                    data = ReadBinaryData<uint8_t>(document, document.bufferViews.Get(image.bufferViewId));
                }
                else if (IsUriBase64(image.uri, itBegin, itEnd))
                {
                    data = ReadBinaryDataUri<uint8_t>({ itBegin, itEnd });
                }
                else if (auto stream = m_streamReaderCache->Get(image.uri))
                {
                    data = StreamUtils::ReadBinaryFull<uint8_t>(*stream);
                }
                else
                {
                    throw GLTFException("Unable to read image data");
                }

                return data;
            }

            template<typename T>
            std::vector<T> ReadBinaryData(const Document& gltfDocument, const Accessor& accessor) const
            {
                bool isValid;

                switch (accessor.componentType)
                {
                case COMPONENT_BYTE:
                    isValid = std::is_same<T, int8_t>::value;
                    break;
                case COMPONENT_UNSIGNED_BYTE:
                    isValid = std::is_same<T, uint8_t>::value;
                    break;
                case COMPONENT_SHORT:
                    isValid = std::is_same<T, int16_t>::value;
                    break;
                case COMPONENT_UNSIGNED_SHORT:
                    isValid = std::is_same<T, uint16_t>::value;
                    break;
                case COMPONENT_UNSIGNED_INT:
                    isValid = std::is_same<T, uint32_t>::value;
                    break;
                case COMPONENT_FLOAT:
                    isValid = std::is_same<T, float>::value;
                    break;
                default:
                    throw GLTFException("Unsupported accessor ComponentType");
                }

                if (!isValid)
                {
                    throw GLTFException("ReadAccessorData: Template type T does not match accessor ComponentType");
                }

                Validation::ValidateAccessor(gltfDocument, accessor);

                if (accessor.sparse.count > 0U)
                {
                    return ReadSparseAccessor<T>(gltfDocument, accessor);
                }

                return ReadAccessor<T>(gltfDocument, accessor);
            }

            template<typename T>
            std::vector<T> ReadBinaryData(const Document& document, const BufferView& bufferView) const
            {
                const Buffer& buffer = document.buffers.Get(bufferView.bufferId);

                Validation::ValidateBufferView(bufferView, buffer);

                auto count = bufferView.byteLength / sizeof(T);
                assert(bufferView.byteLength % sizeof(T) == 0);

                return ReadBinaryData<T>(buffer, bufferView.byteOffset, count);
            }

        protected:
            template<typename T>
            std::vector<T> ReadAccessor(const Document& gltfDocument, const Accessor& accessor) const
            {
                const auto typeCount = Accessor::GetTypeCount(accessor.type);
                const auto elementSize = sizeof(T) * typeCount;

                std::vector<T> data;

                const BufferView& bufferView = gltfDocument.bufferViews.Get(accessor.bufferViewId);
                const Buffer& buffer = gltfDocument.buffers.Get(bufferView.bufferId);

                const size_t offset = accessor.byteOffset + bufferView.byteOffset;

                if (!bufferView.byteStride || bufferView.byteStride.Get() == elementSize)
                {
                    data = ReadBinaryData<T>(buffer, offset, accessor.count * typeCount);
                }
                else
                {
                    data = ReadBinaryDataInterleaved<T>(buffer, offset, accessor.count, typeCount, bufferView.byteStride.Get());
                }

                return data;
            }

            template<typename T>
            std::vector<T> ReadSparseAccessor(const Document& gltfDocument, const Accessor& accessor) const
            {
                const auto typeCount = Accessor::GetTypeCount(accessor.type);
                const auto elementSize = sizeof(T) * typeCount;

                std::vector<T> baseData;

                if (accessor.bufferViewId.empty())
                {
                    baseData.resize(typeCount * accessor.count, T());
                }
                else
                {
                    const BufferView& bufferView = gltfDocument.bufferViews.Get(accessor.bufferViewId);
                    const Buffer& buffer = gltfDocument.buffers.Get(bufferView.bufferId);

                    const size_t offset = accessor.byteOffset + bufferView.byteOffset;

                    if (!bufferView.byteStride || bufferView.byteStride.Get() == elementSize)
                    {
                        baseData = ReadBinaryData<T>(buffer, offset, accessor.count * typeCount);
                    }
                    else
                    {
                        baseData = ReadBinaryDataInterleaved<T>(buffer, offset, accessor.count, typeCount, bufferView.byteStride.Get());
                    }
                }

                switch (accessor.sparse.indicesComponentType)
                {
                case COMPONENT_UNSIGNED_BYTE:
                    ReadSparseBinaryData<T, uint8_t>(gltfDocument, baseData, accessor);
                    break;
                case COMPONENT_UNSIGNED_SHORT:
                    ReadSparseBinaryData<T, uint16_t>(gltfDocument, baseData, accessor);
                    break;
                case COMPONENT_UNSIGNED_INT:
                    ReadSparseBinaryData<T, uint32_t>(gltfDocument, baseData, accessor);
                    break;
                default:
                    throw GLTFException("Unsupported sparse indices ComponentType");
                }

                return baseData;
            }

            virtual std::shared_ptr<std::istream> GetBinaryStream(const Buffer& buffer) const
            {
                if (buffer.uri.empty())
                {
                    throw GLTFException("Buffer.uri was not specified.");
                }

                return m_streamReaderCache->Get(buffer.uri);
            }

            virtual std::streampos GetBinaryStreamPos(const Buffer&) const
            {
                return {};
            }

        private:
            void ReadBinaryDataUri(Base64StringView encodedData, Base64BufferView decodedData, const std::streamoff* offsetOverride = nullptr) const
            {
                // The number of unwanted extra bytes that must be decoded for the specified byte offset
                size_t offsetAdjustment = 0;

                if (offsetOverride)
                {
                    if (*offsetOverride < 0)
                    {
                        throw GLTFException("Negative offsets are not supported");
                    }

                    auto byteCount = static_cast<size_t>(*offsetOverride);
                    offsetAdjustment = ByteCountToCharCountRemainder(byteCount);
                    const size_t offsetBegin = ByteCountToCharCount(byteCount);

                    if (offsetBegin < encodedData.GetCharCount())
                    {
                        encodedData.itBegin += offsetBegin;
                    }
                    else
                    {
                        throw GLTFException("Offset position as a base64 character index is outside the input range");
                    }
                }

                auto offsetByteCount = decodedData.bufferByteLength + offsetAdjustment;
                size_t offsetEnd = ByteCountToCharCount(offsetByteCount);

                switch (ByteCountToCharCountRemainder(offsetByteCount))
                {
                case 1U:
                    offsetEnd += 2U;// Decode 2 more characters for 1 extra byte
                    break;
                case 2U:
                    offsetEnd += 3U;// Decode 3 more characters for 2 extra bytes
                    break;
                }

                if (offsetEnd <= encodedData.GetCharCount())
                {
                    encodedData.itEnd = encodedData.itBegin + offsetEnd;
                }
                else
                {
                    throw GLTFException("Offset position as a base64 character index is outside the input range");
                }

                Base64Decode(encodedData, decodedData, offsetAdjustment);
            }

            template<typename T>
            std::vector<T> ReadBinaryDataUri(Base64StringView encodedData, const std::streamoff* offsetOverride = nullptr, const size_t* componentCountOverride = nullptr) const
            {
                size_t componentCount;

                if (componentCountOverride)
                {
                    componentCount = *componentCountOverride;
                }
                else
                {
                    componentCount = encodedData.GetByteCount() / sizeof(T);
                }

                std::vector<T> decodedData(componentCount);
                ReadBinaryDataUri(encodedData, Base64BufferView(decodedData), offsetOverride);
                return decodedData;
            }

            template<typename T>
            std::vector<T> ReadBinaryData(const Buffer& buffer, std::streamoff offset, size_t componentCount) const
            {
                std::vector<T> data;

                std::string::const_iterator itBegin;
                std::string::const_iterator itEnd;

                if (IsUriBase64(buffer.uri, itBegin, itEnd))
                {
                    data = ReadBinaryDataUri<T>({ itBegin, itEnd }, &offset, &componentCount);
                }
                else
                {
                    data.resize(componentCount);

                    auto bufferStream = GetBinaryStream(buffer);
                    auto bufferStreamPos = GetBinaryStreamPos(buffer);

                    bufferStream->seekg(bufferStreamPos);
                    bufferStream->seekg(offset, std::ios_base::cur);

                    StreamUtils::ReadBinary(*bufferStream, reinterpret_cast<char*>(data.data()), componentCount * sizeof(T));
                }

                return data;
            }

            template<typename T>
            std::vector<T> ReadBinaryDataInterleaved(const Buffer& buffer, std::streamoff offset, size_t elementCount, uint8_t typeCount, size_t stride) const
            {
                const size_t elementSize = sizeof(T) * typeCount;
                const size_t componentCount = elementCount * typeCount;

                std::vector<T> data(componentCount);

                std::string::const_iterator itBegin;
                std::string::const_iterator itEnd;

                if (IsUriBase64(buffer.uri, itBegin, itEnd))
                {
                    Base64StringView encodedData(itBegin, itEnd);

                    for (size_t componentsRead = 0U; componentsRead < componentCount; componentsRead += typeCount, offset += stride)
                    {
                        ReadBinaryDataUri(encodedData, Base64BufferView(data.data() + componentsRead, elementSize), &offset);
                    }
                }
                else
                {
                    auto bufferStream = GetBinaryStream(buffer);
                    auto bufferStreamPos = GetBinaryStreamPos(buffer) + offset;

                    for (size_t componentsRead = 0U; componentsRead < componentCount; componentsRead += typeCount)
                    {
                        bufferStream->seekg(bufferStreamPos);
                        bufferStreamPos += stride;

                        StreamUtils::ReadBinary(*bufferStream, reinterpret_cast<char*>(data.data() + componentsRead), elementSize);
                    }
                }

                return data;
            }

            template<typename T, typename I>
            void ReadSparseBinaryData(const Document& gltfDocument, std::vector<T>& baseData, const Accessor& accessor) const
            {
                const auto typeCount = Accessor::GetTypeCount(accessor.type);
                const auto elementSize = sizeof(T) * typeCount;

                const size_t count = accessor.sparse.count;

                const BufferView& indicesBufferView = gltfDocument.bufferViews.Get(accessor.sparse.indicesBufferViewId);
                const Buffer& indicesBuffer = gltfDocument.buffers.Get(indicesBufferView.bufferId);
                const size_t indicesOffset = accessor.sparse.indicesByteOffset + indicesBufferView.byteOffset;

                const BufferView& valuesBufferView = gltfDocument.bufferViews.Get(accessor.sparse.valuesBufferViewId);
                const Buffer& valuesBuffer = gltfDocument.buffers.Get(valuesBufferView.bufferId);
                const size_t valuesOffset = accessor.sparse.valuesByteOffset + valuesBufferView.byteOffset;

                std::vector<I> indices;

                if (!indicesBufferView.byteStride || indicesBufferView.byteStride.Get() == sizeof(I))
                {
                    indices = ReadBinaryData<I>(indicesBuffer, indicesOffset, count);
                }
                else
                {
                    indices = ReadBinaryDataInterleaved<I>(indicesBuffer, indicesOffset, count, 1U, indicesBufferView.byteStride.Get());
                }

                std::vector<T> values;

                if (!valuesBufferView.byteStride || valuesBufferView.byteStride.Get() == elementSize)
                {
                    values = ReadBinaryData<T>(valuesBuffer, valuesOffset, count * typeCount);
                }
                else
                {
                    values = ReadBinaryDataInterleaved<T>(valuesBuffer, valuesOffset, count, typeCount, valuesBufferView.byteStride.Get());
                }

                for (size_t i = 0; i < indices.size(); i++)
                {
                    for (size_t j = 0; j < typeCount; j++)
                    {
                        baseData[indices[i] * typeCount + j] = values[i * typeCount + j];
                    }
                }
            }

            std::unique_ptr<IStreamReaderCache> m_streamReaderCache;
        };
    }
}
