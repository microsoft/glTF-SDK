// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <GLTFSDK/RapidJsonUtils.h>

namespace Microsoft
{
    namespace glTF
    {
        class ExtrasDocument
        {
        public:
            ExtrasDocument() = default;

            ExtrasDocument(const char* extras)
            {
                rapidjson::ParseResult result = m_document.Parse(extras);

                if (result.IsError())
                {
                    throw GLTFException(std::string("Extras JSON parse error: ") + rapidjson::GetParseError_En(result.Code()));
                }
            }

            template<typename T>
            T GetValueOrDefault(T t = {}) const
            {
                return glTF::GetValueOrDefault<T>(m_document, std::move(t));
            }

            template<typename T>
            T GetMemberValueOrDefault(const char* member, T t = {}) const
            {
                return glTF::GetMemberValueOrDefault<T>(m_document, member, std::move(t));
            }

            template<typename T>
            T GetPointerValueOrDefault(const char* pointer, T t = {}) const
            {
                auto valuePtr = rapidjson::Pointer(pointer).Get(m_document, nullptr);

                if (!valuePtr)
                {
                    return std::move(t);
                }
                return glTF::GetValueOrDefault<T>(*valuePtr, std::move(t));
            }

            template<typename T>
            void SetValue(const T& t)
            {
                SetValue(m_document, t, m_document.GetAllocator());
            }

            template<typename T>
            void SetMemberValue(const char* member, const T& t)
            {
                auto& allocator = m_document.GetAllocator();

                if (m_document.IsNull())
                {
                    m_document.SetObject();
                }

                if (m_document.IsObject())
                {
                    auto it = m_document.FindMember(member);

                    // If the member doesn't already exist then add it to the document with a null value
                    if (it == m_document.MemberEnd())
                    {
                        it = m_document.AddMember(rapidjson::Value(member, allocator), rapidjson::Value(), allocator).FindMember(member);
                    }

                    SetValue(it->value, t, allocator);
                }
                else
                {
                    throw GLTFException("Extras JSON document has already been assigned an incompatible type");
                }
            }

            template<typename T>
            void SetPointerValue(const char* pointer, const T& t)
            {
                SetValue(rapidjson::Pointer(pointer).Create(m_document), t, m_document.GetAllocator());
            }

            const rapidjson::Document& GetDocument() const
            {
                return m_document;
            }

        private:
            static void SwapValues(rapidjson::Value& valueOld, rapidjson::Value&& valueNew)
            {
                assert(!valueNew.IsNull());

                if (valueOld.IsNull() ||
                    valueOld.GetType() == valueNew.GetType())
                {
                    valueOld.Swap(valueNew);
                }
                else
                {
                    throw GLTFException("Extras JSON value has already been assigned an incompatible type");
                }
            }

            static void SetValue(rapidjson::Value& valueOld, const char* str, rapidjson::Document::AllocatorType& allocator)
            {
                SwapValues(valueOld, rapidjson::Value(str, allocator));
            }

            template<typename T>
            static void SetValue(rapidjson::Value& valueOld, const T& t, rapidjson::Document::AllocatorType&)
            {
                SwapValues(valueOld, rapidjson::Value(t));
            }

            rapidjson::Document m_document;
        };

        // Explicit specialization of SetValue for std::string (must be defined outside of the class definition)
        template<>
        inline void ExtrasDocument::SetValue<std::string>(rapidjson::Value& valueOld, const std::string& str, rapidjson::Document::AllocatorType& allocator)
        {
            SwapValues(valueOld, rapidjson::Value(str.c_str(), static_cast<rapidjson::SizeType>(str.length()), allocator));
        }
    }
}
