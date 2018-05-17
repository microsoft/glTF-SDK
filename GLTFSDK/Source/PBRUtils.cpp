// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/PBRUtils.h>

using namespace Microsoft::glTF;

// https://bghgary.github.io/glTF/convert-between-workflows-bjs/js/babylon.pbrUtilities.js
float Detail::SolveMetallic(float dielectricSpecular, float diffuse, float specular, float oneMinusSpecularStrength)
{
    if (specular <= dielectricSpecular)
    {
        return 0.0f;
    }

    const float a = dielectricSpecular;
    const float b = diffuse * oneMinusSpecularStrength / (1.0f - dielectricSpecular) + specular - 2.0f * dielectricSpecular;
    const float c = dielectricSpecular - specular;
    const float D = b * b - 4.0f * a * c;
    return Math::Clamp((-b + std::sqrt(D)) / (2.0f * a), 0.0f, 1.0f);
}

namespace Microsoft
{
    namespace glTF
    {
        constexpr float R(const Color3& color)
        {
            return color.r;
        }

        constexpr float G(const Color3& color)
        {
            return color.g;
        }

        constexpr float B(const Color3& color)
        {
            return color.b;
        }
    }
}

MetallicRoughnessValue Microsoft::glTF::SGToMR(const SpecularGlossinessValue& sg)
{
    return SGToMR<Color3>(sg);
}

SpecularGlossinessValue Microsoft::glTF::MRToSG(const MetallicRoughnessValue& mr)
{
    return MRToSG<Color3>(mr);
}
