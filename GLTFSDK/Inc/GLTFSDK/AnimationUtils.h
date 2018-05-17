// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <vector>
#include <algorithm>
#include <cmath>

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

            //  Conversions of normalized component types to/from floats are explicitly defined in the 2.0 spec
            inline float ComponentToFloat(const float w)   { return w; }
            inline float ComponentToFloat(const int8_t w)  { return std::max(static_cast<float>(w) / 127.0f, -1.0f); }
            inline float ComponentToFloat(const uint8_t w) { return static_cast<float>(w) / 255.0f; }
            inline float ComponentToFloat(const int16_t w) { return std::max(static_cast<float>(w) / 32767.0f, -1.0f); }
            inline float ComponentToFloat(const uint16_t w){ return static_cast<float>(w) / 65535.0f; }

            template<typename T>
            inline T FloatToComponent(const float f)
            {
                static_assert(std::is_same<float, T>::value, "Microsoft::glTF::AnimationUtils::FloatToComponent: expecting float template type");
                return f;
            }
            template<> inline int8_t   FloatToComponent<int8_t>(const float f)  { return static_cast<int8_t>(std::round(f*127.0f)); }
            template<> inline uint8_t  FloatToComponent<uint8_t>(const float f) { return static_cast<uint8_t>(std::round(f*255.0f)); }
            template<> inline int16_t  FloatToComponent<int16_t>(const float f) { return static_cast<int16_t>(std::round(f*32767.0f)); }
            template<> inline uint16_t FloatToComponent<uint16_t>(const float f){ return static_cast<uint16_t>(std::round(f*65535.0f)); }
        };
    }
}
