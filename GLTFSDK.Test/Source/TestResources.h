// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            // NOTE: Make sure the names of resource files here exactly matches the actual file names
            //       as file/path names are case-sensitive in Android.
            constexpr const char* c_glbSampleBoxInterleaved          = "Resources\\glb\\BoxInterleaved.glb";
            constexpr const char* c_glbCubeInvalidAccessorByteLength = "Resources\\glb\\CubeInvalidAccessorByteLength.glb";
            constexpr const char* c_glbCubeInvalidAccessorByteOffset = "Resources\\glb\\CubeInvalidAccessorByteOffset.glb";
            constexpr const char* c_glbCubeInvalidBufferViewLength   = "Resources\\glb\\CubeInvalidBufferViewLength.glb";
            constexpr const char* c_glbCubeMissingTextureRef         = "Resources\\glb\\CubeMissingTextureRef.glb";
            constexpr const char* c_glbDuckMissingMaterialRef        = "Resources\\glb\\DuckMissingMaterialRef.glb";
            constexpr const char* c_glbWrongBinHeaderLength          = "Resources\\glb\\WrongBinHeaderLength.glb";
            constexpr const char* c_glbWrongJsonLength               = "Resources\\glb\\WrongJsonLength.glb";
            constexpr const char* c_glbWrongReportedLength           = "Resources\\glb\\WrongReportedLength.glb";

            constexpr const char* c_validMorphTarget                 = "Resources\\gltf\\AnimatedMorphCube.gltf";
            constexpr const char* c_animatedTriangleJson             = "Resources\\gltf\\AnimatedTriangle.gltf";
            constexpr const char* c_cameraInvalidPerspective         = "Resources\\gltf\\CameraInvalidPerspective.gltf";
            constexpr const char* c_cameraInvalidProjectionJson      = "Resources\\gltf\\CameraInvalidProjection.gltf";
            constexpr const char* c_cameraMissingProperties          = "Resources\\gltf\\CameraMissingProperties.gltf";
            constexpr const char* c_cartoonCurse01FbxJson            = "Resources\\gltf\\CartoonCurse01Fbx.gltf";
            constexpr const char* c_cubeJson                         = "Resources\\gltf\\Cube.gltf";
            constexpr const char* c_cubeWithLODJson                  = "Resources\\gltf\\CubeWithLOD.gltf";
            constexpr const char* c_doubleNodesJson                  = "Resources\\gltf\\DoubleNodes.gltf";
            constexpr const char* c_doubleTriangleJson               = "Resources\\gltf\\DoubleTriangle.gltf";
            constexpr const char* c_dracoBox                         = "Resources\\gltf\\DracoBox.gltf";
            constexpr const char* c_duplicateNodesJson               = "Resources\\gltf\\DuplicateNodes.gltf";
            constexpr const char* c_meshPrimitivesUV04               = "Resources\\gltf\\Mesh_PrimitivesUV_04.gltf";
            constexpr const char* c_missingDefaultSceneJson          = "Resources\\gltf\\MissingDefaultScene.gltf";
            constexpr const char* c_missingMeshRefJson               = "Resources\\gltf\\MissingMeshRef.gltf";
            constexpr const char* c_missingNodeRefJson               = "Resources\\gltf\\MissingNodeRef.gltf";
            constexpr const char* c_reciprocatingSaw                 = "Resources\\gltf\\ReciprocatingSaw.gltf";
            constexpr const char* c_riggedSimpleJson                 = "Resources\\gltf\\RiggedSimple.gltf";
            constexpr const char* c_simpleSparseAccessor             = "Resources\\gltf\\SimpleSparseAccessor.gltf";
            constexpr const char* c_singlePolyWithNormalsJson        = "Resources\\gltf\\SinglePolyWithNormals.gltf";
            constexpr const char* c_singleTriangleJson               = "Resources\\gltf\\SingleTriangle.gltf";
            constexpr const char* c_singleTriangleWithTextureJson    = "Resources\\gltf\\SingleTriangleWithTexture.gltf";
            constexpr const char* c_transformsJson                   = "Resources\\gltf\\Transforms.gltf";
            constexpr const char* c_validTriangleWithoutIndices      = "Resources\\gltf\\TriangleWithoutIndices.gltf";
            constexpr const char* c_validTriangleMatrix              = "Resources\\gltf\\TriangleWithoutIndices_Matrix.gltf";
            constexpr const char* c_validTriangleTRS                 = "Resources\\gltf\\TriangleWithoutIndices_TRS.gltf";
            constexpr const char* c_validCameraJson                  = "Resources\\gltf\\ValidCamera.gltf";
            constexpr const char* c_cameraWithExtensions             = "Resources\\gltf\\ValidCameraWithExtensions.gltf";
        }
    }
}