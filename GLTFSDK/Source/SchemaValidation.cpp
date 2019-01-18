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
        { SCHEMA_URI_GLTF, SchemaFlags::DisableSchemaRoot },
        { SCHEMA_URI_GLTFID, SchemaFlags::DisableSchemaId },
        { SCHEMA_URI_GLTFCHILDOFROOTPROPERTY, SchemaFlags::DisableSchemaChildOfRoot },
        { SCHEMA_URI_GLTFPROPERTY, SchemaFlags::DisableSchemaProperty },
        { SCHEMA_URI_BUFFER, SchemaFlags::DisableSchemaBuffer },
        { SCHEMA_URI_BUFFERVIEW, SchemaFlags::DisableSchemaBufferView },
        { SCHEMA_URI_ACCESSOR, SchemaFlags::DisableSchemaAccessor },
        { SCHEMA_URI_ACCESSORSPARSE, SchemaFlags::DisableSchemaAccessorSparse },
        { SCHEMA_URI_ACCESSORSPARSEVALUES, SchemaFlags::DisableSchemaAccessorSparseValues },
        { SCHEMA_URI_ACCESSORSPARSEINDICES, SchemaFlags::DisableSchemaAccessorSparseIndices },
        { SCHEMA_URI_ASSET, SchemaFlags::DisableSchemaAsset },
        { SCHEMA_URI_SCENE, SchemaFlags::DisableSchemaScene },
        { SCHEMA_URI_NODE, SchemaFlags::DisableSchemaNode },
        { SCHEMA_URI_MESH, SchemaFlags::DisableSchemaMesh },
        { SCHEMA_URI_MESHPRIMITIVE, SchemaFlags::DisableSchemaMeshPrimitive },
        { SCHEMA_URI_SKIN, SchemaFlags::DisableSchemaSkin },
        { SCHEMA_URI_CAMERA, SchemaFlags::DisableSchemaCamera },
        { SCHEMA_URI_CAMERAORTHOGRAPHIC, SchemaFlags::DisableSchemaCameraOrthographic },
        { SCHEMA_URI_CAMERAPERSPECTIVE, SchemaFlags::DisableSchemaCameraPerspective },
        { SCHEMA_URI_MATERIAL, SchemaFlags::DisableSchemaMaterial },
        { SCHEMA_URI_MATERIALNORMALTEXTUREINFO, SchemaFlags::DisableSchemaMaterialNormalTextureInfo },
        { SCHEMA_URI_MATERIALOCCLUSIONTEXTUREINFO, SchemaFlags::DisableSchemaMaterialOcclusionTextureInfo },
        { SCHEMA_URI_MATERIALPBRMETALLICROUGHNESS, SchemaFlags::DisableSchemaMaterialPBRMetallicRoughness },
        { SCHEMA_URI_TEXTURE, SchemaFlags::DisableSchemaTexture },
        { SCHEMA_URI_TEXTUREINFO, SchemaFlags::DisableSchemaTextureInfo },
        { SCHEMA_URI_IMAGE, SchemaFlags::DisableSchemaImage },
        { SCHEMA_URI_SAMPLER, SchemaFlags::DisableSchemaSampler },
        { SCHEMA_URI_ANIMATION, SchemaFlags::DisableSchemaAnimation },
        { SCHEMA_URI_ANIMATIONSAMPLER, SchemaFlags::DisableSchemaAnimationSampler },
        { SCHEMA_URI_ANIMATIONCHANNEL, SchemaFlags::DisableSchemaAnimationChannel },
        { SCHEMA_URI_ANIMATIONCHANNELTARGET, SchemaFlags::DisableSchemaAnimationChannelTarget },
        { SCHEMA_URI_EXTENSION, SchemaFlags::DisableSchemaExtension },
        { SCHEMA_URI_EXTRAS, SchemaFlags::DisableSchemaExtras }
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
                const char* schemaContent = schemaLocator->GetSchemaContent(uri);

                if (document.Parse(schemaContent).HasParseError())
                {
                    std::stringstream ss;

                    ss << "Schema document (";
                    ss << uri;
                    ss << ") is not valid JSON";

                    throw GLTFException(ss.str());
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

void Microsoft::glTF::Schema::ValidateDocument(const rapidjson::Document& document, const std::string& schemaUri, SchemaLocatorPtr schemaLocator, SchemaFlags schemaFlags)
{
    if (schemaLocator == nullptr)
    {
        schemaLocator = GetDefaultSchemaLocator();
    }

    RemoteSchemaDocumentProvider provider(std::move(schemaLocator), schemaFlags);

    if (auto* schemaDocument = provider.GetRemoteDocument(schemaUri))
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
