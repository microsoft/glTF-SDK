// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <nlohmann/json.hpp>

#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validation_results.hpp>
#include <valijson/validator.hpp>

#include <string>
#include <vector>

using namespace glTF::UnitTest;

namespace
{
    using Json = nlohmann::ordered_json;
    using Adapter = valijson::adapters::NlohmannJsonAdapter;

    valijson::Schema Compile(const char* schemaJson)
    {
        const Json schemaDocument = Json::parse(schemaJson);
        valijson::Schema schema;
        valijson::SchemaParser parser(valijson::SchemaParser::kDraft4);
        parser.populateSchema(Adapter(schemaDocument), schema);
        return schema;
    }

    valijson::ValidationResults Validate(
        const valijson::Schema& schema,
        const char* documentJson)
    {
        const Json document = Json::parse(documentJson);
        valijson::ValidationResults results;
        valijson::Validator validator;
        Assert::IsFalse(validator.validate(schema, Adapter(document), &results));
        return results;
    }

    const valijson::ValidationResults::Error& SelectPrimary(
        const valijson::ValidationResults& results)
    {
        const valijson::ValidationResults::Error* selected = nullptr;
        for (const auto& error : results)
        {
            if (!error.keyword.empty() &&
                (selected == nullptr ||
                 error.context.size() > selected->context.size()))
            {
                selected = &error;
            }
        }

        Assert::IsTrue(selected != nullptr);
        return *selected;
    }

    bool ContainsKeyword(
        const valijson::ValidationResults& results,
        const std::string& keyword)
    {
        for (const auto& error : results)
        {
            if (error.keyword == keyword)
            {
                return true;
            }
        }
        return false;
    }

    void AssertAllErrorsHaveKeywords(
        const valijson::ValidationResults& results)
    {
        for (const auto& error : results)
        {
            Assert::IsFalse(error.keyword.empty());
            Assert::IsFalse(error.description.empty());
            Assert::IsFalse(error.context.empty());
        }
    }
}

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(JsonSchemaDiagnosticsTests)
            {
                GLTFSDK_TEST_METHOD(JsonSchemaDiagnosticsTests, ReportsDeepestMinimum)
                {
                    const auto schema = Compile(R"({
                        "$schema": "http://json-schema.org/draft-04/schema",
                        "type": "object",
                        "properties": {
                            "accessors": {
                                "type": "array",
                                "items": {
                                    "type": "object",
                                    "properties": {
                                        "count": {
                                            "type": "integer",
                                            "minimum": 1
                                        }
                                    }
                                }
                            }
                        }
                    })");

                    const auto results = Validate(
                        schema, R"({"accessors":[{"count":0}]})");
                    const auto& primary = SelectPrimary(results);

                    Assert::IsTrue(primary.keyword == "minimum");
                    Assert::IsTrue(primary.context == std::vector<std::string>({
                        "<root>",
                        "[accessors]",
                        "[0]",
                        "[count]"
                    }));
                    AssertAllErrorsHaveKeywords(results);
                }

                GLTFSDK_TEST_METHOD(JsonSchemaDiagnosticsTests, ReportsRequired)
                {
                    const auto schema = Compile(R"({
                        "$schema": "http://json-schema.org/draft-04/schema",
                        "type": "object",
                        "required": ["asset"]
                    })");

                    const auto results = Validate(schema, "{}");
                    const auto& primary = SelectPrimary(results);

                    Assert::IsTrue(primary.keyword == "required");
                    Assert::IsTrue(primary.context == std::vector<std::string>({
                        "<root>"
                    }));
                    AssertAllErrorsHaveKeywords(results);
                }

                GLTFSDK_TEST_METHOD(JsonSchemaDiagnosticsTests, PreservesCompositionKeywords)
                {
                    const auto schema = Compile(R"({
                        "$schema": "http://json-schema.org/draft-04/schema",
                        "type": "object",
                        "properties": {
                            "value": {
                                "anyOf": [
                                    {"type": "string"},
                                    {"type": "integer", "minimum": 5}
                                ]
                            }
                        }
                    })");

                    const auto results = Validate(
                        schema, R"({"value":false})");
                    const auto& primary = SelectPrimary(results);

                    Assert::IsTrue(primary.keyword == "type");
                    Assert::IsTrue(ContainsKeyword(results, "anyOf"));
                    AssertAllErrorsHaveKeywords(results);
                }

                GLTFSDK_TEST_METHOD(JsonSchemaDiagnosticsTests, MissingDependencyFailsCleanly)
                {
                    const auto schema = Compile(R"({
                        "$schema": "http://json-schema.org/draft-04/schema",
                        "type": "object",
                        "properties": {
                            "accessor": {
                                "type": "object",
                                "dependencies": {
                                    "byteOffset": ["bufferView"]
                                }
                            }
                        }
                    })");

                    const auto results = Validate(
                        schema, R"({"accessor":{"byteOffset":0}})");
                    const auto& primary = SelectPrimary(results);

                    Assert::IsTrue(primary.keyword == "dependencies");
                    Assert::IsTrue(primary.context == std::vector<std::string>({
                        "<root>",
                        "[accessor]"
                    }));
                    AssertAllErrorsHaveKeywords(results);
                }
            };
        }
    }
}
