// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include "Internal/Json.h"
#include "Internal/JsonSchema.h"
#include "TestUtils.h"

#include <GLTFSDK/Exceptions.h>
#include <GLTFSDK/SchemaValidation.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace glTF::UnitTest;

namespace
{
    using namespace Microsoft::glTF;

    constexpr const char* ResourceRoot =
        "Resources\\JSON-Schema-Test-Suite\\";

    class SuiteSchemaLocator : public ISchemaLocator
    {
    public:
        explicit SuiteSchemaLocator(std::string rootSchema)
        {
            m_documents.emplace(
                "suite-root.json", std::move(rootSchema));
            AddRemote(
                "http://localhost:1234/integer.json",
                "remotes\\integer.json");
            AddRemote(
                "http://localhost:1234/name.json",
                "remotes\\name.json");
            AddRemote(
                "http://localhost:1234/subSchemas.json",
                "remotes\\subSchemas.json");
            AddRemote(
                "http://localhost:1234/folder/folderInteger.json",
                "remotes\\folder\\folderInteger.json");
            AddRemote(
                "http://json-schema.org/draft-04/schema",
                "remotes\\draft4-schema.json");
        }

        const char* GetSchemaContent(
            const std::string& uri) const override
        {
            const auto document = m_documents.find(uri);
            return document == m_documents.end()
                ? nullptr
                : document->second.c_str();
        }

    private:
        void AddRemote(
            const std::string& uri,
            const std::string& relativePath)
        {
            m_documents.emplace(
                uri,
                Microsoft::glTF::Test::ReadLocalJson(
                    (std::string(ResourceRoot) + relativePath).c_str()));
        }

        std::unordered_map<std::string, std::string> m_documents;
    };

    std::string GetDescription(
        const Internal::JsonValue& object,
        const char* fallback)
    {
        const auto* value =
            Internal::FindJsonMember(object, "description");
        std::string description;
        return value != nullptr &&
            Internal::TryGetJsonString(*value, description)
            ? description
            : std::string(fallback);
    }

    std::size_t RunConformanceFile(
        const std::string& relativePath,
        const char* selectedGroup = nullptr)
    {
        const auto suite = Internal::ParseJson(
            Microsoft::glTF::Test::ReadLocalJson(
                (std::string(ResourceRoot) +
                 "tests\\draft4\\" + relativePath).c_str()));
        Internal::RequireJsonArray(
            suite, "Conformance file root must be an array");

        std::size_t executed = 0U;
        for (std::size_t groupIndex = 0U;
             groupIndex < Internal::GetJsonArraySize(suite);
             ++groupIndex)
        {
            const auto& group = Internal::GetJsonArrayElement(
                suite, groupIndex, "Missing conformance group");
            const std::string groupDescription =
                GetDescription(group, "unnamed group");
            if (selectedGroup != nullptr &&
                groupDescription != selectedGroup)
            {
                continue;
            }
            const auto& schema = Internal::RequireJsonMember(
                group, "schema", "Conformance group is missing schema");
            const auto& tests = Internal::RequireJsonMember(
                group, "tests", "Conformance group is missing tests");
            Internal::RequireJsonArray(
                tests, "Conformance tests must be an array");

            for (std::size_t testIndex = 0U;
                 testIndex < Internal::GetJsonArraySize(tests);
                 ++testIndex)
            {
                const auto& test = Internal::GetJsonArrayElement(
                    tests, testIndex, "Missing conformance test");
                const std::string testDescription =
                    GetDescription(test, "unnamed test");
                const auto& data = Internal::RequireJsonMember(
                    test, "data", "Conformance test is missing data");
                const auto& expectedValue = Internal::RequireJsonMember(
                    test, "valid", "Conformance test is missing validity");
                bool expected = false;
                if (!Internal::TryGetJsonBoolean(
                        expectedValue, expected))
                {
                    throw std::runtime_error(
                        "Conformance validity is not boolean");
                }

                bool actual = true;
                std::string validationFailure;
                try
                {
                    Internal::ValidateJsonAgainstSchema(
                        data,
                        "suite-root.json",
                        std::unique_ptr<const ISchemaLocator>(
                            new SuiteSchemaLocator(
                                Internal::WriteJson(schema))));
                }
                catch (const ValidationException& exception)
                {
                    actual = false;
                    validationFailure = exception.what();
                }
                catch (const GLTFException& exception)
                {
                    throw std::runtime_error(
                        relativePath + " / " + groupDescription +
                        " / " + testDescription +
                        " failed to execute: " + exception.what());
                }

                if (actual != expected)
                {
                    throw std::runtime_error(
                        relativePath + " / " + groupDescription +
                        " / " + testDescription +
                        " expected " + (expected ? "valid" : "invalid") +
                        " but was " + (actual ? "valid" : "invalid") +
                        (validationFailure.empty()
                            ? std::string()
                            : ": " + validationFailure));
                }
                ++executed;
            }
        }

        return executed;
    }
}

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(JsonSchemaConformanceTests)
            {
                GLTFSDK_TEST_METHOD(JsonSchemaConformanceTests, CoreKeywords)
                {
                    const char* files[] = {
                        "additionalItems.json",
                        "additionalProperties.json",
                        "allOf.json",
                        "anyOf.json",
                        "default.json",
                        "definitions.json",
                        "dependencies.json",
                        "enum.json",
                        "items.json",
                        "maximum.json",
                        "maxItems.json",
                        "maxLength.json",
                        "maxProperties.json",
                        "minimum.json",
                        "minItems.json",
                        "minLength.json",
                        "minProperties.json",
                        "multipleOf.json",
                        "not.json",
                        "oneOf.json",
                        "pattern.json",
                        "patternProperties.json",
                        "properties.json",
                        "required.json",
                        "type.json",
                        "uniqueItems.json"
                    };

                    std::size_t executed = 0U;
                    for (const auto* file : files)
                    {
                        executed += RunConformanceFile(file);
                    }
                    Assert::IsTrue(executed == 280U);
                }

                GLTFSDK_TEST_METHOD(JsonSchemaConformanceTests, References)
                {
                    const auto local = RunConformanceFile("ref.json");
                    const auto remote = RunConformanceFile("refRemote.json");

                    Assert::IsTrue(local + remote == 40U);
                }

                GLTFSDK_TEST_METHOD(JsonSchemaConformanceTests, Format)
                {
                    Assert::IsTrue(
                        RunConformanceFile(
                            "optional\\format.json",
                            "validation of date-time strings") == 9U);
                }
            };
        }
    }
}
