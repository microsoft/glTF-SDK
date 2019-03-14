// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/ExtensionsKHR.h>

#include <GLTFSDK/Document.h>
#include <GLTFSDK/RapidJsonUtils.h>

using namespace Microsoft::glTF;

namespace
{
    void ParseExtensions(const rapidjson::Value& v, glTFProperty& node)
    {
        const auto& extensionsIt = v.FindMember("extensions");
        if (extensionsIt != v.MemberEnd())
        {
            const rapidjson::Value& extensionsObject = extensionsIt->value;
            for (const auto& entry : extensionsObject.GetObject())
            {
                const std::string extensionName = entry.name.GetString();
                const std::string extensionValue = Serialize(entry.value);
                node.extensions.emplace(extensionName, extensionValue);
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

    void ParseProperty(const rapidjson::Value& v, glTFProperty& node)
    {
        ParseExtensions(v, node);
        ParseExtras(v, node);
    }

    void ParseTextureInfo(const rapidjson::Value& v, TextureInfo& textureInfo)
    {
        auto textureIndexIt = FindRequiredMember("index", v);
        textureInfo.textureId = std::to_string(textureIndexIt->value.GetUint());
        textureInfo.texCoord = GetMemberValueOrDefault<size_t>(v, "texCoord", 0U);
        ParseProperty(v, textureInfo);
    }

    void SerializePropertyExtensions(const glTFProperty& property, rapidjson::Value& propertyValue, rapidjson::Document::AllocatorType& a)
    {
        auto registeredExtensions = property.GetExtensions();

        if (!property.extensions.empty() || !registeredExtensions.empty())
        {
            rapidjson::Value& extensions = RapidJsonUtils::FindOrAddMember(propertyValue, "extensions", a);

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

    void SerializeProperty(const glTFProperty& property, rapidjson::Value& propertyValue, rapidjson::Document::AllocatorType& a)
    {
        SerializePropertyExtensions(property, propertyValue, a);
        SerializePropertyExtras(property, propertyValue, a);
    }

    void SerializeTextureInfo(const TextureInfo& textureInfo, rapidjson::Value& textureValue, rapidjson::Document::AllocatorType& a, const IndexedContainer<const Texture>& textures)
    {
        RapidJsonUtils::AddOptionalMemberIndex("index", textureValue, textureInfo.textureId, textures, a);
        if (textureInfo.texCoord != 0)
        {
            textureValue.AddMember("texCoord", ToKnownSizeType(textureInfo.texCoord), a);
        }
        SerializeProperty(textureInfo, textureValue, a);
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
    extensionSerializer.AddHandler<DracoMeshCompression, MeshPrimitive>(DRACOMESHCOMPRESSION_NAME, SerializeDracoMeshCompression);
    extensionSerializer.AddHandler<TextureTransform, TextureInfo>(TEXTURETRANSFORM_NAME, SerializeTextureTransform);
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
    extensionDeserializer.AddHandler<DracoMeshCompression, MeshPrimitive>(DRACOMESHCOMPRESSION_NAME, DeserializeDracoMeshCompression);
    extensionDeserializer.AddHandler<TextureTransform, TextureInfo>(TEXTURETRANSFORM_NAME, DeserializeTextureTransform);
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

std::string KHR::Materials::SerializePBRSpecGloss(const Materials::PBRSpecularGlossiness& specGloss, const Document& gltfDocument)
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
            SerializeTextureInfo(specGloss.diffuseTexture, diffuseTexture, a, gltfDocument.textures);
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
            SerializeTextureInfo(specGloss.specularGlossinessTexture, specularGlossinessTexture, a, gltfDocument.textures);
            KHR_pbrSpecularGlossiness.AddMember("specularGlossinessTexture", specularGlossinessTexture, a);
        }

        SerializeProperty(specGloss, KHR_pbrSpecularGlossiness, a);
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    KHR_pbrSpecularGlossiness.Accept(writer);

    return buffer.GetString();
}

std::unique_ptr<Extension> KHR::Materials::DeserializePBRSpecGloss(const std::string& json)
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
        ParseTextureInfo(diffuseTextureIt->value, specGloss.diffuseTexture);
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
        ParseTextureInfo(specularGlossinessTextureIt->value, specGloss.specularGlossinessTexture);
    }

    ParseProperty(sit, specGloss);

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

std::string KHR::Materials::SerializeUnlit(const Materials::Unlit& extension, const Document&)
{
    rapidjson::Document doc;
    auto& a = doc.GetAllocator();
    rapidjson::Value unlitValue(rapidjson::kObjectType);

    SerializeProperty(extension, unlitValue, a);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    unlitValue.Accept(writer);

    return buffer.GetString();
}

std::unique_ptr<Extension> KHR::Materials::DeserializeUnlit(const std::string& json)
{
    Unlit unlit;

    auto doc = RapidJsonUtils::CreateDocumentFromString(json);
    const rapidjson::Value objValue = doc.GetObject();

    ParseProperty(objValue, unlit);

    return std::make_unique<Unlit>(unlit);
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

std::string KHR::MeshPrimitives::SerializeDracoMeshCompression(const MeshPrimitives::DracoMeshCompression& dracoMeshCompression, const Document& glTFdoc)
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

        SerializeProperty(dracoMeshCompression, KHR_draco_mesh_compression, a);
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    KHR_draco_mesh_compression.Accept(writer);

    return buffer.GetString();
}

std::unique_ptr<Extension> KHR::MeshPrimitives::DeserializeDracoMeshCompression(const std::string& json)
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

    ParseProperty(v, *extension);

    return extension;
}

// KHR::TextureInfos::TextureTransform

KHR::TextureInfos::TextureTransform::TextureTransform() :
    offset(0.0f, 0.0f),
    rotation(0.0f),
    scale(1.0f, 1.0f),
    texCoord(0)
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

std::string KHR::TextureInfos::SerializeTextureTransform(const TextureTransform& textureTransform, const Document& /*gltfDocument*/)
{
    rapidjson::Document doc;
    auto& a = doc.GetAllocator();
    rapidjson::Value KHR_textureTransform(rapidjson::kObjectType);
    {
        if (textureTransform.offset != Vector2(0.0f, 0.0f))
        {
            KHR_textureTransform.AddMember("offset", RapidJsonUtils::ToJsonArray(textureTransform.offset, a), a);
        }

        if (textureTransform.rotation != 0.0f)
        {
            KHR_textureTransform.AddMember("rotation", textureTransform.rotation, a);
        }

        if (textureTransform.scale != Vector2(1.0f, 1.0f))
        {
            KHR_textureTransform.AddMember("scale", RapidJsonUtils::ToJsonArray(textureTransform.scale, a), a);
        }

        if (textureTransform.texCoord != 0)
        {
            KHR_textureTransform.AddMember("texCoord", textureTransform.texCoord, a);
        }

        SerializeProperty(textureTransform, KHR_textureTransform, a);
    }

    glTF::rapidjson::StringBuffer buffer;
    glTF::rapidjson::Writer<glTF::rapidjson::StringBuffer> writer(buffer);
    KHR_textureTransform.Accept(writer);

    return buffer.GetString();
}

std::unique_ptr<Extension> KHR::TextureInfos::DeserializeTextureTransform(const std::string& json)
{
    TextureTransform textureTransform;

    auto doc = RapidJsonUtils::CreateDocumentFromString(json);
    const rapidjson::Value sit = doc.GetObject();

    // Offset
    auto offsetIt = sit.FindMember("offset");
    if (offsetIt != sit.MemberEnd())
    {
        std::vector<float> offset;
        for (rapidjson::Value::ConstValueIterator ait = offsetIt->value.Begin(); ait != offsetIt->value.End(); ++ait)
        {
            offset.push_back(static_cast<float>(ait->GetDouble()));
        }
        // NOTE: Schema validation will handle this once the extension is added to the GLTF SDK
        textureTransform.offset = (offset.size() == 2) ? Vector2(offset[0], offset[1]) : Vector2(0.0f, 0.0f);
    }

    // Rotation
    textureTransform.rotation = GetMemberValueOrDefault<float>(sit, "rotation", 0.0f);

    // Scale
    auto scaleIt = sit.FindMember("scale");
    if (scaleIt != sit.MemberEnd())
    {
        std::vector<float> scale;
        for (rapidjson::Value::ConstValueIterator ait = scaleIt->value.Begin(); ait != scaleIt->value.End(); ++ait)
        {
            scale.push_back(static_cast<float>(ait->GetDouble()));
        }
        // NOTE: Schema validation will handle this once the extension is added to the GLTF SDK
        textureTransform.scale = (scale.size() == 2) ? Vector2(scale[0], scale[1]) : Vector2(1.0f, 1.0f);
    }

    // TexCoord
    textureTransform.texCoord = glTF::GetMemberValueOrDefault<unsigned int>(sit, "texCoord", 0);

    ParseProperty(sit, textureTransform);

    return std::make_unique<TextureTransform>(textureTransform);
}
