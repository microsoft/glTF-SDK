// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/Exceptions.h>
#include <GLTFSDK/SchemaValidation.h>

#include "Internal/Json.h"
#include "Internal/JsonSchema.h"

#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validation_results.hpp>
#include <valijson/validator.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace Microsoft::glTF;

namespace
{
    using Adapter = valijson::adapters::NlohmannJsonAdapter;
    using JsonValue = Internal::JsonValue;

    std::string CanonicalDocumentUri(const std::string& uri)
    {
        const auto fragment = uri.find('#');
        return fragment == std::string::npos
            ? uri
            : uri.substr(0U, fragment);
    }

    std::string EscapePointerToken(const std::string& token)
    {
        std::string escaped;
        for (const char character : token)
        {
            if (character == '~')
            {
                escaped += "~0";
            }
            else if (character == '/')
            {
                escaped += "~1";
            }
            else
            {
                escaped += character;
            }
        }
        return escaped;
    }

    std::string ContextToPointer(
        const std::vector<std::string>& context)
    {
        std::string pointer = "#";
        for (const auto& component : context)
        {
            if (component == "<root>")
            {
                continue;
            }

            std::string token = component;
            if (token.size() >= 2U &&
                token.front() == '[' &&
                token.back() == ']')
            {
                token = token.substr(1U, token.size() - 2U);
            }
            pointer += "/" + EscapePointerToken(token);
        }
        return pointer;
    }

    const valijson::ValidationResults::Error* SelectPrimaryError(
        const valijson::ValidationResults& results)
    {
        const valijson::ValidationResults::Error* selected = nullptr;
        for (const auto& error : results)
        {
            if (error.keyword.empty())
            {
                continue;
            }
            if (selected == nullptr ||
                error.context.size() > selected->context.size())
            {
                selected = &error;
            }
        }
        return selected;
    }
}

struct Internal::Draft4ValidationSession::Impl
{
    enum class DocumentState
    {
        Loading,
        Loaded
    };

    Impl(
        std::unique_ptr<const ISchemaLocator> locator,
        std::string schemaUri)
        : schemaLocator(std::move(locator)),
          rootSchemaUri(std::move(schemaUri)),
          rootDocumentUri(CanonicalDocumentUri(rootSchemaUri))
    {
    }

    JsonValue& LoadDocument(const std::string& requestedUri)
    {
        const std::string uri = CanonicalDocumentUri(requestedUri);
        if (uri.empty())
        {
            throw GLTFException(
                "Schema document URI must not be empty");
        }

        const auto cached = documents.find(uri);
        if (cached != documents.end())
        {
            return *cached->second;
        }

        const auto state = documentStates.find(uri);
        if (state != documentStates.end() &&
            state->second == DocumentState::Loading)
        {
            throw GLTFException(
                "Schema document at " + uri +
                " has an unresolved reference cycle");
        }
        documentStates[uri] = DocumentState::Loading;

        const char* locatedContent = nullptr;
        try
        {
            locatedContent = schemaLocator->GetSchemaContent(uri);
        }
        catch (const std::exception& exception)
        {
            throw GLTFException(
                "Schema locator failed for " + uri + ": " +
                exception.what());
        }

        if (locatedContent == nullptr)
        {
            throw GLTFException(
                "Schema document at " + uri + " could not be located");
        }

        const std::string content(locatedContent);
        std::unique_ptr<JsonValue> parsed;
        try
        {
            parsed.reset(new JsonValue(Internal::ParseJson(content)));
        }
        catch (const GLTFException&)
        {
            throw GLTFException(
                "Schema document at " + uri + " is not valid JSON");
        }

        JsonValue& result = *parsed;
        documents.emplace(uri, std::move(parsed));
        documentStates[uri] = DocumentState::Loaded;
        return result;
    }

    void Validate(const JsonValue& document)
    {
        JsonValue& rootSchema = LoadDocument(rootDocumentUri);
        JsonValue fragmentSchema;
        JsonValue* schemaToCompile = &rootSchema;
        const auto fragment = rootSchemaUri.find('#');
        if (fragment != std::string::npos &&
            fragment + 1U < rootSchemaUri.size())
        {
            fragmentSchema = Internal::CreateJsonObject();
            Internal::SetJsonMember(
                fragmentSchema,
                "id",
                Internal::CreateJsonString(
                    "urn:gltf-sdk:validation-root"));
            Internal::SetJsonMember(
                fragmentSchema,
                "$ref",
                Internal::CreateJsonString(rootSchemaUri));
            schemaToCompile = &fragmentSchema;
        }
        else if (rootSchema.is_object() &&
                 rootSchema.find("id") == rootSchema.end())
        {
            Internal::SetJsonMember(
                rootSchema,
                "id",
                Internal::CreateJsonString(rootDocumentUri));
        }

        valijson::Schema schema;
        try
        {
            valijson::SchemaParser parser(
                valijson::SchemaParser::kDraft4);
            parser.populateSchema(
                Adapter(*schemaToCompile),
                schema,
                [this](const std::string& uri)
                {
                    return &LoadDocument(uri);
                },
                [](const JsonValue*)
                {
                });
        }
        catch (const GLTFException&)
        {
            throw;
        }
        catch (const std::exception& exception)
        {
            throw GLTFException(
                "Schema document at " + rootSchemaUri +
                " is invalid: " + exception.what());
        }

        valijson::ValidationResults results;
        valijson::Validator validator(
            valijson::Validator::kStrongTypes,
            valijson::Validator::kStrictDateTime);
        bool valid = validator.validate(
            schema,
            Adapter(document),
            nullptr);
        if (!valid)
        {
            valid = validator.validate(
                schema,
                Adapter(document),
                &results);
        }
        if (valid)
        {
            return;
        }

        const auto* error = SelectPrimaryError(results);
        if (error == nullptr)
        {
            throw ValidationException(
                "Schema violation at # due to unknown");
        }

        throw ValidationException(
            "Schema violation at " +
            ContextToPointer(error->context) +
            " due to " + error->keyword);
    }

    std::unique_ptr<const ISchemaLocator> schemaLocator;
    std::string rootSchemaUri;
    std::string rootDocumentUri;
    std::unordered_map<std::string, std::unique_ptr<JsonValue>> documents;
    std::unordered_map<std::string, DocumentState> documentStates;
};

Internal::Draft4ValidationSession::Draft4ValidationSession(
    std::unique_ptr<const ISchemaLocator> schemaLocator,
    std::string rootSchemaUri)
{
    if (!schemaLocator)
    {
        throw GLTFException(
            "ISchemaLocator instance must not be null");
    }
    m_impl.reset(new Impl(
        std::move(schemaLocator),
        std::move(rootSchemaUri)));
}

Internal::Draft4ValidationSession::~Draft4ValidationSession() = default;

void Internal::Draft4ValidationSession::Validate(
    const JsonValue& document)
{
    m_impl->Validate(document);
}

void Microsoft::glTF::Internal::ValidateJsonAgainstSchema(
    const JsonValue& document,
    const std::string& schemaUri,
    std::unique_ptr<const ISchemaLocator> schemaLocator)
{
    Draft4ValidationSession session(
        std::move(schemaLocator),
        schemaUri);
    session.Validate(document);
}

void GLTFSDK_API Microsoft::glTF::ValidateDocumentAgainstSchema(
    const std::string& documentJson,
    const std::string& schemaUri,
    std::unique_ptr<const ISchemaLocator> schemaLocator)
{
    Internal::ValidateJsonAgainstSchema(
        Internal::ParseJson(documentJson),
        schemaUri,
        std::move(schemaLocator));
}
