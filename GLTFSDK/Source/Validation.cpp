// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/Validation.h>

#include <GLTFSDK/BufferBuilder.h>

#include <sstream>

using namespace Microsoft::glTF;

namespace
{
    template<typename It>
    std::string Join(It it, It itEnd, const char* const delimiter)
    {
        std::stringstream sstream;

        if(it != itEnd)
        {
            sstream << *it++;

            while(it != itEnd)
            {
                sstream << delimiter << *it++;
            }
        }

        return sstream.str();
    }

    std::string GetAccessorTypesAsString(const std::set<AccessorType>& accessorTypes)
    {
        std::vector<std::string> accessorTypeNames(accessorTypes.size());

        std::transform(
            accessorTypes.begin(),
            accessorTypes.end(),
            accessorTypeNames.begin(),
            Accessor::GetAccessorTypeName);

        return Join(accessorTypeNames.begin(), accessorTypeNames.end(), ", ");
    }

    std::string GetComponentTypesAsString(const std::set<ComponentType>& componentTypes)
    {
        std::vector<std::string> componentTypeNames(componentTypes.size());

        std::transform(
            componentTypes.begin(),
            componentTypes.end(),
            componentTypeNames.begin(),
            Accessor::GetComponentTypeName);

        return Join(componentTypeNames.begin(), componentTypeNames.end(), ", ");
    }

    void ValidateAccessorsImpl(const size_t count, const size_t byteOffset, const ComponentType& componentType,
        const AccessorType& accessorType, const std::string& id, const BufferView& bufferView, const Buffer& buffer)
    {
        if (byteOffset > bufferView.byteLength)
        {
            std::string byteLength = std::to_string(bufferView.byteLength);
            throw ValidationException("Accessor" + id + " byteoffset (" + std::to_string(byteOffset) + ") is larger than bufferview byte length (" + byteLength + ")");
        }

        // Check the multiplication in accessor.GetByteLength() for overflow
        size_t testAccessorByteLength;
        if (!Validation::SafeMultiplication(static_cast<size_t>(count),
                (static_cast<size_t>(Accessor::GetComponentTypeSize(componentType) *
                Accessor::GetTypeCount(accessorType))),
            testAccessorByteLength))
        {
            throw ValidationException("Accessor" + id + " byte length too large");
        }

        size_t byteLength = static_cast<size_t>(count * Accessor::GetComponentTypeSize(componentType) * Accessor::GetTypeCount(accessorType));

        if (testAccessorByteLength != byteLength)
        {
            throw ValidationException("Accessor" + id + " byte length safe value does not match actual value");
        }

        if (byteLength > bufferView.byteLength)
        {
            std::string accessorByteLengthStr = std::to_string(byteLength);
            std::string bvByteLength = std::to_string(bufferView.byteLength);
            throw ValidationException("Accessor" + id + " byte length (" + accessorByteLengthStr + ") greater than buffer view (" + bvByteLength + ")");
        }

        short accessorComponentTypeSize = Accessor::GetComponentTypeSize(componentType);
        if ((byteOffset + bufferView.byteOffset) % accessorComponentTypeSize != 0)
        {
            throw ValidationException("Accessor" + id + ": the accessor offset must be a multiple of the size of the accessor component type.");
        }

        Validation::ValidateBufferView(bufferView, buffer);
    }

    void ValidateVertexCount(const MeshMode mode, const size_t count, const std::string& type)
    {
        switch (mode)
        {
        case MESH_POINTS:
            break;

        case MESH_LINES:
            if (count < 2)
            {
                throw ValidationException(type + " count must be at least 2.");
            }

            if (count % 2 != 0)
            {
                throw ValidationException(type + " count for MESH_LINES must be a multiple of 2.");
            }
            break;

        case MESH_LINE_LOOP:
        case MESH_LINE_STRIP:
            if (count < 2)
            {
                throw ValidationException(type + " count must be at least 2.");
            }
            break;

        case MESH_TRIANGLES:
            if (count < 3)
            {
                throw ValidationException(type + " count must be at least 3.");
            }

            if (count % 3 != 0)
            {
                throw ValidationException(type + " count for MESH_TRIANGLES must be a multiple of 3.");
            }
            break;

        case MESH_TRIANGLE_FAN:
        case MESH_TRIANGLE_STRIP:
            if (count < 3)
            {
                throw ValidationException(type + " count must be at least 3.");
            }
            break;

        default:
            throw ValidationException(type + " invalid mesh mode for validation " + std::to_string(mode));
            break;
        }
    }
}

void Validation::Validate(const Document& doc)
{
    ValidateAccessors(doc);
    ValidateMeshes(doc);
}

void Validation::ValidateAccessors(const Document& doc)
{
    for (const auto& accessor : doc.accessors.Elements())
    {
        ValidateAccessor(doc, accessor);
    }
}

void Validation::ValidateMeshes(const Document& doc)
{
    for (const auto& mesh : doc.meshes.Elements())
    {
        for (const auto& primitive : mesh.primitives)
        {
            ValidateMeshPrimitive(doc, primitive);
        }
    }
}

void Validation::ValidateMeshPrimitive(const Document& doc, const MeshPrimitive& primitive)
{
    if (!primitive.HasAttribute(ACCESSOR_POSITION))
    {
        throw ValidationException("MeshPrimitive must have 'POSITION' attribute.");
    }

    size_t vertexCount = doc.accessors.Get(primitive.GetAttributeAccessorId(ACCESSOR_POSITION)).count;

    const auto& indicesAccessorId = primitive.indicesAccessorId;
    if (!indicesAccessorId.empty())
    {
        const auto& indicesAccessor = doc.accessors.Get(indicesAccessorId);
        ValidateAccessorTypes(indicesAccessor, "indices", { TYPE_SCALAR }, { COMPONENT_UNSIGNED_BYTE, COMPONENT_UNSIGNED_SHORT, COMPONENT_UNSIGNED_INT });
        ValidateVertexCount(primitive.mode, indicesAccessor.count, "Index");
    }
    else
    {
        ValidateVertexCount(primitive.mode, vertexCount, "Position");
    }

    ValidateMeshPrimitiveAttributeAccessors(doc, primitive.attributes, vertexCount);
}

void Validation::ValidateMeshPrimitiveAttributeAccessors(
    const Document& doc,
    const std::unordered_map<std::string, std::string>& attributes,
    const size_t vertexCount
)
{
    // https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#meshes
    static const std::unordered_map <std::string, std::pair<std::set<AccessorType>, std::set<ComponentType>>> attributeDefinitions =
    {
        { ACCESSOR_POSITION,   { { TYPE_VEC3 },            { COMPONENT_FLOAT } } },
        { ACCESSOR_NORMAL,     { { TYPE_VEC3 },            { COMPONENT_FLOAT } } },
        { ACCESSOR_TANGENT,    { { TYPE_VEC4 },            { COMPONENT_FLOAT } } },
        { ACCESSOR_TEXCOORD_0, { { TYPE_VEC2 },            { COMPONENT_FLOAT, COMPONENT_UNSIGNED_BYTE, COMPONENT_UNSIGNED_SHORT } } },
        { ACCESSOR_TEXCOORD_1, { { TYPE_VEC2 },            { COMPONENT_FLOAT, COMPONENT_UNSIGNED_BYTE, COMPONENT_UNSIGNED_SHORT } } },
        { ACCESSOR_COLOR_0,    { { TYPE_VEC3, TYPE_VEC4 }, { COMPONENT_FLOAT, COMPONENT_UNSIGNED_BYTE, COMPONENT_UNSIGNED_SHORT } } },
        { ACCESSOR_JOINTS_0,   { { TYPE_VEC4 },            {                  COMPONENT_UNSIGNED_BYTE, COMPONENT_UNSIGNED_SHORT } } },
        { ACCESSOR_WEIGHTS_0,  { { TYPE_VEC4 },            { COMPONENT_FLOAT, COMPONENT_UNSIGNED_BYTE, COMPONENT_UNSIGNED_SHORT } } }
    };

    // TODO: Validate by prefix TEXCOORD_/COLOR_/JOINTS_/WEIGHTS_ 
    for (const auto& attribute : attributes)
    {
        const auto& attributeName = attribute.first;
        const auto& attributeAccessorId = attribute.second;

        const auto it = attributeDefinitions.find(attributeName);
        if (it != attributeDefinitions.end())
        {
            const auto& accessor = doc.accessors.Get(attributeAccessorId);
            ValidateAccessorTypes(accessor, attributeName, it->second.first, it->second.second);

            if (accessor.count != vertexCount)
            {
                throw ValidationException(
                    "MeshPrimitive attribute '" + std::string(attributeName) + "' had an incorrect count ("
                    + std::to_string(accessor.count) + " vs. " + std::to_string(vertexCount));
            }
        }
    }
}

void Validation::ValidateAccessorTypes(
    const Accessor& accessor,
    const std::string& accessorName,
    const std::set<AccessorType>& accessorTypes,
    const std::set<ComponentType>& componentTypes
)
{
    if (accessorTypes.find(accessor.type) == accessorTypes.end())
    {
        throw ValidationException(
            "Accessor " + accessor.id + " " + accessorName + " type must be: [" + GetAccessorTypesAsString(accessorTypes) + "]"
        );
    }

    if (componentTypes.find(accessor.componentType) == componentTypes.end())
    {
        throw ValidationException(
            "Accessor " + accessor.id + " " + accessorName + " componentType must be: [" + GetComponentTypesAsString(componentTypes) + "]"
        );
    }
}

void Validation::ValidateAccessor(const Document& gltfDocument, const Accessor& accessor)
{
    if (!accessor.bufferViewId.empty())
    {
        const BufferView& bufferView = gltfDocument.bufferViews.Get(accessor.bufferViewId);
        const Buffer& buffer = gltfDocument.buffers.Get(bufferView.bufferId);
        ValidateAccessorsImpl(accessor.count, accessor.byteOffset, accessor.componentType, accessor.type, accessor.id, bufferView, buffer);
    }

    if (accessor.sparse.count > 0U) 
    {
        const BufferView& indicesBufferView = gltfDocument.bufferViews.Get(accessor.sparse.indicesBufferViewId);
        const Buffer& indicesBuffer = gltfDocument.buffers.Get(indicesBufferView.bufferId);
        std::string indices_id = accessor.id + "_sparseIndices";
        ValidateAccessorsImpl(accessor.sparse.count, accessor.sparse.indicesByteOffset, accessor.sparse.indicesComponentType,
            TYPE_SCALAR, indices_id, indicesBufferView, indicesBuffer);

        const BufferView& valuesBufferView = gltfDocument.bufferViews.Get(accessor.sparse.valuesBufferViewId);
        const Buffer& valuesBuffer = gltfDocument.buffers.Get(valuesBufferView.bufferId);
        std::string values_id = accessor.id + "_sparseValues";
        ValidateAccessorsImpl(accessor.sparse.count, accessor.sparse.valuesByteOffset, accessor.componentType,
            accessor.type, values_id, valuesBufferView, valuesBuffer);
    }
}

void Validation::ValidateBufferView(const BufferView& buffer_view, const Buffer& buffer)
{
    size_t totalBufferViewLength;
    if (!SafeAddition(buffer_view.byteOffset, buffer_view.byteLength, totalBufferViewLength))
    {
        throw ValidationException("Buffer view size too large");
    }

    if (totalBufferViewLength > buffer.byteLength)
    {
        std::string totalBufferViewLengthStr = std::to_string(totalBufferViewLength);
        std::string byteLength = std::to_string(buffer.byteLength);
        throw ValidationException("BufferView " + buffer_view.bufferId + " offset + length (" + totalBufferViewLengthStr + ") greater than buffer length (" + byteLength + ")");
    }
}

// Figure out if the two arguments, when summed, will overflow a size_t or not.
// If addition was successful, return true and the result of the addition in result.
// If addition was unsuccessful, return false and the value in result is not valid.
bool Validation::SafeAddition(size_t a, size_t b, size_t& result)
{
    if (b <= std::numeric_limits<size_t>::max() - a)
    {
        // No overflow
        result = a + b;
        return true;
    }

    return false;
}

// Figure out if the two arguments, when multiplied, will overflow a size_t or not.
// If multiplication was successful, return true and the result of the addition in result.
// If multiplication was unsuccessful, return false and the value in result is not valid.
bool Validation::SafeMultiplication(size_t a, size_t b, size_t& result)
{
    if (b == 0)
    {
        result = 0;
        return true;
    }

    if (a <= std::numeric_limits<size_t>::max() / b)
    {
        // No overflow
        result = a * b;
        return true;
    }

    return false;
}