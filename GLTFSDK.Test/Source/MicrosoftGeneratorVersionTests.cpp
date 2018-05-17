// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/MicrosoftGeneratorVersion.h>

#include <functional>
#include <locale>
#include <memory>
#include <regex>
#include <string>

using namespace glTF::UnitTest;

namespace Microsoft
{
    namespace  glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(MicrosoftGeneratorVersionTests)
            {
                BEGIN_TEST_CLASS_ATTRIBUTE()
                    TEST_CLASS_ATTRIBUTE(L"Priority", L"1")
                    TEST_CLASS_ATTRIBUTE(L"Category", L"Unit-Integration")
                    END_TEST_CLASS_ATTRIBUTE()

                struct MicrosoftGeneratorVersionTestStruct
                {
                    std::string version;
                    std::string testValue;
                    bool isMicrosoftGeneratorValue;
                    std::function<bool(const MicrosoftGeneratorVersion& srcVersion, const MicrosoftGeneratorVersion& currentVersion)> testFunction;

                    MicrosoftGeneratorVersionTestStruct(const std::string& inVersion, const std::string& inTestValue,
                        bool inIsMicrosoftGeneratorValue,
                        const std::function<bool(const MicrosoftGeneratorVersion& srcVersion, const MicrosoftGeneratorVersion& currentVersion)>& testFunc
                    )
                    {
                        version = inVersion;
                        testValue = inTestValue;
                        isMicrosoftGeneratorValue = inIsMicrosoftGeneratorValue;
                        testFunction = testFunc;
                    }
                };

                GLTFSDK_TEST_METHOD(MicrosoftGeneratorVersionTests, MicrosoftGeneratorVersionTests_ParseTest)
                {
                    const std::string threeValues("1.1.1");
                    const std::string fourValues("1.1.1.1");
                    const std::string threeValuesPre("1.1.1-b23");
                    const std::string fourValuesPre("1.1.1.1-b23");

                    std::vector<MicrosoftGeneratorVersionTestStruct> testVersions =
                    {
                        //IsMicrosoftGenerator == false
                        MicrosoftGeneratorVersionTestStruct("1.1.2-b2",                                   threeValues,    false, std::greater<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Some Other Exporter 1.1.1.1-b39-g0ef2ed0",   fourValues,     false, std::less<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("1.0.1-b2",                                   threeValues,    false, std::less<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Some Other Exporter 1.1.1.0-b39-g0ef2ed0",   fourValues,     false, std::less<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Some Other Exporter 1.1.1.0-b39-g0ef2ed0",   fourValues,     false, std::less_equal<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Some Other Exporter 1.1.1.1-b39-g0ef2ed0",   fourValues,     false, std::less<MicrosoftGeneratorVersion>()),

                        //IsMicrosoftGenerator == true
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.1.1",                      threeValues,    true, std::equal_to<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter     1.1.1    ",              threeValues,    true, std::equal_to<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.1.1.1",                    fourValues,     true, std::equal_to<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter   1.1.1.1   ",               fourValues,     true, std::equal_to<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter   1.1.1.2   ",               fourValues,     true, std::greater<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter   1.1.2.1   ",               fourValues,     true, std::greater<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter   1.2.1.1   ",               fourValues,     true, std::greater_equal<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter   1.2.1.1   ",               fourValues,     true, std::not_equal_to<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 2.1.1",                      threeValues,    true, std::not_equal_to<MicrosoftGeneratorVersion>()),

                        //less than 
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.1.0",                      threeValues,    true, std::less<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.0.1",                      threeValues,    true, std::less<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 0.1.1",                      threeValues,    true, std::less<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.0.1-b2",                   threeValues,    true, std::less<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter   1.0.1-b2  ",               threeValues,    true, std::less<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.1.0.1-b39-g0ef2ed0",       fourValues,     true, std::less<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter    1.1.0.1-b39-g0ef2ed0   ", fourValues,     true, std::less<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.0.0.1",                    fourValues,     true, std::less<MicrosoftGeneratorVersion>()),

                        //pre-release
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.1.2-b2",                   threeValues,    true, std::greater<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.1.1-b2",                   threeValues,    true, std::less<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.1.1-b2",                   threeValues,    true, std::not_equal_to<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.1.2-b2",                   threeValuesPre, true, std::greater<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.1.2-b2",                   threeValuesPre, true, std::greater_equal<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.1.1-b2",                   threeValuesPre, true, std::less<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.1.1",                      threeValuesPre, true, std::greater<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.1.1",                      threeValuesPre, true, std::greater_equal<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.1.0",                      threeValuesPre, true, std::less<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.1.0",                      threeValuesPre, true, std::less_equal<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.1.1-b2",                   threeValuesPre, true, std::less_equal<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.1.1-b23",                  threeValuesPre, true, std::equal_to<MicrosoftGeneratorVersion>()),
                        
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.1.1.2-b2",                 fourValues,     true, std::greater<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.1.1.1-b2",                 fourValues,     true, std::less<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.1.1.1-b2",                 fourValues,     true, std::not_equal_to<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.1.1.2-b2",                 fourValues,     true, std::greater<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.1.1.2-b2",                 fourValuesPre,  true, std::greater_equal<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.1.1.1-b2",                 fourValuesPre,  true, std::less<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.1.1.1-b2",                 fourValuesPre,  true, std::less_equal<MicrosoftGeneratorVersion>()),
                        MicrosoftGeneratorVersionTestStruct("Microsoft GLTF Exporter 1.1.1.1-b23",                fourValuesPre,  true, std::equal_to<MicrosoftGeneratorVersion>()),
                    };

                    std::not_equal_to<MicrosoftGeneratorVersion> notEqual;
                    MicrosoftGeneratorVersion zeroVersion("0.0.0.0");
                    for (auto s : testVersions)
                    {
                        MicrosoftGeneratorVersion testVersion(s.version);
                        MicrosoftGeneratorVersion testValue(s.testValue);
                        Assert::IsTrue(notEqual(testVersion, zeroVersion)); //this should not happen. If testVersion is 0.0.0.0 then regex parse error
                        Assert::IsTrue(testVersion.IsMicrosoftGenerator() == s.isMicrosoftGeneratorValue);
                        Assert::IsTrue(s.testFunction(testVersion, testValue));
                    }

                    //test empty version strings
                    auto nostring = MicrosoftGeneratorVersion("");
                    auto noversion = MicrosoftGeneratorVersion("Some Other Exporter");
                    Assert::IsTrue(nostring == zeroVersion); 
                    Assert::IsTrue(noversion == zeroVersion);
                }
            };
        }
    }
}