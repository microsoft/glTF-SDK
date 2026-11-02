// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/Exceptions.h>
#include <GLTFSDK/SchemaValidation.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

using namespace glTF::UnitTest;

namespace
{
    class TestSchemaLocator final : public Microsoft::glTF::ISchemaLocator
    {
    public:
        explicit TestSchemaLocator(
            std::unordered_map<std::string, std::string> schemas)
            : m_schemas(std::move(schemas))
        {
        }

        const char* GetSchemaContent(
            const std::string& uri) const override
        {
            const auto schema = m_schemas.find(uri);
            return schema == m_schemas.end()
                ? nullptr
                : schema->second.c_str();
        }

    private:
        std::unordered_map<std::string, std::string> m_schemas;
    };

    std::unique_ptr<const Microsoft::glTF::ISchemaLocator> SchemaLocator()
    {
        return std::unique_ptr<const Microsoft::glTF::ISchemaLocator>(
            new TestSchemaLocator({
                {
                    "root.json",
                    R"({
                        "$schema":
                            "http://json-schema.org/draft-04/schema",
                        "type": "object",
                        "additionalProperties": {
                            "type": "integer"
                        }
                    })"
                }
            }));
    }

    std::string EscapePointerToken(const std::string& token)
    {
        std::string escaped;
        escaped.reserve(token.size() * 2U);
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
                escaped.push_back(character);
            }
        }
        return escaped;
    }

    void ExpectTypeFailureAt(const std::string& key)
    {
        const std::string document =
            "{\"" + key + "\":false}";
        const std::string expected =
            "Schema violation at #/" +
            EscapePointerToken(key) +
            " due to type";

        Assert::ExpectException<Microsoft::glTF::ValidationException>([&]()
        {
            try
            {
                Microsoft::glTF::ValidateDocumentAgainstSchema(
                    document,
                    "root.json",
                    SchemaLocator());
            }
            catch (const Microsoft::glTF::ValidationException& exception)
            {
                Assert::AreEqual(expected, std::string(exception.what()));
                throw;
            }
        });
    }
}

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(SchemaValidationTests)
            {
                GLTFSDK_TEST_METHOD(
                    SchemaValidationTests,
                    EscapedMemberPaths)
                {
                    ExpectTypeFailureAt("");
                    ExpectTypeFailureAt("field");
                    ExpectTypeFailureAt("a~/");
                    ExpectTypeFailureAt(std::string(512U, '/'));
                }

                GLTFSDK_TEST_METHOD(
                    SchemaValidationTests,
                    LongEscapedMemberPath)
                {
                    const std::string key(65536U, '/');
                    ExpectTypeFailureAt(key);
                }

                GLTFSDK_TEST_METHOD(
                    SchemaValidationTests,
                    AdditionalMembers)
                {
                    const Document document = Deserialize(
                        R"({
                            "asset": {"version": "2.0"},
                            "": null,
                            "field/~": {"nested": [1, 2, 3]}
                        })");
                    Assert::AreEqual(
                        std::string("2.0"),
                        document.asset.version);
                }
            };
        }
    }
}
