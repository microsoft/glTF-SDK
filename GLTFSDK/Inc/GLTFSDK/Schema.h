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
        constexpr const char SCHEMA_URI_GLTF[] = "glTF.schema.json";
        constexpr const char SCHEMA_URI_GLTFID[] = "glTFid.schema.json";
        constexpr const char SCHEMA_URI_GLTFCHILDOFROOTPROPERTY[] = "glTFChildOfRootProperty.schema.json";
        constexpr const char SCHEMA_URI_GLTFPROPERTY[] = "glTFProperty.schema.json";
        constexpr const char SCHEMA_URI_BUFFER[] = "buffer.schema.json";
        constexpr const char SCHEMA_URI_BUFFERVIEW[] = "bufferView.schema.json";
        constexpr const char SCHEMA_URI_ACCESSOR[] = "accessor.schema.json";
        constexpr const char SCHEMA_URI_ACCESSORSPARSE[] = "accessor.sparse.schema.json";
        constexpr const char SCHEMA_URI_ACCESSORSPARSEVALUES[] = "accessor.sparse.values.schema.json";
        constexpr const char SCHEMA_URI_ACCESSORSPARSEINDICES[] = "accessor.sparse.indices.schema.json";
        constexpr const char SCHEMA_URI_ASSET[] = "asset.schema.json";
        constexpr const char SCHEMA_URI_SCENE[] = "scene.schema.json";
        constexpr const char SCHEMA_URI_NODE[] = "node.schema.json";
        constexpr const char SCHEMA_URI_MESH[] = "mesh.schema.json";
        constexpr const char SCHEMA_URI_MESHPRIMITIVE[] = "mesh.primitive.schema.json";
        constexpr const char SCHEMA_URI_SKIN[] = "skin.schema.json";
        constexpr const char SCHEMA_URI_CAMERA[] = "camera.schema.json";
        constexpr const char SCHEMA_URI_CAMERAORTHOGRAPHIC[] = "camera.orthographic.schema.json";
        constexpr const char SCHEMA_URI_CAMERAPERSPECTIVE[] = "camera.perspective.schema.json";
        constexpr const char SCHEMA_URI_MATERIAL[] = "material.schema.json";
        constexpr const char SCHEMA_URI_MATERIALNORMALTEXTUREINFO[] = "material.normalTextureInfo.schema.json";
        constexpr const char SCHEMA_URI_MATERIALOCCLUSIONTEXTUREINFO[] = "material.occlusionTextureInfo.schema.json";
        constexpr const char SCHEMA_URI_MATERIALPBRMETALLICROUGHNESS[] = "material.pbrMetallicRoughness.schema.json";
        constexpr const char SCHEMA_URI_TEXTURE[] = "texture.schema.json";
        constexpr const char SCHEMA_URI_TEXTUREINFO[] = "textureInfo.schema.json";
        constexpr const char SCHEMA_URI_IMAGE[] = "image.schema.json";
        constexpr const char SCHEMA_URI_SAMPLER[] = "sampler.schema.json";
        constexpr const char SCHEMA_URI_ANIMATION[] = "animation.schema.json";
        constexpr const char SCHEMA_URI_ANIMATIONSAMPLER[] = "animation.sampler.schema.json";
        constexpr const char SCHEMA_URI_ANIMATIONCHANNEL[] = "animation.channel.schema.json";
        constexpr const char SCHEMA_URI_ANIMATIONCHANNELTARGET[] = "animation.channel.target.schema.json";
        constexpr const char SCHEMA_URI_EXTENSION[] = "extension.schema.json";
        constexpr const char SCHEMA_URI_EXTRAS[] = "extras.schema.json";

        const std::unordered_map<std::string, std::string>& GetDefaultSchemaUriMap();

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
            DisableSchemaExtension = 0x40000000,
            DisableSchemaExtras = 0x80000000
        };

        SchemaFlags  operator| (SchemaFlags lhs,  SchemaFlags rhs);
        SchemaFlags& operator|=(SchemaFlags& lhs, SchemaFlags rhs);
        SchemaFlags  operator& (SchemaFlags lhs,  SchemaFlags rhs);
        SchemaFlags& operator&=(SchemaFlags& lhs, SchemaFlags rhs);

        std::unique_ptr<const class ISchemaLocator> GetDefaultSchemaLocator(SchemaFlags schemaFlags);
    }
}
