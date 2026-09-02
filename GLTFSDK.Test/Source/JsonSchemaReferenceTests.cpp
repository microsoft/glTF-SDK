// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <nlohmann/json.hpp>

#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/internal/uri.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validator.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using namespace glTF::UnitTest;

namespace
{
    using Json = nlohmann::ordered_json;
    using Adapter = valijson::adapters::NlohmannJsonAdapter;

    struct SchemaSource
    {
        std::unordered_map<std::string, Json> documents;
        std::vector<std::string> requests;

        const Json* Fetch(const std::string& uri)
        {
            requests.push_back(uri);
            const auto document = documents.find(uri);
            return document == documents.end()
                ? nullptr
                : new Json(document->second);
        }
    };

    void PopulateSchema(
        const Json& schemaDocument,
        SchemaSource& source,
        valijson::Schema& schema)
    {
        valijson::SchemaParser parser(valijson::SchemaParser::kDraft4);
        parser.populateSchema(
            Adapter(schemaDocument),
            schema,
            [&source](const std::string& uri)
            {
                return source.Fetch(uri);
            },
            [](const Json* document)
            {
                delete document;
            });
    }

    bool Validate(const valijson::Schema& schema, const char* instance)
    {
        const Json document = Json::parse(instance);
        valijson::Validator validator;
        return validator.validate(schema, Adapter(document), nullptr);
    }

    std::size_t CountRequests(
        const SchemaSource& source,
        const std::string& uri)
    {
        std::size_t count = 0U;
        for (const auto& request : source.requests)
        {
            if (request == uri)
            {
                ++count;
            }
        }
        return count;
    }
}

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(JsonSchemaReferenceTests)
            {
                GLTFSDK_TEST_METHOD(JsonSchemaReferenceTests, ResolvesRelativeUris)
                {
                    using valijson::internal::uri::resolveRelativeUri;

                    Assert::IsTrue(
                        resolveRelativeUri(
                            "schemas/root.json",
                            "nested/value.json") ==
                        "schemas/nested/value.json");
                    Assert::IsTrue(
                        resolveRelativeUri(
                            "schemas/nested/child.json",
                            "../common/value.json") ==
                        "schemas/common/value.json");
                    Assert::IsTrue(
                        resolveRelativeUri(
                            "http://example.com/a/b/root.json",
                            "../../common.json") ==
                        "http://example.com/common.json");
                }

                GLTFSDK_TEST_METHOD(JsonSchemaReferenceTests, ResolvesNestedParentReference)
                {
                    const Json root = Json::parse(R"({
                        "id": "schemas/root.json",
                        "type": "object",
                        "required": ["child"],
                        "properties": {
                            "child": {"$ref": "level/child.json"}
                        }
                    })");

                    SchemaSource source;
                    source.documents.emplace(
                        "schemas/level/child.json",
                        Json::parse(R"({
                            "type": "object",
                            "required": ["value"],
                            "properties": {
                                "value": {
                                    "$ref": "../common/value.json#/definitions/value"
                                }
                            }
                        })"));
                    source.documents.emplace(
                        "schemas/common/value.json",
                        Json::parse(R"({
                            "definitions": {
                                "value": {
                                    "type": "integer",
                                    "minimum": 1
                                }
                            }
                        })"));

                    valijson::Schema schema;
                    PopulateSchema(root, source, schema);

                    Assert::IsTrue(Validate(
                        schema, R"({"child":{"value":2}})"));
                    Assert::IsFalse(Validate(
                        schema, R"({"child":{"value":0}})"));
                    Assert::IsTrue(source.requests == std::vector<std::string>({
                        "schemas/level/child.json",
                        "schemas/common/value.json"
                    }));
                }

                GLTFSDK_TEST_METHOD(JsonSchemaReferenceTests, ResolvesExternalFragmentWithoutFetchingFragment)
                {
                    const Json root = Json::parse(R"({
                        "id": "root.json",
                        "$ref": "defs.json#/definitions/value"
                    })");

                    SchemaSource source;
                    source.documents.emplace(
                        "defs.json",
                        Json::parse(R"({
                            "definitions": {
                                "value": {"type": "string", "minLength": 2}
                            }
                        })"));

                    valijson::Schema schema;
                    PopulateSchema(root, source, schema);

                    Assert::IsTrue(Validate(schema, R"("ok")"));
                    Assert::IsFalse(Validate(schema, R"("x")"));
                    Assert::IsTrue(source.requests == std::vector<std::string>({
                        "defs.json"
                    }));
                }

                GLTFSDK_TEST_METHOD(JsonSchemaReferenceTests, ResolvesDraft04IdScopes)
                {
                    const Json root = Json::parse(R"({
                        "id": "schemas/root.json",
                        "definitions": {
                            "wrapper": {
                                "id": "scoped/",
                                "type": "object",
                                "required": ["value"],
                                "properties": {
                                    "value": {"$ref": "value.json"}
                                }
                            }
                        },
                        "$ref": "#/definitions/wrapper"
                    })");

                    SchemaSource source;
                    source.documents.emplace(
                        "schemas/scoped/value.json",
                        Json::parse(R"({"type":"integer","minimum":10})"));

                    valijson::Schema schema;
                    PopulateSchema(root, source, schema);

                    Assert::IsTrue(Validate(schema, R"({"value":10})"));
                    Assert::IsFalse(Validate(schema, R"({"value":9})"));
                    Assert::IsTrue(source.requests == std::vector<std::string>({
                        "schemas/scoped/value.json"
                    }));
                }

                GLTFSDK_TEST_METHOD(JsonSchemaReferenceTests, ResolvesExternalNamedId)
                {
                    const Json root = Json::parse(R"({
                        "id": "root.json",
                        "$ref": "defs.json#named"
                    })");

                    SchemaSource source;
                    source.documents.emplace(
                        "defs.json",
                        Json::parse(R"({
                            "definitions": {
                                "value": {
                                    "id": "defs.json#named",
                                    "type": "integer",
                                    "minimum": 5
                                }
                            }
                        })"));

                    valijson::Schema schema;
                    PopulateSchema(root, source, schema);

                    Assert::IsTrue(Validate(schema, "5"));
                    Assert::IsFalse(Validate(schema, "4"));
                    Assert::IsTrue(source.requests == std::vector<std::string>({
                        "defs.json"
                    }));
                }

                GLTFSDK_TEST_METHOD(JsonSchemaReferenceTests, ReusesRepeatedReference)
                {
                    const Json root = Json::parse(R"({
                        "id": "root.json",
                        "type": "object",
                        "properties": {
                            "first": {"$ref": "defs.json#/definitions/value"},
                            "second": {"$ref": "defs.json#/definitions/value"}
                        }
                    })");

                    SchemaSource source;
                    source.documents.emplace(
                        "defs.json",
                        Json::parse(R"({
                            "definitions": {
                                "value": {"type": "boolean"}
                            }
                        })"));

                    valijson::Schema schema;
                    PopulateSchema(root, source, schema);

                    Assert::IsTrue(Validate(
                        schema, R"({"first":true,"second":false})"));
                    Assert::IsFalse(Validate(
                        schema, R"({"first":true,"second":0})"));
                    Assert::IsTrue(CountRequests(source, "defs.json") == 1U);
                }

                GLTFSDK_TEST_METHOD(JsonSchemaReferenceTests, SupportsRecursiveSchema)
                {
                    const Json root = Json::parse(R"({
                        "id": "root.json",
                        "type": "object",
                        "properties": {
                            "node": {"$ref": "node.json"}
                        }
                    })");

                    SchemaSource source;
                    source.documents.emplace(
                        "node.json",
                        Json::parse(R"({
                            "id": "node.json",
                            "type": "object",
                            "required": ["value"],
                            "properties": {
                                "value": {"type": "integer"},
                                "next": {"$ref": "node.json"}
                            }
                        })"));

                    valijson::Schema schema;
                    PopulateSchema(root, source, schema);

                    Assert::IsTrue(Validate(
                        schema,
                        R"({"node":{"value":1,"next":{"value":2}}})"));
                    Assert::IsFalse(Validate(
                        schema,
                        R"({"node":{"value":1,"next":{"value":"bad"}}})"));
                    Assert::IsTrue(CountRequests(source, "node.json") == 1U);
                }

                GLTFSDK_TEST_METHOD(JsonSchemaReferenceTests, RejectsReferenceOnlyCycle)
                {
                    const Json root = Json::parse(R"({
                        "id": "root.json",
                        "type": "object",
                        "properties": {
                            "value": {"$ref": "a.json"}
                        }
                    })");

                    SchemaSource source;
                    source.documents.emplace(
                        "a.json",
                        Json::parse(R"({"$ref":"b.json"})"));
                    source.documents.emplace(
                        "b.json",
                        Json::parse(R"({"$ref":"a.json"})"));

                    valijson::Schema schema;
                    Assert::ExpectException<std::runtime_error>(
                        [&root, &source, &schema]()
                        {
                            PopulateSchema(root, source, schema);
                        });
                }

                GLTFSDK_TEST_METHOD(JsonSchemaReferenceTests, RejectsMissingFragment)
                {
                    const Json root = Json::parse(R"({
                        "id": "root.json",
                        "$ref": "defs.json#/definitions/missing"
                    })");

                    SchemaSource source;
                    source.documents.emplace(
                        "defs.json",
                        Json::parse(R"({"definitions":{}})"));

                    valijson::Schema schema;
                    Assert::ExpectException<std::runtime_error>(
                        [&root, &source, &schema]()
                        {
                            PopulateSchema(root, source, schema);
                        });
                    Assert::IsTrue(source.requests == std::vector<std::string>({
                        "defs.json"
                    }));
                }

                GLTFSDK_TEST_METHOD(JsonSchemaReferenceTests, KeepsCompilationCachesIsolated)
                {
                    const Json root = Json::parse(R"({
                        "id": "root.json",
                        "$ref": "value.json"
                    })");

                    SchemaSource integerSource;
                    integerSource.documents.emplace(
                        "value.json",
                        Json::parse(R"({"type":"integer"})"));
                    valijson::Schema integerSchema;
                    PopulateSchema(root, integerSource, integerSchema);

                    SchemaSource stringSource;
                    stringSource.documents.emplace(
                        "value.json",
                        Json::parse(R"({"type":"string"})"));
                    valijson::Schema stringSchema;
                    PopulateSchema(root, stringSource, stringSchema);

                    Assert::IsTrue(Validate(integerSchema, "1"));
                    Assert::IsFalse(Validate(integerSchema, R"("one")"));
                    Assert::IsTrue(Validate(stringSchema, R"("one")"));
                    Assert::IsFalse(Validate(stringSchema, "1"));
                    Assert::IsTrue(
                        CountRequests(integerSource, "value.json") == 1U);
                    Assert::IsTrue(
                        CountRequests(stringSource, "value.json") == 1U);
                }
            };
        }
    }
}
