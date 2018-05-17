// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <GLTFSDK/Color.h>
#include <GLTFSDK/ExtensionsKHR.h>

namespace Microsoft
{
    namespace glTF
    {
        namespace Detail
        {
            template<typename TColor>
            TColor DIELECTRIC_SPECULAR = TColor(0.04f, 0.04f, 0.04f);

            template<typename TColor>
            TColor BLACK = TColor(0.0f, 0.0f, 0.0f);

            template<typename TColor>
            inline float GetMaxComponent(const TColor& color)
            {
                // Access the color channels using R/G/B functions - this uses ADL to locate the functions in the same namespace as TColor
                const auto r = R(color);
                const auto g = G(color);
                const auto b = B(color);

                return std::max(std::max(r, g), b);
            }

            // http://www.itu.int/rec/R-REC-BT.601
            // http://www.itu.int/dms_pubrec/itu-r/rec/bt/R-REC-BT.601-7-201103-I!!PDF-E.pdf
            constexpr const float R_BRIGHTNESS_COEFF = 0.299f;
            constexpr const float G_BRIGHTNESS_COEFF = 0.587f;
            constexpr const float B_BRIGHTNESS_COEFF = 0.114f;

            template<typename TColor>
            inline float GetPerceivedBrightness(const TColor& color)
            {
                const TColor value = TColor(R_BRIGHTNESS_COEFF, G_BRIGHTNESS_COEFF, B_BRIGHTNESS_COEFF) * (color * color);

                // Access the color channels using R/G/B functions - this uses ADL to locate the functions in the same namespace as TColor
                const auto r = R(value);
                const auto g = G(value);
                const auto b = B(value);

                return std::sqrt(r + g + b);
            }

            float SolveMetallic(float dielectricSpecular, float diffuse, float specular, float oneMinusSpecularStrength);
        }

        template<typename TColor>
        struct SpecularGlossinessValueTypeless
        {
            SpecularGlossinessValueTypeless() :
                diffuse(1.0f, 1.0f, 1.0f),
                opacity(1.0f),
                specular(1.0f, 1.0f, 1.0f),
                glossiness(1.0f)
            {
            }

            explicit SpecularGlossinessValueTypeless(const KHR::Materials::PBRSpecularGlossiness& sg) :
                diffuse(sg.diffuseFactor.r, sg.diffuseFactor.g, sg.diffuseFactor.b),
                opacity(sg.diffuseFactor.a),
                specular(sg.specularFactor.r, sg.specularFactor.g, sg.specularFactor.b),
                glossiness(sg.glossinessFactor)
            {
            }

            TColor diffuse;
            float opacity;
            TColor specular;
            float glossiness;
        };

        using SpecularGlossinessValue = SpecularGlossinessValueTypeless<Color3>;

        template<typename TColor>
        struct MetallicRoughnessValueTypeless
        {
            MetallicRoughnessValueTypeless() :
                base(1.0f, 1.0f, 1.0f),
                opacity(1.0f),
                metallic(1.0f),
                roughness(1.0f)
            {
            }

            explicit MetallicRoughnessValueTypeless(const Material::PBRMetallicRoughness& mr) :
                base(mr.baseColorFactor.r, mr.baseColorFactor.g, mr.baseColorFactor.b),
                opacity(mr.baseColorFactor.a),
                metallic(mr.metallicFactor),
                roughness(mr.roughnessFactor)
            {
            }

            TColor base;
            float opacity;
            float metallic;
            float roughness;
        };

        using MetallicRoughnessValue = MetallicRoughnessValueTypeless<Color3>;

        template<typename TColor>
        inline MetallicRoughnessValueTypeless<TColor> SGToMR(const SpecularGlossinessValueTypeless<TColor>& sg)
        {
            using namespace Detail;

            const float oneMinusSpecularStrength = 1.0f - GetMaxComponent(sg.specular);
            const float dielectricSpecularRed = R(DIELECTRIC_SPECULAR<TColor>);

            const auto brightnessDiffuse = GetPerceivedBrightness(sg.diffuse);
            const auto brightnessSpecular = GetPerceivedBrightness(sg.specular);

            const float metallic = SolveMetallic(dielectricSpecularRed, brightnessDiffuse, brightnessSpecular, oneMinusSpecularStrength);
            const float oneMinusMetallic = 1.0f - metallic;

            const TColor baseColorFromDiffuse = sg.diffuse * (oneMinusSpecularStrength / (1.0f - dielectricSpecularRed) / std::max((oneMinusMetallic), std::numeric_limits<float>::epsilon()));
            const TColor baseColorFromSpecular = (sg.specular - (DIELECTRIC_SPECULAR<TColor> * (oneMinusMetallic))) * (1.0f / std::max(metallic, std::numeric_limits<float>::epsilon()));
            const TColor baseColor = TColor::Clamp(TColor::Lerp(baseColorFromDiffuse, baseColorFromSpecular, metallic * metallic), 0.0f, 1.0f);

            MetallicRoughnessValueTypeless<TColor> mr;
            mr.base = baseColor;
            mr.opacity = sg.opacity;
            mr.metallic = metallic;
            mr.roughness = 1.0f - sg.glossiness;
            return mr;
        }

        MetallicRoughnessValue SGToMR(const SpecularGlossinessValue& sg);

        template<typename TColor>
        inline SpecularGlossinessValueTypeless<TColor> MRToSG(const MetallicRoughnessValueTypeless<TColor>& mr)
        {
            using namespace Detail;

            const auto specular = TColor::Lerp(DIELECTRIC_SPECULAR<TColor>, mr.base, mr.metallic);
            const float oneMinusSpecularStrength = 1.0f - GetMaxComponent(specular);
            const auto diffuse = oneMinusSpecularStrength < std::numeric_limits<float>::epsilon()
                ? BLACK<TColor>
                : mr.base * ((1.0f - R(DIELECTRIC_SPECULAR<TColor>)) * (1.0f - mr.metallic) / oneMinusSpecularStrength);

            SpecularGlossinessValueTypeless<TColor> sg;
            sg.diffuse = diffuse;
            sg.opacity = mr.opacity;
            sg.specular = specular;
            sg.glossiness = 1.0f - mr.roughness;
            return sg;
        }

        SpecularGlossinessValue MRToSG(const MetallicRoughnessValue& mr);
    }
}
