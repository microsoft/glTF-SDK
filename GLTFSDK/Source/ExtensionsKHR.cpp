// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/ExtensionsKHR.h>

#include <GLTFSDK/Document.h>
#include <GLTFSDK/RapidJsonUtils.h>

using namespace Microsoft::glTF;

namespace
{
    void ParseExtensions(const rapidjson::Value& v, glTFProperty& node, const ExtensionDeserializer& extensionDeserializer)
    {
        const auto& extensionsIt = v.FindMember("extensions");
        if (extensionsIt != v.MemberEnd())
        {
            const rapidjson::Value& extensionsObject = extensionsIt->value;
            for (const auto& entry : extensionsObject.GetObject())
            {
                ExtensionPair extensionPair = { entry.name.GetString(), Serialize(entry.value) };

                if (extensionDeserializer.HasHandler(extensionPair.name, node) ||
                    extensionDeserializer.HasHandler(extensionPair.name))
                {
                    node.SetExtension(extensionDeserializer.Deserialize(extensionPair, node));
                }
                else
                {
                    node.extensions.emplace(std::move(extensionPair.name), std::move(extensionPair.value));
                }
            }
        }
    }

    void ParseExtras(const rapidjson::Value& v, glTFProperty& node)
    {
        rapidjson::Value::ConstMemberIterator it;
        if (TryFindMember("extras", v, it))
        {
            const rapidjson::Value& a = it->value;
            node.extras = Serialize(a);
        }
    }

    void ParseProperty(const rapidjson::Value& v, glTFProperty& node, const ExtensionDeserializer& extensionDeserializer)
    {
        ParseExtensions(v, node, extensionDeserializer);
        ParseExtras(v, node);
    }

    void ParseTextureInfo(const rapidjson::Value& v, TextureInfo& textureInfo, const ExtensionDeserializer& extensionDeserializer)
    {
        auto textureIndexIt = FindRequiredMember("index", v);
        textureInfo.textureId = std::to_string(textureIndexIt->value.GetUint());
        textureInfo.texCoord = GetMemberValueOrDefault<size_t>(v, "texCoord", 0U);
        ParseProperty(v, textureInfo, extensionDeserializer);
    }

    void SerializePropertyExtensions(const Document& gltfDocument, const glTFProperty& property, rapidjson::Value& propertyValue, rapidjson::Document::AllocatorType& a, const ExtensionSerializer& extensionSerializer)
    {
        auto registeredExtensions = property.GetExtensions();

        if (!property.extensions.empty() || !registeredExtensions.empty())
        {
            rapidjson::Value& extensions = RapidJsonUtils::FindOrAddMember(propertyValue, "extensions", a);

            // Add registered extensions
            for (const auto& extension : registeredExtensions)
            {
                const auto extensionPair = extensionSerializer.Serialize(extension, property, gltfDocument);

                if (property.HasUnregisteredExtension(extensionPair.name))
                {
                    throw GLTFException("Registered extension '" + extensionPair.name + "' is also present as an unregistered extension.");
                }

                if (gltfDocument.extensionsUsed.find(extensionPair.name) == gltfDocument.extensionsUsed.end())
                {
                    throw GLTFException("Registered extension '" + extensionPair.name + "' is not present in extensionsUsed");
                }

                const auto d = RapidJsonUtils::CreateDocumentFromString(extensionPair.value);//TODO: validate the returned document against the extension schema!
                rapidjson::Value v(rapidjson::kObjectType);
                v.CopyFrom(d, a);
                extensions.AddMember(RapidJsonUtils::ToStringValue(extensionPair.name, a), v, a);
            }

            // Add unregistered extensions
            for (const auto& extension : property.extensions)
            {
                const auto d = RapidJsonUtils::CreateDocumentFromString(extension.second);
                rapidjson::Value v(rapidjson::kObjectType);
                v.CopyFrom(d, a);
                extensions.AddMember(RapidJsonUtils::ToStringValue(extension.first, a), v, a);
            }
        }
    }

    void SerializePropertyExtras(const glTFProperty& property, rapidjson::Value& propertyValue, rapidjson::Document::AllocatorType& a)
    {
        if (!property.extras.empty())
        {
            auto d = RapidJsonUtils::CreateDocumentFromString(property.extras);
            rapidjson::Value v(rapidjson::kObjectType);
            v.CopyFrom(d, a);
            propertyValue.AddMember("extras", v, a);
        }
    }

    void SerializeProperty(const Document& gltfDocument, const glTFProperty& property, rapidjson::Value& propertyValue, rapidjson::Document::AllocatorType& a, const ExtensionSerializer& extensionSerializer)
    {
        SerializePropertyExtensions(gltfDocument, property, propertyValue, a, extensionSerializer);
        SerializePropertyExtras(property, propertyValue, a);
    }

    void SerializeTextureInfo(const Document& gltfDocument, const TextureInfo& textureInfo, rapidjson::Value& textureValue, rapidjson::Document::AllocatorType& a, const IndexedContainer<const Texture>& textures, const ExtensionSerializer& extensionSerializer)
    {
        RapidJsonUtils::AddOptionalMemberIndex("index", textureValue, textureInfo.textureId, textures, a);
        if (textureInfo.texCoord != 0)
        {
            textureValue.AddMember("texCoord", ToKnownSizeType(textureInfo.texCoord), a);
        }
        SerializeProperty(gltfDocument, textureInfo, textureValue, a, extensionSerializer);
    }
}

ExtensionSerializer KHR::GetKHRExtensionSerializer()
{
    using namespace Materials;
    using namespace MeshPrimitives;
    using namespace TextureInfos;

    ExtensionSerializer extensionSerializer;
    extensionSerializer.AddHandler<PBRSpecularGlossiness, Material>(PBRSPECULARGLOSSINESS_NAME, SerializePBRSpecGloss);
    extensionSerializer.AddHandler<Unlit, Material>(UNLIT_NAME, SerializeUnlit);
    extensionSerializer.AddHandler<Clearcoat, Material>(CLEARCOAT_NAME, SerializeClearcoat);
    extensionSerializer.AddHandler<Volume, Material>(VOLUME_NAME, SerializeVolume);
    extensionSerializer.AddHandler<Iridescence, Material>(IRIDESCENCE_NAME, SerializeIridescence);
    extensionSerializer.AddHandler<DracoMeshCompression, MeshPrimitive>(DRACOMESHCOMPRESSION_NAME, SerializeDracoMeshCompression);
    extensionSerializer.AddHandler<MaterialsVariants, MeshPrimitive>(MATERIALSVARIANTS_NAME, SerializeMaterialsVariants);
    extensionSerializer.AddHandler<TextureTransform, TextureInfo>(TEXTURETRANSFORM_NAME, SerializeTextureTransform);
    extensionSerializer.AddHandler<TextureTransform, Material::NormalTextureInfo>(TEXTURETRANSFORM_NAME, SerializeTextureTransform);
    extensionSerializer.AddHandler<TextureTransform, Material::OcclusionTextureInfo>(TEXTURETRANSFORM_NAME, SerializeTextureTransform);
    return extensionSerializer;
}

ExtensionDeserializer KHR::GetKHRExtensionDeserializer()
{
    using namespace Materials;
    using namespace MeshPrimitives;
    using namespace TextureInfos;

    ExtensionDeserializer extensionDeserializer;
    extensionDeserializer.AddHandler<PBRSpecularGlossiness, Material>(PBRSPECULARGLOSSINESS_NAME, DeserializePBRSpecGloss);
    extensionDeserializer.AddHandler<Unlit, Material>(UNLIT_NAME, DeserializeUnlit);
    extensionDeserializer.AddHandler<Clearcoat, Material>(CLEARCOAT_NAME, DeserializeClearcoat);
    extensionDeserializer.AddHandler<Volume, Material>(VOLUME_NAME, DeserializeVolume);
    extensionDeserializer.AddHandler<Iridescence, Material>(IRIDESCENCE_NAME, DeserializeIridescence);
    extensionDeserializer.AddHandler<DracoMeshCompression, MeshPrimitive>(DRACOMESHCOMPRESSION_NAME, DeserializeDracoMeshCompression);
    extensionDeserializer.AddHandler<MaterialsVariants, MeshPrimitive>(MATERIALSVARIANTS_NAME, DeserializeMaterialsVariants);
    extensionDeserializer.AddHandler<TextureTransform, TextureInfo>(TEXTURETRANSFORM_NAME, DeserializeTextureTransform);
    extensionDeserializer.AddHandler<TextureTransform, Material::NormalTextureInfo>(TEXTURETRANSFORM_NAME, DeserializeTextureTransform);
    extensionDeserializer.AddHandler<TextureTransform, Material::OcclusionTextureInfo>(TEXTURETRANSFORM_NAME, DeserializeTextureTransform);
    return extensionDeserializer;
}

// KHR::Materials::PBRSpecularGlossiness

KHR::Materials::PBRSpecularGlossiness::PBRSpecularGlossiness() :
    diffuseFactor(1.0f, 1.0f, 1.0f, 1.0f),
    specularFactor(1.0f, 1.0f, 1.0f),
    glossinessFactor(1.0f)
{
}

std::unique_ptr<Extension> KHR::Materials::PBRSpecularGlossiness::Clone() const
{
    return std::make_unique<PBRSpecularGlossiness>(*this);
}

bool KHR::Materials::PBRSpecularGlossiness::IsEqual(const Extension& rhs) const
{
    const auto other = dynamic_cast<const PBRSpecularGlossiness*>(&rhs);

    return other != nullptr
        && glTFProperty::Equals(*this, *other)
        && this->diffuseFactor == other->diffuseFactor
        && this->diffuseTexture == other->diffuseTexture
        && this->specularFactor == other->specularFactor
        && this->glossinessFactor == other->glossinessFactor
        && this->specularGlossinessTexture == other->specularGlossinessTexture;
}

std::string KHR::Materials::SerializePBRSpecGloss(const Materials::PBRSpecularGlossiness& specGloss, const Document& gltfDocument, const ExtensionSerializer& extensionSerializer)
{
    rapidjson::Document doc;
    auto& a = doc.GetAllocator();
    rapidjson::Value KHR_pbrSpecularGlossiness(rapidjson::kObjectType);
    {
        if (specGloss.diffuseFactor != Color4(1.0f, 1.0f, 1.0f, 1.0f))
        {
            KHR_pbrSpecularGlossiness.AddMember("diffuseFactor", RapidJsonUtils::ToJsonArray(specGloss.diffuseFactor, a), a);
        }

        if (!specGloss.diffuseTexture.textureId.empty())
        {
            rapidjson::Value diffuseTexture(rapidjson::kObjectType);
            SerializeTextureInfo(gltfDocument, specGloss.diffuseTexture, diffuseTexture, a, gltfDocument.textures, extensionSerializer);
            KHR_pbrSpecularGlossiness.AddMember("diffuseTexture", diffuseTexture, a);
        }

        if (specGloss.specularFactor != Color3(1.0f, 1.0f, 1.0f))
        {
            KHR_pbrSpecularGlossiness.AddMember("specularFactor", RapidJsonUtils::ToJsonArray(specGloss.specularFactor, a), a);
        }

        if (specGloss.glossinessFactor != 1.0)
        {
            KHR_pbrSpecularGlossiness.AddMember("glossinessFactor", specGloss.glossinessFactor, a);
        }

        if (!specGloss.specularGlossinessTexture.textureId.empty())
        {
            rapidjson::Value specularGlossinessTexture(rapidjson::kObjectType);
            SerializeTextureInfo(gltfDocument, specGloss.specularGlossinessTexture, specularGlossinessTexture, a, gltfDocument.textures, extensionSerializer);
            KHR_pbrSpecularGlossiness.AddMember("specularGlossinessTexture", specularGlossinessTexture, a);
        }

        SerializeProperty(gltfDocument, specGloss, KHR_pbrSpecularGlossiness, a, extensionSerializer);
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    KHR_pbrSpecularGlossiness.Accept(writer);

    return buffer.GetString();
}

std::unique_ptr<Extension> KHR::Materials::DeserializePBRSpecGloss(const std::string& json, const ExtensionDeserializer& extensionDeserializer)
{
    Materials::PBRSpecularGlossiness specGloss;

    auto doc = RapidJsonUtils::CreateDocumentFromString(json);
    const rapidjson::Value sit = doc.GetObject();

    // Diffuse Factor
    auto diffuseFactIt = sit.FindMember("diffuseFactor");
    if (diffuseFactIt != sit.MemberEnd())
    {
        std::vector<float> diffuseFactor;
        for (rapidjson::Value::ConstValueIterator ait = diffuseFactIt->value.Begin(); ait != diffuseFactIt->value.End(); ++ait)
        {
            diffuseFactor.push_back(static_cast<float>(ait->GetDouble()));
        }
        specGloss.diffuseFactor = Color4(diffuseFactor[0], diffuseFactor[1], diffuseFactor[2], diffuseFactor[3]);
    }

    // Diffuse Texture
    const auto diffuseTextureIt = sit.FindMember("diffuseTexture");
    if (diffuseTextureIt != sit.MemberEnd())
    {
        ParseTextureInfo(diffuseTextureIt->value, specGloss.diffuseTexture, extensionDeserializer);
    }

    // Specular Factor
    auto specularFactIt = sit.FindMember("specularFactor");
    if (specularFactIt != sit.MemberEnd())
    {
        std::vector<float> specularFactor;
        for (rapidjson::Value::ConstValueIterator ait = specularFactIt->value.Begin(); ait != specularFactIt->value.End(); ++ait)
        {
            specularFactor.push_back(static_cast<float>(ait->GetDouble()));
        }
        specGloss.specularFactor = Color3(specularFactor[0], specularFactor[1], specularFactor[2]);
    }

    // Glossiness Factor
    specGloss.glossinessFactor = GetMemberValueOrDefault<float>(sit, "glossinessFactor", 1.0f);

    // SpecularGlossinessTexture
    const auto specularGlossinessTextureIt = sit.FindMember("specularGlossinessTexture");
    if (specularGlossinessTextureIt != sit.MemberEnd())
    {
        ParseTextureInfo(specularGlossinessTextureIt->value, specGloss.specularGlossinessTexture, extensionDeserializer);
    }

    ParseProperty(sit, specGloss, extensionDeserializer);

    return std::make_unique<PBRSpecularGlossiness>(specGloss);
}

// KHR::Materials::Unlit

std::unique_ptr<Extension> KHR::Materials::Unlit::Clone() const
{
    return std::make_unique<Unlit>(*this);
}

bool KHR::Materials::Unlit::IsEqual(const Extension& rhs) const
{
    return dynamic_cast<const Unlit*>(&rhs) != nullptr;
}

std::string KHR::Materials::SerializeUnlit(const Materials::Unlit& extension, const Document& gltfDocument, const ExtensionSerializer& extensionSerializer)
{
    rapidjson::Document doc;
    auto& a = doc.GetAllocator();
    rapidjson::Value unlitValue(rapidjson::kObjectType);

    SerializeProperty(gltfDocument, extension, unlitValue, a, extensionSerializer);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    unlitValue.Accept(writer);

    return buffer.GetString();
}

std::unique_ptr<Extension> KHR::Materials::DeserializeUnlit(const std::string& json, const ExtensionDeserializer& extensionDeserializer)
{
    Unlit unlit;

    auto doc = RapidJsonUtils::CreateDocumentFromString(json);
    const rapidjson::Value objValue = doc.GetObject();

    ParseProperty(objValue, unlit, extensionDeserializer);

    return std::make_unique<Unlit>(unlit);
}

// KHR::Materials::Clearcoat

KHR::Materials::Clearcoat::Clearcoat() :
    factor(0.0f),
    roughnessFactor(0.0f)
{
}

std::unique_ptr<Extension> KHR::Materials::Clearcoat::Clone() const
{
    return std::make_unique<Clearcoat>(*this);
}

bool KHR::Materials::Clearcoat::IsEqual(const Extension& rhs) const
{
    const auto other = dynamic_cast<const Clearcoat*>(&rhs);

    return other != nullptr
        && glTFProperty::Equals(*this, *other)
        && this->factor == other->factor
        && this->texture == other->texture
        && this->roughnessFactor == other->roughnessFactor
        && this->roughnessTexture == other->roughnessTexture
        && this->normalTexture == other->normalTexture;
}

std::string KHR::Materials::SerializeClearcoat(const Materials::Clearcoat& clearcoat, const Document& gltfDocument, const ExtensionSerializer& extensionSerializer)
{
    rapidjson::Document doc;
    auto& a = doc.GetAllocator();
    rapidjson::Value KHR_clearcoat(rapidjson::kObjectType);
    {
        if (clearcoat.factor != 0.0f)
        {
            KHR_clearcoat.AddMember("clearcoatFactor", RapidJsonUtils::ToFloatValue(clearcoat.factor), a);
        }

        if (!clearcoat.texture.textureId.empty())
        {
            rapidjson::Value texture(rapidjson::kObjectType);
            SerializeTextureInfo(gltfDocument, clearcoat.texture, texture, a, gltfDocument.textures, extensionSerializer);
            KHR_clearcoat.AddMember("clearcoatTexture", texture, a);
        }

        if (clearcoat.roughnessFactor != 0.0f)
        {
            KHR_clearcoat.AddMember("clearcoatRoughnessFactor", RapidJsonUtils::ToFloatValue(clearcoat.roughnessFactor), a);
        }

        if (!clearcoat.roughnessTexture.textureId.empty())
        {
            rapidjson::Value roughnessTexture(rapidjson::kObjectType);
            SerializeTextureInfo(gltfDocument, clearcoat.roughnessTexture, roughnessTexture, a, gltfDocument.textures, extensionSerializer);
            KHR_clearcoat.AddMember("clearcoatRoughnessTexture", roughnessTexture, a);
        }

        if (!clearcoat.normalTexture.textureId.empty())
        {
            rapidjson::Value normalTexture(rapidjson::kObjectType);
            SerializeTextureInfo(gltfDocument, clearcoat.normalTexture, normalTexture, a, gltfDocument.textures, extensionSerializer);
            KHR_clearcoat.AddMember("clearcoatNormalTexture", normalTexture, a);
        }

        SerializeProperty(gltfDocument, clearcoat, KHR_clearcoat, a, extensionSerializer);
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    KHR_clearcoat.Accept(writer);

    return buffer.GetString();
}

std::unique_ptr<Extension> KHR::Materials::DeserializeClearcoat(const std::string& json, const ExtensionDeserializer& extensionDeserializer)
{
    Materials::Clearcoat clearcoat;

    auto doc = RapidJsonUtils::CreateDocumentFromString(json);
    const auto sit = doc.GetObject();

    // Clearcoat Factor
    const auto factorIt = sit.FindMember("clearcoatFactor");
    clearcoat.factor = factorIt == sit.MemberEnd() ? 0.0f : factorIt->value.GetFloat();

    // Clearcoat Texture
    const auto textureIt = sit.FindMember("clearcoatTexture");
    if (textureIt != sit.MemberEnd())
    {
        ParseTextureInfo(textureIt->value, clearcoat.texture, extensionDeserializer);
    }

    // Clearcoat Roughness Factor
    const auto roughnessFactorIt = sit.FindMember("clearcoatRoughnessFactor");
    clearcoat.roughnessFactor = roughnessFactorIt == sit.MemberEnd() ? 0.0f : roughnessFactorIt->value.GetFloat();

    // Clearcoat Roughness Texture
    const auto roughnessTextureIt = sit.FindMember("clearcoatRoughnessTexture");
    if (roughnessTextureIt != sit.MemberEnd())
    {
        ParseTextureInfo(roughnessTextureIt->value, clearcoat.roughnessTexture, extensionDeserializer);
    }

    // Clearcoat Normal Texture
    const auto normalTextureIt = sit.FindMember("clearcoatNormalTexture");
    if (normalTextureIt != sit.MemberEnd())
    {
        ParseTextureInfo(normalTextureIt->value, clearcoat.normalTexture, extensionDeserializer);
    }

    ParseProperty(sit, clearcoat, extensionDeserializer);

    return std::make_unique<Clearcoat>(clearcoat);
}

// KHR::Materials::Volume

KHR::Materials::Volume::Volume() :
    attenuationColor(1.0f, 1.0f, 1.0f),
    attenuationDistance(std::numeric_limits<float>::infinity()),
    thicknessFactor(0.0f)
{
}

std::unique_ptr<Extension> KHR::Materials::Volume::Clone() const
{
    return std::make_unique<Volume>(*this);
}

bool KHR::Materials::Volume::IsEqual(const Extension& rhs) const
{
    const auto other = dynamic_cast<const Volume*>(&rhs);

    return other != nullptr
        && glTFProperty::Equals(*this, *other)
        && this->attenuationColor == other->attenuationColor
        && this->attenuationDistance == other->attenuationDistance
        && this->thicknessFactor == other->thicknessFactor
        && this->thicknessTexture == other->thicknessTexture;
}

std::string KHR::Materials::SerializeVolume(const Materials::Volume& volume, const Document& gltfDocument, const ExtensionSerializer& extensionSerializer)
{
    rapidjson::Document doc;
    auto& a = doc.GetAllocator();
    rapidjson::Value KHR_volume(rapidjson::kObjectType);
    {
        if (volume.attenuationColor != Color3(1.0f, 1.0f, 1.0f))
        {
            KHR_volume.AddMember("attenuationColor", RapidJsonUtils::ToJsonArray(volume.attenuationColor, a), a);
        }

        if (volume.attenuationDistance != std::numeric_limits<float>::infinity())
        {
            KHR_volume.AddMember("attenuationDistance", RapidJsonUtils::ToFloatValue(volume.attenuationDistance), a);
        }

        if (volume.thicknessFactor != 0.0f)
        {
            KHR_volume.AddMember("thicknessFactor", RapidJsonUtils::ToFloatValue(volume.thicknessFactor), a);
        }

        if (!volume.thicknessTexture.textureId.empty())
        {
            rapidjson::Value thicknessTexture(rapidjson::kObjectType);
            SerializeTextureInfo(gltfDocument, volume.thicknessTexture, thicknessTexture, a, gltfDocument.textures, extensionSerializer);
            KHR_volume.AddMember("thicknessTexture", thicknessTexture, a);
        }

        SerializeProperty(gltfDocument, volume, KHR_volume, a, extensionSerializer);
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    KHR_volume.Accept(writer);

    return buffer.GetString();
}

std::unique_ptr<Extension> KHR::Materials::DeserializeVolume(const std::string& json, const ExtensionDeserializer& extensionDeserializer)
{
    Materials::Volume volume;

    auto doc = RapidJsonUtils::CreateDocumentFromString(json);
    const auto sit = doc.GetObject();

    // Attenuation Color
    const auto attenuationColorIt = sit.FindMember("attenuationColor");
    if (attenuationColorIt != sit.MemberEnd())
    {
        std::vector<float> attenuationColor;
        for (rapidjson::Value::ConstValueIterator ait = attenuationColorIt->value.Begin(); ait != attenuationColorIt->value.End(); ++ait)
        {
            attenuationColor.push_back(static_cast<float>(ait->GetDouble()));
        }
        volume.attenuationColor = Color3(attenuationColor[0], attenuationColor[1], attenuationColor[2]);
    }

    // Attenuation Distance
    const auto attenuationDistanceIt = sit.FindMember("attenuationDistance");
    if (attenuationDistanceIt != sit.MemberEnd())
    {
        volume.attenuationDistance = attenuationDistanceIt->value.GetFloat();
    }

    // Thickness Factor
    const auto thicknessFactorIt = sit.FindMember("thicknessFactor");
    if (thicknessFactorIt != sit.MemberEnd())
    {
        volume.thicknessFactor = thicknessFactorIt->value.GetFloat();
    }

    // Thickness Texture
    const auto thicknessTextureIt = sit.FindMember("thicknessTexture");
    if (thicknessTextureIt != sit.MemberEnd())
    {
        ParseTextureInfo(thicknessTextureIt->value, volume.thicknessTexture, extensionDeserializer);
    }

    ParseProperty(sit, volume, extensionDeserializer);

    return std::make_unique<Volume>(volume);
}

// KHR::Materials::Iridescence

KHR::Materials::Iridescence::Iridescence() :
    factor(0.0f),
    ior(1.3f),
    thicknessMin(100.0f),
    thicknessMax(400.0f)
{
}

std::unique_ptr<Extension> KHR::Materials::Iridescence::Clone() const
{
    return std::make_unique<Iridescence>(*this);
}

bool KHR::Materials::Iridescence::IsEqual(const Extension& rhs) const
{
    const auto other = dynamic_cast<const Iridescence*>(&rhs);

    return other != nullptr
        && glTFProperty::Equals(*this, *other)
        && this->factor == other->factor
        && this->texture == other->texture
        && this->ior == other->ior
        && this->thicknessMin == other->thicknessMin
        && this->thicknessMax == other->thicknessMax
        && this->thicknessTexture == other->thicknessTexture;
}

std::string KHR::Materials::SerializeIridescence(const Materials::Iridescence& iridescence, const Document& gltfDocument, const ExtensionSerializer& extensionSerializer)
{
    rapidjson::Document doc;
    auto& a = doc.GetAllocator();
    rapidjson::Value KHR_iridescence(rapidjson::kObjectType);
    {
        if (iridescence.factor != 0.0f)
        {
            KHR_iridescence.AddMember("iridescenceFactor", RapidJsonUtils::ToFloatValue(iridescence.factor), a);
        }

        if (!iridescence.texture.textureId.empty())
        {
            rapidjson::Value texture(rapidjson::kObjectType);
            SerializeTextureInfo(gltfDocument, iridescence.texture, texture, a, gltfDocument.textures, extensionSerializer);
            KHR_iridescence.AddMember("iridescenceTexture", texture, a);
        }

        if (iridescence.ior != 1.3f)
        {
            KHR_iridescence.AddMember("iridescenceIor", RapidJsonUtils::ToFloatValue(iridescence.ior), a);
        }

        if (iridescence.thicknessMin != 100.0f)
        {
            KHR_iridescence.AddMember("iridescenceThicknessMinimum", RapidJsonUtils::ToFloatValue(iridescence.thicknessMin), a);
        }

        if (iridescence.thicknessMax != 400.0f)
        {
            KHR_iridescence.AddMember("iridescenceThicknessMaximum", RapidJsonUtils::ToFloatValue(iridescence.thicknessMax), a);
        }

        if (!iridescence.thicknessTexture.textureId.empty())
        {
            rapidjson::Value thicknessTexture(rapidjson::kObjectType);
            SerializeTextureInfo(gltfDocument, iridescence.thicknessTexture, thicknessTexture, a, gltfDocument.textures, extensionSerializer);
            KHR_iridescence.AddMember("iridescenceThicknessTexture", thicknessTexture, a);
        }

        SerializeProperty(gltfDocument, iridescence, KHR_iridescence, a, extensionSerializer);
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    KHR_iridescence.Accept(writer);

    return buffer.GetString();
}

std::unique_ptr<Extension> KHR::Materials::DeserializeIridescence(const std::string& json, const ExtensionDeserializer& extensionDeserializer)
{
    Materials::Iridescence iridescence;

    auto doc = RapidJsonUtils::CreateDocumentFromString(json);
    const auto sit = doc.GetObject();

    // Iridescence Factor
    const auto factorIt = sit.FindMember("iridescenceFactor");
    if (factorIt != sit.MemberEnd())
    {
        iridescence.factor = factorIt->value.GetFloat();
    }

    // Iridescence Texture
    const auto textureIt = sit.FindMember("iridescenceTexture");
    if (textureIt != sit.MemberEnd())
    {
        ParseTextureInfo(textureIt->value, iridescence.texture, extensionDeserializer);
    }

    // IOR
    const auto iorIt = sit.FindMember("iridescenceIor");
    if (iorIt != sit.MemberEnd())
    {
        iridescence.ior = iorIt->value.GetFloat();
    }

    // Thickness Minimum
    const auto thicknessMinIt = sit.FindMember("iridescenceThicknessMinimum");
    if (thicknessMinIt != sit.MemberEnd())
    {
        iridescence.thicknessMin = thicknessMinIt->value.GetFloat();
    }

    // Thickness Maximum
    const auto thicknessMaxIt = sit.FindMember("iridescenceThicknessMaximum");
    if (thicknessMaxIt != sit.MemberEnd())
    {
        iridescence.thicknessMax = thicknessMaxIt->value.GetFloat();
    }

    // Thickness Texture
    const auto thicknessTextureIt = sit.FindMember("iridescenceThicknessTexture");
    if (thicknessTextureIt != sit.MemberEnd())
    {
        ParseTextureInfo(thicknessTextureIt->value, iridescence.thicknessTexture, extensionDeserializer);
    }

    ParseProperty(sit, iridescence, extensionDeserializer);

    return std::make_unique<Iridescence>(iridescence);
}

// KHR::MeshPrimitives::DracoMeshCompression

std::unique_ptr<Extension> KHR::MeshPrimitives::DracoMeshCompression::Clone() const
{
    return std::make_unique<DracoMeshCompression>(*this);
}

bool KHR::MeshPrimitives::DracoMeshCompression::IsEqual(const Extension& rhs) const
{
    const auto other = dynamic_cast<const DracoMeshCompression*>(&rhs);

    return other != nullptr
        && glTFProperty::Equals(*this, *other)
        && this->bufferViewId == other->bufferViewId
        && this->attributes == other->attributes;
}

std::string KHR::MeshPrimitives::SerializeDracoMeshCompression(const MeshPrimitives::DracoMeshCompression& dracoMeshCompression, const Document& glTFdoc, const ExtensionSerializer& extensionSerializer)
{
    rapidjson::Document doc;
    auto& a = doc.GetAllocator();
    rapidjson::Value KHR_draco_mesh_compression(rapidjson::kObjectType);
    {
        if (!dracoMeshCompression.bufferViewId.empty())
        {
            RapidJsonUtils::AddOptionalMemberIndex("bufferView", KHR_draco_mesh_compression, dracoMeshCompression.bufferViewId, glTFdoc.bufferViews, a);
        }

        rapidjson::Value attributesValue(rapidjson::kObjectType);

        for (const auto& attribute : dracoMeshCompression.attributes)
        {
            rapidjson::Value attributeValue;
            attributeValue.SetUint(attribute.second);
            attributesValue.AddMember(RapidJsonUtils::ToStringValue(attribute.first, a), attributeValue, a);
        }

        KHR_draco_mesh_compression.AddMember(RapidJsonUtils::ToStringValue("attributes", a), attributesValue, a);

        SerializeProperty(glTFdoc, dracoMeshCompression, KHR_draco_mesh_compression, a, extensionSerializer);
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    KHR_draco_mesh_compression.Accept(writer);

    return buffer.GetString();
}

std::unique_ptr<Extension> KHR::MeshPrimitives::DeserializeDracoMeshCompression(const std::string& json, const ExtensionDeserializer& extensionDeserializer)
{
    auto extension = std::make_unique<DracoMeshCompression>();

    auto doc = RapidJsonUtils::CreateDocumentFromString(json);
    const rapidjson::Value v = doc.GetObject();

    extension->bufferViewId = GetMemberValueAsString<uint32_t>(v, "bufferView");

    rapidjson::Value::ConstMemberIterator it = v.FindMember("attributes");
    if (it != v.MemberEnd())
    {
        if (!it->value.IsObject())
        {
            throw GLTFException("Member attributes of " + std::string(DRACOMESHCOMPRESSION_NAME) + " is not an object.");
        }
        const auto& attributes = it->value.GetObject();

        for (const auto& attribute : attributes)
        {
            auto name = attribute.name.GetString();

            if (!attribute.value.IsInt())
            {
                throw GLTFException("Attribute " + std::string(name) + " of " + std::string(DRACOMESHCOMPRESSION_NAME) + " is not a number.");
            }
            extension->attributes.emplace(name, attribute.value.Get<uint32_t>());
        }
    }

    ParseProperty(v, *extension, extensionDeserializer);

    return extension;
}

// KHR::MeshPrimitives::MaterialsVariants

std::unique_ptr<Extension> KHR::MeshPrimitives::MaterialsVariants::Clone() const
{
    return std::make_unique<MaterialsVariants>(*this);
}

bool KHR::MeshPrimitives::MaterialsVariants::IsEqual(const Extension& rhs) const
{
    const auto other = dynamic_cast<const MaterialsVariants*>(&rhs);

    return other != nullptr
        && glTFProperty::Equals(*this, *other);
        //&& this->mappings == other->mappings;
}

std::string KHR::MeshPrimitives::SerializeMaterialsVariants(const MeshPrimitives::MaterialsVariants& materialsVariants, const Document& glTFdoc, const ExtensionSerializer& extensionSerializer)
{
    rapidjson::Document doc;
    auto& a = doc.GetAllocator();
    rapidjson::Value KHR_materials_variants(rapidjson::kObjectType);
    {
        if (!materialsVariants.mappings.empty())
        {
        }

        SerializeProperty(glTFdoc, materialsVariants, KHR_materials_variants, a, extensionSerializer);
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    KHR_materials_variants.Accept(writer);

    return buffer.GetString();
}

std::unique_ptr<Extension> KHR::MeshPrimitives::DeserializeMaterialsVariants(const std::string& json, const ExtensionDeserializer& extensionDeserializer)
{
    auto extension = std::make_unique<MaterialsVariants>();

    auto doc = RapidJsonUtils::CreateDocumentFromString(json);
    const auto v = doc.GetObject();

    const auto itMappings = v.FindMember("mappings");
    if (itMappings != v.MemberEnd())
    {
        if (!itMappings->value.IsArray())
        {
            throw GLTFException("Member mappings of " + std::string(MATERIALSVARIANTS_NAME) + " is not an array.");
        }
        const auto& mappings = itMappings->value.GetArray();

        auto i = 0;
        for (const auto& mapping : mappings)
        {
            if (!mapping.IsObject())
            {
                throw GLTFException("Member mapping " + std::to_string(i) + " of " + std::string(MATERIALSVARIANTS_NAME) + " is not an object.");
            }

            MaterialsVariants::Mapping extensionMapping;

            const auto itMaterial = mapping.FindMember("material");
            if (itMaterial != mapping.MemberEnd())
            {
                if (!itMaterial->value.IsUint())
                {
                    throw GLTFException("Member mapping " + std::to_string(i) + " material of " + std::string(MATERIALSVARIANTS_NAME) + " is not an integer.");
                }

                extensionMapping.materialId = std::to_string(itMaterial->value.GetUint());
            }

            extension->mappings.push_back(extensionMapping);

            i++;
        }
    }

    ParseProperty(v, *extension, extensionDeserializer);

    return extension;
}

// KHR::TextureInfos::TextureTransform

KHR::TextureInfos::TextureTransform::TextureTransform() :
    offset(Vector2::ZERO),
    rotation(0.0f),
    scale(Vector2::ONE),
    texCoord()
{
}

KHR::TextureInfos::TextureTransform::TextureTransform(const TextureTransform& textureTransform) :
    offset(textureTransform.offset),
    rotation(textureTransform.rotation),
    scale(textureTransform.scale),
    texCoord(textureTransform.texCoord)
{
}

std::unique_ptr<Extension> KHR::TextureInfos::TextureTransform::Clone() const
{
    return std::make_unique<TextureTransform>(*this);
}

bool KHR::TextureInfos::TextureTransform::IsEqual(const Extension& rhs) const
{
    const auto other = dynamic_cast<const TextureTransform*>(&rhs);

    return other != nullptr
        && glTFProperty::Equals(*this, *other)
        && this->offset == other->offset
        && this->rotation == other->rotation
        && this->scale == other->scale
        && this->texCoord == other->texCoord;
}

std::string KHR::TextureInfos::SerializeTextureTransform(const TextureTransform& textureTransform, const Document& gltfDocument, const ExtensionSerializer& extensionSerializer)
{
    rapidjson::Document doc;
    auto& a = doc.GetAllocator();
    rapidjson::Value KHR_textureTransform(rapidjson::kObjectType);
    {
        if (textureTransform.offset != Vector2::ZERO)
        {
            KHR_textureTransform.AddMember("offset", RapidJsonUtils::ToJsonArray(textureTransform.offset, a), a);
        }

        if (textureTransform.rotation != 0.0f)
        {
            KHR_textureTransform.AddMember("rotation", textureTransform.rotation, a);
        }

        if (textureTransform.scale != Vector2::ONE)
        {
            KHR_textureTransform.AddMember("scale", RapidJsonUtils::ToJsonArray(textureTransform.scale, a), a);
        }

        if (textureTransform.texCoord)
        {
            KHR_textureTransform.AddMember("texCoord", ToKnownSizeType(textureTransform.texCoord.Get()), a);
        }

        SerializeProperty(gltfDocument, textureTransform, KHR_textureTransform, a, extensionSerializer);
    }

    glTF::rapidjson::StringBuffer buffer;
    glTF::rapidjson::Writer<glTF::rapidjson::StringBuffer> writer(buffer);
    KHR_textureTransform.Accept(writer);

    return buffer.GetString();
}

std::unique_ptr<Extension> KHR::TextureInfos::DeserializeTextureTransform(const std::string& json, const ExtensionDeserializer& extensionDeserializer)
{
    TextureTransform textureTransform;

    auto doc = RapidJsonUtils::CreateDocumentFromString(json);
    const rapidjson::Value sit = doc.GetObject();

    // Offset
    auto offsetIt = sit.FindMember("offset");
    if (offsetIt != sit.MemberEnd())
    {
        if (offsetIt->value.Size() != 2)
        {
            throw GLTFException("Offset member of " + std::string(TEXTURETRANSFORM_NAME) + " must have two values.");
        }

        std::vector<float> offset;
        for (rapidjson::Value::ConstValueIterator ait = offsetIt->value.Begin(); ait != offsetIt->value.End(); ++ait)
        {
            offset.push_back(static_cast<float>(ait->GetDouble()));
        }
        textureTransform.offset.x = offset[0];
        textureTransform.offset.y = offset[1];
    }

    // Rotation
    textureTransform.rotation = GetMemberValueOrDefault<float>(sit, "rotation", 0.0f);

    // Scale
    auto scaleIt = sit.FindMember("scale");
    if (scaleIt != sit.MemberEnd())
    {
        if (scaleIt->value.Size() != 2)
        {
            throw GLTFException("Scale member of " + std::string(TEXTURETRANSFORM_NAME) + " must have two values.");
        }

        std::vector<float> scale;
        for (rapidjson::Value::ConstValueIterator ait = scaleIt->value.Begin(); ait != scaleIt->value.End(); ++ait)
        {
            scale.push_back(static_cast<float>(ait->GetDouble()));
        }
        textureTransform.scale.x = scale[0];
        textureTransform.scale.y = scale[1];
    }

    // TexCoord
    auto texCoordIt = sit.FindMember("texCoord");
    if (texCoordIt != sit.MemberEnd())
    {
        textureTransform.texCoord = static_cast<size_t>(texCoordIt->value.GetUint());
    }

    ParseProperty(sit, textureTransform, extensionDeserializer);

    return std::make_unique<TextureTransform>(textureTransform);
}
