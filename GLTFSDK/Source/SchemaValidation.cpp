// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/SchemaValidation.h>
#include <GLTFSDK/Exceptions.h>

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

        const rapidjson::SchemaDocument* GetRemoteDocumentStr(const std::string& uri)
        {
            auto itDoc = schemaDocuments.find(uri);

            if (itDoc != schemaDocuments.end())
            {
                return &(itDoc->second);
            }

            rapidjson::Document document;

            if (document.Parse(schemaLocator->GetSchemaContent(uri)).HasParseError())
            {
                throw GLTFException("Schema document at " + uri + " is not valid JSON");
            }

            auto result = schemaDocuments.emplace(uri, rapidjson::SchemaDocument(document, nullptr, 0, this));
            assert(result.second);
            auto resultSchemaDoc = &(result.first->second);
            assert(resultSchemaDoc);

            return resultSchemaDoc;
        }

        const rapidjson::SchemaDocument* GetRemoteDocument(const char* uri, rapidjson::SizeType length) override
        {
            return GetRemoteDocumentStr(std::string(uri, length));
        }

        const std::unique_ptr<const ISchemaLocator> schemaLocator;

    private:
        std::unordered_map<std::string, rapidjson::SchemaDocument> schemaDocuments;
    };
}

void Microsoft::glTF::ValidateDocumentAgainstSchema(const rapidjson::Document& document, const std::string& schemaUri, std::unique_ptr<const ISchemaLocator> schemaLocator)
{
    if (!schemaLocator)
    {
        throw GLTFException("ISchemaLocator instance must not be null");
    }

    RemoteSchemaDocumentProvider provider(std::move(schemaLocator));

    if (auto* schemaDocument = provider.GetRemoteDocumentStr(schemaUri))
    {
        rapidjson::SchemaValidator schemaValidator(*schemaDocument);

        if (!document.Accept(schemaValidator))
        {
            rapidjson::StringBuffer sb;

            const std::string schemaKeyword = schemaValidator.GetInvalidSchemaKeyword();
            schemaValidator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
            const std::string schemaInvalid = sb.GetString();

            throw ValidationException("Schema violation at " + schemaInvalid + " due to " + schemaKeyword);
        }
    }
    else
    {
        throw GLTFException("Schema document at " + schemaUri + " could not be located");
    }
}
