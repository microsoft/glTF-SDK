// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <GLTFSDK/GLTFResourceReader.h>

namespace Microsoft
{
    namespace glTF
    {
        class GLBResourceReader : public GLTFResourceReader
        {
        public:
            GLBResourceReader(std::shared_ptr<const IStreamReader> streamReader, std::shared_ptr<std::istream> glbStream);
            GLBResourceReader(std::unique_ptr<IStreamReaderCache> streamCache, std::shared_ptr<std::istream> glbStream);

            std::shared_ptr<std::istream> GetBinaryStream(const Buffer& buffer) const override;
            std::streampos                GetBinaryStreamPos(const Buffer& buffer) const override;

            const std::string& GetJson() const;

        private:
            void Init();

            std::string m_json;

            std::shared_ptr<std::istream> m_buffer;
            std::streamoff                m_bufferOffset;
        };
    }
}
