// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/MeshPrimitiveUtils.h>

#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLTFResourceReader.h>
#include <GLTFSDK/BufferBuilder.h>

#include <cassert>
#include <numeric>

using namespace Microsoft::glTF;

namespace
{
    const float FLOAT_UINT8_MAX = std::numeric_limits<uint8_t>::max();
    const float FLOAT_UINT16_MAX = std::numeric_limits<uint16_t>::max();

    uint64_t ToUint64(const uint16_t short0, const uint16_t short1, const uint16_t short2, const uint16_t short3)
    {
        return
            static_cast<uint64_t>(short3) << 48UL |
            static_cast<uint64_t>(short2) << 32UL |
            static_cast<uint64_t>(short1) << 16UL |
            static_cast<uint64_t>(short0);
    }

    uint64_t ToUint64(const uint8_t byte0, const uint8_t byte1, const uint8_t byte2, const uint8_t byte3)
    {
        return
            static_cast<uint64_t>(byte3) << 48UL |
            static_cast<uint64_t>(byte2) << 32UL |
            static_cast<uint64_t>(byte1) << 16UL |
            static_cast<uint64_t>(byte0);
    }

    uint32_t ToUint32(const uint8_t byte0, const uint8_t byte1, const uint8_t byte2, const uint8_t byte3)
    {
        return
            static_cast<uint32_t>(byte3) << 24U |
            static_cast<uint32_t>(byte2) << 16U |
            static_cast<uint32_t>(byte1) <<  8U |
            static_cast<uint32_t>(byte0);
    }

    uint8_t ToUint8(const uint16_t value)
    {
        return static_cast<uint8_t>(round((value / FLOAT_UINT16_MAX) * FLOAT_UINT8_MAX));
    }

    template<typename TIn, typename TOut>
    std::vector<TOut> ReadIndices(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor)
    {
        assert(sizeof(TOut) > sizeof(TIn));

        const auto indices = reader.ReadBinaryData<TIn>(doc, accessor);
        return std::vector<TOut>(indices.begin(), indices.end());
    }

    std::vector<uint32_t> PackColorsRGBA(const std::vector<float>& colors)
    {
        assert(colors.size() % 4 == 0);

        std::vector<uint32_t> colors32;
        colors32.reserve(colors.size() / 4);

        for (size_t i = 0; i < colors.size(); i += 4)
        {
            colors32.push_back(Color4(colors[i], colors[i + 1], colors[i + 2], colors[i + 3]).AsUint32RGBA());
        }

        return colors32;
    }

    std::vector<uint32_t> PackColorsRGB(const std::vector<float>& colors)
    {
        assert(colors.size() % 3 == 0);

        std::vector<uint32_t> colors32;
        colors32.reserve(colors.size() / 3);

        for (size_t i = 0; i < colors.size(); i += 3)
        {
            colors32.push_back(Color4(colors[i], colors[i + 1], colors[i + 2], 1.0f).AsUint32RGBA());
        }

        return colors32;
    }

    std::vector<uint32_t> PackColorsRGBA(const std::vector<uint8_t>& colors)
    {
        assert(colors.size() % 4 == 0);

        std::vector<uint32_t> colors32;
        colors32.reserve(colors.size() / 4);

        for (size_t i = 0; i < colors.size(); i += 4)
        {
            uint8_t r = colors[i];
            uint8_t g = colors[i + 1];
            uint8_t b = colors[i + 2];
            uint8_t a = colors[i + 3];
            uint32_t rgba = ToUint32(r, g, b, a);
            colors32.push_back(rgba);
        }

        return colors32;
    }

    std::vector<uint32_t> PackColorsRGB(const std::vector<uint8_t>& colors)
    {
        assert(colors.size() % 3 == 0);

        std::vector<uint32_t> colors32;
        colors32.reserve(colors.size() / 3);

        for (size_t i = 0; i < colors.size(); i += 3)
        {
            uint8_t r = colors[i];
            uint8_t g = colors[i + 1];
            uint8_t b = colors[i + 2];
            uint32_t rgba = ToUint32(r, g, b, 255);
            colors32.push_back(rgba);
        }

        return colors32;
    }

    std::vector<uint32_t> PackColorsRGBA(const std::vector<uint16_t>& colors)
    {
        assert(colors.size() % 4 == 0);

        std::vector<uint32_t> colors32;
        colors32.reserve(colors.size() / 4);

        for (size_t i = 0; i < colors.size(); i += 4)
        {
            uint8_t r = ToUint8(colors[i]);
            uint8_t g = ToUint8(colors[i + 1]);
            uint8_t b = ToUint8(colors[i + 2]);
            uint8_t a = ToUint8(colors[i + 3]);
            uint32_t rgba = ToUint32(r, g, b, a);
            colors32.push_back(rgba);
        }

        return colors32;
    }

    std::vector<uint32_t> PackColorsRGB(const std::vector<uint16_t>& colors)
    {
        assert(colors.size() % 3 == 0);

        std::vector<uint32_t> colors32;
        colors32.reserve(colors.size() / 3);

        for (size_t i = 0; i < colors.size(); i += 3)
        {
            uint8_t r = ToUint8(colors[i]);
            uint8_t g = ToUint8(colors[i + 1]);
            uint8_t b = ToUint8(colors[i + 2]);
            uint32_t rgba = ToUint32(r, g, b, 255);
            colors32.push_back(rgba);
        }

        return colors32;
    }

    template<typename T>
    std::vector<uint32_t> ReadColors(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor)
    {
        std::vector<T> colors = reader.ReadBinaryData<T>(doc, accessor);

        switch (accessor.type)
        {
        case TYPE_VEC4:
            return PackColorsRGBA(colors);

        case TYPE_VEC3:
            return PackColorsRGB(colors);

        default:
            throw GLTFException("Invalid type for color accessor " + accessor.id);
        }
    }

    std::vector<float> ReadTexCoords(const std::vector<float>&& texcoords)
    {
        return std::move(texcoords);
    }

    std::vector<float> ReadTexCoords(const std::vector<uint8_t>&& texcoords)
    {
        std::vector<float> texcoordsFloat;
        texcoordsFloat.reserve(texcoords.size());
        for (size_t i = 0; i < texcoords.size(); i++)
        {
            texcoordsFloat.push_back(texcoords[i] / FLOAT_UINT8_MAX);
        }
        return texcoordsFloat;
    }

    std::vector<float> ReadTexCoords(const std::vector<uint16_t>&& texcoords)
    {
        std::vector<float> texcoordsFloat;
        texcoordsFloat.reserve(texcoords.size());
        for (size_t i = 0; i < texcoords.size(); i++)
        {
            texcoordsFloat.push_back(texcoords[i] / FLOAT_UINT16_MAX);
        }
        return texcoordsFloat;
    }

    template<typename T>
    std::vector<float> ReadTexCoords(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor)
    {
        std::vector<T> texcoords = reader.ReadBinaryData<T>(doc, accessor);
        return ReadTexCoords(std::move(texcoords));
    }

    std::vector<uint32_t> ReadJoints32(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor)
    {
        std::vector<uint8_t> joints = reader.ReadBinaryData<uint8_t>(doc, accessor);
        std::vector<uint32_t> joints32;
        joints32.reserve(joints.size() / 4);
        for (size_t i = 0; i < joints.size(); i += 4)
        {
            joints32.push_back(ToUint32(joints[i], joints[i + 1], joints[i + 2], joints[i + 3]));
        }
        return joints32;
    }

    std::vector<uint64_t> ReadJoints64(const std::vector<uint8_t>& joints)
    {
        std::vector<uint64_t> joints64;
        joints64.reserve(joints.size() / 4);
        for (size_t i = 0; i < joints.size(); i += 4)
        {
            joints64.push_back(ToUint64(joints[i], joints[i + 1], joints[i + 2], joints[i + 3]));
        }
        return joints64;
    }

    std::vector<uint64_t> ReadJoints64(const std::vector<uint16_t>& joints)
    {
        std::vector<uint64_t> joints64;
        joints64.reserve(joints.size() / 4);
        for (size_t i = 0; i < joints.size(); i += 4)
        {
            joints64.push_back(ToUint64(joints[i], joints[i + 1], joints[i + 2], joints[i + 3]));
        }
        return joints64;
    }

    template<typename T>
    std::vector<uint64_t> ReadJoints64(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor)
    {
        std::vector<T> joints = reader.ReadBinaryData<T>(doc, accessor);
        return ReadJoints64(joints);
    }

    std::vector<uint32_t> ReadWeights32(const std::vector<float>&& weights)
    {
        std::vector<uint32_t> weights32;
        weights32.reserve(weights.size() / 4);
        for (size_t i = 0; i < weights.size(); i += 4)
        {
            weights32.push_back(
                ToUint32(
                    Math::FloatToByte(weights[i]),
                    Math::FloatToByte(weights[i + 1]),
                    Math::FloatToByte(weights[i + 2]),
                    Math::FloatToByte(weights[i + 3])));
        }
        return weights32;
    }

    std::vector<uint32_t> ReadWeights32(const std::vector<uint8_t>&& weights)
    {
        std::vector<uint32_t> weights32;
        weights32.reserve(weights.size() / 4);
        for (size_t i = 0; i < weights.size(); i += 4)
        {
            weights32.push_back(
                ToUint32(
                    weights[i],
                    weights[i + 1],
                    weights[i + 2],
                    weights[i + 3]));
        }
        return weights32;
    }

    std::vector<uint32_t> ReadWeights32(const std::vector<uint16_t>&& weights)
    {
        std::vector<uint32_t> weights32;
        weights32.reserve(weights.size() / 4);
        for (size_t i = 0; i < weights.size(); i += 4)
        {
            weights32.push_back(
                ToUint32(
                    ToUint8(weights[i]),
                    ToUint8(weights[i + 1]),
                    ToUint8(weights[i + 2]),
                    ToUint8(weights[i + 3])));
        }
        return weights32;
    }

    template<typename T>
    std::vector<uint32_t> ReadWeights32(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor)
    {
        std::vector<T> weights = reader.ReadBinaryData<T>(doc, accessor);
        return ReadWeights32(std::move(weights));
    }

    template<typename T>
    std::vector<T> GetTrianglesFromTriangleStrip(const std::vector<T>& stripIndices)
    {
        if (stripIndices.size() < 3)
        {
            throw GLTFException("Triangle strip must contain at least 3 vertices.");
        }

        const size_t triangleCount = stripIndices.size() - 2;

        std::vector<T> indices;
        indices.reserve(triangleCount * 3);

        // vertexCount = 5
        // triangleCount = 3
        // indices:
        //     0,1,2
        //     1,3,2
        //     2,3,4
        for (size_t i = 0; i < triangleCount; i++)
        {
            if (i % 2 == 0)
            {
                indices.push_back(stripIndices[i]);
                indices.push_back(stripIndices[i + 1]);
                indices.push_back(stripIndices[i + 2]);
            }
            else
            {
                indices.push_back(stripIndices[i]);
                indices.push_back(stripIndices[i + 2]);
                indices.push_back(stripIndices[i + 1]);
            }
        }

        return indices;
    }

    template<typename T>
    std::vector<T> GetTrianglesFromTriangleFan(const std::vector<T>& fanIndices)
    {
        if (fanIndices.size() < 3)
        {
            throw GLTFException("Triangle fan must contain at least 3 vertices.");
        }

        const size_t triangleCount = fanIndices.size() - 2;

        std::vector<T> indices;
        indices.reserve(triangleCount * 3);

        // vertexCount = 5
        // triangleCount = 3
        // indices:
        //     0,1,2
        //     0,2,3
        //     0,3,4
        for (size_t i = 0; i < triangleCount; i++)
        {
            indices.push_back(fanIndices[0]);
            indices.push_back(fanIndices[i + 1]);
            indices.push_back(fanIndices[i + 2]);
        }

        return indices;
    }

    template<typename T>
    std::vector<T> GetSegmentsFromLineStrip(const std::vector<T>& stripIndices)
    {
        if (stripIndices.size() < 2)
        {
            throw GLTFException("Line must contain at least 2 vertices.");
        }

        const size_t segmentCount = stripIndices.size() - 1;

        std::vector<T> indices;
        indices.reserve(segmentCount * 2);

        // vertexCount = 4
        // segmentCount = 3
        // indices:
        //     0,1
        //     1,2
        //     2,3
        //     3,4
        for (size_t i = 0; i < segmentCount; i++)
        {
            indices.push_back(stripIndices[i]);
            indices.push_back(stripIndices[i + 1]);
        }

        return indices;
    }

    template<typename T>
    std::vector<T> GetSegmentsFromLineLoop(const std::vector<T>& stripIndices)
    {
        auto indices = GetSegmentsFromLineStrip(stripIndices);

        const size_t segmentCount = stripIndices.size();
        indices.push_back(stripIndices[segmentCount - 1]);
        indices.push_back(stripIndices[0]);

        return indices;
    }

    template<typename T>
    std::vector<T> GetTriangulatedIndices(const MeshMode meshMode, const std::vector<T>& rawIndices)
    {
        if (rawIndices.size() < 3)
        {
            throw GLTFException("MeshPrimitive has fewer than 3 indices.");
        }

        switch (meshMode)
        {
        case MESH_TRIANGLES:
            if (rawIndices.size() % 3 != 0)
            {
                throw GLTFException("MeshPrimitives with mode MESH_TRIANGLES has non-multiple-of-3 indices.");
            }
            return rawIndices;
        case MESH_TRIANGLE_STRIP:
            return GetTrianglesFromTriangleStrip(rawIndices);
        case MESH_TRIANGLE_FAN:
            return GetTrianglesFromTriangleFan(rawIndices);
        default:
            throw GLTFException("Invalid mesh mode for triangulation " + std::to_string(meshMode));
        }
    }

    template<typename T>
    std::vector<T> GetSegmentedIndices(const MeshMode meshMode, const std::vector<T>& rawIndices)
    {
        if (rawIndices.size() < 2)
        {
            throw GLTFException("MeshPrimitive has fewer than 2 indices.");
        }

        switch (meshMode)
        {
        case MESH_LINES:
            if (rawIndices.size() % 2 != 0)
            {
                throw GLTFException("MeshPrimitives with mode MESH_LINES has non-multiple-of-2 indices.");
            }
            return rawIndices;
        case MESH_LINE_STRIP:
            return GetSegmentsFromLineStrip(rawIndices);
        case MESH_LINE_LOOP:
            return GetSegmentsFromLineLoop(rawIndices);
        default:
            throw GLTFException("Invalid mesh mode for triangulation " + std::to_string(meshMode));
        }
    }

    std::vector<uint16_t> GetOrCreateIndices16(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive)
    {
        if (doc.accessors.Has(meshPrimitive.indicesAccessorId))
        {
            const auto& indicesAccessor = doc.accessors.Get(meshPrimitive.indicesAccessorId);
            return MeshPrimitiveUtils::GetIndices16(doc, reader, indicesAccessor);
        }
        else
        {
            size_t vertexCount = doc.accessors.Get(meshPrimitive.GetAttributeAccessorId(ACCESSOR_POSITION)).count;

            if (vertexCount > std::numeric_limits<uint16_t>::max())
            {
                throw GLTFException("Cannot generate 16-bit indices for MeshPrimitive with " + std::to_string(vertexCount) + " vertices.");
            }

            std::vector<uint16_t> rawIndices(vertexCount);
            std::iota(rawIndices.begin(), rawIndices.end(), static_cast<uint16_t>(0U));
            return rawIndices;
        }
    }

    std::vector<uint32_t> GetOrCreateIndices32(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive)
    {
        if (doc.accessors.Has(meshPrimitive.indicesAccessorId))
        {
            const auto& indicesAccessor = doc.accessors.Get(meshPrimitive.indicesAccessorId);
            return MeshPrimitiveUtils::GetIndices32(doc, reader, indicesAccessor);
        }
        else
        {
            size_t vertexCount = doc.accessors.Get(meshPrimitive.GetAttributeAccessorId(ACCESSOR_POSITION)).count;

            std::vector<uint32_t> rawIndices(vertexCount);
            std::iota(rawIndices.begin(), rawIndices.end(), static_cast<uint32_t>(0U));
            return rawIndices;
        }
    }

    template<typename T>
    std::vector<T> ReconstructTriangleStripIndexing(const T* indices, size_t indexCount)
    {
        if (indexCount % 3 != 0)
        {
            throw GLTFException("Input triangulated triangle strip has non-multiple-of-3 indices.");
        }

        if (indexCount < 3)
        {
            throw GLTFException("Input triangulated triangle strip has fewer than 3 indices.");
        }

        std::vector<T> result;
        result.reserve(2 + indexCount / 3);

        result.push_back(indices[0]);
        result.push_back(indices[1]);

        for (size_t i = 2; i < indexCount; i += 3)
        {
            if (i % 2 == 0)
            {
                result.push_back(indices[i]);
            }
            else
            {
                result.push_back(indices[i - 1]);
            }
        }

        return result;
    }

    template<typename T>
    std::vector<T> ReconstructTriangleFanIndexing(const T* indices, size_t indexCount)
    {
        if (indexCount % 3 != 0)
        {
            throw GLTFException("Input triangulated triangle fan has non-multiple-of-3 indices.");
        }

        if (indexCount < 3)
        {
            throw GLTFException("Input triangulated triangle fan has fewer than 3 indices.");
        }

        std::vector<T> result;
        result.reserve(2 + indexCount / 3);

        result.push_back(indices[0]);
        result.push_back(indices[1]);

        for (size_t i = 2; i < indexCount; i += 3)
        {
            result.push_back(indices[i]);
        }

        return result;
    }

    template<typename T>
    std::vector<T> ReverseTriangulateIndices(const T* indices, size_t indexCount, MeshMode mode)
    {
        if (mode == MeshMode::MESH_TRIANGLE_STRIP)
        {
            return ReconstructTriangleStripIndexing(indices, indexCount);
        }
        else if (mode == MeshMode::MESH_TRIANGLE_FAN)
        {
            return ReconstructTriangleFanIndexing(indices, indexCount);
        }
        else
        {
            throw GLTFException("Non-triangulated mesh mode specificed.");
        }
    }

    template<typename T>
    std::vector<T> ReconstructLineLoopIndexing(const T* indices, size_t indexCount)
    {
        if (indexCount % 2 != 0)
        {
            throw GLTFException("Input segmented line has non-multiple-of-2 indices.");
        }

        std::vector<T> result;
        result.reserve(indexCount / 2);

        for (size_t i = 0; i < indexCount; i += 2)
        {
            result.push_back(indices[i]);
        }

        return result;
    }

    template<typename T>
    std::vector<T> ReconstructLineStripIndexing(const T* indices, size_t indexCount)
    {
        auto result = ReconstructLineLoopIndexing(indices, indexCount);
        result.push_back(indices[indexCount - 1]);
        return result;
    }

    template<typename T>
    std::vector<T> ReverseSegmentIndices(const T* indices, size_t indexCount, MeshMode mode)
    {
        if (mode == MeshMode::MESH_LINE_STRIP)
        {
            return ReconstructLineStripIndexing(indices, indexCount);
        }
        else if (mode == MeshMode::MESH_LINE_LOOP)
        {
            return ReconstructLineLoopIndexing(indices, indexCount);
        }
        else
        {
            throw GLTFException("Non-segmented mesh mode specificed.");
        }
    }
}

std::vector<uint16_t> MeshPrimitiveUtils::GetIndices16(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor)
{
    if (accessor.type != TYPE_SCALAR)
    {
        throw GLTFException("Invalid type for indices accessor " + accessor.id);
    }

    switch (accessor.componentType)
    {
    case COMPONENT_UNSIGNED_BYTE:
        return ReadIndices<uint8_t, uint16_t>(doc, reader, accessor);

    case COMPONENT_UNSIGNED_SHORT:
        return reader.ReadBinaryData<uint16_t>(doc, accessor);

    case COMPONENT_UNSIGNED_INT:
        throw GLTFException("Cannot convert 32-bit indices to 16-bit");

    default:
        throw GLTFException("Invalid componentType for indices accessor " + accessor.id);
    }
}

std::vector<uint16_t> MeshPrimitiveUtils::GetIndices16(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive)
{
    const auto& accessor = doc.accessors.Get(meshPrimitive.indicesAccessorId);
    return GetIndices16(doc, reader, accessor);
}

std::vector<uint32_t> MeshPrimitiveUtils::GetIndices32(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor)
{
    if (accessor.type != TYPE_SCALAR)
    {
        throw GLTFException("Invalid type for indices accessor " + accessor.id);
    }

    switch (accessor.componentType)
    {
    case COMPONENT_UNSIGNED_BYTE:
        return ReadIndices<uint8_t, uint32_t>(doc, reader, accessor);

    case COMPONENT_UNSIGNED_SHORT:
        return ReadIndices<uint16_t, uint32_t>(doc, reader, accessor);

    case COMPONENT_UNSIGNED_INT:
        return reader.ReadBinaryData<uint32_t>(doc, accessor);

    default:
        throw GLTFException("Invalid componentType for indices accessor " + accessor.id);
    }
}

std::vector<uint32_t> MeshPrimitiveUtils::GetIndices32(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive)
{
    const auto& accessor = doc.accessors.Get(meshPrimitive.indicesAccessorId);
    return GetIndices32(doc, reader, accessor);
}

std::vector<uint16_t> MeshPrimitiveUtils::GetTriangulatedIndices16(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive)
{
    return GetTriangulatedIndices<uint16_t>(meshPrimitive.mode, GetOrCreateIndices16(doc, reader, meshPrimitive));
}

std::vector<uint32_t> MeshPrimitiveUtils::GetTriangulatedIndices32(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive)
{
    return GetTriangulatedIndices<uint32_t>(meshPrimitive.mode, GetOrCreateIndices32(doc, reader, meshPrimitive));
}

std::vector<uint16_t> MeshPrimitiveUtils::GetSegmentedIndices16(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive)
{

    return GetSegmentedIndices<uint16_t>(meshPrimitive.mode, GetOrCreateIndices16(doc, reader, meshPrimitive));
}

std::vector<uint32_t> MeshPrimitiveUtils::GetSegmentedIndices32(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive)
{
    return GetSegmentedIndices<uint32_t>(meshPrimitive.mode, GetOrCreateIndices32(doc, reader, meshPrimitive));
}

// Positions
std::vector<float> MeshPrimitiveUtils::GetPositions(const Document& doc, const GLTFResourceReader& reader, const Accessor& positionsAccessor)
{
    if (positionsAccessor.type != TYPE_VEC3)
    {
        throw GLTFException("Invalid type for positions accessor " + positionsAccessor.id);
    }

    if (positionsAccessor.componentType != COMPONENT_FLOAT)
    {
        throw GLTFException("Invalid component type for positions accessor " + positionsAccessor.id);
    }

    return reader.ReadBinaryData<float>(doc, positionsAccessor);
}

std::vector<float> MeshPrimitiveUtils::GetPositions(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive)
{
    const auto& positionsAccessor = doc.accessors.Get(meshPrimitive.GetAttributeAccessorId(ACCESSOR_POSITION));
    return GetPositions(doc, reader, positionsAccessor);
}

std::vector<float> MeshPrimitiveUtils::GetPositions(const Document& doc, const GLTFResourceReader& reader, const MorphTarget& morphTarget)
{
    const auto& positionsAccessor = doc.accessors.Get(morphTarget.positionsAccessorId);
    return GetPositions(doc, reader, positionsAccessor);
}

// Normals
std::vector<float> MeshPrimitiveUtils::GetNormals(const Document& doc, const GLTFResourceReader& reader, const Accessor& normalsAccessor)
{
    if (normalsAccessor.type != TYPE_VEC3)
    {
        throw GLTFException("Invalid type for normals accessor " + normalsAccessor.id);
    }

    if (normalsAccessor.componentType != COMPONENT_FLOAT)
    {
        throw GLTFException("Invalid component type for normals accessor " + normalsAccessor.id);
    }

    return reader.ReadBinaryData<float>(doc, normalsAccessor);
}

std::vector<float> MeshPrimitiveUtils::GetNormals(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive)
{
    const auto& accessor = doc.accessors.Get(meshPrimitive.GetAttributeAccessorId(ACCESSOR_NORMAL));
    return GetNormals(doc, reader, accessor);
}

std::vector<float> MeshPrimitiveUtils::GetNormals(const Document& doc, const GLTFResourceReader& reader, const MorphTarget& morphTarget)
{
    const auto& accessor = doc.accessors.Get(morphTarget.normalsAccessorId);
    return GetNormals(doc, reader, accessor);
}

// Tangents
std::vector<float> MeshPrimitiveUtils::GetTangents(const Document& doc, const GLTFResourceReader& reader, const Accessor& tangentsAccessor)
{
    if (tangentsAccessor.type != TYPE_VEC4)
    {
        throw GLTFException("Invalid type for tangents accessor " + tangentsAccessor.id);
    }

    if (tangentsAccessor.componentType != COMPONENT_FLOAT)
    {
        throw GLTFException("Invalid component type for tangents accessor " + tangentsAccessor.id);
    }

    return reader.ReadBinaryData<float>(doc, tangentsAccessor);
}

std::vector<float> MeshPrimitiveUtils::GetTangents(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive)
{
    const auto& accessor = doc.accessors.Get(meshPrimitive.GetAttributeAccessorId(ACCESSOR_TANGENT));
    return GetTangents(doc, reader, accessor);
}

// Morph Target Tangents (which have a different accessor type than base mesh tangents)
std::vector<float> MeshPrimitiveUtils::GetMorphTangents(const Document& doc, const GLTFResourceReader& reader, const Accessor& tangentsAccessor)
{
    if (tangentsAccessor.type != TYPE_VEC3)
    {
        throw GLTFException("Invalid type for tangents accessor " + tangentsAccessor.id);
    }

    if (tangentsAccessor.componentType != COMPONENT_FLOAT)
    {
        throw GLTFException("Invalid component type for tangents accessor " + tangentsAccessor.id);
    }

    return reader.ReadBinaryData<float>(doc, tangentsAccessor);
}

std::vector<float> MeshPrimitiveUtils::GetTangents(const Document& doc, const GLTFResourceReader& reader, const MorphTarget& morphTarget)
{
    const auto& accessor = doc.accessors.Get(morphTarget.tangentsAccessorId);
    return GetMorphTangents(doc, reader, accessor);
}

// Texcoords
std::vector<float> MeshPrimitiveUtils::GetTexCoords(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor)
{
    switch (accessor.componentType)
    {
    case COMPONENT_FLOAT:
        return ReadTexCoords<float>(doc, reader, accessor);

    case COMPONENT_UNSIGNED_BYTE:
        return ReadTexCoords<uint8_t>(doc, reader, accessor);

    case COMPONENT_UNSIGNED_SHORT:
        return ReadTexCoords<uint16_t>(doc, reader, accessor);

    default:
        throw GLTFException("Invalid componentType for texcoords accessor " + accessor.id);
    }
}

std::vector<float> MeshPrimitiveUtils::GetTexCoords_0(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive)
{
    const auto& accessor = doc.accessors.Get(meshPrimitive.GetAttributeAccessorId(ACCESSOR_TEXCOORD_0));
    return GetTexCoords(doc, reader, accessor);
}

std::vector<float> MeshPrimitiveUtils::GetTexCoords_1(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive)
{
    const auto& accessor = doc.accessors.Get(meshPrimitive.GetAttributeAccessorId(ACCESSOR_TEXCOORD_1));
    return GetTexCoords(doc, reader, accessor);
}

// Colors
std::vector<uint32_t> MeshPrimitiveUtils::GetColors(const Document& doc, const GLTFResourceReader& reader, const Accessor& colorsAccessor)
{
    switch (colorsAccessor.componentType)
    {
    case COMPONENT_FLOAT:
        return ReadColors<float>(doc, reader, colorsAccessor);

    case COMPONENT_UNSIGNED_BYTE:
        return ReadColors<uint8_t>(doc, reader, colorsAccessor);

    case COMPONENT_UNSIGNED_SHORT:
        return ReadColors<uint16_t>(doc, reader, colorsAccessor);

    default:
        throw GLTFException("Invalid componentType for color accessor " + colorsAccessor.id);
    }
}

std::vector<uint32_t> MeshPrimitiveUtils::GetColors_0(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive)
{
    const auto& accessor = doc.accessors.Get(meshPrimitive.GetAttributeAccessorId(ACCESSOR_COLOR_0));
    return GetColors(doc, reader, accessor);
}

// Joints
std::vector<uint32_t> MeshPrimitiveUtils::GetJointIndices32(const Document& doc, const GLTFResourceReader& reader, const Accessor& jointsAccessor)
{
    if (jointsAccessor.type != TYPE_VEC4)
    {
        throw GLTFException("Invalid type for joints accessor " + jointsAccessor.id);
    }

    switch (jointsAccessor.componentType)
    {
    case COMPONENT_UNSIGNED_BYTE:
        return ReadJoints32(doc, reader, jointsAccessor);

    case COMPONENT_UNSIGNED_SHORT:
        throw GLTFException("Cannot pack 4 x 16-bit indices into 32-bits");

    default:
        throw GLTFException("Invalid componentType for joints accessor " + jointsAccessor.id);
    }
}

std::vector<uint32_t> MeshPrimitiveUtils::GetJointIndices32_0(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive)
{
    const auto& accessor = doc.accessors.Get(meshPrimitive.GetAttributeAccessorId(ACCESSOR_JOINTS_0));
    return GetJointIndices32(doc, reader, accessor);
}

std::vector<uint64_t> MeshPrimitiveUtils::GetJointIndices64(const Document& doc, const GLTFResourceReader& reader, const Accessor& jointsAccessor)
{
    if (jointsAccessor.type != TYPE_VEC4)
    {
        throw GLTFException("Invalid type for joints accessor " + jointsAccessor.id);
    }

    switch (jointsAccessor.componentType)
    {
    case COMPONENT_UNSIGNED_BYTE:
        return ReadJoints64<uint8_t>(doc, reader, jointsAccessor);

    case COMPONENT_UNSIGNED_SHORT:
        return ReadJoints64<uint16_t>(doc, reader, jointsAccessor);

    default:
        throw GLTFException("Invalid componentType for joints accessor " + jointsAccessor.id);
    }
}

std::vector<uint64_t> MeshPrimitiveUtils::GetJointIndices64_0(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive)
{
    const auto& accessor = doc.accessors.Get(meshPrimitive.GetAttributeAccessorId(ACCESSOR_JOINTS_0));
    return GetJointIndices64(doc, reader, accessor);
}

// Weights
std::vector<uint32_t> MeshPrimitiveUtils::GetJointWeights32(const Document& doc, const GLTFResourceReader& reader, const Accessor& weightsAccessor)
{
    if (weightsAccessor.type != TYPE_VEC4)
    {
        throw GLTFException("Invalid type for weights accessor " + weightsAccessor.id);
    }

    switch (weightsAccessor.componentType)
    {
    case COMPONENT_FLOAT:
        return ReadWeights32<float>(doc, reader, weightsAccessor);

    case COMPONENT_UNSIGNED_BYTE:
        return ReadWeights32<uint8_t>(doc, reader, weightsAccessor);

    case COMPONENT_UNSIGNED_SHORT:
        return ReadWeights32<uint16_t>(doc, reader, weightsAccessor);

    default:
        throw GLTFException("Invalid componentType for weights accessor " + weightsAccessor.id);
    }
}

std::vector<uint32_t> MeshPrimitiveUtils::GetJointWeights32_0(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive)
{
    const auto& accessor = doc.accessors.Get(meshPrimitive.GetAttributeAccessorId(ACCESSOR_WEIGHTS_0));
    return GetJointWeights32(doc, reader, accessor);
}

std::vector<uint16_t> MeshPrimitiveUtils::ReverseTriangulateIndices16(const uint16_t* indices, size_t indexCount, MeshMode mode)
{
    return ReverseTriangulateIndices(indices, indexCount, mode);
}

std::vector<uint32_t> MeshPrimitiveUtils::ReverseTriangulateIndices32(const uint32_t* indices, size_t indexCount, MeshMode mode)
{
    return ReverseTriangulateIndices(indices, indexCount, mode);
}

std::vector<uint16_t> MeshPrimitiveUtils::ReverseTriangulateIndices16(const std::vector<uint16_t>& indices, MeshMode mode)
{
    return ReverseTriangulateIndices(indices.data(), indices.size(), mode);
}

std::vector<uint32_t> MeshPrimitiveUtils::ReverseTriangulateIndices32(const std::vector<uint32_t>& indices, MeshMode mode)
{
    return ReverseTriangulateIndices(indices.data(), indices.size(), mode);
}

std::vector<uint16_t> MeshPrimitiveUtils::ReverseSegmentIndices16(const uint16_t* indices, size_t indexCount, MeshMode mode)
{
    return ReverseSegmentIndices(indices, indexCount, mode);
}

std::vector<uint32_t> MeshPrimitiveUtils::ReverseSegmentIndices32(const uint32_t* indices, size_t indexCount, MeshMode mode)
{
    return ReverseSegmentIndices(indices, indexCount, mode);
}

std::vector<uint16_t> MeshPrimitiveUtils::ReverseSegmentIndices16(const std::vector<uint16_t>& indices, MeshMode mode)
{
    return ReverseSegmentIndices(indices.data(), indices.size(), mode);
}

std::vector<uint32_t> MeshPrimitiveUtils::ReverseSegmentIndices32(const std::vector<uint32_t>& indices, MeshMode mode)
{
    return ReverseSegmentIndices(indices.data(), indices.size(), mode);
}
