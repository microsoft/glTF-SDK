// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <array>
#include <cmath>

namespace Microsoft
{
    namespace glTF
    {
        struct Matrix4
        {
            Matrix4();

            bool operator==(const Matrix4& other) const;
            bool operator!=(const Matrix4& other) const;

            std::array<float, 16> values;

            static const Matrix4 IDENTITY;
        };

        struct Vector2
        {
            Vector2();
            Vector2(float x, float y);

            bool operator==(const Vector2& other) const;
            bool operator!=(const Vector2& other) const;

            float x;
            float y;

            static const Vector2 ZERO;
            static const Vector2 ONE;
        };

        struct Vector3
        {
            Vector3();
            Vector3(float x, float y, float z);

            bool operator==(const Vector3& other) const;
            bool operator!=(const Vector3& other) const;

            float x;
            float y;
            float z;

            static const Vector3 ZERO;
            static const Vector3 ONE;
        };

        struct Quaternion
        {
            Quaternion();
            Quaternion(float x, float y, float z, float w);

            bool operator==(const Quaternion& other) const;
            bool operator!=(const Quaternion& other) const;

            float x;
            float y;
            float z;
            float w;

            static const Quaternion IDENTITY;
        };

        namespace Math
        {
            template<class T>
            const T& Clamp(const T& v, const T& lo, const T& hi) {
                return std::min(hi, std::max(v, lo));
            }

            // https://en.wikipedia.org/wiki/SRGB#The_reverse_transformation
            inline float ToLinear(float value)
            {
                if (value < 0.04045f)
                {
                    return value / 12.92f;
                }

                return std::powf((value + 0.055f) / 1.055f, 2.4f);
            }

            // https://en.wikipedia.org/wiki/SRGB#The_forward_transformation_.28CIE_XYZ_to_sRGB.29
            inline float ToGamma(float value)
            {
                if (value <= 0.0031308f)
                {
                    return value * 12.92f;
                }

                return 1.055f * std::powf(value, 1.0f / 2.4f) - 0.055f;
            }

            inline float ByteToFloat(uint8_t value)
            {
                return value / 255.0f;
            }

            inline uint8_t FloatToByte(float value)
            {
                return static_cast<uint8_t>(value * 255.0f + 0.5f);
            }
        }
    }
}
