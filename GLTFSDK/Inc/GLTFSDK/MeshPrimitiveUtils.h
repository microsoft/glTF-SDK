// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <vector>

namespace Microsoft
{
    namespace glTF
    {
        class Document;
        class GLTFResourceReader;
        struct Accessor;
        struct MeshPrimitive;
        struct MorphTarget;

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
        };
    }
}
