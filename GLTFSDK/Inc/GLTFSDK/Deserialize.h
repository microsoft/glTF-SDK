// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <GLTFSDK/Document.h>
#include <GLTFSDK/Schema.h>

namespace Microsoft 
{
    namespace glTF
    {
        // IgnoreByteOrderMark -> According to the spec, "JSON must use UTF-8 encoding without BOM". Specifying this flag will ignore the presence of a byte order mark rather than treating it as an error.
        enum class DeserializeFlags
        {
            None = 0x0,
            IgnoreByteOrderMark = 0x1
        };

        DeserializeFlags  operator| (DeserializeFlags lhs,  DeserializeFlags rhs);
        DeserializeFlags& operator|=(DeserializeFlags& lhs, DeserializeFlags rhs);
        DeserializeFlags  operator& (DeserializeFlags lhs,  DeserializeFlags rhs);
        DeserializeFlags& operator&=(DeserializeFlags& lhs, DeserializeFlags rhs);

        class ExtensionDeserializer;

        Document Deserialize(const std::string& json, DeserializeFlags flags = DeserializeFlags::None, SchemaFlags schemaFlags = SchemaFlags::None);
        Document Deserialize(const std::string& json, const ExtensionDeserializer& extensions, DeserializeFlags flags = DeserializeFlags::None, SchemaFlags schemaFlags = SchemaFlags::None);

        Document Deserialize(std::istream& jsonStream, DeserializeFlags flags = DeserializeFlags::None, SchemaFlags schemaFlags = SchemaFlags::None);
        Document Deserialize(std::istream& jsonStream, const ExtensionDeserializer& extensions, DeserializeFlags flags = DeserializeFlags::None, SchemaFlags schemaFlags = SchemaFlags::None);
    }
}
