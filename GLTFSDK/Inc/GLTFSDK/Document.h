// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/IndexedContainer.h>

#include <unordered_set>

namespace Microsoft
{
    namespace glTF
    {
        class Document : public glTFProperty
        {
        public:
            Document();
            Document(Asset&& asset);

            bool operator==(const Document& rhs) const;

            bool IsExtensionUsed(const std::string& extension) const;
            bool IsExtensionRequired(const std::string& extension) const;

            bool HasDefaultScene() const;
            const Scene& GetDefaultScene() const;
            const Scene& SetDefaultScene(Scene&& scene, AppendIdPolicy policy = AppendIdPolicy::ThrowOnEmpty);

            Asset asset;

            IndexedContainer<const Accessor> accessors;
            IndexedContainer<const Animation> animations;
            IndexedContainer<const Buffer> buffers;
            IndexedContainer<const BufferView> bufferViews;
            IndexedContainer<const Camera> cameras;
            IndexedContainer<const Image> images;
            IndexedContainer<const Material> materials;
            IndexedContainer<const Mesh> meshes;
            IndexedContainer<const Node> nodes;
            IndexedContainer<const Sampler> samplers;
            IndexedContainer<const Scene> scenes;
            IndexedContainer<const Skin> skins;
            IndexedContainer<const Texture> textures;

            std::unordered_set<std::string> extensionsUsed;
            std::unordered_set<std::string> extensionsRequired;

            std::string defaultSceneId;
        };
    }
}
