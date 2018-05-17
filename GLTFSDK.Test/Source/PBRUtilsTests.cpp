// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/PBRUtils.h>

using namespace glTF::UnitTest;

namespace
{
    float Random(float a = 0.0f, float b = 1.0f)
    {
        return a + (b - a) * (static_cast<float>(rand()) / RAND_MAX);
    }

    bool FuzzyEqual(float a, float b, float epsilon = std::numeric_limits<float>::epsilon())
    {
        return fabs(a - b) < epsilon;
    }

    bool FuzzyEqual(Microsoft::glTF::Color3 a, Microsoft::glTF::Color3 b, float epsilon = std::numeric_limits<float>::epsilon())
    {
        return
            FuzzyEqual(a.r, b.r, epsilon) &&
            FuzzyEqual(a.g, b.g, epsilon) &&
            FuzzyEqual(a.b, b.b, epsilon);
    }
}

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(PBRUtilsTests)
            {
                GLTFSDK_TEST_METHOD(PBRUtilsTests, MRToSG_Dieletric)
                {
                    MetallicRoughnessValue mr;
                    mr.base = Color3(0.5f, 0.1f, 0.2f);
                    mr.opacity = 1.0f;
                    mr.metallic = 0.0f;
                    mr.roughness = 0.5f;

                    SpecularGlossinessValue sg = MRToSG(mr);
                    Assert::IsTrue(FuzzyEqual(sg.diffuse, mr.base));
                    Assert::IsTrue(FuzzyEqual(sg.opacity, mr.opacity));
                    Assert::IsTrue(FuzzyEqual(sg.specular, Detail::DIELECTRIC_SPECULAR<Color3>));
                    Assert::IsTrue(FuzzyEqual(sg.glossiness, 1.0f - mr.roughness));
                }

                GLTFSDK_TEST_METHOD(PBRUtilsTests, MRToSG_Metallic)
                {
                    MetallicRoughnessValue mr;
                    mr.base = Color3(0.5f, 0.1f, 0.2f);
                    mr.opacity = 1.0f;
                    mr.metallic = 1.0f;
                    mr.roughness = 0.5f;

                    SpecularGlossinessValue sg = MRToSG(mr);
                    Assert::IsTrue(FuzzyEqual(sg.diffuse, Detail::BLACK<Color3>));
                    Assert::IsTrue(FuzzyEqual(sg.opacity, mr.opacity));
                    Assert::IsTrue(FuzzyEqual(sg.specular, mr.base));
                    Assert::IsTrue(FuzzyEqual(sg.glossiness, 1.0f - mr.roughness));
                }

                GLTFSDK_TEST_METHOD(PBRUtilsTests, SGToMR_Dielectric)
                {
                    SpecularGlossinessValue sg;
                    sg.diffuse = Color3(0.5f, 0.1f, 0.2f);
                    sg.opacity = 1.0f;
                    sg.specular = Detail::DIELECTRIC_SPECULAR<Color3>;
                    sg.glossiness = 0.5;

                    MetallicRoughnessValue mr = SGToMR(sg);
                    Assert::IsTrue(FuzzyEqual(mr.base, sg.diffuse));
                    Assert::IsTrue(FuzzyEqual(mr.opacity, 1.0f));
                    Assert::IsTrue(FuzzyEqual(mr.metallic, 0.0f));
                    Assert::IsTrue(FuzzyEqual(mr.roughness, 1.0f - sg.glossiness));
                }

                GLTFSDK_TEST_METHOD(PBRUtilsTests, SGToMR_Metallic)
                {
                    SpecularGlossinessValue sg;
                    sg.diffuse = Detail::BLACK<Color3>;
                    sg.opacity = 1.0f;
                    sg.specular = Color3(0.5f, 0.1f, 0.2f);
                    sg.glossiness = 0.5;

                    MetallicRoughnessValue mr = SGToMR(sg);
                    Assert::IsTrue(FuzzyEqual(mr.base, sg.specular));
                    Assert::IsTrue(FuzzyEqual(mr.opacity, 1.0f));
                    Assert::IsTrue(FuzzyEqual(mr.metallic, 1.0f));
                    Assert::IsTrue(FuzzyEqual(mr.roughness, 1.0f - sg.glossiness));
                }

                GLTFSDK_TEST_METHOD(PBRUtilsTests, RoundTrip)
                {
                    // Initialize from a fixed seed.
                    srand(1234);

                    for (int i = 0; i < 100; i++)
                    {
                        // Don't test colors lower than 0.04 to avoid larger deltas due to numerical issues.
                        MetallicRoughnessValue mrBefore;
                        mrBefore.base = Color3(Random(0.04f), Random(0.04f), Random(0.04f));
                        mrBefore.opacity = Random();
                        mrBefore.metallic = Random();
                        mrBefore.roughness = Random();

                        MetallicRoughnessValue mrAfter = SGToMR(MRToSG(mrBefore));

                        // 0.04 is derived from the max delta after 10000 iterations.
                        const float epsilon = 0.04f;
                        Assert::IsTrue(FuzzyEqual(mrBefore.base, mrAfter.base, epsilon));
                        Assert::IsTrue(FuzzyEqual(mrBefore.metallic, mrAfter.metallic, epsilon));

                        // Opacity and roughness should be exact.
                        Assert::IsTrue(FuzzyEqual(mrBefore.opacity, mrAfter.opacity));
                        Assert::IsTrue(FuzzyEqual(mrBefore.roughness, mrAfter.roughness));
                    }
                }
            };
        }
    }
}
