// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/ExtrasDocument.h>
#include <GLTFSDK/Exceptions.h>

#include "Internal/Json.h"

#include <utility>

using namespace Microsoft::glTF;

struct ExtrasDocument::Impl
{
    Impl()
        : value(Internal::CreateJsonNull())
    {
    }

    explicit Impl(Internal::JsonValue json)
        : value(std::move(json))
    {
    }

    Internal::JsonValue value;
};

ExtrasDocument::ExtrasDocument()
    : m_impl(new Impl())
{
}

ExtrasDocument::ExtrasDocument(const char* extras)
{
    if (extras == nullptr)
    {
        throw GLTFException("Extras JSON string must not be null");
    }
    m_impl.reset(new Impl(Internal::ParseJson(std::string(extras))));
}

ExtrasDocument::ExtrasDocument(const std::string& extras)
    : m_impl(new Impl(Internal::ParseJson(extras)))
{
}

ExtrasDocument::~ExtrasDocument() = default;

ExtrasDocument::ExtrasDocument(ExtrasDocument&& other) noexcept = default;

ExtrasDocument& ExtrasDocument::operator=(
    ExtrasDocument&& other) noexcept = default;

std::string ExtrasDocument::ToJson() const
{
    return m_impl
        ? Internal::WriteJson(m_impl->value)
        : std::string("null");
}

bool ExtrasDocument::HasMember(const char* member) const
{
    if (member == nullptr)
    {
        throw GLTFException("Extras member name must not be null");
    }
    if (!m_impl || !Internal::IsJsonObject(m_impl->value))
    {
        return false;
    }
    return Internal::FindJsonMember(m_impl->value, member) != nullptr;
}

bool ExtrasDocument::TryGetPrimitive(
    Target target,
    const char* selector,
    PrimitiveType type,
    void* result) const
{
    if ((target == Target::Member || target == Target::Pointer) &&
        selector == nullptr)
    {
        throw GLTFException(
            target == Target::Member
                ? "Extras member name must not be null"
                : "Extras JSON Pointer must not be null");
    }
    if (!m_impl)
    {
        return false;
    }

    const Internal::JsonValue* value = &m_impl->value;
    if (target == Target::Member)
    {
        if (!Internal::IsJsonObject(*value))
        {
            return false;
        }
        value = Internal::FindJsonMember(*value, selector);
    }
    else if (target == Target::Pointer)
    {
        value = Internal::FindJsonPointer(*value, selector);
    }

    if (value == nullptr)
    {
        return false;
    }

    switch (type)
    {
    case PrimitiveType::Boolean:
        return Internal::TryGetJsonBoolean(
            *value, *static_cast<bool*>(result));
    case PrimitiveType::Int32:
        return Internal::TryGetJsonInt32(
            *value, *static_cast<std::int32_t*>(result));
    case PrimitiveType::UInt32:
        return Internal::TryGetJsonUInt32(
            *value, *static_cast<std::uint32_t*>(result));
    case PrimitiveType::Int64:
        return Internal::TryGetJsonInt64(
            *value, *static_cast<std::int64_t*>(result));
    case PrimitiveType::UInt64:
        return Internal::TryGetJsonUInt64(
            *value, *static_cast<std::uint64_t*>(result));
    case PrimitiveType::Size:
        return Internal::TryGetJsonSize(
            *value, *static_cast<std::size_t*>(result));
    case PrimitiveType::Float:
        return Internal::TryGetJsonFloat(
            *value, *static_cast<float*>(result));
    case PrimitiveType::Double:
        return Internal::TryGetJsonDouble(
            *value, *static_cast<double*>(result));
    case PrimitiveType::String:
        return Internal::TryGetJsonString(
            *value, *static_cast<std::string*>(result));
    case PrimitiveType::CString:
        break;
    }

    throw GLTFException("Unsupported ExtrasDocument getter type");
}

void ExtrasDocument::SetPrimitive(
    Target target,
    const char* selector,
    PrimitiveType type,
    const void* value)
{
    if ((target == Target::Member || target == Target::Pointer) &&
        selector == nullptr)
    {
        throw GLTFException(
            target == Target::Member
                ? "Extras member name must not be null"
                : "Extras JSON Pointer must not be null");
    }

    if (!m_impl)
    {
        m_impl.reset(new Impl());
    }

    Internal::JsonValue jsonValue;
    switch (type)
    {
    case PrimitiveType::Boolean:
        jsonValue = Internal::CreateJsonBoolean(
            *static_cast<const bool*>(value));
        break;
    case PrimitiveType::Int32:
        jsonValue = Internal::CreateJsonInt32(
            *static_cast<const std::int32_t*>(value));
        break;
    case PrimitiveType::UInt32:
        jsonValue = Internal::CreateJsonUInt32(
            *static_cast<const std::uint32_t*>(value));
        break;
    case PrimitiveType::Int64:
        jsonValue = Internal::CreateJsonInt64(
            *static_cast<const std::int64_t*>(value));
        break;
    case PrimitiveType::UInt64:
        jsonValue = Internal::CreateJsonUInt64(
            *static_cast<const std::uint64_t*>(value));
        break;
    case PrimitiveType::Size:
        jsonValue = Internal::CreateJsonSize(
            *static_cast<const std::size_t*>(value));
        break;
    case PrimitiveType::Float:
        jsonValue = Internal::CreateJsonFloat(
            *static_cast<const float*>(value));
        break;
    case PrimitiveType::Double:
        jsonValue = Internal::CreateJsonDouble(
            *static_cast<const double*>(value));
        break;
    case PrimitiveType::String:
        jsonValue = Internal::CreateJsonString(
            *static_cast<const std::string*>(value));
        break;
    case PrimitiveType::CString:
        jsonValue = Internal::CreateJsonString(
            *static_cast<const char* const*>(value));
        break;
    }

    if (target == Target::Root)
    {
        Internal::SetJsonPointer(
            m_impl->value, "", std::move(jsonValue));
        return;
    }

    if (target == Target::Pointer)
    {
        Internal::SetJsonPointer(
            m_impl->value, selector, std::move(jsonValue));
        return;
    }

    if (Internal::IsJsonNull(m_impl->value))
    {
        m_impl->value = Internal::CreateJsonObject();
    }
    else if (!Internal::IsJsonObject(m_impl->value))
    {
        throw GLTFException(
            "Extras JSON document has already been assigned an "
            "incompatible type");
    }

    Internal::JsonValue* existing =
        Internal::FindJsonMember(m_impl->value, selector);
    if (existing != nullptr &&
        !Internal::AreJsonAssignmentCategoriesCompatible(
            *existing, jsonValue))
    {
        throw GLTFException(
            "Extras JSON value has already been assigned an "
            "incompatible type");
    }

    Internal::SetJsonMember(
        m_impl->value, selector, std::move(jsonValue));
}
