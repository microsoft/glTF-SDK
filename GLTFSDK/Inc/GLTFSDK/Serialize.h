// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <string>

namespace Microsoft 
{
    namespace glTF
    {
        enum class SerializeFlags
        {
            None = 0x0,
            Pretty = 0x1
        };

        SerializeFlags  operator| (SerializeFlags lhs,  SerializeFlags rhs);
        SerializeFlags& operator|=(SerializeFlags& lhs, SerializeFlags rhs);
        SerializeFlags  operator& (SerializeFlags lhs,  SerializeFlags rhs);
        SerializeFlags& operator&=(SerializeFlags& lhs, SerializeFlags rhs);

        class Document;
        class ExtensionSerializer;

        std::string Serialize(const Document& gltfDocument, SerializeFlags flags = SerializeFlags::None);
        std::string Serialize(const Document& gltfDocument, const ExtensionSerializer& extensionHandler, SerializeFlags flags = SerializeFlags::None);
    }
}
