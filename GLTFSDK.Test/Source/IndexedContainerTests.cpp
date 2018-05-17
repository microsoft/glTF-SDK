// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/Exceptions.h>
#include <GLTFSDK/IndexedContainer.h>

using namespace glTF::UnitTest;

namespace
{
    using namespace Microsoft::glTF;

    struct Uint8WithId
    {
        std::string id;
        uint8_t value = 0;

        bool operator==(const Uint8WithId& rhs) const
        {
            if (id == rhs.id && value == rhs.value)
            {
                return true;
            }
            return false;
        }
    };

    IndexedContainer<Uint8WithId> GetSampleContainer()
    {
        IndexedContainer<Uint8WithId> container;
        container.Append({ "foo0", 0 });
        container.Append({ "foo2", 2 });
        container.Append({ "foo4", 4 });
        container.Append({ "foo6", 6 });
        container.Append({ "foo8", 8 });
        container.Append({ "foo10", 10 });
        return container;
    }
}

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(IndexedContainerTests)
            {
                GLTFSDK_TEST_METHOD(IndexedContainerTests, IndexedContainer_Test_Operator_At_SizeT)
                {
                    auto container = GetSampleContainer();

                    Assert::IsTrue(container[2].value == 4);

                    Assert::ExpectException<GLTFException>([&container]()
                    {
                        container[10];
                    });
                }

                GLTFSDK_TEST_METHOD(IndexedContainerTests, IndexedContainer_Test_Operator_At_String)
                {
                    auto container = GetSampleContainer();

                    Assert::IsTrue(container["foo4"].value == 4);

                    Assert::ExpectException<GLTFException>([&container]()
                    {
                        container["foo100"];
                    });
                }

                GLTFSDK_TEST_METHOD(IndexedContainerTests, IndexedContainer_Test_Operator_Equals)
                {
                    IndexedContainer<Uint8WithId> container;
                    container.Append({ "foo0", 0 });
                    container.Append({ "foo2", 2 });
                    container.Append({ "foo4", 4 });
                    container.Append({ "foo6", 6 });
                    container.Append({ "foo8", 8 });
                    container.Append({ "foo10", 10 });
                    
                    Assert::IsTrue(GetSampleContainer() == container);
                }

                GLTFSDK_TEST_METHOD(IndexedContainerTests, IndexedContainer_Test_Operator_NotEquals)
                {
                    IndexedContainer<Uint8WithId> container;
                    container.Append({ "foo0", 0 });
                    container.Append({ "foo2", 2 });
                    container.Append({ "foo4", 4 });
                    container.Append({ "foo6", 6 });
                    container.Append({ "foo8", 8 });

                    Assert::IsTrue(GetSampleContainer() != container);
                }

                GLTFSDK_TEST_METHOD(IndexedContainerTests, IndexedContainer_Test_Append)
                {
                    auto container = GetSampleContainer();

                    Assert::ExpectException<GLTFException>([&container]()
                    {
                        container["foo100"];
                    });

                    Uint8WithId bar{ "bar", 99 };

                    container.Append(bar);
                    container.Append({ "foo100", 100 });

                    Assert::IsTrue(container["bar"].value == 99);
                    Assert::IsTrue(container["foo100"].value == 100);

                    Assert::ExpectException<GLTFException>([&container, &bar]()
                    {
                        container.Append(bar);
                    });
                }

                GLTFSDK_TEST_METHOD(IndexedContainerTests, IndexedContainer_Test_Clear)
                {
                    auto container = GetSampleContainer();

                    Assert::IsTrue(container.Size() > 0);

                    container.Clear();

                    Assert::IsTrue(container.Size() == 0);
                }

                GLTFSDK_TEST_METHOD(IndexedContainerTests, IndexedContainer_Test_Elements)
                {
                    auto container = GetSampleContainer();

                    auto elements = container.Elements();

                    Assert::IsTrue(elements[0].value == 0);
                    Assert::IsTrue(elements[1].value == 2);
                    Assert::IsTrue(elements[2].value == 4);
                    Assert::IsTrue(elements[3].value == 6);
                    Assert::IsTrue(elements[4].value == 8);
                    Assert::IsTrue(elements[5].value == 10);
                }

                GLTFSDK_TEST_METHOD(IndexedContainerTests, IndexedContainer_Test_Get_SizeT)
                {
                    auto container = GetSampleContainer();

                    Assert::IsTrue(container.Get(2).value == 4);

                    Assert::ExpectException<GLTFException>([&container]()
                    {
                        container.Get(10);
                    });
                }

                GLTFSDK_TEST_METHOD(IndexedContainerTests, IndexedContainer_Test_Get_String)
                {
                    auto container = GetSampleContainer();

                    Assert::IsTrue(container.Get("foo4").value == 4);

                    Assert::ExpectException<GLTFException>([&container]()
                    {
                        container.Get("foo100");
                    });
                }

                GLTFSDK_TEST_METHOD(IndexedContainerTests, IndexedContainer_Test_GetIndex)
                {
                    auto container = GetSampleContainer();

                    Assert::IsTrue(container.GetIndex("foo4") == 2);

                    Assert::ExpectException<GLTFException>([&container]()
                    {
                        container.GetIndex("foo100");
                    });
                }

                GLTFSDK_TEST_METHOD(IndexedContainerTests, IndexedContainer_Test_Has)
                {
                    auto container = GetSampleContainer();

                    Assert::IsTrue(container.Has("foo4"));

                    Assert::IsTrue(container.Has("foo100") == false);
                }

                GLTFSDK_TEST_METHOD(IndexedContainerTests, IndexedContainer_Test_Remove)
                {
                    auto container = GetSampleContainer();

                    Assert::IsTrue(container[0].value == 0);
                    Assert::IsTrue(container[1].value == 2);
                    Assert::IsTrue(container[2].value == 4);
                    Assert::IsTrue(container[3].value == 6);
                    Assert::IsTrue(container[4].value == 8);
                    Assert::IsTrue(container[5].value == 10);

                    container.Remove("foo4");
                    
                    Assert::IsTrue(container[0].value == 0);
                    Assert::IsTrue(container[1].value == 2);
                    Assert::IsTrue(container[2].value == 6);
                    Assert::IsTrue(container[3].value == 8);
                    Assert::IsTrue(container[4].value == 10);

                    Assert::ExpectException<GLTFException>([&container]()
                    {
                        container.GetIndex("foo100");
                    });

                    Assert::IsTrue(container[0].value == 0);
                    Assert::IsTrue(container[1].value == 2);
                    Assert::IsTrue(container[2].value == 6);
                    Assert::IsTrue(container[3].value == 8);
                    Assert::IsTrue(container[4].value == 10);
                }

                GLTFSDK_TEST_METHOD(IndexedContainerTests, IndexedContainer_Test_Replace)
                {
                    auto container = GetSampleContainer();

                    Assert::IsTrue(container[2].value == 4);

                    container.Replace({ "foo4", 40 });

                    Assert::IsTrue(container[2].value == 40);

                    Uint8WithId foo6{ "foo6", 60 };
                    container.Replace(foo6);

                    Assert::IsTrue(container[3].value == 60);
                }

                GLTFSDK_TEST_METHOD(IndexedContainerTests, IndexedContainer_Test_Replace_Rvalue_Reference)
                {
                    auto container = GetSampleContainer();
                    Uint8WithId foo4{ "foo4", 40 };
                    Uint8WithId foo6{ "foo6", 60 };

                    Assert::IsTrue(container[2].value == 4);

                    container.Replace(std::move(foo4)); // Move
                    Assert::IsTrue(foo4.id.empty());
                    Assert::IsTrue(container[2].value == 40);

                    container.Replace(foo6); // Copy
                    Assert::IsTrue(foo6.id == "foo6");
                    Assert::IsTrue(container[3].value == 60);
                }

                GLTFSDK_TEST_METHOD(IndexedContainerTests, IndexedContainer_Test_Reserve)
                {
                    auto container = GetSampleContainer();

                    auto capacity = container.Elements().capacity();
                    
                    Assert::IsTrue(container.Elements().capacity() == capacity);

                    container.Reserve(capacity + 10);
                    Assert::IsTrue(container.Elements().capacity() > capacity);
                }

                GLTFSDK_TEST_METHOD(IndexedContainerTests, IndexedContainer_Test_Size)
                {
                    auto container = GetSampleContainer();

                    Assert::IsTrue(container.Size() == 6);

                    container.Remove("foo4");

                    Assert::IsTrue(container.Size() == 5);
                }

                GLTFSDK_TEST_METHOD(IndexedContainerTests, IndexedContainer_Test_Append_ThrowOnEmpty)
                {
                    Assert::ExpectException<GLTFException>([]
                    {
                        IndexedContainer<const Uint8WithId> container;
                        container.Append({}, AppendIdPolicy::ThrowOnEmpty);
                    }, L"IndexedContainer did not throw the expected exception when appending an item with an empty string id");
                }

                GLTFSDK_TEST_METHOD(IndexedContainerTests, IndexedContainer_Test_Append_GenerateOnEmpty)
                {
                    IndexedContainer<const Uint8WithId> container;

                    {
                        auto& item = container.Append({}, AppendIdPolicy::GenerateOnEmpty);
                        Assert::AreEqual("0", item.id.c_str(), L"The expected item id was not generated when specifying the GenerateOnEmpty append policy");
                    }

                    {
                        auto& item = container.Append({}, AppendIdPolicy::GenerateOnEmpty);
                        Assert::AreEqual("1", item.id.c_str(), L"The expected item id was not generated when specifying the GenerateOnEmpty append policy");
                    }

                    container.Clear();

                    {
                        auto& item = container.Append({}, AppendIdPolicy::GenerateOnEmpty);
                        Assert::AreEqual("0", item.id.c_str(), L"The expected item id was not generated when specifying the GenerateOnEmpty append policy");
                    }
                }

                GLTFSDK_TEST_METHOD(IndexedContainerTests, IndexedContainer_Test_Append_GenerateOnEmpty_Unique)
                {
                    IndexedContainer<const Uint8WithId> container;

                    container.Append({ "2", 0 }, AppendIdPolicy::ThrowOnEmpty);
                    container.Append({ "2+", 0 }, AppendIdPolicy::ThrowOnEmpty);

                    {
                        auto& item = container.Append({}, AppendIdPolicy::GenerateOnEmpty);
                        Assert::AreEqual("2++", item.id.c_str(), L"The expected item id was not generated when specifying the GenerateOnEmpty append policy");
                    }
                }

                GLTFSDK_TEST_METHOD(IndexedContainerTests, IndexedContainer_Test_Append_GenerateOnEmpty_Duplicate)
                {
                    IndexedContainer<const Uint8WithId> container;

                    container.Append({ "2", 0 });

                    Assert::ExpectException<GLTFException>([&container]
                    {
                        container.Append({ "2", 0 }, AppendIdPolicy::GenerateOnEmpty);
                    }, L"IndexedContainer did not throw the expected exception when appending an item with a duplicate string id");
                }
            };
        }
    }
}