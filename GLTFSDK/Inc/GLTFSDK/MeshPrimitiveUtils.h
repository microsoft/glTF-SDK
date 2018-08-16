// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <vector>

#include <GLTFSDK/GLTF.h>

namespace Microsoft
{
    namespace glTF
    {
        class Document;
        class GLTFResourceReader;

        namespace MeshPrimitiveUtils
        {
            std::vector<uint16_t> GetIndices16(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<uint16_t> GetIndices16(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<uint32_t> GetIndices32(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<uint32_t> GetIndices32(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<uint16_t> GetTriangulatedIndices16(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);
            std::vector<uint32_t> GetTriangulatedIndices32(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<uint16_t> GetSegmentedIndices16(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);
            std::vector<uint32_t> GetSegmentedIndices32(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<float> GetPositions(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GetPositions(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);
            std::vector<float> GetPositions(const Document& doc, const GLTFResourceReader& reader, const MorphTarget& morphTarget);

            std::vector<float> GetNormals(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GetNormals(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);
            std::vector<float> GetNormals(const Document& doc, const GLTFResourceReader& reader, const MorphTarget& morphTarget);

            std::vector<float> GetTangents(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GetTangents(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);
            std::vector<float> GetTangents(const Document& doc, const GLTFResourceReader& reader, const MorphTarget& morphTarget);
            std::vector<float> GetMorphTangents(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);

            std::vector<float> GetTexCoords(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GetTexCoords_0(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);
            std::vector<float> GetTexCoords_1(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<uint32_t> GetColors(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<uint32_t> GetColors_0(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<uint32_t> GetJointIndices32(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<uint32_t> GetJointIndices32_0(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<uint64_t> GetJointIndices64(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<uint64_t> GetJointIndices64_0(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<uint32_t> GetJointWeights32(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<uint32_t> GetJointWeights32_0(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<uint16_t> ReverseTriangulateIndices16(const uint16_t* indices, size_t indexCount, MeshMode mode);
            std::vector<uint32_t> ReverseTriangulateIndices32(const uint32_t* indices, size_t indexCount, MeshMode mode);

            std::vector<uint16_t> ReverseTriangulateIndices16(const std::vector<uint16_t>& indices, MeshMode mode);
            std::vector<uint32_t> ReverseTriangulateIndices32(const std::vector<uint32_t>& indices, MeshMode mode);

            std::vector<uint16_t> ReverseSegmentIndices16(const uint16_t* indices, size_t indexCount, MeshMode mode);
            std::vector<uint32_t> ReverseSegmentIndices32(const uint32_t* indices, size_t indexCount, MeshMode mode);

            std::vector<uint16_t> ReverseSegmentIndices16(const std::vector<uint16_t>& indices, MeshMode mode);
            std::vector<uint32_t> ReverseSegmentIndices32(const std::vector<uint32_t>& indices, MeshMode mode);
        };
    }
}
