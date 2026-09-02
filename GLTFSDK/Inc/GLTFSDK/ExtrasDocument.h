// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <GLTFSDK/Definitions.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

namespace Microsoft
{
    namespace glTF
    {
        namespace Detail
        {
            struct ExtrasUnsupportedTag {};
            struct ExtrasBooleanTag {};
            struct ExtrasSignedTag {};
            struct ExtrasUnsignedTag {};
            struct ExtrasFloatingTag {};
            struct ExtrasStringTag {};
            struct ExtrasCStringTag {};

            template<typename T, typename Enable = void>
            struct ExtrasTypeTraits
            {
                static constexpr bool CanGet = false;
                static constexpr bool CanSet = false;
                using Tag = ExtrasUnsupportedTag;
            };

            template<typename T>
            struct ExtrasTypeTraits<
                T,
                typename std::enable_if<
                    std::is_same<T, bool>::value>::type>
            {
                static constexpr bool CanGet = true;
                static constexpr bool CanSet = true;
                using Tag = ExtrasBooleanTag;
            };

            template<typename T>
            struct ExtrasTypeTraits<
                T,
                typename std::enable_if<
                    std::is_same<T, std::int32_t>::value ||
                    std::is_same<T, std::int64_t>::value>::type>
            {
                static constexpr bool CanGet = true;
                static constexpr bool CanSet = true;
                using Tag = ExtrasSignedTag;
            };

            template<typename T>
            struct ExtrasTypeTraits<
                T,
                typename std::enable_if<
                    std::is_same<T, std::uint32_t>::value ||
                    std::is_same<T, std::uint64_t>::value ||
                    std::is_same<T, std::size_t>::value>::type>
            {
                static constexpr bool CanGet = true;
                static constexpr bool CanSet = true;
                using Tag = ExtrasUnsignedTag;
            };

            template<typename T>
            struct ExtrasTypeTraits<
                T,
                typename std::enable_if<
                    std::is_same<T, float>::value ||
                    std::is_same<T, double>::value>::type>
            {
                static constexpr bool CanGet = true;
                static constexpr bool CanSet = true;
                using Tag = ExtrasFloatingTag;
            };

            template<typename T>
            struct ExtrasTypeTraits<
                T,
                typename std::enable_if<
                    std::is_same<T, std::string>::value>::type>
            {
                static constexpr bool CanGet = true;
                static constexpr bool CanSet = true;
                using Tag = ExtrasStringTag;
            };

            template<typename T>
            struct ExtrasTypeTraits<
                T,
                typename std::enable_if<
                    std::is_same<T, const char*>::value ||
                    std::is_same<T, char*>::value>::type>
            {
                static constexpr bool CanGet = false;
                static constexpr bool CanSet = true;
                using Tag = ExtrasCStringTag;
            };
        }

        class ExtrasDocument
        {
        public:
            ExtrasDocument();
            explicit ExtrasDocument(const char* extras);
            explicit ExtrasDocument(
                const std::string& extras);
            ~ExtrasDocument();

            ExtrasDocument(ExtrasDocument&& other) noexcept;
            ExtrasDocument& GLTFSDK_API operator=(
                ExtrasDocument&& other) noexcept;

            ExtrasDocument(const ExtrasDocument&) = delete;
            ExtrasDocument& operator=(const ExtrasDocument&) = delete;

            std::string GLTFSDK_API ToJson() const;
            bool GLTFSDK_API HasMember(const char* member) const;

            template<typename T>
            T GetValueOrDefault(T defaultValue = {}) const
            {
                return GetValueOrDefaultDispatch(
                    Target::Root,
                    nullptr,
                    std::move(defaultValue),
                    typename Detail::ExtrasTypeTraits<
                        typename std::decay<T>::type>::Tag());
            }

            template<typename T>
            T GetMemberValueOrDefault(
                const char* member,
                T defaultValue = {}) const
            {
                return GetValueOrDefaultDispatch(
                    Target::Member,
                    member,
                    std::move(defaultValue),
                    typename Detail::ExtrasTypeTraits<
                        typename std::decay<T>::type>::Tag());
            }

            template<typename T>
            T GetPointerValueOrDefault(
                const char* pointer,
                T defaultValue = {}) const
            {
                return GetValueOrDefaultDispatch(
                    Target::Pointer,
                    pointer,
                    std::move(defaultValue),
                    typename Detail::ExtrasTypeTraits<
                        typename std::decay<T>::type>::Tag());
            }

            template<typename T>
            void SetValue(const T& value)
            {
                SetValueDispatch(
                    Target::Root,
                    nullptr,
                    value,
                    typename Detail::ExtrasTypeTraits<
                        typename std::decay<T>::type>::Tag());
            }

            template<typename T>
            void SetMemberValue(const char* member, const T& value)
            {
                SetValueDispatch(
                    Target::Member,
                    member,
                    value,
                    typename Detail::ExtrasTypeTraits<
                        typename std::decay<T>::type>::Tag());
            }

            template<typename T>
            void SetPointerValue(const char* pointer, const T& value)
            {
                SetValueDispatch(
                    Target::Pointer,
                    pointer,
                    value,
                    typename Detail::ExtrasTypeTraits<
                        typename std::decay<T>::type>::Tag());
            }

        private:
            enum class Target
            {
                Root,
                Member,
                Pointer
            };

            enum class PrimitiveType
            {
                Boolean,
                Int32,
                UInt32,
                Int64,
                UInt64,
                Size,
                Float,
                Double,
                String,
                CString
            };

            struct Impl;

            bool GLTFSDK_API TryGetPrimitive(
                Target target,
                const char* selector,
                PrimitiveType type,
                void* result) const;
            void GLTFSDK_API SetPrimitive(
                Target target,
                const char* selector,
                PrimitiveType type,
                const void* value);

            template<typename T>
            T GetValueOrDefaultDispatch(
                Target target,
                const char* selector,
                T defaultValue,
                Detail::ExtrasBooleanTag) const
            {
                static_assert(
                    Detail::ExtrasTypeTraits<T>::CanGet,
                    "Unsupported ExtrasDocument getter type");
                T result{};
                return TryGetPrimitive(
                    target, selector, PrimitiveType::Boolean, &result)
                    ? result
                    : std::move(defaultValue);
            }

            template<typename T>
            T GetValueOrDefaultDispatch(
                Target target,
                const char* selector,
                T defaultValue,
                Detail::ExtrasSignedTag) const
            {
                static_assert(
                    Detail::ExtrasTypeTraits<T>::CanGet,
                    "Unsupported ExtrasDocument getter type");
                T result{};
                const PrimitiveType type = sizeof(T) == sizeof(std::int32_t)
                    ? PrimitiveType::Int32
                    : PrimitiveType::Int64;
                return TryGetPrimitive(target, selector, type, &result)
                    ? result
                    : std::move(defaultValue);
            }

            template<typename T>
            T GetValueOrDefaultDispatch(
                Target target,
                const char* selector,
                T defaultValue,
                Detail::ExtrasUnsignedTag) const
            {
                static_assert(
                    Detail::ExtrasTypeTraits<T>::CanGet,
                    "Unsupported ExtrasDocument getter type");
                T result{};
                const PrimitiveType type =
                    std::is_same<T, std::size_t>::value
                    ? PrimitiveType::Size
                    : sizeof(T) == sizeof(std::uint32_t)
                        ? PrimitiveType::UInt32
                        : PrimitiveType::UInt64;
                return TryGetPrimitive(target, selector, type, &result)
                    ? result
                    : std::move(defaultValue);
            }

            template<typename T>
            T GetValueOrDefaultDispatch(
                Target target,
                const char* selector,
                T defaultValue,
                Detail::ExtrasFloatingTag) const
            {
                static_assert(
                    Detail::ExtrasTypeTraits<T>::CanGet,
                    "Unsupported ExtrasDocument getter type");
                T result{};
                const PrimitiveType type = std::is_same<T, float>::value
                    ? PrimitiveType::Float
                    : PrimitiveType::Double;
                return TryGetPrimitive(target, selector, type, &result)
                    ? result
                    : std::move(defaultValue);
            }

            template<typename T>
            T GetValueOrDefaultDispatch(
                Target target,
                const char* selector,
                T defaultValue,
                Detail::ExtrasStringTag) const
            {
                static_assert(
                    Detail::ExtrasTypeTraits<T>::CanGet,
                    "Unsupported ExtrasDocument getter type");
                T result;
                return TryGetPrimitive(
                    target, selector, PrimitiveType::String, &result)
                    ? result
                    : std::move(defaultValue);
            }

            template<typename T>
            T GetValueOrDefaultDispatch(
                Target,
                const char*,
                T,
                Detail::ExtrasUnsupportedTag) const
            {
                static_assert(
                    Detail::ExtrasTypeTraits<T>::CanGet,
                    "Unsupported ExtrasDocument getter type");
                return T();
            }

            template<typename T>
            void SetValueDispatch(
                Target target,
                const char* selector,
                const T& value,
                Detail::ExtrasBooleanTag)
            {
                static_assert(
                    Detail::ExtrasTypeTraits<
                        typename std::decay<T>::type>::CanSet,
                    "Unsupported ExtrasDocument setter type");
                SetPrimitive(
                    target, selector, PrimitiveType::Boolean, &value);
            }

            template<typename T>
            void SetValueDispatch(
                Target target,
                const char* selector,
                const T& value,
                Detail::ExtrasSignedTag)
            {
                static_assert(
                    Detail::ExtrasTypeTraits<
                        typename std::decay<T>::type>::CanSet,
                    "Unsupported ExtrasDocument setter type");
                const PrimitiveType type = sizeof(T) == sizeof(std::int32_t)
                    ? PrimitiveType::Int32
                    : PrimitiveType::Int64;
                SetPrimitive(target, selector, type, &value);
            }

            template<typename T>
            void SetValueDispatch(
                Target target,
                const char* selector,
                const T& value,
                Detail::ExtrasUnsignedTag)
            {
                static_assert(
                    Detail::ExtrasTypeTraits<
                        typename std::decay<T>::type>::CanSet,
                    "Unsupported ExtrasDocument setter type");
                const PrimitiveType type =
                    std::is_same<
                        typename std::decay<T>::type,
                        std::size_t>::value
                    ? PrimitiveType::Size
                    : sizeof(T) == sizeof(std::uint32_t)
                        ? PrimitiveType::UInt32
                        : PrimitiveType::UInt64;
                SetPrimitive(target, selector, type, &value);
            }

            template<typename T>
            void SetValueDispatch(
                Target target,
                const char* selector,
                const T& value,
                Detail::ExtrasFloatingTag)
            {
                static_assert(
                    Detail::ExtrasTypeTraits<
                        typename std::decay<T>::type>::CanSet,
                    "Unsupported ExtrasDocument setter type");
                const PrimitiveType type =
                    std::is_same<typename std::decay<T>::type, float>::value
                    ? PrimitiveType::Float
                    : PrimitiveType::Double;
                SetPrimitive(target, selector, type, &value);
            }

            template<typename T>
            void SetValueDispatch(
                Target target,
                const char* selector,
                const T& value,
                Detail::ExtrasStringTag)
            {
                static_assert(
                    Detail::ExtrasTypeTraits<
                        typename std::decay<T>::type>::CanSet,
                    "Unsupported ExtrasDocument setter type");
                SetPrimitive(
                    target, selector, PrimitiveType::String, &value);
            }

            template<typename T>
            void SetValueDispatch(
                Target target,
                const char* selector,
                const T& value,
                Detail::ExtrasCStringTag)
            {
                static_assert(
                    Detail::ExtrasTypeTraits<
                        typename std::decay<T>::type>::CanSet,
                    "Unsupported ExtrasDocument setter type");
                const char* stringValue = value;
                SetPrimitive(
                    target,
                    selector,
                    PrimitiveType::CString,
                    &stringValue);
            }

            template<typename T>
            void SetValueDispatch(
                Target,
                const char*,
                const T&,
                Detail::ExtrasUnsupportedTag)
            {
                static_assert(
                    Detail::ExtrasTypeTraits<
                        typename std::decay<T>::type>::CanSet,
                    "Unsupported ExtrasDocument setter type");
            }

            std::unique_ptr<Impl> m_impl;
        };
    }
}
