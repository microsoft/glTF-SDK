// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/ExtrasDocument.h>
#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/Document.h>
#include <GLTFSDK/Serialize.h>

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

                        Assert::AreEqual("1", Serialize(extrasDoc.GetDocument()).c_str());
                    }

                    {
                        ExtrasDocument extrasDoc;
                        extrasDoc.SetValue(1U); // Unsigned value

                        Assert::AreEqual("1", Serialize(extrasDoc.GetDocument()).c_str());
                    }

                    {
                        ExtrasDocument extrasDoc;
                        extrasDoc.SetValue(1.0f); // Float value

                        Assert::AreEqual("1.0", Serialize(extrasDoc.GetDocument()).c_str());
                    }

                    {
                        ExtrasDocument extrasDoc;
                        extrasDoc.SetValue(false); // Boolean value

                        Assert::AreEqual("false", Serialize(extrasDoc.GetDocument()).c_str());
                    }

                    {
                        ExtrasDocument extrasDoc;
                        extrasDoc.SetValue<const char*>("Test String"); // C-string value

                        Assert::AreEqual("\"Test String\"", Serialize(extrasDoc.GetDocument()).c_str());
                    }

                    {
                        ExtrasDocument extrasDoc;
                        extrasDoc.SetValue(std::string("Test String")); // std::string value

                        Assert::AreEqual("\"Test String\"", Serialize(extrasDoc.GetDocument()).c_str());
                    }
                }

                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentSetValueMultiple)
                {
                    ExtrasDocument extrasDoc;

                    extrasDoc.SetValue(1);
                    extrasDoc.SetValue(2);

                    Assert::AreEqual("2", Serialize(extrasDoc.GetDocument()).c_str());
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

                    Assert::AreEqual(test_json_extras_set_member, Serialize(extrasDoc.GetDocument()).c_str());
                }

                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentSetMemberValueMultiple)
                {
                    ExtrasDocument extrasDoc;

                    extrasDoc.SetMemberValue("prop", 1);
                    extrasDoc.SetMemberValue("prop", 2);

                    Assert::AreEqual("{\"prop\":2}", Serialize(extrasDoc.GetDocument()).c_str());
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

                    Assert::AreEqual(test_json_extras_set_pointer, Serialize(extrasDoc.GetDocument()).c_str());
                }

                GLTFSDK_TEST_METHOD(GLTFExtrasDocumentTests, ExtrasDocumentSetPointerValueMultiple)
                {
                    ExtrasDocument extrasDoc;

                    extrasDoc.SetPointerValue("/prop", 1.23);
                    extrasDoc.SetPointerValue("/prop", 4.56);

                    Assert::AreEqual("{\"prop\":4.56}", Serialize(extrasDoc.GetDocument()).c_str());
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
            };
        }
    }
}
