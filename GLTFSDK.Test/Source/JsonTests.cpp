// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include "Internal/Json.h"
#include "TestUtils.h"

#include <GLTFSDK/Exceptions.h>

#include <initializer_list>
#include <sstream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

using namespace glTF::UnitTest;

namespace
{
    using namespace Microsoft::glTF;

    void ExpectStrictFailureForStringAndStream(const std::string& json)
    {
        Assert::ExpectException<GLTFException>([&json]()
        {
            Internal::ParseJson(json);
        });

        Assert::ExpectException<GLTFException>([&json]()
        {
            std::stringstream stream(json);
            Internal::ParseJson(stream);
        });
    }

    void ExpectStrictFailureMessageForStringAndStream(
        const std::string& json,
        const std::string& expected)
    {
        Assert::ExpectException<GLTFException>([&]()
        {
            try
            {
                Internal::ParseJson(json);
            }
            catch (const GLTFException& exception)
            {
                Assert::AreEqual(expected.c_str(), exception.what());
                throw;
            }
        });

        Assert::ExpectException<GLTFException>([&]()
        {
            try
            {
                std::stringstream stream(json);
                Internal::ParseJson(stream);
            }
            catch (const GLTFException& exception)
            {
                Assert::AreEqual(expected.c_str(), exception.what());
                throw;
            }
        });
    }

    std::string NestedArray(std::size_t depth)
    {
        return std::string(depth, '[') + "0" + std::string(depth, ']');
    }

    std::string JsonStringWithBytes(
        std::initializer_list<unsigned int> bytes)
    {
        std::string json = R"({"value":")";
        for (const auto byte : bytes)
        {
            json.push_back(static_cast<char>(byte));
        }
        json += R"("})";
        return json;
    }
}

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(JsonTests)
            {
                GLTFSDK_TEST_METHOD(JsonTests, FoundationUsesApprovedVersion)
                {
                    Assert::AreEqual(
                        "3.12.0",
                        Internal::GetJsonLibraryVersion());
                }

                GLTFSDK_TEST_METHOD(JsonTests, FoundationCreatesOrderedDomKinds)
                {
                    const auto nullValue = Internal::CreateJsonNull();
                    const auto objectValue = Internal::CreateJsonObject();
                    const auto arrayValue = Internal::CreateJsonArray();

                    Assert::IsTrue(Internal::IsJsonNull(nullValue));
                    Assert::IsTrue(Internal::IsJsonObject(objectValue));
                    Assert::IsTrue(Internal::IsJsonArray(arrayValue));
                    Assert::IsFalse(Internal::IsJsonNumber(objectValue));
                    Assert::IsFalse(Internal::IsJsonString(arrayValue));
                }

                GLTFSDK_TEST_METHOD(JsonTests, FoundationUsesCompatibilityCategories)
                {
                    const auto nullValue = Internal::CreateJsonNull();
                    const auto objectValue = Internal::CreateJsonObject();
                    const auto anotherObject = Internal::CreateJsonObject();
                    const auto arrayValue = Internal::CreateJsonArray();

                    Assert::IsTrue(
                        Internal::AreJsonAssignmentCategoriesCompatible(
                            nullValue, arrayValue));
                    Assert::IsTrue(
                        Internal::AreJsonAssignmentCategoriesCompatible(
                            objectValue, anotherObject));
                    Assert::IsFalse(
                        Internal::AreJsonAssignmentCategoriesCompatible(
                            objectValue, arrayValue));
                }

                GLTFSDK_TEST_METHOD(JsonTests, StrictStringAndStreamAgree)
                {
                    const std::string json =
                        R"({"first":1,"second":[true,null,"text"]})";
                    std::stringstream stream(json);

                    const auto fromString = Internal::ParseJson(json);
                    const auto fromStream = Internal::ParseJson(stream);

                    Assert::IsTrue(fromString == fromStream);
                    Assert::IsTrue(Internal::IsJsonObject(fromString));
                }

                GLTFSDK_TEST_METHOD(JsonTests, StrictParsesRootKindsAndNumericCategories)
                {
                    Assert::IsTrue(
                        Internal::IsJsonNull(Internal::ParseJson("null")));
                    Assert::IsTrue(
                        Internal::IsJsonBoolean(Internal::ParseJson("true")));
                    Assert::IsTrue(
                        Internal::IsJsonString(Internal::ParseJson(R"("text")")));
                    Assert::IsTrue(
                        Internal::IsJsonArray(Internal::ParseJson("[]")));
                    Assert::IsTrue(
                        Internal::IsJsonObject(Internal::ParseJson("{}")));

                    const auto signedValue = Internal::ParseJson(
                        "-9223372036854775808");
                    const auto unsignedValue = Internal::ParseJson(
                        "18446744073709551615");
                    const auto floatingValue = Internal::ParseJson("1.5");

                    std::int64_t signedResult = 0;
                    std::uint64_t unsignedResult = 0U;
                    double floatingResult = 0.0;
                    Assert::IsTrue(Internal::IsJsonSignedInteger(signedValue));
                    Assert::IsTrue(
                        Internal::TryGetJsonInt64(
                            signedValue, signedResult));
                    Assert::IsTrue(
                        signedResult ==
                        std::numeric_limits<std::int64_t>::min());
                    Assert::IsTrue(
                        Internal::IsJsonUnsignedInteger(unsignedValue));
                    Assert::IsTrue(
                        Internal::TryGetJsonUInt64(
                            unsignedValue, unsignedResult));
                    Assert::IsTrue(
                        unsignedResult ==
                        std::numeric_limits<std::uint64_t>::max());
                    Assert::IsTrue(
                        Internal::IsJsonFloatingPoint(floatingValue));
                    Assert::IsTrue(
                        Internal::TryGetJsonDouble(
                            floatingValue, floatingResult));
                    Assert::IsTrue(floatingResult == 1.5);
                }

                GLTFSDK_TEST_METHOD(JsonTests, StrictPreservesParsedInsertionOrder)
                {
                    const auto value = Internal::ParseJson(
                        R"({"third":3,"first":1,"second":{"z":0,"a":1}})");

                    std::vector<std::string> rootKeys;
                    for (auto iterator = value.begin();
                         iterator != value.end();
                         ++iterator)
                    {
                        rootKeys.push_back(iterator.key());
                    }
                    Assert::IsTrue(rootKeys == std::vector<std::string>({
                        "third",
                        "first",
                        "second"
                    }));

                    const auto& nested = Internal::RequireJsonMember(
                        value, "second", "Missing nested object");
                    std::vector<std::string> nestedKeys;
                    for (auto iterator = nested.begin();
                         iterator != nested.end();
                         ++iterator)
                    {
                        nestedKeys.push_back(iterator.key());
                    }
                    Assert::IsTrue(nestedKeys == std::vector<std::string>({
                        "z",
                        "a"
                    }));
                    Assert::AreEqual(
                        R"({"third":3,"first":1,"second":{"z":0,"a":1}})",
                        Internal::WriteJson(value).c_str());
                }

                GLTFSDK_TEST_METHOD(JsonTests, StrictRejectsDuplicateMembers)
                {
                    ExpectStrictFailureMessageForStringAndStream(
                        R"({"outer":{"value":1,"value":2}})",
                        "The document contains a duplicate object member: "
                        "value");
                    ExpectStrictFailureMessageForStringAndStream(
                        R"({"outer":{"x":1,"\u0078":2}})",
                        "The document contains a duplicate object member: x");
                }

                GLTFSDK_TEST_METHOD(JsonTests, StrictRejectsMalformedGrammar)
                {
                    const std::string expected =
                        "The document is invalid due to bad JSON formatting";
                    ExpectStrictFailureMessageForStringAndStream("", expected);
                    ExpectStrictFailureMessageForStringAndStream("   ", expected);
                    ExpectStrictFailureMessageForStringAndStream("{", expected);
                    ExpectStrictFailureMessageForStringAndStream("[1,", expected);
                    ExpectStrictFailureMessageForStringAndStream(
                        R"({"value":})", expected);
                    ExpectStrictFailureForStringAndStream(
                        R"({"value":1} trailing)");
                    ExpectStrictFailureForStringAndStream(
                        R"({"value":1,})");
                    ExpectStrictFailureForStringAndStream("[1,]");
                    ExpectStrictFailureForStringAndStream(
                        R"({"value":/*comment*/1})");
                    ExpectStrictFailureForStringAndStream(
                        "{\n// comment\n\"value\":1}");
                    ExpectStrictFailureForStringAndStream("[NaN]");
                    ExpectStrictFailureForStringAndStream("[Infinity]");
                    ExpectStrictFailureForStringAndStream("[-Infinity]");
                    ExpectStrictFailureForStringAndStream("[1e9999]");
                }

                GLTFSDK_TEST_METHOD(JsonTests, StrictRejectsInvalidUtf8)
                {
                    const std::string invalidValues[] = {
                        JsonStringWithBytes({0xC3U, 0x28U}),
                        JsonStringWithBytes({0x80U}),
                        JsonStringWithBytes({0xC0U, 0xAFU}),
                        JsonStringWithBytes({0xE2U, 0x82U}),
                        JsonStringWithBytes({0xEDU, 0xA0U, 0x80U}),
                        JsonStringWithBytes({0xF4U, 0x90U, 0x80U, 0x80U})
                    };

                    for (const auto& json : invalidValues)
                    {
                        ExpectStrictFailureForStringAndStream(json);
                    }
                }

                GLTFSDK_TEST_METHOD(JsonTests, StrictAppliesBomPolicy)
                {
                    const std::string compact = R"({"value":1})";
                    const std::string bom = std::string("\xEF\xBB\xBF") + compact;
                    const std::string doubleBom =
                        std::string("\xEF\xBB\xBF\xEF\xBB\xBF") + compact;

                    Assert::ExpectException<GLTFException>([&bom]()
                    {
                        Internal::ParseJson(bom);
                    });
                    Assert::ExpectException<GLTFException>([&bom]()
                    {
                        std::stringstream stream(bom);
                        Internal::ParseJson(stream);
                    });

                    const auto expected = Internal::ParseJson(compact);
                    const auto fromString = Internal::ParseJson(bom, true);
                    std::stringstream stream(bom);
                    const auto fromStream = Internal::ParseJson(stream, true);

                    Assert::IsTrue(expected == fromString);
                    Assert::IsTrue(expected == fromStream);

                    Assert::ExpectException<GLTFException>([&doubleBom]()
                    {
                        Internal::ParseJson(doubleBom, true);
                    });
                    Assert::ExpectException<GLTFException>([&doubleBom]()
                    {
                        std::stringstream stream(doubleBom);
                        Internal::ParseJson(stream, true);
                    });
                }

                GLTFSDK_TEST_METHOD(JsonTests, StrictRejectsNonUtf8Bom)
                {
                    const std::string utf16Le =
                        std::string("\xFF\xFE") + "{}";
                    const std::string utf16Be =
                        std::string("\xFE\xFF") + "{}";
                    const std::string utf32Le =
                        std::string("\xFF\xFE\x00\x00", 4U) + "{}";
                    const std::string utf32Be =
                        std::string("\x00\x00\xFE\xFF", 4U) + "{}";

                    ExpectStrictFailureForStringAndStream(utf16Le);
                    ExpectStrictFailureForStringAndStream(utf16Be);
                    ExpectStrictFailureForStringAndStream(utf32Le);
                    ExpectStrictFailureForStringAndStream(utf32Be);

                    Assert::ExpectException<GLTFException>([&utf16Le]()
                    {
                        Internal::ParseJson(utf16Le, true);
                    });
                }

                GLTFSDK_TEST_METHOD(JsonTests, StrictEnforcesDepthLimit)
                {
                    const auto belowLimit = NestedArray(
                        Internal::MaxJsonNestingDepth - 1U);
                    const auto atLimit = NestedArray(
                        Internal::MaxJsonNestingDepth);
                    const auto overLimit = NestedArray(
                        Internal::MaxJsonNestingDepth + 1U);

                    Assert::IsTrue(
                        Internal::IsJsonArray(
                            Internal::ParseJson(belowLimit)));
                    Assert::IsTrue(
                        Internal::IsJsonArray(Internal::ParseJson(atLimit)));
                    std::stringstream atLimitStream(atLimit);
                    Assert::IsTrue(Internal::IsJsonArray(
                        Internal::ParseJson(atLimitStream)));

                    ExpectStrictFailureForStringAndStream(overLimit);
                }

                GLTFSDK_TEST_METHOD(JsonTests, StrictCleansUpAfterSaxFailures)
                {
                    for (int iteration = 0; iteration < 32; ++iteration)
                    {
                        ExpectStrictFailureForStringAndStream(
                            R"({"outer":[{"value":1},{"value":2,}]})");
                        ExpectStrictFailureForStringAndStream(
                            R"({"outer":{"duplicate":1,"duplicate":2}})");
                    }

                    const auto valid = Internal::ParseJson(
                        R"({"outer":[{"value":1},{"value":2}]})");
                    Assert::IsTrue(Internal::IsJsonObject(valid));
                }

                GLTFSDK_TEST_METHOD(JsonTests, StrictRejectsUnreadableStream)
                {
                    std::stringstream stream("{}");
                    stream.setstate(std::ios::badbit);

                    Assert::ExpectException<GLTFException>([&stream]()
                    {
                        Internal::ParseJson(stream);
                    });
                }

                GLTFSDK_TEST_METHOD(JsonTests, NumericSignedBoundaries)
                {
                    std::int32_t int32Value = 0;
                    std::int64_t int64Value = 0;

                    Assert::IsTrue(Internal::TryGetJsonInt32(
                        Internal::CreateJsonInt32(
                            std::numeric_limits<std::int32_t>::min()),
                        int32Value));
                    Assert::IsTrue(
                        int32Value == std::numeric_limits<std::int32_t>::min());
                    Assert::IsTrue(Internal::TryGetJsonInt32(
                        Internal::CreateJsonUInt32(
                            static_cast<std::uint32_t>(
                                std::numeric_limits<std::int32_t>::max())),
                        int32Value));
                    Assert::IsFalse(Internal::TryGetJsonInt32(
                        Internal::CreateJsonUInt32(
                            static_cast<std::uint32_t>(
                                std::numeric_limits<std::int32_t>::max()) + 1U),
                        int32Value));

                    Assert::IsTrue(Internal::TryGetJsonInt64(
                        Internal::CreateJsonInt64(
                            std::numeric_limits<std::int64_t>::min()),
                        int64Value));
                    Assert::IsTrue(
                        int64Value == std::numeric_limits<std::int64_t>::min());
                    Assert::IsFalse(Internal::TryGetJsonInt64(
                        Internal::CreateJsonUInt64(
                            static_cast<std::uint64_t>(
                                std::numeric_limits<std::int64_t>::max()) + 1U),
                        int64Value));
                }

                GLTFSDK_TEST_METHOD(JsonTests, NumericUnsignedBoundaries)
                {
                    std::uint32_t uint32Value = 0U;
                    std::uint64_t uint64Value = 0U;
                    std::size_t sizeValue = 0U;

                    Assert::IsTrue(Internal::TryGetJsonUInt32(
                        Internal::CreateJsonInt32(0),
                        uint32Value));
                    Assert::IsFalse(Internal::TryGetJsonUInt32(
                        Internal::CreateJsonInt32(-1),
                        uint32Value));
                    Assert::IsTrue(Internal::TryGetJsonUInt32(
                        Internal::CreateJsonUInt32(
                            std::numeric_limits<std::uint32_t>::max()),
                        uint32Value));
                    Assert::IsFalse(Internal::TryGetJsonUInt32(
                        Internal::CreateJsonUInt64(
                            static_cast<std::uint64_t>(
                                std::numeric_limits<std::uint32_t>::max()) + 1U),
                        uint32Value));

                    Assert::IsTrue(Internal::TryGetJsonUInt64(
                        Internal::CreateJsonUInt64(
                            std::numeric_limits<std::uint64_t>::max()),
                        uint64Value));
                    Assert::IsTrue(
                        uint64Value ==
                        std::numeric_limits<std::uint64_t>::max());

                    Assert::IsTrue(Internal::TryGetJsonSize(
                        Internal::CreateJsonSize(
                            std::numeric_limits<std::size_t>::max()),
                        sizeValue));
                    Assert::IsTrue(
                        sizeValue == std::numeric_limits<std::size_t>::max());
                }

                GLTFSDK_TEST_METHOD(JsonTests, NumericRejectsWrongCategories)
                {
                    std::int32_t integer = 0;
                    bool boolean = false;
                    std::string string;

                    Assert::IsFalse(Internal::TryGetJsonInt32(
                        Internal::CreateJsonDouble(1.0),
                        integer));
                    Assert::IsFalse(Internal::TryGetJsonBoolean(
                        Internal::CreateJsonInt32(1),
                        boolean));
                    Assert::IsFalse(Internal::TryGetJsonString(
                        Internal::CreateJsonBoolean(true),
                        string));
                }

                GLTFSDK_TEST_METHOD(JsonTests, NumericFloatingChecks)
                {
                    const auto floating = Internal::CreateJsonFloat(1.0F);
                    float floatValue = 0.0F;
                    double doubleValue = 0.0;

                    Assert::IsTrue(Internal::IsJsonFloatingPoint(floating));
                    Assert::IsTrue(
                        Internal::TryGetJsonFloat(floating, floatValue));
                    Assert::IsTrue(floatValue == 1.0F);
                    Assert::IsTrue(Internal::TryGetJsonDouble(
                        Internal::CreateJsonUInt64(
                            std::numeric_limits<std::uint64_t>::max()),
                        doubleValue));
                    Assert::IsFalse(Internal::TryGetJsonFloat(
                        Internal::CreateJsonDouble(
                            std::numeric_limits<double>::max()),
                        floatValue));
                    Assert::IsFalse(Internal::TryGetJsonFloat(
                        Internal::CreateJsonDouble(
                            std::numeric_limits<double>::denorm_min()),
                        floatValue));

                    const Internal::JsonValue infinity(
                        std::numeric_limits<double>::infinity());
                    Assert::IsFalse(
                        Internal::TryGetJsonDouble(infinity, doubleValue));
                    Assert::ExpectException<GLTFException>([]()
                    {
                        Internal::CreateJsonFloat(
                            std::numeric_limits<float>::infinity());
                    });
                    Assert::ExpectException<GLTFException>([]()
                    {
                        Internal::CreateJsonDouble(
                            std::numeric_limits<double>::quiet_NaN());
                    });
                }

                GLTFSDK_TEST_METHOD(JsonTests, AccessMembersAndArraysDefensively)
                {
                    auto object = Internal::CreateJsonObject();
                    Internal::SetJsonMember(
                        object, "value", Internal::CreateJsonInt32(7));

                    const auto* found =
                        Internal::FindJsonMember(object, "value");
                    std::int32_t value = 0;
                    Assert::IsTrue(found != nullptr);
                    Assert::IsTrue(
                        Internal::TryGetJsonInt32(*found, value));
                    Assert::IsTrue(value == 7);
                    Assert::IsTrue(
                        Internal::FindJsonMember(object, "missing") == nullptr);
                    Assert::ExpectException<InvalidGLTFException>([&object]()
                    {
                        Internal::RequireJsonMember(
                            object, "missing", "missing member");
                    });
                    Assert::ExpectException<InvalidGLTFException>([]()
                    {
                        const auto scalar = Internal::CreateJsonInt32(1);
                        Internal::FindJsonMember(scalar, "value");
                    });

                    auto array = Internal::CreateJsonArray();
                    Internal::AppendJsonValue(
                        array, Internal::CreateJsonString("first"));
                    Internal::AppendJsonValue(
                        array, Internal::CreateJsonString("second"));
                    Assert::IsTrue(Internal::GetJsonArraySize(array) == 2U);
                    std::string second;
                    Assert::IsTrue(Internal::TryGetJsonString(
                        Internal::GetJsonArrayElement(
                            array, 1U, "missing array element"),
                        second));
                    Assert::IsTrue(second == "second");
                    Assert::ExpectException<InvalidGLTFException>([&array]()
                    {
                        Internal::GetJsonArrayElement(
                            array, 2U, "missing array element");
                    });
                }

                GLTFSDK_TEST_METHOD(JsonTests, ConstructionPreservesMemberOrder)
                {
                    auto object = Internal::CreateJsonObject();
                    Internal::SetJsonMember(
                        object, "first", Internal::CreateJsonInt32(1));
                    Internal::SetJsonMember(
                        object, "second", Internal::CreateJsonInt32(2));
                    Internal::SetJsonMember(
                        object, "third", Internal::CreateJsonInt32(3));
                    Internal::SetJsonMember(
                        object, "second", Internal::CreateJsonInt32(22));

                    std::vector<std::string> keys;
                    for (auto iterator = object.begin();
                         iterator != object.end();
                         ++iterator)
                    {
                        keys.push_back(iterator.key());
                    }

                    Assert::IsTrue(keys == std::vector<std::string>({
                        "first",
                        "second",
                        "third"
                    }));

                    std::int32_t value = 0;
                    Assert::IsTrue(Internal::TryGetJsonInt32(
                        *Internal::FindJsonMember(object, "second"),
                        value));
                    Assert::IsTrue(value == 22);
                    Assert::ExpectException<GLTFException>([]()
                    {
                        Internal::CreateJsonString(nullptr);
                    });
                }

                GLTFSDK_TEST_METHOD(JsonTests, ConstructionMeasuresProgrammaticDepth)
                {
                    auto value = Internal::CreateJsonNull();
                    for (std::size_t level = 0U;
                         level < Internal::MaxJsonNestingDepth;
                         ++level)
                    {
                        auto array = Internal::CreateJsonArray();
                        Internal::AppendJsonValue(
                            array, std::move(value));
                        value = std::move(array);
                    }

                    Assert::IsTrue(
                        Internal::GetJsonNestingDepth(value) ==
                        Internal::MaxJsonNestingDepth);
                    Internal::ValidateJsonNestingDepth(value);

                    auto overLimit = Internal::CreateJsonArray();
                    Internal::AppendJsonValue(
                        overLimit, std::move(value));
                    Assert::ExpectException<GLTFException>([&overLimit]()
                    {
                        Internal::ValidateJsonNestingDepth(overLimit);
                    });
                }

                GLTFSDK_TEST_METHOD(JsonTests, WriterCompactAndPretty)
                {
                    auto array = Internal::CreateJsonArray();
                    Internal::AppendJsonValue(
                        array, Internal::CreateJsonBoolean(true));
                    Internal::AppendJsonValue(
                        array, Internal::CreateJsonString("text"));

                    auto object = Internal::CreateJsonObject();
                    Internal::SetJsonMember(
                        object, "first", Internal::CreateJsonInt32(1));
                    Internal::SetJsonMember(
                        object, "array", std::move(array));

                    Assert::AreEqual(
                        R"({"first":1,"array":[true,"text"]})",
                        Internal::WriteJson(object).c_str());
                    Assert::AreEqual(
                        R"({
    "first": 1,
    "array": [
        true,
        "text"
    ]
})",
                        Internal::WriteJson(object, true).c_str());
                }

                GLTFSDK_TEST_METHOD(JsonTests, WriterNumericLexemes)
                {
                    auto values = Internal::CreateJsonArray();
                    Internal::AppendJsonValue(
                        values,
                        Internal::CreateJsonInt64(
                            std::numeric_limits<std::int64_t>::min()));
                    Internal::AppendJsonValue(
                        values,
                        Internal::CreateJsonUInt64(
                            std::numeric_limits<std::uint64_t>::max()));
                    Internal::AppendJsonValue(
                        values, Internal::CreateJsonDouble(1.0));
                    Internal::AppendJsonValue(
                        values, Internal::CreateJsonDouble(-0.0));
                    Internal::AppendJsonValue(
                        values, Internal::CreateJsonDouble(1.0e-7));
                    Internal::AppendJsonValue(
                        values, Internal::CreateJsonDouble(1.0e20));

                    Assert::AreEqual(
                        "[-9223372036854775808,18446744073709551615,"
                        "1.0,-0.0,1e-7,1e+20]",
                        Internal::WriteJson(values).c_str());
                }

                GLTFSDK_TEST_METHOD(JsonTests, WriterEscapesAndPreservesUtf8)
                {
                    std::string value = "quote\"\n\\\t";
                    value += "\xC3\xA9";
                    value.push_back('\x01');

                    const auto json = Internal::CreateJsonString(value);
                    Assert::AreEqual(
                        "\"quote\\\"\\n\\\\\\t\xC3\xA9\\u0001\"",
                        Internal::WriteJson(json).c_str());
                }

                GLTFSDK_TEST_METHOD(JsonTests, WriterRejectsInvalidValues)
                {
                    std::string invalidUtf8;
                    invalidUtf8.push_back(static_cast<char>(0xC3));
                    invalidUtf8.push_back(static_cast<char>(0x28));
                    const auto invalidString =
                        Internal::CreateJsonString(invalidUtf8);
                    Assert::ExpectException<GLTFException>([&invalidString]()
                    {
                        Internal::WriteJson(invalidString);
                    });

                    const Internal::JsonValue infinity(
                        std::numeric_limits<double>::infinity());
                    Assert::ExpectException<GLTFException>([&infinity]()
                    {
                        Internal::WriteJson(infinity);
                    });

                    auto overLimit = Internal::CreateJsonNull();
                    for (std::size_t level = 0U;
                         level <= Internal::MaxJsonNestingDepth;
                         ++level)
                    {
                        auto array = Internal::CreateJsonArray();
                        Internal::AppendJsonValue(
                            array, std::move(overLimit));
                        overLimit = std::move(array);
                    }
                    Assert::ExpectException<GLTFException>([&overLimit]()
                    {
                        Internal::WriteJson(overLimit);
                    });
                }

                GLTFSDK_TEST_METHOD(JsonTests, WriterMatchesBaselineFixtures)
                {
                    const char* fixtureNames[] = {
                        "default",
                        "extension-heavy",
                        "extras-heavy",
                        "numeric"
                    };

                    for (const auto* fixtureName : fixtureNames)
                    {
                        const std::string path =
                            "Resources\\JsonMigrationBaseline\\" +
                            std::string(fixtureName);
                        const auto compact =
                            Microsoft::glTF::Test::ReadLocalJson(
                                (path + ".compact.json").c_str());
                        const auto pretty =
                            Microsoft::glTF::Test::ReadLocalJson(
                                (path + ".pretty.json").c_str());
                        const auto value = Internal::ParseJson(compact);

                        Assert::AreEqual(
                            compact.c_str(),
                            Internal::WriteJson(value).c_str());
                        Assert::AreEqual(
                            pretty.c_str(),
                            Internal::WriteJson(value, true).c_str());
                    }
                }

                GLTFSDK_TEST_METHOD(JsonTests, WriterIsRepeatable)
                {
                    const auto value = Internal::ParseJson(
                        R"({"z":1,"a":[3,2,1],"m":{"b":true,"a":false}})");
                    const auto compact = Internal::WriteJson(value);
                    const auto pretty = Internal::WriteJson(value, true);

                    for (int iteration = 0; iteration < 20; ++iteration)
                    {
                        Assert::AreEqual(
                            compact.c_str(),
                            Internal::WriteJson(value).c_str());
                        Assert::AreEqual(
                            pretty.c_str(),
                            Internal::WriteJson(value, true).c_str());
                    }
                }

                GLTFSDK_TEST_METHOD(JsonTests, PointerTokenizesEscapes)
                {
                    const auto tokens =
                        Internal::TokenizeJsonPointer("/a~1b/~0key//");

                    Assert::IsTrue(tokens == std::vector<std::string>({
                        "a/b",
                        "~key",
                        "",
                        ""
                    }));
                    Assert::IsTrue(
                        Internal::TokenizeJsonPointer("").empty());
                }

                GLTFSDK_TEST_METHOD(JsonTests, PointerLookupDoesNotCreate)
                {
                    auto value = Internal::ParseJson(
                        R"({"object":{"value":7},"array":[true,false]})");
                    const auto before = Internal::WriteJson(value);

                    const auto* found =
                        Internal::FindJsonPointer(value, "/object/value");
                    std::int32_t integer = 0;
                    Assert::IsTrue(found != nullptr);
                    Assert::IsTrue(
                        Internal::TryGetJsonInt32(*found, integer));
                    Assert::IsTrue(integer == 7);
                    Assert::IsTrue(
                        Internal::FindJsonPointer(
                            value, "/object/missing") == nullptr);
                    Assert::IsTrue(
                        Internal::FindJsonPointer(
                            value, "/array/2") == nullptr);
                    Assert::IsTrue(
                        Internal::WriteJson(value) == before);
                    Assert::IsTrue(
                        Internal::FindJsonPointer(value, "") == &value);
                }

                GLTFSDK_TEST_METHOD(JsonTests, PointerCreatesObjectsAndArrays)
                {
                    auto value = Internal::CreateJsonNull();
                    Internal::SetJsonPointer(
                        value,
                        "/prop/child",
                        Internal::CreateJsonDouble(1.25));
                    Internal::SetJsonPointer(
                        value,
                        "/array/2",
                        Internal::CreateJsonBoolean(true));
                    Internal::SetJsonPointer(
                        value,
                        "/a~1b/~0key",
                        Internal::CreateJsonString("escaped"));

                    Assert::AreEqual(
                        R"({"prop":{"child":1.25},"array":[null,null,true],"a/b":{"~key":"escaped"}})",
                        Internal::WriteJson(value).c_str());

                    auto indexedRoot = Internal::CreateJsonNull();
                    Internal::SetJsonPointer(
                        indexedRoot,
                        "/0",
                        Internal::CreateJsonString("first"));
                    Assert::AreEqual(
                        R"(["first"])",
                        Internal::WriteJson(indexedRoot).c_str());
                }

                GLTFSDK_TEST_METHOD(JsonTests, PointerUsesLiteralObjectTokens)
                {
                    auto object = Internal::CreateJsonObject();
                    Internal::SetJsonPointer(
                        object,
                        "/01",
                        Internal::CreateJsonString("object-key"));
                    Internal::SetJsonPointer(
                        object,
                        "/-",
                        Internal::CreateJsonString("dash-key"));

                    Assert::AreEqual(
                        R"({"01":"object-key","-":"dash-key"})",
                        Internal::WriteJson(object).c_str());
                }

                GLTFSDK_TEST_METHOD(JsonTests, PointerRejectsInvalidSyntaxAndIndices)
                {
                    Assert::ExpectException<GLTFException>([]()
                    {
                        Internal::TokenizeJsonPointer("missing-slash");
                    });
                    Assert::ExpectException<GLTFException>([]()
                    {
                        Internal::TokenizeJsonPointer("/bad~");
                    });
                    Assert::ExpectException<GLTFException>([]()
                    {
                        Internal::TokenizeJsonPointer("/bad~2escape");
                    });

                    const auto array = Internal::ParseJson("[1,2]");
                    const char* invalidIndices[] = {
                        "/01",
                        "/-",
                        "/+1",
                        "/-1",
                        "/184467440737095516160"
                    };
                    for (const auto* pointer : invalidIndices)
                    {
                        Assert::ExpectException<GLTFException>(
                            [&array, pointer]()
                            {
                                Internal::FindJsonPointer(array, pointer);
                            });
                    }
                }

                GLTFSDK_TEST_METHOD(JsonTests, PointerProtectsCategoriesAndTraversal)
                {
                    auto value = Internal::ParseJson(
                        R"({"flag":true,"number":1,"path":5})");

                    Internal::SetJsonPointer(
                        value,
                        "/flag",
                        Internal::CreateJsonBoolean(false));
                    Internal::SetJsonPointer(
                        value,
                        "/number",
                        Internal::CreateJsonDouble(1.5));
                    const auto beforeFailures = Internal::WriteJson(value);

                    Assert::ExpectException<GLTFException>([&value]()
                    {
                        Internal::SetJsonPointer(
                            value,
                            "/flag",
                            Internal::CreateJsonInt32(1));
                    });
                    Assert::ExpectException<GLTFException>([&value]()
                    {
                        Internal::SetJsonPointer(
                            value,
                            "/path/child",
                            Internal::CreateJsonInt32(1));
                    });
                    Assert::IsTrue(
                        Internal::WriteJson(value) == beforeFailures);
                }

                GLTFSDK_TEST_METHOD(JsonTests, PointerReplacesRootCompatibly)
                {
                    auto value = Internal::CreateJsonNull();
                    Internal::SetJsonPointer(
                        value, "", Internal::CreateJsonString("first"));
                    Internal::SetJsonPointer(
                        value, "", Internal::CreateJsonString("second"));

                    Assert::AreEqual(
                        R"("second")",
                        Internal::WriteJson(value).c_str());
                    Assert::ExpectException<GLTFException>([&value]()
                    {
                        Internal::SetJsonPointer(
                            value,
                            "",
                            Internal::CreateJsonBoolean(true));
                    });
                    Assert::AreEqual(
                        R"("second")",
                        Internal::WriteJson(value).c_str());
                }

                GLTFSDK_TEST_METHOD(JsonTests, PointerEnforcesDepthAtomically)
                {
                    std::string atLimit;
                    for (std::size_t level = 0U;
                         level < Internal::MaxJsonNestingDepth;
                         ++level)
                    {
                        atLimit += "/value";
                    }

                    auto accepted = Internal::CreateJsonNull();
                    Internal::SetJsonPointer(
                        accepted,
                        atLimit,
                        Internal::CreateJsonInt32(1));
                    Assert::IsTrue(
                        Internal::GetJsonNestingDepth(accepted) ==
                        Internal::MaxJsonNestingDepth);

                    auto rejected = Internal::CreateJsonNull();
                    Assert::ExpectException<GLTFException>(
                        [&rejected, &atLimit]()
                        {
                            Internal::SetJsonPointer(
                                rejected,
                                atLimit + "/value",
                                Internal::CreateJsonInt32(1));
                        });
                    Assert::IsTrue(Internal::IsJsonNull(rejected));
                }
            };
        }
    }
}
