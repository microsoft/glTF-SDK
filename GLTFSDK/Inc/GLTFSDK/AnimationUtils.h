// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <GLTFSDK/Definitions.h>

#include <vector>

namespace Microsoft
{
    namespace glTF
    {
        class Document;
        class GLTFResourceReader;
        struct Accessor;
        struct AnimationSampler;
        struct Skin;

        namespace AnimationUtils
        {
            std::vector<float> GLTFSDK_API GetKeyframeTimes(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GLTFSDK_API GetKeyframeTimes(const Document& doc, const GLTFResourceReader& reader, const AnimationSampler& sampler);

            std::vector<float> GLTFSDK_API GetInverseBindMatrices(const Document& doc, const GLTFResourceReader& reader, const Accessor& skin);
            std::vector<float> GLTFSDK_API GetInverseBindMatrices(const Document& doc, const GLTFResourceReader& reader, const Skin& skin);

            std::vector<float> GLTFSDK_API GetTranslations(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GLTFSDK_API GetTranslations(const Document& doc, const GLTFResourceReader& reader, const AnimationSampler& accessor);

            std::vector<float> GLTFSDK_API GetRotations(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GLTFSDK_API GetRotations(const Document& doc, const GLTFResourceReader& reader, const AnimationSampler& accessor);

            std::vector<float> GLTFSDK_API GetScales(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GLTFSDK_API GetScales(const Document& doc, const GLTFResourceReader& reader, const AnimationSampler& accessor);

            std::vector<float> GLTFSDK_API GetMorphWeights(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GLTFSDK_API GetMorphWeights(const Document& doc, const GLTFResourceReader& reader, const AnimationSampler& accessor);
        };
    }
}
