// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLTFResourceWriter.h>

#include <memory>

namespace Microsoft
{
    namespace glTF
    {
        class GLBResourceWriter : public GLTFResourceWriter
        {
        public:
            GLBResourceWriter(std::shared_ptr<const IStreamWriter> streamWriter);
            GLBResourceWriter(std::shared_ptr<const IStreamWriter> streamWriter, std::unique_ptr<std::iostream> tempBufferStream);
            GLBResourceWriter(std::unique_ptr<IStreamWriterCache> streamCache);
            GLBResourceWriter(std::unique_ptr<IStreamWriterCache> streamCache, std::unique_ptr<std::iostream> tempBufferStream);

            void Flush(const std::string& manifest, const std::string& uri);
            std::string GenerateBufferUri(const std::string& bufferId) const override;
            std::ostream* GetBufferStream(const std::string& bufferId) override;

        private:
            std::shared_ptr<std::iostream> m_stream;
        };
    }
}
