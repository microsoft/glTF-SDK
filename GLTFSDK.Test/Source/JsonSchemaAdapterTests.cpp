// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <nlohmann/json.hpp>

#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validation_results.hpp>
#include <valijson/validator.hpp>

#include <cstdint>
#include <cstring>
#include <limits>
#include <new>
#include <string>
#include <type_traits>
#include <vector>

using namespace glTF::UnitTest;

namespace
{
    void* AllocatePatternedMemory(std::size_t size)
    {
        void* memory = ::operator new(size, std::nothrow);
        if (memory != nullptr)
        {
            std::memset(memory, 0xA5, size);
        }
        return memory;
    }

    void FreePatternedMemory(void* memory)
    {
        ::operator delete(memory);
    }

    using Json = nlohmann::ordered_json;
    using Adapter = valijson::adapters::NlohmannJsonAdapter;

    valijson::Schema CompileUniqueItemsSchema()
    {
        const Json schemaDocument = Json::parse(
            R"({"type":"array","uniqueItems":true})");
        valijson::Schema schema;
        valijson::SchemaParser parser(
            valijson::SchemaParser::kDraft4);
        parser.populateSchema(Adapter(schemaDocument), schema);
        return schema;
    }

    bool ContainsUniqueItemsError(
        const valijson::ValidationResults& results)
    {
        for (const auto& error : results)
        {
            if (error.keyword == "uniqueItems")
            {
                return true;
            }
        }
        return false;
    }

    void AssertUniqueItemsResult(
        const valijson::Schema& schema,
        const Json& document,
        bool expected)
    {
        valijson::Validator fastValidator(
            valijson::Validator::kStrongTypes,
            valijson::Validator::kStrictDateTime);
        const bool fastResult = fastValidator.validate(
            schema, Adapter(document), nullptr);

        valijson::ValidationResults results;
        valijson::Validator diagnosticValidator(
            valijson::Validator::kStrongTypes,
            valijson::Validator::kStrictDateTime);
        const bool diagnosticResult = diagnosticValidator.validate(
            schema, Adapter(document), &results);

        Assert::IsTrue(fastResult == diagnosticResult);
        Assert::IsTrue(fastResult == expected);
        Assert::IsTrue(
            expected
                ? results.numErrors() == 0U
                : ContainsUniqueItemsError(results));
    }

    void AssertFastMatchesPairwise(
        const valijson::Schema& schema,
        const Json& document)
    {
        valijson::Validator fastValidator(
            valijson::Validator::kStrongTypes,
            valijson::Validator::kStrictDateTime);
        const bool fastResult = fastValidator.validate(
            schema, Adapter(document), nullptr);

        valijson::ValidationResults results;
        valijson::Validator diagnosticValidator(
            valijson::Validator::kStrongTypes,
            valijson::Validator::kStrictDateTime);
        const bool pairwiseResult = diagnosticValidator.validate(
            schema, Adapter(document), &results);

        Assert::IsTrue(fastResult == pairwiseResult);
    }
}

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(JsonSchemaAdapterTests)
            {
                GLTFSDK_TEST_METHOD(JsonSchemaAdapterTests, UsesOrderedJsonDocumentType)
                {
                    using Adapter = valijson::adapters::NlohmannJsonAdapter;
                    using DocumentType =
                        valijson::adapters::AdapterTraits<Adapter>::DocumentType;

                    static_assert(
                        std::is_same<DocumentType, nlohmann::ordered_json>::value,
                        "Valijson must use the authoritative ordered_json DOM");

                    DocumentType document = DocumentType::object();
                    document["first"] = 1;
                    document["second"] = 2;
                    document["third"] = 3;

                    const Adapter adapter(document);
                    const auto object = adapter.getObject();
                    std::vector<std::string> keys;
                    for (const auto& member : object)
                    {
                        keys.push_back(member.first);
                    }

                    Assert::IsTrue(keys == std::vector<std::string>({
                        "first",
                        "second",
                        "third"
                    }));
                }

                GLTFSDK_TEST_METHOD(JsonSchemaAdapterTests, PreservesNumericCategories)
                {
                    nlohmann::ordered_json document = nlohmann::ordered_json::object();
                    document["signed"] = -1;
                    document["unsigned"] = static_cast<std::uint64_t>(4294967295ULL);
                    document["floating"] = 1.5;

                    const valijson::adapters::NlohmannJsonAdapter signedValue(
                        document["signed"]);
                    const valijson::adapters::NlohmannJsonAdapter unsignedValue(
                        document["unsigned"]);
                    const valijson::adapters::NlohmannJsonAdapter floatingValue(
                        document["floating"]);

                    Assert::IsTrue(signedValue.isInteger());
                    Assert::IsTrue(unsignedValue.isInteger());
                    Assert::IsTrue(floatingValue.isDouble());
                    Assert::IsTrue(signedValue.getInteger() == -1);
                    Assert::IsTrue(unsignedValue.getInteger() == 4294967295LL);
                    Assert::IsTrue(floatingValue.getDouble() == 1.5);
                    Assert::IsTrue(document["unsigned"].is_number_unsigned());
                }

                GLTFSDK_TEST_METHOD(JsonSchemaAdapterTests, ValidatesSameOrderedDom)
                {
                    using Adapter = valijson::adapters::NlohmannJsonAdapter;
                    using DocumentType =
                        valijson::adapters::AdapterTraits<Adapter>::DocumentType;

                    const DocumentType schemaDocument = DocumentType::parse(R"({
                        "$schema": "http://json-schema.org/draft-04/schema",
                        "type": "object",
                        "required": ["value"],
                        "properties": {
                            "value": {
                                "type": "integer",
                                "minimum": 0
                            }
                        }
                    })");
                    const DocumentType validDocument =
                        DocumentType::parse(R"({"value":4294967295})");
                    const DocumentType invalidDocument =
                        DocumentType::parse(R"({"value":-1})");

                    valijson::Schema schema;
                    valijson::SchemaParser parser(valijson::SchemaParser::kDraft4);
                    parser.populateSchema(Adapter(schemaDocument), schema);

                    valijson::Validator validator;
                    Assert::IsTrue(
                        validator.validate(schema, Adapter(validDocument), nullptr));
                    Assert::IsFalse(
                        validator.validate(schema, Adapter(invalidDocument), nullptr));
                    Assert::IsTrue(validDocument["value"].is_number_unsigned());
                }

                GLTFSDK_TEST_METHOD(JsonSchemaAdapterTests, InitializesDefaultSubschemaMetadata)
                {
                    valijson::Schema schema(
                        AllocatePatternedMemory,
                        FreePatternedMemory);
                    const auto* empty = schema.emptySubschema();

                    Assert::IsFalse(empty->hasDescription());
                    Assert::IsFalse(empty->hasId());
                    Assert::IsFalse(empty->hasTitle());
                }

                GLTFSDK_TEST_METHOD(JsonSchemaAdapterTests, UniqueItemsFastPathCoversScalarKinds)
                {
                    const auto schema = CompileUniqueItemsSchema();

                    AssertUniqueItemsResult(
                        schema, Json::parse("[null,null]"), false);
                    AssertUniqueItemsResult(
                        schema, Json::parse("[null,true]"), true);
                    AssertUniqueItemsResult(
                        schema, Json::parse("[true,true]"), false);
                    AssertUniqueItemsResult(
                        schema, Json::parse("[true,false]"), true);
                    AssertUniqueItemsResult(
                        schema, Json::parse(R"(["a","a"])"), false);
                    AssertUniqueItemsResult(
                        schema, Json::parse(R"(["a","b"])"), true);
                    AssertUniqueItemsResult(
                        schema, Json::parse("[-1,-1]"), false);
                    AssertUniqueItemsResult(
                        schema, Json::parse("[-1,-2]"), true);
                    AssertUniqueItemsResult(
                        schema, Json::parse("[1,1]"), false);
                    AssertUniqueItemsResult(
                        schema, Json::parse("[1,2]"), true);
                    AssertUniqueItemsResult(
                        schema, Json::parse("[1.25,1.25]"), false);
                    AssertUniqueItemsResult(
                        schema, Json::parse("[1.25,1.5]"), true);
                    AssertUniqueItemsResult(
                        schema,
                        Json::parse(R"([null,true,1,"1"])"),
                        true);
                    AssertUniqueItemsResult(
                        schema,
                        Json::parse(R"([null,true,1,1.0,"1"])"),
                        false);
                }

                GLTFSDK_TEST_METHOD(JsonSchemaAdapterTests, UniqueItemsFastPathPreservesNumericEquality)
                {
                    const auto schema = CompileUniqueItemsSchema();

                    AssertUniqueItemsResult(
                        schema, Json::parse("[-0.0,0]"), false);
                    AssertUniqueItemsResult(
                        schema, Json::parse("[-0.0,0.0]"), false);
                    AssertUniqueItemsResult(
                        schema, Json::parse("[1,1.0]"), false);
                    AssertUniqueItemsResult(
                        schema,
                        Json::parse(
                            "[9007199254740991,9007199254740992]"),
                        true);
                    AssertUniqueItemsResult(
                        schema,
                        Json::parse(
                            "[9007199254740992,9007199254740994]"),
                        true);

                    AssertFastMatchesPairwise(
                        schema,
                        Json::parse(
                            "[9007199254740992,9007199254740993]"));

                    Json signedBoundaries = Json::array();
                    signedBoundaries.push_back(
                        std::numeric_limits<std::int64_t>::min());
                    signedBoundaries.push_back(
                        std::numeric_limits<std::int64_t>::max());
                    AssertUniqueItemsResult(
                        schema, signedBoundaries, true);
                    signedBoundaries.push_back(
                        std::numeric_limits<std::int64_t>::max());
                    AssertUniqueItemsResult(
                        schema, signedBoundaries, false);

                    Json unsignedBoundaries = Json::array();
                    unsignedBoundaries.push_back(
                        std::numeric_limits<std::uint64_t>::max());
                    unsignedBoundaries.push_back(
                        std::numeric_limits<std::uint64_t>::max() - 1U);
                    AssertUniqueItemsResult(
                        schema, unsignedBoundaries, true);
                    unsignedBoundaries.push_back(
                        std::numeric_limits<std::uint64_t>::max());
                    AssertUniqueItemsResult(
                        schema, unsignedBoundaries, false);

                    Json signedAndUnsigned = Json::array();
                    signedAndUnsigned.push_back(
                        static_cast<std::int64_t>(1));
                    signedAndUnsigned.push_back(
                        static_cast<std::uint64_t>(1));
                    AssertUniqueItemsResult(
                        schema, signedAndUnsigned, false);
                }

                GLTFSDK_TEST_METHOD(JsonSchemaAdapterTests, UniqueItemsFallsBackForStructuredValues)
                {
                    const auto schema = CompileUniqueItemsSchema();

                    AssertUniqueItemsResult(
                        schema,
                        Json::parse(R"([{"a":1},{"a":2}])"),
                        true);
                    AssertUniqueItemsResult(
                        schema,
                        Json::parse(R"([{"a":1},{"a":1}])"),
                        false);
                    AssertUniqueItemsResult(
                        schema, Json::parse("[[1],[2]]"), true);
                    AssertUniqueItemsResult(
                        schema, Json::parse("[[1],[1]]"), false);
                    AssertUniqueItemsResult(
                        schema, Json::parse("[[1,2],[2,1]]"), true);
                    AssertUniqueItemsResult(
                        schema, Json::parse("[[1,2],[1,2]]"), false);
                    AssertUniqueItemsResult(
                        schema,
                        Json::parse(R"([1,{"a":1},2])"),
                        true);
                    AssertUniqueItemsResult(
                        schema,
                        Json::parse(R"([1,{"a":1},1])"),
                        false);
                }

                GLTFSDK_TEST_METHOD(JsonSchemaAdapterTests, UniqueItemsFallsBackForNonFiniteNumbers)
                {
                    const auto schema = CompileUniqueItemsSchema();

                    Json notANumber = Json::array();
                    notANumber.push_back(
                        std::numeric_limits<double>::quiet_NaN());
                    notANumber.push_back(
                        std::numeric_limits<double>::quiet_NaN());
                    AssertFastMatchesPairwise(schema, notANumber);

                    Json infinity = Json::array();
                    infinity.push_back(
                        std::numeric_limits<double>::infinity());
                    infinity.push_back(
                        std::numeric_limits<double>::infinity());
                    AssertFastMatchesPairwise(schema, infinity);
                }
            };
        }
    }
}
