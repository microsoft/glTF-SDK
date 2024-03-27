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
            std::vector<float> GLTFSDK_CDECL GetKeyframeTimes(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GLTFSDK_CDECL GetKeyframeTimes(const Document& doc, const GLTFResourceReader& reader, const AnimationSampler& sampler);

            std::vector<float> GLTFSDK_CDECL GetInverseBindMatrices(const Document& doc, const GLTFResourceReader& reader, const Accessor& skin);
            std::vector<float> GLTFSDK_CDECL GetInverseBindMatrices(const Document& doc, const GLTFResourceReader& reader, const Skin& skin);

            std::vector<float> GLTFSDK_CDECL GetTranslations(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GLTFSDK_CDECL GetTranslations(const Document& doc, const GLTFResourceReader& reader, const AnimationSampler& accessor);

            std::vector<float> GLTFSDK_CDECL GetRotations(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GLTFSDK_CDECL GetRotations(const Document& doc, const GLTFResourceReader& reader, const AnimationSampler& accessor);

            std::vector<float> GLTFSDK_CDECL GetScales(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GLTFSDK_CDECL GetScales(const Document& doc, const GLTFResourceReader& reader, const AnimationSampler& accessor);

            std::vector<float> GLTFSDK_CDECL GetMorphWeights(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GLTFSDK_CDECL GetMorphWeights(const Document& doc, const GLTFResourceReader& reader, const AnimationSampler& accessor);
        };
    }
}
