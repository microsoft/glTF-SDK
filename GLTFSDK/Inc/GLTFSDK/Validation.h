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
            void Validate(const Document& doc);
            void ValidateAccessors(const Document& doc);
            void ValidateMeshes(const Document& doc);
            void ValidateMeshPrimitive(const Document& doc, const MeshPrimitive& primitive);
            void ValidateMeshPrimitiveAttributeAccessors(const Document& doc, const std::unordered_map<std::string, std::string>& attributes, const size_t vertexCount);
            void ValidateAccessorTypes(const Accessor& accessor, const std::string& accessorName,
                const std::set<AccessorType>& accessorTypes, const std::set<ComponentType>& componentTypes);
            void ValidateAccessor(const Document& doc, const Accessor& accessor);
            void ValidateBufferView(const BufferView& buffer_view, const Buffer& buffer);

            bool SafeAddition(size_t a, size_t b, size_t& result);
            bool SafeMultiplication(size_t a, size_t b, size_t& result);
        };
    }
}
