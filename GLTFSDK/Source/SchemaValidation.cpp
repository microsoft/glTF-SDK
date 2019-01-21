// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/SchemaValidation.h>
#include <GLTFSDK/Exceptions.h>

#include <sstream>
#include <unordered_map>

using namespace Microsoft::glTF;

namespace
{
    class RemoteSchemaDocumentProvider : public rapidjson::IRemoteSchemaDocumentProvider
    {
    public:
        RemoteSchemaDocumentProvider(std::unique_ptr<const ISchemaLocator> schemaLocator) : schemaLocator(std::move(schemaLocator))
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

            if (document.Parse(schemaLocator->GetSchemaContent(uri)).HasParseError())
            {
                std::stringstream ss;

                ss << "Schema document (";
                ss << uri;
                ss << ") is not valid JSON";

                throw GLTFException(ss.str());
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

        const std::unique_ptr<const ISchemaLocator> schemaLocator;

    private:
        std::unordered_map<std::string, rapidjson::SchemaDocument> schemaDocuments;
    };
}

void Microsoft::glTF::ValidateSchema(const rapidjson::Document& document, const std::string& schemaUri, std::unique_ptr<const ISchemaLocator> schemaLocator)
{
    if (!schemaLocator)
    {
        throw GLTFException("ISchemaLocator instance must not be null");
    }

    RemoteSchemaDocumentProvider provider(std::move(schemaLocator));

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
