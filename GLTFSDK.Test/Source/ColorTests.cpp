// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/Color.h>

#include <cmath>

using namespace glTF::UnitTest;

namespace Microsoft
{
    namespace glTF
    {
        std::wstring ToString(const Color3& color)
        {
            std::wstringstream ss;

            ss << L"{ r = " << color.r;
            ss << L", g = " << color.g;
            ss << L", b = " << color.b << " }";

            return ss.str();
        }

        std::wstring ToString(const Color4& color)
        {
            std::wstringstream ss;

            ss << L"{ r = " << color.r;
            ss << L", g = " << color.g;
            ss << L", b = " << color.b;
            ss << L", a = " << color.a << " }";

            return ss.str();
        }

        namespace Test
        {
            GLTFSDK_TEST_CLASS(ColorTests)
            {
                GLTFSDK_TEST_METHOD(ColorTests, Color3Lerp)
                {
                    const Color3 c1 = { 0.0f, 0.0f, 0.0f };
                    const Color3 c2 = { 1.0f, 1.0f, 1.0f };

                    {
                        Color3 cRes = Color3::Lerp(c1, c2, 0.0f);
                        Assert::AreEqual(c1, cRes, L"Color3::Lerp with interpolation amount of zero didn't equal the start value");
                    }

                    {
                        Color3 cRes = Color3::Lerp(c1, c2, 1.0f);
                        Assert::AreEqual(c2, cRes, L"Color3::Lerp with interpolation amount of one didn't equal the end value");
                    }

                    {
                        Color3 cRes = Color3::Lerp(c1, c2, 0.5f);
                        Assert::AreEqual({ 0.5f, 0.5f, 0.5f }, cRes, L"Color3::Lerp with interpolation amount of half didn't produce the expected result");
                    }
                }

                GLTFSDK_TEST_METHOD(ColorTests, Color4Lerp)
                {
                    const Color4 c1 = { 0.0f, 0.0f, 0.0f, 0.0f };
                    const Color4 c2 = { 1.0f, 1.0f, 1.0f, 1.0f };

                    {
                        Color4 cRes = Color4::Lerp(c1, c2, 0.0f);
                        Assert::AreEqual(c1, cRes, L"Color4::Lerp with interpolation amount of zero didn't equal the start value");
                    }

                    {
                        Color4 cRes = Color4::Lerp(c1, c2, 1.0f);
                        Assert::AreEqual(c2, cRes, L"Color4::Lerp with interpolation amount of one didn't equal the end value");
                    }

                    {
                        Color4 cRes = Color4::Lerp(c1, c2, 0.5f);
                        Assert::AreEqual({ 0.5f, 0.5f, 0.5f, 0.5f }, cRes, L"Color4::Lerp with interpolation amount of half didn't produce the expected result");
                    }
                }

                GLTFSDK_TEST_METHOD(ColorTests, OperatorMultiply)
                {
                    const Color3 c1 = { 0.0f, 1.0f, 2.0f };
                    const Color3 c2 = { 2.0f, 2.0f, 2.0f };

                    {
                        Color3 cRes = c1 * c2;
                        Assert::AreEqual({ 0.0f, 2.0f, 4.0f }, cRes, L"Operator: operator*(const Color3&, const Color3&) didn't produce the expected result");
                    }

                    {
                        Color3 cRes = c2 * c1; // Ensure operator is commutative
                        Assert::AreEqual({ 0.0f, 2.0f, 4.0f }, cRes, L"Operator: operator*(const Color3&, const Color3&) didn't produce the expected result");
                    }

                    {
                        Color3 cRes = c1 * 2.0f;
                        Assert::AreEqual({ 0.0f, 2.0f, 4.0f }, cRes, L"Operator: operator*(Color3, float) didn't produce the expected result");
                    }

                    {
                        Color3 cRes = 2.0f * c1;
                        Assert::AreEqual({ 0.0f, 2.0f, 4.0f }, cRes, L"Operator: operator*(float, Color3) didn't produce the expected result");
                    }
                }

                GLTFSDK_TEST_METHOD(ColorTests, OperatorDivide)
                {
                    const Color3 c1 = { 0.0f, 1.0f, 2.0f };
                    const Color3 c2 = { 2.0f, 2.0f, 2.0f };

                    auto fnListsEqual = [](const std::initializer_list<float>& lhs, const std::initializer_list<float>& rhs)
                    {
                        return std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs));
                    };

                    {
                        Color3 cRes = c1 / c2;
                        Assert::AreEqual({ 0.0f, 0.5f, 1.0f }, cRes, L"Operator: operator/(const Color3&, const Color3&) didn't produce the expected result");
                    }

                    {
                        Color3 cRes = c2 / c1;

                        Assert::IsTrue(std::isinf(cRes.r)); // Test the red channel separately
                        Assert::IsTrue(fnListsEqual({ 2.0f, 1.0f }, { cRes.g, cRes.b }), L"Operator: operator/(const Color3&, const Color3&) didn't produce the expected result");
                    }

                    {
                        Color3 cRes = c1 / 2.0f;
                        Assert::AreEqual({ 0.0f, 0.5f, 1.0f }, cRes, L"Operator: operator/(Color3, float) didn't produce the expected result");
                    }

                    {
                        Color3 cRes = 2.0f / c1;

                        Assert::IsTrue(std::isinf(cRes.r)); // Test the red channel separately
                        Assert::IsTrue(fnListsEqual({ 2.0f, 1.0f }, { cRes.g, cRes.b }), L"Operator: operator/(float, Color3) didn't produce the expected result");
                    }
                }

                GLTFSDK_TEST_METHOD(ColorTests, OperatorAdd)
                {
                    const Color3 c1 = { 0.0f, 1.0f, 2.0f };
                    const Color3 c2 = { 2.0f, 2.0f, 2.0f };

                    {
                        Color3 cRes = c1 + c2;
                        Assert::AreEqual({ 2.0f, 3.0f, 4.0f }, cRes, L"Operator: operator+(const Color3&, const Color3&) didn't produce the expected result");
                    }

                    {
                        Color3 cRes = c2 + c1; // Ensure operator is commutative
                        Assert::AreEqual({ 2.0f, 3.0f, 4.0f }, cRes, L"Operator: operator+(const Color3&, const Color3&) didn't produce the expected result");
                    }

                    {
                        Color3 cRes = c1 + 2.0f;
                        Assert::AreEqual({ 2.0f, 3.0f, 4.0f }, cRes, L"Operator: operator+(Color3, float) didn't produce the expected result");
                    }

                    {
                        Color3 cRes = 2.0f + c1;
                        Assert::AreEqual({ 2.0f, 3.0f, 4.0f }, cRes, L"Operator: operator+(float, Color3) didn't produce the expected result");
                    }
                }

                GLTFSDK_TEST_METHOD(ColorTests, OperatorSubtract)
                {
                    const Color3 c1 = { 0.0f, 1.0f, 2.0f };
                    const Color3 c2 = { 2.0f, 2.0f, 2.0f };

                    {
                        Color3 cRes = c1 - c2;
                        Assert::AreEqual({ -2.0f, -1.0f, 0.0f }, cRes, L"Operator: operator-(const Color3&, const Color3&) didn't produce the expected result");
                    }

                    {
                        Color3 cRes = c2 - c1;
                        Assert::AreEqual({ 2.0f, 1.0f, 0.0f }, cRes, L"Operator: operator-(const Color3&, const Color3&) didn't produce the expected result");
                    }

                    {
                        Color3 cRes = c1 - 2.0f;
                        Assert::AreEqual({ -2.0f, -1.0f, 0.0f }, cRes, L"Operator: operator-(Color3, float) didn't produce the expected result");
                    }

                    {
                        Color3 cRes = 2.0f - c1;
                        Assert::AreEqual({ 2.0f, 1.0f, 0.0f }, cRes, L"Operator: operator-(float, Color3) didn't produce the expected result");
                    }
                }

                GLTFSDK_TEST_METHOD(ColorTests, Color3Clamp)
                {
                    const Color3 c = { -1.0f, 0.0f, +1.0f };

                    {
                        Color3 cRes = Color3::Clamp(c, 0.0f, +1.0f);
                        Assert::AreEqual({ 0.0f, 0.0f, +1.0f }, cRes);
                    }

                    {
                        Color3 cRes = Color3::Clamp(c, -1.0f, 0.0f);
                        Assert::AreEqual({ -1.0f, 0.0f, 0.0f }, cRes);
                    }

                    {
                        Color3 cRes = Color3::Clamp(c, -0.5f, +0.5f);
                        Assert::AreEqual({ -0.5f, 0.0f, +0.5f }, cRes);
                    }
                }

                GLTFSDK_TEST_METHOD(ColorTests, Color4Clamp)
                {
                    const Color4 c = { -1.0f, 0.0f, 0.0f, +1.0f };

                    {
                        Color4 cRes = Color4::Clamp(c, 0.0f, +1.0f);
                        Assert::AreEqual({ 0.0f, 0.0f, 0.0f, +1.0f }, cRes);
                    }

                    {
                        Color4 cRes = Color4::Clamp(c, -1.0f, 0.0f);
                        Assert::AreEqual({ -1.0f, 0.0f, 0.0f, 0.0f }, cRes);
                    }

                    {
                        Color4 cRes = Color4::Clamp(c, -0.5f, +0.5f);
                        Assert::AreEqual({ -0.5f, 0.0f, 0.0f, +0.5f }, cRes);
                    }
                }

                GLTFSDK_TEST_METHOD(ColorTests, Color3Uint32RGBA)
                {
                    const Color3 cIn = {
                        static_cast<uint8_t>(0x3F),
                        static_cast<uint8_t>(0x1F),
                        static_cast<uint8_t>(0x0F)
                    };

                    const auto cValue = cIn.AsUint32RGBA(); // Alpha channel (MSB) is assigned 0xFF

                    Assert::AreEqual(0xFF0F1F3Fu, cValue);
                    const Color3 cOut = Color3::FromUint32RGBA(cValue);
                    Assert::AreEqual(cIn, cOut);
                }

                GLTFSDK_TEST_METHOD(ColorTests, Color4Uint32RGBA)
                {
                    const Color4 cIn = {
                        static_cast<uint8_t>(0x7F),
                        static_cast<uint8_t>(0x3F),
                        static_cast<uint8_t>(0x1F),
                        static_cast<uint8_t>(0x0F)
                    };

                    const auto cValue = cIn.AsUint32RGBA();

                    Assert::AreEqual(0x0F1F3F7Fu, cValue);
                    const Color4 cOut = Color4::FromUint32RGBA(cValue);
                    Assert::AreEqual(cIn, cOut);
                }

                GLTFSDK_TEST_METHOD(ColorTests, Color3AsColor4)
                {
                    {
                        const Color3 c3(0.25f, 0.35f, 0.45f);
                        const Color4 c4 = c3.AsColor4(); // The alpha channel should default to 1.0f

                        Assert::AreEqual({ 0.25f, 0.35f, 0.45f, 1.0f }, c4);
                    }

                    {
                        const Color3 c3(0.25f, 0.35f, 0.45f);
                        const Color4 c4 = c3.AsColor4(0.55f);

                        Assert::AreEqual({ 0.25f, 0.35f, 0.45f, 0.55f }, c4);
                    }
                }
            };
        }
    }
}
