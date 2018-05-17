// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/Color.h>

#include <GLTFSDK/Math.h>

using namespace Microsoft::glTF;

Color3::Color3(float r, float g, float b) :
    r(r),
    g(g),
    b(b)
{
}

Color3::Color3(uint8_t r, uint8_t g, uint8_t b) :
    r(Math::ByteToFloat(r)),
    g(Math::ByteToFloat(g)),
    b(Math::ByteToFloat(b))
{
}

Color3 Color3::FromScalar(float value)
{
    return { value, value, value };
}

Color3& Color3::operator*=(const Color3& rhs)
{
    r *= rhs.r;
    g *= rhs.g;
    b *= rhs.b;

    return *this;
}

Color3& Color3::operator*=(float rhs)
{
    r *= rhs;
    g *= rhs;
    b *= rhs;

    return *this;
}

Color3& Color3::operator/=(const Color3& rhs)
{
    r /= rhs.r;
    g /= rhs.g;
    b /= rhs.b;

    return *this;
}

Color3& Color3::operator/=(float rhs)
{
    r /= rhs;
    g /= rhs;
    b /= rhs;

    return *this;
}

Color3& Color3::operator+=(const Color3& rhs)
{
    r += rhs.r;
    g += rhs.g;
    b += rhs.b;

    return *this;
}

Color3& Color3::operator+=(float rhs)
{
    r += rhs;
    g += rhs;
    b += rhs;

    return *this;
}

Color3& Color3::operator-=(const Color3& rhs)
{
    r -= rhs.r;
    g -= rhs.g;
    b -= rhs.b;

    return *this;
}

Color3& Color3::operator-=(float rhs)
{
    r -= rhs;
    g -= rhs;
    b -= rhs;

    return *this;
}

Color3 Color3::ToGamma() const
{
    return Color3(Math::ToGamma(r), Math::ToGamma(g), Math::ToGamma(b));
}

Color3 Color3::ToLinear() const
{
    return Color3(Math::ToLinear(r), Math::ToLinear(g), Math::ToLinear(b));
}

Color4 Color3::AsColor4(float a) const
{
    return { r, g, b, a };
}

uint32_t Color3::AsUint32RGBA() const
{
    const uint8_t rByte = Math::FloatToByte(r);
    const uint8_t gByte = Math::FloatToByte(g);
    const uint8_t bByte = Math::FloatToByte(b);
    const uint8_t aByte = std::numeric_limits<uint8_t>::max();

    const uint32_t rgba = aByte << 24 | bByte << 16 | gByte << 8 | rByte;
    return rgba;
}

uint32_t Color3::AsUint32BGRA() const
{
    const uint8_t bByte = Math::FloatToByte(b);
    const uint8_t gByte = Math::FloatToByte(g);
    const uint8_t rByte = Math::FloatToByte(r);
    const uint8_t aByte = std::numeric_limits<uint8_t>::max();

    const uint32_t bgra = aByte << 24 | rByte << 16 | gByte << 8 | bByte;
    return bgra;
}

Color3 Color3::FromUint32RGBA(uint32_t color)
{
    const uint8_t r = static_cast<uint8_t>( color & 0x000000ff);
    const uint8_t g = static_cast<uint8_t>((color & 0x0000ff00) >> 8);
    const uint8_t b = static_cast<uint8_t>((color & 0x00ff0000) >> 16);

    return Color3(r, g, b);
}

Color3 Color3::FromUint32BGRA(uint32_t color)
{
    const uint8_t b = static_cast<uint8_t>( color & 0x000000ff);
    const uint8_t g = static_cast<uint8_t>((color & 0x0000ff00) >> 8);
    const uint8_t r = static_cast<uint8_t>((color & 0x00ff0000) >> 16);

    return Color3(r, g, b);
}

Color3 Color3::Clamp(const Color3& color, float lo, float hi)
{
    return {
        Math::Clamp(color.r, lo, hi),
        Math::Clamp(color.g, lo, hi),
        Math::Clamp(color.b, lo, hi)
    };
}

bool Microsoft::glTF::operator==(const Color3& lhs, const Color3& rhs)
{
    return lhs.r == rhs.r
        && lhs.g == rhs.g
        && lhs.b == rhs.b;
}

bool Microsoft::glTF::operator!=(const Color3& lhs, const Color3& rhs)
{
    return !(lhs == rhs);
}

Color4::Color4(float r, float g, float b, float a) :
    r(r),
    g(g),
    b(b),
    a(a)
{
}

Color4::Color4(uint8_t r, uint8_t g, uint8_t b, uint8_t a) :
    r(Math::ByteToFloat(r)),
    g(Math::ByteToFloat(g)),
    b(Math::ByteToFloat(b)),
    a(Math::ByteToFloat(a))
{
}

Color4 Color4::FromScalar(float value)
{
    return { value, value, value, value };
}

Color4& Color4::operator*=(const Color4& rhs)
{
    r *= rhs.r;
    g *= rhs.g;
    b *= rhs.b;
    a *= rhs.a;

    return *this;
}

Color4& Color4::operator*=(float rhs)
{
    r *= rhs;
    g *= rhs;
    b *= rhs;
    a *= rhs;

    return *this;
}

Color4& Color4::operator/=(const Color4& rhs)
{
    r /= rhs.r;
    g /= rhs.g;
    b /= rhs.b;
    a /= rhs.a;

    return *this;
}

Color4& Color4::operator/=(float rhs)
{
    r /= rhs;
    g /= rhs;
    b /= rhs;
    a /= rhs;

    return *this;
}

Color4& Color4::operator+=(const Color4& rhs)
{
    r += rhs.r;
    g += rhs.g;
    b += rhs.b;
    a += rhs.a;

    return *this;
}

Color4& Color4::operator+=(float rhs)
{
    r += rhs;
    g += rhs;
    b += rhs;
    a += rhs;

    return *this;
}

Color4& Color4::operator-=(const Color4& rhs)
{
    r -= rhs.r;
    g -= rhs.g;
    b -= rhs.b;
    a -= rhs.a;

    return *this;
}

Color4& Color4::operator-=(float rhs)
{
    r -= rhs;
    g -= rhs;
    b -= rhs;
    a -= rhs;

    return *this;
}

Color3 Color4::AsColor3() const
{
    return { r, g, b };
}

uint32_t Color4::AsUint32RGBA() const
{
    const uint8_t rByte = Math::FloatToByte(r);
    const uint8_t gByte = Math::FloatToByte(g);
    const uint8_t bByte = Math::FloatToByte(b);
    const uint8_t aByte = Math::FloatToByte(a);

    const uint32_t rgba = aByte << 24 | bByte << 16 | gByte << 8 | rByte;
    return rgba;
}

uint32_t Color4::AsUint32BGRA() const
{
    const uint8_t bByte = Math::FloatToByte(b);
    const uint8_t gByte = Math::FloatToByte(g);
    const uint8_t rByte = Math::FloatToByte(r);
    const uint8_t aByte = Math::FloatToByte(a);

    const uint32_t bgra = aByte << 24 | rByte << 16 | gByte << 8 | bByte;
    return bgra;
}

Color4 Color4::FromUint32RGBA(uint32_t color)
{
    const uint8_t r = static_cast<uint8_t>( color & 0x000000ff);
    const uint8_t g = static_cast<uint8_t>((color & 0x0000ff00) >> 8);
    const uint8_t b = static_cast<uint8_t>((color & 0x00ff0000) >> 16);
    const uint8_t a = static_cast<uint8_t>((color & 0xff000000) >> 24);

    return Color4(r, g, b, a);
}

Color4 Color4::FromUint32BGRA(uint32_t color)
{
    const uint8_t b = static_cast<uint8_t>( color & 0x000000ff);
    const uint8_t g = static_cast<uint8_t>((color & 0x0000ff00) >> 8);
    const uint8_t r = static_cast<uint8_t>((color & 0x00ff0000) >> 16);
    const uint8_t a = static_cast<uint8_t>((color & 0xff000000) >> 24);

    return Color4(r, g, b, a);
}

Color4 Color4::Clamp(const Color4& color, float lo, float hi)
{
    return {
        Math::Clamp(color.r, lo, hi),
        Math::Clamp(color.g, lo, hi),
        Math::Clamp(color.b, lo, hi),
        Math::Clamp(color.a, lo, hi)
    };
}

bool Microsoft::glTF::operator==(const Color4& lhs, const Color4& rhs)
{
    return lhs.r == rhs.r
        && lhs.g == rhs.g
        && lhs.b == rhs.b
        && lhs.a == rhs.a;
}

bool Microsoft::glTF::operator!=(const Color4& lhs, const Color4& rhs)
{
    return !(lhs == rhs);
}
