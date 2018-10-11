// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <GLTFSDK/Document.h>
#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/IStreamCache.h>
#include <GLTFSDK/StreamUtils.h>

#include <memory>

namespace Microsoft
{
    namespace glTF
    {
        // The task of populating a glTF document with valid Buffer, BufferView
        // and Accessor instances is the responsibility of other, higher-level,
        // APIs (e.g. BufferBuilder).
        class ResourceWriter
        {
        public:
            virtual ~ResourceWriter();

            virtual std::string GenerateBufferUri(const std::string& bufferId) const = 0;

            void Write(const BufferView& bufferView, const void* data);
            void Write(const BufferView& bufferView, const void* data, const Accessor& accessor);

            template<typename T>
            void Write(const BufferView& bufferView, const std::vector<T>& data)
            {
                const auto byteLength = data.size() * sizeof(T);

                if (byteLength != bufferView.byteLength)
                {
                    throw InvalidGLTFException("The given vector's size in bytes doesn't equal bufferView.byteLength");
                }

                Write(bufferView, data.data());
            }

            template<typename T>
            void Write(const BufferView& bufferView, const std::vector<T>& data, const Accessor& accessor)
            {
                const auto byteLength = data.size() * sizeof(T);

                if (byteLength != accessor.GetByteLength())
                {
                    throw InvalidGLTFException("The given vector's size in bytes doesn't equal the accessor's byte length");
                }

                Write(bufferView, data.data(), accessor);
            }

            // Writes data to an output stream without referencing a Buffer, BufferView or
            // Accessor. Useful for outputting image data to an external resource (in this
            // case the Image instance must use the uri property).
            void WriteExternal(const std::string& uri, const void* data, size_t byteLength) const;

            void WriteExternal(const std::string& uri, const std::string& data) const;

            template<typename T>
            void WriteExternal(const std::string& uri, const std::vector<T>& data) const
            {
                WriteExternal(uri, data.data(), data.size() * sizeof(T));
            }

        protected:
            ResourceWriter(std::unique_ptr<IStreamWriterCache> streamWriter);

            virtual std::ostream*  GetBufferStream(const std::string& bufferId) = 0;
            virtual std::streamoff GetBufferOffset(const std::string& bufferId) = 0;
            virtual void           SetBufferOffset(const std::string& bufferId, std::streamoff offset) = 0;

            std::unique_ptr<IStreamWriterCache> m_streamWriterCache;

        private:
            void WriteImpl(const BufferView& bufferView, const void* data, std::streamoff totalOffset, size_t totalByteLength);
        };
    }
}