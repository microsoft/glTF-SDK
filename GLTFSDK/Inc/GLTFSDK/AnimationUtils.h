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
        struct AnimationSampler;
        struct Skin;

        namespace AnimationUtils
        {
            std::vector<float> GetKeyframeTimes(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GetKeyframeTimes(const Document& doc, const GLTFResourceReader& reader, const AnimationSampler& sampler);

            std::vector<float> GetInverseBindMatrices(const Document& doc, const GLTFResourceReader& reader, const Accessor& skin);
            std::vector<float> GetInverseBindMatrices(const Document& doc, const GLTFResourceReader& reader, const Skin& skin);

            std::vector<float> GetTranslations(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GetTranslations(const Document& doc, const GLTFResourceReader& reader, const AnimationSampler& accessor);

            std::vector<float> GetRotations(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GetRotations(const Document& doc, const GLTFResourceReader& reader, const AnimationSampler& accessor);

            std::vector<float> GetScales(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GetScales(const Document& doc, const GLTFResourceReader& reader, const AnimationSampler& accessor);

            std::vector<float> GetMorphWeights(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor);
            std::vector<float> GetMorphWeights(const Document& doc, const GLTFResourceReader& reader, const AnimationSampler& accessor);
        };
    }
}
