// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <GLTFSDK/ResourceWriter.h>

#include <memory>

namespace Microsoft
{
    namespace glTF
    {
        class IStreamWriter;

        class GLTFResourceWriter : public ResourceWriter
        {
        public:
            GLTFResourceWriter(std::shared_ptr<const IStreamWriter> streamWriter);
            GLTFResourceWriter(std::unique_ptr<IStreamWriterCache> streamCache);

            std::string GenerateBufferUri(const std::string& bufferId) const override;
            void SetUriPrefix(std::string uriPrefix);

        protected:
            std::ostream*  GetBufferStream(const std::string& bufferId) override;
            std::streamoff GetBufferOffset(const std::string& bufferId) override;
            void           SetBufferOffset(const std::string& bufferId, std::streamoff offset) override;

        private:
            std::string m_uriPrefix;

            std::unordered_map<std::string, std::streamoff> m_streamOffsets;
        };
    }
}
