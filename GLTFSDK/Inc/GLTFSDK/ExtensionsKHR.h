// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <GLTFSDK/ExtensionHandlers.h>

#include <memory>
#include <string>

namespace Microsoft
{
    namespace glTF
    {
        namespace KHR
        {
            ExtensionSerializer   GetKHRExtensionSerializer();
            ExtensionDeserializer GetKHRExtensionDeserializer();

            namespace Materials
            {
                constexpr const char* PBRSPECULARGLOSSINESS_NAME = "KHR_materials_pbrSpecularGlossiness";

                // KHR_materials_pbrSpecularGlossiness
                struct PBRSpecularGlossiness : Extension, glTFProperty
                {
                    PBRSpecularGlossiness();

                    Color4 diffuseFactor;
                    TextureInfo diffuseTexture;
                    Color3 specularFactor;
                    float glossinessFactor;
                    TextureInfo specularGlossinessTexture;

                    std::unique_ptr<Extension> Clone() const override;
                    bool IsEqual(const Extension& rhs) const override;
                };

                std::string SerializePBRSpecGloss(const PBRSpecularGlossiness& specGloss, const Document& gltfDocument);
                std::unique_ptr<Extension> DeserializePBRSpecGloss(const std::string& json);

                constexpr const char* UNLIT_NAME = "KHR_materials_unlit";

                // KHR_materials_unlit
                struct Unlit : Extension, glTFProperty
                {
                    std::unique_ptr<Extension> Clone() const override;
                    bool IsEqual(const Extension& rhs) const override;
                };

                std::string SerializeUnlit(const Unlit& unlit, const Document& gltfDocument);
                std::unique_ptr<Extension> DeserializeUnlit(const std::string& json);
            }

            namespace MeshPrimitives
            {
                constexpr const char* DRACOMESHCOMPRESSION_NAME = "KHR_draco_mesh_compression";

                // KHR_draco_mesh_compression
                struct DracoMeshCompression : Extension, glTFProperty
                {
                    std::string bufferViewId;

                    std::unordered_map<std::string, uint32_t> attributes;

                    std::unique_ptr<Extension> Clone() const override;
                    bool IsEqual(const Extension& rhs) const override;
                };

                std::string SerializeDracoMeshCompression(const DracoMeshCompression& dracoMeshCompression, const Document& gltfDocument);
                std::unique_ptr<Extension> DeserializeDracoMeshCompression(const std::string& json);
            }
        }
    }
}
