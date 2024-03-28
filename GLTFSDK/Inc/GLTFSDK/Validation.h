// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <GLTFSDK/Document.h>

#include <set>

namespace Microsoft
{
    namespace glTF
    {
        namespace Validation
        {
            void GLTFSDK_API Validate(const Document& doc);
            void GLTFSDK_API ValidateAccessors(const Document& doc);
            void GLTFSDK_API ValidateMeshes(const Document& doc);
            void GLTFSDK_API ValidateMeshPrimitive(const Document& doc, const MeshPrimitive& primitive);
            void GLTFSDK_API ValidateMeshPrimitiveAttributeAccessors(const Document& doc, const std::unordered_map<std::string, std::string>& attributes, const size_t vertexCount);
            void GLTFSDK_API ValidateAccessorTypes(const Accessor& accessor, const std::string& accessorName,
                const std::set<AccessorType>& accessorTypes, const std::set<ComponentType>& componentTypes);
            void GLTFSDK_API ValidateAccessor(const Document& doc, const Accessor& accessor);
            void GLTFSDK_API ValidateBufferView(const BufferView& buffer_view, const Buffer& buffer);

            bool GLTFSDK_API SafeAddition(size_t a, size_t b, size_t& result);
            bool GLTFSDK_API SafeMultiplication(size_t a, size_t b, size_t& result);
        };
    }
}
