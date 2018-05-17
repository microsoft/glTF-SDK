// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/ExtensionsKHR.h>
#include <GLTFSDK/Visitor.h>

using namespace glTF::UnitTest;

namespace
{
    static const char testVisitorJson[] = R"(
{
    "asset":
        {
            "version": "2.0"
        },
    "scenes": [
        {
            "nodes": [0, 1]
        }
    ],
    "nodes": [
        {
            "children": [2],
            "name": "parent_node0"
        },
        {
            "children": [3],
            "name": "parent_node1"
        },
        {
            "mesh": 0,
            "name": "child_node0"
        },
        {
            "mesh": 0,
            "name": "child_node1"
        }
    ],
    "meshes": [
        {
            "primitives": [
                {
                    "attributes":
                        {
                            "POSITION": 0
                        },
                    "mode": 4
                }
            ],
            "name": "test_mesh"
        }
    ]
}
)";

    static const char testVisitorDefaultActionSpecGlossJson[] = R"(
{
    "asset":
        {
            "version": "2.0"
        },
    "scenes": [
        {
            "nodes": [0]
        }
    ],
    "nodes": [
        {
            "mesh": 0
        }
    ],
    "meshes": [
        {
            "primitives": [
                {
                    "attributes":
                        {
                            "POSITION": 0
                        },
                    "material": 0
                }
            ],
            "name": "test_mesh"
        }
    ],
    "materials": [
        {
            "extensions":
                {
                    "KHR_materials_pbrSpecularGlossiness":
                        {
                            "diffuseTexture":
                                {
                                    "index": 0
                                },
                            "specularGlossinessTexture":
                                {
                                    "index": 1
                                }
                        }
                },
            "pbrMetallicRoughness":
                {
                    "baseColorTexture":
                        {
                            "index": 0
                        }
                }
        }
    ],
    "textures": [
        {
            "source": 0
        },
        {
            "source": 0
        }
    ],
    "images": [
        {
            "uri": "http://test"
        }
    ]
}
)";

    static const char testTraversalJson[] = R"(
{
    "asset":
        {
            "version": "2.0"
        },
    "scenes": [
        {
            "nodes": [0]
        }
    ],
    "nodes": [
        {
            "children": [1, 2],
            "name": "parent_node0"
        },
        {
            "children": [3, 4],
            "name": "parent_node1"
        },
        {
            "children": [5, 6],
            "name": "parent_node3"
        },
        {
            "mesh": 0,
            "name": "child_node0"
        },
        {
            "mesh": 0,
            "name": "child_node1"
        },
        {
            "mesh": 0,
            "name": "child_node2"
        },
        {
            "mesh": 0,
            "name": "child_node3"
        }
    ],
    "meshes": [
        {
            "primitives": [
                {
                    "attributes":
                        {
                            "POSITION": 0
                        },
                    "mode": 4
                }
            ],
            "name": "test_mesh"
        }
    ]
}
)";
}

namespace Microsoft
{
    namespace  glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(VisitorTests)
            {
                GLTFSDK_TEST_METHOD(VisitorTests, TestVisitor)
                {
                    Document gltfDoc = Deserialize(testVisitorJson);

                    size_t countNode = 0;
                    size_t countNodeRoot = 0;

                    size_t countMesh = 0;
                    size_t countMeshInstances = 0;

                    size_t countMeshPrimitive = 0;
                    size_t countMeshPrimitiveInstances = 0;

                    Visit(gltfDoc, DefaultSceneIndex,
                        [&countNode, &countNodeRoot](const Node&, const Node* nodeParent)
                    {
                        countNode++;
                        countNodeRoot += nodeParent ? 1 : 0;
                    },
                        [&countMesh, &countMeshInstances](const Mesh&, VisitState visitState)
                    {
                        countMesh += (visitState == VisitState::New) ? 1 : 0;
                        countMeshInstances++;
                    },
                        [&countMeshPrimitive, &countMeshPrimitiveInstances](const MeshPrimitive&, VisitState visitState)
                    {
                        countMeshPrimitive += (visitState == VisitState::New) ? 1 : 0;
                        countMeshPrimitiveInstances++;
                    });

                    Assert::AreEqual<size_t>(4UL, countNode);
                    Assert::AreEqual<size_t>(2UL, countNodeRoot);

                    Assert::AreEqual<size_t>(1UL, countMesh);
                    Assert::AreEqual<size_t>(2UL, countMeshInstances);

                    Assert::AreEqual<size_t>(1UL, countMeshPrimitive);
                    Assert::AreEqual<size_t>(2UL, countMeshPrimitiveInstances);
                }

                GLTFSDK_TEST_METHOD(VisitorTests, TestVisitorDefaultAction)
                {
                    Document gltfDoc = Deserialize(testVisitorJson);

                    size_t countNode = 0;
                    size_t countNodeRoot = 0;

                    size_t countMesh = 0;
                    size_t countMeshInstances = 0;

                    size_t countMeshPrimitive = 0;
                    size_t countMeshPrimitiveInstances = 0;

                    Visit(gltfDoc, DefaultSceneIndex,
                        [&countNode, &countNodeRoot](const Node&, const Node* nodeParent)
                    {
                        countNode++;
                        countNodeRoot += nodeParent ? 1 : 0;
                    },
                        [&countMesh, &countMeshInstances](const Mesh&, VisitState visitState, const VisitDefaultAction&)
                    {
                        countMesh += (visitState == VisitState::New) ? 1 : 0;
                        countMeshInstances++;
                    },
                        [&countMeshPrimitive, &countMeshPrimitiveInstances](const MeshPrimitive&, VisitState visitState, const VisitDefaultAction&)
                    {
                        countMeshPrimitive += (visitState == VisitState::New) ? 1 : 0;
                        countMeshPrimitiveInstances++;
                    });

                    Assert::AreEqual<size_t>(4UL, countNode);
                    Assert::AreEqual<size_t>(2UL, countNodeRoot);

                    Assert::AreEqual<size_t>(1UL, countMesh);
                    Assert::AreEqual<size_t>(2UL, countMeshInstances);

                    Assert::AreEqual<size_t>(1UL, countMeshPrimitive);
                    Assert::AreEqual<size_t>(2UL, countMeshPrimitiveInstances);
                }

                GLTFSDK_TEST_METHOD(VisitorTests, TestVisitorDefaultActionSpecGloss)
                {
                    Document gltfDoc = Deserialize(testVisitorDefaultActionSpecGlossJson, KHR::GetKHRExtensionDeserializer());

                    enum class TextureTypeExt : std::underlying_type_t<TextureType>
                    {
                        Diffuse = static_cast<std::underlying_type_t<TextureType>>(TextureType::Emissive) + 1,
                        SpecularGlossiness
                    };

                    TextureType textureTypeExpected = TextureType::BaseColor;

                    size_t countTexture = 0;
                    size_t countTextureInstances = 0;

                    size_t countImage = 0;
                    size_t countImageInstances = 0;

                    Visit(gltfDoc, DefaultSceneIndex,
                        [&gltfDoc, &textureTypeExpected](const Material& material, VisitState visitState, const VisitDefaultAction& visitDefaultAction)
                    {
                        if (visitState == VisitState::New)
                        {
                            if (material.HasExtension<KHR::Materials::PBRSpecularGlossiness>())
                            {
                                const auto& specGloss = material.GetExtension<KHR::Materials::PBRSpecularGlossiness>();

                                if (!specGloss.diffuseTexture.textureId.empty())
                                {
                                    textureTypeExpected = static_cast<TextureType>(TextureTypeExt::Diffuse);
                                    visitDefaultAction.Visit(gltfDoc.textures.Get(specGloss.diffuseTexture.textureId), textureTypeExpected);
                                }

                                if (!specGloss.specularGlossinessTexture.textureId.empty())
                                {
                                    textureTypeExpected = static_cast<TextureType>(TextureTypeExt::SpecularGlossiness);
                                    visitDefaultAction.Visit(gltfDoc.textures.Get(specGloss.specularGlossinessTexture.textureId), textureTypeExpected);
                                }
                            }

                            if (!material.metallicRoughness.baseColorTexture.textureId.empty())
                            {
                                textureTypeExpected = TextureType::BaseColor;
                            }
                        }
                    },
                        [&countTexture, &countTextureInstances, &textureTypeExpected](const Texture&, TextureType textureType, VisitState visitState)
                    {
                        countTexture += (visitState == VisitState::New) ? 1 : 0;
                        countTextureInstances++;

                        Assert::IsTrue(textureTypeExpected == textureType);
                    },
                        [&countImage, &countImageInstances](const Image&, VisitState visitState)
                    {
                        countImage += (visitState == VisitState::New) ? 1 : 0;
                        countImageInstances++;
                    });

                    Assert::AreEqual<size_t>(2UL, countTexture);
                    Assert::AreEqual<size_t>(3UL, countTextureInstances);

                    Assert::AreEqual<size_t>(1UL, countImage);
                    Assert::AreEqual<size_t>(3UL, countImageInstances);
                }

                GLTFSDK_TEST_METHOD(VisitorTests, TestVisitorTraversalDepthFirst)
                {
                    Document gltfDoc = Deserialize(testTraversalJson);

                    std::vector<std::string> ids;

                    Visit<DepthFirst>(gltfDoc, DefaultSceneIndex,
                        [&ids](const Node& node, const Node*)
                    {
                        ids.push_back(node.id);
                    });

                    std::string idsExpected[] = { "0", "1", "3", "4", "2", "5", "6" };

                    Assert::IsTrue(std::equal(std::begin(idsExpected), std::end(idsExpected), ids.begin()));
                }

                GLTFSDK_TEST_METHOD(VisitorTests, TestVisitorTraversalBreadthFirst)
                {
                    Document gltfDoc = Deserialize(testTraversalJson);

                    std::vector<std::string> ids;

                    Visit<BreadthFirst>(gltfDoc, DefaultSceneIndex,
                        [&ids](const Node& node, const Node*)
                    {
                        ids.push_back(node.id);
                    });

                    std::string idsExpected[] = { "0", "1", "2", "3", "4", "5", "6" };

                    Assert::IsTrue(std::equal(std::begin(idsExpected), std::end(idsExpected), ids.begin()));
                }

                GLTFSDK_TEST_METHOD(VisitorTests, TestVisitorFunctionPointer)
                {
                    static bool g_isVisited = false;

                    struct VisitorFunctions
                    {
                        static void MeshPrimitiveCallback(const MeshPrimitive&, VisitState)
                        {
                            g_isVisited = true;
                        }
                    };

                    Document gltfDoc = Deserialize(testVisitorJson);

                    Visit(gltfDoc, DefaultSceneIndex, &VisitorFunctions::MeshPrimitiveCallback);

                    Assert::IsTrue(g_isVisited);

                    // Reset back to false - just in case the test is run multiple times by the same process
                    g_isVisited = false;
                }
            };
        }
    }
}
