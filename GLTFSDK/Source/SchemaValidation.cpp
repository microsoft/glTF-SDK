// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/SchemaValidation.h>
#include <GLTFSDK/Exceptions.h>

#include <memory>
#include <sstream>
#include <unordered_map>

using namespace Microsoft::glTF;
using namespace Microsoft::glTF::Schema;

namespace
{
    const std::unordered_map<std::string, SchemaFlags> schemaFlagMap = {
        { "glTF.schema.json", SchemaFlags::DisableSchemaRoot },
        { "glTFid.schema.json", SchemaFlags::DisableSchemaId },
        { "glTFChildOfRootProperty.schema.json", SchemaFlags::DisableSchemaChildOfRoot },
        { "glTFProperty.schema.json", SchemaFlags::DisableSchemaProperty },
        { "buffer.schema.json", SchemaFlags::DisableSchemaBuffer },
        { "bufferView.schema.json", SchemaFlags::DisableSchemaBufferView },
        { "accessor.schema.json", SchemaFlags::DisableSchemaAccessor },
        { "accessor.sparse.schema.json", SchemaFlags::DisableSchemaAccessorSparse },
        { "accessor.sparse.values.schema.json", SchemaFlags::DisableSchemaAccessorSparseValues },
        { "accessor.sparse.indices.schema.json", SchemaFlags::DisableSchemaAccessorSparseIndices },
        { "asset.schema.json", SchemaFlags::DisableSchemaAsset },
        { "scene.schema.json", SchemaFlags::DisableSchemaScene },
        { "node.schema.json", SchemaFlags::DisableSchemaNode },
        { "mesh.schema.json", SchemaFlags::DisableSchemaMesh },
        { "mesh.primitive.schema.json", SchemaFlags::DisableSchemaMeshPrimitive },
        { "skin.schema.json", SchemaFlags::DisableSchemaSkin },
        { "camera.schema.json", SchemaFlags::DisableSchemaCamera },
        { "camera.orthographic.schema.json", SchemaFlags::DisableSchemaCameraOrthographic },
        { "camera.perspective.schema.json", SchemaFlags::DisableSchemaCameraPerspective },
        { "material.schema.json", SchemaFlags::DisableSchemaMaterial },
        { "material.normalTextureInfo.schema.json", SchemaFlags::DisableSchemaMaterialNormalTextureInfo },
        { "material.occlusionTextureInfo.schema.json", SchemaFlags::DisableSchemaMaterialOcclusionTextureInfo },
        { "material.pbrMetallicRoughness.schema.json", SchemaFlags::DisableSchemaMaterialPBRMetallicRoughness },
        { "texture.schema.json", SchemaFlags::DisableSchemaTexture },
        { "textureInfo.schema.json", SchemaFlags::DisableSchemaTextureInfo },
        { "image.schema.json", SchemaFlags::DisableSchemaImage },
        { "sampler.schema.json", SchemaFlags::DisableSchemaSampler },
        { "animation.schema.json", SchemaFlags::DisableSchemaAnimation },
        { "animation.sampler.schema.json", SchemaFlags::DisableSchemaAnimationSampler },
        { "animation.channel.schema.json", SchemaFlags::DisableSchemaAnimationChannel },
        { "animation.channel.target.schema.json", SchemaFlags::DisableSchemaAnimationChannelTarget },
        { "extensions.schema.json", SchemaFlags::DisableSchemaExtensions },
        { "extras.schema.json", SchemaFlags::DisableSchemaExtras }
    };

    bool HasSchemaFlag(SchemaFlags flag, SchemaFlags flags)
    {
        return (flags & flag) == flag;
    }

    class RemoteSchemaDocumentProvider : public rapidjson::IRemoteSchemaDocumentProvider
    {
    public:
        RemoteSchemaDocumentProvider(SchemaLocatorPtr schemaLocator, SchemaFlags schemaFlags) :
            schemaFlags(schemaFlags),
            schemaLocator(std::move(schemaLocator))
        {
            assert(this->schemaLocator);
        }

        const rapidjson::SchemaDocument* GetRemoteDocument(const std::string& uri)
        {
            auto itDoc = schemaDocuments.find(uri);
            if (itDoc != schemaDocuments.end())
            {
                return &(itDoc->second);
            }

            rapidjson::Document document;

            auto itFlag = schemaFlagMap.find(uri);
            if (itFlag != schemaFlagMap.end() && HasSchemaFlag(itFlag->second, schemaFlags))
            {
                // If the schema for this uri is disabled via the relevant SchemaFlags enum value then create
                // the SchemaDocument instance with a Document containing only an empty object (empty objects
                // are a completely valid schema that will accept any valid JSON)
                document.SetObject();
            }
            else
            {
                const char* schemaContent = schemaLocator->GetSchemaContent(uri.c_str());

                if (document.Parse(schemaContent).HasParseError())
                {
                    std::stringstream ss;

                    ss << "Schema document (";
                    ss << uri;
                    ss << ") is not valid JSON";

                    throw ValidationException(ss.str());
                }
            }

            auto result = schemaDocuments.emplace(uri, rapidjson::SchemaDocument(document, this));
            assert(result.second);
            auto resultSchemaDoc = &(result.first->second);
            assert(resultSchemaDoc);

            return resultSchemaDoc;
        }

        const rapidjson::SchemaDocument* GetRemoteDocument(const char* uri, rapidjson::SizeType length) override
        {
            return GetRemoteDocument({ uri, length });
        }

    private:
        const SchemaFlags      schemaFlags;
        const SchemaLocatorPtr schemaLocator;

        std::unordered_map<std::string, rapidjson::SchemaDocument> schemaDocuments;
    };
}

// Microsoft::glTF::Schema namespace function definitions

void Microsoft::glTF::Schema::ValidateDocument(const rapidjson::Document& document, SchemaLocatorPtr schemaLocator, SchemaFlags schemaFlags)
{
    if (schemaLocator == nullptr)
    {
        schemaLocator = GetDefaultSchemaLocator();
    }

    RemoteSchemaDocumentProvider provider(std::move(schemaLocator), schemaFlags);

    if (auto* schemaDocument = provider.GetRemoteDocument(SCHEMA_URI_GLTF))
    {
        rapidjson::SchemaValidator schemaValidator(*schemaDocument);

        if (!document.Accept(schemaValidator))
        {
            rapidjson::StringBuffer sb;

            const char* schemaKeyword = schemaValidator.GetInvalidSchemaKeyword();
            schemaValidator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
            const char* schemaInvalid = sb.GetString();

            std::stringstream ss;

            ss << "Schema violation at " << schemaInvalid << " ";
            ss << "due to " << schemaKeyword;

            throw ValidationException(ss.str());
        }
    }
    else
    {
        throw GLTFException("Schema document could not be located");
    }
}
