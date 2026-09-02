// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include "Internal/Json.h"
#include "Internal/JsonSchema.h"

#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/Exceptions.h>
#include <GLTFSDK/Schema.h>
#include <GLTFSDK/SchemaValidation.h>

#include <functional>
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

    struct LocatorState
    {
        std::unordered_map<std::string, std::string> documents;
        std::unordered_map<std::string, std::size_t> requests;
        std::string throwUri;
        bool reuseBuffer = false;
        mutable std::string buffer;
    };

    class TestSchemaLocator : public ISchemaLocator
    {
    public:
        explicit TestSchemaLocator(std::shared_ptr<LocatorState> state)
            : m_state(std::move(state))
        {
        }

        const char* GetSchemaContent(
            const std::string& uri) const override
        {
            ++m_state->requests[uri];
            if (uri == m_state->throwUri)
            {
                throw std::runtime_error("locator failure");
            }

            const auto document = m_state->documents.find(uri);
            if (document == m_state->documents.end())
            {
                return nullptr;
            }

            if (m_state->reuseBuffer)
            {
                m_state->buffer = document->second;
                return m_state->buffer.c_str();
            }
            return document->second.c_str();
        }

    private:
        std::shared_ptr<LocatorState> m_state;
    };

    std::unique_ptr<const ISchemaLocator> Locator(
        const std::shared_ptr<LocatorState>& state)
    {
        return std::unique_ptr<const ISchemaLocator>(
            new TestSchemaLocator(state));
    }

    std::shared_ptr<LocatorState> SingleSchema(
        const std::string& schema)
    {
        auto state = std::make_shared<LocatorState>();
        state->documents.emplace("root.json", schema);
        return state;
    }

    void ExpectGltfFailureContaining(
        const std::string& expected,
        const std::function<void()>& action)
    {
        Assert::ExpectException<GLTFException>([&]()
        {
            try
            {
                action();
            }
            catch (const GLTFException& exception)
            {
                Assert::IsTrue(
                    std::string(exception.what()).find(expected) !=
                    std::string::npos);
                throw;
            }
        });
    }

    struct SchemaFlagCase
    {
        const char* uri;
        SchemaFlags flag;
    };

    const SchemaFlagCase SchemaFlagCases[] = {
        {SCHEMA_URI_GLTF, SchemaFlags::DisableSchemaRoot},
        {SCHEMA_URI_GLTFID, SchemaFlags::DisableSchemaId},
        {SCHEMA_URI_GLTFCHILDOFROOTPROPERTY, SchemaFlags::DisableSchemaChildOfRoot},
        {SCHEMA_URI_GLTFPROPERTY, SchemaFlags::DisableSchemaProperty},
        {SCHEMA_URI_BUFFER, SchemaFlags::DisableSchemaBuffer},
        {SCHEMA_URI_BUFFERVIEW, SchemaFlags::DisableSchemaBufferView},
        {SCHEMA_URI_ACCESSOR, SchemaFlags::DisableSchemaAccessor},
        {SCHEMA_URI_ACCESSORSPARSE, SchemaFlags::DisableSchemaAccessorSparse},
        {SCHEMA_URI_ACCESSORSPARSEVALUES, SchemaFlags::DisableSchemaAccessorSparseValues},
        {SCHEMA_URI_ACCESSORSPARSEINDICES, SchemaFlags::DisableSchemaAccessorSparseIndices},
        {SCHEMA_URI_ASSET, SchemaFlags::DisableSchemaAsset},
        {SCHEMA_URI_SCENE, SchemaFlags::DisableSchemaScene},
        {SCHEMA_URI_NODE, SchemaFlags::DisableSchemaNode},
        {SCHEMA_URI_MESH, SchemaFlags::DisableSchemaMesh},
        {SCHEMA_URI_MESHPRIMITIVE, SchemaFlags::DisableSchemaMeshPrimitive},
        {SCHEMA_URI_SKIN, SchemaFlags::DisableSchemaSkin},
        {SCHEMA_URI_CAMERA, SchemaFlags::DisableSchemaCamera},
        {SCHEMA_URI_CAMERAORTHOGRAPHIC, SchemaFlags::DisableSchemaCameraOrthographic},
        {SCHEMA_URI_CAMERAPERSPECTIVE, SchemaFlags::DisableSchemaCameraPerspective},
        {SCHEMA_URI_MATERIAL, SchemaFlags::DisableSchemaMaterial},
        {SCHEMA_URI_MATERIALNORMALTEXTUREINFO, SchemaFlags::DisableSchemaMaterialNormalTextureInfo},
        {SCHEMA_URI_MATERIALOCCLUSIONTEXTUREINFO, SchemaFlags::DisableSchemaMaterialOcclusionTextureInfo},
        {SCHEMA_URI_MATERIALPBRMETALLICROUGHNESS, SchemaFlags::DisableSchemaMaterialPBRMetallicRoughness},
        {SCHEMA_URI_TEXTURE, SchemaFlags::DisableSchemaTexture},
        {SCHEMA_URI_TEXTUREINFO, SchemaFlags::DisableSchemaTextureInfo},
        {SCHEMA_URI_IMAGE, SchemaFlags::DisableSchemaImage},
        {SCHEMA_URI_SAMPLER, SchemaFlags::DisableSchemaSampler},
        {SCHEMA_URI_ANIMATION, SchemaFlags::DisableSchemaAnimation},
        {SCHEMA_URI_ANIMATIONSAMPLER, SchemaFlags::DisableSchemaAnimationSampler},
        {SCHEMA_URI_ANIMATIONCHANNEL, SchemaFlags::DisableSchemaAnimationChannel},
        {SCHEMA_URI_ANIMATIONCHANNELTARGET, SchemaFlags::DisableSchemaAnimationChannelTarget},
        {SCHEMA_URI_EXTENSION, SchemaFlags::DisableSchemaExtension},
        {SCHEMA_URI_EXTRAS, SchemaFlags::DisableSchemaExtras}
    };
}

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(JsonSchemaTests)
            {
                GLTFSDK_TEST_METHOD(JsonSchemaTests, BundledSchemasParseAndCompile)
                {
                    const auto& schemas = GetDefaultSchemaUriMap();
                    Assert::IsTrue(schemas.size() == 33U);

                    for (const auto& schemaEntry : schemas)
                    {
                        const auto schemaDocument =
                            Internal::ParseJson(schemaEntry.second);
                        const auto* dialect = Internal::FindJsonMember(
                            schemaDocument, "$schema");
                        std::string dialectValue;

                        Assert::IsTrue(dialect != nullptr);
                        Assert::IsTrue(Internal::TryGetJsonString(
                            *dialect, dialectValue));
                        Assert::IsTrue(
                            dialectValue ==
                            "http://json-schema.org/draft-04/schema");

                        try
                        {
                            ValidateDocumentAgainstSchema(
                                "null",
                                schemaEntry.first,
                                GetDefaultSchemaLocator(
                                    SchemaFlags::None));
                        }
                        catch (const ValidationException&)
                        {
                        }
                    }
                }

                GLTFSDK_TEST_METHOD(JsonSchemaTests, BundledRootAcceptsAndRejectsKnownDocuments)
                {
                    ValidateDocumentAgainstSchema(
                        R"({"asset":{"version":"2.0"}})",
                        SCHEMA_URI_GLTF,
                        GetDefaultSchemaLocator(SchemaFlags::None));

                    Assert::ExpectException<ValidationException>([]()
                    {
                        ValidateDocumentAgainstSchema(
                            R"({"asset":{"version":"2.0.0"}})",
                            SCHEMA_URI_GLTF,
                            GetDefaultSchemaLocator(SchemaFlags::None));
                    });
                }

                GLTFSDK_TEST_METHOD(JsonSchemaTests, BundledGraphKeywordStrength)
                {
                    struct KeywordCase
                    {
                        const char* schema;
                        const char* invalidDocument;
                    };

                    const KeywordCase cases[] = {
                        {R"({"type":"array","items":[{}],"additionalItems":false})", "[1,2]"},
                        {R"({"type":"object","additionalProperties":false})", R"({"x":1})"},
                        {R"({"type":"object","dependencies":{"a":["b"]}})", R"({"a":1})"},
                        {R"({"type":"array","items":{"type":"integer"}})", R"(["x"])"},
                        {R"({"type":"array","maxItems":1})", "[1,2]"},
                        {R"({"type":"array","minItems":2})", "[1]"},
                        {R"({"type":"string","maxLength":1})", R"("ab")"},
                        {R"({"type":"string","minLength":2})", R"("a")"},
                        {R"({"type":"object","maxProperties":1})", R"({"a":1,"b":2})"},
                        {R"({"type":"object","minProperties":1})", "{}"},
                        {R"({"minimum":2})", "1"},
                        {R"({"maximum":1})", "2"},
                        {R"({"minimum":1,"exclusiveMinimum":true})", "1"},
                        {R"({"multipleOf":2})", "3"},
                        {R"({"type":"string","pattern":"^a$"})", R"("b")"},
                        {R"({"enum":[1,2]})", "3"},
                        {R"({"type":"string","format":"time"})", R"("12:00:00")"},
                        {R"({"required":["value"]})", "{}"},
                        {R"({"type":"integer"})", "1.0"},
                        {R"({"allOf":[{"type":"integer"},{"minimum":2}]})", "1"},
                        {R"({"anyOf":[{"type":"integer"},{"type":"string"}]})", "false"},
                        {R"({"oneOf":[{"type":"number"},{"type":"integer"}]})", "1"},
                        {R"({"not":{"type":"integer"}})", "1"},
                        {R"({"uniqueItems":true})", "[1,1]"}
                    };

                    for (const auto& testCase : cases)
                    {
                        auto state = SingleSchema(testCase.schema);
                        bool rejected = false;
                        try
                        {
                            ValidateDocumentAgainstSchema(
                                testCase.invalidDocument,
                                "root.json",
                                Locator(state));
                        }
                        catch (const ValidationException&)
                        {
                            rejected = true;
                        }
                        Assert::IsTrue(rejected);
                    }
                }

                GLTFSDK_TEST_METHOD(JsonSchemaTests, FlagsSubstituteEveryBundledSchema)
                {
                    for (const auto& testCase : SchemaFlagCases)
                    {
                        const auto locator =
                            GetDefaultSchemaLocator(testCase.flag);
                        Assert::AreEqual(
                            "{}",
                            locator->GetSchemaContent(testCase.uri));

                        ValidateDocumentAgainstSchema(
                            "null",
                            testCase.uri,
                            GetDefaultSchemaLocator(testCase.flag));
                    }
                }

                GLTFSDK_TEST_METHOD(JsonSchemaTests, FlagsPreserveSparseAlias)
                {
                    Assert::IsTrue(
                        static_cast<std::uint64_t>(
                            SchemaFlags::DisableSchemaAccessorSparse) ==
                        static_cast<std::uint64_t>(
                            SchemaFlags::DisableSchemaAccessorSparseValues));

                    const auto locator = GetDefaultSchemaLocator(
                        SchemaFlags::DisableSchemaAccessorSparse);
                    Assert::AreEqual(
                        "{}",
                        locator->GetSchemaContent(
                            SCHEMA_URI_ACCESSORSPARSE));
                    Assert::AreEqual(
                        "{}",
                        locator->GetSchemaContent(
                            SCHEMA_URI_ACCESSORSPARSEVALUES));
                }

                GLTFSDK_TEST_METHOD(JsonSchemaTests, FlagsDisableRootKeepsStrictChecks)
                {
                    Assert::ExpectException<GLTFException>([]()
                    {
                        Deserialize(
                            R"({"asset":{"version":"2.0"},"asset":{"version":"2.0"}})",
                            DeserializeFlags::None,
                            SchemaFlags::DisableSchemaRoot);
                    });

                    Assert::ExpectException<GLTFException>([]()
                    {
                        std::string invalidUtf8 =
                            R"({"asset":{"version":"2.0","generator":")";
                        invalidUtf8.push_back(
                            static_cast<char>(0xC3));
                        invalidUtf8.push_back(
                            static_cast<char>(0x28));
                        invalidUtf8 += R"("}})";
                        Deserialize(
                            invalidUtf8,
                            DeserializeFlags::None,
                            SchemaFlags::DisableSchemaRoot);
                    });

                    Assert::ExpectException<InvalidGLTFException>([]()
                    {
                        Deserialize(
                            R"({"asset":{"version":"2.0"},"nodes":[{"matrix":[1]}]})",
                            DeserializeFlags::None,
                            SchemaFlags::DisableSchemaRoot);
                    });

                    const auto document = Deserialize(
                        R"({"asset":{"version":"2.0.0"}})",
                        DeserializeFlags::None,
                        SchemaFlags::DisableSchemaRoot);
                    Assert::IsTrue(document.asset.version == "2.0.0");
                }

                GLTFSDK_TEST_METHOD(JsonSchemaTests, DiagnosticsEscapesPointerTokens)
                {
                    auto state = SingleSchema(R"({
                        "type":"object",
                        "properties":{
                            "a/b~c":{"type":"integer","minimum":2}
                        }
                    })");

                    Assert::ExpectException<ValidationException>([&state]()
                    {
                        try
                        {
                            ValidateDocumentAgainstSchema(
                                R"({"a/b~c":1})",
                                "root.json",
                                Locator(state));
                        }
                        catch (const ValidationException& exception)
                        {
                            Assert::AreEqual(
                                "Schema violation at #/a~1b~0c due to minimum",
                                exception.what());
                            throw;
                        }
                    });
                }

                GLTFSDK_TEST_METHOD(JsonSchemaTests, DiagnosticsSelectionIsDeterministic)
                {
                    const std::string schema = R"({
                        "type":"object",
                        "properties":{
                            "outer":{
                                "type":"object",
                                "properties":{
                                    "value":{
                                        "type":"integer",
                                        "minimum":2
                                    }
                                }
                            }
                        }
                    })";

                    for (int iteration = 0; iteration < 5; ++iteration)
                    {
                        auto state = SingleSchema(schema);
                        Assert::ExpectException<ValidationException>([&state]()
                        {
                            try
                            {
                                ValidateDocumentAgainstSchema(
                                    R"({"outer":{"value":1}})",
                                    "root.json",
                                    Locator(state));
                            }
                            catch (const ValidationException& exception)
                            {
                                Assert::AreEqual(
                                    "Schema violation at #/outer/value due to minimum",
                                    exception.what());
                                throw;
                            }
                        });
                    }
                }

                GLTFSDK_TEST_METHOD(JsonSchemaTests, SessionUsesOriginalOrderedDom)
                {
                    auto state = SingleSchema(R"({
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

                    auto document = Internal::CreateJsonObject();
                    Internal::SetJsonMember(
                        document,
                        "value",
                        Internal::CreateJsonUInt32(4294967295U));

                    Internal::ValidateJsonAgainstSchema(
                        document,
                        "root.json",
                        Locator(state));
                    Assert::IsTrue(state->requests["root.json"] == 1U);
                }

                GLTFSDK_TEST_METHOD(JsonSchemaTests, SessionCachesAndCopiesLocatorContent)
                {
                    auto state = std::make_shared<LocatorState>();
                    state->reuseBuffer = true;
                    state->documents.emplace(
                        "schemas/root.json",
                        R"({
                            "$schema": "http://json-schema.org/draft-04/schema",
                            "type": "object",
                            "properties": {
                                "first": {
                                    "$ref": "defs.json#/definitions/value"
                                },
                                "second": {
                                    "$ref": "defs.json#/definitions/value"
                                }
                            }
                        })");
                    state->documents.emplace(
                        "schemas/defs.json",
                        R"({
                            "definitions": {
                                "value": {"type": "integer"}
                            }
                        })");

                    ValidateDocumentAgainstSchema(
                        R"({"first":1,"second":2})",
                        "schemas/root.json",
                        Locator(state));

                    Assert::IsTrue(
                        state->requests["schemas/root.json"] == 1U);
                    Assert::IsTrue(
                        state->requests["schemas/defs.json"] == 1U);
                }

                GLTFSDK_TEST_METHOD(JsonSchemaTests, SessionKeepsLocatorsIsolated)
                {
                    auto integerState = std::make_shared<LocatorState>();
                    integerState->documents.emplace(
                        "root.json", R"({"$ref":"value.json"})");
                    integerState->documents.emplace(
                        "value.json", R"({"type":"integer"})");

                    auto stringState = std::make_shared<LocatorState>();
                    stringState->documents.emplace(
                        "root.json", R"({"$ref":"value.json"})");
                    stringState->documents.emplace(
                        "value.json", R"({"type":"string"})");

                    ValidateDocumentAgainstSchema(
                        "1", "root.json", Locator(integerState));
                    ValidateDocumentAgainstSchema(
                        R"("one")", "root.json", Locator(stringState));

                    Assert::ExpectException<ValidationException>(
                        [&integerState]()
                        {
                            ValidateDocumentAgainstSchema(
                                R"("one")",
                                "root.json",
                                Locator(integerState));
                        });
                    Assert::ExpectException<ValidationException>(
                        [&stringState]()
                        {
                            ValidateDocumentAgainstSchema(
                                "1",
                                "root.json",
                                Locator(stringState));
                        });
                }

                GLTFSDK_TEST_METHOD(JsonSchemaTests, SessionForcesDraft04)
                {
                    auto state = SingleSchema("true");
                    ExpectGltfFailureContaining(
                        "Schema document at root.json is invalid",
                        [&state]()
                        {
                            ValidateDocumentAgainstSchema(
                                "1",
                                "root.json",
                                Locator(state));
                        });
                }

                GLTFSDK_TEST_METHOD(JsonSchemaTests, ResolverNormalizesAndStripsFragments)
                {
                    auto state = std::make_shared<LocatorState>();
                    state->documents.emplace(
                        "schemas/root.json",
                        R"({
                            "type": "object",
                            "properties": {
                                "child": {"$ref":"nested/child.json"}
                            }
                        })");
                    state->documents.emplace(
                        "schemas/nested/child.json",
                        R"({
                            "type": "object",
                            "properties": {
                                "value": {
                                    "$ref": "../defs.json#/definitions/value"
                                }
                            }
                        })");
                    state->documents.emplace(
                        "schemas/defs.json",
                        R"({
                            "definitions": {
                                "value": {
                                    "type": "integer",
                                    "minimum": 2
                                }
                            }
                        })");

                    ValidateDocumentAgainstSchema(
                        R"({"child":{"value":2}})",
                        "schemas/root.json",
                        Locator(state));

                    Assert::IsTrue(
                        state->requests["schemas/nested/child.json"] == 1U);
                    Assert::IsTrue(
                        state->requests["schemas/defs.json"] == 1U);
                    for (const auto& request : state->requests)
                    {
                        Assert::IsTrue(
                            request.first.find('#') == std::string::npos);
                    }
                }

                GLTFSDK_TEST_METHOD(JsonSchemaTests, ResolverSupportsLegalRecursion)
                {
                    auto state = std::make_shared<LocatorState>();
                    state->documents.emplace(
                        "root.json",
                        R"({
                            "type":"object",
                            "properties":{"node":{"$ref":"node.json"}}
                        })");
                    state->documents.emplace(
                        "node.json",
                        R"({
                            "id":"node.json",
                            "type":"object",
                            "required":["value"],
                            "properties":{
                                "value":{"type":"integer"},
                                "next":{"$ref":"node.json"}
                            }
                        })");

                    ValidateDocumentAgainstSchema(
                        R"({"node":{"value":1,"next":{"value":2}}})",
                        "root.json",
                        Locator(state));
                    Assert::IsTrue(state->requests["node.json"] == 1U);
                }

                GLTFSDK_TEST_METHOD(JsonSchemaTests, ResolverSupportsRootFragment)
                {
                    auto state = std::make_shared<LocatorState>();
                    state->documents.emplace(
                        "root.json",
                        R"({
                            "definitions": {
                                "value": {"type":"integer"}
                            }
                        })");

                    ValidateDocumentAgainstSchema(
                        "1",
                        "root.json#/definitions/value",
                        Locator(state));
                    Assert::ExpectException<ValidationException>([&state]()
                    {
                        ValidateDocumentAgainstSchema(
                            R"("one")",
                            "root.json#/definitions/value",
                            Locator(state));
                    });
                    Assert::IsTrue(state->requests["root.json"] == 2U);
                    Assert::IsTrue(
                        state->requests.find(
                            "root.json#/definitions/value") ==
                        state->requests.end());
                }

                GLTFSDK_TEST_METHOD(JsonSchemaTests, ResolverRejectsReferenceOnlyCycle)
                {
                    auto state = std::make_shared<LocatorState>();
                    state->documents.emplace(
                        "root.json",
                        R"({"properties":{"value":{"$ref":"a.json"}}})");
                    state->documents.emplace(
                        "a.json", R"({"$ref":"b.json"})");
                    state->documents.emplace(
                        "b.json", R"({"$ref":"a.json"})");

                    ExpectGltfFailureContaining(
                        "Schema document at root.json is invalid",
                        [&state]()
                        {
                            ValidateDocumentAgainstSchema(
                                R"({"value":1})",
                                "root.json",
                                Locator(state));
                        });
                }

                GLTFSDK_TEST_METHOD(JsonSchemaTests, ErrorsRejectNullLocator)
                {
                    Assert::ExpectException<GLTFException>([]()
                    {
                        ValidateDocumentAgainstSchema(
                            "{}",
                            "root.json",
                            nullptr);
                    });
                }

                GLTFSDK_TEST_METHOD(JsonSchemaTests, ErrorsDistinguishMissingAndThrowingLocator)
                {
                    auto missing = std::make_shared<LocatorState>();
                    ExpectGltfFailureContaining(
                        "could not be located",
                        [&missing]()
                        {
                            ValidateDocumentAgainstSchema(
                                "{}",
                                "missing.json",
                                Locator(missing));
                        });

                    auto throwing = std::make_shared<LocatorState>();
                    throwing->throwUri = "root.json";
                    ExpectGltfFailureContaining(
                        "Schema locator failed for root.json",
                        [&throwing]()
                        {
                            ValidateDocumentAgainstSchema(
                                "{}",
                                "root.json",
                                Locator(throwing));
                        });
                }

                GLTFSDK_TEST_METHOD(JsonSchemaTests, ErrorsDistinguishMalformedJson)
                {
                    auto validSchema = SingleSchema("{}");
                    ExpectGltfFailureContaining(
                        "bad JSON formatting",
                        [&validSchema]()
                        {
                            ValidateDocumentAgainstSchema(
                                "{",
                                "root.json",
                                Locator(validSchema));
                        });

                    auto malformedSchema =
                        SingleSchema(R"({"type":"object",})");
                    ExpectGltfFailureContaining(
                        "Schema document at root.json is not valid JSON",
                        [&malformedSchema]()
                        {
                            ValidateDocumentAgainstSchema(
                                "{}",
                                "root.json",
                                Locator(malformedSchema));
                        });
                }

                GLTFSDK_TEST_METHOD(JsonSchemaTests, ErrorsDistinguishInvalidReference)
                {
                    auto state = std::make_shared<LocatorState>();
                    state->documents.emplace(
                        "root.json",
                        R"({"$ref":"defs.json#/definitions/missing"})");
                    state->documents.emplace(
                        "defs.json",
                        R"({"definitions":{}})");

                    ExpectGltfFailureContaining(
                        "Schema document at root.json is invalid",
                        [&state]()
                        {
                            ValidateDocumentAgainstSchema(
                                "{}",
                                "root.json",
                                Locator(state));
                        });
                }

                GLTFSDK_TEST_METHOD(JsonSchemaTests, ErrorsReportPointerAndKeyword)
                {
                    auto state = SingleSchema(R"({
                        "type":"object",
                        "properties":{
                            "value":{"type":"integer","minimum":2}
                        }
                    })");

                    Assert::ExpectException<ValidationException>([&state]()
                    {
                        try
                        {
                            ValidateDocumentAgainstSchema(
                                R"({"value":1})",
                                "root.json",
                                Locator(state));
                        }
                        catch (const ValidationException& exception)
                        {
                            Assert::AreEqual(
                                "Schema violation at #/value due to minimum",
                                exception.what());
                            throw;
                        }
                    });
                }

                GLTFSDK_TEST_METHOD(JsonSchemaTests, ErrorsMissingDependencyDoesNotCrash)
                {
                    auto state = SingleSchema(R"({
                        "type":"object",
                        "dependencies":{"scene":["scenes"]}
                    })");

                    Assert::ExpectException<ValidationException>([&state]()
                    {
                        try
                        {
                            ValidateDocumentAgainstSchema(
                                R"({"scene":0})",
                                "root.json",
                                Locator(state));
                        }
                        catch (const ValidationException& exception)
                        {
                            Assert::AreEqual(
                                "Schema violation at # due to dependencies",
                                exception.what());
                            throw;
                        }
                    });
                }
            };
        }
    }
}
