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
            void GLTFSDK_CDECL Validate(const Document& doc);
            void GLTFSDK_CDECL ValidateAccessors(const Document& doc);
            void GLTFSDK_CDECL ValidateMeshes(const Document& doc);
            void GLTFSDK_CDECL ValidateMeshPrimitive(const Document& doc, const MeshPrimitive& primitive);
            void GLTFSDK_CDECL ValidateMeshPrimitiveAttributeAccessors(const Document& doc, const std::unordered_map<std::string, std::string>& attributes, const size_t vertexCount);
            void GLTFSDK_CDECL ValidateAccessorTypes(const Accessor& accessor, const std::string& accessorName,
                const std::set<AccessorType>& accessorTypes, const std::set<ComponentType>& componentTypes);
            void GLTFSDK_CDECL ValidateAccessor(const Document& doc, const Accessor& accessor);
            void GLTFSDK_CDECL ValidateBufferView(const BufferView& buffer_view, const Buffer& buffer);

            bool GLTFSDK_CDECL SafeAddition(size_t a, size_t b, size_t& result);
            bool GLTFSDK_CDECL SafeMultiplication(size_t a, size_t b, size_t& result);
        };
    }
}
