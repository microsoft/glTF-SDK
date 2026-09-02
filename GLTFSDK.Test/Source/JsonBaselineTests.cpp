// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/ExtensionsKHR.h>
#include <GLTFSDK/Serialize.h>

#include "TestUtils.h"

#include <string>

using namespace glTF::UnitTest;

namespace
{
    using namespace Microsoft::glTF;

    std::string ReadFixture(const std::string& name, const std::string& suffix)
    {
        return Test::ReadLocalJson(
            ("Resources\\JsonMigrationBaseline\\" + name + "." + suffix + ".json").c_str());
    }

    void AssertCoreFixture(const std::string& name)
    {
        const auto expectedCompact = ReadFixture(name, "compact");
        const auto expectedPretty = ReadFixture(name, "pretty");
        const auto document = Deserialize(
            expectedCompact,
            DeserializeFlags::None,
            SchemaFlags::DisableSchemaRoot);

        const auto actualCompact = Serialize(document);
        const auto actualPretty = Serialize(document, SerializeFlags::Pretty);

        Assert::AreEqual(expectedCompact.c_str(), actualCompact.c_str());
        Assert::AreEqual(expectedPretty.c_str(), actualPretty.c_str());

        const auto roundTrip = Deserialize(
            actualCompact,
            DeserializeFlags::None,
            SchemaFlags::DisableSchemaRoot);
        Assert::IsTrue(document == roundTrip);
    }

    void AssertExtensionFixture(const std::string& name)
    {
        const auto expectedCompact = ReadFixture(name, "compact");
        const auto expectedPretty = ReadFixture(name, "pretty");
        const auto deserializer = KHR::GetKHRExtensionDeserializer();
        const auto serializer = KHR::GetKHRExtensionSerializer();
        const auto document = Deserialize(
            expectedCompact,
            deserializer,
            DeserializeFlags::None,
            SchemaFlags::DisableSchemaRoot);

        const auto actualCompact = Serialize(document, serializer);
        const auto actualPretty = Serialize(document, serializer, SerializeFlags::Pretty);

        Assert::AreEqual(expectedCompact.c_str(), actualCompact.c_str());
        Assert::AreEqual(expectedPretty.c_str(), actualPretty.c_str());

        const auto roundTrip = Deserialize(
            actualCompact,
            deserializer,
            DeserializeFlags::None,
            SchemaFlags::DisableSchemaRoot);
        Assert::IsTrue(document == roundTrip);
    }
}

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(JsonBaselineTests)
            {
                GLTFSDK_TEST_METHOD(JsonBaselineTests, DefaultDocument)
                {
                    AssertCoreFixture("default");
                }

                GLTFSDK_TEST_METHOD(JsonBaselineTests, ExtrasHeavyDocument)
                {
                    AssertCoreFixture("extras-heavy");
                }

                GLTFSDK_TEST_METHOD(JsonBaselineTests, NumericDocument)
                {
                    AssertCoreFixture("numeric");
                }

                GLTFSDK_TEST_METHOD(JsonBaselineTests, ExtensionHeavyDocument)
                {
                    AssertExtensionFixture("extension-heavy");
                }
            };
        }
    }
}
