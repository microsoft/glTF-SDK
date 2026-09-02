// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/Deserialize.h>

#include <GLTFSDK/Constants.h>
#include <GLTFSDK/ExtensionHandlers.h>
#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/SchemaValidation.h>

#include "Internal/Json.h"
#include "Internal/JsonSchema.h"

#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace Microsoft::glTF;

namespace
{
    using JsonValue = Internal::JsonValue;

    std::string ReadString(
        const JsonValue& value,
        const std::string& error)
    {
        std::string result;
        if (!Internal::TryGetJsonString(value, result))
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

    std::string GetStringMemberOrDefault(
        const JsonValue& object,
        const char* name,
        std::string defaultValue = {})
    {
        const auto* value = Internal::FindJsonMember(object, name);
        if (value == nullptr)
        {
            return defaultValue;
        }

        std::string result;
        if (!Internal::TryGetJsonString(*value, result))
        {
            throw InvalidGLTFException(
                std::string(name) + " must be a string");
        }
        return result;
    }

    bool GetBooleanMemberOrDefault(
        const JsonValue& object,
        const char* name,
        bool defaultValue)
    {
        const auto* value = Internal::FindJsonMember(object, name);
        if (value == nullptr)
        {
            return defaultValue;
        }

        bool result = false;
        if (!Internal::TryGetJsonBoolean(*value, result))
        {
            throw InvalidGLTFException(
                std::string(name) + " must be a boolean");
        }
        return result;
    }

    std::int32_t GetInt32MemberOrDefault(
        const JsonValue& object,
        const char* name,
        std::int32_t defaultValue)
    {
        const auto* value = Internal::FindJsonMember(object, name);
        if (value == nullptr)
        {
            return defaultValue;
        }

        std::int32_t result = 0;
        if (!Internal::TryGetJsonInt32(*value, result))
        {
            throw InvalidGLTFException(
                std::string(name) + " must be a signed integer");
        }
        return result;
    }

    std::uint32_t GetUInt32MemberOrDefault(
        const JsonValue& object,
        const char* name,
        std::uint32_t defaultValue)
    {
        const auto* value = Internal::FindJsonMember(object, name);
        if (value == nullptr)
        {
            return defaultValue;
        }

        std::uint32_t result = 0U;
        if (!Internal::TryGetJsonUInt32(*value, result))
        {
            throw InvalidGLTFException(
                std::string(name) + " must be an unsigned integer");
        }
        return result;
    }

    std::size_t GetSizeMemberOrDefault(
        const JsonValue& object,
        const char* name,
        std::size_t defaultValue = 0U)
    {
        const auto* value = Internal::FindJsonMember(object, name);
        if (value == nullptr)
        {
            return defaultValue;
        }

        std::size_t result = 0U;
        if (!Internal::TryGetJsonSize(*value, result))
        {
            throw InvalidGLTFException(
                std::string(name) + " must be an unsigned integer");
        }
        return result;
    }

    float GetFloatMemberOrDefault(
        const JsonValue& object,
        const char* name,
        float defaultValue)
    {
        const auto* value = Internal::FindJsonMember(object, name);
        if (value == nullptr)
        {
            return defaultValue;
        }

        float result = 0.0F;
        if (!Internal::TryGetJsonFloat(*value, result))
        {
            throw InvalidGLTFException(
                std::string(name) + " must be a finite number");
        }
        return result;
    }

    std::string GetUInt32MemberAsString(
        const JsonValue& object,
        const char* name)
    {
        const auto* value = Internal::FindJsonMember(object, name);
        if (value == nullptr)
        {
            return {};
        }
        return std::to_string(ReadUInt32(
            *value,
            std::string(name) + " must be an unsigned integer"));
    }

    std::string GetSizeMemberAsString(
        const JsonValue& object,
        const char* name)
    {
        const auto* value = Internal::FindJsonMember(object, name);
        if (value == nullptr)
        {
            return {};
        }
        return std::to_string(ReadSize(
            *value,
            std::string(name) + " must be an unsigned integer"));
    }

    std::vector<float> GetFloatArrayMember(
        const JsonValue& object,
        const char* name,
        const std::string& arrayError,
        const std::string& elementError)
    {
        const auto* value = Internal::FindJsonMember(object, name);
        if (value == nullptr)
        {
            return {};
        }

        Internal::RequireJsonArray(*value, arrayError);
        std::vector<float> result;
        const std::size_t size = Internal::GetJsonArraySize(*value);
        result.reserve(size);
        for (std::size_t index = 0U; index < size; ++index)
        {
            result.push_back(ReadFloat(
                Internal::GetJsonArrayElement(
                    *value, index, elementError),
                elementError));
        }
        return result;
    }

    std::vector<float> GetFixedSizeFloatArray(
        const JsonValue& object,
        const char* memberName,
        std::size_t expectedSize,
        const char* error)
    {
        const auto* value =
            Internal::FindJsonMember(object, memberName);
        if (value == nullptr ||
            !Internal::IsJsonArray(*value) ||
            Internal::GetJsonArraySize(*value) != expectedSize)
        {
            throw InvalidGLTFException(error);
        }

        std::vector<float> result;
        result.reserve(expectedSize);
        for (std::size_t index = 0U;
             index < expectedSize;
             ++index)
        {
            float element = 0.0F;
            if (!Internal::TryGetJsonFloat(
                    Internal::GetJsonArrayElement(
                        *value, index, error),
                    element))
            {
                throw InvalidGLTFException(error);
            }
            result.push_back(element);
        }
        return result;
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
            const auto* extension =
                Internal::FindJsonMember(*extensions, name);
            ExtensionPair extensionPair = {
                name,
                Internal::WriteJson(*extension)
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
        const auto& index = Internal::RequireJsonMember(
            value,
            "index",
            "TextureInfo.index was not found");
        textureInfo.textureId = std::to_string(ReadUInt32(
            index,
            "TextureInfo.index must be an unsigned integer"));
        textureInfo.texCoord =
            GetSizeMemberOrDefault(value, "texCoord", 0U);
        ParseProperty(value, textureInfo, extensionDeserializer);
    }

    template<typename T>
    IndexedContainer<const T> DeserializeToIndexedContainer(
        const char* name,
        const JsonValue& value,
        const ExtensionDeserializer& extensionDeserializer,
        T(*parse)(const JsonValue&, const ExtensionDeserializer&))
    {
        IndexedContainer<const T> items;
        const auto* array = Internal::FindJsonMember(value, name);
        if (array == nullptr)
        {
            return items;
        }

        const std::string arrayError =
            std::string(name) + " must be a JSON array";
        Internal::RequireJsonArray(*array, arrayError);
        const std::string elementError =
            std::string(name) +
            " array elements must be JSON objects";
        const std::size_t size = Internal::GetJsonArraySize(*array);

        for (std::size_t index = 0U; index < size; ++index)
        {
            const auto& element = Internal::GetJsonArrayElement(
                *array, index, elementError);
            Internal::RequireJsonObject(element, elementError);
            try
            {
                const auto& item = items.Append(
                    parse(element, extensionDeserializer),
                    AppendIdPolicy::GenerateOnEmpty);
                (void)item;
                assert(item.id == std::to_string(index));
            }
            catch (const InvalidGLTFException& exception)
            {
                std::cerr
                    << "Could not parse " << name << "[" << index
                    << "]: " << exception.what() << "\n";
                throw;
            }
        }

        return items;
    }

    Asset ParseAsset(
        const JsonValue& value,
        const ExtensionDeserializer& extensionDeserializer)
    {
        Asset asset;
        asset.copyright =
            GetStringMemberOrDefault(value, "copyright");
        asset.generator =
            GetStringMemberOrDefault(value, "generator");
        asset.version = ReadString(
            Internal::RequireJsonMember(
                value, "version", "The member version was not found"),
            "Asset version must be a string");
        asset.minVersion =
            GetStringMemberOrDefault(value, "minVersion");
        ParseProperty(value, asset, extensionDeserializer);
        return asset;
    }

    Accessor ParseAccessor(
        const JsonValue& value,
        const ExtensionDeserializer& extensionDeserializer)
    {
        Accessor accessor;
        accessor.name = GetStringMemberOrDefault(value, "name");

        const auto* sparse =
            Internal::FindJsonMember(value, "sparse");
        if (sparse != nullptr)
        {
            Internal::RequireJsonObject(
                *sparse, "Accessor sparse must be a JSON object");
            const auto& sparseIndices = Internal::RequireJsonMember(
                *sparse,
                "indices",
                "The member indices was not found");
            const auto& sparseValues = Internal::RequireJsonMember(
                *sparse,
                "values",
                "The member values was not found");
            Internal::RequireJsonObject(
                sparseIndices,
                "Accessor sparse indices must be a JSON object");
            Internal::RequireJsonObject(
                sparseValues,
                "Accessor sparse values must be a JSON object");

            accessor.sparse.count = ReadSize(
                Internal::RequireJsonMember(
                    *sparse,
                    "count",
                    "The member count was not found"),
                "Accessor sparse count must be an unsigned integer");
            accessor.sparse.indicesBufferViewId = std::to_string(
                ReadUInt32(
                    Internal::RequireJsonMember(
                        sparseIndices,
                        "bufferView",
                        "The member bufferView was not found"),
                    "Accessor sparse indices bufferView must be an "
                    "unsigned integer"));
            accessor.sparse.indicesComponentType =
                Accessor::GetComponentType(ReadUInt32(
                    Internal::RequireJsonMember(
                        sparseIndices,
                        "componentType",
                        "The member componentType was not found"),
                    "Accessor sparse indices componentType must be an "
                    "unsigned integer"));
            accessor.sparse.indicesByteOffset =
                GetSizeMemberOrDefault(
                    sparseIndices, "byteOffset");
            accessor.sparse.valuesBufferViewId = std::to_string(
                ReadUInt32(
                    Internal::RequireJsonMember(
                        sparseValues,
                        "bufferView",
                        "The member bufferView was not found"),
                    "Accessor sparse values bufferView must be an "
                    "unsigned integer"));
            accessor.sparse.valuesByteOffset =
                GetSizeMemberOrDefault(
                    sparseValues, "byteOffset");

            const auto* bufferView =
                Internal::FindJsonMember(value, "bufferView");
            if (bufferView != nullptr)
            {
                accessor.bufferViewId = std::to_string(ReadUInt32(
                    *bufferView,
                    "Accessor bufferView must be an unsigned integer"));
            }
        }
        else
        {
            accessor.bufferViewId =
                GetSizeMemberAsString(value, "bufferView");
        }

        accessor.byteOffset =
            GetSizeMemberOrDefault(value, "byteOffset");
        accessor.componentType = Accessor::GetComponentType(
            ReadUInt32(
                Internal::RequireJsonMember(
                    value,
                    "componentType",
                    "The member componentType was not found"),
                "Accessor componentType must be an unsigned integer"));
        accessor.normalized =
            GetBooleanMemberOrDefault(value, "normalized", false);
        accessor.count = ReadSize(
            Internal::RequireJsonMember(
                value, "count", "The member count was not found"),
            "Accessor count must be an unsigned integer");
        accessor.type = Accessor::ParseType(ReadString(
            Internal::RequireJsonMember(
                value, "type", "The member type was not found"),
            "Accessor type must be a string"));

        accessor.min = GetFloatArrayMember(
            value,
            "min",
            "Accessor min must be a JSON array",
            "Accessor min array elements must be JSON numbers");
        accessor.max = GetFloatArrayMember(
            value,
            "max",
            "Accessor max must be a JSON array",
            "Accessor max array elements must be JSON numbers");

        ParseProperty(value, accessor, extensionDeserializer);
        return accessor;
    }

    BufferView ParseBufferView(
        const JsonValue& value,
        const ExtensionDeserializer& extensionDeserializer)
    {
        BufferView bufferView;
        bufferView.name = GetStringMemberOrDefault(value, "name");
        bufferView.bufferId = std::to_string(ReadUInt32(
            Internal::RequireJsonMember(
                value, "buffer", "The member buffer was not found"),
            "BufferView buffer must be an unsigned integer"));
        bufferView.byteOffset =
            GetSizeMemberOrDefault(value, "byteOffset");
        bufferView.byteLength = ReadSize(
            Internal::RequireJsonMember(
                value,
                "byteLength",
                "The member byteLength was not found"),
            "BufferView byteLength must be an unsigned integer");

        const auto* byteStride =
            Internal::FindJsonMember(value, "byteStride");
        if (byteStride != nullptr)
        {
            bufferView.byteStride = ReadUInt32(
                *byteStride,
                "BufferView byteStride must be an unsigned integer");
        }

        const auto* target =
            Internal::FindJsonMember(value, "target");
        if (target != nullptr)
        {
            bufferView.target = static_cast<BufferViewTarget>(
                ReadUInt32(
                    *target,
                    "BufferView target must be an unsigned integer"));
        }

        ParseProperty(value, bufferView, extensionDeserializer);
        return bufferView;
    }

    Scene ParseScene(
        const JsonValue& value,
        const ExtensionDeserializer& extensionDeserializer)
    {
        Scene scene;
        scene.name = GetStringMemberOrDefault(value, "name");

        const auto* nodes =
            Internal::FindJsonMember(value, "nodes");
        if (nodes != nullptr)
        {
            Internal::RequireJsonArray(
                *nodes, "Scene nodes must be a JSON array");
            const std::size_t size =
                Internal::GetJsonArraySize(*nodes);
            scene.nodes.reserve(size);
            for (std::size_t index = 0U; index < size; ++index)
            {
                scene.nodes.push_back(std::to_string(ReadUInt32(
                    Internal::GetJsonArrayElement(
                        *nodes,
                        index,
                        "Scene node index is missing"),
                    "Scene node indices must be unsigned integers")));
            }
        }

        ParseProperty(value, scene, extensionDeserializer);
        return scene;
    }

    MorphTarget ParseTarget(const JsonValue& value)
    {
        MorphTarget target;
        target.positionsAccessorId =
            GetUInt32MemberAsString(value, ACCESSOR_POSITION);
        target.normalsAccessorId =
            GetUInt32MemberAsString(value, ACCESSOR_NORMAL);
        target.tangentsAccessorId =
            GetUInt32MemberAsString(value, ACCESSOR_TANGENT);
        return target;
    }

    void ParseTargets(
        const JsonValue& value,
        MeshPrimitive& primitive)
    {
        const auto* targets =
            Internal::FindJsonMember(value, "targets");
        if (targets == nullptr)
        {
            return;
        }

        Internal::RequireJsonArray(
            *targets, "MeshPrimitive targets must be a JSON array");
        const std::size_t size =
            Internal::GetJsonArraySize(*targets);
        primitive.targets.reserve(size);
        for (std::size_t index = 0U; index < size; ++index)
        {
            const auto& target = Internal::GetJsonArrayElement(
                *targets,
                index,
                "MeshPrimitive target is missing");
            Internal::RequireJsonObject(
                target,
                "MeshPrimitive targets array elements must be JSON objects");
            primitive.targets.push_back(ParseTarget(target));
        }
    }

    MeshPrimitive ParseMeshPrimitive(
        const JsonValue& value,
        const ExtensionDeserializer& extensionDeserializer)
    {
        MeshPrimitive primitive;
        const auto* attributes =
            Internal::FindJsonMember(value, "attributes");
        if (attributes != nullptr)
        {
            for (const auto& name :
                 Internal::GetJsonObjectMemberNames(
                     *attributes,
                     "MeshPrimitive attributes must be a JSON object"))
            {
                primitive.attributes[name] = std::to_string(
                    ReadUInt32(
                        *Internal::FindJsonMember(*attributes, name),
                        "MeshPrimitive attribute indices must be "
                        "unsigned integers"));
            }
        }

        primitive.indicesAccessorId =
            GetUInt32MemberAsString(value, "indices");
        primitive.materialId =
            GetUInt32MemberAsString(value, "material");
        const std::int32_t mode = GetInt32MemberOrDefault(
            value, "mode", MESH_TRIANGLES);
        if (mode < MESH_POINTS || mode > MESH_TRIANGLE_FAN)
        {
            throw InvalidGLTFException(
                "MeshPrimitive mode is outside the valid enum range");
        }
        primitive.mode = static_cast<MeshMode>(mode);
        ParseTargets(value, primitive);
        ParseProperty(value, primitive, extensionDeserializer);
        return primitive;
    }

    Mesh ParseMesh(
        const JsonValue& value,
        const ExtensionDeserializer& extensionDeserializer)
    {
        Mesh mesh;
        mesh.name = GetStringMemberOrDefault(value, "name");

        const auto* primitives =
            Internal::FindJsonMember(value, "primitives");
        if (primitives != nullptr)
        {
            Internal::RequireJsonArray(
                *primitives,
                "Mesh primitives must be a JSON array");
            const std::size_t size =
                Internal::GetJsonArraySize(*primitives);
            mesh.primitives.reserve(size);
            for (std::size_t index = 0U; index < size; ++index)
            {
                const auto& primitive =
                    Internal::GetJsonArrayElement(
                        *primitives,
                        index,
                        "Mesh primitive is missing");
                Internal::RequireJsonObject(
                    primitive,
                    "Mesh primitives array elements must be JSON objects");
                mesh.primitives.push_back(
                    ParseMeshPrimitive(
                        primitive, extensionDeserializer));
            }
        }

        mesh.weights = GetFloatArrayMember(
            value,
            "weights",
            "The weights member must be a JSON array",
            "The weights array elements must be JSON numbers");
        ParseProperty(value, mesh, extensionDeserializer);
        return mesh;
    }

    void ParseNodeScale(
        const JsonValue& value,
        Node& node)
    {
        if (Internal::FindJsonMember(value, "scale") == nullptr)
        {
            node.scale = Vector3::ONE;
            return;
        }

        const auto elements = GetFixedSizeFloatArray(
            value,
            "scale",
            3U,
            "A node must have a scale with 3 numeric elements");
        node.scale = Vector3(
            elements[0], elements[1], elements[2]);
    }

    void ParseNodeTranslation(
        const JsonValue& value,
        Node& node)
    {
        if (Internal::FindJsonMember(
                value, "translation") == nullptr)
        {
            node.translation = Vector3::ZERO;
            return;
        }

        const auto elements = GetFixedSizeFloatArray(
            value,
            "translation",
            3U,
            "A node must have a translation with 3 numeric elements");
        node.translation = Vector3(
            elements[0], elements[1], elements[2]);
    }

    void ParseNodeRotation(
        const JsonValue& value,
        Node& node)
    {
        if (Internal::FindJsonMember(value, "rotation") == nullptr)
        {
            node.rotation =
                Quaternion(0.0F, 0.0F, 0.0F, 1.0F);
            return;
        }

        const auto elements = GetFixedSizeFloatArray(
            value,
            "rotation",
            4U,
            "A node must have a rotation with 4 numeric elements");
        node.rotation = Quaternion(
            elements[0],
            elements[1],
            elements[2],
            elements[3]);
    }

    void ParseNodeMatrix(
        const JsonValue& value,
        Node& node)
    {
        if (Internal::FindJsonMember(value, "matrix") == nullptr)
        {
            ParseNodeScale(value, node);
            ParseNodeTranslation(value, node);
            ParseNodeRotation(value, node);
            return;
        }

        const auto elements = GetFixedSizeFloatArray(
            value,
            "matrix",
            16U,
            "A node must have a matrix transform with 16 numeric "
            "elements");
        for (std::size_t index = 0U;
             index < elements.size();
             ++index)
        {
            node.matrix.values[index] = elements[index];
        }
    }

    void ParseNodeChildren(
        const JsonValue& value,
        Node& node)
    {
        const auto* children =
            Internal::FindJsonMember(value, "children");
        if (children == nullptr)
        {
            return;
        }

        Internal::RequireJsonArray(
            *children,
            "Node children must be a JSON array");
        const std::size_t size =
            Internal::GetJsonArraySize(*children);
        node.children.reserve(size);
        for (std::size_t index = 0U; index < size; ++index)
        {
            node.children.push_back(std::to_string(ReadUInt32(
                Internal::GetJsonArrayElement(
                    *children,
                    index,
                    "Node child index is missing"),
                "Node children array elements must be unsigned integers")));
        }
    }

    Camera ParseCamera(
        const JsonValue& value,
        const ExtensionDeserializer& extensionDeserializer)
    {
        std::unique_ptr<Projection> projection;
        const std::string projectionType = ReadString(
            Internal::RequireJsonMember(
                value, "type", "The member type was not found"),
            "Camera type must be a string");

        if (projectionType == "perspective")
        {
            const auto* perspectiveValue =
                Internal::FindJsonMember(value, "perspective");
            if (perspectiveValue == nullptr)
            {
                throw InvalidGLTFException(
                    "Camera perspective projection undefined");
            }
            Internal::RequireJsonObject(
                *perspectiveValue,
                "Camera perspective must be a JSON object");

            Optional<float> aspectRatio;
            const auto* aspect =
                Internal::FindJsonMember(
                    *perspectiveValue, "aspectRatio");
            if (aspect != nullptr)
            {
                aspectRatio = ReadFloat(
                    *aspect,
                    "Camera perspective aspectRatio must be a number");
            }

            const float yfov = ReadFloat(
                Internal::RequireJsonMember(
                    *perspectiveValue,
                    "yfov",
                    "The member yfov was not found"),
                "Camera perspective yfov must be a number");
            const float znear = ReadFloat(
                Internal::RequireJsonMember(
                    *perspectiveValue,
                    "znear",
                    "The member znear was not found"),
                "Camera perspective znear must be a number");

            Optional<float> zfar;
            const auto* farValue =
                Internal::FindJsonMember(
                    *perspectiveValue, "zfar");
            if (farValue != nullptr)
            {
                zfar = ReadFloat(
                    *farValue,
                    "Camera perspective zfar must be a number");
            }

            auto perspective =
                std::make_unique<Perspective>(znear, yfov);
            perspective->zfar = zfar;
            perspective->aspectRatio = aspectRatio;
            ParseProperty(
                *perspectiveValue,
                *perspective,
                extensionDeserializer);
            projection = std::move(perspective);
        }
        else if (projectionType == "orthographic")
        {
            const auto* orthographicValue =
                Internal::FindJsonMember(value, "orthographic");
            if (orthographicValue == nullptr)
            {
                throw InvalidGLTFException(
                    "Camera orthographic projection undefined");
            }
            Internal::RequireJsonObject(
                *orthographicValue,
                "Camera orthographic must be a JSON object");

            const float xmag = ReadFloat(
                Internal::RequireJsonMember(
                    *orthographicValue,
                    "xmag",
                    "The member xmag was not found"),
                "Camera orthographic xmag must be a number");
            const float ymag = ReadFloat(
                Internal::RequireJsonMember(
                    *orthographicValue,
                    "ymag",
                    "The member ymag was not found"),
                "Camera orthographic ymag must be a number");
            const float zfar = ReadFloat(
                Internal::RequireJsonMember(
                    *orthographicValue,
                    "zfar",
                    "The member zfar was not found"),
                "Camera orthographic zfar must be a number");
            const float znear = ReadFloat(
                Internal::RequireJsonMember(
                    *orthographicValue,
                    "znear",
                    "The member znear was not found"),
                "Camera orthographic znear must be a number");
            projection = std::make_unique<Orthographic>(
                zfar, znear, xmag, ymag);
            ParseProperty(
                *orthographicValue,
                *projection,
                extensionDeserializer);
        }

        Camera camera(std::move(projection));
        camera.name = GetStringMemberOrDefault(value, "name");
        if (!camera.projection->IsValid())
        {
            throw InvalidGLTFException(
                "Camera's projection is not valid");
        }
        ParseProperty(value, camera, extensionDeserializer);
        return camera;
    }

    Node ParseNode(
        const JsonValue& value,
        const ExtensionDeserializer& extensionDeserializer)
    {
        Node node;
        node.name = GetStringMemberOrDefault(value, "name");
        ParseNodeChildren(value, node);
        node.meshId = GetUInt32MemberAsString(value, "mesh");
        node.skinId = GetUInt32MemberAsString(value, "skin");
        node.cameraId = GetUInt32MemberAsString(value, "camera");
        ParseNodeMatrix(value, node);
        node.weights = GetFloatArrayMember(
            value,
            "weights",
            "The weights member must be a JSON array",
            "The weights array elements must be JSON numbers");
        ParseProperty(value, node, extensionDeserializer);
        return node;
    }

    Buffer ParseBuffer(
        const JsonValue& value,
        const ExtensionDeserializer& extensionDeserializer)
    {
        Buffer buffer;
        buffer.byteLength = ReadSize(
            Internal::RequireJsonMember(
                value,
                "byteLength",
                "The member byteLength was not found"),
            "Buffer byteLength must be an unsigned integer");
        buffer.uri = GetStringMemberOrDefault(value, "uri");
        ParseProperty(value, buffer, extensionDeserializer);
        return buffer;
    }

    Sampler ParseSampler(
        const JsonValue& value,
        const ExtensionDeserializer& extensionDeserializer)
    {
        Sampler sampler;
        sampler.name = GetStringMemberOrDefault(value, "name");
        sampler.wrapT = Sampler::GetSamplerWrapMode(
            GetUInt32MemberOrDefault(
                value,
                "wrapT",
                static_cast<std::uint32_t>(
                    WrapMode::Wrap_REPEAT)));
        sampler.wrapS = Sampler::GetSamplerWrapMode(
            GetUInt32MemberOrDefault(
                value,
                "wrapS",
                static_cast<std::uint32_t>(
                    WrapMode::Wrap_REPEAT)));

        const auto* minFilter =
            Internal::FindJsonMember(value, "minFilter");
        if (minFilter != nullptr)
        {
            sampler.minFilter =
                Sampler::GetSamplerMinFilterMode(ReadUInt32(
                    *minFilter,
                    "Sampler minFilter must be an unsigned integer"));
        }

        const auto* magFilter =
            Internal::FindJsonMember(value, "magFilter");
        if (magFilter != nullptr)
        {
            sampler.magFilter =
                Sampler::GetSamplerMagFilterMode(ReadUInt32(
                    *magFilter,
                    "Sampler magFilter must be an unsigned integer"));
        }

        ParseProperty(value, sampler, extensionDeserializer);
        return sampler;
    }

    AnimationTarget ParseAnimationTarget(
        const JsonValue& value,
        const ExtensionDeserializer& extensionDeserializer)
    {
        try
        {
            AnimationTarget target;
            target.nodeId =
                GetUInt32MemberAsString(value, "node");
            const auto* path =
                Internal::FindJsonMember(value, "path");
            if (path != nullptr)
            {
                target.path = ParseTargetPath(ReadString(
                    *path,
                    "Animation target path must be a string"));
            }
            ParseProperty(
                value, target, extensionDeserializer);
            return target;
        }
        catch (const InvalidGLTFException& exception)
        {
            std::cerr
                << "Could not parse animation target\n"
                << exception.what() << "\n";
            throw;
        }
    }

    AnimationChannel ParseAnimationChannel(
        const JsonValue& value,
        const ExtensionDeserializer& extensionDeserializer)
    {
        try
        {
            AnimationChannel channel;
            channel.samplerId =
                GetUInt32MemberAsString(value, "sampler");
            const auto& target = Internal::RequireJsonMember(
                value,
                "target",
                "The member target was not found");
            Internal::RequireJsonObject(
                target,
                "Animation channel target must be a JSON object");
            channel.target = ParseAnimationTarget(
                target, extensionDeserializer);
            ParseProperty(
                value, channel, extensionDeserializer);
            return channel;
        }
        catch (const InvalidGLTFException& exception)
        {
            std::cerr
                << "Could not parse channel\n"
                << exception.what() << "\n";
            throw;
        }
    }

    AnimationSampler ParseAnimationSampler(
        const JsonValue& value,
        const ExtensionDeserializer& extensionDeserializer)
    {
        AnimationSampler sampler;
        sampler.inputAccessorId =
            GetUInt32MemberAsString(value, "input");
        sampler.outputAccessorId =
            GetUInt32MemberAsString(value, "output");

        const auto* interpolation =
            Internal::FindJsonMember(value, "interpolation");
        if (interpolation != nullptr)
        {
            sampler.interpolation = ParseInterpolationType(
                ReadString(
                    *interpolation,
                    "Animation interpolation must be a string"));
        }

        ParseProperty(value, sampler, extensionDeserializer);
        return sampler;
    }

    Animation ParseAnimation(
        const JsonValue& value,
        const ExtensionDeserializer& extensionDeserializer)
    {
        Animation animation;
        animation.name = GetStringMemberOrDefault(value, "name");
        animation.channels =
            DeserializeToIndexedContainer<AnimationChannel>(
                "channels",
                value,
                extensionDeserializer,
                ParseAnimationChannel);
        animation.samplers =
            DeserializeToIndexedContainer<AnimationSampler>(
                "samplers",
                value,
                extensionDeserializer,
                ParseAnimationSampler);
        ParseProperty(value, animation, extensionDeserializer);
        return animation;
    }

    Skin ParseSkin(
        const JsonValue& value,
        const ExtensionDeserializer& extensionDeserializer)
    {
        Skin skin;
        skin.name = GetStringMemberOrDefault(value, "name");
        skin.inverseBindMatricesAccessorId =
            GetUInt32MemberAsString(
                value, "inverseBindMatrices");
        skin.skeletonId =
            GetUInt32MemberAsString(value, "skeleton");

        const auto* joints =
            Internal::FindJsonMember(value, "joints");
        if (joints != nullptr)
        {
            Internal::RequireJsonArray(
                *joints, "Skin joints must be a JSON array");
            const std::size_t size =
                Internal::GetJsonArraySize(*joints);
            skin.jointIds.reserve(size);
            for (std::size_t index = 0U; index < size; ++index)
            {
                skin.jointIds.push_back(std::to_string(ReadUInt32(
                    Internal::GetJsonArrayElement(
                        *joints,
                        index,
                        "Skin joint index is missing"),
                    "Skin joint indices must be unsigned integers")));
            }
        }

        ParseProperty(value, skin, extensionDeserializer);
        return skin;
    }

    void ParseStringSet(
        const JsonValue& document,
        const char* memberName,
        std::unordered_set<std::string>& values)
    {
        const auto* array =
            Internal::FindJsonMember(document, memberName);
        if (array == nullptr)
        {
            return;
        }

        Internal::RequireJsonArray(
            *array,
            std::string(memberName) + " must be a JSON array");
        const std::size_t size =
            Internal::GetJsonArraySize(*array);
        for (std::size_t index = 0U; index < size; ++index)
        {
            values.insert(ReadString(
                Internal::GetJsonArrayElement(
                    *array,
                    index,
                    std::string(memberName) +
                    " element is missing"),
                std::string(memberName) +
                " elements must be strings"));
        }
    }

    void ValidateMaterial(Material& material)
    {
        if (material.occlusionTexture.strength > 1.0F ||
            material.occlusionTexture.strength < 0.0F)
        {
            throw InvalidGLTFException(
                "Material " + material.name +
                " has invalid occlusionStrength "
                "(value out of range [0,1])");
        }
    }

    Material ParseMaterial(
        const JsonValue& value,
        const ExtensionDeserializer& extensionDeserializer)
    {
        Material material;

        const auto* metallicRoughness =
            Internal::FindJsonMember(
                value, "pbrMetallicRoughness");
        if (metallicRoughness != nullptr)
        {
            Internal::RequireJsonObject(
                *metallicRoughness,
                "pbrMetallicRoughness must be a JSON object");

            if (Internal::FindJsonMember(
                    *metallicRoughness,
                    "baseColorFactor") != nullptr)
            {
                const auto elements = GetFixedSizeFloatArray(
                    *metallicRoughness,
                    "baseColorFactor",
                    4U,
                    "baseColorFactor must be an array of 4 "
                    "numeric elements");
                material.metallicRoughness.baseColorFactor =
                    Color4(
                        elements[0],
                        elements[1],
                        elements[2],
                        elements[3]);
            }

            const auto* baseColorTexture =
                Internal::FindJsonMember(
                    *metallicRoughness,
                    "baseColorTexture");
            if (baseColorTexture != nullptr)
            {
                ParseTextureInfo(
                    *baseColorTexture,
                    material.metallicRoughness.baseColorTexture,
                    extensionDeserializer);
            }

            material.metallicRoughness.metallicFactor =
                GetFloatMemberOrDefault(
                    *metallicRoughness,
                    "metallicFactor",
                    1.0F);
            material.metallicRoughness.roughnessFactor =
                GetFloatMemberOrDefault(
                    *metallicRoughness,
                    "roughnessFactor",
                    1.0F);

            const auto* metallicRoughnessTexture =
                Internal::FindJsonMember(
                    *metallicRoughness,
                    "metallicRoughnessTexture");
            if (metallicRoughnessTexture != nullptr)
            {
                ParseTextureInfo(
                    *metallicRoughnessTexture,
                    material.metallicRoughness
                        .metallicRoughnessTexture,
                    extensionDeserializer);
            }
        }

        const auto* normalTexture =
            Internal::FindJsonMember(value, "normalTexture");
        if (normalTexture != nullptr)
        {
            ParseTextureInfo(
                *normalTexture,
                material.normalTexture,
                extensionDeserializer);
            material.normalTexture.scale =
                GetFloatMemberOrDefault(
                    *normalTexture, "scale", 1.0F);
        }

        const auto* occlusionTexture =
            Internal::FindJsonMember(value, "occlusionTexture");
        if (occlusionTexture != nullptr)
        {
            ParseTextureInfo(
                *occlusionTexture,
                material.occlusionTexture,
                extensionDeserializer);
            material.occlusionTexture.strength =
                GetFloatMemberOrDefault(
                    *occlusionTexture, "strength", 1.0F);
        }

        const auto* emissiveTexture =
            Internal::FindJsonMember(value, "emissiveTexture");
        if (emissiveTexture != nullptr)
        {
            ParseTextureInfo(
                *emissiveTexture,
                material.emissiveTexture,
                extensionDeserializer);
        }

        if (Internal::FindJsonMember(
                value, "emissiveFactor") != nullptr)
        {
            const auto elements = GetFixedSizeFloatArray(
                value,
                "emissiveFactor",
                3U,
                "emissiveFactor must be an array of 3 "
                "numeric elements");
            material.emissiveFactor = Color3(
                elements[0], elements[1], elements[2]);
        }

        const auto* alphaMode =
            Internal::FindJsonMember(value, "alphaMode");
        if (alphaMode != nullptr)
        {
            material.alphaMode = ParseAlphaMode(ReadString(
                *alphaMode,
                "Material alphaMode must be a string"));
        }

        material.alphaCutoff =
            GetFloatMemberOrDefault(
                value, "alphaCutoff", 0.5F);
        material.name =
            GetStringMemberOrDefault(value, "name");
        material.doubleSided =
            GetBooleanMemberOrDefault(
                value, "doubleSided", false);

        ParseProperty(value, material, extensionDeserializer);
        ValidateMaterial(material);
        return material;
    }

    Texture ParseTexture(
        const JsonValue& value,
        const ExtensionDeserializer& extensionDeserializer)
    {
        Texture texture;
        texture.name = GetStringMemberOrDefault(value, "name");
        texture.imageId =
            GetUInt32MemberAsString(value, "source");
        texture.samplerId =
            GetUInt32MemberAsString(value, "sampler");
        ParseProperty(value, texture, extensionDeserializer);
        return texture;
    }

    Image ParseImage(
        const JsonValue& value,
        const ExtensionDeserializer& extensionDeserializer)
    {
        Image image;
        image.name = GetStringMemberOrDefault(value, "name");
        image.uri = GetStringMemberOrDefault(value, "uri");
        image.bufferViewId =
            GetUInt32MemberAsString(value, "bufferView");
        image.mimeType =
            GetStringMemberOrDefault(value, "mimeType");
        ParseProperty(value, image, extensionDeserializer);
        return image;
    }

    Document DeserializeInternal(
        const JsonValue& document,
        const ExtensionDeserializer& extensionDeserializer,
        SchemaFlags schemaFlags)
    {
        Internal::RequireJsonObject(
            document,
            "glTF document must be a JSON object");
        Internal::ValidateJsonAgainstSchema(
            document,
            SCHEMA_URI_GLTF,
            GetDefaultSchemaLocator(schemaFlags));

        Document gltfDocument;
        const auto* asset =
            Internal::FindJsonMember(document, "asset");
        if (asset != nullptr)
        {
            Internal::RequireJsonObject(
                *asset, "Asset must be a JSON object");
            gltfDocument.asset =
                ParseAsset(*asset, extensionDeserializer);
        }

        gltfDocument.accessors =
            DeserializeToIndexedContainer<Accessor>(
                "accessors",
                document,
                extensionDeserializer,
                ParseAccessor);
        gltfDocument.animations =
            DeserializeToIndexedContainer<Animation>(
                "animations",
                document,
                extensionDeserializer,
                ParseAnimation);
        gltfDocument.buffers =
            DeserializeToIndexedContainer<Buffer>(
                "buffers",
                document,
                extensionDeserializer,
                ParseBuffer);
        gltfDocument.bufferViews =
            DeserializeToIndexedContainer<BufferView>(
                "bufferViews",
                document,
                extensionDeserializer,
                ParseBufferView);
        gltfDocument.cameras =
            DeserializeToIndexedContainer<Camera>(
                "cameras",
                document,
                extensionDeserializer,
                ParseCamera);
        gltfDocument.images =
            DeserializeToIndexedContainer<Image>(
                "images",
                document,
                extensionDeserializer,
                ParseImage);
        gltfDocument.materials =
            DeserializeToIndexedContainer<Material>(
                "materials",
                document,
                extensionDeserializer,
                ParseMaterial);
        gltfDocument.meshes =
            DeserializeToIndexedContainer<Mesh>(
                "meshes",
                document,
                extensionDeserializer,
                ParseMesh);
        gltfDocument.nodes =
            DeserializeToIndexedContainer<Node>(
                "nodes",
                document,
                extensionDeserializer,
                ParseNode);
        gltfDocument.samplers =
            DeserializeToIndexedContainer<Sampler>(
                "samplers",
                document,
                extensionDeserializer,
                ParseSampler);
        gltfDocument.scenes =
            DeserializeToIndexedContainer<Scene>(
                "scenes",
                document,
                extensionDeserializer,
                ParseScene);
        gltfDocument.skins =
            DeserializeToIndexedContainer<Skin>(
                "skins",
                document,
                extensionDeserializer,
                ParseSkin);
        gltfDocument.textures =
            DeserializeToIndexedContainer<Texture>(
                "textures",
                document,
                extensionDeserializer,
                ParseTexture);

        ParseProperty(
            document, gltfDocument, extensionDeserializer);

        const auto* scene =
            Internal::FindJsonMember(document, "scene");
        if (scene != nullptr)
        {
            gltfDocument.defaultSceneId =
                std::to_string(ReadUInt32(
                    *scene,
                    "Default scene must be an unsigned integer"));
        }

        ParseStringSet(
            document,
            "extensionsUsed",
            gltfDocument.extensionsUsed);
        ParseStringSet(
            document,
            "extensionsRequired",
            gltfDocument.extensionsRequired);

        return gltfDocument;
    }

    bool HasFlag(
        DeserializeFlags flags,
        DeserializeFlags flag)
    {
        return (flags & flag) == flag;
    }
}

Document GLTFSDK_API Microsoft::glTF::Deserialize(
    const std::string& json,
    DeserializeFlags flags,
    SchemaFlags schemaFlags)
{
    return Deserialize(
        json,
        ExtensionDeserializer(),
        flags,
        schemaFlags);
}

Document GLTFSDK_API Microsoft::glTF::Deserialize(
    const std::string& json,
    const ExtensionDeserializer& extensionDeserializer,
    DeserializeFlags flags,
    SchemaFlags schemaFlags)
{
    const auto document = Internal::ParseJson(
        json,
        HasFlag(
            flags,
            DeserializeFlags::IgnoreByteOrderMark));
    return DeserializeInternal(
        document,
        extensionDeserializer,
        schemaFlags);
}

Document GLTFSDK_API Microsoft::glTF::Deserialize(
    std::istream& jsonStream,
    DeserializeFlags flags,
    SchemaFlags schemaFlags)
{
    return Deserialize(
        jsonStream,
        ExtensionDeserializer(),
        flags,
        schemaFlags);
}

Document GLTFSDK_API Microsoft::glTF::Deserialize(
    std::istream& jsonStream,
    const ExtensionDeserializer& extensionDeserializer,
    DeserializeFlags flags,
    SchemaFlags schemaFlags)
{
    const auto document = Internal::ParseJson(
        jsonStream,
        HasFlag(
            flags,
            DeserializeFlags::IgnoreByteOrderMark));
    return DeserializeInternal(
        document,
        extensionDeserializer,
        schemaFlags);
}

DeserializeFlags Microsoft::glTF::operator|(
    DeserializeFlags lhs,
    DeserializeFlags rhs)
{
    const auto result =
        static_cast<std::underlying_type_t<DeserializeFlags>>(lhs) |
        static_cast<std::underlying_type_t<DeserializeFlags>>(rhs);
    return static_cast<DeserializeFlags>(result);
}

DeserializeFlags& Microsoft::glTF::operator|=(
    DeserializeFlags& lhs,
    DeserializeFlags rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

DeserializeFlags Microsoft::glTF::operator&(
    DeserializeFlags lhs,
    DeserializeFlags rhs)
{
    const auto result =
        static_cast<std::underlying_type_t<DeserializeFlags>>(lhs) &
        static_cast<std::underlying_type_t<DeserializeFlags>>(rhs);
    return static_cast<DeserializeFlags>(result);
}

DeserializeFlags& Microsoft::glTF::operator&=(
    DeserializeFlags& lhs,
    DeserializeFlags rhs)
{
    lhs = lhs & rhs;
    return lhs;
}
