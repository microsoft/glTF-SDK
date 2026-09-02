// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/Serialize.h>

#include <GLTFSDK/Document.h>
#include <GLTFSDK/ExtensionHandlers.h>
#include <GLTFSDK/GLTF.h>

#include "Internal/Json.h"

#include <algorithm>
#include <array>
#include <initializer_list>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace Microsoft::glTF;

namespace
{
    using JsonValue = Internal::JsonValue;

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

    std::string InterpolationTypeToString(
        InterpolationType interpolationType)
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

    template<typename Iterator>
    JsonValue CreateFloatArray(
        Iterator begin,
        Iterator end)
    {
        JsonValue array = Internal::CreateJsonArray();
        for (Iterator iterator = begin; iterator != end; ++iterator)
        {
            Internal::AppendJsonValue(
                array,
                Internal::CreateJsonFloat(*iterator));
        }
        return array;
    }

    JsonValue CreateFloatArray(const std::vector<float>& values)
    {
        return CreateFloatArray(values.begin(), values.end());
    }

    template<std::size_t Size>
    JsonValue CreateFloatArray(
        const std::array<float, Size>& values)
    {
        return CreateFloatArray(values.begin(), values.end());
    }

    JsonValue CreateFloatArray(
        std::initializer_list<float> values)
    {
        return CreateFloatArray(values.begin(), values.end());
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

    JsonValue CreateFloatArray(const Vector3& value)
    {
        return CreateFloatArray({value.x, value.y, value.z});
    }

    JsonValue CreateFloatArray(const Quaternion& value)
    {
        return CreateFloatArray(
            {value.x, value.y, value.z, value.w});
    }

    void AddOptionalString(
        JsonValue& object,
        const char* name,
        const std::string& value)
    {
        if (!value.empty())
        {
            Internal::SetJsonMember(
                object,
                name,
                Internal::CreateJsonString(value));
        }
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

    void AddFloatArray(
        JsonValue& object,
        const char* name,
        const std::vector<float>& values)
    {
        if (!values.empty())
        {
            Internal::SetJsonMember(
                object,
                name,
                CreateFloatArray(values));
        }
    }

    void SerializePropertyExtensions(
        const Document& document,
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
        std::vector<ExtensionPair> serializedRegistered;
        serializedRegistered.reserve(registeredExtensions.size());
        for (const auto& extension : registeredExtensions)
        {
            auto extensionPair =
                extensionSerializer.Serialize(
                    extension, property, document);
            if (property.HasUnregisteredExtension(
                    extensionPair.name))
            {
                throw GLTFException(
                    "Registered extension '" +
                    extensionPair.name +
                    "' is also present as an unregistered extension.");
            }
            if (document.extensionsUsed.find(extensionPair.name) ==
                document.extensionsUsed.end())
            {
                throw GLTFException(
                    "Registered extension '" +
                    extensionPair.name +
                    "' is not present in extensionsUsed");
            }
            serializedRegistered.push_back(
                std::move(extensionPair));
        }
        std::sort(
            serializedRegistered.begin(),
            serializedRegistered.end(),
            [](const ExtensionPair& left, const ExtensionPair& right)
            {
                return left.name < right.name;
            });
        for (const auto& extensionPair : serializedRegistered)
        {
            Internal::SetJsonMember(
                extensions,
                extensionPair.name,
                Internal::ParseJson(extensionPair.value));
        }

        std::vector<std::pair<std::string, std::string>>
            serializedUnregistered(
                property.extensions.begin(),
                property.extensions.end());
        std::sort(
            serializedUnregistered.begin(),
            serializedUnregistered.end(),
            [](const std::pair<std::string, std::string>& left,
               const std::pair<std::string, std::string>& right)
            {
                return left.first < right.first;
            });
        for (const auto& extension : serializedUnregistered)
        {
            if (document.extensionsUsed.find(extension.first) ==
                document.extensionsUsed.end())
            {
                throw GLTFException(
                    "Unregistered extension '" +
                    extension.first +
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
        const Document& document,
        const glTFProperty& property,
        JsonValue& propertyValue,
        const ExtensionSerializer& extensionSerializer)
    {
        SerializePropertyExtensions(
            document,
            property,
            propertyValue,
            extensionSerializer);
        SerializePropertyExtras(property, propertyValue);
    }

    void SerializeTextureInfo(
        const Document& document,
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
            document,
            textureInfo,
            textureValue,
            extensionSerializer);
    }

    void SerializeAsset(
        const Document& gltfDocument,
        JsonValue& document,
        const ExtensionSerializer& extensionSerializer)
    {
        JsonValue asset = Internal::CreateJsonObject();
        AddOptionalString(
            asset, "copyright", gltfDocument.asset.copyright);
        AddOptionalString(
            asset, "generator", gltfDocument.asset.generator);
        Internal::SetJsonMember(
            asset,
            "version",
            Internal::CreateJsonString(
                gltfDocument.asset.version));
        AddOptionalString(
            asset, "minVersion", gltfDocument.asset.minVersion);
        SerializeProperty(
            gltfDocument,
            gltfDocument.asset,
            asset,
            extensionSerializer);
        Internal::SetJsonMember(
            document, "asset", std::move(asset));
    }

    void SerializeDefaultScene(
        const Document& gltfDocument,
        JsonValue& document)
    {
        if (gltfDocument.HasDefaultScene())
        {
            Internal::SetJsonMember(
                document,
                "scene",
                Internal::CreateJsonSize(
                    gltfDocument.scenes.GetIndex(
                        gltfDocument.defaultSceneId)));
        }
    }

    template<typename T>
    void SerializeIndexedContainer(
        const char* name,
        const IndexedContainer<const T>& indexedContainer,
        const Document& gltfDocument,
        JsonValue& document,
        const ExtensionSerializer& extensionSerializer,
        JsonValue(*serialize)(
            const T&,
            const Document&,
            const ExtensionSerializer&))
    {
        if (indexedContainer.Size() == 0U)
        {
            return;
        }

        JsonValue values = Internal::CreateJsonArray();
        for (const auto& element : indexedContainer.Elements())
        {
            Internal::AppendJsonValue(
                values,
                serialize(
                    element,
                    gltfDocument,
                    extensionSerializer));
        }
        Internal::SetJsonMember(
            document, name, std::move(values));
    }

    JsonValue SerializeAccessor(
        const Accessor& accessor,
        const Document& gltfDocument,
        const ExtensionSerializer& extensionSerializer)
    {
        JsonValue value = Internal::CreateJsonObject();
        AddOptionalString(value, "name", accessor.name);

        if (accessor.sparse.count > 0U)
        {
            if (!accessor.bufferViewId.empty())
            {
                Internal::SetJsonMember(
                    value,
                    "bufferView",
                    Internal::CreateJsonSize(
                        gltfDocument.bufferViews.GetIndex(
                            accessor.bufferViewId)));
            }

            JsonValue sparse = Internal::CreateJsonObject();
            JsonValue indices = Internal::CreateJsonObject();
            JsonValue values = Internal::CreateJsonObject();

            Internal::SetJsonMember(
                indices,
                "bufferView",
                Internal::CreateJsonSize(
                    gltfDocument.bufferViews.GetIndex(
                        accessor.sparse.indicesBufferViewId)));
            if (accessor.sparse.indicesByteOffset != 0U)
            {
                Internal::SetJsonMember(
                    indices,
                    "byteOffset",
                    Internal::CreateJsonSize(
                        accessor.sparse.indicesByteOffset));
            }
            Internal::SetJsonMember(
                indices,
                "componentType",
                Internal::CreateJsonInt32(
                    accessor.sparse.indicesComponentType));

            Internal::SetJsonMember(
                values,
                "bufferView",
                Internal::CreateJsonSize(
                    gltfDocument.bufferViews.GetIndex(
                        accessor.sparse.valuesBufferViewId)));
            if (accessor.sparse.valuesByteOffset != 0U)
            {
                Internal::SetJsonMember(
                    values,
                    "byteOffset",
                    Internal::CreateJsonSize(
                        accessor.sparse.valuesByteOffset));
            }

            Internal::SetJsonMember(
                sparse,
                "count",
                Internal::CreateJsonSize(accessor.sparse.count));
            Internal::SetJsonMember(
                sparse, "indices", std::move(indices));
            Internal::SetJsonMember(
                sparse, "values", std::move(values));
            Internal::SetJsonMember(
                value, "sparse", std::move(sparse));
        }
        else
        {
            AddOptionalIndex(
                value,
                "bufferView",
                accessor.bufferViewId,
                gltfDocument.bufferViews);
        }

        if (accessor.byteOffset != 0U)
        {
            Internal::SetJsonMember(
                value,
                "byteOffset",
                Internal::CreateJsonSize(accessor.byteOffset));
        }
        if (accessor.normalized)
        {
            Internal::SetJsonMember(
                value,
                "normalized",
                Internal::CreateJsonBoolean(true));
        }

        Internal::SetJsonMember(
            value,
            "componentType",
            Internal::CreateJsonInt32(accessor.componentType));
        Internal::SetJsonMember(
            value,
            "count",
            Internal::CreateJsonSize(accessor.count));
        Internal::SetJsonMember(
            value,
            "type",
            Internal::CreateJsonString(
                AccessorTypeToString(accessor.type)));

        if (!accessor.max.empty())
        {
            Internal::SetJsonMember(
                value, "max", CreateFloatArray(accessor.max));
        }
        if (!accessor.min.empty())
        {
            Internal::SetJsonMember(
                value, "min", CreateFloatArray(accessor.min));
        }

        SerializeProperty(
            gltfDocument,
            accessor,
            value,
            extensionSerializer);
        return value;
    }

    JsonValue SerializeAnimation(
        const Animation& animation,
        const Document& gltfDocument,
        const ExtensionSerializer& extensionSerializer)
    {
        JsonValue channels = Internal::CreateJsonArray();
        for (const auto& channel : animation.channels.Elements())
        {
            JsonValue channelValue =
                Internal::CreateJsonObject();
            JsonValue target = Internal::CreateJsonObject();
            AddOptionalIndex(
                target,
                "node",
                channel.target.nodeId,
                gltfDocument.nodes);
            Internal::SetJsonMember(
                target,
                "path",
                Internal::CreateJsonString(
                    TargetPathToString(channel.target.path)));
            Internal::SetJsonMember(
                channelValue,
                "sampler",
                Internal::CreateJsonSize(
                    animation.samplers.GetIndex(
                        channel.samplerId)));
            Internal::SetJsonMember(
                channelValue, "target", std::move(target));
            SerializeProperty(
                gltfDocument,
                channel,
                channelValue,
                extensionSerializer);
            Internal::AppendJsonValue(
                channels, std::move(channelValue));
        }

        JsonValue samplers = Internal::CreateJsonArray();
        for (const auto& sampler : animation.samplers.Elements())
        {
            JsonValue samplerValue =
                Internal::CreateJsonObject();
            Internal::SetJsonMember(
                samplerValue,
                "input",
                Internal::CreateJsonSize(
                    gltfDocument.accessors.GetIndex(
                        sampler.inputAccessorId)));
            AddOptionalString(
                samplerValue,
                "interpolation",
                InterpolationTypeToString(
                    sampler.interpolation));
            Internal::SetJsonMember(
                samplerValue,
                "output",
                Internal::CreateJsonSize(
                    gltfDocument.accessors.GetIndex(
                        sampler.outputAccessorId)));
            SerializeProperty(
                gltfDocument,
                sampler,
                samplerValue,
                extensionSerializer);
            Internal::AppendJsonValue(
                samplers, std::move(samplerValue));
        }

        JsonValue value = Internal::CreateJsonObject();
        Internal::SetJsonMember(
            value, "channels", std::move(channels));
        Internal::SetJsonMember(
            value, "samplers", std::move(samplers));
        AddOptionalString(value, "name", animation.name);
        SerializeProperty(
            gltfDocument,
            animation,
            value,
            extensionSerializer);
        return value;
    }

    JsonValue SerializeBufferView(
        const BufferView& bufferView,
        const Document& gltfDocument,
        const ExtensionSerializer& extensionSerializer)
    {
        JsonValue value = Internal::CreateJsonObject();
        AddOptionalString(value, "name", bufferView.name);
        Internal::SetJsonMember(
            value,
            "buffer",
            Internal::CreateJsonSize(
                gltfDocument.buffers.GetIndex(
                    bufferView.bufferId)));
        Internal::SetJsonMember(
            value,
            "byteOffset",
            Internal::CreateJsonSize(bufferView.byteOffset));
        Internal::SetJsonMember(
            value,
            "byteLength",
            Internal::CreateJsonSize(bufferView.byteLength));
        if (bufferView.byteStride)
        {
            Internal::SetJsonMember(
                value,
                "byteStride",
                Internal::CreateJsonSize(
                    bufferView.byteStride.Get()));
        }
        if (bufferView.target)
        {
            Internal::SetJsonMember(
                value,
                "target",
                Internal::CreateJsonInt32(
                    bufferView.target.Get()));
        }
        SerializeProperty(
            gltfDocument,
            bufferView,
            value,
            extensionSerializer);
        return value;
    }

    JsonValue SerializeBuffer(
        const Buffer& buffer,
        const Document& gltfDocument,
        const ExtensionSerializer& extensionSerializer)
    {
        JsonValue value = Internal::CreateJsonObject();
        Internal::SetJsonMember(
            value,
            "byteLength",
            Internal::CreateJsonSize(buffer.byteLength));
        AddOptionalString(value, "uri", buffer.uri);
        SerializeProperty(
            gltfDocument,
            buffer,
            value,
            extensionSerializer);
        return value;
    }

    JsonValue SerializeImage(
        const Image& image,
        const Document& gltfDocument,
        const ExtensionSerializer& extensionSerializer)
    {
        if (image.uri.empty())
        {
            if (image.bufferViewId.empty() ||
                image.mimeType.empty())
            {
                throw InvalidGLTFException(
                    "Invalid image: " + image.id +
                    ". Images must have either a uri or a "
                    "bufferView and a mimeType.");
            }
        }
        else if (!image.bufferViewId.empty())
        {
            throw InvalidGLTFException(
                "Invalid image: " + image.id +
                ". Images can only have a uri or a bufferView, "
                "but not both.");
        }

        JsonValue value = Internal::CreateJsonObject();
        AddOptionalString(value, "name", image.name);
        AddOptionalString(value, "uri", image.uri);
        AddOptionalIndex(
            value,
            "bufferView",
            image.bufferViewId,
            gltfDocument.bufferViews);
        AddOptionalString(value, "mimeType", image.mimeType);
        SerializeProperty(
            gltfDocument,
            image,
            value,
            extensionSerializer);
        return value;
    }

    JsonValue SerializeMaterial(
        const Material& material,
        const Document& gltfDocument,
        const ExtensionSerializer& extensionSerializer)
    {
        JsonValue value = Internal::CreateJsonObject();
        JsonValue metallicRoughness =
            Internal::CreateJsonObject();

        if (material.metallicRoughness.baseColorFactor !=
            Color4(1.0F, 1.0F, 1.0F, 1.0F))
        {
            Internal::SetJsonMember(
                metallicRoughness,
                "baseColorFactor",
                CreateFloatArray(
                    material.metallicRoughness
                        .baseColorFactor));
        }
        if (!material.metallicRoughness
                 .baseColorTexture.textureId.empty())
        {
            JsonValue texture = Internal::CreateJsonObject();
            SerializeTextureInfo(
                gltfDocument,
                material.metallicRoughness.baseColorTexture,
                texture,
                gltfDocument.textures,
                extensionSerializer);
            Internal::SetJsonMember(
                metallicRoughness,
                "baseColorTexture",
                std::move(texture));
        }
        if (material.metallicRoughness.metallicFactor != 1.0F)
        {
            Internal::SetJsonMember(
                metallicRoughness,
                "metallicFactor",
                Internal::CreateJsonFloat(
                    material.metallicRoughness
                        .metallicFactor));
        }
        if (material.metallicRoughness.roughnessFactor != 1.0F)
        {
            Internal::SetJsonMember(
                metallicRoughness,
                "roughnessFactor",
                Internal::CreateJsonFloat(
                    material.metallicRoughness
                        .roughnessFactor));
        }
        if (!material.metallicRoughness
                 .metallicRoughnessTexture.textureId.empty())
        {
            JsonValue texture = Internal::CreateJsonObject();
            SerializeTextureInfo(
                gltfDocument,
                material.metallicRoughness
                    .metallicRoughnessTexture,
                texture,
                gltfDocument.textures,
                extensionSerializer);
            Internal::SetJsonMember(
                metallicRoughness,
                "metallicRoughnessTexture",
                std::move(texture));
        }
        if (!Internal::GetJsonObjectMemberNames(
                metallicRoughness,
                "pbrMetallicRoughness must be an object").empty())
        {
            Internal::SetJsonMember(
                value,
                "pbrMetallicRoughness",
                std::move(metallicRoughness));
        }

        if (!material.normalTexture.textureId.empty())
        {
            JsonValue texture = Internal::CreateJsonObject();
            SerializeTextureInfo(
                gltfDocument,
                material.normalTexture,
                texture,
                gltfDocument.textures,
                extensionSerializer);
            if (material.normalTexture.scale != 1.0F)
            {
                Internal::SetJsonMember(
                    texture,
                    "scale",
                    Internal::CreateJsonFloat(
                        material.normalTexture.scale));
            }
            Internal::SetJsonMember(
                value,
                "normalTexture",
                std::move(texture));
        }

        if (!material.occlusionTexture.textureId.empty())
        {
            JsonValue texture = Internal::CreateJsonObject();
            SerializeTextureInfo(
                gltfDocument,
                material.occlusionTexture,
                texture,
                gltfDocument.textures,
                extensionSerializer);
            if (material.occlusionTexture.strength != 1.0F)
            {
                Internal::SetJsonMember(
                    texture,
                    "strength",
                    Internal::CreateJsonFloat(
                        material.occlusionTexture.strength));
            }
            Internal::SetJsonMember(
                value,
                "occlusionTexture",
                std::move(texture));
        }

        if (!material.emissiveTexture.textureId.empty())
        {
            JsonValue texture = Internal::CreateJsonObject();
            SerializeTextureInfo(
                gltfDocument,
                material.emissiveTexture,
                texture,
                gltfDocument.textures,
                extensionSerializer);
            Internal::SetJsonMember(
                value,
                "emissiveTexture",
                std::move(texture));
        }

        if (material.emissiveFactor !=
            Color3(0.0F, 0.0F, 0.0F))
        {
            Internal::SetJsonMember(
                value,
                "emissiveFactor",
                CreateFloatArray(material.emissiveFactor));
        }
        if (material.alphaMode != ALPHA_OPAQUE &&
            material.alphaMode != ALPHA_UNKNOWN)
        {
            Internal::SetJsonMember(
                value,
                "alphaMode",
                Internal::CreateJsonString(
                    AlphaModeToString(material.alphaMode)));
        }
        if (material.alphaCutoff != 0.5F)
        {
            Internal::SetJsonMember(
                value,
                "alphaCutoff",
                Internal::CreateJsonFloat(
                    material.alphaCutoff));
        }
        AddOptionalString(value, "name", material.name);
        if (material.doubleSided)
        {
            Internal::SetJsonMember(
                value,
                "doubleSided",
                Internal::CreateJsonBoolean(true));
        }

        SerializeProperty(
            gltfDocument,
            material,
            value,
            extensionSerializer);
        return value;
    }

    JsonValue SerializeTarget(
        const MorphTarget& target,
        const Document& gltfDocument)
    {
        JsonValue value = Internal::CreateJsonObject();
        AddOptionalIndex(
            value,
            ACCESSOR_POSITION,
            target.positionsAccessorId,
            gltfDocument.accessors);
        AddOptionalIndex(
            value,
            ACCESSOR_NORMAL,
            target.normalsAccessorId,
            gltfDocument.accessors);
        AddOptionalIndex(
            value,
            ACCESSOR_TANGENT,
            target.tangentsAccessorId,
            gltfDocument.accessors);
        return value;
    }

    void SerializeTargets(
        const MeshPrimitive& primitive,
        JsonValue& primitiveValue,
        const Document& gltfDocument)
    {
        if (primitive.targets.empty())
        {
            return;
        }

        JsonValue targets = Internal::CreateJsonArray();
        for (const auto& target : primitive.targets)
        {
            Internal::AppendJsonValue(
                targets,
                SerializeTarget(target, gltfDocument));
        }
        Internal::SetJsonMember(
            primitiveValue,
            "targets",
            std::move(targets));
    }

    JsonValue SerializeMesh(
        const Mesh& mesh,
        const Document& gltfDocument,
        const ExtensionSerializer& extensionSerializer)
    {
        JsonValue primitiveValues =
            Internal::CreateJsonArray();
        for (const auto& primitive : mesh.primitives)
        {
            JsonValue attributes =
                Internal::CreateJsonObject();
            std::vector<std::pair<std::string, std::string>>
                sortedAttributes(
                    primitive.attributes.begin(),
                    primitive.attributes.end());
            std::sort(
                sortedAttributes.begin(),
                sortedAttributes.end(),
                [](const std::pair<std::string, std::string>& left,
                   const std::pair<std::string, std::string>& right)
                {
                    return left.first < right.first;
                });
            for (const auto& attribute : sortedAttributes)
            {
                Internal::SetJsonMember(
                    attributes,
                    attribute.first,
                    Internal::CreateJsonSize(
                        gltfDocument.accessors.GetIndex(
                            attribute.second)));
            }

            JsonValue primitiveValue =
                Internal::CreateJsonObject();
            Internal::SetJsonMember(
                primitiveValue,
                "attributes",
                std::move(attributes));
            AddOptionalIndex(
                primitiveValue,
                "indices",
                primitive.indicesAccessorId,
                gltfDocument.accessors);
            AddOptionalIndex(
                primitiveValue,
                "material",
                primitive.materialId,
                gltfDocument.materials);
            if (primitive.mode != MESH_TRIANGLES)
            {
                Internal::SetJsonMember(
                    primitiveValue,
                    "mode",
                    Internal::CreateJsonInt32(primitive.mode));
            }
            SerializeTargets(
                primitive,
                primitiveValue,
                gltfDocument);
            SerializeProperty(
                gltfDocument,
                primitive,
                primitiveValue,
                extensionSerializer);
            Internal::AppendJsonValue(
                primitiveValues,
                std::move(primitiveValue));
        }

        JsonValue value = Internal::CreateJsonObject();
        AddFloatArray(value, "weights", mesh.weights);
        AddOptionalString(value, "name", mesh.name);
        Internal::SetJsonMember(
            value,
            "primitives",
            std::move(primitiveValues));
        SerializeProperty(
            gltfDocument,
            mesh,
            value,
            extensionSerializer);
        return value;
    }

    JsonValue SerializeNode(
        const Node& node,
        const Document& gltfDocument,
        const ExtensionSerializer& extensionSerializer)
    {
        JsonValue value = Internal::CreateJsonObject();
        if (!node.children.empty())
        {
            JsonValue children = Internal::CreateJsonArray();
            for (const auto& childId : node.children)
            {
                Internal::AppendJsonValue(
                    children,
                    Internal::CreateJsonSize(
                        gltfDocument.nodes.GetIndex(childId)));
            }
            Internal::SetJsonMember(
                value, "children", std::move(children));
        }

        if (!node.HasValidTransformType())
        {
            throw DocumentException(
                "Node " + node.id +
                " doesn't have a valid transform type");
        }

        if (node.GetTransformationType() ==
            TransformationType::TRANSFORMATION_MATRIX)
        {
            Internal::SetJsonMember(
                value,
                "matrix",
                CreateFloatArray(node.matrix.values));
        }
        else if (node.GetTransformationType() ==
                 TransformationType::TRANSFORMATION_TRS)
        {
            if (node.translation != Vector3::ZERO)
            {
                Internal::SetJsonMember(
                    value,
                    "translation",
                    CreateFloatArray(node.translation));
            }
            if (node.rotation != Quaternion::IDENTITY)
            {
                Internal::SetJsonMember(
                    value,
                    "rotation",
                    CreateFloatArray(node.rotation));
            }
            if (node.scale != Vector3::ONE)
            {
                Internal::SetJsonMember(
                    value,
                    "scale",
                    CreateFloatArray(node.scale));
            }
        }

        AddOptionalIndex(
            value,
            "mesh",
            node.meshId,
            gltfDocument.meshes);
        AddOptionalIndex(
            value,
            "skin",
            node.skinId,
            gltfDocument.skins);
        AddOptionalIndex(
            value,
            "camera",
            node.cameraId,
            gltfDocument.cameras);
        AddFloatArray(value, "weights", node.weights);
        AddOptionalString(value, "name", node.name);
        SerializeProperty(
            gltfDocument,
            node,
            value,
            extensionSerializer);
        return value;
    }

    JsonValue SerializeCamera(
        const Camera& camera,
        const Document& gltfDocument,
        const ExtensionSerializer& extensionSerializer)
    {
        JsonValue value = Internal::CreateJsonObject();
        JsonValue projection = Internal::CreateJsonObject();
        const ProjectionType type =
            camera.projection->GetProjectionType();

        if (type == PROJECTION_PERSPECTIVE)
        {
            const auto& perspective = camera.GetPerspective();
            Internal::SetJsonMember(
                projection,
                "znear",
                Internal::CreateJsonFloat(perspective.znear));
            Internal::SetJsonMember(
                projection,
                "yfov",
                Internal::CreateJsonFloat(perspective.yfov));
            if (perspective.zfar)
            {
                Internal::SetJsonMember(
                    projection,
                    "zfar",
                    Internal::CreateJsonFloat(
                        perspective.zfar.Get()));
            }
            if (perspective.aspectRatio)
            {
                Internal::SetJsonMember(
                    projection,
                    "aspectRatio",
                    Internal::CreateJsonFloat(
                        perspective.aspectRatio.Get()));
            }
            SerializeProperty(
                gltfDocument,
                perspective,
                projection,
                extensionSerializer);
            Internal::SetJsonMember(
                value,
                "perspective",
                std::move(projection));
            Internal::SetJsonMember(
                value,
                "type",
                Internal::CreateJsonString("perspective"));
        }
        else if (type == PROJECTION_ORTHOGRAPHIC)
        {
            const auto& orthographic =
                camera.GetOrthographic();
            Internal::SetJsonMember(
                projection,
                "xmag",
                Internal::CreateJsonFloat(orthographic.xmag));
            Internal::SetJsonMember(
                projection,
                "ymag",
                Internal::CreateJsonFloat(orthographic.ymag));
            Internal::SetJsonMember(
                projection,
                "znear",
                Internal::CreateJsonFloat(orthographic.znear));
            Internal::SetJsonMember(
                projection,
                "zfar",
                Internal::CreateJsonFloat(orthographic.zfar));
            SerializeProperty(
                gltfDocument,
                orthographic,
                projection,
                extensionSerializer);
            Internal::SetJsonMember(
                value,
                "orthographic",
                std::move(projection));
            Internal::SetJsonMember(
                value,
                "type",
                Internal::CreateJsonString("orthographic"));
        }
        else
        {
            throw DocumentException(
                "Camera " + camera.id +
                " doesn't have a valid projection type");
        }

        SerializeProperty(
            gltfDocument,
            camera,
            value,
            extensionSerializer);
        AddOptionalString(value, "name", camera.name);
        return value;
    }

    JsonValue SerializeSampler(
        const Sampler& sampler,
        const Document& gltfDocument,
        const ExtensionSerializer& extensionSerializer)
    {
        JsonValue value = Internal::CreateJsonObject();
        AddOptionalString(value, "name", sampler.name);
        if (sampler.magFilter)
        {
            Internal::SetJsonMember(
                value,
                "magFilter",
                Internal::CreateJsonInt32(
                    sampler.magFilter.Get()));
        }
        if (sampler.minFilter)
        {
            Internal::SetJsonMember(
                value,
                "minFilter",
                Internal::CreateJsonInt32(
                    sampler.minFilter.Get()));
        }
        if (sampler.wrapS != WrapMode::Wrap_REPEAT)
        {
            Internal::SetJsonMember(
                value,
                "wrapS",
                Internal::CreateJsonInt32(sampler.wrapS));
        }
        if (sampler.wrapT != WrapMode::Wrap_REPEAT)
        {
            Internal::SetJsonMember(
                value,
                "wrapT",
                Internal::CreateJsonInt32(sampler.wrapT));
        }
        SerializeProperty(
            gltfDocument,
            sampler,
            value,
            extensionSerializer);
        return value;
    }

    JsonValue SerializeScene(
        const Scene& scene,
        const Document& gltfDocument,
        const ExtensionSerializer& extensionSerializer)
    {
        JsonValue value = Internal::CreateJsonObject();
        if (!scene.nodes.empty())
        {
            JsonValue nodes = Internal::CreateJsonArray();
            for (const auto& nodeId : scene.nodes)
            {
                Internal::AppendJsonValue(
                    nodes,
                    Internal::CreateJsonSize(
                        gltfDocument.nodes.GetIndex(nodeId)));
            }
            Internal::SetJsonMember(
                value, "nodes", std::move(nodes));
        }
        AddOptionalString(value, "name", scene.name);
        SerializeProperty(
            gltfDocument,
            scene,
            value,
            extensionSerializer);
        return value;
    }

    JsonValue SerializeSkin(
        const Skin& skin,
        const Document& gltfDocument,
        const ExtensionSerializer& extensionSerializer)
    {
        JsonValue value = Internal::CreateJsonObject();
        AddOptionalIndex(
            value,
            "inverseBindMatrices",
            skin.inverseBindMatricesAccessorId,
            gltfDocument.accessors);
        AddOptionalIndex(
            value,
            "skeleton",
            skin.skeletonId,
            gltfDocument.nodes);
        if (!skin.jointIds.empty())
        {
            JsonValue joints = Internal::CreateJsonArray();
            for (const auto& jointId : skin.jointIds)
            {
                Internal::AppendJsonValue(
                    joints,
                    Internal::CreateJsonSize(
                        gltfDocument.nodes.GetIndex(jointId)));
            }
            Internal::SetJsonMember(
                value, "joints", std::move(joints));
        }
        AddOptionalString(value, "name", skin.name);
        SerializeProperty(
            gltfDocument,
            skin,
            value,
            extensionSerializer);
        return value;
    }

    JsonValue SerializeTexture(
        const Texture& texture,
        const Document& gltfDocument,
        const ExtensionSerializer& extensionSerializer)
    {
        JsonValue value = Internal::CreateJsonObject();
        AddOptionalString(value, "name", texture.name);
        AddOptionalIndex(
            value,
            "sampler",
            texture.samplerId,
            gltfDocument.samplers);
        AddOptionalIndex(
            value,
            "source",
            texture.imageId,
            gltfDocument.images);
        SerializeProperty(
            gltfDocument,
            texture,
            value,
            extensionSerializer);
        return value;
    }

    void SerializeRootProperty(
        const Document& gltfDocument,
        JsonValue& document,
        const ExtensionSerializer& extensionSerializer)
    {
        SerializeProperty(
            gltfDocument,
            gltfDocument,
            document,
            extensionSerializer);
    }

    void SerializeStringSet(
        const char* name,
        const std::unordered_set<std::string>& values,
        JsonValue& document)
    {
        if (values.empty())
        {
            return;
        }

        JsonValue array = Internal::CreateJsonArray();
        std::vector<std::string> sortedValues(
            values.begin(), values.end());
        std::sort(sortedValues.begin(), sortedValues.end());
        for (const auto& value : sortedValues)
        {
            Internal::AppendJsonValue(
                array,
                Internal::CreateJsonString(value));
        }
        Internal::SetJsonMember(
            document, name, std::move(array));
    }

    JsonValue CreateJsonDocument(
        const Document& gltfDocument,
        const ExtensionSerializer& extensionSerializer)
    {
        JsonValue document = Internal::CreateJsonObject();
        SerializeAsset(
            gltfDocument, document, extensionSerializer);
        SerializeIndexedContainer<Accessor>(
            "accessors",
            gltfDocument.accessors,
            gltfDocument,
            document,
            extensionSerializer,
            SerializeAccessor);
        SerializeIndexedContainer<Animation>(
            "animations",
            gltfDocument.animations,
            gltfDocument,
            document,
            extensionSerializer,
            SerializeAnimation);
        SerializeIndexedContainer<BufferView>(
            "bufferViews",
            gltfDocument.bufferViews,
            gltfDocument,
            document,
            extensionSerializer,
            SerializeBufferView);
        SerializeIndexedContainer<Buffer>(
            "buffers",
            gltfDocument.buffers,
            gltfDocument,
            document,
            extensionSerializer,
            SerializeBuffer);
        SerializeIndexedContainer<Camera>(
            "cameras",
            gltfDocument.cameras,
            gltfDocument,
            document,
            extensionSerializer,
            SerializeCamera);
        SerializeIndexedContainer<Image>(
            "images",
            gltfDocument.images,
            gltfDocument,
            document,
            extensionSerializer,
            SerializeImage);
        SerializeIndexedContainer<Material>(
            "materials",
            gltfDocument.materials,
            gltfDocument,
            document,
            extensionSerializer,
            SerializeMaterial);
        SerializeIndexedContainer<Mesh>(
            "meshes",
            gltfDocument.meshes,
            gltfDocument,
            document,
            extensionSerializer,
            SerializeMesh);
        SerializeIndexedContainer<Node>(
            "nodes",
            gltfDocument.nodes,
            gltfDocument,
            document,
            extensionSerializer,
            SerializeNode);
        SerializeIndexedContainer<Sampler>(
            "samplers",
            gltfDocument.samplers,
            gltfDocument,
            document,
            extensionSerializer,
            SerializeSampler);
        SerializeIndexedContainer<Scene>(
            "scenes",
            gltfDocument.scenes,
            gltfDocument,
            document,
            extensionSerializer,
            SerializeScene);
        SerializeIndexedContainer<Skin>(
            "skins",
            gltfDocument.skins,
            gltfDocument,
            document,
            extensionSerializer,
            SerializeSkin);
        SerializeIndexedContainer<Texture>(
            "textures",
            gltfDocument.textures,
            gltfDocument,
            document,
            extensionSerializer,
            SerializeTexture);

        SerializeDefaultScene(gltfDocument, document);
        SerializeRootProperty(
            gltfDocument, document, extensionSerializer);
        SerializeStringSet(
            "extensionsUsed",
            gltfDocument.extensionsUsed,
            document);

        for (const auto& extensionName :
             gltfDocument.extensionsRequired)
        {
            if (gltfDocument.extensionsUsed.find(extensionName) ==
                gltfDocument.extensionsUsed.end())
            {
                throw GLTFException(
                    "required extension '" + extensionName +
                    "' not present in extensionsUsed.");
            }
        }
        SerializeStringSet(
            "extensionsRequired",
            gltfDocument.extensionsRequired,
            document);
        return document;
    }

    bool HasFlag(SerializeFlags flags, SerializeFlags flag)
    {
        return (flags & flag) == flag;
    }
}

std::string GLTFSDK_API Microsoft::glTF::Serialize(
    const Document& gltfDocument,
    SerializeFlags flags)
{
    return Serialize(
        gltfDocument,
        ExtensionSerializer(),
        flags);
}

std::string GLTFSDK_API Microsoft::glTF::Serialize(
    const Document& gltfDocument,
    const ExtensionSerializer& extensionSerializer,
    SerializeFlags flags)
{
    const auto document =
        CreateJsonDocument(
            gltfDocument, extensionSerializer);
    return Internal::WriteJson(
        document,
        HasFlag(flags, SerializeFlags::Pretty));
}

SerializeFlags Microsoft::glTF::operator|(
    SerializeFlags lhs,
    SerializeFlags rhs)
{
    const auto result =
        static_cast<std::underlying_type_t<SerializeFlags>>(lhs) |
        static_cast<std::underlying_type_t<SerializeFlags>>(rhs);
    return static_cast<SerializeFlags>(result);
}

SerializeFlags& Microsoft::glTF::operator|=(
    SerializeFlags& lhs,
    SerializeFlags rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

SerializeFlags Microsoft::glTF::operator&(
    SerializeFlags lhs,
    SerializeFlags rhs)
{
    const auto result =
        static_cast<std::underlying_type_t<SerializeFlags>>(lhs) &
        static_cast<std::underlying_type_t<SerializeFlags>>(rhs);
    return static_cast<SerializeFlags>(result);
}

SerializeFlags& Microsoft::glTF::operator&=(
    SerializeFlags& lhs,
    SerializeFlags rhs)
{
    lhs = lhs & rhs;
    return lhs;
}
