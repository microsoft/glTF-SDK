// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/Math.h>

using namespace Microsoft::glTF;

const Matrix4 Matrix4::IDENTITY = Matrix4();
const Vector2 Vector2::ZERO = { 0.0f, 0.0f };
const Vector2 Vector2::ONE = { 1.0f, 1.0f };
const Vector3 Vector3::ZERO = { 0.0f, 0.0f, 0.0f };
const Vector3 Vector3::ONE = { 1.0f, 1.0f, 1.0f };
const Quaternion Quaternion::IDENTITY = { 0.0f, 0.0f, 0.0f, 1.0f };

Matrix4::Matrix4()
    : values({{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    }})
{
}

bool Matrix4::operator==(const Matrix4& other) const
{
    return values == other.values;
}

bool Matrix4::operator!=(const Matrix4& other) const
{
    return !(*this == other);
}

Vector2::Vector2()
    : x(0.0f), y(0.0f)
{
}

Vector2::Vector2(float x, float y)
    : x(x), y(y)
{
}

bool Vector2::operator==(const Vector2& other) const
{
    return std::tie(x, y) == std::tie(other.x, other.y);
}

bool Vector2::operator!=(const Vector2& other) const
{
    return !operator==(other);
}

Vector3::Vector3()
    : x(0.0f), y(0.0f), z(0.0f)
{
}

Vector3::Vector3(float x, float y, float z)
    : x(x), y(y), z(z)
{
}

bool Vector3::operator==(const Vector3& other) const
{
    return std::tie(x, y, z) == std::tie(other.x, other.y, other.z);
}

bool Vector3::operator!=(const Vector3& other) const
{
    return !operator==(other);
}

Quaternion::Quaternion() :
    x(0.0f), y(0.0f), z(0.0f), w(1.0f)
{
}

Quaternion::Quaternion(float x, float y, float z, float w)
    : x(x), y(y), z(z), w(w)
{
}

bool Quaternion::operator==(const Quaternion& other) const
{
    return std::tie(x, y, z, w) == std::tie(other.x, other.y, other.z, other.w);
}

bool Quaternion::operator!=(const Quaternion& other) const
{
    return !operator==(other);
}

