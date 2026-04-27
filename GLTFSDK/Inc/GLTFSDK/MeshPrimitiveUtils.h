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
            std::vector<uint16_t> GLTFSDK_API GetIndices16(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<uint16_t> GLTFSDK_API GetIndices16(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<uint32_t> GLTFSDK_API GetIndices32(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<uint32_t> GLTFSDK_API GetIndices32(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<uint16_t> GLTFSDK_API GetTriangulatedIndices16(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);
            std::vector<uint32_t> GLTFSDK_API GetTriangulatedIndices32(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<uint16_t> GLTFSDK_API GetSegmentedIndices16(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);
            std::vector<uint32_t> GLTFSDK_API GetSegmentedIndices32(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<float> GLTFSDK_API GetPositions(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GLTFSDK_API GetPositions(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);
            std::vector<float> GLTFSDK_API GetPositions(const Document& doc, const GLTFResourceReader& reader, const MorphTarget& morphTarget);

            std::vector<float> GLTFSDK_API GetNormals(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GLTFSDK_API GetNormals(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);
            std::vector<float> GLTFSDK_API GetNormals(const Document& doc, const GLTFResourceReader& reader, const MorphTarget& morphTarget);

            std::vector<float> GLTFSDK_API GetTangents(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GLTFSDK_API GetTangents(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);
            std::vector<float> GLTFSDK_API GetTangents(const Document& doc, const GLTFResourceReader& reader, const MorphTarget& morphTarget);
            std::vector<float> GLTFSDK_API GetMorphTangents(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);

            std::vector<float> GLTFSDK_API GetTexCoords(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GLTFSDK_API GetTexCoords_0(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);
            std::vector<float> GLTFSDK_API GetTexCoords_1(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<uint32_t> GLTFSDK_API GetColors(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<uint32_t> GLTFSDK_API GetColors_0(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<uint32_t> GLTFSDK_API GetJointIndices32(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<uint32_t> GLTFSDK_API GetJointIndices32_0(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<uint64_t> GLTFSDK_API GetJointIndices64(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<uint64_t> GLTFSDK_API GetJointIndices64_0(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<uint32_t> GLTFSDK_API GetJointWeights32(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<uint32_t> GLTFSDK_API GetJointWeights32_0(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<uint16_t> GLTFSDK_API ReverseTriangulateIndices16(const uint16_t* indices, size_t indexCount, MeshMode mode);
            std::vector<uint32_t> GLTFSDK_API ReverseTriangulateIndices32(const uint32_t* indices, size_t indexCount, MeshMode mode);

            std::vector<uint16_t> GLTFSDK_API ReverseTriangulateIndices16(const std::vector<uint16_t>& indices, MeshMode mode);
            std::vector<uint32_t> GLTFSDK_API ReverseTriangulateIndices32(const std::vector<uint32_t>& indices, MeshMode mode);

            std::vector<uint16_t> GLTFSDK_API ReverseSegmentIndices16(const uint16_t* indices, size_t indexCount, MeshMode mode);
            std::vector<uint32_t> GLTFSDK_API ReverseSegmentIndices32(const uint32_t* indices, size_t indexCount, MeshMode mode);

            std::vector<uint16_t> GLTFSDK_API ReverseSegmentIndices16(const std::vector<uint16_t>& indices, MeshMode mode);
            std::vector<uint32_t> GLTFSDK_API ReverseSegmentIndices32(const std::vector<uint32_t>& indices, MeshMode mode);
        };
    }
}
