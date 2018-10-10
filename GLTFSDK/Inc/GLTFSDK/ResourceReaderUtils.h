// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <GLTFSDK/Exceptions.h>

#include <string>
#include <vector>

namespace Microsoft
{
    namespace glTF
    {
        // Documentation for RFC2397 base64 encoding:
        // https://en.wikipedia.org/wiki/Base64
        // https://tools.ietf.org/html/rfc2397

        constexpr char characterSet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        constexpr size_t CharCountToByteCount(size_t charCount)
        {
            return (charCount * 3U) / 4U;
        }

        constexpr size_t ByteCountToCharCount(size_t byteCount)
        {
            return (byteCount / 3U) * 4U;
        }

        constexpr size_t ByteCountToCharCountRemainder(size_t byteCount)
        {
            return byteCount % 3U;
        }

        // Note: intended to be used as a pass-by-value function parameter
        struct Base64StringView
        {
            Base64StringView(std::string::const_iterator itBegin_, std::string::const_iterator itEnd_) :
                itBegin(itBegin_),
                itEnd(itEnd_)
            {
                // Ignore trailing '=' characters
                while ((itBegin != itEnd) && (*(itEnd - 1) == '='))
                {
                    --itEnd;
                }
            }

            explicit Base64StringView(const std::string& string) : Base64StringView(string.begin(), string.end())
            {
            }

            size_t GetCharCount() const
            {
                return std::distance(itBegin, itEnd);
            }

            size_t GetByteCount() const
            {
                return CharCountToByteCount(GetCharCount());
            }

            std::string::const_iterator begin() const
            {
                return itBegin;
            }

            std::string::const_iterator end() const
            {
                return itEnd;
            }

            std::string::const_iterator itBegin;
            std::string::const_iterator itEnd;
        };

        // Note: intended to be used as a pass-by-value function parameter
        struct Base64BufferView
        {
            Base64BufferView(void* buffer, size_t bufferByteLength) :
                buffer(buffer),
                bufferByteLength(bufferByteLength)
            {
            }

            template<typename T>
            explicit Base64BufferView(std::vector<T>& decodedDataBuffer) : Base64BufferView(decodedDataBuffer.data(), decodedDataBuffer.size() * sizeof(T))
            {
            }

            void* const  buffer;
            const size_t bufferByteLength;
        };

        inline std::vector<uint8_t> GetDecodeTable()
        {
            std::vector<uint8_t> decodeTable(128, std::numeric_limits<uint8_t>::max());

            static constexpr size_t characterSetCount = std::extent<decltype(characterSet)>::value - 1U;

            for (size_t i = 0; i < characterSetCount; ++i)
            {
                decodeTable[static_cast<size_t>(characterSet[i])] = static_cast<uint8_t>(i);
            }

            return decodeTable;
        }

        inline void Base64Decode(Base64StringView encodedData, Base64BufferView decodedData, size_t bytesToSkip)
        {
            if (encodedData.GetByteCount() != (decodedData.bufferByteLength + bytesToSkip))
            {
                throw GLTFException("The specified decode buffer's size is incorrect");
            }

            static const std::vector<uint8_t> decodeTable = GetDecodeTable();

            uint32_t block = 0U;
            uint32_t blockBits = 0U;

            uint8_t* decodedBytePtr = static_cast<uint8_t*>(decodedData.buffer);

            for (const auto encodedChar : encodedData)
            {
                // For platforms where char is unsigned (encodedChar < 0) results in a tautology warning.
                // Only do that test on platforms where char is signed.
                if ((std::numeric_limits<char>::is_signed && (static_cast<signed char>(encodedChar) < 0)) || (static_cast<size_t>(encodedChar) >= decodeTable.size()))
                {
                    throw GLTFException("Invalid base64 character");
                }

                const auto decodedChar = decodeTable[encodedChar];

                if (decodedChar == std::numeric_limits<uint8_t>::max())
                {
                    throw GLTFException("Invalid base64 character");
                }

                // Each character of a base64 string encodes 6 bits of data so left shift any remaining
                // bits to accomodate another character's worth of data before performing a bitwise OR
                block <<= 6U;
                block |= decodedChar & 0x3F;

                // Keep track of how many bits of the 'block' variable are currently used
                blockBits += 6U;

                // If there are 8 or more bits stored in 'block' then write a single byte to the output buffer
                if (blockBits >= 8U)
                {
                    blockBits -= 8U;

                    if (bytesToSkip > 0)
                    {
                        bytesToSkip--;
                    }
                    else
                    {
                        // Right shift the decoded data stored in 'block' so that the byte of
                        // data to be written to the output buffer occupies the low-order byte
                        *(decodedBytePtr++) = (block >> blockBits) & 0xFF;
                    }

                    // Generate a bitmask that discards only the byte just written to the output buffer during the bitwise AND
                    block &= (1 << blockBits) - 1;
                }
            }
        }

        inline std::vector<uint8_t> Base64Decode(const Base64StringView& encodedData)
        {
            std::vector<uint8_t> decodedDataBuffer(encodedData.GetByteCount());

            Base64BufferView decodedData(decodedDataBuffer);
            Base64Decode(encodedData, decodedData, 0);

            return decodedDataBuffer;
        }

        inline std::vector<uint8_t> Base64Decode(const std::string& encodedData)
        {
            return Base64Decode(Base64StringView(encodedData));
        }

        inline bool IsUriBase64(const std::string& uri, std::string::const_iterator& itBegin, std::string::const_iterator& itEnd)
        {
            // A valid base64 data URI must begin with "data:"
            // and contain ";base64," somewhere in the string

            static const char dataPrefix[] = "data:";
            constexpr size_t dataPrefixLength = std::extent<decltype(dataPrefix)>::value - 1;

            static const char base64Indicator[] = ";base64,";
            constexpr size_t base64IndicatorLength = std::extent<decltype(base64Indicator)>::value - 1;

            if (uri.compare(0, dataPrefixLength, dataPrefix) == 0)
            {
                const size_t findIndex = uri.find(base64Indicator);

                if (findIndex != std::string::npos)
                {
                    itBegin = uri.begin() + findIndex + base64IndicatorLength;
                    itEnd = uri.end();

                    return true;
                }
            }

            return false;
        }

        inline bool IsUriBase64(const std::string& uri)
        {
            std::string::const_iterator itBegin;
            std::string::const_iterator itEnd;

            return IsUriBase64(uri, itBegin, itEnd);
        }
    }
}
