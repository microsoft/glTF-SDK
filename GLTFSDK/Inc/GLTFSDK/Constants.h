// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

namespace Microsoft
{
    namespace glTF
    {
        const int GLB_HEADER_BYTE_SIZE = 20;
        const int GLB2_HEADER_BYTE_SIZE = 12;
        const int GLB_HEADER_MAGIC_STRING_SIZE = 4;
        const int GLB_HEADER_VERSION_1 = 1;
        const int GLB_HEADER_VERSION_2 = 2;
        const int GLB_HEADER_FORMAT = 0;
        const int GLB_CHUNK_ALIGNMENT_SIZE = 4;
        const int GLB_BUFFER_OFFSET_ALIGNMENT = 4; // temporary, see comment on alignment padding in SaveBin
        const int GLB_CHUNK_TYPE_SIZE = 4;

        constexpr const char* MSFT_GLTF_EXPORTER_NAME = "Microsoft GLTF Exporter";
        constexpr const char* GLB_HEADER_MAGIC_STRING = "glTF";
        constexpr const char* GLTF_VERSION_2_0 = "2.0";
        constexpr const char* GLB_CHUNK_TYPE_JSON = "JSON";
        constexpr const char* GLB_CHUNK_TYPE_BIN = "BIN\0";

        constexpr const char* GLTF_EXTENSION = "gltf";
        constexpr const char* GLB_EXTENSION = "glb";
        constexpr const char* BUFFER_EXTENSION = "bin";

        constexpr const char* GLB_BUFFER_ID = "binary_glTF";
        constexpr const char* EMPTY_URI = "data:,";

        constexpr const char* MIMETYPE_PNG = "image/png";
        constexpr const char* MIMETYPE_JPEG = "image/jpeg";

        constexpr const char* FILE_EXT_PNG = "png";
        constexpr const char* FILE_EXT_JPEG = "jpg";

        constexpr const char* DEFAULT_BUFFER_ID = "buffer_default";

        constexpr const char* BUFFERVIEW_INDICES_ID = "bufferView_indices";
        constexpr const char* BUFFERVIEW_UV0_ID = "bufferView_uv0";
        constexpr const char* BUFFERVIEW_UV1_ID = "bufferView_uv1";
        constexpr const char* BUFFERVIEW_POSITIONS_NORMALS_ID = "bufferView_positions_normals";
        constexpr const char* MESH_TRIANGLES_ID = "triangles";

        constexpr const char* ACCESSOR_POSITION = "POSITION";
        constexpr const char* ACCESSOR_NORMAL = "NORMAL";
        constexpr const char* ACCESSOR_TANGENT = "TANGENT";
        constexpr const char* ACCESSOR_TEXCOORD_0 = "TEXCOORD_0";
        constexpr const char* ACCESSOR_TEXCOORD_1 = "TEXCOORD_1";
        constexpr const char* ACCESSOR_COLOR_0 = "COLOR_0";
        constexpr const char* ACCESSOR_JOINTS_0 = "JOINTS_0";
        constexpr const char* ACCESSOR_WEIGHTS_0 = "WEIGHTS_0";

        constexpr const char* TYPE_NAME_SCALAR = "SCALAR";
        constexpr const char* TYPE_NAME_VEC2 = "VEC2";
        constexpr const char* TYPE_NAME_VEC3 = "VEC3";
        constexpr const char* TYPE_NAME_VEC4 = "VEC4";
        constexpr const char* TYPE_NAME_MAT2 = "MAT2";
        constexpr const char* TYPE_NAME_MAT3 = "MAT3";
        constexpr const char* TYPE_NAME_MAT4 = "MAT4";

        constexpr const char* COMPONENT_TYPE_NAME_BYTE = "BYTE";
        constexpr const char* COMPONENT_TYPE_NAME_UNSIGNED_BYTE = "UNSIGNED_BYTE";
        constexpr const char* COMPONENT_TYPE_NAME_SHORT = "SHORT";
        constexpr const char* COMPONENT_TYPE_NAME_UNSIGNED_SHORT = "UNSIGNED_SHORT";
        constexpr const char* COMPONENT_TYPE_NAME_UNSIGNED_INT = "UNSIGNED_INT";
        constexpr const char* COMPONENT_TYPE_NAME_FLOAT = "FLOAT";

        constexpr const char* ALPHAMODE_NAME_OPAQUE = "OPAQUE";
        constexpr const char* ALPHAMODE_NAME_BLEND = "BLEND";
        constexpr const char* ALPHAMODE_NAME_MASK = "MASK";

        constexpr const char* TARGETPATH_NAME_TRANSLATION = "translation";
        constexpr const char* TARGETPATH_NAME_ROTATION = "rotation";
        constexpr const char* TARGETPATH_NAME_SCALE = "scale";
        constexpr const char* TARGETPATH_NAME_WEIGHTS = "weights";

        constexpr const char* INTERPOLATIONTYPE_NAME_LINEAR = "LINEAR";
        constexpr const char* INTERPOLATIONTYPE_NAME_STEP = "STEP";
        constexpr const char* INTERPOLATIONTYPE_NAME_CUBICSPLINE = "CUBICSPLINE";
    }
}
