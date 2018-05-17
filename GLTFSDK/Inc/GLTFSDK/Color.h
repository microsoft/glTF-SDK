// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <cstdint>

namespace Microsoft
{
    namespace glTF
    {
        template<typename TColor>
        struct ColorBase
        {
            using Super = ColorBase<TColor>;

            static TColor Lerp(const TColor& start, const TColor& end, float amount)
            {
                return (start * (1.0f - amount)) + (end * amount);
            }

            friend TColor operator*(const TColor& lhs, const TColor& rhs)
            {
                TColor lhsCopy = lhs;

                // Implemented using TColor::operator*=(const TColor&)
                lhsCopy *= rhs;
                return lhsCopy;
            }

            friend TColor operator*(TColor lhs, float rhs)
            {
                // Implemented using TColor::operator*=(float)
                lhs *= rhs;
                return lhs;
            }

            friend TColor operator*(float lhs, TColor rhs)
            {
                // Implemented using TColor::operator*=(float) - takes advantage of commutative nature of scalar/vector multiplication
                rhs *= lhs;
                return rhs;
            }

            friend TColor operator/(const TColor& lhs, const TColor& rhs)
            {
                TColor lhsCopy = lhs;

                // Implemented using TColor::operator/=(const TColor&)
                lhsCopy /= rhs;
                return lhsCopy;
            }

            friend TColor operator/(TColor lhs, float rhs)
            {
                // Implemented using TColor::operator/=(float)
                lhs /= rhs;
                return lhs;
            }

            friend TColor operator/(float lhs, TColor rhs)
            {
                // Implemented using operator/(const TColor&, const TColor&)
                return TColor::FromScalar(lhs) / rhs;
            }

            friend TColor operator+(const TColor& lhs, const TColor& rhs)
            {
                TColor lhsCopy = lhs;

                // Implemented using TColor::operator+=(const TColor&)
                lhsCopy += rhs;
                return lhsCopy;
            }

            friend TColor operator+(TColor lhs, float rhs)
            {
                // Implemented using TColor::operator+=(float)
                lhs += rhs;
                return lhs;
            }

            friend TColor operator+(float lhs, TColor rhs)
            {
                // Implemented using TColor::operator+=(float) - takes advantage of commutative nature of scalar/vector addition
                rhs += lhs;
                return rhs;
            }

            friend TColor operator-(const TColor& lhs, const TColor& rhs)
            {
                TColor lhsCopy = lhs;

                // Implemented using TColor::operator-=(const TColor&)
                lhsCopy -= rhs;
                return lhsCopy;
            }

            friend TColor operator-(TColor lhs, float rhs)
            {
                // Implemented using TColor::operator-=(float)
                lhs -= rhs;
                return lhs;
            }

            friend TColor operator-(float lhs, TColor rhs)
            {
                // Implemented using operator-(const TColor&, const TColor&)
                return TColor::FromScalar(lhs) - rhs;
            }
        };

        struct Color4; // Forward declaration required for Color3::AsColor4 to have a return type of Color4

        struct Color3 : private ColorBase<Color3>
        {
            using Super::Lerp;

            float r;
            float g;
            float b;

            Color3(float r, float g, float b);
            Color3(uint8_t r, uint8_t g, uint8_t b);

            static Color3 FromScalar(float value);

            Color3& operator*=(const Color3& rhs);
            Color3& operator*=(float rhs);

            Color3& operator/=(const Color3& rhs);
            Color3& operator/=(float rhs);

            Color3& operator+=(const Color3& rhs);
            Color3& operator+=(float rhs);

            Color3& operator-=(const Color3& rhs);
            Color3& operator-=(float rhs);

            Color3 ToGamma() const;
            Color3 ToLinear() const;

            Color4 AsColor4(float a = 1.0f) const;

            uint32_t AsUint32RGBA() const;
            uint32_t AsUint32BGRA() const;

            static Color3 FromUint32RGBA(uint32_t color);
            static Color3 FromUint32BGRA(uint32_t color);

            static Color3 Clamp(const Color3& color, float lo, float hi);
        };

        bool operator==(const Color3& lhs, const Color3& rhs);
        bool operator!=(const Color3& lhs, const Color3& rhs);

        struct Color4 : private ColorBase<Color4>
        {
            using Super::Lerp;

            float r;
            float g;
            float b;
            float a;

            Color4(float r, float g, float b, float a);
            Color4(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

            static Color4 FromScalar(float value);

            Color4& operator*=(const Color4& rhs);
            Color4& operator*=(float);

            Color4& operator/=(const Color4& rhs);
            Color4& operator/=(float);

            Color4& operator+=(const Color4& rhs);
            Color4& operator+=(float);

            Color4& operator-=(const Color4& rhs);
            Color4& operator-=(float);

            Color3 AsColor3() const;

            uint32_t AsUint32RGBA() const;
            uint32_t AsUint32BGRA() const;

            static Color4 FromUint32RGBA(uint32_t color);
            static Color4 FromUint32BGRA(uint32_t color);

            static Color4 Clamp(const Color4& color, float lo, float hi);
        };

        bool operator==(const Color4& lhs, const Color4& rhs);
        bool operator!=(const Color4& lhs, const Color4& rhs);
    }
}
