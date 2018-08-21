// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <memory>

#include <GLTFSDK/RapidJsonUtils.h>
#include <GLTFSDK/Schema.h>

namespace Microsoft
{
    namespace glTF
    {
        class RemoteSchemaDocumentProvider : public rapidjson::IRemoteSchemaDocumentProvider
        {
            std::unordered_map<std::string, std::shared_ptr<const rapidjson::SchemaDocument>> m_schemaDocuments;
        public:
            virtual const rapidjson::SchemaDocument* GetRemoteDocument(const char* uri, rapidjson::SizeType length);
        };

        Microsoft::glTF::rapidjson::Document FetchSchemaDocument(const std::string& uriStr)
        {
            rapidjson::Document sd;
            std::string gltf = GLTFSchema::GLTF_SCHEMA_MAP.at(uriStr);
            sd.Parse(gltf.data());
            if (sd.HasParseError())
            {
                // the schema is not a valid JSON.
                const std::string msg = "Unexpected error: could not parse the GLTF schema at " + uriStr + ".Please try again.";
                throw GLTFException(msg.data());
            }

            return sd;
        }

        const Microsoft::glTF::rapidjson::SchemaDocument* RemoteSchemaDocumentProvider::GetRemoteDocument(const char* uri, rapidjson::SizeType length)
        {
            // Resolve the uri and returns a pointer to that schema.
            std::string schemaToFind(uri, length);
            auto foundSchema = m_schemaDocuments.find(schemaToFind);
            if (foundSchema != m_schemaDocuments.end())
            {
                // schema already created and cached
                return foundSchema->second.get();
            }
            else
            {
                // create the schema and cache it in map
                rapidjson::Document sd = FetchSchemaDocument(schemaToFind);
                auto schema = std::make_shared<rapidjson::SchemaDocument>(sd, this);
                m_schemaDocuments[schemaToFind] = schema;
                return schema.get();
            }
        }

        Microsoft::glTF::rapidjson::SchemaDocument ParseSchemaDocument(const std::string& uriStr, RemoteSchemaDocumentProvider* provider)
        {
            rapidjson::Document sd = FetchSchemaDocument(uriStr);
            rapidjson::SchemaDocument schema(sd, provider);

            return schema;
        }

        void ValidateDocument(const rapidjson::Document& d)
        {
            RemoteSchemaDocumentProvider provider;
            rapidjson::SchemaDocument schema = ParseSchemaDocument(GLTFSchema::ROOT_GLTF_SCHEMA, &provider);
            rapidjson::SchemaValidator validator(schema);
            if (!d.Accept(validator))
            {
                // Input JSON is invalid according to the schema
                rapidjson::StringBuffer sb;
                std::string keyword = validator.GetInvalidSchemaKeyword();
                validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
                std::string document = sb.GetString();
                throw ValidationException("Schema problem at " + document + " due to " + keyword);
            }
        }
    }
}






