// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <rapidjson/document.h>
#include <rapidjson/internal/stack.h>

#include <new>

using namespace glTF::UnitTest;

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            namespace
            {
                class FailingAllocator : public rapidjson::CrtAllocator
                {
                public:
                    size_t calls = 0;
                    size_t failAt = 0;

                    void* Realloc(void* original, size_t originalSize, size_t newSize)
                    {
                        if (++calls == failAt)
                        {
                            return nullptr;
                        }
                        return rapidjson::CrtAllocator::Realloc(original, originalSize, newSize);
                    }
                };
            }

            GLTFSDK_TEST_CLASS(StackAllocationTests)
            {
                GLTFSDK_TEST_METHOD(StackAllocationTests, InitialAllocationFailure)
                {
                    FailingAllocator allocator;
                    allocator.failAt = 1;
                    rapidjson::internal::Stack<FailingAllocator> stack(&allocator, 16);

                    bool threw = false;
                    try
                    {
                        stack.Reserve<char>();
                    }
                    catch (const std::bad_alloc&)
                    {
                        threw = true;
                    }
                    Assert::IsTrue(threw);
                    if (!threw)
                    {
                        return;
                    }

                    Assert::IsTrue(stack.Empty());
                    Assert::IsNull(stack.Bottom<char>());
                    allocator.failAt = 0;
                    *stack.Push<char>() = 'a';
                    Assert::AreEqual('a', *stack.Bottom<char>());
                }

                GLTFSDK_TEST_METHOD(StackAllocationTests, GrowingAllocationFailure)
                {
                    FailingAllocator allocator;
                    rapidjson::internal::Stack<FailingAllocator> stack(&allocator, 16);
                    char* original = stack.Push<char>(4);
                    original[0] = 'a';
                    original[3] = 'd';
                    allocator.failAt = allocator.calls + 1;

                    bool threw = false;
                    try
                    {
                        stack.Reserve<char>(13);
                    }
                    catch (const std::bad_alloc&)
                    {
                        threw = true;
                    }
                    Assert::IsTrue(threw);
                    if (!threw)
                    {
                        rapidjson::CrtAllocator::Free(original);
                        return;
                    }

                    Assert::AreEqual(size_t(4), stack.GetSize());
                    Assert::AreEqual(size_t(16), stack.GetCapacity());
                    Assert::AreEqual('a', stack.Bottom<char>()[0]);
                    Assert::AreEqual('d', stack.Bottom<char>()[3]);
                    allocator.failAt = 0;
                    *stack.Push<char>(13) = 'e';
                    Assert::AreEqual('a', stack.Bottom<char>()[0]);
                    Assert::AreEqual('d', stack.Bottom<char>()[3]);
                    Assert::AreEqual('e', stack.Bottom<char>()[4]);
                }

                GLTFSDK_TEST_METHOD(StackAllocationTests, ShrinkingAllocationFailure)
                {
                    FailingAllocator allocator;
                    rapidjson::internal::Stack<FailingAllocator> stack(&allocator, 16);
                    char* original = stack.Push<char>();
                    *original = 'a';
                    allocator.failAt = allocator.calls + 1;

                    bool threw = false;
                    try
                    {
                        stack.ShrinkToFit();
                    }
                    catch (const std::bad_alloc&)
                    {
                        threw = true;
                    }
                    Assert::IsTrue(threw);
                    if (!threw)
                    {
                        rapidjson::CrtAllocator::Free(original);
                        return;
                    }

                    Assert::AreEqual(size_t(1), stack.GetSize());
                    Assert::AreEqual(size_t(16), stack.GetCapacity());
                    Assert::AreEqual('a', *stack.Bottom<char>());
                    allocator.failAt = 0;
                    stack.ShrinkToFit();
                    Assert::AreEqual(size_t(1), stack.GetCapacity());
                    Assert::AreEqual('a', *stack.Bottom<char>());
                }

                GLTFSDK_TEST_METHOD(StackAllocationTests, ParseRecoversAfterAllocationFailure)
                {
                    FailingAllocator allocator;
                    allocator.failAt = 2;
                    using Document = rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<>, FailingAllocator>;
                    Document document(nullptr, 16, &allocator);
                    const char* json = "[0,1,2,3,4,5]";

                    Assert::ExpectException<std::bad_alloc>([&] { document.Parse(json); });
                    Assert::AreEqual(size_t(2), allocator.calls);
                    allocator.failAt = 0;
                    document.Parse(json);
                    if (document.HasParseError() || !document.IsArray() || document.Size() != 6)
                    {
                        Assert::Fail(L"Document did not recover after the allocation failure.");
                        return;
                    }
                    if (!document[5].IsInt())
                    {
                        Assert::Fail(L"Document has an unexpected element type.");
                        return;
                    }
                    Assert::AreEqual(5, document[5].GetInt());
                }
            }
        }
    }
}
