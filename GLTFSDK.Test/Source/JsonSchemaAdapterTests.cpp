// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <nlohmann/json.hpp>

#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validator.hpp>

#include <cstdint>
#include <cstring>
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
            };
        }
    }
}
