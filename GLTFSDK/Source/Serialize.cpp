// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/Serialize.h>

#include <GLTFSDK/Document.h>
#include <GLTFSDK/ExtensionHandlers.h>
#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/RapidJsonUtils.h>

using namespace Microsoft::glTF;

namespace
{
    std::string AccessorTypeToString(AccessorType type)
    {
        switch (type)
        {
        case TYPE_SCALAR:
            return TYPE_NAME_SCALAR;
        case TYPE_VEC2:
            return TYPE_NAME_VEC2;
        case TYPE_VEC3:
            return TYPE_NAME_VEC3;
        case TYPE_VEC4:
            return TYPE_NAME_VEC4;
        case TYPE_MAT2:
            return TYPE_NAME_MAT2;
        case TYPE_MAT3:
            return TYPE_NAME_MAT3;
        case TYPE_MAT4:
            return TYPE_NAME_MAT4;
        default:
            return "";
        }
    }

    std::string TargetPathToString(TargetPath target)
    {
        switch (target)
        {
        case TARGET_TRANSLATION:
            return TARGETPATH_NAME_TRANSLATION;
        case TARGET_ROTATION:
            return TARGETPATH_NAME_ROTATION;
        case TARGET_SCALE:
            return TARGETPATH_NAME_SCALE;
        case TARGET_WEIGHTS:
            return TARGETPATH_NAME_WEIGHTS;
        default:
            return "";
        }
    }

    std::string AlphaModeToString(AlphaMode mode)
    {
        switch (mode)
        {
        case ALPHA_OPAQUE:
            return ALPHAMODE_NAME_OPAQUE;
        case ALPHA_BLEND:
            return ALPHAMODE_NAME_BLEND;
        case ALPHA_MASK:
            return ALPHAMODE_NAME_MASK;
        default:
            return "";
        }
    }

    std::string InterpolationTypeToString(InterpolationType interpolationType)
    {
        switch (interpolationType)
        {
        case INTERPOLATION_LINEAR:
            return INTERPOLATIONTYPE_NAME_LINEAR;
        case INTERPOLATION_STEP:
            return INTERPOLATIONTYPE_NAME_STEP;
        case INTERPOLATION_CUBICSPLINE:
            return INTERPOLATIONTYPE_NAME_CUBICSPLINE;
        default:
            return "";
        }
    }

    void SerializePropertyExtensions(const Document& doc, const glTFProperty& property, rapidjson::Value& propertyValue, rapidjson::Document::AllocatorType& a, const ExtensionSerializer& extensionSerializer)
    {
        auto registeredExtensions = property.GetExtensions();

        if (!property.extensions.empty() || !registeredExtensions.empty())
        {
            rapidjson::Value& extensions = RapidJsonUtils::FindOrAddMember(propertyValue, "extensions", a);

            // Add registered extensions
            for (const auto& extension : registeredExtensions)
            {
                const auto extensionPair = extensionSerializer.Serialize(extension, property, doc);

                if (property.HasUnregisteredExtension(extensionPair.name))
                {
                    throw GLTFException("Registered extension '" + extensionPair.name + "' is also present as an unregistered extension.");
                }

                if (doc.extensionsUsed.find(extensionPair.name) == doc.extensionsUsed.end())
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
                if (doc.extensionsUsed.find(extension.first) == doc.extensionsUsed.end())
                {
                    throw GLTFException("Unregistered extension '" + extension.first + "' is not present in extensionsUsed");
                }

                const auto d = RapidJsonUtils::CreateDocumentFromString(extension.second);//TODO: validate the returned document against the extension schema!
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

    void SerializeProperty(const Document& doc, const glTFProperty& property, rapidjson::Value& propertyValue, rapidjson::Document::AllocatorType& a, const ExtensionSerializer& extensionSerializer)
    {
        SerializePropertyExtensions(doc, property, propertyValue, a, extensionSerializer);
        SerializePropertyExtras(property, propertyValue, a);
    }

    void SerializeTextureInfo(const Document& doc, const TextureInfo& textureInfo, rapidjson::Value& textureValue, rapidjson::Document::AllocatorType& a, const IndexedContainer<const Texture>& textures, const ExtensionSerializer& extensionSerializer)
    {
        RapidJsonUtils::AddOptionalMemberIndex("index", textureValue, textureInfo.textureId, textures, a);
        if (textureInfo.texCoord != 0)
        {
            textureValue.AddMember("texCoord", ToKnownSizeType(textureInfo.texCoord), a);
        }
        SerializeProperty(doc, textureInfo, textureValue, a, extensionSerializer);
    }

    void SerializeAsset(const Document& gltfDocument, rapidjson::Document& document, const ExtensionSerializer& extensionSerializer)
    {
        rapidjson::Document::AllocatorType& a = document.GetAllocator();
        rapidjson::Value assetValue(rapidjson::kObjectType);

        RapidJsonUtils::AddOptionalMember("copyright", assetValue, gltfDocument.asset.copyright, a);
        RapidJsonUtils::AddOptionalMember("generator", assetValue, gltfDocument.asset.generator, a);
        assetValue.AddMember("version", RapidJsonUtils::ToStringValue(gltfDocument.asset.version, a), a);
        RapidJsonUtils::AddOptionalMember("minVersion", assetValue, gltfDocument.asset.minVersion, a);

        SerializeProperty(gltfDocument, gltfDocument.asset, assetValue, a, extensionSerializer);

        document.AddMember("asset", assetValue, a);
    }

    void SerializeDefaultScene(const Document& gltfDocument, rapidjson::Document& document)
    {
        rapidjson::Document::AllocatorType& a = document.GetAllocator();

        if (gltfDocument.HasDefaultScene())
        {
            document.AddMember("scene", ToKnownSizeType(gltfDocument.scenes.GetIndex(gltfDocument.defaultSceneId)), a);
        }
    }

    template<typename T>
    void SerializeIndexedContainer(
        const char* name,
        const IndexedContainer<const T>& indexedContainer,
        const Document& gltfDocument,
        rapidjson::Document& document,
        const ExtensionSerializer& ext,
        rapidjson::Value(*fn)(const T&, const Document&, rapidjson::Document&, const ExtensionSerializer&))
    {
        if (indexedContainer.Size() > 0)
        {
            rapidjson::Document::AllocatorType& a = document.GetAllocator();
            rapidjson::Value containerItems(rapidjson::kArrayType);
            for (const auto& containerElement : indexedContainer.Elements())
            {
                rapidjson::Value value = fn(containerElement, gltfDocument, document, ext);
                containerItems.PushBack(value, a);
            }

            document.AddMember(rapidjson::StringRef(name), containerItems, a);
        }
    }

    rapidjson::Value SerializeAccessor(const Accessor& accessor, const Document& gltfDocument, rapidjson::Document& document, const ExtensionSerializer& extensionSerializer)
    {
        rapidjson::Document::AllocatorType& a = document.GetAllocator();

        rapidjson::Value accessorValue;
        accessorValue.SetObject();

        RapidJsonUtils::AddOptionalMember("name", accessorValue, accessor.name, a);

        if (accessor.sparse.count > 0U)
        {
            if (!accessor.bufferViewId.empty())
            {
                accessorValue.AddMember("bufferView", ToKnownSizeType(gltfDocument.bufferViews.GetIndex(accessor.bufferViewId)), a);
            }

            rapidjson::Value sparseRoot(rapidjson::kObjectType);
            rapidjson::Value indices(rapidjson::kObjectType);
            rapidjson::Value values(rapidjson::kObjectType);

            indices.AddMember("bufferView", ToKnownSizeType(gltfDocument.bufferViews.GetIndex(accessor.sparse.indicesBufferViewId)), a);
            if (accessor.sparse.indicesByteOffset != 0U)
            {
                indices.AddMember("byteOffset", ToKnownSizeType(accessor.sparse.indicesByteOffset), a);
            }
            indices.AddMember("componentType", accessor.sparse.indicesComponentType, a);

            values.AddMember("bufferView", ToKnownSizeType(gltfDocument.bufferViews.GetIndex(accessor.sparse.valuesBufferViewId)), a);
            if (accessor.sparse.valuesByteOffset != 0U)
            {
                values.AddMember("byteOffset", ToKnownSizeType(accessor.sparse.valuesByteOffset), a);
            }

            sparseRoot.AddMember("count", ToKnownSizeType(accessor.sparse.count), a);
            sparseRoot.AddMember("indices", indices, a);
            sparseRoot.AddMember("values", values, a);

            accessorValue.AddMember("sparse", sparseRoot, a);
        }
        else
        {
            RapidJsonUtils::AddOptionalMemberIndex("bufferView", accessorValue, accessor.bufferViewId, gltfDocument.bufferViews, a);
        }

        if (accessor.byteOffset != 0U)
        {
            accessorValue.AddMember("byteOffset", ToKnownSizeType(accessor.byteOffset), a);
        }

        // Normalized
        if (accessor.normalized)
        {
            accessorValue.AddMember("normalized", accessor.normalized, a);
        }

        accessorValue.AddMember("componentType", accessor.componentType, a);
        accessorValue.AddMember("count", ToKnownSizeType(accessor.count), a);
        accessorValue.AddMember("type", RapidJsonUtils::ToStringValue(AccessorTypeToString(accessor.type), a), a);

        rapidjson::Value max(rapidjson::kArrayType);
        rapidjson::Value min(rapidjson::kArrayType);

        if (!accessor.max.empty())
        {
            accessorValue.AddMember("max", RapidJsonUtils::ToJsonArray(accessor.max, a), a);
        }

        if (!accessor.min.empty())
        {
            accessorValue.AddMember("min", RapidJsonUtils::ToJsonArray(accessor.min, a), a);
        }

        SerializeProperty(gltfDocument, accessor, accessorValue, a, extensionSerializer);

        return accessorValue;
    }

    rapidjson::Value SerializeAnimation(const Animation& animation, const Document& gltfDocument, rapidjson::Document& document, const ExtensionSerializer& extensionSerializer)
    {
        rapidjson::Document::AllocatorType& a = document.GetAllocator();

        rapidjson::Value animationValue(rapidjson::kObjectType);
        rapidjson::Value channelValues(rapidjson::kArrayType);
        rapidjson::Value samplerValues(rapidjson::kArrayType);

        for (const auto& channel : animation.channels.Elements())
        {
            rapidjson::Value channelValue(rapidjson::kObjectType);
            rapidjson::Value targetValue(rapidjson::kObjectType);

            RapidJsonUtils::AddOptionalMemberIndex("node", targetValue, channel.target.nodeId, gltfDocument.nodes, a);
            targetValue.AddMember("path", RapidJsonUtils::ToStringValue(TargetPathToString(channel.target.path), a), a);

            channelValue.AddMember("sampler", ToKnownSizeType(animation.samplers.GetIndex(channel.samplerId)), a);
            channelValue.AddMember("target", targetValue, a);

            SerializeProperty(gltfDocument, channel, channelValue, a, extensionSerializer);

            channelValues.PushBack(channelValue, a);
        }

        for (const auto& sampler : animation.samplers.Elements())
        {
            rapidjson::Value samplerValue(rapidjson::kObjectType);
            samplerValue.AddMember("input", ToKnownSizeType(gltfDocument.accessors.GetIndex(sampler.inputAccessorId)), a);
            RapidJsonUtils::AddOptionalMember("interpolation", samplerValue, InterpolationTypeToString(sampler.interpolation), a);
            samplerValue.AddMember("output", ToKnownSizeType(gltfDocument.accessors.GetIndex(sampler.outputAccessorId)), a);

            SerializeProperty(gltfDocument, sampler, samplerValue, a, extensionSerializer);

            samplerValues.PushBack(samplerValue, a);
        }

        animationValue.AddMember("channels", channelValues, a);
        animationValue.AddMember("samplers", samplerValues, a);
        RapidJsonUtils::AddOptionalMember("name", animationValue, animation.name, a);

        SerializeProperty(gltfDocument, animation, animationValue, a, extensionSerializer);

        return animationValue;
    }

    rapidjson::Value SerializeBufferView(const BufferView& bufferView, const Document& gltfDocument, rapidjson::Document& document, const ExtensionSerializer& extensionSerializer)
    {
        rapidjson::Document::AllocatorType& a = document.GetAllocator();

        rapidjson::Value bufferViewValue;
        bufferViewValue.SetObject();

        RapidJsonUtils::AddOptionalMember("name", bufferViewValue, bufferView.name, a);
        bufferViewValue.AddMember("buffer", ToKnownSizeType(gltfDocument.buffers.GetIndex(bufferView.bufferId)), a);
        bufferViewValue.AddMember("byteOffset", ToKnownSizeType(bufferView.byteOffset), a);
        bufferViewValue.AddMember("byteLength", ToKnownSizeType(bufferView.byteLength), a);

        if (bufferView.byteStride)
        {
            bufferViewValue.AddMember("byteStride", ToKnownSizeType(bufferView.byteStride.Get()), a);
        }

        if (bufferView.target)
        {
            bufferViewValue.AddMember("target", bufferView.target.Get(), a);
        }

        SerializeProperty(gltfDocument, bufferView, bufferViewValue, a, extensionSerializer);

        return bufferViewValue;
    }

    rapidjson::Value SerializeBuffer(const Buffer& buffer, const Document& gltfDocument, rapidjson::Document& document, const ExtensionSerializer& extensionSerializer)
    {
        rapidjson::Document::AllocatorType& a = document.GetAllocator();
        rapidjson::Value bufferValue(rapidjson::kObjectType);

        bufferValue.AddMember("byteLength", ToKnownSizeType(buffer.byteLength), a);
        RapidJsonUtils::AddOptionalMember("uri", bufferValue, buffer.uri, a);

        SerializeProperty(gltfDocument, buffer, bufferValue, a, extensionSerializer);

        return bufferValue;
    }

    rapidjson::Value SerializeImage(const Image& image, const Document& gltfDocument, rapidjson::Document& document, const ExtensionSerializer& extensionSerializer)
    {
        if (image.uri.empty())
        {
            if (image.bufferViewId.empty() || image.mimeType.empty())
            {
                throw InvalidGLTFException("Invalid image: " + image.id + ". Images must have either a uri or a bufferView and a mimeType.");
            }
        }
        else if (!image.bufferViewId.empty())
        {
            throw InvalidGLTFException("Invalid image: " + image.id + ". Images can only have a uri or a bufferView, but not both.");
        }

        rapidjson::Document::AllocatorType& a = document.GetAllocator();
        rapidjson::Value imageValue(rapidjson::kObjectType);

        RapidJsonUtils::AddOptionalMember("name", imageValue, image.name, a);
        RapidJsonUtils::AddOptionalMember("uri", imageValue, image.uri, a);
        RapidJsonUtils::AddOptionalMemberIndex("bufferView", imageValue, image.bufferViewId, gltfDocument.bufferViews, a);
        RapidJsonUtils::AddOptionalMember("mimeType", imageValue, image.mimeType, a);

        SerializeProperty(gltfDocument, image, imageValue, a, extensionSerializer);

        return imageValue;
    }

    rapidjson::Value SerializeMaterial(const Material& material, const Document& gltfDocument, rapidjson::Document& document, const ExtensionSerializer& extensionSerializer)
    {
        rapidjson::Document::AllocatorType& a = document.GetAllocator();
        rapidjson::Value materialValue(rapidjson::kObjectType);
        {
            rapidjson::Value pbrMetallicRoughness(rapidjson::kObjectType);
            {
                if (material.metallicRoughness.baseColorFactor != Color4(1.0f, 1.0f, 1.0f, 1.0f))
                {
                    pbrMetallicRoughness.AddMember("baseColorFactor", RapidJsonUtils::ToJsonArray(material.metallicRoughness.baseColorFactor, a), a);
                }

                if (!material.metallicRoughness.baseColorTexture.textureId.empty())
                {
                    rapidjson::Value baseColorTexture(rapidjson::kObjectType);
                    SerializeTextureInfo(gltfDocument, material.metallicRoughness.baseColorTexture, baseColorTexture, a, gltfDocument.textures, extensionSerializer);
                    pbrMetallicRoughness.AddMember("baseColorTexture", baseColorTexture, a);
                }

                if (material.metallicRoughness.metallicFactor != 1.0f)
                {
                    pbrMetallicRoughness.AddMember("metallicFactor", material.metallicRoughness.metallicFactor, a);
                }

                if (material.metallicRoughness.roughnessFactor != 1.0f)
                {
                    pbrMetallicRoughness.AddMember("roughnessFactor", material.metallicRoughness.roughnessFactor, a);
                }

                if (!material.metallicRoughness.metallicRoughnessTexture.textureId.empty())
                {
                    rapidjson::Value metallicRoughnessTexture(rapidjson::kObjectType);
                    SerializeTextureInfo(gltfDocument, material.metallicRoughness.metallicRoughnessTexture, metallicRoughnessTexture, a, gltfDocument.textures, extensionSerializer);
                    pbrMetallicRoughness.AddMember("metallicRoughnessTexture", metallicRoughnessTexture, a);
                }
            }

            materialValue.AddMember("pbrMetallicRoughness", pbrMetallicRoughness, a);

            // Normal
            if (!material.normalTexture.textureId.empty())
            {
                rapidjson::Value normalTexture(rapidjson::kObjectType);
                SerializeTextureInfo(gltfDocument, material.normalTexture, normalTexture, a, gltfDocument.textures, extensionSerializer);
                if (material.normalTexture.scale != 1.0f)
                {
                    normalTexture.AddMember("scale", material.normalTexture.scale, a);
                }
                materialValue.AddMember("normalTexture", normalTexture, a);
            }

            // Occlusion
            if (!material.occlusionTexture.textureId.empty())
            {
                rapidjson::Value occlusionTexture(rapidjson::kObjectType);
                SerializeTextureInfo(gltfDocument, material.occlusionTexture, occlusionTexture, a, gltfDocument.textures, extensionSerializer);
                if (material.occlusionTexture.strength != 1.0f)
                {
                    occlusionTexture.AddMember("strength", material.occlusionTexture.strength, a);
                }
                materialValue.AddMember("occlusionTexture", occlusionTexture, a);
            }

            // Emissive Texture
            if (!material.emissiveTexture.textureId.empty())
            {
                rapidjson::Value emissiveTexture(rapidjson::kObjectType);
                SerializeTextureInfo(gltfDocument, material.emissiveTexture, emissiveTexture, a, gltfDocument.textures, extensionSerializer);
                materialValue.AddMember("emissiveTexture", emissiveTexture, a);
            }

            //// Emissive Factor
            if (material.emissiveFactor != Color3(0.0f, 0.0f, 0.0f))
            {
                materialValue.AddMember("emissiveFactor", RapidJsonUtils::ToJsonArray(material.emissiveFactor, a), a);
            }

            // Alpha Mode : do not serialize default value (opaque) or currently-unsupported value (unknown)
            if (material.alphaMode != ALPHA_OPAQUE && material.alphaMode != ALPHA_UNKNOWN)
            {
                materialValue.AddMember("alphaMode", RapidJsonUtils::ToStringValue(AlphaModeToString(material.alphaMode), a), a);
            }

            // Alpha Cutoff
            if (material.alphaCutoff != 0.5f)
            {
                materialValue.AddMember("alphaCutoff", material.alphaCutoff, a);
            }

            // Name
            RapidJsonUtils::AddOptionalMember("name", materialValue, material.name, a);

            // Double Sided
            if (material.doubleSided)
            {
                materialValue.AddMember("doubleSided", material.doubleSided, a);
            }

            SerializeProperty(gltfDocument, material, materialValue, a, extensionSerializer);
        }

        return materialValue;
    }

    rapidjson::Value SerializeTarget(const MorphTarget& target, const Document& gltfDocument, rapidjson::Document& document)
    {
        rapidjson::Value targetValue(rapidjson::kObjectType);
        rapidjson::Document::AllocatorType& a = document.GetAllocator();

        RapidJsonUtils::AddOptionalMemberIndex(ACCESSOR_POSITION, targetValue, target.positionsAccessorId, gltfDocument.accessors, a);
        RapidJsonUtils::AddOptionalMemberIndex(ACCESSOR_NORMAL, targetValue, target.normalsAccessorId, gltfDocument.accessors, a);
        RapidJsonUtils::AddOptionalMemberIndex(ACCESSOR_TANGENT, targetValue, target.tangentsAccessorId, gltfDocument.accessors, a);

        return targetValue;
    }

    void SerializeTargets(const MeshPrimitive& primitive, rapidjson::Value& primitiveValue, const Document& gltfDocument, rapidjson::Document& document)
    {
        if (!primitive.targets.empty())
        {
            rapidjson::Document::AllocatorType& a = document.GetAllocator();
            rapidjson::Value targets(rapidjson::kArrayType);
            for (const auto& morphTarget : primitive.targets)
            {
                rapidjson::Value targetValue = SerializeTarget(morphTarget, gltfDocument, document);
                targets.PushBack(targetValue, a);
            }
            primitiveValue.AddMember("targets", targets, a);
        }
    }

    rapidjson::Value SerializeMesh(const Mesh& mesh, const Document& gltfDocument, rapidjson::Document& document, const ExtensionSerializer& extensionSerializer)
    {
        rapidjson::Document::AllocatorType& a = document.GetAllocator();

        rapidjson::Value meshValue(rapidjson::kObjectType);
        rapidjson::Value primitiveValues(rapidjson::kArrayType);

        for (const auto& primitive : mesh.primitives)
        {
            rapidjson::Value attributes(rapidjson::kObjectType);

            for (const auto& attribute : primitive.attributes)
            {
                attributes.AddMember(RapidJsonUtils::ToStringValue(attribute.first, a), rapidjson::Value(ToKnownSizeType(gltfDocument.accessors.GetIndex(attribute.second))), a);
            }

            rapidjson::Value primitiveValue(rapidjson::kObjectType);

            primitiveValue.AddMember("attributes", attributes, a);
            RapidJsonUtils::AddOptionalMemberIndex("indices", primitiveValue, primitive.indicesAccessorId, gltfDocument.accessors, a);
            RapidJsonUtils::AddOptionalMemberIndex("material", primitiveValue, primitive.materialId, gltfDocument.materials, a);

            if (primitive.mode != MESH_TRIANGLES)
            {
                primitiveValue.AddMember("mode", primitive.mode, a);
            }

            SerializeTargets(primitive, primitiveValue, gltfDocument, document);

            SerializeProperty(gltfDocument, primitive, primitiveValue, a, extensionSerializer);

            primitiveValues.PushBack(primitiveValue, a);
        }

        RapidJsonUtils::AddArrayMember(meshValue, "weights", mesh.weights, a);

        RapidJsonUtils::AddOptionalMember("name", meshValue, mesh.name, a);
        meshValue.AddMember("primitives", primitiveValues, a);

        SerializeProperty(gltfDocument, mesh, meshValue, a, extensionSerializer);

        return meshValue;
    }

    rapidjson::Value SerializeNode(const Node& node, const Document& gltfDocument, rapidjson::Document& document, const ExtensionSerializer& extensionSerializer)
    {
        rapidjson::Document::AllocatorType& a = document.GetAllocator();

        rapidjson::Value nodeValue(rapidjson::kObjectType);

        if (!node.children.empty())
        {
            rapidjson::Value nodeChildren(rapidjson::kArrayType);
            {
                for (const auto& childId : node.children)
                {
                    nodeChildren.PushBack(ToKnownSizeType(gltfDocument.nodes.GetIndex(childId)), a);
                }
            }
            nodeValue.AddMember("children", nodeChildren, a);
        }

        if (!node.HasValidTransformType())
        {
            throw DocumentException("Node " + node.id + " doesn't have a valid transform type");
        }

        if (node.GetTransformationType() == Microsoft::glTF::TransformationType::TRANSFORMATION_MATRIX)
        {
            nodeValue.AddMember("matrix", RapidJsonUtils::ToJsonArray<float, 16>(node.matrix.values, a), a);
        }
        else if (node.GetTransformationType() == Microsoft::glTF::TransformationType::TRANSFORMATION_TRS)
        {
            if (node.translation != Vector3::ZERO)
            {
                nodeValue.AddMember("translation", RapidJsonUtils::ToJsonArray(node.translation, a), a);
            }
            if (node.rotation != Quaternion::IDENTITY)
            {
                nodeValue.AddMember("rotation", RapidJsonUtils::ToJsonArray(node.rotation, a), a);
            }
            if (node.scale != Vector3::ONE)
            {
                nodeValue.AddMember("scale", RapidJsonUtils::ToJsonArray(node.scale, a), a);
            }
        }

        RapidJsonUtils::AddOptionalMemberIndex("mesh", nodeValue, node.meshId, gltfDocument.meshes, a);
        RapidJsonUtils::AddOptionalMemberIndex("skin", nodeValue, node.skinId, gltfDocument.skins, a);

        if (!node.cameraId.empty())
        {
            nodeValue.AddMember("camera", ToKnownSizeType(gltfDocument.cameras.GetIndex(node.cameraId)), a);
        }

        RapidJsonUtils::AddArrayMember(nodeValue, "weights", node.weights, a);

        RapidJsonUtils::AddOptionalMember("name", nodeValue, node.name, a);

        SerializeProperty(gltfDocument, node, nodeValue, a, extensionSerializer);

        return nodeValue;
    }

    rapidjson::Value SerializeCamera(const Camera& camera, const Document& gltfDocument, rapidjson::Document& document, const ExtensionSerializer& extensionSerializer)
    {
        rapidjson::Document::AllocatorType& a = document.GetAllocator();
        rapidjson::Value cameraValue(rapidjson::kObjectType);
        rapidjson::Value projectionValue(rapidjson::kObjectType);

        const ProjectionType projectionType = camera.projection->GetProjectionType();

        if (projectionType == PROJECTION_PERSPECTIVE)
        {
            const auto& perspective = camera.GetPerspective();

            projectionValue.AddMember("znear", RapidJsonUtils::ToFloatValue(perspective.znear), a);
            projectionValue.AddMember("yfov", RapidJsonUtils::ToFloatValue(perspective.yfov), a);

            if (perspective.zfar)
            {
                projectionValue.AddMember("zfar", RapidJsonUtils::ToFloatValue(perspective.zfar.Get()), a);
            }

            if (perspective.aspectRatio)
            {
                projectionValue.AddMember("aspectRatio", RapidJsonUtils::ToFloatValue(perspective.aspectRatio.Get()), a);
            }

            SerializeProperty(gltfDocument, perspective, projectionValue, a, extensionSerializer);

            cameraValue.AddMember("perspective", projectionValue, a);
            cameraValue.AddMember("type", RapidJsonUtils::ToStringValue("perspective", a), a);
        }
        else if (projectionType == PROJECTION_ORTHOGRAPHIC)
        {
            const auto& orthographic = camera.GetOrthographic();

            projectionValue.AddMember("xmag", RapidJsonUtils::ToFloatValue(orthographic.xmag), a);
            projectionValue.AddMember("ymag", RapidJsonUtils::ToFloatValue(orthographic.ymag), a);
            projectionValue.AddMember("znear", RapidJsonUtils::ToFloatValue(orthographic.znear), a);
            projectionValue.AddMember("zfar", RapidJsonUtils::ToFloatValue(orthographic.zfar), a);

            SerializeProperty(gltfDocument, orthographic, projectionValue, a, extensionSerializer);

            cameraValue.AddMember("orthographic", projectionValue, a);
            cameraValue.AddMember("type", RapidJsonUtils::ToStringValue("orthographic", a), a);
        }
        else
        {
            throw DocumentException("Camera " + camera.id + " doesn't have a valid projection type");
        }

        SerializeProperty(gltfDocument, camera, cameraValue, a, extensionSerializer);

        RapidJsonUtils::AddOptionalMember("name", cameraValue, camera.name, a);

        return cameraValue;
    }

    rapidjson::Value SerializeSampler(const Sampler& sampler, const Document& gltfDocument, rapidjson::Document& document, const ExtensionSerializer& extensionSerializer)
    {
        rapidjson::Document::AllocatorType& a = document.GetAllocator();
        rapidjson::Value samplerValue(rapidjson::kObjectType);

        {
            RapidJsonUtils::AddOptionalMember("name", samplerValue, sampler.name, a);

            if (sampler.magFilter)
            {
                samplerValue.AddMember("magFilter", sampler.magFilter.Get(), a);
            }

            if (sampler.minFilter)
            {
                samplerValue.AddMember("minFilter", sampler.minFilter.Get(), a);
            }

            if (sampler.wrapS != WrapMode::Wrap_REPEAT)
            {
                samplerValue.AddMember("wrapS", sampler.wrapS, a);
            }

            if (sampler.wrapT != WrapMode::Wrap_REPEAT)
            {
                samplerValue.AddMember("wrapT", sampler.wrapT, a);
            }

            SerializeProperty(gltfDocument, sampler, samplerValue, a, extensionSerializer);
        }

        return samplerValue;
    }

    rapidjson::Value SerializeScene(const Scene& scene, const Document& gltfDocument, rapidjson::Document& document, const ExtensionSerializer& extensionSerializer)
    {
        rapidjson::Document::AllocatorType& a = document.GetAllocator();
        rapidjson::Value sceneValue(rapidjson::kObjectType);

        if (!scene.nodes.empty())
        {
            rapidjson::Value nodesArray(rapidjson::kArrayType);
            for (const auto& nodeId : scene.nodes)
            {
                nodesArray.PushBack(ToKnownSizeType(gltfDocument.nodes.GetIndex(nodeId)), a);
            }
            sceneValue.AddMember("nodes", nodesArray, a);
        }

        RapidJsonUtils::AddOptionalMember("name", sceneValue, scene.name, a);

        SerializeProperty(gltfDocument, scene, sceneValue, a, extensionSerializer);

        return sceneValue;
    }

    rapidjson::Value SerializeSkin(const Skin& skin, const Document& gltfDocument, rapidjson::Document& document, const ExtensionSerializer& extensionSerializer)
    {
        rapidjson::Document::AllocatorType& a = document.GetAllocator();
        rapidjson::Value skinValue(rapidjson::kObjectType);
        {
            RapidJsonUtils::AddOptionalMemberIndex("inverseBindMatrices", skinValue, skin.inverseBindMatricesAccessorId, gltfDocument.accessors, a);
            RapidJsonUtils::AddOptionalMemberIndex("skeleton", skinValue, skin.skeletonId, gltfDocument.nodes, a);

            if (!skin.jointIds.empty())
            {
                rapidjson::Value jointIds(rapidjson::kArrayType);
                {
                    for (const auto& jointId : skin.jointIds)
                    {
                        jointIds.PushBack(ToKnownSizeType(gltfDocument.nodes.GetIndex(jointId)), a);
                    }
                }
                skinValue.AddMember("joints", jointIds, a);
            }

            RapidJsonUtils::AddOptionalMember("name", skinValue, skin.name, a);

            SerializeProperty(gltfDocument, skin, skinValue, a, extensionSerializer);
        }
        return skinValue;
    }

    rapidjson::Value SerializeTexture(const Texture& texture, const Document& gltfDocument, rapidjson::Document& document, const ExtensionSerializer& extensionSerializer)
    {
        rapidjson::Document::AllocatorType& a = document.GetAllocator();
        rapidjson::Value textureValue(rapidjson::kObjectType);

        RapidJsonUtils::AddOptionalMember("name", textureValue, texture.name, a);
        RapidJsonUtils::AddOptionalMemberIndex("sampler", textureValue, texture.samplerId, gltfDocument.samplers, a);
        RapidJsonUtils::AddOptionalMemberIndex("source", textureValue, texture.imageId, gltfDocument.images, a);

        SerializeProperty(gltfDocument, texture, textureValue, a, extensionSerializer);

        return textureValue;
    }

    void SerializeExtensions(const Document& gltfDocument, rapidjson::Document& document, const ExtensionSerializer& extensionSerializer)
    {
        rapidjson::Document::AllocatorType& a = document.GetAllocator();
        rapidjson::Value extensionValue(rapidjson::kObjectType);

        SerializePropertyExtensions(gltfDocument, gltfDocument, extensionValue, a, extensionSerializer);

        if (extensionValue.HasMember("extensions"))
        {
            auto& value = extensionValue.FindMember("extensions")->value;
            document.AddMember("extensions", value, a);
        }
    }

    void SerializeStringSet(const std::string& key, const std::unordered_set<std::string> set, rapidjson::Document& document)
    {
        if (!set.empty())
        {
            rapidjson::Document::AllocatorType& a = document.GetAllocator();
            rapidjson::Value extensions(rapidjson::kArrayType);
            for (const auto& element : set)
            {
                extensions.PushBack(RapidJsonUtils::ToStringValue(element, a), a);
            }
            document.AddMember(RapidJsonUtils::ToStringValue(key, a), extensions, a);
        }
    }

    void SerializeExtensionsUsed(const Document& gltfDocument, rapidjson::Document& document)
    {
        SerializeStringSet("extensionsUsed", gltfDocument.extensionsUsed, document);
    }

    void SerializeExtensionsRequired(const Document& gltfDocument, rapidjson::Document& document)
    {
        for (auto& extensionName : gltfDocument.extensionsRequired)
        {
            if (gltfDocument.extensionsUsed.find(extensionName) == gltfDocument.extensionsUsed.end())
            {
                throw GLTFException("required extension '" + extensionName + "' not present in extensionsUsed.");
            }
        }
        
        SerializeStringSet("extensionsRequired", gltfDocument.extensionsRequired, document);
    }

    rapidjson::Document CreateJsonDocument(const Document& gltfDocument, const ExtensionSerializer& extensionSerializer)
    {
        rapidjson::Document document(rapidjson::kObjectType);

        SerializeAsset(gltfDocument, document, extensionSerializer);

        SerializeIndexedContainer<Accessor>("accessors", gltfDocument.accessors, gltfDocument, document, extensionSerializer, SerializeAccessor);
        SerializeIndexedContainer<Animation>("animations", gltfDocument.animations, gltfDocument, document, extensionSerializer, SerializeAnimation);
        SerializeIndexedContainer<BufferView>("bufferViews", gltfDocument.bufferViews, gltfDocument, document, extensionSerializer, SerializeBufferView);
        SerializeIndexedContainer<Buffer>("buffers", gltfDocument.buffers, gltfDocument, document, extensionSerializer, SerializeBuffer);
        SerializeIndexedContainer<Camera>("cameras", gltfDocument.cameras, gltfDocument, document, extensionSerializer, SerializeCamera);
        SerializeIndexedContainer<Image>("images", gltfDocument.images, gltfDocument, document, extensionSerializer, SerializeImage);
        SerializeIndexedContainer<Material>("materials", gltfDocument.materials, gltfDocument, document, extensionSerializer, SerializeMaterial);
        SerializeIndexedContainer<Mesh>("meshes", gltfDocument.meshes, gltfDocument, document, extensionSerializer, SerializeMesh);
        SerializeIndexedContainer<Node>("nodes", gltfDocument.nodes, gltfDocument, document, extensionSerializer, SerializeNode);
        SerializeIndexedContainer<Sampler>("samplers", gltfDocument.samplers, gltfDocument, document, extensionSerializer, SerializeSampler);
        SerializeIndexedContainer<Scene>("scenes", gltfDocument.scenes, gltfDocument, document, extensionSerializer, SerializeScene);
        SerializeIndexedContainer<Skin>("skins", gltfDocument.skins, gltfDocument, document, extensionSerializer, SerializeSkin);
        SerializeIndexedContainer<Texture>("textures", gltfDocument.textures, gltfDocument, document, extensionSerializer, SerializeTexture);

        SerializeDefaultScene(gltfDocument, document);

        SerializeExtensions(gltfDocument, document, extensionSerializer);

        SerializeExtensionsUsed(gltfDocument, document);
        SerializeExtensionsRequired(gltfDocument, document);

        return document;
    }

    bool HasFlag(SerializeFlags flags, SerializeFlags flag)
    {
        return ((flags & flag) == flag);
    }
}

std::string Microsoft::glTF::Serialize(const Document& gltfDocument, SerializeFlags flags)
{
    return Serialize(gltfDocument, ExtensionSerializer(), flags);
}

std::string Microsoft::glTF::Serialize(const Document& gltfDocument, const ExtensionSerializer& extensionSerializer, SerializeFlags flags)
{
    auto doc = CreateJsonDocument(gltfDocument, extensionSerializer);

    rapidjson::StringBuffer stringBuffer;
    if (HasFlag(flags, SerializeFlags::Pretty))
    {
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(stringBuffer);
        doc.Accept(writer);
    }
    else
    {
        rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);
        doc.Accept(writer);
    }

    return stringBuffer.GetString();
}

SerializeFlags Microsoft::glTF::operator|(SerializeFlags lhs, SerializeFlags rhs)
{
    const auto result =
        static_cast<std::underlying_type_t<SerializeFlags>>(lhs) |
        static_cast<std::underlying_type_t<SerializeFlags>>(rhs);

    return static_cast<SerializeFlags>(result);
}

SerializeFlags& Microsoft::glTF::operator|=(SerializeFlags& lhs, SerializeFlags rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

SerializeFlags Microsoft::glTF::operator&(SerializeFlags lhs, SerializeFlags rhs)
{
    const auto result =
        static_cast<std::underlying_type_t<SerializeFlags>>(lhs) &
        static_cast<std::underlying_type_t<SerializeFlags>>(rhs);

    return static_cast<SerializeFlags>(result);
}

SerializeFlags& Microsoft::glTF::operator&=(SerializeFlags& lhs, SerializeFlags rhs)
{
    lhs = lhs & rhs;
    return lhs;
}
