// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/ExtrasDocument.h>
#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/Document.h>

#include <cstdint>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using namespace glTF::UnitTest;

namespace
{
    static const char test_json_extras_object[] = R"(
{
    "asset":
    {
        "version": "2.0"
    },
    "extras":
    {
        "propertyA": 1,
        "propertyB": 1.23,
        "propertyC": ["test1", "test2"]
    }
}
)";

    static const char test_json_extras_value[] = R"(
{
    "asset":
    {
        "version": "2.0"
    },
    "extras": "testValue"
}
)";

    static const char test_json_extras_none[] = R"(
{
    "asset":
    {
        "version": "2.0"
    }
}
)";

    static const char test_json_extras_set_member[] = R"({"prop1":1,"prop2":"value","prop3":true})";

    static const char test_json_extras_set_pointer[] = R"({"array":[true],"prop":{"propChild":1.23}})";
}

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(GLTFExtrasDocumentTests)
            {
                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentTest)
                {
                    Document gltfDoc = Deserialize(test_json_extras_object);

                    ExtrasDocument extrasDoc(gltfDoc.extras.c_str());

                    const auto propValueA = extrasDoc.GetMemberValueOrDefault<uint32_t>("propertyA");
                    const auto propValueB = extrasDoc.GetMemberValueOrDefault<float>("propertyB");
                    const auto propValueC = extrasDoc.GetMemberValueOrDefault<float>("propertyMissing", 888.8f);

                    Assert::AreEqual(1U, propValueA);
                    Assert::AreEqual(1.23f, propValueB);
                    Assert::AreEqual(888.8f, propValueC);
                }

                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentPointer)
                {
                    Document gltfDoc = Deserialize(test_json_extras_object);

                    ExtrasDocument extrasDoc(gltfDoc.extras.c_str());

                    const auto propValueA = extrasDoc.GetPointerValueOrDefault<std::string>("/propertyC/0");
                    const auto propValueB = extrasDoc.GetPointerValueOrDefault<std::string>("/propertyC/1");
                    const auto propValueC = extrasDoc.GetPointerValueOrDefault<std::string>("/propertyMissing/1", "missing!");

                    Assert::AreEqual("test1", propValueA.c_str());
                    Assert::AreEqual("test2", propValueB.c_str());
                    Assert::AreEqual("missing!", propValueC.c_str());
                }

                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentValue)
                {
                    Document gltfDoc = Deserialize(test_json_extras_value);

                    ExtrasDocument extrasDoc(gltfDoc.extras.c_str());

                    const auto extraValue = extrasDoc.GetValueOrDefault<std::string>();
                    const auto extraMissing = extrasDoc.GetValueOrDefault<float>(444.4f);

                    Assert::AreEqual("testValue", extraValue.c_str());
                    Assert::AreEqual(444.4f, extraMissing);
                }

                // Negative regression: GetMemberValueOrDefault on a string-rooted
                // extras document (not a JSON object) must return the default rather
                // than read the string's storage as object member metadata. An extras
                // value may be any JSON type, so a member lookup must tolerate a
                // non-object root.
                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentValueGetMemberReturnsDefault)
                {
                    Document gltfDoc = Deserialize(test_json_extras_value);

                    ExtrasDocument extrasDoc(gltfDoc.extras.c_str());

                    const auto missing = extrasDoc.GetMemberValueOrDefault<float>("anyMember", 7.5f);

                    Assert::AreEqual(7.5f, missing);
                }

                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentNone)
                {
                    Document gltfDoc = Deserialize(test_json_extras_none);

                    Assert::ExpectException<GLTFException>([&gltfDoc]()
                    {
                        ExtrasDocument extrasDoc(gltfDoc.extras.c_str());
                    }, L"Expected GLTFException to be thrown for an empty extras string");
                }

                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentSetValue)
                {
                    {
                        ExtrasDocument extrasDoc;
                        extrasDoc.SetValue(1); // Integer value

                        Assert::AreEqual("1", extrasDoc.ToJson().c_str());
                    }

                    {
                        ExtrasDocument extrasDoc;
                        extrasDoc.SetValue(1U); // Unsigned value

                        Assert::AreEqual("1", extrasDoc.ToJson().c_str());
                    }

                    {
                        ExtrasDocument extrasDoc;
                        extrasDoc.SetValue(1.0f); // Float value

                        Assert::AreEqual("1.0", extrasDoc.ToJson().c_str());
                    }

                    {
                        ExtrasDocument extrasDoc;
                        extrasDoc.SetValue(false); // Boolean value

                        Assert::AreEqual("false", extrasDoc.ToJson().c_str());
                    }

                    {
                        ExtrasDocument extrasDoc;
                        extrasDoc.SetValue<const char*>("Test String"); // C-string value

                        Assert::AreEqual("\"Test String\"", extrasDoc.ToJson().c_str());
                    }

                    {
                        ExtrasDocument extrasDoc;
                        extrasDoc.SetValue(std::string("Test String")); // std::string value

                        Assert::AreEqual("\"Test String\"", extrasDoc.ToJson().c_str());
                    }
                }

                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentSetValueMultiple)
                {
                    ExtrasDocument extrasDoc;

                    extrasDoc.SetValue(1);
                    extrasDoc.SetValue(2);

                    Assert::AreEqual("2", extrasDoc.ToJson().c_str());
                }

                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentSetValueInvalidType)
                {
                    ExtrasDocument extrasDoc;

                    Assert::ExpectException<GLTFException>([&extrasDoc]()
                    {
                        extrasDoc.SetValue(1);
                        extrasDoc.SetValue(false);
                    });
                }

                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentSetMemberValue)
                {
                    ExtrasDocument extrasDoc;

                    extrasDoc.SetMemberValue("prop1", 1);
                    extrasDoc.SetMemberValue("prop2", std::string("value"));
                    extrasDoc.SetMemberValue("prop3", true);

                    Assert::AreEqual(test_json_extras_set_member, extrasDoc.ToJson().c_str());
                }

                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentSetMemberValueMultiple)
                {
                    ExtrasDocument extrasDoc;

                    extrasDoc.SetMemberValue("prop", 1);
                    extrasDoc.SetMemberValue("prop", 2);

                    Assert::AreEqual("{\"prop\":2}", extrasDoc.ToJson().c_str());
                }

                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentSetMemberValueInvalidType)
                {
                    ExtrasDocument extrasDoc;

                    Assert::ExpectException<GLTFException>([&extrasDoc]()
                    {
                        extrasDoc.SetValue(1);
                        extrasDoc.SetMemberValue("prop1", 1);
                    });
                }

                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentSetPointerValue)
                {
                    ExtrasDocument extrasDoc;

                    extrasDoc.SetPointerValue("/array/0", true);
                    extrasDoc.SetPointerValue("/prop/propChild", 1.23);

                    Assert::AreEqual(test_json_extras_set_pointer, extrasDoc.ToJson().c_str());
                }

                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentSetPointerValueMultiple)
                {
                    ExtrasDocument extrasDoc;

                    extrasDoc.SetPointerValue("/prop", 1.23);
                    extrasDoc.SetPointerValue("/prop", 4.56);

                    Assert::AreEqual("{\"prop\":4.56}", extrasDoc.ToJson().c_str());
                }

                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentSetPointerValueInvalidType)
                {
                    ExtrasDocument extrasDoc;

                    Assert::ExpectException<GLTFException>([&extrasDoc]()
                    {
                        extrasDoc.SetPointerValue("/prop", 1);
                        extrasDoc.SetPointerValue("/prop", false);
                    });
                }

                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentPublicTypeContract)
                {
                    static_assert(
                        !std::is_copy_constructible<ExtrasDocument>::value,
                        "ExtrasDocument must not be copy constructible");
                    static_assert(
                        !std::is_copy_assignable<ExtrasDocument>::value,
                        "ExtrasDocument must not be copy assignable");
                    static_assert(
                        std::is_nothrow_move_constructible<
                            ExtrasDocument>::value,
                        "ExtrasDocument move construction must be noexcept");
                    static_assert(
                        std::is_nothrow_move_assignable<ExtrasDocument>::value,
                        "ExtrasDocument move assignment must be noexcept");
                    static_assert(
                        !Detail::ExtrasTypeTraits<
                            std::vector<int>>::CanGet,
                        "Unsupported getter types must be rejected");
                    static_assert(
                        !Detail::ExtrasTypeTraits<
                            std::vector<int>>::CanSet,
                        "Unsupported setter types must be rejected");
                }

                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentStrictConstructors)
                {
                    const std::string json = R"({"first":1,"second":2})";
                    const ExtrasDocument fromString(json);
                    Assert::AreEqual(json.c_str(), fromString.ToJson().c_str());

                    Assert::ExpectException<GLTFException>([]()
                    {
                        ExtrasDocument nullString(
                            static_cast<const char*>(nullptr));
                    });
                    Assert::ExpectException<GLTFException>([]()
                    {
                        ExtrasDocument duplicate(
                            R"({"value":1,"value":2})");
                    });
                    Assert::ExpectException<GLTFException>([]()
                    {
                        std::string invalidUtf8 = R"({"value":")";
                        invalidUtf8.push_back(static_cast<char>(0xC3));
                        invalidUtf8.push_back(static_cast<char>(0x28));
                        invalidUtf8 += R"("})";
                        ExtrasDocument invalid(invalidUtf8);
                    });
                }

                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentToJsonAndHasMember)
                {
                    ExtrasDocument empty;
                    Assert::AreEqual("null", empty.ToJson().c_str());
                    Assert::IsFalse(empty.HasMember("value"));

                    ExtrasDocument object(R"({"value":1})");
                    Assert::IsTrue(object.HasMember("value"));
                    Assert::IsFalse(object.HasMember("missing"));

                    ExtrasDocument scalar(R"("value")");
                    Assert::IsFalse(scalar.HasMember("value"));

                    Assert::ExpectException<GLTFException>([&object]()
                    {
                        object.HasMember(nullptr);
                    });
                }

                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentSupportsAllScalarTypes)
                {
                    ExtrasDocument extras;
                    const std::int32_t int32Value =
                        std::numeric_limits<std::int32_t>::min();
                    const std::uint32_t uint32Value =
                        std::numeric_limits<std::uint32_t>::max();
                    const std::int64_t int64Value =
                        std::numeric_limits<std::int64_t>::min();
                    const std::uint64_t uint64Value =
                        std::numeric_limits<std::uint64_t>::max();
                    const std::size_t sizeValue =
                        std::numeric_limits<std::size_t>::max();

                    extras.SetMemberValue("boolean", true);
                    extras.SetMemberValue("int32", int32Value);
                    extras.SetMemberValue("uint32", uint32Value);
                    extras.SetMemberValue("int64", int64Value);
                    extras.SetMemberValue("uint64", uint64Value);
                    extras.SetMemberValue("size", sizeValue);
                    extras.SetMemberValue("float", 1.25F);
                    extras.SetMemberValue("double", 2.5);
                    extras.SetMemberValue("string", std::string("text"));
                    extras.SetMemberValue("cstring", "literal");

                    Assert::IsTrue(
                        extras.GetMemberValueOrDefault<bool>("boolean") ==
                        true);
                    Assert::IsTrue(
                        extras.GetMemberValueOrDefault<std::int32_t>(
                            "int32") == int32Value);
                    Assert::IsTrue(
                        extras.GetMemberValueOrDefault<std::uint32_t>(
                            "uint32") == uint32Value);
                    Assert::IsTrue(
                        extras.GetMemberValueOrDefault<std::int64_t>(
                            "int64") == int64Value);
                    Assert::IsTrue(
                        extras.GetMemberValueOrDefault<std::uint64_t>(
                            "uint64") == uint64Value);
                    Assert::IsTrue(
                        extras.GetMemberValueOrDefault<std::size_t>(
                            "size") == sizeValue);
                    Assert::IsTrue(
                        extras.GetMemberValueOrDefault<float>(
                            "float") == 1.25F);
                    Assert::IsTrue(
                        extras.GetMemberValueOrDefault<double>(
                            "double") == 2.5);
                    Assert::IsTrue(
                        extras.GetMemberValueOrDefault<std::string>(
                            "string") == "text");
                    Assert::IsTrue(
                        extras.GetMemberValueOrDefault<std::string>(
                            "cstring") == "literal");
                }

                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentMovedFromStateIsReusable)
                {
                    ExtrasDocument source(R"({"value":7})");
                    ExtrasDocument moved(std::move(source));

                    Assert::AreEqual(
                        R"({"value":7})",
                        moved.ToJson().c_str());
                    Assert::AreEqual("null", source.ToJson().c_str());
                    Assert::IsFalse(source.HasMember("value"));
                    Assert::IsTrue(
                        source.GetMemberValueOrDefault<int>(
                            "value", 11) == 11);

                    source.SetMemberValue("newValue", 3);
                    Assert::AreEqual(
                        R"({"newValue":3})",
                        source.ToJson().c_str());

                    ExtrasDocument assigned;
                    assigned.SetValue(false);
                    assigned = std::move(moved);
                    Assert::AreEqual(
                        R"({"value":7})",
                        assigned.ToJson().c_str());
                    Assert::AreEqual("null", moved.ToJson().c_str());
                }

                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentPreservesNumericCategoryAndOrder)
                {
                    ExtrasDocument scalar;
                    scalar.SetValue(1);
                    scalar.SetValue(2.5);
                    Assert::AreEqual("2.5", scalar.ToJson().c_str());
                    Assert::ExpectException<GLTFException>([&scalar]()
                    {
                        scalar.SetValue(false);
                    });

                    ExtrasDocument object;
                    object.SetMemberValue("first", 1);
                    object.SetMemberValue("second", 2);
                    object.SetMemberValue("first", 3.5);
                    Assert::AreEqual(
                        R"({"first":3.5,"second":2})",
                        object.ToJson().c_str());
                }

                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentPublicPointerMatrix)
                {
                    ExtrasDocument extras;
                    extras.SetPointerValue("/array/2", true);
                    extras.SetPointerValue("/a~1b/~0key", "value");

                    Assert::AreEqual(
                        R"({"array":[null,null,true],"a/b":{"~key":"value"}})",
                        extras.ToJson().c_str());
                    Assert::IsTrue(
                        extras.GetPointerValueOrDefault<bool>(
                            "/array/2") == true);
                    Assert::IsTrue(
                        extras.GetPointerValueOrDefault<std::string>(
                            "/a~1b/~0key") == "value");
                    Assert::IsTrue(
                        extras.GetPointerValueOrDefault<int>(
                            "/array/9", 12) == 12);
                    Assert::ExpectException<GLTFException>([&extras]()
                    {
                        extras.GetPointerValueOrDefault<int>("/array/01");
                    });
                }

                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentRejectsNullSelectorsAndNonFinite)
                {
                    ExtrasDocument extras;
                    Assert::ExpectException<GLTFException>([&extras]()
                    {
                        extras.GetMemberValueOrDefault<int>(nullptr);
                    });
                    Assert::ExpectException<GLTFException>([&extras]()
                    {
                        extras.SetMemberValue(nullptr, 1);
                    });
                    Assert::ExpectException<GLTFException>([&extras]()
                    {
                        extras.GetPointerValueOrDefault<int>(nullptr);
                    });
                    Assert::ExpectException<GLTFException>([&extras]()
                    {
                        extras.SetPointerValue(nullptr, 1);
                    });
                    Assert::ExpectException<GLTFException>([&extras]()
                    {
                        extras.SetValue(
                            std::numeric_limits<double>::infinity());
                    });
                    Assert::ExpectException<GLTFException>([&extras]()
                    {
                        extras.SetValue(static_cast<const char*>(nullptr));
                    });
                }

                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentGetterDefaultsAreStrict)
                {
                    ExtrasDocument extras(R"({
                        "integer": 1,
                        "floating": 1.0,
                        "negative": -1,
                        "string": "value"
                    })");

                    Assert::IsTrue(
                        extras.GetMemberValueOrDefault<int>(
                            "floating", 9) == 9);
                    Assert::IsTrue(
                        extras.GetMemberValueOrDefault<unsigned int>(
                            "negative", 8U) == 8U);
                    Assert::IsTrue(
                        extras.GetMemberValueOrDefault<bool>(
                            "integer", true) == true);
                    Assert::IsTrue(
                        extras.GetMemberValueOrDefault<std::string>(
                            "missing", "default") == "default");
                }
            };
        }
    }
}
