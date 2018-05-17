// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/Version.h>
#include <GLTFSDK/Constants.h>
#include <GLTFSDK/Exceptions.h>

using namespace glTF::UnitTest;

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(VersionTests)
            {
                GLTFSDK_TEST_METHOD(VersionTests, VersionAsString)
                {
                    auto version = Version(2U, 0U);
                    auto versionStr = version.AsString();

                    Assert::AreEqual(GLTF_VERSION_2_0, versionStr.c_str(), L"Unexpected version string");
                }

                GLTFSDK_TEST_METHOD(VersionTests, VersionAsTupleSuccess)
                {
                    Version versionDefault = Version::AsTuple(GLTF_VERSION_2_0);

                    Assert::AreEqual(2U, static_cast<unsigned int>(versionDefault.major), L"Unexpected major version number");
                    Assert::AreEqual(0U, static_cast<unsigned int>(versionDefault.minor), L"Unexpected minor version number");
                }

                GLTFSDK_TEST_METHOD(VersionTests, VersionAsTupleSuccessMultiDigit)
                {
                    Version versionDefault = Version::AsTuple("777.888");

                    Assert::AreEqual(777U, static_cast<unsigned int>(versionDefault.major), L"Unexpected major version number");
                    Assert::AreEqual(888U, static_cast<unsigned int>(versionDefault.minor), L"Unexpected minor version number");
                }

                GLTFSDK_TEST_METHOD(VersionTests, VersionAsTupleInvalid)
                {
                    Assert::ExpectException<GLTFException>([] {Version::AsTuple(""); });      // Empty string is invalid
                    Assert::ExpectException<GLTFException>([] {Version::AsTuple("0"); });     // Single number
                    Assert::ExpectException<GLTFException>([] {Version::AsTuple("."); });     // Missing major & minor version numbers
                    Assert::ExpectException<GLTFException>([] {Version::AsTuple(".0"); });    // Missing major version number
                    Assert::ExpectException<GLTFException>([] {Version::AsTuple("0."); });    // Missing minor version number
                    Assert::ExpectException<GLTFException>([] {Version::AsTuple("0.0.0"); }); // Unexpected use of major, minor and patch numbers
                    Assert::ExpectException<GLTFException>([] {Version::AsTuple("A.0"); });   // Non-numeric major version number
                    Assert::ExpectException<GLTFException>([] {Version::AsTuple("0.A"); });   // Non-numeric minor version number
                    Assert::ExpectException<GLTFException>([] {Version::AsTuple("+0.0"); });  // Unexpected prefix
                    Assert::ExpectException<GLTFException>([] {Version::AsTuple("0.0+"); });  // Unexpected postfix
                    Assert::ExpectException<GLTFException>([] {Version::AsTuple("-0.0"); });  // Unexpected prefix
                    Assert::ExpectException<GLTFException>([] {Version::AsTuple("0.0-"); });  // Unexpected postfix
                    Assert::ExpectException<GLTFException>([] {Version::AsTuple("0x0.0"); }); // Unexpected major number base prefix (hex)
                    Assert::ExpectException<GLTFException>([] {Version::AsTuple("0.0x0"); }); // Unexpected minor number base prefix (hex)
                    Assert::ExpectException<GLTFException>([] {Version::AsTuple("9876543210.0"); }); // Large major number (outside 32bit range)
                    Assert::ExpectException<GLTFException>([] {Version::AsTuple("0.9876543210"); }); // Large minor number (outside 32bit range)
                    Assert::ExpectException<GLTFException>([] {Version::AsTuple("9876543210.9876543210"); }); // Large numbers (outside 32bit range)
                    Assert::ExpectException<GLTFException>([] {Version::AsTuple(nullptr); }); // nullptr is invalid
                }

                GLTFSDK_TEST_METHOD(VersionTests, IsMinVersionRequirementSatisfiedDefault)
                {
                    Assert::IsTrue(IsMinVersionRequirementSatisfied(GLTF_VERSION_2_0));
                }

                GLTFSDK_TEST_METHOD(VersionTests, IsMinVersionRequirementSatisfiedEmptyVersion)
                {
                    Assert::IsTrue(IsMinVersionRequirementSatisfied({}));
                }

                GLTFSDK_TEST_METHOD(VersionTests, IsMinVersionRequirementSatisfiedEmptySupported)
                {
                    Assert::ExpectException<GLTFException>([] { IsMinVersionRequirementSatisfied({}, {}); }); // Exception due to empty 'supported' list has precedence over empty min version success
                }

                GLTFSDK_TEST_METHOD(VersionTests, IsMinVersionRequirementSatisfiedMultipleMinorVersions)
                {
                    // 2.1 support isn't explicitly listed but is implied by inclusion of 2.2 and 2.3
                    auto supportedVersions = {
                        Version(2U, 0U),
                        Version(2U, 2U),
                        Version(2U, 3U)
                    };

                    // Supported
                    Assert::IsTrue(IsMinVersionRequirementSatisfied("2.0", supportedVersions));
                    Assert::IsTrue(IsMinVersionRequirementSatisfied("2.1", supportedVersions));
                    Assert::IsTrue(IsMinVersionRequirementSatisfied("2.2", supportedVersions));
                    Assert::IsTrue(IsMinVersionRequirementSatisfied("2.3", supportedVersions));

                    // Not supported
                    Assert::IsFalse(IsMinVersionRequirementSatisfied("2.4", supportedVersions));
                }

                GLTFSDK_TEST_METHOD(VersionTests, IsMinVersionRequirementSatisfiedMajorMultipleMajorVersions)
                {
                    // 1.x -> no support
                    // 2.x -> supports 2.0, 2.1 and 2.2
                    // 3.x -> supports 3.0 and 3.1
                    // 4.x -> supports 4.0
                    auto supportedVersions = {
                        Version(2U, 2U),
                        Version(3U, 1U),
                        Version(4U, 0U)
                    };

                    // 1.0 -> not supported
                    Assert::IsFalse(IsMinVersionRequirementSatisfied("1.0", supportedVersions));

                    // 2.0 -> supported
                    // 2.1 -> supported
                    // 2.2 -> supported
                    // 2.3 -> not supported
                    Assert::IsTrue( IsMinVersionRequirementSatisfied("2.0", supportedVersions));
                    Assert::IsTrue( IsMinVersionRequirementSatisfied("2.1", supportedVersions));
                    Assert::IsTrue( IsMinVersionRequirementSatisfied("2.2", supportedVersions));
                    Assert::IsFalse(IsMinVersionRequirementSatisfied("2.3", supportedVersions));

                    // 3.0 -> supported
                    // 3.1 -> supported
                    // 3.2 -> not supported
                    Assert::IsTrue( IsMinVersionRequirementSatisfied("3.0", supportedVersions));
                    Assert::IsTrue( IsMinVersionRequirementSatisfied("3.1", supportedVersions));
                    Assert::IsFalse(IsMinVersionRequirementSatisfied("3.2", supportedVersions));

                    // 4.0 -> supported
                    // 4.1 -> not supported
                    Assert::IsTrue( IsMinVersionRequirementSatisfied("4.0", supportedVersions));
                    Assert::IsFalse(IsMinVersionRequirementSatisfied("4.1", supportedVersions));

                    // 5.0 -> not supported
                    Assert::IsFalse(IsMinVersionRequirementSatisfied("5.0", supportedVersions));
                }
            };
        }
    }
}
