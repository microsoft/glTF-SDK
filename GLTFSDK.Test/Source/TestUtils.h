// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <GLTFSDK/BufferBuilder.h>
#include <GLTFSDK/GLTFResourceReader.h>
#include <GLTFSDK/GLTFResourceWriter.h>
#include <GLTFSDK/IStreamReader.h>
#include <GLTFSDK/IStreamWriter.h>

#include <fstream>
#include <memory>
#include <unordered_map>
#include <sstream>

using namespace glTF::UnitTest;

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            template <typename T>
            static void AreEqual(const std::vector<T>& a, const std::vector<T>& b, wchar_t const* message = nullptr)
            {
                Assert::IsTrue(a == b, message);
            }

            class StreamReaderWriter : public Microsoft::glTF::IStreamWriter, public Microsoft::glTF::IStreamReader
            {
            public:
                StreamReaderWriter()
                    : m_streams()
                {
                }

                std::shared_ptr<std::ostream> GetOutputStream(const std::string& uri) const override
                {
                    return GetStream(uri);
                }

                std::shared_ptr<std::istream> GetInputStream(const std::string& uri) const override
                {
                    return GetStream(uri);
                }
            private:
                std::shared_ptr<std::iostream> GetStream(const std::string& uri) const
                {
                    if (m_streams.find(uri) == m_streams.end())
                    {
                        m_streams[uri] = std::make_shared<std::stringstream>();
                    }
                    return m_streams[uri];
                }

                mutable std::unordered_map<std::string, std::shared_ptr<std::stringstream>> m_streams;
            };

            inline std::string GetAbsolutePath(const char * relativePath)
            {
#ifndef _WIN32
                // Leaving Win32 alone (below), but macOS and Android requires working directory to be set 
                std::string finalPath(relativePath);
                std::replace(finalPath.begin(), finalPath.end(), '\\', '/');
                return finalPath;
#else
                std::string currentPath = __FILE__;
                std::string sourcePath = currentPath.substr(0, currentPath.rfind('\\'));
                std::string resourcePath = sourcePath.substr(0, sourcePath.rfind('\\'));
                std::string finalPath = resourcePath + "\\" + relativePath;
                return finalPath;
#endif
            }

            inline std::shared_ptr<std::stringstream> ReadLocalAsset(const std::string& relativePath)
            {
                auto filename = GetAbsolutePath(relativePath.c_str());

                // Read local file
                int64_t m_readPosition = 0;
                std::shared_ptr<const std::vector<int8_t>> m_buffer;
                std::ifstream ifs;
                ifs.open(filename.c_str(), std::ifstream::in | std::ifstream::binary);
                if (ifs.is_open())
                {
                    std::streampos start = ifs.tellg();
                    ifs.seekg(0, std::ios::end);
                    m_buffer = std::make_shared<const std::vector<int8_t>>(static_cast<unsigned int>(ifs.tellg() - start));
                    ifs.seekg(0, std::ios::beg);
                    ifs.read(reinterpret_cast<char*>(const_cast<int8_t*>(m_buffer->data())), m_buffer->size());
                    ifs.close();
                }
                else
                {
                    throw std::runtime_error("Could not open the file for reading");
                }

                // To IStream
                unsigned long writeBufferLength = 4096L * 1024L;
                auto tempStream = std::make_shared<std::stringstream>();
                auto tempBuffer = new char[writeBufferLength];
                // Read the file for as long as we can fill the buffer completely.
                // This means there is more content to be read.
                unsigned long bytesRead;
                do
                {
                    auto bytesAvailable = m_buffer->size() - m_readPosition;
                    unsigned long br = std::min(static_cast<unsigned long>(bytesAvailable), writeBufferLength);
#ifdef _WIN32
                    memcpy_s(tempBuffer, br, m_buffer->data() + m_readPosition, br);
#else
                    memcpy(tempBuffer, m_buffer->data() + m_readPosition, br);
#endif
                    m_readPosition += br;
                    bytesRead = br;

                    tempStream->write(tempBuffer, bytesRead);
                } while (bytesRead == writeBufferLength);

                delete[] tempBuffer;

                if (tempStream.get()->bad())
                {
                    throw std::runtime_error("Bad std::stringstream after copying the file");
                }

                return tempStream;
            }

            inline std::string ReadLocalJson(const char * relativePath)
            {
                auto input = ReadLocalAsset(relativePath);
                auto json = std::string(std::istreambuf_iterator<char>(*input), std::istreambuf_iterator<char>());
                return json;
            }
        }
    }
}