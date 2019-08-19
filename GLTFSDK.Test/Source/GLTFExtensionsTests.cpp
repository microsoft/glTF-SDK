// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/Extension.h>
#include <GLTFSDK/ExtensionHandlers.h>
#include <GLTFSDK/ExtensionsKHR.h>
#include <GLTFSDK/RapidJsonUtils.h>
#include <GLTFSDK/Serialize.h>
#include <GLTFSDK/SchemaValidation.h>

#include "TestResources.h"
#include "TestUtils.h"

#include <fstream>

using namespace glTF::UnitTest;

namespace
{
    using namespace Microsoft::glTF;

    //NOTE: while the TestExtension schema specifies extras and extensions properties they are not
    // serialized or deserialized and the TestExtension class does not inherit from glTFProperty.
    // The schema references the glTFProperty schema only to validate that schema dependencies work
    // correctly when using the ISchemaLocator interface with glTF extensions.

    constexpr const char TestExtensionName[] = "TestExtension";
    constexpr const char TestExtensionSchemaUri[] = "test.schema.json";
    constexpr const char TestExtensionSchema[] =
R"({
    "$schema": "http://json-schema.org/draft-04/schema",
    "type": "object",
    "allOf": [ { "$ref": "glTFProperty.schema.json" } ],
    "properties": {
        "flag": {
            "type": "boolean"
        },
        "extensions": { },
        "extras": { }
    },
    "additionalProperties": false,
    "required": [ "flag" ]
})";

    struct TestExtension : Extension
    {
        TestExtension(bool flag) : flag(flag) {}

        std::unique_ptr<Extension> Clone() const override
        {
            return std::make_unique<TestExtension>(*this);
        }

        bool IsEqual(const Extension& rhs) const override
        {
            bool isEqual = false;

            if (auto other = dynamic_cast<const TestExtension*>(&rhs))
            {
                isEqual = other->flag == flag;
            }

            return isEqual;
        }

        bool flag;
    };

    class TextExtensionSchemaLocator : public ISchemaLocator
    {
    public:
        TextExtensionSchemaLocator(std::unordered_map<std::string, std::string> schemaUriMap) : schemaUriMap(std::move(schemaUriMap))
        {
        }

        const char* GetSchemaContent(const std::string& uri) const override
        {
            return schemaUriMap.at(uri).c_str();
        }

        static std::unique_ptr<const TextExtensionSchemaLocator> Create()
        {
            const auto& defaultSchemaUriMap = GetDefaultSchemaUriMap();

            // Insert schema(s) for TextExtension and its schema dependencies (i.e. glTFProperty.schema.json, extension.schema.json and extras.schema.json)
            std::unordered_map<std::string, std::string> schemaUriMap = {
                { TestExtensionSchemaUri, TestExtensionSchema },
                *defaultSchemaUriMap.find(SCHEMA_URI_GLTFPROPERTY),
                *defaultSchemaUriMap.find(SCHEMA_URI_EXTENSION),
                *defaultSchemaUriMap.find(SCHEMA_URI_EXTRAS)
            };

            return std::make_unique<TextExtensionSchemaLocator>(std::move(schemaUriMap));
        }

    private:
        std::unordered_map<std::string, std::string> schemaUriMap;
    };

    std::string SerializeTestExtension(const TestExtension& extension)
    {
        rapidjson::Document doc;

        doc.SetObject();
        doc.AddMember("flag", extension.flag, doc.GetAllocator());

        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

        doc.Accept(writer);

        return sb.GetString();
    }

    std::unique_ptr<Extension> DeserializeTestExtension(const std::string& json, bool isValidationRequired)
    {
        rapidjson::Document documentExtension = RapidJsonUtils::CreateDocumentFromString(json);

        if (isValidationRequired)
        {
            ValidateDocumentAgainstSchema(documentExtension, TestExtensionSchemaUri, TextExtensionSchemaLocator::Create());
        }

        return std::make_unique<TestExtension>(documentExtension["flag"].GetBool());
    }

    constexpr const char expectedExtensionAddHandler[] =
R"({
    "asset": {
        "version": "2.0"
    },
    "nodes": [
        {
            "extensions": {
                "TestExtension": {
                    "flag": true
                }
            }
        }
    ],
    "scenes": [
        {
            "nodes": [
                0
            ],
            "extensions": {
                "TestExtension": {
                    "flag": true
                }
            }
        }
    ],
    "scene": 0,
    "extensions": {
        "TestExtension": {
            "flag": false
        }
    },
    "extensionsUsed": [
        "TestExtension"
    ]
})";

    constexpr const char extensionSchemaValid[] =
R"({
    "asset": {
        "version": "2.0"
    },
    "nodes": [
        {
            "extensions": {
                "TestExtension": {
                    "flag": true
                }
            }
        }
    ],
    "extensionsUsed": [
        "TestExtension"
    ]
})";

    constexpr const char extensionSchemaInvalidNoFlag[] =
R"({
    "asset": {
        "version": "2.0"
    },
    "nodes": [
        {
            "extensions": {
                "TestExtension": {
                }
            }
        }
    ],
    "extensionsUsed": [
        "TestExtension"
    ]
})";

    constexpr const char extensionSchemaInvalidUnknownProperty[] =
R"({
    "asset": {
        "version": "2.0"
    },
    "nodes": [
        {
            "extensions": {
                "TestExtension": {
                    "flag": true,
                    "flagOther": true
                }
            }
        }
    ],
    "extensionsUsed": [
        "TestExtension"
    ]
})";

    constexpr const char extensionSchemaKHRTextureTransform_TexCoord[] =
R"({
    "asset": {
        "version": "2.0"
    },
  "materials": [
    {
      "name": "HasTexCoord",
      "pbrMetallicRoughness": {
        "baseColorTexture": {
          "index": 0,
          "extensions": {
            "KHR_texture_transform": {
              "offset": [
                -0.2,
                -0.1
              ],
              "rotation": 0.3,
              "scale": [
                1.5,
                1.5
              ],
              "texCoord": 1234
            }
          }
        },
        "metallicFactor": 0
      }
    },
    {
      "name": "DoesNotHaveTexCoord",
      "pbrMetallicRoughness": {
        "baseColorTexture": {
          "index": 0,
          "extensions": {
            "KHR_texture_transform": {
              "offset": [
                -0.2,
                -0.1
              ],
              "rotation": 0.3,
              "scale": [
                1.5,
                1.5
              ]
            }
          }
        },
        "metallicFactor": 0
      }
    }
  ],
  "extensionsUsed": [
    "KHR_texture_transform"
  ],
  "textures": [
    {
      "source": 0
    }
  ],
  "images": [
    {
      "uri": "placeholder.png"
    }
  ]
})";
}

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(ExtensionsTests)
            {
                GLTFSDK_TEST_METHOD(ExtensionsTests, Extensions_Test_RoundTrip_And_Equality)
                {
                    const auto inputJson = ReadLocalJson(c_cubeJson);

                    const auto extensionDeserializer = KHR::GetKHRExtensionDeserializer();
                    const auto extensionSerializer = KHR::GetKHRExtensionSerializer();

                    auto doc = Deserialize(inputJson, extensionDeserializer);

                    // Serialize Document back to json
                    auto outputJson = Serialize(doc, extensionSerializer);
                    auto outputDoc = Deserialize(outputJson, extensionDeserializer);

                    // Compare input and output Documents
                    Assert::IsTrue(doc == outputDoc, L"Input gltf and output gltf are not equal");
                }

                GLTFSDK_TEST_METHOD(ExtensionsTests, Extensions_Test_RoundTrip_And_Equality_Draco)
                {
                    const auto inputJson = ReadLocalJson(c_dracoBox);

                    const auto extensionDeserializer = KHR::GetKHRExtensionDeserializer();
                    const auto extensionSerializer = KHR::GetKHRExtensionSerializer();

                    auto doc = Deserialize(inputJson, extensionDeserializer);

                    Assert::AreEqual(doc.meshes.Size(), size_t(1));
                    Assert::AreEqual(doc.meshes[0].primitives.size(), size_t(1));
                    Assert::AreEqual(doc.meshes[0].primitives[0].GetExtensions().size(), size_t(1));

                    auto draco = doc.meshes[0].primitives[0].GetExtension<KHR::MeshPrimitives::DracoMeshCompression>();

                    Assert::AreEqual<std::string>(draco.bufferViewId, "0");
                    Assert::AreEqual<size_t>(draco.attributes.size(), 2);
                    Assert::AreEqual<size_t>(draco.attributes[ACCESSOR_POSITION], 1);
                    Assert::AreEqual<size_t>(draco.attributes[ACCESSOR_NORMAL], 0);

                    // Serialize GLTFDocument back to json
                    auto outputJson = Serialize(doc, extensionSerializer);
                    auto outputDoc = Deserialize(outputJson, extensionDeserializer);

                    // Compare input and output GLTFDocuments
                    Assert::IsTrue(doc == outputDoc, L"Input gltf and output gltf are not equal");
                }

                GLTFSDK_TEST_METHOD(ExtensionsTests, Extensions_Test_GetExtension)
                {
                    const auto inputJson = ReadLocalJson(c_cubeJson);

                    const auto extensionDeserializer = KHR::GetKHRExtensionDeserializer();
                    auto doc = Deserialize(inputJson, extensionDeserializer);

                    Assert::AreEqual(doc.materials.Size(), size_t(3));
                    Assert::AreEqual(doc.materials[0].extensions.size(), size_t(0));
                    Assert::AreEqual(doc.materials[0].GetExtensions().size(), size_t(1));

                    auto specGloss = doc.materials[0].GetExtension<KHR::Materials::PBRSpecularGlossiness>();

                    Assert::IsTrue(specGloss.specularFactor == Color3(.0f, .0f, .0f));
                    Assert::IsTrue(specGloss.diffuseFactor == Color4(.49803921580314639f, .49803921580314639f, .49803921580314639f, 1.0f));
                }

                GLTFSDK_TEST_METHOD(ExtensionsTests, Extensions_Test_RemoveExtension)
                {
                    const auto inputJson = ReadLocalJson(c_cubeJson);

                    const auto extensionDeserializer = KHR::GetKHRExtensionDeserializer();
                    auto doc = Deserialize(inputJson, extensionDeserializer);

                    Assert::AreEqual(doc.materials.Size(), size_t(3));
                    Assert::AreEqual(doc.materials[0].extensions.size(), size_t(0));
                    Assert::AreEqual(doc.materials[0].GetExtensions().size(), size_t(1));
                    Material mat = doc.materials[0];
                    Assert::AreEqual(mat.GetExtensions().size(), size_t(1));

                    mat.RemoveExtension<KHR::Materials::PBRSpecularGlossiness>();
                    doc.materials.Replace(mat);
                    Assert::AreEqual(doc.materials[0].GetExtensions().size(), size_t(0));
                }

                GLTFSDK_TEST_METHOD(ExtensionsTests, Extensions_Test_HasExtension)
                {
                    struct NonExistentExtension : Extension
                    {
                        std::unique_ptr<Extension> Clone() const override
                        {
                            throw GLTFException("Not implemented");
                        }

                        bool IsEqual(const Extension&) const override
                        {
                            throw GLTFException("Not implemented");
                        }
                    };

                    const auto inputJson = ReadLocalJson(c_cubeJson);

                    const auto extensionDeserializer = KHR::GetKHRExtensionDeserializer();
                    auto doc = Deserialize(inputJson, extensionDeserializer);

                    Assert::AreEqual(doc.materials.Size(), size_t(3));
                    Assert::AreEqual(doc.materials[0].extensions.size(), size_t(0));
                    Assert::AreEqual(doc.materials[0].GetExtensions().size(), size_t(1));

                    Assert::IsTrue(doc.materials[0].HasExtension<KHR::Materials::PBRSpecularGlossiness>());
                    Assert::IsFalse(doc.materials[0].HasExtension<NonExistentExtension>());
                }

                GLTFSDK_TEST_METHOD(ExtensionsTests, Extensions_Test_HasSpecGlossExtension)
                {
                    const auto inputJson = ReadLocalJson(c_singleTriangleWithTextureJson);

                    const auto extensionDeserializer = KHR::GetKHRExtensionDeserializer();
                    auto doc = Deserialize(inputJson, extensionDeserializer);

                    Assert::IsTrue(doc.materials[0].HasExtension<KHR::Materials::PBRSpecularGlossiness>());

                    auto& specGloss = doc.materials[0].GetExtension<KHR::Materials::PBRSpecularGlossiness>();

                    Assert::AreEqual(specGloss.diffuseTexture.textureId.c_str(), "0");
                    Assert::IsTrue(specGloss.specularFactor == Color3(.0f, .0f, .0f));
                }

                GLTFSDK_TEST_METHOD(ExtensionsTests, Extensions_Test_HasTextureTransformExtension)
                {
                    const auto inputJson = ReadLocalJson(c_textureTransformTestJson);

                    const auto extensionDeserializer = KHR::GetKHRExtensionDeserializer();
                    auto doc = Deserialize(inputJson, extensionDeserializer);

                    auto checkTextureInfo = [](
                        const Material& material, 
                        const Vector2& offset, float rotation, const Vector2& scale, Optional<size_t> texCoord = {})
                    {
                        auto& textureInfo = material.metallicRoughness.baseColorTexture;

                        Assert::IsTrue(textureInfo.HasExtension<KHR::TextureInfos::TextureTransform>());

                        auto& textureTransform = textureInfo.GetExtension<KHR::TextureInfos::TextureTransform>();

                        KHR::TextureInfos::TextureTransform expectedTextureTransform;
                        expectedTextureTransform.offset = offset;
                        expectedTextureTransform.scale = scale;
                        expectedTextureTransform.rotation = rotation;
                        expectedTextureTransform.texCoord = texCoord;

                        Assert::IsTrue(textureTransform == expectedTextureTransform);
                    };

                    Assert::IsTrue(doc.materials.Size() == 9);

                    checkTextureInfo(doc.materials[0], Vector2(0.5f, 0.0f), 0.0f, Vector2(1.0f, 1.0f)); // Note: texCoord not specified
                    checkTextureInfo(doc.materials[1], Vector2(0.0f, 0.5f), 0.0f, Vector2(1.0f, 1.0f));
                    checkTextureInfo(doc.materials[2], Vector2(0.5f, 0.5f), 0.0f, Vector2(1.0f, 1.0f));
                    checkTextureInfo(doc.materials[3], Vector2(0.0f, 0.0f), 0.39269908169872415480783042290994f, Vector2(1.0f, 1.0f));
                    checkTextureInfo(doc.materials[4], Vector2(0.0f, 0.0f), 0.0f, Vector2(1.5f, 1.5f));
                    checkTextureInfo(doc.materials[5], Vector2(-0.2f, -0.1f), 0.3f, Vector2(1.5f, 1.5f));
                }

                GLTFSDK_TEST_METHOD(ExtensionsTests, Extensions_Test_HasTextureTransformExtension_Normal)
                {
                  const auto inputJson = ReadLocalJson(c_textureTransformTestJson);

                  const auto extensionDeserializer = KHR::GetKHRExtensionDeserializer();
                  auto doc = Deserialize(inputJson, extensionDeserializer);

                  auto checkTextureInfo = [](
                                             const Material& material,
                                             const Vector2& offset, float rotation, const Vector2& scale, Optional<size_t> texCoord = {})
                  {
                    auto& textureInfo = material.normalTexture;

                    Assert::IsTrue(textureInfo.HasExtension<KHR::TextureInfos::TextureTransform>());

                    auto& textureTransform = textureInfo.GetExtension<KHR::TextureInfos::TextureTransform>();

                    KHR::TextureInfos::TextureTransform expectedTextureTransform;
                    expectedTextureTransform.offset = offset;
                    expectedTextureTransform.scale = scale;
                    expectedTextureTransform.rotation = rotation;
                    expectedTextureTransform.texCoord = texCoord;

                    Assert::IsTrue(textureTransform == expectedTextureTransform);
                  };

                  Assert::IsTrue(doc.materials.Size() == 9);

                  checkTextureInfo(doc.materials[0], Vector2(0.5f, 0.0f), 0.0f, Vector2(1.0f, 1.0f));
                }

                GLTFSDK_TEST_METHOD(ExtensionsTests, Extensions_Test_HasTextureTransformExtension_Occlusion)
                {
                  const auto inputJson = ReadLocalJson(c_textureTransformTestJson);

                  const auto extensionDeserializer = KHR::GetKHRExtensionDeserializer();
                  auto doc = Deserialize(inputJson, extensionDeserializer);

                  auto checkTextureInfo = [](
                                             const Material& material,
                                             const Vector2& offset, float rotation, const Vector2& scale, Optional<size_t> texCoord = {})
                  {
                    auto& textureInfo = material.occlusionTexture;

                    Assert::IsTrue(textureInfo.HasExtension<KHR::TextureInfos::TextureTransform>());

                    auto& textureTransform = textureInfo.GetExtension<KHR::TextureInfos::TextureTransform>();

                    KHR::TextureInfos::TextureTransform expectedTextureTransform;
                    expectedTextureTransform.offset = offset;
                    expectedTextureTransform.scale = scale;
                    expectedTextureTransform.rotation = rotation;
                    expectedTextureTransform.texCoord = texCoord;

                    Assert::IsTrue(textureTransform == expectedTextureTransform);
                  };

                  Assert::IsTrue(doc.materials.Size() == 9);

                  checkTextureInfo(doc.materials[0], Vector2(0.5f, 0.0f), 0.0f, Vector2(1.0f, 1.0f));
                }

                GLTFSDK_TEST_METHOD(ExtensionsTests, Extensions_Test_HasTextureTransformExtension_TexCoord)
                {
                    // Ensure the optionality of the texCoord property is preserved
                    const auto extensionDeserializer = KHR::GetKHRExtensionDeserializer();
                    auto doc = Deserialize(extensionSchemaKHRTextureTransform_TexCoord, extensionDeserializer);

                    auto checkTextureInfo = [](
                        const Material& material,
                        const Vector2& offset, float rotation, const Vector2& scale, Optional<size_t> texCoord = {})
                    {
                        auto& textureInfo = material.metallicRoughness.baseColorTexture;

                        Assert::IsTrue(textureInfo.HasExtension<KHR::TextureInfos::TextureTransform>());

                        auto& textureTransform = textureInfo.GetExtension<KHR::TextureInfos::TextureTransform>();

                        KHR::TextureInfos::TextureTransform expectedTextureTransform;
                        expectedTextureTransform.offset = offset;
                        expectedTextureTransform.scale = scale;
                        expectedTextureTransform.rotation = rotation;
                        expectedTextureTransform.texCoord = texCoord;

                        Assert::IsTrue(textureTransform == expectedTextureTransform);
                    };

                    Assert::IsTrue(doc.materials.Size() == 2);

                    checkTextureInfo(doc.materials[0], Vector2(-0.2f, -0.1f), 0.3f, Vector2(1.5f, 1.5f), 1234);
                    checkTextureInfo(doc.materials[1], Vector2(-0.2f, -0.1f), 0.3f, Vector2(1.5f, 1.5f));

                    const auto extensionSerializer = KHR::GetKHRExtensionSerializer();
                    auto tt = Serialize(doc, extensionSerializer);

                    auto roundTrippedDoc = Deserialize(tt, extensionDeserializer);
                    Assert::IsTrue(doc == roundTrippedDoc, L"Input gltf and output gltf are not equal");
                }

                GLTFSDK_TEST_METHOD(ExtensionsTests, Extensions_Test_RoundTrip_And_Equality_TextureTransform)
                {
                    const auto inputJson = ReadLocalJson(c_textureTransformTestJson);

                    const auto extensionDeserializer = KHR::GetKHRExtensionDeserializer();
                    const auto extensionSerializer = KHR::GetKHRExtensionSerializer();

                    auto doc = Deserialize(inputJson, extensionDeserializer);

                    // Serialize GLTFDocument back to json
                    auto outputJson = Serialize(doc, extensionSerializer);
                    auto outputDoc = Deserialize(outputJson, extensionDeserializer);

                    // Compare input and output GLTFDocuments
                    Assert::IsTrue(doc == outputDoc, L"Input gltf and output gltf are not equal");
                }

                GLTFSDK_TEST_METHOD(ExtensionsTests, Extensions_Test_RoundTrip_And_Equality_TextureTransform_SGOnly)
                {
                    // Test document has TextureInfo extensions embedded in SpecGloss extension
                    const auto inputJson = ReadLocalJson(c_textureTransformTestSGOnlyJson);

                    const auto extensionDeserializer = KHR::GetKHRExtensionDeserializer();
                    const auto extensionSerializer = KHR::GetKHRExtensionSerializer();

                    auto doc = Deserialize(inputJson, extensionDeserializer);

                    // Serialize GLTFDocument back to json
                    auto outputJson = Serialize(doc, extensionSerializer);
                    auto outputDoc = Deserialize(outputJson, extensionDeserializer);

                    // Compare input and output GLTFDocuments
                    Assert::IsTrue(doc == outputDoc, L"Input gltf and output gltf are not equal");
                }

                GLTFSDK_TEST_METHOD(ExtensionsTests, ExtensionSerializerAddHandler)
                {
                    Node node;

                    node.id = "0";
                    node.SetExtension<TestExtension>(true);

                    Scene scene;

                    scene.nodes.push_back(node.id);
                    scene.SetExtension<TestExtension>(true);

                    Document document;

                    document.nodes.Append(std::move(node));
                    document.SetDefaultScene(std::move(scene), AppendIdPolicy::GenerateOnEmpty);
                    document.SetExtension<TestExtension>(false);
                    document.extensionsUsed.emplace(TestExtensionName);

                    ExtensionSerializer extensionSerializer;

                    size_t handlerCountDocument = 0;
                    size_t handlerCountScene = 0;
                    size_t handlerCountAll = 0;

                    extensionSerializer.AddHandler<TestExtension, Document>(TestExtensionName,
                        [&handlerCountDocument](const TestExtension& extension, const Document&, const ExtensionSerializer& /*extensionSerializer*/)
                    {
                        ++handlerCountDocument;
                        return SerializeTestExtension(extension);
                    });

                    extensionSerializer.AddHandler<TestExtension, Scene>(TestExtensionName,
                        [&handlerCountScene](const TestExtension& extension, const Document&, const ExtensionSerializer& /*extensionSerializer*/)
                    {
                        ++handlerCountScene;
                        return SerializeTestExtension(extension);
                    });

                    // The 'all properties' handler will process the Node's extension
                    extensionSerializer.AddHandler<TestExtension>(TestExtensionName,
                        [&handlerCountAll](const TestExtension& extension, const Document&, const ExtensionSerializer& /*extensionSerializer*/)
                    {
                        ++handlerCountAll;
                        return SerializeTestExtension(extension);
                    });

                    Assert::IsTrue(extensionSerializer.HasHandler<TestExtension, Document>());
                    Assert::IsTrue(extensionSerializer.HasHandler<TestExtension, Scene>());
                    Assert::IsTrue(extensionSerializer.HasHandler<TestExtension>());

                    const auto actual = Serialize(document, extensionSerializer, SerializeFlags::Pretty);

                    Assert::AreEqual(size_t(1), handlerCountDocument, L"Document extension serializer handler called an unexpected number of times");
                    Assert::AreEqual(size_t(1), handlerCountScene, L"Scene extension serializer handler called an unexpected number of times");
                    Assert::AreEqual(size_t(1), handlerCountAll, L"Generic extension serializer handler called an unexpected number of times");

                    Assert::AreEqual(expectedExtensionAddHandler, actual.c_str(), L"Document and Scene extension serialization did not produce the expected output");
                }

                GLTFSDK_TEST_METHOD(ExtensionsTests, ExtensionDeserializerAddHandler)
                {
                    ExtensionDeserializer extensionDeserializer;

                    size_t handlerCountDocument = 0;
                    size_t handlerCountScene = 0;
                    size_t handlerCountAll = 0;

                    extensionDeserializer.AddHandler<TestExtension, Document>(TestExtensionName,
                        [&handlerCountDocument](const std::string& json, const ExtensionDeserializer& /*extensionDeserializer*/)
                    {
                        ++handlerCountDocument;
                        return DeserializeTestExtension(json, false);
                    });

                    extensionDeserializer.AddHandler<TestExtension, Scene>(TestExtensionName,
                        [&handlerCountScene](const std::string& json, const ExtensionDeserializer& /*extensionDeserializer*/)
                    {
                        ++handlerCountScene;
                        return DeserializeTestExtension(json, false);
                    });

                    // The 'all properties' handler will process the Node's extension
                    extensionDeserializer.AddHandler<TestExtension>(TestExtensionName,
                        [&handlerCountAll](const std::string& json, const ExtensionDeserializer& /*extensionDeserializer*/)
                    {
                        ++handlerCountAll;
                        return DeserializeTestExtension(json, false);
                    });

                    Assert::IsTrue(extensionDeserializer.HasHandler<TestExtension, Document>());
                    Assert::IsTrue(extensionDeserializer.HasHandler<TestExtension, Scene>());
                    Assert::IsTrue(extensionDeserializer.HasHandler<TestExtension>());

                    const Document document = Deserialize(expectedExtensionAddHandler, extensionDeserializer);

                    Assert::AreEqual(size_t(1), handlerCountDocument, L"Document extension serializer handler called an unexpected number of times");
                    Assert::AreEqual(size_t(1), handlerCountScene, L"Scene extension serializer handler called an unexpected number of times");
                    Assert::AreEqual(size_t(1), handlerCountAll, L"Generic extension serializer handler called an unexpected number of times");

                    Assert::IsTrue(document.HasExtension<TestExtension>(), L"Document is missing TestExtension instance");
                    Assert::IsFalse(document.GetExtension<TestExtension>().flag, L"Document's TestExtension's flag property expected to be false");

                    const Scene& scene = document.GetDefaultScene();

                    Assert::IsTrue(scene.HasExtension<TestExtension>(), L"Scene is missing TestExtension instance");
                    Assert::IsTrue(scene.GetExtension<TestExtension>().flag, L"Scene's TestExtension's flag property expected to be true");

                    const Node& node = document.nodes.Get(scene.nodes.front());

                    Assert::IsTrue(node.HasExtension<TestExtension>(), L"Node is missing TestExtension instance");
                    Assert::IsTrue(node.GetExtension<TestExtension>().flag, L"Node's TestExtension's flag property expected to be true");
                }

                GLTFSDK_TEST_METHOD(ExtensionsTests, ExtensionDeserializerSchemaLocatorValid)
                {
                    ExtensionDeserializer extensionDeserializer;

                    extensionDeserializer.AddHandler<TestExtension>(TestExtensionName,
                        [](const std::string& json, const ExtensionDeserializer& /*extensionDeserializer*/)
                    {
                        return DeserializeTestExtension(json, true); // Enable schema validation
                    });

                    Assert::IsTrue(extensionDeserializer.HasHandler<TestExtension>());

                    const Document document = Deserialize(extensionSchemaValid, extensionDeserializer);

                    Assert::AreEqual(size_t(1), document.nodes.Size());
                    Assert::IsTrue(document.nodes.Front().HasExtension<TestExtension>());
                    Assert::IsTrue(document.nodes.Front().GetExtension<TestExtension>().flag);
                }

                GLTFSDK_TEST_METHOD(ExtensionsTests, ExtensionDeserializerSchemaLocatorInvalidNoFlag)
                {
                    ExtensionDeserializer extensionDeserializer;

                    extensionDeserializer.AddHandler<TestExtension>(TestExtensionName,
                        [](const std::string& json, const ExtensionDeserializer& /*extensionDeserializer*/)
                    {
                        return DeserializeTestExtension(json, true); // Enable schema validation
                    });

                    Assert::IsTrue(extensionDeserializer.HasHandler<TestExtension>());

                    // Check that the extensionSchemaInvalidNoFlag glTF manifest throws the expected ValidationException
                    Assert::ExpectException<ValidationException>([&extensionDeserializer]()
                    {
                        Deserialize(extensionSchemaInvalidNoFlag, extensionDeserializer);
                    });

                    // Check that the extensionSchemaInvalidUnknownProperty glTF manifest throws the expected ValidationException
                    Assert::ExpectException<ValidationException>([&extensionDeserializer]()
                    {
                        Deserialize(extensionSchemaInvalidUnknownProperty, extensionDeserializer);
                    });
                }
            };
        }
    }
}
