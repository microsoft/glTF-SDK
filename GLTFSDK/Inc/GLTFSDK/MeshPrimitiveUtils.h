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
            std::vector<uint16_t> GLTFSDK_CDECL GetIndices16(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<uint16_t> GLTFSDK_CDECL GetIndices16(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<uint32_t> GLTFSDK_CDECL GetIndices32(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<uint32_t> GLTFSDK_CDECL GetIndices32(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<uint16_t> GLTFSDK_CDECL GetTriangulatedIndices16(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);
            std::vector<uint32_t> GLTFSDK_CDECL GetTriangulatedIndices32(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<uint16_t> GLTFSDK_CDECL GetSegmentedIndices16(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);
            std::vector<uint32_t> GLTFSDK_CDECL GetSegmentedIndices32(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<float> GLTFSDK_CDECL GetPositions(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GLTFSDK_CDECL GetPositions(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);
            std::vector<float> GLTFSDK_CDECL GetPositions(const Document& doc, const GLTFResourceReader& reader, const MorphTarget& morphTarget);

            std::vector<float> GLTFSDK_CDECL GetNormals(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GLTFSDK_CDECL GetNormals(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);
            std::vector<float> GLTFSDK_CDECL GetNormals(const Document& doc, const GLTFResourceReader& reader, const MorphTarget& morphTarget);

            std::vector<float> GLTFSDK_CDECL GetTangents(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GLTFSDK_CDECL GetTangents(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);
            std::vector<float> GLTFSDK_CDECL GetTangents(const Document& doc, const GLTFResourceReader& reader, const MorphTarget& morphTarget);
            std::vector<float> GLTFSDK_CDECL GetMorphTangents(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);

            std::vector<float> GLTFSDK_CDECL GetTexCoords(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GLTFSDK_CDECL GetTexCoords_0(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);
            std::vector<float> GLTFSDK_CDECL GetTexCoords_1(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<uint32_t> GLTFSDK_CDECL GetColors(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<uint32_t> GLTFSDK_CDECL GetColors_0(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<uint32_t> GLTFSDK_CDECL GetJointIndices32(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<uint32_t> GLTFSDK_CDECL GetJointIndices32_0(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<uint64_t> GLTFSDK_CDECL GetJointIndices64(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<uint64_t> GLTFSDK_CDECL GetJointIndices64_0(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<uint32_t> GLTFSDK_CDECL GetJointWeights32(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<uint32_t> GLTFSDK_CDECL GetJointWeights32_0(const Document& doc, const GLTFResourceReader& reader, const MeshPrimitive& meshPrimitive);

            std::vector<uint16_t> GLTFSDK_CDECL ReverseTriangulateIndices16(const uint16_t* indices, size_t indexCount, MeshMode mode);
            std::vector<uint32_t> GLTFSDK_CDECL ReverseTriangulateIndices32(const uint32_t* indices, size_t indexCount, MeshMode mode);

            std::vector<uint16_t> GLTFSDK_CDECL ReverseTriangulateIndices16(const std::vector<uint16_t>& indices, MeshMode mode);
            std::vector<uint32_t> GLTFSDK_CDECL ReverseTriangulateIndices32(const std::vector<uint32_t>& indices, MeshMode mode);

            std::vector<uint16_t> GLTFSDK_CDECL ReverseSegmentIndices16(const uint16_t* indices, size_t indexCount, MeshMode mode);
            std::vector<uint32_t> GLTFSDK_CDECL ReverseSegmentIndices32(const uint32_t* indices, size_t indexCount, MeshMode mode);

            std::vector<uint16_t> GLTFSDK_CDECL ReverseSegmentIndices16(const std::vector<uint16_t>& indices, MeshMode mode);
            std::vector<uint32_t> GLTFSDK_CDECL ReverseSegmentIndices32(const std::vector<uint32_t>& indices, MeshMode mode);
        };
    }
}
