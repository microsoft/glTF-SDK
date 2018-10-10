// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <istream>
#include <ostream>

namespace Microsoft
{
    namespace glTF
    {
        class StreamUtils
        {
        public:
            template<typename T>
            static size_t WriteBinary(std::ostream& stream, const T& value)
            {
                return WriteBinary(stream, &value, sizeof(value));
            }

            static size_t WriteBinary(std::ostream& stream, const std::string& str)
            {
                return WriteBinary(stream, str.c_str(), str.length());
            }

            template<typename T>
            static size_t WriteBinary(std::ostream& stream, const std::vector<T>& values)
            {
                return WriteBinary(stream, values.data(), values.size() * sizeof(T));
            }

            template<typename T>
            static size_t WriteBinary(std::ostream& stream, const T* data, size_t size)
            {
                if (!stream.good())
                {
                    throw std::runtime_error("The output stream is not in a good state.");
                }

                stream.write(reinterpret_cast<const char *>(data), size);

                if (stream.fail())
                {
                    throw std::runtime_error("Unable to write to buffer.");
                }

                return size;
            }

            template<typename T>
            static T ReadBinary(std::istream& stream)
            {
                T value;
                ReadBinary(stream, reinterpret_cast<char*>(&value), sizeof(T));
                return value;
            }

            template<typename T>
            static std::vector<T> ReadBinaryFull(std::istream& stream)
            {
                if (stream.fail())
                {
                    throw std::runtime_error("Cannot read the binary data");
                }

                stream.seekg(0, std::ios::end);
                auto size = static_cast<size_t>(stream.tellg());
                stream.seekg(0, std::ios::beg);

                std::vector<T> data(size);
                ReadBinary(stream, reinterpret_cast<char*>(data.data()), size);
                return data;
            }

            static void ReadBinary(std::istream& stream, char* data, size_t size)
            {
                stream.read(data, size);

                if (stream.fail())
                {
                    throw std::runtime_error("Cannot read the binary data");
                }
            }
        };
    }
}
