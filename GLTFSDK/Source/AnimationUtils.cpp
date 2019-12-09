// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/AnimationUtils.h>

#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLTFResourceReader.h>

using namespace Microsoft::glTF;

std::vector<float> AnimationUtils::GetKeyframeTimes(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor)
{
    if (accessor.type != TYPE_SCALAR)
    {
        throw GLTFException("Invalid type for animation input accessor " + accessor.id);
    }

    if (accessor.componentType != COMPONENT_FLOAT)
    {
        throw GLTFException("Invalid componentType for animation input accessor " + accessor.id);
    }

    return reader.ReadBinaryData<float>(doc, accessor);
}

std::vector<float> AnimationUtils::GetKeyframeTimes(const Document& doc, const GLTFResourceReader& reader, const AnimationSampler& sampler)
{
    auto& accessor = doc.accessors[sampler.inputAccessorId];
    return GetKeyframeTimes(doc, reader, accessor);
}

std::vector<float> AnimationUtils::GetInverseBindMatrices(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor)
{
    if (accessor.type != TYPE_MAT4)
    {
        throw GLTFException("Invalid type for inverse bind matrices accessor " + accessor.id);
    }

    if (accessor.componentType != COMPONENT_FLOAT)
    {
        throw GLTFException("Invalid componentType for inverse bind matrices accessor " + accessor.id);
    }

    return reader.ReadBinaryData<float>(doc, accessor);
}

std::vector<float> AnimationUtils::GetInverseBindMatrices(const Document& doc, const GLTFResourceReader& reader, const Skin& skin)
{
    auto& accessor = doc.accessors[skin.inverseBindMatricesAccessorId];
    return GetInverseBindMatrices(doc, reader, accessor);
}

std::vector<float> AnimationUtils::GetTranslations(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor)
{
    if (accessor.type != TYPE_VEC3)
    {
        throw GLTFException("Invalid type for translations accessor " + accessor.id);
    }

    if (accessor.componentType != COMPONENT_FLOAT)
    {
        throw GLTFException("Invalid componentType for translations accessor " + accessor.id);
    }

    return reader.ReadBinaryData<float>(doc, accessor);
}

std::vector<float> AnimationUtils::GetTranslations(const Document& doc, const GLTFResourceReader& reader, const AnimationSampler& sampler)
{
    auto& accessor = doc.accessors[sampler.outputAccessorId];
    return GetTranslations(doc, reader, accessor);
}

std::vector<float> AnimationUtils::GetRotations(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor)
{
    if (accessor.type != TYPE_VEC4)
    {
        throw GLTFException("Invalid type for rotations accessor " + accessor.id);
    }

    return reader.ReadFloatData(doc, accessor);
}

std::vector<float> AnimationUtils::GetRotations(const Document& doc, const GLTFResourceReader& reader, const AnimationSampler& sampler)
{
    auto& accessor = doc.accessors[sampler.outputAccessorId];
    return GetRotations(doc, reader, accessor);
}

std::vector<float> AnimationUtils::GetScales(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor)
{
    if (accessor.type != TYPE_VEC3)
    {
        throw GLTFException("Invalid type for scales accessor " + accessor.id);
    }

    if (accessor.componentType != COMPONENT_FLOAT)
    {
        throw GLTFException("Invalid componentType for scales accessor " + accessor.id);
    }

    return reader.ReadBinaryData<float>(doc, accessor);
}

std::vector<float> AnimationUtils::GetScales(const Document& doc, const GLTFResourceReader& reader, const AnimationSampler& sampler)
{
    auto& accessor = doc.accessors[sampler.outputAccessorId];
    return GetScales(doc, reader, accessor);
}

std::vector<float> AnimationUtils::GetMorphWeights(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor)
{
    if (accessor.type != TYPE_SCALAR)
    {
        throw GLTFException("Invalid type for weights accessor " + accessor.id);
    }

    return reader.ReadFloatData(doc, accessor);
}

std::vector<float> AnimationUtils::GetMorphWeights(const Document& doc, const GLTFResourceReader& reader, const AnimationSampler& sampler)
{
    auto& accessor = doc.accessors[sampler.outputAccessorId];
    return GetMorphWeights(doc, reader, accessor);
}
