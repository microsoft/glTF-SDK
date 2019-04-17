// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/Deserialize.h>

#include <GLTFSDK/Constants.h>
#include <GLTFSDK/ExtensionHandlers.h>
#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/RapidJsonUtils.h>
#include <GLTFSDK/Serialize.h>
#include <GLTFSDK/SchemaValidation.h>

#include <iostream>

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

    template<typename T>
    IndexedContainer<const T> DeserializeToIndexedContainer(
        const char* name,
        const rapidjson::Value& value,
        const ExtensionDeserializer& extensionDeserializer,
        T(*fn)(const rapidjson::Value&, const ExtensionDeserializer&))
    {
        IndexedContainer<const T> items;

        rapidjson::Value::ConstMemberIterator it;
        if (TryFindMember(name, value, it))
        {
            size_t index = 0;

            for (auto& valueArray : it->value.GetArray())
            {
                try
                {
                    const auto& item = items.Append(fn(valueArray, extensionDeserializer), AppendIdPolicy::GenerateOnEmpty);
                    const auto& itemId = item.id;

                    (void)itemId;   // To disable unused-variable warnings when assert is compiled away.
                    assert(itemId == std::to_string(index));
                }
                catch (const InvalidGLTFException& e)
                {
                    std::cerr << "Could not parse " << name << "[" << index << "]: " << e.what() << "\n";
                    throw;
                }

                ++index;
            }
        }

        return items;
    }

    Asset ParseAsset(const rapidjson::Value& assetValue, const ExtensionDeserializer& extensionDeserializer)
    {
        Asset asset;

        asset.copyright = GetMemberValueOrDefault<std::string>(assetValue, "copyright");
        asset.generator = GetMemberValueOrDefault<std::string>(assetValue, "generator");
        asset.version = FindRequiredMember("version", assetValue)->value.GetString();
        asset.minVersion = GetMemberValueOrDefault<std::string>(assetValue, "minVersion");

        ParseProperty(assetValue, asset, extensionDeserializer);

        return asset;
    }

    Accessor ParseAccessor(const rapidjson::Value& v, const ExtensionDeserializer& extensionDeserializer)
    {
        Accessor accessor;
        accessor.name = GetMemberValueOrDefault<std::string>(v, "name");

        rapidjson::Value::ConstMemberIterator it;

        if (TryFindMember("sparse", v, it)) 
        {
            const rapidjson::Value& sparseMember = it->value;
            const rapidjson::Value& sparseIndicesMember = FindRequiredMember("indices", sparseMember)->value;
            const rapidjson::Value& sparseValuesMember = FindRequiredMember("values", sparseMember)->value;

            accessor.sparse.count = GetValue<size_t>(FindRequiredMember("count", sparseMember)->value);

            accessor.sparse.indicesBufferViewId = std::to_string(FindRequiredMember("bufferView", sparseIndicesMember)->value.GetUint());
            accessor.sparse.indicesComponentType = Accessor::GetComponentType(FindRequiredMember("componentType", sparseIndicesMember)->value.GetUint());
            accessor.sparse.indicesByteOffset = GetMemberValueOrDefault<size_t>(sparseIndicesMember, "byteOffset");

            accessor.sparse.valuesBufferViewId = std::to_string(FindRequiredMember("bufferView", sparseValuesMember)->value.GetUint());
            accessor.sparse.valuesByteOffset = GetMemberValueOrDefault<size_t>(sparseValuesMember, "byteOffset");

            if (TryFindMember("bufferView", v, it))
            {
                accessor.bufferViewId = std::to_string(it->value.GetUint());
            }
        }
        else
        {
            accessor.bufferViewId = GetMemberValueAsString<size_t>(v, "bufferView");
        }

        accessor.byteOffset = GetMemberValueOrDefault<size_t>(v, "byteOffset");
        accessor.componentType = Accessor::GetComponentType(FindRequiredMember("componentType", v)->value.GetUint());
        accessor.normalized = GetMemberValueOrDefault<bool>(v, "normalized", false);
        accessor.count = GetValue<size_t>(FindRequiredMember("count", v)->value);
        accessor.type = Accessor::ParseType(FindRequiredMember("type", v)->value.GetString());

        if (TryFindMember("min", v, it))
        {
            for (rapidjson::Value::ConstValueIterator ait = it->value.Begin(); ait != it->value.End(); ++ait)
            {
                accessor.min.push_back(static_cast<float>(ait->GetDouble()));
            }
        }

        if (TryFindMember("max", v, it))
        {
            for (rapidjson::Value::ConstValueIterator ait = it->value.Begin(); ait != it->value.End(); ++ait)
            {
                accessor.max.push_back(static_cast<float>(ait->GetDouble()));
            }
        }

        ParseProperty(v, accessor, extensionDeserializer);

        return accessor;
    }

    BufferView ParseBufferView(const rapidjson::Value& v, const ExtensionDeserializer& extensionDeserializer)
    {
        BufferView bv;

        bv.name = GetMemberValueOrDefault<std::string>(v, "name");
        bv.bufferId = std::to_string(FindRequiredMember("buffer", v)->value.GetUint());
        bv.byteOffset = GetMemberValueOrDefault<size_t>(v, "byteOffset");
        bv.byteLength = GetValue<size_t>(FindRequiredMember("byteLength", v)->value);

        auto itByteStride = v.FindMember("byteStride");
        if (itByteStride != v.MemberEnd())
        {
            bv.byteStride = itByteStride->value.GetUint();
        }

        // When target is not provided, the bufferView contains animation or skin data
        auto itTarget = v.FindMember("target");
        if (itTarget != v.MemberEnd())
        {
            bv.target = static_cast<BufferViewTarget>(itTarget->value.GetUint());
        }

        ParseProperty(v, bv, extensionDeserializer);

        return bv;
    }

    Scene ParseScene(const rapidjson::Value& v, const ExtensionDeserializer& extensionDeserializer)
    {
        Scene scene;

        rapidjson::Value::ConstMemberIterator it = v.FindMember("nodes");
        if (it != v.MemberEnd())
        {
            const rapidjson::Value& a = it->value;
            scene.nodes.reserve(a.Capacity());
            for (rapidjson::Value::ConstValueIterator ait = a.Begin(); ait != a.End(); ++ait)
            {
                scene.nodes.push_back(std::to_string(ait->GetUint()));
            }
        }

        ParseProperty(v, scene, extensionDeserializer);

        return scene;
    }

    MorphTarget ParseTarget(const rapidjson::Value& v)
    {
        MorphTarget target;
        target.positionsAccessorId = GetMemberValueAsString<uint32_t>(v, ACCESSOR_POSITION);
        target.normalsAccessorId = GetMemberValueAsString<uint32_t>(v, ACCESSOR_NORMAL);
        target.tangentsAccessorId = GetMemberValueAsString<uint32_t>(v, ACCESSOR_TANGENT);

        return target;
    }

    void ParseTargets(const rapidjson::Value& v, MeshPrimitive& primitive)
    {
        rapidjson::Value::ConstMemberIterator it = v.FindMember("targets");
        if (it != v.MemberEnd())
        {
            const rapidjson::Value& a = it->value;
            primitive.targets.reserve(a.Capacity());
            for (rapidjson::Value::ConstValueIterator ait = a.Begin(); ait != a.End(); ++ait)
            {
                primitive.targets.push_back(ParseTarget(*ait));
            }
        }
    }

    MeshPrimitive ParseMeshPrimitive(const rapidjson::Value& v, const ExtensionDeserializer& extensionDeserializer)
    {
        MeshPrimitive primitive;

        rapidjson::Value::ConstMemberIterator it = v.FindMember("attributes");
        if (it != v.MemberEnd())
        {
            const auto& attributes = it->value.GetObject();

            for (const auto& attribute : attributes)
            {
                auto name = attribute.name.GetString();
                primitive.attributes[name] = std::to_string(attribute.value.Get<uint32_t>());
            }
        }

        primitive.indicesAccessorId = GetMemberValueAsString<uint32_t>(v, "indices");
        primitive.materialId = GetMemberValueAsString<uint32_t>(v, "material");
        primitive.mode = static_cast<MeshMode>(GetMemberValueOrDefault<int>(v, "mode", MESH_TRIANGLES));
        ParseTargets(v, primitive);

        ParseProperty(v, primitive, extensionDeserializer);

        return primitive;
    }

    Mesh ParseMesh(const rapidjson::Value& v, const ExtensionDeserializer& extensionDeserializer)
    {
        Mesh mesh;
        mesh.name = GetMemberValueOrDefault<std::string>(v, "name");

        rapidjson::Value::ConstMemberIterator it = v.FindMember("primitives");

        if (it != v.MemberEnd())
        {
            const rapidjson::Value& a = it->value;
            mesh.primitives.reserve(a.Capacity());
            for (rapidjson::Value::ConstValueIterator ait = a.Begin(); ait != a.End(); ++ait)
            {
                mesh.primitives.push_back(ParseMeshPrimitive(*ait, extensionDeserializer));
            }
        }

        mesh.weights = RapidJsonUtils::ToFloatArray(v, "weights");

        ParseProperty(v, mesh, extensionDeserializer);

        return mesh;
    }

    void ParseNodeScale(const rapidjson::Value& v, Node& node)
    {
        const int scaleCapacity = 3;
        auto it = v.FindMember("scale");
        if (it == v.MemberEnd())
        {
            node.scale = Vector3::ONE;
            return;
        }

        const rapidjson::Value& a = it->value;
        if (a.Capacity() != scaleCapacity)
        {
            throw InvalidGLTFException("A node must have a scale with 3 elements");
        }

        rapidjson::Value::ConstValueIterator ait = a.Begin();
        node.scale.x = ait++->GetFloat();
        node.scale.y = ait++->GetFloat();
        node.scale.z = ait->GetFloat();
    }

    void ParseNodeTranslation(const rapidjson::Value& v, Node& node)
    {
        const int translationCapacity = 3;
        auto it = v.FindMember("translation");
        if (it == v.MemberEnd())
        {
            node.translation = Vector3::ZERO;
            return;
        }

        const rapidjson::Value& a = it->value;
        if (a.Capacity() != translationCapacity)
        {
            throw InvalidGLTFException("A node must have a translation with 3 elements");
        }

        rapidjson::Value::ConstValueIterator ait = a.Begin();
        node.translation.x = ait++->GetFloat();
        node.translation.y = ait++->GetFloat();
        node.translation.z = ait->GetFloat();
    }

    void ParseNodeRotation(const rapidjson::Value& v, Node& node)
    {
        const int rotationCapacity = 4;
        auto it = v.FindMember("rotation");
        if (it == v.MemberEnd())
        {
            node.rotation = Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
            return;
        }

        const rapidjson::Value& a = it->value;
        if (a.Capacity() != rotationCapacity)
        {
            throw InvalidGLTFException("A node must have a rotation with 4 elements");
        }

        rapidjson::Value::ConstValueIterator ait = a.Begin();
        node.rotation.x = ait++->GetFloat();
        node.rotation.y = ait++->GetFloat();
        node.rotation.z = ait++->GetFloat();
        node.rotation.w = ait->GetFloat();
    }

    void ParseNodeMatrix(const rapidjson::Value& v, Node& node)
    {
        auto it = v.FindMember("matrix");
        if (it == v.MemberEnd())
        {
            ParseNodeScale(v, node);
            ParseNodeTranslation(v, node);
            ParseNodeRotation(v, node);
            return;
        }

        const rapidjson::Value& a = it->value;
        if (a.Capacity() != 16)
        {
            throw InvalidGLTFException("A node must have a matrix transform with 16 elements");
        }

        uint8_t index = 0;
        for (rapidjson::Value::ConstValueIterator ait = a.Begin(); ait != a.End(); ++ait)
        {
            node.matrix.values[index] = static_cast<float>(ait->GetDouble());
            index++;
        }
    }

    void ParseNodeChildren(const rapidjson::Value& v, Node& node)
    {
        rapidjson::Value::ConstMemberIterator it = v.FindMember("children");
        if (it != v.MemberEnd())
        {
            const rapidjson::Value& a = it->value;
            node.children.reserve(a.Capacity());
            for (rapidjson::Value::ConstValueIterator ait = a.Begin(); ait != a.End(); ++ait)
            {
                node.children.push_back(std::to_string(ait->GetUint()));
            }
        }
    }

    Camera ParseCamera(const rapidjson::Value& v, const ExtensionDeserializer& extensionDeserializer)
    {
        std::unique_ptr<Projection> projection;
        std::string projectionType = FindRequiredMember("type", v)->value.GetString();

        if (projectionType == "perspective")
        {
            auto perspectiveIt = v.FindMember("perspective");
            if (perspectiveIt == v.MemberEnd())
            {
                throw InvalidGLTFException("Camera perspective projection undefined");
            }

            Optional<float> aspectRatio;

            auto itAspectRatio = perspectiveIt->value.FindMember("target");
            if (itAspectRatio != perspectiveIt->value.MemberEnd())
            {
                aspectRatio = itAspectRatio->value.GetFloat();
            }

            float yfov = GetValue<float>(FindRequiredMember("yfov", perspectiveIt->value)->value);
            float znear = GetValue<float>(FindRequiredMember("znear", perspectiveIt->value)->value);

            Optional<float> zfar;

            auto itZFar = perspectiveIt->value.FindMember("zfar");
            if (itZFar != perspectiveIt->value.MemberEnd())
            {
                zfar = itZFar->value.GetFloat();
            }

            auto perspective = std::make_unique<Perspective>(znear, yfov);

            perspective->zfar = zfar;
            perspective->aspectRatio = aspectRatio;

            ParseProperty(perspectiveIt->value, *perspective, extensionDeserializer);

            projection = std::move(perspective);
        }
        else if (projectionType == "orthographic")
        {
            auto orthographicIt = v.FindMember("orthographic");
            if (orthographicIt == v.MemberEnd())
            {
                throw InvalidGLTFException("Camera orthographic projection undefined");
            }

            float xmag = GetValue<float>(FindRequiredMember("xmag", orthographicIt->value)->value);
            float ymag = GetValue<float>(FindRequiredMember("ymag", orthographicIt->value)->value);
            float zfar = GetValue<float>(FindRequiredMember("zfar", orthographicIt->value)->value);
            float znear = GetValue<float>(FindRequiredMember("znear", orthographicIt->value)->value);
            projection = std::make_unique<Orthographic>(zfar, znear, xmag, ymag);

            ParseProperty(orthographicIt->value, *projection, extensionDeserializer);
        }

        // Camera constructor will throw a GLTFException when projection is null (i.e. source manifest specified an invalid projection type)
        Camera camera(std::move(projection));
        camera.name = GetMemberValueOrDefault<std::string>(v, "name");

        if (!camera.projection->IsValid())
        {
            throw InvalidGLTFException("Camera's projection is not valid");
        }

        ParseProperty(v, camera, extensionDeserializer);

        return camera;
    }

    Node ParseNode(const rapidjson::Value& v, const ExtensionDeserializer& extensionDeserializer)
    {
        Node node;
        node.name = GetMemberValueOrDefault<std::string>(v, "name");

        ParseNodeChildren(v, node);
        node.meshId = GetMemberValueAsString<uint32_t>(v, "mesh");
        node.skinId = GetMemberValueAsString<uint32_t>(v, "skin");
        node.cameraId = GetMemberValueAsString<uint32_t>(v, "camera");
        ParseNodeMatrix(v, node);
        node.weights = RapidJsonUtils::ToFloatArray(v, "weights");

        ParseProperty(v, node, extensionDeserializer);

        return node;
    }

    Buffer ParseBuffer(const rapidjson::Value& v, const ExtensionDeserializer& extensionDeserializer)
    {
        Buffer buffer;

        buffer.byteLength = GetValue<size_t>(FindRequiredMember("byteLength", v)->value);
        buffer.uri = GetMemberValueOrDefault<std::string>(v, "uri");

        ParseProperty(v, buffer, extensionDeserializer);

        return buffer;
    }

    Sampler ParseSampler(const rapidjson::Value& v, const ExtensionDeserializer& extensionDeserializer)
    {
        Sampler sampler;

        sampler.name = GetMemberValueOrDefault<std::string>(v, "name");
        sampler.wrapT = Sampler::GetSamplerWrapMode(GetMemberValueOrDefault<unsigned int>(v, "wrapT", static_cast<unsigned int>(WrapMode::Wrap_REPEAT)));
        sampler.wrapS = Sampler::GetSamplerWrapMode(GetMemberValueOrDefault<unsigned int>(v, "wrapS", static_cast<unsigned int>(WrapMode::Wrap_REPEAT)));

        auto itMin = v.FindMember("minFilter");
        if (itMin != v.MemberEnd())
        {
            sampler.minFilter = Sampler::GetSamplerMinFilterMode(itMin->value.GetUint());
        }

        auto itMag = v.FindMember("magFilter");
        if (itMag != v.MemberEnd())
        {
            sampler.magFilter = Sampler::GetSamplerMagFilterMode(itMag->value.GetUint());
        }

        ParseProperty(v, sampler, extensionDeserializer);

        return sampler;
    }

    AnimationTarget ParseAnimationTarget(const rapidjson::Value& v, const ExtensionDeserializer& extensionDeserializer)
    {
        try
        {
            AnimationTarget target;

            target.nodeId = GetMemberValueAsString<uint32_t>(v, "node");

            auto it = v.FindMember("path");
            if (it != v.MemberEnd())
            {
                target.path = ParseTargetPath(it->value.GetString());
            }

            ParseProperty(v, target, extensionDeserializer);

            return target;
        }
        catch (const InvalidGLTFException& e)
        {
            std::cerr << "Could not parse animation target\n" << e.what() << "\n";
            throw;
        }
    }

    AnimationChannel ParseAnimationChannel(const rapidjson::Value& v, const ExtensionDeserializer& extensionDeserializer)
    {
        try
        {
            AnimationChannel channel;

            channel.samplerId = GetMemberValueAsString<uint32_t>(v, "sampler");
            channel.target = ParseAnimationTarget(FindRequiredMember("target", v)->value, extensionDeserializer);

            ParseProperty(v, channel, extensionDeserializer);

            return channel;
        }
        catch (const InvalidGLTFException& e)
        {
            std::cerr << "Could not parse channel\n" << e.what() << "\n";
            throw;
        }
    }

    AnimationSampler ParseAnimationSampler(const rapidjson::Value& v, const ExtensionDeserializer& extensionDeserializer)
    {
        AnimationSampler sampler;

        sampler.inputAccessorId = GetMemberValueAsString<uint32_t>(v, "input");
        sampler.outputAccessorId = GetMemberValueAsString<uint32_t>(v, "output");

        auto it = v.FindMember("interpolation");
        if (it != v.MemberEnd())
        {
            sampler.interpolation = ParseInterpolationType(it->value.GetString());
        }

        ParseProperty(v, sampler, extensionDeserializer);

        return sampler;
    }

    Animation ParseAnimation(const rapidjson::Value& v, const ExtensionDeserializer& extensionDeserializer)
    {
        Animation anim;
        anim.name = GetMemberValueOrDefault<std::string>(v, "name");

        anim.channels = DeserializeToIndexedContainer<AnimationChannel>("channels", v, extensionDeserializer, ParseAnimationChannel);

        anim.samplers = DeserializeToIndexedContainer<AnimationSampler>("samplers", v, extensionDeserializer, ParseAnimationSampler);

        ParseProperty(v, anim, extensionDeserializer);

        return anim;
    }

    Skin ParseSkin(const rapidjson::Value& v, const ExtensionDeserializer& extensionDeserializer)
    {
        Skin skin;

        skin.name = GetMemberValueOrDefault<std::string>(v, "name");
        skin.inverseBindMatricesAccessorId = GetMemberValueAsString<uint32_t>(v, "inverseBindMatrices");
        skin.skeletonId = GetMemberValueAsString<uint32_t>(v, "skeleton");

        rapidjson::Value::ConstMemberIterator it = v.FindMember("joints");
        if (it != v.MemberEnd())
        {
            const rapidjson::Value& a = it->value;
            skin.jointIds.reserve(a.Capacity());
            for (rapidjson::Value::ConstValueIterator ait = a.Begin(); ait != a.End(); ++ait)
            {
                skin.jointIds.push_back(std::to_string(ait->GetInt()));
            }
        }

        ParseProperty(v, skin, extensionDeserializer);

        return skin;
    }

    void ParseExtensionsUsed(const rapidjson::Document& d, Document& gltfDocument)
    {
        rapidjson::Value::ConstMemberIterator it;
        if (TryFindMember("extensionsUsed", d, it))
        {
            const rapidjson::Value& a = it->value;
            for (rapidjson::Value::ConstValueIterator ait = a.Begin(); ait != a.End(); ++ait)
            {
                gltfDocument.extensionsUsed.insert(ait->GetString());
            }
        }
    }

    void ParseExtensionsRequired(const rapidjson::Document& d, Document& gltfDocument)
    {
        rapidjson::Value::ConstMemberIterator it;
        if (TryFindMember("extensionsRequired", d, it))
        {
            const rapidjson::Value& a = it->value;
            for (rapidjson::Value::ConstValueIterator ait = a.Begin(); ait != a.End(); ++ait)
            {
                gltfDocument.extensionsRequired.insert(ait->GetString());
            }
        }
    }

    void ValidateMaterial(Material& material)
    {
        if (material.occlusionTexture.strength > 1 || material.occlusionTexture.strength < 0)
        {
            throw InvalidGLTFException("Material " + material.name + " has invalid occlusionStrength (value out of range [0,1])");
        }
    }

    Material ParseMaterial(const rapidjson::Value& v, const ExtensionDeserializer& extensionDeserializer)
    {
        Material material;

        auto mit = v.FindMember("pbrMetallicRoughness");
        if (mit != v.MemberEnd())
        {
            auto& pbrMr = mit->value;

            // Diffuse
            auto baseColorFactorIt = pbrMr.FindMember("baseColorFactor");
            if (baseColorFactorIt != pbrMr.MemberEnd())
            {
                std::vector<float> baseColorFactor;
                for (rapidjson::Value::ConstValueIterator ait = baseColorFactorIt->value.Begin(); ait != baseColorFactorIt->value.End(); ++ait)
                {
                    baseColorFactor.push_back(static_cast<float>(ait->GetDouble()));
                }
                material.metallicRoughness.baseColorFactor = Color4(baseColorFactor[0], baseColorFactor[1], baseColorFactor[2], baseColorFactor[3]);
            }
            
            auto baseColorTextureIt = pbrMr.FindMember("baseColorTexture");
            if (baseColorTextureIt != pbrMr.MemberEnd())
            {
                ParseTextureInfo(baseColorTextureIt->value, material.metallicRoughness.baseColorTexture, extensionDeserializer);
            }

            material.metallicRoughness.metallicFactor = GetMemberValueOrDefault<float>(pbrMr, "metallicFactor", 1.0f);
            material.metallicRoughness.roughnessFactor = GetMemberValueOrDefault<float>(pbrMr, "roughnessFactor", 1.0f);

            auto metallicRoughnessTextureIt = pbrMr.FindMember("metallicRoughnessTexture");
            if (metallicRoughnessTextureIt != pbrMr.MemberEnd())
            {
                ParseTextureInfo(metallicRoughnessTextureIt->value, material.metallicRoughness.metallicRoughnessTexture, extensionDeserializer);
            }
        }

        // Normal
        auto normalTextureIt = v.FindMember("normalTexture");
        if (normalTextureIt != v.MemberEnd())
        {
            ParseTextureInfo(normalTextureIt->value, material.normalTexture, extensionDeserializer);
            material.normalTexture.scale = GetMemberValueOrDefault<float>(normalTextureIt->value, "scale", 1.0f);
        }

        // Occlusion
        auto occlusionTextureIt = v.FindMember("occlusionTexture");
        if (occlusionTextureIt != v.MemberEnd())
        {
            ParseTextureInfo(occlusionTextureIt->value, material.occlusionTexture, extensionDeserializer);
            material.occlusionTexture.strength = GetMemberValueOrDefault<float>(occlusionTextureIt->value, "strength", 1.0f);
        }

        // Emissive Texture
        auto emissionTextureIt = v.FindMember("emissiveTexture");
        if (emissionTextureIt != v.MemberEnd())
        {
            ParseTextureInfo(emissionTextureIt->value, material.emissiveTexture, extensionDeserializer);
        }

        // Emissive Factor
        auto emissionFactorIt = v.FindMember("emissiveFactor");
        if (emissionFactorIt != v.MemberEnd())
        {
            std::vector<float> emissiveFactor;
            for (rapidjson::Value::ConstValueIterator ait = emissionFactorIt->value.Begin(); ait != emissionFactorIt->value.End(); ++ait)
            {
                emissiveFactor.push_back(static_cast<float>(ait->GetDouble()));
            }
            material.emissiveFactor = Color3(emissiveFactor[0], emissiveFactor[1], emissiveFactor[2]);
        }

        // Alpha Mode
        auto alphaModeIt = v.FindMember("alphaMode");
        if (alphaModeIt != v.MemberEnd())
        {
            material.alphaMode = ParseAlphaMode(alphaModeIt->value.GetString());
        }

        // Alpha Cutoff
        material.alphaCutoff = GetMemberValueOrDefault<float>(v, "alphaCutoff", 0.5f);

        // Name
        material.name = GetMemberValueOrDefault<std::string>(v, "name");

        // Double Sided
        material.doubleSided = GetMemberValueOrDefault<bool>(v, "doubleSided", false);

        ParseProperty(v, material, extensionDeserializer);

        ValidateMaterial(material);

        return material;
    }

    Texture ParseTexture(const rapidjson::Value& v, const ExtensionDeserializer& extensionDeserializer)
    {
        // Parse texture fields or assign default values see:
        // https://github.com/KhronosGroup/glTF/blob/master/specification/README.md

        Texture texture;

        texture.name = GetMemberValueOrDefault<std::string>(v, "name");
        texture.imageId = GetMemberValueAsString<uint32_t>(v, "source");
        texture.samplerId = GetMemberValueAsString<uint32_t>(v, "sampler");

        ParseProperty(v, texture, extensionDeserializer);

        return texture;
    }

    Image ParseImage(const rapidjson::Value& v, const ExtensionDeserializer& extensionDeserializer)
    {
        // Parse image fields or assign default values see:
        // https://github.com/KhronosGroup/glTF/blob/master/specification/README.md

        Image image;

        image.name = GetMemberValueOrDefault<std::string>(v, "name");
        image.uri = GetMemberValueOrDefault<std::string>(v, "uri");
        image.bufferViewId = GetMemberValueAsString<uint32_t>(v, "bufferView");
        image.mimeType = GetMemberValueOrDefault<std::string>(v, "mimeType");

        ParseProperty(v, image, extensionDeserializer);

        return image;
    }

    Document DeserializeInternal(const rapidjson::Document& document, const ExtensionDeserializer& extensionDeserializer, SchemaFlags schemaFlags)
    {
        ValidateDocumentAgainstSchema(document, SCHEMA_URI_GLTF, GetDefaultSchemaLocator(schemaFlags));

        Document gltfDocument;

        rapidjson::Value::ConstMemberIterator it;
        if (TryFindMember("asset", document, it))
        {
            gltfDocument.asset = ParseAsset(it->value, extensionDeserializer);
        }

        gltfDocument.accessors   = DeserializeToIndexedContainer<Accessor>("accessors", document, extensionDeserializer, ParseAccessor);
        gltfDocument.animations  = DeserializeToIndexedContainer<Animation>("animations", document, extensionDeserializer, ParseAnimation);
        gltfDocument.buffers     = DeserializeToIndexedContainer<Buffer>("buffers", document, extensionDeserializer, ParseBuffer);
        gltfDocument.bufferViews = DeserializeToIndexedContainer<BufferView>("bufferViews", document, extensionDeserializer, ParseBufferView);
        gltfDocument.cameras     = DeserializeToIndexedContainer<Camera>("cameras", document, extensionDeserializer, ParseCamera);
        gltfDocument.images      = DeserializeToIndexedContainer<Image>("images", document, extensionDeserializer, ParseImage);
        gltfDocument.materials   = DeserializeToIndexedContainer<Material>("materials", document, extensionDeserializer, ParseMaterial);
        gltfDocument.meshes      = DeserializeToIndexedContainer<Mesh>("meshes", document, extensionDeserializer, ParseMesh);
        gltfDocument.nodes       = DeserializeToIndexedContainer<Node>("nodes", document, extensionDeserializer, ParseNode);
        gltfDocument.samplers    = DeserializeToIndexedContainer<Sampler>("samplers", document, extensionDeserializer, ParseSampler);
        gltfDocument.scenes      = DeserializeToIndexedContainer<Scene>("scenes", document, extensionDeserializer, ParseScene);
        gltfDocument.skins       = DeserializeToIndexedContainer<Skin>("skins", document, extensionDeserializer, ParseSkin);
        gltfDocument.textures    = DeserializeToIndexedContainer<Texture>("textures", document, extensionDeserializer, ParseTexture);

        ParseProperty(document, gltfDocument, extensionDeserializer);

        if (TryFindMember("scene", document, it))
        {
            gltfDocument.defaultSceneId = std::to_string(it->value.GetUint());
        }

        ParseExtensionsUsed(document, gltfDocument);
        ParseExtensionsRequired(document, gltfDocument);

        return gltfDocument;
    }

    bool HasFlag(DeserializeFlags flags, DeserializeFlags flag)
    {
        return ((flags & flag) == flag);
    }
}

Document Microsoft::glTF::Deserialize(const std::string& json, DeserializeFlags flags, SchemaFlags schemaFlags)
{
    return Deserialize(json, ExtensionDeserializer(), flags, schemaFlags);
}

Document Microsoft::glTF::Deserialize(const std::string& json, const ExtensionDeserializer& extensionDeserializer, DeserializeFlags flags, SchemaFlags schemaFlags)
{
    const auto document = HasFlag(flags, DeserializeFlags::IgnoreByteOrderMark) ?
        RapidJsonUtils::CreateDocumentFromEncodedString(json) :
        RapidJsonUtils::CreateDocumentFromString(json);

    return DeserializeInternal(document, extensionDeserializer, schemaFlags);
}

Document Microsoft::glTF::Deserialize(std::istream& jsonStream, DeserializeFlags flags, SchemaFlags schemaFlags)
{
    return Deserialize(jsonStream, ExtensionDeserializer(), flags, schemaFlags);
}

Document Microsoft::glTF::Deserialize(std::istream& jsonStream, const ExtensionDeserializer& extensionDeserializer, DeserializeFlags flags, SchemaFlags schemaFlags)
{
    const auto document = HasFlag(flags, DeserializeFlags::IgnoreByteOrderMark) ?
        RapidJsonUtils::CreateDocumentFromEncodedStream(jsonStream) :
        RapidJsonUtils::CreateDocumentFromStream(jsonStream);

    return DeserializeInternal(document, extensionDeserializer, schemaFlags);
}

DeserializeFlags Microsoft::glTF::operator|(DeserializeFlags lhs, DeserializeFlags rhs)
{
    const auto result =
        static_cast<std::underlying_type_t<DeserializeFlags>>(lhs) |
        static_cast<std::underlying_type_t<DeserializeFlags>>(rhs);

    return static_cast<DeserializeFlags>(result);
}

DeserializeFlags& Microsoft::glTF::operator|=(DeserializeFlags& lhs, DeserializeFlags rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

DeserializeFlags Microsoft::glTF::operator&(DeserializeFlags lhs, DeserializeFlags rhs)
{
    const auto result =
        static_cast<std::underlying_type_t<DeserializeFlags>>(lhs) &
        static_cast<std::underlying_type_t<DeserializeFlags>>(rhs);

    return static_cast<DeserializeFlags>(result);
}

DeserializeFlags& Microsoft::glTF::operator&=(DeserializeFlags& lhs, DeserializeFlags rhs)
{
    lhs = lhs & rhs;
    return lhs;
}
