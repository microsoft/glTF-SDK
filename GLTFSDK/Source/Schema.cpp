// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/Schema.h>
#include <GLTFSDK/Exceptions.h>

#include "SchemaJson.h" // Auto-generated header, don't include in any other translation units to avoid linker errors

using namespace Microsoft::glTF;

namespace
{
    class DefaultSchemaLocator : public ISchemaLocator
    {
    public:
        const char* GetSchemaContent(const std::string& uri) const override
        {
            auto it = SchemaJson::GLTF_SCHEMA_MAP.find(uri);

            if (it == SchemaJson::GLTF_SCHEMA_MAP.end())
            {
                throw GLTFException("Unknown Schema uri: " + uri);
            }

            return it->second.c_str();
        }
    };
}

// Microsoft::glTF namespace function definitions

const std::unordered_map<std::string, std::string>& Microsoft::glTF::GetSchemaUriMap()
{
    return SchemaJson::GLTF_SCHEMA_MAP;
}

SchemaLocatorPtr Microsoft::glTF::GetDefaultSchemaLocator()
{
    return std::make_unique<const DefaultSchemaLocator>();
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
