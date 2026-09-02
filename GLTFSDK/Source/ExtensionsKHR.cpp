// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/ExtensionsKHR.h>

#include <GLTFSDK/Document.h>

#include "Internal/Json.h"

#include <algorithm>
#include <initializer_list>
#include <limits>
#include <string>
#include <utility>
#include <vector>

using namespace Microsoft::glTF;

namespace
{
    using JsonValue = Internal::JsonValue;

    float ReadFloat(
        const JsonValue& value,
        const std::string& error)
    {
        float result = 0.0F;
        if (!Internal::TryGetJsonFloat(value, result))
        {
            throw InvalidGLTFException(error);
        }
        return result;
    }

    std::uint32_t ReadUInt32(
        const JsonValue& value,
        const std::string& error)
    {
        std::uint32_t result = 0U;
        if (!Internal::TryGetJsonUInt32(value, result))
        {
            throw InvalidGLTFException(error);
        }
        return result;
    }

    std::size_t ReadSize(
        const JsonValue& value,
        const std::string& error)
    {
        std::size_t result = 0U;
        if (!Internal::TryGetJsonSize(value, result))
        {
            throw InvalidGLTFException(error);
        }
        return result;
    }

    float GetFloatMemberOrDefault(
        const JsonValue& object,
        const char* name,
        float defaultValue)
    {
        const auto* value =
            Internal::FindJsonMember(object, name);
        return value == nullptr
            ? defaultValue
            : ReadFloat(
                *value,
                std::string(name) + " must be a finite number");
    }

    std::vector<float> GetFixedSizeFloatArray(
        const JsonValue& value,
        std::size_t expectedSize,
        const char* error)
    {
        if (!Internal::IsJsonArray(value) ||
            Internal::GetJsonArraySize(value) != expectedSize)
        {
            throw InvalidGLTFException(error);
        }

        std::vector<float> result;
        result.reserve(expectedSize);
        for (std::size_t index = 0U;
             index < expectedSize;
             ++index)
        {
            result.push_back(ReadFloat(
                Internal::GetJsonArrayElement(
                    value, index, error),
                error));
        }
        return result;
    }

    JsonValue CreateFloatArray(
        std::initializer_list<float> values)
    {
        JsonValue array = Internal::CreateJsonArray();
        for (const float value : values)
        {
            Internal::AppendJsonValue(
                array,
                Internal::CreateJsonFloat(value));
        }
        return array;
    }

    JsonValue CreateFloatArray(const Color3& value)
    {
        return CreateFloatArray({value.r, value.g, value.b});
    }

    JsonValue CreateFloatArray(const Color4& value)
    {
        return CreateFloatArray(
            {value.r, value.g, value.b, value.a});
    }

    JsonValue CreateFloatArray(const Vector2& value)
    {
        return CreateFloatArray({value.x, value.y});
    }

    void ParseExtensions(
        const JsonValue& value,
        glTFProperty& property,
        const ExtensionDeserializer& extensionDeserializer)
    {
        const auto* extensions =
            Internal::FindJsonMember(value, "extensions");
        if (extensions == nullptr)
        {
            return;
        }

        for (const auto& name : Internal::GetJsonObjectMemberNames(
                 *extensions,
                 "The extensions member must be a JSON object"))
        {
            ExtensionPair extensionPair = {
                name,
                Internal::WriteJson(
                    *Internal::FindJsonMember(*extensions, name))
            };
            if (extensionDeserializer.HasHandler(
                    extensionPair.name, property) ||
                extensionDeserializer.HasHandler(extensionPair.name))
            {
                property.SetExtension(
                    extensionDeserializer.Deserialize(
                        extensionPair, property));
            }
            else
            {
                property.extensions.emplace(
                    std::move(extensionPair.name),
                    std::move(extensionPair.value));
            }
        }
    }

    void ParseExtras(
        const JsonValue& value,
        glTFProperty& property)
    {
        const auto* extras =
            Internal::FindJsonMember(value, "extras");
        if (extras != nullptr)
        {
            property.extras = Internal::WriteJson(*extras);
        }
    }

    void ParseProperty(
        const JsonValue& value,
        glTFProperty& property,
        const ExtensionDeserializer& extensionDeserializer)
    {
        ParseExtensions(value, property, extensionDeserializer);
        ParseExtras(value, property);
    }

    void ParseTextureInfo(
        const JsonValue& value,
        TextureInfo& textureInfo,
        const ExtensionDeserializer& extensionDeserializer)
    {
        Internal::RequireJsonObject(
            value, "TextureInfo must be a JSON object");
        textureInfo.textureId = std::to_string(ReadUInt32(
            Internal::RequireJsonMember(
                value,
                "index",
                "TextureInfo.index was not found"),
            "TextureInfo.index must be an unsigned integer"));
        const auto* texCoord =
            Internal::FindJsonMember(value, "texCoord");
        textureInfo.texCoord = texCoord == nullptr
            ? 0U
            : ReadSize(
                *texCoord,
                "TextureInfo.texCoord must be an unsigned integer");
        ParseProperty(value, textureInfo, extensionDeserializer);
    }

    template<typename T>
    void AddOptionalIndex(
        JsonValue& object,
        const char* name,
        const std::string& id,
        const IndexedContainer<const T>& container)
    {
        if (!id.empty())
        {
            Internal::SetJsonMember(
                object,
                name,
                Internal::CreateJsonSize(container.GetIndex(id)));
        }
    }

    void SerializePropertyExtensions(
        const Document& gltfDocument,
        const glTFProperty& property,
        JsonValue& propertyValue,
        const ExtensionSerializer& extensionSerializer)
    {
        const auto registeredExtensions =
            property.GetExtensions();
        if (property.extensions.empty() &&
            registeredExtensions.empty())
        {
            return;
        }

        JsonValue extensions = Internal::CreateJsonObject();
        std::vector<ExtensionPair> registered;
        registered.reserve(registeredExtensions.size());
        for (const auto& extension : registeredExtensions)
        {
            auto extensionPair = extensionSerializer.Serialize(
                extension, property, gltfDocument);
            if (property.HasUnregisteredExtension(
                    extensionPair.name))
            {
                throw GLTFException(
                    "Registered extension '" +
                    extensionPair.name +
                    "' is also present as an unregistered extension.");
            }
            if (gltfDocument.extensionsUsed.find(
                    extensionPair.name) ==
                gltfDocument.extensionsUsed.end())
            {
                throw GLTFException(
                    "Registered extension '" +
                    extensionPair.name +
                    "' is not present in extensionsUsed");
            }
            registered.push_back(std::move(extensionPair));
        }
        std::sort(
            registered.begin(),
            registered.end(),
            [](const ExtensionPair& left, const ExtensionPair& right)
            {
                return left.name < right.name;
            });
        for (const auto& extension : registered)
        {
            Internal::SetJsonMember(
                extensions,
                extension.name,
                Internal::ParseJson(extension.value));
        }

        std::vector<std::pair<std::string, std::string>>
            unregistered(
                property.extensions.begin(),
                property.extensions.end());
        std::sort(
            unregistered.begin(),
            unregistered.end(),
            [](const std::pair<std::string, std::string>& left,
               const std::pair<std::string, std::string>& right)
            {
                return left.first < right.first;
            });
        for (const auto& extension : unregistered)
        {
            if (gltfDocument.extensionsUsed.find(extension.first) ==
                gltfDocument.extensionsUsed.end())
            {
                throw GLTFException(
                    "Unregistered extension '" + extension.first +
                    "' is not present in extensionsUsed");
            }
            Internal::SetJsonMember(
                extensions,
                extension.first,
                Internal::ParseJson(extension.second));
        }

        Internal::SetJsonMember(
            propertyValue,
            "extensions",
            std::move(extensions));
    }

    void SerializePropertyExtras(
        const glTFProperty& property,
        JsonValue& propertyValue)
    {
        if (!property.extras.empty())
        {
            Internal::SetJsonMember(
                propertyValue,
                "extras",
                Internal::ParseJson(property.extras));
        }
    }

    void SerializeProperty(
        const Document& gltfDocument,
        const glTFProperty& property,
        JsonValue& propertyValue,
        const ExtensionSerializer& extensionSerializer)
    {
        SerializePropertyExtensions(
            gltfDocument,
            property,
            propertyValue,
            extensionSerializer);
        SerializePropertyExtras(property, propertyValue);
    }

    void SerializeTextureInfo(
        const Document& gltfDocument,
        const TextureInfo& textureInfo,
        JsonValue& textureValue,
        const IndexedContainer<const Texture>& textures,
        const ExtensionSerializer& extensionSerializer)
    {
        AddOptionalIndex(
            textureValue,
            "index",
            textureInfo.textureId,
            textures);
        if (textureInfo.texCoord != 0U)
        {
            Internal::SetJsonMember(
                textureValue,
                "texCoord",
                Internal::CreateJsonSize(textureInfo.texCoord));
        }
        SerializeProperty(
            gltfDocument,
            textureInfo,
            textureValue,
            extensionSerializer);
    }

    JsonValue ParseExtensionObject(const std::string& json)
    {
        JsonValue value = Internal::ParseJson(json);
        Internal::RequireJsonObject(
            value, "The extension value must be a JSON object");
        return value;
    }

    std::string GetUInt32MemberAsString(
        const JsonValue& object,
        const char* name)
    {
        const auto* value =
            Internal::FindJsonMember(object, name);
        return value == nullptr
            ? std::string()
            : std::to_string(ReadUInt32(
                *value,
                std::string(name) +
                " must be an unsigned integer"));
    }
}

ExtensionSerializer GLTFSDK_API KHR::GetKHRExtensionSerializer()
{
    using namespace Materials;
    using namespace MeshPrimitives;
    using namespace Nodes;
    using namespace TextureInfos;

    ExtensionSerializer serializer;
    serializer.AddHandler<PBRSpecularGlossiness, Material>(
        PBRSPECULARGLOSSINESS_NAME, SerializePBRSpecGloss);
    serializer.AddHandler<Unlit, Material>(
        UNLIT_NAME, SerializeUnlit);
    serializer.AddHandler<Clearcoat, Material>(
        CLEARCOAT_NAME, SerializeClearcoat);
    serializer.AddHandler<Volume, Material>(
        VOLUME_NAME, SerializeVolume);
    serializer.AddHandler<Iridescence, Material>(
        IRIDESCENCE_NAME, SerializeIridescence);
    serializer.AddHandler<Transmission, Material>(
        TRANSMISSION_NAME, SerializeTransmission);
    serializer.AddHandler<Sheen, Material>(
        SHEEN_NAME, SerializeSheen);
    serializer.AddHandler<Specular, Material>(
        SPECULAR_NAME, SerializeSpecular);
    serializer.AddHandler<DracoMeshCompression, MeshPrimitive>(
        DRACOMESHCOMPRESSION_NAME,
        SerializeDracoMeshCompression);
    serializer.AddHandler<MeshGPUInstancing, Node>(
        MESHGPUINSTANCING_NAME,
        SerializeMeshGPUInstancing);
    serializer.AddHandler<TextureTransform, TextureInfo>(
        TEXTURETRANSFORM_NAME, SerializeTextureTransform);
    serializer.AddHandler<
        TextureTransform,
        Material::NormalTextureInfo>(
            TEXTURETRANSFORM_NAME,
            SerializeTextureTransform);
    serializer.AddHandler<
        TextureTransform,
        Material::OcclusionTextureInfo>(
            TEXTURETRANSFORM_NAME,
            SerializeTextureTransform);
    return serializer;
}

ExtensionDeserializer GLTFSDK_API KHR::GetKHRExtensionDeserializer()
{
    using namespace Materials;
    using namespace MeshPrimitives;
    using namespace Nodes;
    using namespace TextureInfos;

    ExtensionDeserializer deserializer;
    deserializer.AddHandler<PBRSpecularGlossiness, Material>(
        PBRSPECULARGLOSSINESS_NAME, DeserializePBRSpecGloss);
    deserializer.AddHandler<Unlit, Material>(
        UNLIT_NAME, DeserializeUnlit);
    deserializer.AddHandler<Clearcoat, Material>(
        CLEARCOAT_NAME, DeserializeClearcoat);
    deserializer.AddHandler<Volume, Material>(
        VOLUME_NAME, DeserializeVolume);
    deserializer.AddHandler<Iridescence, Material>(
        IRIDESCENCE_NAME, DeserializeIridescence);
    deserializer.AddHandler<Transmission, Material>(
        TRANSMISSION_NAME, DeserializeTransmission);
    deserializer.AddHandler<Sheen, Material>(
        SHEEN_NAME, DeserializeSheen);
    deserializer.AddHandler<Specular, Material>(
        SPECULAR_NAME, DeserializeSpecular);
    deserializer.AddHandler<DracoMeshCompression, MeshPrimitive>(
        DRACOMESHCOMPRESSION_NAME,
        DeserializeDracoMeshCompression);
    deserializer.AddHandler<MeshGPUInstancing, Node>(
        MESHGPUINSTANCING_NAME,
        DeserializeMeshGPUInstancing);
    deserializer.AddHandler<TextureTransform, TextureInfo>(
        TEXTURETRANSFORM_NAME, DeserializeTextureTransform);
    deserializer.AddHandler<
        TextureTransform,
        Material::NormalTextureInfo>(
            TEXTURETRANSFORM_NAME,
            DeserializeTextureTransform);
    deserializer.AddHandler<
        TextureTransform,
        Material::OcclusionTextureInfo>(
            TEXTURETRANSFORM_NAME,
            DeserializeTextureTransform);
    return deserializer;
}

KHR::Materials::PBRSpecularGlossiness::PBRSpecularGlossiness()
    : diffuseFactor(1.0F, 1.0F, 1.0F, 1.0F),
      specularFactor(1.0F, 1.0F, 1.0F),
      glossinessFactor(1.0F)
{
}

std::unique_ptr<Extension>
KHR::Materials::PBRSpecularGlossiness::Clone() const
{
    return std::make_unique<PBRSpecularGlossiness>(*this);
}

bool KHR::Materials::PBRSpecularGlossiness::IsEqual(
    const Extension& rhs) const
{
    const auto other =
        dynamic_cast<const PBRSpecularGlossiness*>(&rhs);
    return other != nullptr &&
        glTFProperty::Equals(*this, *other) &&
        diffuseFactor == other->diffuseFactor &&
        diffuseTexture == other->diffuseTexture &&
        specularFactor == other->specularFactor &&
        glossinessFactor == other->glossinessFactor &&
        specularGlossinessTexture ==
            other->specularGlossinessTexture;
}

std::string GLTFSDK_API KHR::Materials::SerializePBRSpecGloss(
    const Materials::PBRSpecularGlossiness& extension,
    const Document& gltfDocument,
    const ExtensionSerializer& extensionSerializer)
{
    JsonValue value = Internal::CreateJsonObject();
    if (extension.diffuseFactor !=
        Color4(1.0F, 1.0F, 1.0F, 1.0F))
    {
        Internal::SetJsonMember(
            value,
            "diffuseFactor",
            CreateFloatArray(extension.diffuseFactor));
    }
    if (!extension.diffuseTexture.textureId.empty())
    {
        JsonValue texture = Internal::CreateJsonObject();
        SerializeTextureInfo(
            gltfDocument,
            extension.diffuseTexture,
            texture,
            gltfDocument.textures,
            extensionSerializer);
        Internal::SetJsonMember(
            value, "diffuseTexture", std::move(texture));
    }
    if (extension.specularFactor !=
        Color3(1.0F, 1.0F, 1.0F))
    {
        Internal::SetJsonMember(
            value,
            "specularFactor",
            CreateFloatArray(extension.specularFactor));
    }
    if (extension.glossinessFactor != 1.0F)
    {
        Internal::SetJsonMember(
            value,
            "glossinessFactor",
            Internal::CreateJsonFloat(
                extension.glossinessFactor));
    }
    if (!extension.specularGlossinessTexture.textureId.empty())
    {
        JsonValue texture = Internal::CreateJsonObject();
        SerializeTextureInfo(
            gltfDocument,
            extension.specularGlossinessTexture,
            texture,
            gltfDocument.textures,
            extensionSerializer);
        Internal::SetJsonMember(
            value,
            "specularGlossinessTexture",
            std::move(texture));
    }
    SerializeProperty(
        gltfDocument,
        extension,
        value,
        extensionSerializer);
    return Internal::WriteJson(value);
}

std::unique_ptr<Extension> GLTFSDK_API
KHR::Materials::DeserializePBRSpecGloss(
    const std::string& json,
    const ExtensionDeserializer& extensionDeserializer)
{
    Materials::PBRSpecularGlossiness extension;
    const JsonValue value = ParseExtensionObject(json);

    const auto* diffuse =
        Internal::FindJsonMember(value, "diffuseFactor");
    if (diffuse != nullptr)
    {
        const auto elements = GetFixedSizeFloatArray(
            *diffuse,
            4U,
            "diffuseFactor must be a JSON array of 4 "
            "numeric elements");
        extension.diffuseFactor = Color4(
            elements[0],
            elements[1],
            elements[2],
            elements[3]);
    }
    const auto* diffuseTexture =
        Internal::FindJsonMember(value, "diffuseTexture");
    if (diffuseTexture != nullptr)
    {
        ParseTextureInfo(
            *diffuseTexture,
            extension.diffuseTexture,
            extensionDeserializer);
    }
    const auto* specular =
        Internal::FindJsonMember(value, "specularFactor");
    if (specular != nullptr)
    {
        const auto elements = GetFixedSizeFloatArray(
            *specular,
            3U,
            "specularFactor must be a JSON array of 3 "
            "numeric elements");
        extension.specularFactor = Color3(
            elements[0], elements[1], elements[2]);
    }
    extension.glossinessFactor = GetFloatMemberOrDefault(
        value, "glossinessFactor", 1.0F);
    const auto* texture =
        Internal::FindJsonMember(
            value, "specularGlossinessTexture");
    if (texture != nullptr)
    {
        ParseTextureInfo(
            *texture,
            extension.specularGlossinessTexture,
            extensionDeserializer);
    }
    ParseProperty(
        value, extension, extensionDeserializer);
    return std::make_unique<PBRSpecularGlossiness>(
        extension);
}

std::unique_ptr<Extension>
KHR::Materials::Unlit::Clone() const
{
    return std::make_unique<Unlit>(*this);
}

bool KHR::Materials::Unlit::IsEqual(
    const Extension& rhs) const
{
    return dynamic_cast<const Unlit*>(&rhs) != nullptr;
}

std::string GLTFSDK_API KHR::Materials::SerializeUnlit(
    const Materials::Unlit& extension,
    const Document& gltfDocument,
    const ExtensionSerializer& extensionSerializer)
{
    JsonValue value = Internal::CreateJsonObject();
    SerializeProperty(
        gltfDocument,
        extension,
        value,
        extensionSerializer);
    return Internal::WriteJson(value);
}

std::unique_ptr<Extension> GLTFSDK_API
KHR::Materials::DeserializeUnlit(
    const std::string& json,
    const ExtensionDeserializer& extensionDeserializer)
{
    Materials::Unlit extension;
    const JsonValue value = ParseExtensionObject(json);
    ParseProperty(
        value, extension, extensionDeserializer);
    return std::make_unique<Unlit>(extension);
}

KHR::Materials::Clearcoat::Clearcoat()
    : factor(0.0F),
      roughnessFactor(0.0F)
{
}

std::unique_ptr<Extension>
KHR::Materials::Clearcoat::Clone() const
{
    return std::make_unique<Clearcoat>(*this);
}

bool KHR::Materials::Clearcoat::IsEqual(
    const Extension& rhs) const
{
    const auto other =
        dynamic_cast<const Clearcoat*>(&rhs);
    return other != nullptr &&
        glTFProperty::Equals(*this, *other) &&
        factor == other->factor &&
        texture == other->texture &&
        roughnessFactor == other->roughnessFactor &&
        roughnessTexture == other->roughnessTexture &&
        normalTexture == other->normalTexture;
}

std::string GLTFSDK_API KHR::Materials::SerializeClearcoat(
    const Materials::Clearcoat& extension,
    const Document& gltfDocument,
    const ExtensionSerializer& extensionSerializer)
{
    JsonValue value = Internal::CreateJsonObject();
    if (extension.factor != 0.0F)
    {
        Internal::SetJsonMember(
            value,
            "clearcoatFactor",
            Internal::CreateJsonFloat(extension.factor));
    }
    if (!extension.texture.textureId.empty())
    {
        JsonValue texture = Internal::CreateJsonObject();
        SerializeTextureInfo(
            gltfDocument,
            extension.texture,
            texture,
            gltfDocument.textures,
            extensionSerializer);
        Internal::SetJsonMember(
            value, "clearcoatTexture", std::move(texture));
    }
    if (extension.roughnessFactor != 0.0F)
    {
        Internal::SetJsonMember(
            value,
            "clearcoatRoughnessFactor",
            Internal::CreateJsonFloat(
                extension.roughnessFactor));
    }
    if (!extension.roughnessTexture.textureId.empty())
    {
        JsonValue texture = Internal::CreateJsonObject();
        SerializeTextureInfo(
            gltfDocument,
            extension.roughnessTexture,
            texture,
            gltfDocument.textures,
            extensionSerializer);
        Internal::SetJsonMember(
            value,
            "clearcoatRoughnessTexture",
            std::move(texture));
    }
    if (!extension.normalTexture.textureId.empty())
    {
        JsonValue texture = Internal::CreateJsonObject();
        SerializeTextureInfo(
            gltfDocument,
            extension.normalTexture,
            texture,
            gltfDocument.textures,
            extensionSerializer);
        Internal::SetJsonMember(
            value,
            "clearcoatNormalTexture",
            std::move(texture));
    }
    SerializeProperty(
        gltfDocument,
        extension,
        value,
        extensionSerializer);
    return Internal::WriteJson(value);
}

std::unique_ptr<Extension> GLTFSDK_API
KHR::Materials::DeserializeClearcoat(
    const std::string& json,
    const ExtensionDeserializer& extensionDeserializer)
{
    Materials::Clearcoat extension;
    const JsonValue value = ParseExtensionObject(json);
    extension.factor = GetFloatMemberOrDefault(
        value, "clearcoatFactor", 0.0F);
    const auto* texture =
        Internal::FindJsonMember(value, "clearcoatTexture");
    if (texture != nullptr)
    {
        ParseTextureInfo(
            *texture,
            extension.texture,
            extensionDeserializer);
    }
    extension.roughnessFactor = GetFloatMemberOrDefault(
        value, "clearcoatRoughnessFactor", 0.0F);
    const auto* roughness =
        Internal::FindJsonMember(
            value, "clearcoatRoughnessTexture");
    if (roughness != nullptr)
    {
        ParseTextureInfo(
            *roughness,
            extension.roughnessTexture,
            extensionDeserializer);
    }
    const auto* normal =
        Internal::FindJsonMember(
            value, "clearcoatNormalTexture");
    if (normal != nullptr)
    {
        ParseTextureInfo(
            *normal,
            extension.normalTexture,
            extensionDeserializer);
    }
    ParseProperty(
        value, extension, extensionDeserializer);
    return std::make_unique<Clearcoat>(extension);
}

KHR::Materials::Volume::Volume()
    : attenuationColor(1.0F, 1.0F, 1.0F),
      attenuationDistance(
          std::numeric_limits<float>::infinity()),
      thicknessFactor(0.0F)
{
}

std::unique_ptr<Extension>
KHR::Materials::Volume::Clone() const
{
    return std::make_unique<Volume>(*this);
}

bool KHR::Materials::Volume::IsEqual(
    const Extension& rhs) const
{
    const auto other = dynamic_cast<const Volume*>(&rhs);
    return other != nullptr &&
        glTFProperty::Equals(*this, *other) &&
        attenuationColor == other->attenuationColor &&
        attenuationDistance == other->attenuationDistance &&
        thicknessFactor == other->thicknessFactor &&
        thicknessTexture == other->thicknessTexture;
}

std::string GLTFSDK_API KHR::Materials::SerializeVolume(
    const Materials::Volume& extension,
    const Document& gltfDocument,
    const ExtensionSerializer& extensionSerializer)
{
    JsonValue value = Internal::CreateJsonObject();
    if (extension.attenuationColor !=
        Color3(1.0F, 1.0F, 1.0F))
    {
        Internal::SetJsonMember(
            value,
            "attenuationColor",
            CreateFloatArray(extension.attenuationColor));
    }
    if (extension.attenuationDistance !=
        std::numeric_limits<float>::infinity())
    {
        Internal::SetJsonMember(
            value,
            "attenuationDistance",
            Internal::CreateJsonFloat(
                extension.attenuationDistance));
    }
    if (extension.thicknessFactor != 0.0F)
    {
        Internal::SetJsonMember(
            value,
            "thicknessFactor",
            Internal::CreateJsonFloat(
                extension.thicknessFactor));
    }
    if (!extension.thicknessTexture.textureId.empty())
    {
        JsonValue texture = Internal::CreateJsonObject();
        SerializeTextureInfo(
            gltfDocument,
            extension.thicknessTexture,
            texture,
            gltfDocument.textures,
            extensionSerializer);
        Internal::SetJsonMember(
            value,
            "thicknessTexture",
            std::move(texture));
    }
    SerializeProperty(
        gltfDocument,
        extension,
        value,
        extensionSerializer);
    return Internal::WriteJson(value);
}

std::unique_ptr<Extension> GLTFSDK_API
KHR::Materials::DeserializeVolume(
    const std::string& json,
    const ExtensionDeserializer& extensionDeserializer)
{
    Materials::Volume extension;
    const JsonValue value = ParseExtensionObject(json);
    const auto* color =
        Internal::FindJsonMember(value, "attenuationColor");
    if (color != nullptr)
    {
        const auto elements = GetFixedSizeFloatArray(
            *color,
            3U,
            "attenuationColor must be a JSON array of 3 "
            "numeric elements");
        extension.attenuationColor = Color3(
            elements[0], elements[1], elements[2]);
    }
    extension.attenuationDistance = GetFloatMemberOrDefault(
        value,
        "attenuationDistance",
        std::numeric_limits<float>::infinity());
    extension.thicknessFactor = GetFloatMemberOrDefault(
        value, "thicknessFactor", 0.0F);
    const auto* texture =
        Internal::FindJsonMember(value, "thicknessTexture");
    if (texture != nullptr)
    {
        ParseTextureInfo(
            *texture,
            extension.thicknessTexture,
            extensionDeserializer);
    }
    ParseProperty(
        value, extension, extensionDeserializer);
    return std::make_unique<Volume>(extension);
}

KHR::Materials::Iridescence::Iridescence()
    : factor(0.0F),
      ior(1.3F),
      thicknessMin(100.0F),
      thicknessMax(400.0F)
{
}

std::unique_ptr<Extension>
KHR::Materials::Iridescence::Clone() const
{
    return std::make_unique<Iridescence>(*this);
}

bool KHR::Materials::Iridescence::IsEqual(
    const Extension& rhs) const
{
    const auto other =
        dynamic_cast<const Iridescence*>(&rhs);
    return other != nullptr &&
        glTFProperty::Equals(*this, *other) &&
        factor == other->factor &&
        texture == other->texture &&
        ior == other->ior &&
        thicknessMin == other->thicknessMin &&
        thicknessMax == other->thicknessMax &&
        thicknessTexture == other->thicknessTexture;
}

std::string GLTFSDK_API KHR::Materials::SerializeIridescence(
    const Materials::Iridescence& extension,
    const Document& gltfDocument,
    const ExtensionSerializer& extensionSerializer)
{
    JsonValue value = Internal::CreateJsonObject();
    if (extension.factor != 0.0F)
    {
        Internal::SetJsonMember(
            value,
            "iridescenceFactor",
            Internal::CreateJsonFloat(extension.factor));
    }
    if (!extension.texture.textureId.empty())
    {
        JsonValue texture = Internal::CreateJsonObject();
        SerializeTextureInfo(
            gltfDocument,
            extension.texture,
            texture,
            gltfDocument.textures,
            extensionSerializer);
        Internal::SetJsonMember(
            value,
            "iridescenceTexture",
            std::move(texture));
    }
    if (extension.ior != 1.3F)
    {
        Internal::SetJsonMember(
            value,
            "iridescenceIor",
            Internal::CreateJsonFloat(extension.ior));
    }
    if (extension.thicknessMin != 100.0F)
    {
        Internal::SetJsonMember(
            value,
            "iridescenceThicknessMinimum",
            Internal::CreateJsonFloat(
                extension.thicknessMin));
    }
    if (extension.thicknessMax != 400.0F)
    {
        Internal::SetJsonMember(
            value,
            "iridescenceThicknessMaximum",
            Internal::CreateJsonFloat(
                extension.thicknessMax));
    }
    if (!extension.thicknessTexture.textureId.empty())
    {
        JsonValue texture = Internal::CreateJsonObject();
        SerializeTextureInfo(
            gltfDocument,
            extension.thicknessTexture,
            texture,
            gltfDocument.textures,
            extensionSerializer);
        Internal::SetJsonMember(
            value,
            "iridescenceThicknessTexture",
            std::move(texture));
    }
    SerializeProperty(
        gltfDocument,
        extension,
        value,
        extensionSerializer);
    return Internal::WriteJson(value);
}

std::unique_ptr<Extension> GLTFSDK_API
KHR::Materials::DeserializeIridescence(
    const std::string& json,
    const ExtensionDeserializer& extensionDeserializer)
{
    Materials::Iridescence extension;
    const JsonValue value = ParseExtensionObject(json);
    extension.factor = GetFloatMemberOrDefault(
        value, "iridescenceFactor", 0.0F);
    const auto* texture =
        Internal::FindJsonMember(
            value, "iridescenceTexture");
    if (texture != nullptr)
    {
        ParseTextureInfo(
            *texture,
            extension.texture,
            extensionDeserializer);
    }
    extension.ior = GetFloatMemberOrDefault(
        value, "iridescenceIor", 1.3F);
    extension.thicknessMin = GetFloatMemberOrDefault(
        value, "iridescenceThicknessMinimum", 100.0F);
    extension.thicknessMax = GetFloatMemberOrDefault(
        value, "iridescenceThicknessMaximum", 400.0F);
    const auto* thickness =
        Internal::FindJsonMember(
            value, "iridescenceThicknessTexture");
    if (thickness != nullptr)
    {
        ParseTextureInfo(
            *thickness,
            extension.thicknessTexture,
            extensionDeserializer);
    }
    ParseProperty(
        value, extension, extensionDeserializer);
    return std::make_unique<Iridescence>(extension);
}

KHR::Materials::Transmission::Transmission()
    : factor(0.0F)
{
}

std::unique_ptr<Extension>
KHR::Materials::Transmission::Clone() const
{
    return std::make_unique<Transmission>(*this);
}

bool KHR::Materials::Transmission::IsEqual(
    const Extension& rhs) const
{
    const auto other =
        dynamic_cast<const Transmission*>(&rhs);
    return other != nullptr &&
        glTFProperty::Equals(*this, *other) &&
        factor == other->factor &&
        texture == other->texture;
}

std::string GLTFSDK_API KHR::Materials::SerializeTransmission(
    const Materials::Transmission& extension,
    const Document& gltfDocument,
    const ExtensionSerializer& extensionSerializer)
{
    JsonValue value = Internal::CreateJsonObject();
    if (extension.factor != 0.0F)
    {
        Internal::SetJsonMember(
            value,
            "transmissionFactor",
            Internal::CreateJsonFloat(extension.factor));
    }
    if (!extension.texture.textureId.empty())
    {
        JsonValue texture = Internal::CreateJsonObject();
        SerializeTextureInfo(
            gltfDocument,
            extension.texture,
            texture,
            gltfDocument.textures,
            extensionSerializer);
        Internal::SetJsonMember(
            value,
            "transmissionTexture",
            std::move(texture));
    }
    SerializeProperty(
        gltfDocument,
        extension,
        value,
        extensionSerializer);
    return Internal::WriteJson(value);
}

std::unique_ptr<Extension> GLTFSDK_API
KHR::Materials::DeserializeTransmission(
    const std::string& json,
    const ExtensionDeserializer& extensionDeserializer)
{
    Materials::Transmission extension;
    const JsonValue value = ParseExtensionObject(json);
    extension.factor = GetFloatMemberOrDefault(
        value, "transmissionFactor", 0.0F);
    const auto* texture =
        Internal::FindJsonMember(
            value, "transmissionTexture");
    if (texture != nullptr)
    {
        ParseTextureInfo(
            *texture,
            extension.texture,
            extensionDeserializer);
    }
    ParseProperty(
        value, extension, extensionDeserializer);
    return std::make_unique<Transmission>(extension);
}

KHR::Materials::Sheen::Sheen()
    : colorFactor(0.0F, 0.0F, 0.0F),
      roughnessFactor(0.0F)
{
}

std::unique_ptr<Extension>
KHR::Materials::Sheen::Clone() const
{
    return std::make_unique<Sheen>(*this);
}

bool KHR::Materials::Sheen::IsEqual(
    const Extension& rhs) const
{
    const auto other = dynamic_cast<const Sheen*>(&rhs);
    return other != nullptr &&
        glTFProperty::Equals(*this, *other) &&
        colorFactor == other->colorFactor &&
        colorTexture == other->colorTexture &&
        roughnessFactor == other->roughnessFactor &&
        roughnessTexture == other->roughnessTexture;
}

std::string GLTFSDK_API KHR::Materials::SerializeSheen(
    const Materials::Sheen& extension,
    const Document& gltfDocument,
    const ExtensionSerializer& extensionSerializer)
{
    JsonValue value = Internal::CreateJsonObject();
    if (extension.colorFactor !=
        Color3(0.0F, 0.0F, 0.0F))
    {
        Internal::SetJsonMember(
            value,
            "sheenColorFactor",
            CreateFloatArray(extension.colorFactor));
    }
    if (!extension.colorTexture.textureId.empty())
    {
        JsonValue texture = Internal::CreateJsonObject();
        SerializeTextureInfo(
            gltfDocument,
            extension.colorTexture,
            texture,
            gltfDocument.textures,
            extensionSerializer);
        Internal::SetJsonMember(
            value,
            "sheenColorTexture",
            std::move(texture));
    }
    if (extension.roughnessFactor != 0.0F)
    {
        Internal::SetJsonMember(
            value,
            "sheenRoughnessFactor",
            Internal::CreateJsonFloat(
                extension.roughnessFactor));
    }
    if (!extension.roughnessTexture.textureId.empty())
    {
        JsonValue texture = Internal::CreateJsonObject();
        SerializeTextureInfo(
            gltfDocument,
            extension.roughnessTexture,
            texture,
            gltfDocument.textures,
            extensionSerializer);
        Internal::SetJsonMember(
            value,
            "sheenRoughnessTexture",
            std::move(texture));
    }
    SerializeProperty(
        gltfDocument,
        extension,
        value,
        extensionSerializer);
    return Internal::WriteJson(value);
}

std::unique_ptr<Extension> GLTFSDK_API
KHR::Materials::DeserializeSheen(
    const std::string& json,
    const ExtensionDeserializer& extensionDeserializer)
{
    Materials::Sheen extension;
    const JsonValue value = ParseExtensionObject(json);
    const auto* color =
        Internal::FindJsonMember(value, "sheenColorFactor");
    if (color != nullptr)
    {
        const auto elements = GetFixedSizeFloatArray(
            *color,
            3U,
            "sheenColorFactor must be a JSON array of 3 "
            "numeric elements");
        extension.colorFactor = Color3(
            elements[0], elements[1], elements[2]);
    }
    const auto* colorTexture =
        Internal::FindJsonMember(value, "sheenColorTexture");
    if (colorTexture != nullptr)
    {
        ParseTextureInfo(
            *colorTexture,
            extension.colorTexture,
            extensionDeserializer);
    }
    extension.roughnessFactor = GetFloatMemberOrDefault(
        value, "sheenRoughnessFactor", 0.0F);
    const auto* roughness =
        Internal::FindJsonMember(
            value, "sheenRoughnessTexture");
    if (roughness != nullptr)
    {
        ParseTextureInfo(
            *roughness,
            extension.roughnessTexture,
            extensionDeserializer);
    }
    ParseProperty(
        value, extension, extensionDeserializer);
    return std::make_unique<Sheen>(extension);
}

KHR::Materials::Specular::Specular()
    : factor(0.0F),
      colorFactor(1.0F, 1.0F, 1.0F)
{
}

std::unique_ptr<Extension>
KHR::Materials::Specular::Clone() const
{
    return std::make_unique<Specular>(*this);
}

bool KHR::Materials::Specular::IsEqual(
    const Extension& rhs) const
{
    const auto other =
        dynamic_cast<const Specular*>(&rhs);
    return other != nullptr &&
        glTFProperty::Equals(*this, *other) &&
        colorFactor == other->colorFactor &&
        colorTexture == other->colorTexture &&
        factor == other->factor &&
        texture == other->texture;
}

std::string GLTFSDK_API KHR::Materials::SerializeSpecular(
    const Materials::Specular& extension,
    const Document& gltfDocument,
    const ExtensionSerializer& extensionSerializer)
{
    JsonValue value = Internal::CreateJsonObject();
    if (extension.factor != 0.0F)
    {
        Internal::SetJsonMember(
            value,
            "specularFactor",
            Internal::CreateJsonFloat(extension.factor));
    }
    if (!extension.texture.textureId.empty())
    {
        JsonValue texture = Internal::CreateJsonObject();
        SerializeTextureInfo(
            gltfDocument,
            extension.texture,
            texture,
            gltfDocument.textures,
            extensionSerializer);
        Internal::SetJsonMember(
            value, "specularTexture", std::move(texture));
    }
    if (extension.colorFactor !=
        Color3(1.0F, 1.0F, 1.0F))
    {
        Internal::SetJsonMember(
            value,
            "specularColorFactor",
            CreateFloatArray(extension.colorFactor));
    }
    if (!extension.colorTexture.textureId.empty())
    {
        JsonValue texture = Internal::CreateJsonObject();
        SerializeTextureInfo(
            gltfDocument,
            extension.colorTexture,
            texture,
            gltfDocument.textures,
            extensionSerializer);
        Internal::SetJsonMember(
            value,
            "specularColorTexture",
            std::move(texture));
    }
    SerializeProperty(
        gltfDocument,
        extension,
        value,
        extensionSerializer);
    return Internal::WriteJson(value);
}

std::unique_ptr<Extension> GLTFSDK_API
KHR::Materials::DeserializeSpecular(
    const std::string& json,
    const ExtensionDeserializer& extensionDeserializer)
{
    Materials::Specular extension;
    const JsonValue value = ParseExtensionObject(json);
    extension.factor = GetFloatMemberOrDefault(
        value, "specularFactor", 0.0F);
    const auto* texture =
        Internal::FindJsonMember(value, "specularTexture");
    if (texture != nullptr)
    {
        ParseTextureInfo(
            *texture,
            extension.texture,
            extensionDeserializer);
    }
    const auto* color =
        Internal::FindJsonMember(value, "specularColorFactor");
    if (color != nullptr)
    {
        const auto elements = GetFixedSizeFloatArray(
            *color,
            3U,
            "specularColorFactor must be a JSON array of 3 "
            "numeric elements");
        extension.colorFactor = Color3(
            elements[0], elements[1], elements[2]);
    }
    const auto* colorTexture =
        Internal::FindJsonMember(
            value, "specularColorTexture");
    if (colorTexture != nullptr)
    {
        ParseTextureInfo(
            *colorTexture,
            extension.colorTexture,
            extensionDeserializer);
    }
    ParseProperty(
        value, extension, extensionDeserializer);
    return std::make_unique<Specular>(extension);
}

std::unique_ptr<Extension>
KHR::MeshPrimitives::DracoMeshCompression::Clone() const
{
    return std::make_unique<DracoMeshCompression>(*this);
}

bool KHR::MeshPrimitives::DracoMeshCompression::IsEqual(
    const Extension& rhs) const
{
    const auto other =
        dynamic_cast<const DracoMeshCompression*>(&rhs);
    return other != nullptr &&
        glTFProperty::Equals(*this, *other) &&
        bufferViewId == other->bufferViewId &&
        attributes == other->attributes;
}

std::string GLTFSDK_API
KHR::MeshPrimitives::SerializeDracoMeshCompression(
    const MeshPrimitives::DracoMeshCompression& extension,
    const Document& gltfDocument,
    const ExtensionSerializer& extensionSerializer)
{
    JsonValue value = Internal::CreateJsonObject();
    AddOptionalIndex(
        value,
        "bufferView",
        extension.bufferViewId,
        gltfDocument.bufferViews);

    JsonValue attributes = Internal::CreateJsonObject();
    std::vector<std::pair<std::string, std::uint32_t>>
        sorted(
            extension.attributes.begin(),
            extension.attributes.end());
    std::sort(
        sorted.begin(),
        sorted.end(),
        [](const std::pair<std::string, std::uint32_t>& left,
           const std::pair<std::string, std::uint32_t>& right)
        {
            return left.first < right.first;
        });
    for (const auto& attribute : sorted)
    {
        Internal::SetJsonMember(
            attributes,
            attribute.first,
            Internal::CreateJsonUInt32(attribute.second));
    }
    Internal::SetJsonMember(
        value, "attributes", std::move(attributes));
    SerializeProperty(
        gltfDocument,
        extension,
        value,
        extensionSerializer);
    return Internal::WriteJson(value);
}

std::unique_ptr<Extension> GLTFSDK_API
KHR::MeshPrimitives::DeserializeDracoMeshCompression(
    const std::string& json,
    const ExtensionDeserializer& extensionDeserializer)
{
    auto extension =
        std::make_unique<DracoMeshCompression>();
    const JsonValue value = ParseExtensionObject(json);
    extension->bufferViewId =
        GetUInt32MemberAsString(value, "bufferView");

    const auto* attributes =
        Internal::FindJsonMember(value, "attributes");
    if (attributes != nullptr)
    {
        for (const auto& name :
             Internal::GetJsonObjectMemberNames(
                 *attributes,
                 "Member attributes of " +
                 std::string(DRACOMESHCOMPRESSION_NAME) +
                 " is not an object."))
        {
            extension->attributes.emplace(
                name,
                ReadUInt32(
                    *Internal::FindJsonMember(
                        *attributes, name),
                    "Attribute " + name + " of " +
                    std::string(DRACOMESHCOMPRESSION_NAME) +
                    " is not an unsigned integer."));
        }
    }
    ParseProperty(
        value, *extension, extensionDeserializer);
    return extension;
}

KHR::Nodes::MeshGPUInstancing::MeshGPUInstancing()
{
}

std::unique_ptr<Extension>
KHR::Nodes::MeshGPUInstancing::Clone() const
{
    return std::make_unique<MeshGPUInstancing>(*this);
}

bool KHR::Nodes::MeshGPUInstancing::IsEqual(
    const Extension& rhs) const
{
    const auto other =
        dynamic_cast<const MeshGPUInstancing*>(&rhs);
    return other != nullptr &&
        glTFProperty::Equals(*this, *other);
}

std::string GLTFSDK_API
KHR::Nodes::SerializeMeshGPUInstancing(
    const Nodes::MeshGPUInstancing& extension,
    const Document& gltfDocument,
    const ExtensionSerializer& extensionSerializer)
{
    JsonValue value = Internal::CreateJsonObject();
    JsonValue attributes = Internal::CreateJsonObject();
    std::vector<std::pair<std::string, std::string>>
        sorted(
            extension.attributes.begin(),
            extension.attributes.end());
    std::sort(
        sorted.begin(),
        sorted.end(),
        [](const std::pair<std::string, std::string>& left,
           const std::pair<std::string, std::string>& right)
        {
            return left.first < right.first;
        });
    for (const auto& attribute : sorted)
    {
        Internal::SetJsonMember(
            attributes,
            attribute.first,
            Internal::CreateJsonSize(
                gltfDocument.accessors.GetIndex(
                    attribute.second)));
    }
    Internal::SetJsonMember(
        value, "attributes", std::move(attributes));
    SerializeProperty(
        gltfDocument,
        extension,
        value,
        extensionSerializer);
    return Internal::WriteJson(value);
}

std::unique_ptr<Extension> GLTFSDK_API
KHR::Nodes::DeserializeMeshGPUInstancing(
    const std::string& json,
    const ExtensionDeserializer& extensionDeserializer)
{
    Nodes::MeshGPUInstancing extension;
    const JsonValue value = ParseExtensionObject(json);
    const auto* attributes =
        Internal::FindJsonMember(value, "attributes");
    if (attributes != nullptr)
    {
        for (const auto& name :
             Internal::GetJsonObjectMemberNames(
                 *attributes,
                 "EXT_mesh_gpu_instancing.attributes must be "
                 "a JSON object"))
        {
            extension.attributes[name] = std::to_string(
                ReadUInt32(
                    *Internal::FindJsonMember(
                        *attributes, name),
                    "EXT_mesh_gpu_instancing.attributes values "
                    "must be unsigned integers"));
        }
    }
    ParseProperty(
        value, extension, extensionDeserializer);
    return std::make_unique<MeshGPUInstancing>(extension);
}

KHR::TextureInfos::TextureTransform::TextureTransform()
    : offset(Vector2::ZERO),
      rotation(0.0F),
      scale(Vector2::ONE),
      texCoord()
{
}

KHR::TextureInfos::TextureTransform::TextureTransform(
    const TextureTransform& other)
    : offset(other.offset),
      rotation(other.rotation),
      scale(other.scale),
      texCoord(other.texCoord)
{
}

std::unique_ptr<Extension>
KHR::TextureInfos::TextureTransform::Clone() const
{
    return std::make_unique<TextureTransform>(*this);
}

bool KHR::TextureInfos::TextureTransform::IsEqual(
    const Extension& rhs) const
{
    const auto other =
        dynamic_cast<const TextureTransform*>(&rhs);
    return other != nullptr &&
        glTFProperty::Equals(*this, *other) &&
        offset == other->offset &&
        rotation == other->rotation &&
        scale == other->scale &&
        texCoord == other->texCoord;
}

std::string GLTFSDK_API
KHR::TextureInfos::SerializeTextureTransform(
    const TextureTransform& extension,
    const Document& gltfDocument,
    const ExtensionSerializer& extensionSerializer)
{
    JsonValue value = Internal::CreateJsonObject();
    if (extension.offset != Vector2::ZERO)
    {
        Internal::SetJsonMember(
            value,
            "offset",
            CreateFloatArray(extension.offset));
    }
    if (extension.rotation != 0.0F)
    {
        Internal::SetJsonMember(
            value,
            "rotation",
            Internal::CreateJsonFloat(extension.rotation));
    }
    if (extension.scale != Vector2::ONE)
    {
        Internal::SetJsonMember(
            value,
            "scale",
            CreateFloatArray(extension.scale));
    }
    if (extension.texCoord)
    {
        Internal::SetJsonMember(
            value,
            "texCoord",
            Internal::CreateJsonSize(
                extension.texCoord.Get()));
    }
    SerializeProperty(
        gltfDocument,
        extension,
        value,
        extensionSerializer);
    return Internal::WriteJson(value);
}

std::unique_ptr<Extension> GLTFSDK_API
KHR::TextureInfos::DeserializeTextureTransform(
    const std::string& json,
    const ExtensionDeserializer& extensionDeserializer)
{
    TextureTransform extension;
    const JsonValue value = ParseExtensionObject(json);

    const auto* offset =
        Internal::FindJsonMember(value, "offset");
    if (offset != nullptr)
    {
        if (!Internal::IsJsonArray(*offset))
        {
            throw GLTFException(
                "Offset member of " +
                std::string(TEXTURETRANSFORM_NAME) +
                " must be an array.");
        }
        if (Internal::GetJsonArraySize(*offset) != 2U)
        {
            throw GLTFException(
                "Offset member of " +
                std::string(TEXTURETRANSFORM_NAME) +
                " must have two values.");
        }
        extension.offset.x = ReadFloat(
            Internal::GetJsonArrayElement(
                *offset, 0U, "Offset value is missing"),
            "Offset member of " +
            std::string(TEXTURETRANSFORM_NAME) +
            " must contain numeric values.");
        extension.offset.y = ReadFloat(
            Internal::GetJsonArrayElement(
                *offset, 1U, "Offset value is missing"),
            "Offset member of " +
            std::string(TEXTURETRANSFORM_NAME) +
            " must contain numeric values.");
    }

    const auto* rotation =
        Internal::FindJsonMember(value, "rotation");
    if (rotation != nullptr)
    {
        extension.rotation = ReadFloat(
            *rotation,
            "Rotation member of " +
            std::string(TEXTURETRANSFORM_NAME) +
            " must be a number.");
    }

    const auto* scale =
        Internal::FindJsonMember(value, "scale");
    if (scale != nullptr)
    {
        if (!Internal::IsJsonArray(*scale))
        {
            throw GLTFException(
                "Scale member of " +
                std::string(TEXTURETRANSFORM_NAME) +
                " must be an array.");
        }
        if (Internal::GetJsonArraySize(*scale) != 2U)
        {
            throw GLTFException(
                "Scale member of " +
                std::string(TEXTURETRANSFORM_NAME) +
                " must have two values.");
        }
        extension.scale.x = ReadFloat(
            Internal::GetJsonArrayElement(
                *scale, 0U, "Scale value is missing"),
            "Scale member of " +
            std::string(TEXTURETRANSFORM_NAME) +
            " must contain numeric values.");
        extension.scale.y = ReadFloat(
            Internal::GetJsonArrayElement(
                *scale, 1U, "Scale value is missing"),
            "Scale member of " +
            std::string(TEXTURETRANSFORM_NAME) +
            " must contain numeric values.");
    }

    const auto* texCoord =
        Internal::FindJsonMember(value, "texCoord");
    if (texCoord != nullptr)
    {
        try
        {
            extension.texCoord = ReadSize(
                *texCoord,
                "TexCoord member of " +
                std::string(TEXTURETRANSFORM_NAME) +
                " must be an unsigned integer.");
        }
        catch (const InvalidGLTFException& exception)
        {
            throw GLTFException(exception.what());
        }
    }

    ParseProperty(
        value, extension, extensionDeserializer);
    return std::make_unique<TextureTransform>(extension);
}
