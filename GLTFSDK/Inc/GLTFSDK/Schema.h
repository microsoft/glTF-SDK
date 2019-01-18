// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <string>
#include <memory>
#include <unordered_map>

namespace Microsoft
{
    namespace glTF
    {
        //TODO: add these variables for all the schema uris (and use in the schemaFlagsMap)?
        constexpr const char SCHEMA_URI_GLTF[] = "glTF.schema.json";

        enum class SchemaFlags : uint64_t
        {
            None = 0x0,
            DisableSchemaRoot = 0x1, // Disables all schema validation
            DisableSchemaId = 0x2,
            DisableSchemaChildOfRoot = 0x4,
            DisableSchemaProperty = 0x8,
            DisableSchemaBuffer = 0x10,
            DisableSchemaBufferView = 0x20,
            DisableSchemaAccessor = 0x40,
            DisableSchemaAccessorSparse = 0x80,
            DisableSchemaAccessorSparseValues = 0x80,
            DisableSchemaAccessorSparseIndices = 0x100,
            DisableSchemaAsset = 0x200,
            DisableSchemaScene = 0x400,
            DisableSchemaNode = 0x800,
            DisableSchemaMesh = 0x1000,
            DisableSchemaMeshPrimitive = 0x2000,
            DisableSchemaSkin = 0x4000,
            DisableSchemaCamera = 0x8000,
            DisableSchemaCameraOrthographic = 0x10000,
            DisableSchemaCameraPerspective = 0x20000,
            DisableSchemaMaterial = 0x40000,
            DisableSchemaMaterialNormalTextureInfo = 0x80000,
            DisableSchemaMaterialOcclusionTextureInfo = 0x100000,
            DisableSchemaMaterialPBRMetallicRoughness = 0x200000,
            DisableSchemaTexture = 0x400000,
            DisableSchemaTextureInfo = 0x800000,
            DisableSchemaImage = 0x1000000,
            DisableSchemaSampler = 0x2000000,
            DisableSchemaAnimation = 0x4000000,
            DisableSchemaAnimationSampler = 0x8000000,
            DisableSchemaAnimationChannel = 0x10000000,
            DisableSchemaAnimationChannelTarget = 0x20000000,
            DisableSchemaExtensions = 0x40000000,
            DisableSchemaExtras = 0x80000000
        };

        SchemaFlags  operator| (SchemaFlags lhs,  SchemaFlags rhs);
        SchemaFlags& operator|=(SchemaFlags& lhs, SchemaFlags rhs);
        SchemaFlags  operator& (SchemaFlags lhs,  SchemaFlags rhs);
        SchemaFlags& operator&=(SchemaFlags& lhs, SchemaFlags rhs);

        const std::unordered_map<std::string, std::string>& GetSchemaUriMap();

        class ISchemaLocator
        {
        public:
            virtual ~ISchemaLocator() = default;
            virtual const char* GetSchemaContent(const std::string& uri) const = 0;
        };

        using SchemaLocatorPtr = std::unique_ptr<const ISchemaLocator>;

        SchemaLocatorPtr GetDefaultSchemaLocator();
    }
}