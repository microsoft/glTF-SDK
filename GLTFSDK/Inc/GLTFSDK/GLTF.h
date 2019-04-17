// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <GLTFSDK/Color.h>
#include <GLTFSDK/Constants.h>
#include <GLTFSDK/Exceptions.h>
#include <GLTFSDK/Extension.h>
#include <GLTFSDK/IndexedContainer.h>
#include <GLTFSDK/Math.h>
#include <GLTFSDK/Optional.h>

#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Microsoft
{
    namespace glTF
    {
        enum BufferViewTarget
        {
            ARRAY_BUFFER = 34962,
            ELEMENT_ARRAY_BUFFER = 34963
        };

        enum ComponentType
        {
            COMPONENT_UNKNOWN = 0,
            COMPONENT_BYTE = 5120,
            COMPONENT_UNSIGNED_BYTE = 5121,
            COMPONENT_SHORT = 5122,
            COMPONENT_UNSIGNED_SHORT = 5123,
            COMPONENT_UNSIGNED_INT = 5125,
            COMPONENT_FLOAT = 5126
        };

        enum AccessorType
        {
            TYPE_UNKNOWN = 0,
            TYPE_SCALAR,
            TYPE_VEC2,
            TYPE_VEC3,
            TYPE_VEC4,
            TYPE_MAT2,
            TYPE_MAT3,
            TYPE_MAT4
        };

        const std::unordered_map<std::string, AccessorType> AccessorTypes =
        {
            { TYPE_NAME_SCALAR, TYPE_SCALAR },
            { TYPE_NAME_VEC2,   TYPE_VEC2 },
            { TYPE_NAME_VEC3,   TYPE_VEC3 },
            { TYPE_NAME_VEC4,   TYPE_VEC4 },
            { TYPE_NAME_MAT2,   TYPE_MAT2 },
            { TYPE_NAME_MAT3,   TYPE_MAT3 },
            { TYPE_NAME_MAT4,   TYPE_MAT4 }
        };

        enum MeshMode
        {
            // No MESH_UNKNOWN because the default of 0 is a valid value in the spec
            // prefix to avoid name conflicts
            MESH_POINTS = 0,
            MESH_LINES = 1,
            MESH_LINE_LOOP = 2,
            MESH_LINE_STRIP = 3,
            MESH_TRIANGLES = 4,
            MESH_TRIANGLE_STRIP = 5,
            MESH_TRIANGLE_FAN = 6
        };

        enum AlphaMode
        {
            ALPHA_UNKNOWN = 0,
            ALPHA_OPAQUE,
            ALPHA_BLEND,
            ALPHA_MASK
        };

        enum TargetPath
        {
            TARGET_UNKNOWN = 0,
            TARGET_TRANSLATION,
            TARGET_ROTATION,
            TARGET_SCALE,
            TARGET_WEIGHTS
        };

        enum InterpolationType
        {
            INTERPOLATION_UNKNOWN = 0,
            INTERPOLATION_LINEAR,
            INTERPOLATION_STEP,
            INTERPOLATION_CUBICSPLINE
        };

        enum TransformationType
        {
            TRANSFORMATION_IDENTITY = 0,
            TRANSFORMATION_MATRIX,
            TRANSFORMATION_TRS
        };

        enum ProjectionType
        {
            PROJECTION_PERSPECTIVE,
            PROJECTION_ORTHOGRAPHIC
        };

        inline AlphaMode ParseAlphaMode(const std::string& alphaMode)
        {
            if (alphaMode == ALPHAMODE_NAME_OPAQUE) {
                return ALPHA_OPAQUE;
            }
            if (alphaMode == ALPHAMODE_NAME_BLEND) {
                return ALPHA_BLEND;
            }
            if (alphaMode == ALPHAMODE_NAME_MASK) {
                return ALPHA_MASK;
            }

            return ALPHA_UNKNOWN;
        }

        inline TargetPath ParseTargetPath(const std::string& targetPath)
        {
            if (targetPath == TARGETPATH_NAME_TRANSLATION)
            {
                return TARGET_TRANSLATION;
            }
            if (targetPath == TARGETPATH_NAME_ROTATION)
            {
                return TARGET_ROTATION;
            }
            if (targetPath == TARGETPATH_NAME_SCALE)
            {
                return TARGET_SCALE;
            }
            if (targetPath == TARGETPATH_NAME_WEIGHTS)
            {
                return TARGET_WEIGHTS;
            }

            return TARGET_UNKNOWN;
        }

        inline InterpolationType ParseInterpolationType(const std::string& interpolationType)
        {
            if (interpolationType == INTERPOLATIONTYPE_NAME_LINEAR)
            {
                return INTERPOLATION_LINEAR;
            }
            if (interpolationType == INTERPOLATIONTYPE_NAME_STEP)
            {
                return INTERPOLATION_STEP;
            }
            if (interpolationType == INTERPOLATIONTYPE_NAME_CUBICSPLINE)
            {
                return INTERPOLATION_CUBICSPLINE;
            }

            return INTERPOLATION_UNKNOWN;
        }

        struct glTFProperty
        {
            virtual ~glTFProperty() = default;

            std::unordered_map<std::string, std::string> extensions;
            std::string extras;

            template<typename TExt, typename ...TArgs>
            void SetExtension(TArgs&& ...args)
            {
                SetExtension(std::make_unique<TExt>(std::forward<TArgs>(args)...));
            }

            void SetExtension(std::unique_ptr<Extension>&& extension)
            {
                const auto& typeExpr = *extension; // Workaround for clang -Wpotentially-evaluated-expression
                const auto& typeInfo = typeid(typeExpr);

                registeredExtensions.emplace(typeInfo, std::move(extension));
            }

            template<typename T>
            const T& GetExtension() const
            {
                auto it = registeredExtensions.find(typeid(T));
                if (it != registeredExtensions.end())
                {
                    return static_cast<T&>(*it->second.get());
                }

                throw GLTFException(std::string("Could not find extension: ") + typeid(T).name());
            }

            template<typename T>
            T& GetExtension()
            {
                auto it = registeredExtensions.find(typeid(T));
                if (it != registeredExtensions.end())
                {
                    return static_cast<T&>(*it->second.get());
                }

                throw GLTFException(std::string("Could not find extension: ") + typeid(T).name());
            }

            std::vector<std::reference_wrapper<Extension>> GetExtensions() const
            {
                std::vector<std::reference_wrapper<Extension>> exts;

                for (auto& registeredExt : registeredExtensions)
                {
                    exts.push_back(*registeredExt.second);
                }

                return exts;
            }

            template<typename T>
            bool HasExtension() const
            {
                return registeredExtensions.find(typeid(T)) != registeredExtensions.end();
            }

            bool HasUnregisteredExtension(const std::string& name) const
            {
                return extensions.find(name) != extensions.end();
            }

            template<typename T>
            void RemoveExtension()
            {
                registeredExtensions.erase(typeid(T));
            }

        protected:
            glTFProperty() = default;

            glTFProperty(const glTFProperty& other) : extensions(other.extensions), extras(other.extras)
            {
                for(const auto& ext : other.registeredExtensions)
                {
                    registeredExtensions.emplace(ext.first, ext.second->Clone());
                }
            }

            glTFProperty& operator=(const glTFProperty& other)
            {
                if (this != &other)
                {
                    glTFProperty otherCopy(other);

                    extensions = std::move(otherCopy.extensions);
                    registeredExtensions = std::move(otherCopy.registeredExtensions);
                    extras = std::move(otherCopy.extras);
                }

                return *this;
            }

            static bool Equals(const glTFProperty& lhs, const glTFProperty& rhs)
            {
                auto fnRegisteredExtensionsEquals = [](const glTFProperty& lhs, const glTFProperty& rhs)
                {
                    if (lhs.registeredExtensions.size() == rhs.registeredExtensions.size())
                    {
                        return std::all_of(
                            lhs.registeredExtensions.begin(),
                            lhs.registeredExtensions.end(),
                            [&rhs](const std::pair<const std::type_index, std::unique_ptr<Extension>>& value)
                        {
                            auto it = rhs.registeredExtensions.find(value.first);

                            if (it != rhs.registeredExtensions.end())
                            {
                                return *it->second == *value.second;
                            }

                            return false;
                        });
                    }

                    return false;
                };

                return lhs.extensions == rhs.extensions
                    && lhs.extras == rhs.extras
                    && fnRegisteredExtensionsEquals(lhs, rhs);
            }

        private:
            std::unordered_map<std::type_index, std::unique_ptr<Extension>> registeredExtensions;
        };

        struct glTFChildOfRootProperty : glTFProperty
        {
            std::string id;
            std::string name;

        protected:
            glTFChildOfRootProperty() = default;
            glTFChildOfRootProperty(std::string id, std::string name) : id(std::move(id)), name(std::move(name)) {}

            static bool Equals(const glTFChildOfRootProperty& lhs, const glTFChildOfRootProperty& rhs)
            {
                return lhs.id == rhs.id
                    && lhs.name == rhs.name
                    && glTFProperty::Equals(lhs, rhs);
            }
        };

        struct BufferView : glTFChildOfRootProperty
        {
            std::string bufferId;
            size_t byteOffset = 0U;
            size_t byteLength = 0U;
            Optional<size_t> byteStride;
            Optional<BufferViewTarget> target;

            bool operator==(const BufferView& rhs) const
            {
                return glTFChildOfRootProperty::Equals(*this, rhs)
                    && this->bufferId == rhs.bufferId
                    && this->byteOffset == rhs.byteOffset
                    && this->byteLength == rhs.byteLength
                    && this->byteStride == rhs.byteStride
                    && this->target == rhs.target;
            }

            bool operator!=(const BufferView& rhs) const
            {
                return !operator==(rhs);
            }
        };

        struct Accessor : glTFChildOfRootProperty
        {
            // TODO: Sparse is glTFProperty
            struct Sparse
            {
                Sparse() :
                    count(0U),
                    indicesComponentType(COMPONENT_UNKNOWN),
                    indicesByteOffset(0U),
                    valuesByteOffset(0U)
                {
                }

                size_t count;
                // TODO: indices is glTFProperty
                std::string indicesBufferViewId;
                ComponentType indicesComponentType;
                size_t indicesByteOffset;
                // TODO: values is glTFProperty
                std::string valuesBufferViewId;
                size_t valuesByteOffset;

                bool operator==(const Sparse& rhs) const
                {
                    return this->count == rhs.count
                        && this->indicesBufferViewId == rhs.indicesBufferViewId
                        && this->indicesComponentType == rhs.indicesComponentType
                        && this->indicesByteOffset == rhs.indicesByteOffset
                        && this->valuesBufferViewId == rhs.valuesBufferViewId
                        && this->valuesByteOffset == rhs.valuesByteOffset;
                }

                bool operator!=(const Sparse& rhs) const
                {
                    return !operator==(rhs);
                }
            };

            std::string bufferViewId;
            size_t byteOffset = 0U;
            ComponentType componentType = COMPONENT_UNKNOWN;
            bool normalized = false;
            size_t count = 0U;
            AccessorType type = TYPE_UNKNOWN;

            std::vector<float> max;
            std::vector<float> min;

            Sparse sparse;

            // std::string -> AccessorType
            static AccessorType ParseType(const std::string& type)
            {
                const auto result = AccessorTypes.find(type);
                if (result != AccessorTypes.end())
                {
                    return result->second;
                }
                throw GLTFException("Unknown type " + type);
            }

            // AccessorType -> std::string
            static std::string GetAccessorTypeName(AccessorType type)
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
                    throw GLTFException("Unknown type " + std::to_string(type));
                }
            }

            // count(AccessorType)
            static uint8_t GetTypeCount(AccessorType type)
            {
                switch (type)
                {
                case TYPE_SCALAR:
                    return 1;
                case TYPE_VEC2:
                    return 2;
                case TYPE_VEC3:
                    return 3;
                case TYPE_VEC4:
                case TYPE_MAT2:
                    return 4;
                case TYPE_MAT3:
                    return 9;
                case TYPE_MAT4:
                    return 16;
                default:
                    throw GLTFException("Unknown type " + std::to_string(type));
                }
            }

            // uint32_t -> ComponentType
            static ComponentType GetComponentType(uint32_t value)
            {
                switch (value)
                {
                case COMPONENT_BYTE:
                    return COMPONENT_BYTE;
                case COMPONENT_UNSIGNED_BYTE:
                    return COMPONENT_UNSIGNED_BYTE;
                case COMPONENT_SHORT:
                    return COMPONENT_SHORT;
                case COMPONENT_UNSIGNED_SHORT:
                    return COMPONENT_UNSIGNED_SHORT;
                case COMPONENT_UNSIGNED_INT:
                    return COMPONENT_UNSIGNED_INT;
                case COMPONENT_FLOAT:
                    return COMPONENT_FLOAT;
                default:
                    return COMPONENT_UNKNOWN;
                }
            }

            // ComponentType -> std::string
            static std::string GetComponentTypeName(ComponentType componentType)
            {
                switch (componentType)
                {
                case COMPONENT_BYTE:
                    return COMPONENT_TYPE_NAME_BYTE;
                case COMPONENT_UNSIGNED_BYTE:
                    return COMPONENT_TYPE_NAME_UNSIGNED_BYTE;
                case COMPONENT_SHORT:
                    return COMPONENT_TYPE_NAME_SHORT;
                case COMPONENT_UNSIGNED_SHORT:
                    return COMPONENT_TYPE_NAME_UNSIGNED_SHORT;
                case COMPONENT_UNSIGNED_INT:
                    return COMPONENT_TYPE_NAME_UNSIGNED_INT;
                case COMPONENT_FLOAT:
                    return COMPONENT_TYPE_NAME_FLOAT;
                default:
                    throw GLTFException("Unknown componentType " + std::to_string(componentType));
                }
            }

            // size(ComponentType)
            static uint8_t GetComponentTypeSize(ComponentType componentType)
            {
                switch (componentType)
                {
                case COMPONENT_BYTE:
                case COMPONENT_UNSIGNED_BYTE:
                    return 1;
                case COMPONENT_SHORT:
                case COMPONENT_UNSIGNED_SHORT:
                    return 2;
                case COMPONENT_UNSIGNED_INT:
                case COMPONENT_FLOAT:
                    return 4;
                default:
                    throw GLTFException("Unknown componentType " + std::to_string(componentType));
                }
            }

            size_t GetByteLength() const
            {
                return count * GetComponentTypeSize(componentType) * GetTypeCount(type);
            }

            bool operator==(const Accessor& rhs) const
            {
                return glTFChildOfRootProperty::Equals(*this, rhs)
                    && this->bufferViewId == rhs.bufferViewId
                    && this->byteOffset == rhs.byteOffset
                    && this->componentType == rhs.componentType
                    && this->normalized == rhs.normalized
                    && this->count == rhs.count
                    && this->type == rhs.type
                    && this->max == rhs.max
                    && this->min == rhs.min
                    && this->sparse == rhs.sparse;
            }

            bool operator!=(const Accessor& rhs) const
            {
                return !operator==(rhs);
            }
        };

        struct MorphTarget
        {
            std::string positionsAccessorId;
            std::string normalsAccessorId;
            std::string tangentsAccessorId;
            bool operator==(const MorphTarget& rhs) const
            {
                return this->positionsAccessorId == rhs.positionsAccessorId
                    && this->normalsAccessorId == rhs.normalsAccessorId
                    && this->tangentsAccessorId == rhs.tangentsAccessorId;
            }

            bool operator!=(const MorphTarget& rhs) const
            {
                return !operator==(rhs);
            }
        };

        struct MeshPrimitive : glTFProperty
        {
            std::unordered_map<std::string, std::string> attributes;

            std::string indicesAccessorId;

            std::string materialId;
            MeshMode mode = MESH_TRIANGLES;
            std::vector<MorphTarget> targets;

            bool HasAttribute(const std::string& name) const
            {
                return attributes.find(name) != attributes.end();
            }

            const std::string& GetAttributeAccessorId(const std::string& name) const
            {
                auto it = attributes.find(name);

                if (it != attributes.end())
                {
                    return it->second;
                }

                throw GLTFException("Mesh primitive has no attribute named " + name);
            }

            bool TryGetAttributeAccessorId(const std::string& name, std::string& accessorId) const
            {
                auto it = attributes.find(name);

                if (it != attributes.end())
                {
                    accessorId = it->second;
                    return true;
                }

                return false;
            }

            bool operator==(const MeshPrimitive& rhs) const
            {
                return glTFProperty::Equals(*this, rhs)
                    && this->attributes == rhs.attributes
                    && this->indicesAccessorId == rhs.indicesAccessorId
                    && this->materialId == rhs.materialId
                    && this->mode == rhs.mode
                    && this->targets == rhs.targets;
            }

            bool operator!=(const MeshPrimitive& rhs) const
            {
                return !operator==(rhs);
            }
        };

        struct Mesh : glTFChildOfRootProperty
        {
            std::vector<MeshPrimitive> primitives;
            std::vector<float> weights;

            bool operator==(const Mesh& rhs) const
            {
                return glTFChildOfRootProperty::Equals(*this, rhs)
                    && this->primitives == rhs.primitives
                    && this->weights == rhs.weights;
            }

            bool operator!=(const Mesh& rhs) const
            {
                return !operator==(rhs);
            }
        };

        struct Buffer : glTFChildOfRootProperty
        {
            std::string uri;
            size_t byteLength = 0U;

            bool operator==(const Buffer& rhs) const
            {
                return glTFChildOfRootProperty::Equals(*this, rhs)
                    && this->uri == rhs.uri
                    && this->byteLength == rhs.byteLength;
            }

            bool operator!=(const Buffer& rhs) const
            {
                return !operator==(rhs);
            }
        };

        struct Asset : glTFProperty
        {
            std::string copyright;
            std::string generator;
            std::string version;
            std::string minVersion;

            Asset() : version(GLTF_VERSION_2_0)
            {
            }

            bool operator==(const Asset& rhs) const
            {
                return glTFProperty::Equals(*this, rhs)
                    && this->copyright == rhs.copyright
                    && this->generator == rhs.generator
                    && this->version == rhs.version
                    && this->minVersion == rhs.minVersion;
            }

            bool operator!=(const Asset& rhs) const
            {
                return !operator==(rhs);
            }
        };

        enum class TextureType
        {
            BaseColor,
            MetallicRoughness,
            Normal,
            Occlusion,
            Emissive
        };

        struct TextureInfo : glTFProperty
        {
            std::string textureId;
            size_t texCoord = 0U;

            bool operator==(const TextureInfo& rhs) const
            {
                return TextureInfo::Equals(*this, rhs);
            }

            bool operator!=(const TextureInfo& rhs) const
            {
                return !operator==(rhs);
            }

        protected:
            static bool Equals(const TextureInfo& lhs, const TextureInfo& rhs)
            {
                return lhs.textureId == rhs.textureId
                    && lhs.texCoord == rhs.texCoord
                    && glTFProperty::Equals(lhs, rhs);
            }
        };

        // https://github.com/KhronosGroup/glTF/blob/2.0/specification/2.0/schema/material.schema.json
        struct Material : glTFChildOfRootProperty
        {
            struct PBRMetallicRoughness : glTFProperty
            {
                PBRMetallicRoughness() :
                    baseColorFactor(1.0f, 1.0f, 1.0f, 1.0f),
                    metallicFactor(1.0f),
                    roughnessFactor(1.0f)
                {
                }

                Color4 baseColorFactor;
                TextureInfo baseColorTexture;
                float metallicFactor;
                float roughnessFactor;
                TextureInfo metallicRoughnessTexture;

                bool operator==(const PBRMetallicRoughness& rhs) const
                {
                    return this->baseColorFactor == rhs.baseColorFactor
                        && this->baseColorTexture == rhs.baseColorTexture
                        && this->metallicFactor == rhs.metallicFactor
                        && this->roughnessFactor == rhs.roughnessFactor
                        && this->metallicRoughnessTexture == rhs.metallicRoughnessTexture;
                }

                bool operator!=(const PBRMetallicRoughness& rhs) const
                {
                    return !operator==(rhs);
                }
            };

            struct NormalTextureInfo : TextureInfo
            {
                NormalTextureInfo() :
                    scale(1.0f)
                {
                }

                float scale;

                bool operator==(const NormalTextureInfo& rhs) const
                {
                    return TextureInfo::Equals(*this, rhs)
                        && this->scale == rhs.scale;
                }

                bool operator!=(const NormalTextureInfo& rhs) const
                {
                    return !operator==(rhs);
                }
            };

            struct OcclusionTextureInfo : TextureInfo
            {
                OcclusionTextureInfo() :
                    strength(1.0f)
                {
                }

                float strength;

                bool operator==(const OcclusionTextureInfo& rhs) const
                {
                    return TextureInfo::Equals(*this, rhs)
                        && this->strength == rhs.strength;
                }

                bool operator!=(const OcclusionTextureInfo& rhs) const
                {
                    return !operator==(rhs);
                }
            };

            Material() :
                emissiveFactor(0.0f, 0.0f, 0.0f),
                alphaMode(ALPHA_OPAQUE),
                alphaCutoff(0.5f),
                doubleSided(false)
            {
            }

            PBRMetallicRoughness metallicRoughness;
            NormalTextureInfo normalTexture;
            OcclusionTextureInfo occlusionTexture;
            TextureInfo emissiveTexture;
            Color3 emissiveFactor;
            AlphaMode alphaMode;
            float alphaCutoff;
            bool doubleSided;

            std::vector<std::pair<std::string, TextureType>> GetTextures() const
            {
                return {
                    { metallicRoughness.baseColorTexture.textureId, TextureType::BaseColor },
                    { metallicRoughness.metallicRoughnessTexture.textureId, TextureType::MetallicRoughness },
                    { normalTexture.textureId, TextureType::Normal },
                    { occlusionTexture.textureId, TextureType::Occlusion },
                    { emissiveTexture.textureId, TextureType::Emissive }
                };
            }

            bool operator==(const Material& rhs) const
            {
                return glTFChildOfRootProperty::Equals(*this, rhs)
                    && this->metallicRoughness == rhs.metallicRoughness
                    && this->normalTexture == rhs.normalTexture
                    && this->occlusionTexture == rhs.occlusionTexture
                    && this->emissiveTexture == rhs.emissiveTexture
                    && this->emissiveFactor == rhs.emissiveFactor
                    && this->alphaMode == rhs.alphaMode
                    && this->alphaCutoff == rhs.alphaCutoff
                    && this->doubleSided == rhs.doubleSided;
            }

            bool operator!=(const Material& rhs) const
            {
                return !operator==(rhs);
            }
        };

        // Textures that references an Image
        struct Texture : glTFChildOfRootProperty
        {
            std::string samplerId;
            std::string imageId; // Corresponds to 'source' in the schema

            bool operator==(const Texture& rhs) const
            {
                return glTFChildOfRootProperty::Equals(*this, rhs)
                    && this->samplerId == rhs.samplerId
                    && this->imageId == rhs.imageId;
            }

            bool operator!=(const Texture& rhs) const
            {
                return !operator==(rhs);
            }
        };

        // Images of any type (e.g. PNG) referenced by URI or embedded in the binary buffer. Also supports base64 encoded images.
        struct Image : glTFChildOfRootProperty
        {
            std::string uri;
            std::string mimeType;
            std::string bufferViewId;

            bool operator==(const Image& rhs) const
            {
                return glTFChildOfRootProperty::Equals(*this, rhs)
                    && this->uri == rhs.uri
                    && this->mimeType == rhs.mimeType
                    && this->bufferViewId == rhs.bufferViewId;
            }

            bool operator!=(const Image& rhs) const
            {
                return !operator==(rhs);
            }
        };

        struct Projection : glTFProperty
        {
            float znear;

            virtual ProjectionType GetProjectionType() const = 0;
            virtual std::unique_ptr<Projection> Clone() const = 0;
            
            virtual bool IsValid() const = 0;

            bool operator==(const Projection& rhs) const
            {
                return IsEqual(rhs);
            }

            bool operator!=(const Projection& rhs) const
            {
                return !operator==(rhs);
            }

        protected:
            Projection(float znear) : znear(znear)
            {
            }

            virtual bool IsEqual(const Projection& rhs) const
            {
                return glTFProperty::Equals(*this, rhs)
                    && this->znear == rhs.znear;
            }
        };

        struct Orthographic : Projection
        {
            float xmag;
            float ymag;
            float zfar;

            Orthographic(float zfar, float znear, float xmag, float ymag) :
                Projection(znear),
                xmag(xmag),
                ymag(ymag),
                zfar(zfar)
            {
            }

            bool IsValid() const override
            {
                return (zfar > znear)
                    && (ymag != 0.0)
                    && (xmag != 0.0);
            }

            ProjectionType GetProjectionType() const override
            {
                return PROJECTION_ORTHOGRAPHIC;
            }

            std::unique_ptr<Projection> Clone() const override
            {
                return std::make_unique<Orthographic>(*this);
            }

            bool IsEqual(const Projection& rhs) const override
            {
                if (const auto other = dynamic_cast<const Orthographic*>(&rhs))
                {
                    return Projection::IsEqual(rhs)
                        && xmag == other->xmag
                        && ymag == other->ymag
                        && zfar == other->zfar;
                }

                return false;
            }
        };

        struct Perspective : Projection
        {
            Optional<float> aspectRatio;
            float yfov;
            Optional<float> zfar;

            Perspective(float znear, float yfov) :
                Projection(znear),
                aspectRatio(),
                yfov(yfov),
                zfar()
            {
            }

            Perspective(float zfar, float znear, float aspectRatio, float yfov) :
                Projection(znear),
                aspectRatio(aspectRatio),
                yfov(yfov),
                zfar(zfar)
            {
            }

            bool IsValid() const override
            {
                return !zfar.HasValue() || (zfar.Get() > znear);
            }

            bool IsFinite() const
            {
                return zfar.HasValue(); // If zfar is undefined the runtime must use an infinite projection matrix
            }

            bool HasCustomAspectRatio() const
            {
                return aspectRatio.HasValue(); // When aspectRatio is undefined the aspect ratio of the 'canvas' should be used
            }

            ProjectionType GetProjectionType() const override
            {
                return PROJECTION_PERSPECTIVE;
            }

            std::unique_ptr<Projection> Clone() const override
            {
                return std::make_unique<Perspective>(*this);
            }

            bool IsEqual(const Projection& rhs) const override
            {
                if (const auto other = dynamic_cast<const Perspective*>(&rhs))
                {
                    return Projection::IsEqual(rhs)
                        && aspectRatio == other->aspectRatio
                        && yfov == other->yfov
                        && zfar == other->zfar;
                }

                return false;
            }
        };

        struct Camera : glTFChildOfRootProperty
        {
            std::unique_ptr<Projection> projection;

            Camera(std::unique_ptr<Projection> projection) :
                glTFChildOfRootProperty(),
                projection(std::move(projection))
            {
                if (!(this->projection))
                {
                    throw GLTFException("Cannot create camera with null projection");
                }
            }

            Camera(std::string id, std::string name, std::unique_ptr<Projection> projection) :
                glTFChildOfRootProperty(std::move(id), std::move(name)),
                projection(std::move(projection))
            {
                if (!(this->projection))
                {
                    throw GLTFException("Cannot create camera with null projection");
                }
            }

            Camera(const Camera& rhs) :
                glTFChildOfRootProperty(rhs),
                projection(rhs.projection->Clone())
            {
            }

            const Perspective& GetPerspective() const
            {
                if (const auto ret = dynamic_cast<Perspective*>(projection.get()))
                {
                    return *ret;
                }

                throw GLTFException("Failed to cast projection to orthographic");
            }

            const Orthographic& GetOrthographic() const
            {
                if (const auto ret = dynamic_cast<Orthographic*>(projection.get()))
                {
                    return *ret;
                }

                throw GLTFException("Failed to cast projection to orthographic");
            }

            bool operator==(const Camera& rhs) const
            {
                if (!glTFChildOfRootProperty::Equals(*this, rhs))
                {
                    return false;
                }

                if (!projection || !rhs.projection)
                {
                    return !projection && !rhs.projection;
                }

                return *projection == *(rhs.projection);
            }

            bool operator!=(const Camera& rhs) const
            {
                return !operator==(rhs);
            }
        };

        // https://github.com/KhronosGroup/glTF/blob/master/specification/2.0/schema/node.schema.json
        struct Node : glTFChildOfRootProperty
        {
            std::string cameraId;
            std::vector<std::string> children;
            std::string skinId;
            Matrix4 matrix;
            std::string meshId;
            Quaternion rotation = Quaternion::IDENTITY;
            Vector3 scale = Vector3::ONE;
            Vector3 translation = Vector3::ZERO;
            std::vector<float> weights;

            bool IsEmpty() const
            {
                return children.empty() && meshId.empty() && skinId.empty();
            }

            // It is invalid to have both 'matrix' and any of 'translation'/'rotation'/'scale'
            //   spec: "A node can have either a 'matrix' or any combination of 'translation'/'rotation'/'scale' (TRS) properties"
            bool HasValidTransformType() const
            {
                return matrix == Matrix4::IDENTITY || HasIdentityTRS();
            }

            TransformationType GetTransformationType() const
            {
                if (matrix != Matrix4::IDENTITY)
                {
                    return TRANSFORMATION_MATRIX;
                }

                if (!HasIdentityTRS())
                {
                    return TRANSFORMATION_TRS;
                }

                return TRANSFORMATION_IDENTITY;
            }

            bool HasIdentityTRS() const
            {
                return translation == Vector3::ZERO
                    && rotation == Quaternion::IDENTITY
                    && scale == Vector3::ONE;
            }

            bool operator==(const Node& rhs) const
            {
                return glTFChildOfRootProperty::Equals(*this, rhs)
                    && this->cameraId == rhs.cameraId
                    && this->children == rhs.children
                    && this->skinId == rhs.skinId
                    && this->matrix == rhs.matrix
                    && this->meshId == rhs.meshId
                    && this->rotation == rhs.rotation
                    && this->scale == rhs.scale
                    && this->translation == rhs.translation
                    && this->weights == rhs.weights;
            }

            bool operator!=(const Node& rhs) const
            {
                return !operator==(rhs);
            }
        };

        struct Scene : glTFChildOfRootProperty
        {
            std::vector<std::string> nodes;

            bool operator==(const Scene& rhs) const
            {
                return glTFChildOfRootProperty::Equals(*this, rhs)
                    && this->nodes == rhs.nodes;
            }

            bool operator!=(const Scene& rhs) const
            {
                return !operator==(rhs);
            }
        };

        enum MagFilterMode
        {
            MagFilter_NEAREST = 9728,
            MagFilter_LINEAR = 9729,
        };

        enum MinFilterMode
        {
            MinFilter_NEAREST = 9728,
            MinFilter_LINEAR = 9729,
            MinFilter_NEAREST_MIPMAP_NEAREST = 9984,
            MinFilter_LINEAR_MIPMAP_NEAREST = 9985,
            MinFilter_NEAREST_MIPMAP_LINEAR = 9986,
            MinFilter_LINEAR_MIPMAP_LINEAR = 9987
        };

        enum WrapMode
        {
            Wrap_REPEAT = 10497,
            Wrap_CLAMP_TO_EDGE = 33071,
            Wrap_MIRRORED_REPEAT = 33648
        };

        struct Sampler : glTFChildOfRootProperty
        {
            // The glTF spec doesn't define default values for magFilter and minFilter members. When
            // filtering options are not defined implementations are free to select a suitable value
            Optional<MagFilterMode> magFilter;
            Optional<MinFilterMode> minFilter;
            WrapMode wrapS = Wrap_REPEAT;
            WrapMode wrapT = Wrap_REPEAT;

            static MinFilterMode GetSamplerMinFilterMode(size_t readValue)
            {
                switch (readValue)
                {
                    case MinFilter_NEAREST:
                        return MinFilter_NEAREST;
                    case MinFilter_LINEAR:
                        return MinFilter_LINEAR;
                    case MinFilter_NEAREST_MIPMAP_NEAREST:
                        return MinFilter_NEAREST_MIPMAP_NEAREST;
                    case MinFilter_LINEAR_MIPMAP_NEAREST:
                        return MinFilter_LINEAR_MIPMAP_NEAREST;
                    case MinFilter_NEAREST_MIPMAP_LINEAR:
                        return MinFilter_NEAREST_MIPMAP_LINEAR;
                    case MinFilter_LINEAR_MIPMAP_LINEAR:
                        return MinFilter_LINEAR_MIPMAP_LINEAR;
                    default:
                        throw InvalidGLTFException("Invalid sampler min filter value: " + std::to_string(readValue));
                }
            }

            static MagFilterMode GetSamplerMagFilterMode(size_t readValue)
            {
                switch (readValue)
                {
                    case MagFilter_NEAREST:
                        return MagFilter_NEAREST;
                    case MagFilter_LINEAR:
                        return MagFilter_LINEAR;
                    default:
                        throw InvalidGLTFException("Invalid sampler mag filter value: " + std::to_string(readValue));
                }
            }

            static WrapMode GetSamplerWrapMode(size_t readValue)
            { 
                switch (readValue)
                {
                    case Wrap_CLAMP_TO_EDGE:
                        return Wrap_CLAMP_TO_EDGE;
                    case Wrap_MIRRORED_REPEAT:
                        return Wrap_MIRRORED_REPEAT;
                    case Wrap_REPEAT:
                        return Wrap_REPEAT;
                    default:
                        throw InvalidGLTFException("Invalid sampler wrap value: " + std::to_string(readValue));
                }
            }

            bool operator==(const Sampler& rhs) const
            {
                return glTFChildOfRootProperty::Equals(*this, rhs)
                    && this->magFilter == rhs.magFilter
                    && this->minFilter == rhs.minFilter
                    && this->wrapS == rhs.wrapS
                    && this->wrapT == rhs.wrapT;
            }

            bool operator!=(const Sampler& rhs) const
            {
                return !operator==(rhs);
            }
        };

        struct AnimationTarget : glTFProperty
        {
            std::string nodeId;
            TargetPath path = TARGET_UNKNOWN;

            bool operator==(const AnimationTarget& rhs) const
            {
                return glTFProperty::Equals(*this, rhs)
                    && this->nodeId == rhs.nodeId
                    && this->path == rhs.path;
            }

            bool operator!=(const AnimationTarget& rhs) const
            {
                return !operator==(rhs);
            }
        };

        struct AnimationChannel : glTFProperty
        {
            std::string id;
            std::string samplerId;
            AnimationTarget target;

            bool operator==(const AnimationChannel& rhs) const
            {
                return glTFProperty::Equals(*this, rhs)
                    && this->id == rhs.id
                    && this->samplerId == rhs.samplerId
                    && this->target == rhs.target;
            }

            bool operator!=(const AnimationChannel& rhs) const
            {
                return !operator==(rhs);
            }
        };
        
        struct AnimationSampler : glTFProperty
        {
            std::string id;
            std::string inputAccessorId;
            InterpolationType interpolation = INTERPOLATION_LINEAR;
            std::string outputAccessorId;

            bool operator==(const AnimationSampler& rhs) const
            {
                return glTFProperty::Equals(*this, rhs)
                    && this->id == rhs.id
                    && this->inputAccessorId == rhs.inputAccessorId
                    && this->interpolation == rhs.interpolation
                    && this->outputAccessorId == rhs.outputAccessorId;
            }

            bool operator!=(const AnimationSampler& rhs) const
            {
                return !operator==(rhs);
            }
        };

        struct Animation : glTFChildOfRootProperty
        {
            IndexedContainer<const AnimationChannel> channels;
            IndexedContainer<const AnimationSampler> samplers;

            bool operator==(const Animation& rhs) const
            {
                return glTFChildOfRootProperty::Equals(*this, rhs)
                    && this->channels == rhs.channels
                    && this->samplers == rhs.samplers;
            }

            bool operator!=(const Animation& rhs) const
            {
                return !operator==(rhs);
            }
        };

        struct Skin : glTFChildOfRootProperty
        {
            std::string inverseBindMatricesAccessorId;
            std::string skeletonId;
            std::vector<std::string> jointIds;

            bool operator==(const Skin& rhs) const
            {
                return glTFChildOfRootProperty::Equals(*this, rhs)
                    && this->inverseBindMatricesAccessorId == rhs.inverseBindMatricesAccessorId
                    && this->skeletonId == rhs.skeletonId
                    && this->jointIds == rhs.jointIds;
            }

            bool operator!=(const Skin& rhs) const
            {
                return !operator==(rhs);
            }
        };
    }
}
