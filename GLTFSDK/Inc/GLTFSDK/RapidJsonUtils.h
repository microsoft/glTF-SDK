// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#if defined(RAPIDJSON_NAMESPACE) || defined(RAPIDJSON_NAMESPACE_BEGIN) || defined(RAPIDJSON_NAMESPACE_END)
#error GLTFSDK: another library has likely included rapidjson first and predefined rapidjson namespace macros \
This could result in compiling or linking with a different version of rapidjson than we expect. \
This is know to cause runtime errors if different versions of rapidjson are included in the same namespace. \
GLTFSDK code should only include rapidjson using this header. \
If this is being included from another library then it is important to separate the code in that library from our own use of rapidjson. \
Please do not include their headers and this header in the same cpp file.
#endif


#define RAPIDJSON_NAMESPACE Microsoft::glTF::rapidjson
#define RAPIDJSON_NAMESPACE_BEGIN  namespace Microsoft { namespace glTF { namespace rapidjson {
#define RAPIDJSON_NAMESPACE_END }}}


// RapidJSON uses constant if expressions to support multiple platforms
#pragma warning(push)
#pragma warning(disable:4127)
#include <rapidjson/schema.h>
#pragma warning(pop)

// This file is used as a catch-all for including rapidjson headers in the SDK. 
// If desired header not found below, please add it and include this file instead of the header directly
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/pointer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include <GLTFSDK/Color.h>
#include <GLTFSDK/Exceptions.h>
#include <GLTFSDK/IndexedContainer.h>
#include <GLTFSDK/Math.h>

#include <vector>
#include <string>
#include <istream>

namespace Microsoft
{
    namespace glTF
    {
        // Other platforms may define size_t as unsigned long and the rapidjson doesn't support that very well.
        // We create a type below which is uint32_t if unsigned long is the same sizeof, or uint64_t if it is not.
        // Then we use this to type-convert size_t to a type that rapidjson can take directly.
        constexpr const bool IsUnsignedLongSizeofUInt32 = sizeof(unsigned long) == sizeof(uint32_t);
        typedef std::conditional<IsUnsignedLongSizeofUInt32, uint32_t, uint64_t>::type UnsignedLongKnownSizeType;
        typedef std::conditional<std::is_same<std::size_t, unsigned long>::value, UnsignedLongKnownSizeType, std::size_t>::type KnownSizeType;

        template<typename T>
        std::string GetMemberValueAsString(const rapidjson::Value& v, const char* memberName)
        {
            auto it = v.FindMember(memberName);

            if (it != v.MemberEnd())
            {
                return std::to_string(it->value.Get<T>());
            }
            else
            {
                return {};
            }
        }

        template<>
        inline std::string GetMemberValueAsString<std::size_t>(const rapidjson::Value& v, const char* memberName)
        {
            auto it = v.FindMember(memberName);

            if (it != v.MemberEnd())
            {
                return std::to_string(it->value.Get<KnownSizeType>());
            }
            else
            {
                return {};
            }
        }

        inline bool TryFindMember(const char* name, const rapidjson::Value& v, rapidjson::Value::ConstMemberIterator& value)
        {
            rapidjson::Value::ConstMemberIterator it = v.FindMember(name);
            value = it;
            return it != v.MemberEnd();
        }

        inline rapidjson::Value::ConstMemberIterator FindRequiredMember(const char* name, const rapidjson::Value& v)
        {
            const rapidjson::Value::ConstMemberIterator it = v.FindMember(name);
            if (it == v.MemberEnd())
            {
                throw InvalidGLTFException("The member " + std::string(name) + " was not found");
            }
            return it;
        }

        template<typename T>
        T GetValueOrDefault(const rapidjson::Value& v, T t = {})
        {
            return v.Is<T>() ? v.Get<T>() : std::move(t);
        }

        template<>
        inline std::string GetValueOrDefault<std::string>(const rapidjson::Value& v, std::string str)
        {
            return v.Is<const char*>() ? v.GetString() : std::move(str);
        }

        template<>
        inline float GetValueOrDefault<float>(const rapidjson::Value& v, float defaultValue)
        {
            return v.IsNumber() ? v.GetFloat() : defaultValue;
        }

        inline std::string Serialize(const rapidjson::Value& v)
        {
            rapidjson::StringBuffer stringBuffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);
            v.Accept(writer);
            return stringBuffer.GetString();
        }

        template<>
        inline std::size_t GetValueOrDefault<std::size_t>(const rapidjson::Value& v, std::size_t t)
        {
            return v.Is<KnownSizeType>() ? v.Get<KnownSizeType>() : std::move(t);
        }

        template<typename T>
        T GetMemberValueOrDefault(const rapidjson::Value& v, const char* memberName, T t = {})
        {
            auto it = v.FindMember(memberName);
            if (it != v.MemberEnd())
            {
                return GetValueOrDefault<T>(it->value, std::move(t));
            }
            return std::move(t);
        }

        inline KnownSizeType ToKnownSizeType(std::size_t v)
        {
            return static_cast<KnownSizeType>(v);
        }

        template <typename T>
        T GetValue(const rapidjson::Value& v)
        {
            return v.Get<T>();
        }

        template <>
        inline std::size_t GetValue<std::size_t>(const rapidjson::Value& v)
        {
            return static_cast<std::size_t>(v.Get<KnownSizeType>());
        }

        namespace RapidJsonUtils
        {
            inline rapidjson::Value ToStringValue(const std::string& str, rapidjson::Document::AllocatorType& a)
            {
                rapidjson::Value v;
                v.SetString(str.c_str(), static_cast<uint32_t>(str.length()), a);
                return v;
            }

            inline rapidjson::GenericStringRef<char> ToStringRef(const std::string& str)
            {
                return rapidjson::StringRef(str.c_str());
            }

            inline rapidjson::Value ToFloatValue(const float f)
            {
                rapidjson::Value v;
                v.SetFloat(f);
                return v;
            }

            inline std::vector<float> ToFloatArray(const rapidjson::Value& v, const std::string& key)
            {
                std::vector<float> result;
                const auto& it = v.FindMember(key.c_str());
                if (it != v.MemberEnd())
                {
                    for (rapidjson::Value::ConstValueIterator ait = it->value.Begin(); ait != it->value.End(); ++ait)
                    {
                        result.push_back(static_cast<float>(ait->GetDouble()));
                    }
                }
                return result;
            }

            template<typename T>
            inline rapidjson::Value ToJsonArray(const std::vector<T>& v, rapidjson::Document::AllocatorType& a)
            {
                rapidjson::Value vArray(rapidjson::kArrayType);
                for (size_t i = 0; i < v.size(); ++i)
                {
                    vArray.PushBack(v[i], a);
                }
                return vArray;
            }

            inline rapidjson::Value ToJsonArray(const std::vector<size_t>& v, rapidjson::Document::AllocatorType& a)
            {
                rapidjson::Value vArray(rapidjson::kArrayType);
                for (size_t i = 0; i < v.size(); ++i)
                {
                    vArray.PushBack(ToKnownSizeType(v[i]), a);
                }
                return vArray;
            }

            inline rapidjson::Value ToJsonArray(const std::vector<std::string>& v, rapidjson::Document::AllocatorType& a)
            {
                rapidjson::Value vArray(rapidjson::kArrayType);
                for (size_t i = 0; i < v.size(); ++i)
                {
                    vArray.PushBack(ToStringValue(v[i], a), a);
                }
                return vArray;
            }

            template<typename T, std::size_t N>
            inline rapidjson::Value ToJsonArray(const std::array<T, N>& v, rapidjson::Document::AllocatorType& a)
            {
                rapidjson::Value vArray(rapidjson::kArrayType);
                for (size_t i = 0; i < v.size(); ++i)
                {
                    vArray.PushBack(v[i], a);
                }
                return vArray;
            }

            inline glTF::rapidjson::Value ToJsonArray(const Vector2& v, rapidjson::Document::AllocatorType& a)
            {
                rapidjson::Value vArray(rapidjson::kArrayType);
                vArray.PushBack(v.x, a);
                vArray.PushBack(v.y, a);
                return vArray;
            }

            inline rapidjson::Value ToJsonArray(const Vector3& v, rapidjson::Document::AllocatorType& a)
            {
                rapidjson::Value vArray(rapidjson::kArrayType);
                vArray.PushBack(v.x, a);
                vArray.PushBack(v.y, a);
                vArray.PushBack(v.z, a);
                return vArray;
            }

            inline rapidjson::Value ToJsonArray(const Quaternion& v, rapidjson::Document::AllocatorType& a)
            {
                rapidjson::Value vArray(rapidjson::kArrayType);
                vArray.PushBack(v.x, a);
                vArray.PushBack(v.y, a);
                vArray.PushBack(v.z, a);
                vArray.PushBack(v.w, a);
                return vArray;
            }

            inline rapidjson::Value ToJsonArray(const Color4& v, rapidjson::Document::AllocatorType& a)
            {
                rapidjson::Value vArray(rapidjson::kArrayType);
                vArray.PushBack(v.r, a);
                vArray.PushBack(v.g, a);
                vArray.PushBack(v.b, a);
                vArray.PushBack(v.a, a);
                return vArray;
            }

            inline rapidjson::Value ToJsonArray(const Color3& v, rapidjson::Document::AllocatorType& a)
            {
                rapidjson::Value vArray(rapidjson::kArrayType);
                vArray.PushBack(v.r, a);
                vArray.PushBack(v.g, a);
                vArray.PushBack(v.b, a);
                return vArray;
            }

            inline void AddMember(rapidjson::Value& v, const std::string& key, const std::string& value, rapidjson::Document::AllocatorType& a)
            {
                v.AddMember(ToStringValue(key, a), ToStringValue(value, a), a);
            }

            inline void AddOptionalMember(const std::string& name, rapidjson::Value& v, const std::string& source, rapidjson::Document::AllocatorType& a)
            {
                if (!source.empty())
                {
                    v.AddMember(ToStringValue(name, a), ToStringValue(source, a), a);
                }
            }

            // Adds a member called `name` to the `v` object if the `id` string is not empty.
            // The value of the member is the index of the object identified by `id` on `container`.
            template <typename T>
            inline void AddOptionalMemberIndex(const std::string& name, rapidjson::Value& v, const std::string& id, const IndexedContainer<const T>& container, rapidjson::Document::AllocatorType& a)
            {
                if (!id.empty())
                {
                    v.AddMember(ToStringValue(name, a), rapidjson::Value(ToKnownSizeType(container.GetIndex(id))), a);
                }
            }

            // Adds a member called `name` to the `v` object if the `id` string is not empty.
            // The member is an object with a single member called `childName`, and its value
            // is the index of the object identified by `id` on `container`.
            template <typename T>
            inline void AddOptionalMemberIndexChild(const std::string& name, const std::string& childName, rapidjson::Value& v, const std::string& id, const IndexedContainer<const T>& container, rapidjson::Document::AllocatorType& a)
            {
                if (!id.empty())
                {
                    rapidjson::Value child(rapidjson::kObjectType);
                    {
                        child.AddMember(ToStringValue(childName, a), rapidjson::Value(ToKnownSizeType(container.GetIndex(id))), a);
                    }
                    v.AddMember(ToStringValue(name, a), child, a);
                }
            }

            template <typename T>
            inline void AddArrayMember(rapidjson::Value& v, const std::string& key, const std::vector<T>& list, rapidjson::Document::AllocatorType& a)
            {
                if (list.size() > 0)
                {
                    v.AddMember(ToStringValue(key, a), ToJsonArray<T>(list, a), a);
                }
            }

            inline rapidjson::Value& FindOrAddMember(rapidjson::Value& v, const std::string& memberName, rapidjson::Document::AllocatorType& a)
            {
                auto it = v.FindMember(memberName.c_str());
                if (it == v.MemberEnd())
                {
                    rapidjson::Value newValue(rapidjson::kObjectType);
                    v.AddMember(ToStringValue(memberName, a), newValue, a);
                    it = v.FindMember(ToStringValue(memberName, a));
                }
                return it->value;
            }

            inline rapidjson::Document CreateDocumentFromString(const std::string& json)
            {
                rapidjson::Document document;

                if (document.Parse(json.c_str()).HasParseError())
                {
                    // The input is not valid JSON.
                    throw GLTFException("The document is invalid due to bad JSON formatting");
                }

                return document;
            }

            inline rapidjson::Document CreateDocumentFromEncodedString(const std::string& json)
            {
                rapidjson::MemoryStream memoryStream(json.c_str(), json.size());
                rapidjson::EncodedInputStream<rapidjson::UTF8<>, rapidjson::MemoryStream> encodedStream(memoryStream);
                rapidjson::Document document;

                if (document.ParseStream<rapidjson::kParseDefaultFlags, rapidjson::UTF8<>>(encodedStream).HasParseError())
                {
                    // The input is not valid JSON.
                    throw GLTFException("The document is invalid due to bad JSON formatting");
                }

                return document;
            }

            inline rapidjson::Document CreateDocumentFromStream(std::istream& jsonStream)
            {
                rapidjson::IStreamWrapper streamWrapper(jsonStream);
                rapidjson::Document document;

                if (document.ParseStream(streamWrapper).HasParseError())
                {
                    // The input is not valid JSON.
                    throw GLTFException("The document is invalid due to bad JSON formatting");
                }

                return document;
            }

            inline rapidjson::Document CreateDocumentFromEncodedStream(std::istream& jsonStream)
            {
                rapidjson::IStreamWrapper streamWrapper(jsonStream);
                rapidjson::EncodedInputStream<rapidjson::UTF8<>, rapidjson::IStreamWrapper> encodedStream(streamWrapper);
                rapidjson::Document document;

                if (document.ParseStream<rapidjson::kParseDefaultFlags, rapidjson::UTF8<>>(streamWrapper).HasParseError())
                {
                    // The input is not valid JSON.
                    throw GLTFException("The document is invalid due to bad JSON formatting");
                }

                return document;
            }
        }
    }
}
