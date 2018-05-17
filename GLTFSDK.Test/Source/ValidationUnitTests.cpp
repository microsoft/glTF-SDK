// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/Validation.h>

#include <cmath>

using namespace glTF::UnitTest;

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(ValidationUnitTests)
            {
                GLTFSDK_TEST_METHOD(ValidationUnitTests, TestAddition_size_t_NoOverflow)
                {
                    size_t a = std::numeric_limits<size_t>::max() - 500;
                    size_t b = 42;
                    size_t t;
                    Assert::IsTrue(Validation::SafeAddition(a, b, t) && t == (a + b));
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, TestAddition_size_t_MaxNoOverflow)
                {
                    size_t a = std::numeric_limits<size_t>::max() - 1;
                    size_t b = 1;
                    size_t t;
                    Assert::IsTrue(Validation::SafeAddition(a, b, t) && t == (a + b));
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, TestAddition_size_t_MinOverflow)
                {
                    size_t a = std::numeric_limits<size_t>::max();
                    size_t b = 1;
                    size_t t;
                    Assert::IsFalse(Validation::SafeAddition(a, b, t));
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, TestAddition_size_t_Overflow)
                {
                    size_t a = std::numeric_limits<size_t>::max();
                    size_t b = 42;
                    size_t t;
                    Assert::IsFalse(Validation::SafeAddition(a, b, t));
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, TestMultiplication_size_t_NoOverflow)
                {
                    size_t a = 42;
                    size_t b = 42;
                    size_t t;
                    Assert::IsTrue(Validation::SafeMultiplication(a, b, t) && t == a * b);
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, TestMultiplication_size_t_MaxNoOverflow)
                {
                    size_t a = std::numeric_limits<size_t>::max() >> 1;
                    size_t b = 2;
                    size_t t;
                    Assert::IsTrue(Validation::SafeMultiplication(a, b, t) && t == (a * b));
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, TestMultiplication_size_t_MinOverflow)
                {
                    size_t a = std::numeric_limits<size_t>::max() >> (sizeof(size_t) / 2);
                    size_t b = a;
                    size_t t;
                    Assert::IsFalse(Validation::SafeMultiplication(a, b, t));
                }

                GLTFSDK_TEST_METHOD(ValidationUnitTests, TestMultiplication_size_t_Overflow)
                {
                    size_t a = std::numeric_limits<size_t>::max();
                    size_t b = std::numeric_limits<size_t>::max();
                    size_t t;
                    Assert::IsFalse(Validation::SafeMultiplication(a, b, t));
                }
            };
        }
    }
}
