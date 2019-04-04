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
