// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/Schema.h>
#include <GLTFSDK/Exceptions.h>

#include "SchemaJson.h" // Auto-generated header, don't include in any other translation units to avoid linker errors

using namespace Microsoft::glTF;

// Microsoft::glTF namespace function definitions

const std::unordered_map<std::string, std::string>& Microsoft::glTF::GetSchemaUriMap()
{
    return SchemaJson::GLTF_SCHEMA_MAP;
}

// SchemaLocator class definitions

SchemaLocator::SchemaLocator(std::unordered_map<std::string, std::string> schemaUriMap) : schemaUriMap(std::move(schemaUriMap))
{
}

const char* SchemaLocator::GetSchemaContent(const char* uri) const
{
    auto it = schemaUriMap.find(uri);

    if (it == schemaUriMap.end())
    {
        throw ValidationException("Unknown schema uri");
    }

    return it->second.c_str();
}

// SchemaFlags operator definitions

SchemaFlags Microsoft::glTF::operator|(SchemaFlags lhs, SchemaFlags rhs)
{
    const auto result =
        static_cast<std::underlying_type_t<SchemaFlags>>(lhs) |
        static_cast<std::underlying_type_t<SchemaFlags>>(rhs);

    return static_cast<SchemaFlags>(result);
}

SchemaFlags& Microsoft::glTF::operator|=(SchemaFlags& lhs, SchemaFlags rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

SchemaFlags Microsoft::glTF::operator&(SchemaFlags lhs, SchemaFlags rhs)
{
    const auto result =
        static_cast<std::underlying_type_t<SchemaFlags>>(lhs) &
        static_cast<std::underlying_type_t<SchemaFlags>>(rhs);

    return static_cast<SchemaFlags>(result);
}

SchemaFlags& Microsoft::glTF::operator&=(SchemaFlags& lhs, SchemaFlags rhs)
{
    lhs = lhs & rhs;
    return lhs;
}
