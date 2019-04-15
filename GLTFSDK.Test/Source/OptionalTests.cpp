// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/Optional.h>

using namespace glTF::UnitTest;

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(OptionalTests)
            {
                GLTFSDK_TEST_METHOD(OptionalTests, ConstructorDefault)
                {
                    Optional<int> optional;

                    Assert::IsFalse(static_cast<bool>(optional));
                    Assert::IsFalse(optional.HasValue());

                    Assert::ExpectException<GLTFException>([&optional]{ optional.Get(); });
                }

                GLTFSDK_TEST_METHOD(OptionalTests, ConstructorValueCopy)
                {
                    Optional<double> optional(1.0);

                    Assert::IsTrue(static_cast<bool>(optional));
                    Assert::IsTrue(optional.HasValue());

                    Assert::AreEqual(1.0, optional.Get());
                }

                GLTFSDK_TEST_METHOD(OptionalTests, ConstructorValueMove)
                {
                    Optional<std::unique_ptr<int>> optional(std::make_unique<int>(1));

                    Assert::IsTrue(static_cast<bool>(optional));
                    Assert::IsTrue(optional.HasValue());

                    Assert::AreEqual(1, *optional.Get().get());
                }

                GLTFSDK_TEST_METHOD(OptionalTests, ConstructorOptionalCopy)
                {
                    Optional<unsigned int> opt1;
                    Optional<unsigned int> opt2(opt1);
                    Optional<unsigned int> opt3(3U);
                    Optional<unsigned int> opt4(opt3);

                    Assert::IsFalse(static_cast<bool>(opt1));
                    Assert::IsFalse(opt1.HasValue());

                    Assert::IsFalse(static_cast<bool>(opt2));
                    Assert::IsFalse(opt2.HasValue());

                    Assert::IsTrue(static_cast<bool>(opt3));
                    Assert::IsTrue(opt3.HasValue());

                    Assert::IsTrue(static_cast<bool>(opt4));
                    Assert::IsTrue(opt4.HasValue());

                    Assert::ExpectException<GLTFException>([&opt1] { opt1.Get(); });
                    Assert::ExpectException<GLTFException>([&opt2] { opt2.Get(); });

                    Assert::AreEqual(3U, opt3.Get());
                    Assert::AreEqual(3U, opt4.Get());
                }

                GLTFSDK_TEST_METHOD(OptionalTests, ConstructorOptionalMove)
                {
                    Optional<std::unique_ptr<int>> opt1;
                    Optional<std::unique_ptr<int>> opt2(std::move(opt1));
                    Optional<std::unique_ptr<int>> opt3(std::make_unique<int>(3));
                    Optional<std::unique_ptr<int>> opt4(std::move(opt3));

                    Assert::IsFalse(static_cast<bool>(opt1));
                    Assert::IsFalse(opt1.HasValue());

                    Assert::IsFalse(static_cast<bool>(opt2));
                    Assert::IsFalse(opt2.HasValue());

                    Assert::IsFalse(static_cast<bool>(opt3));
                    Assert::IsFalse(opt3.HasValue());

                    Assert::IsTrue(static_cast<bool>(opt4));
                    Assert::IsTrue(opt4.HasValue());

                    Assert::ExpectException<GLTFException>([&opt1] { opt1.Get(); });
                    Assert::ExpectException<GLTFException>([&opt2] { opt2.Get(); });
                    Assert::ExpectException<GLTFException>([&opt3] { opt3.Get(); });

                    Assert::AreEqual(3, *opt4.Get().get());
                }

                GLTFSDK_TEST_METHOD(OptionalTests, DestructorReset)
                {
                    unsigned int count = 0U;

                    struct Counter
                    {
                        Counter(unsigned int* count_ptr) : count_ptr(count_ptr)
                        {
                            ++(*count_ptr);
                        }

                        Counter(Counter&& other) : count_ptr(other.count_ptr)
                        {
                            ++(*count_ptr);
                        }

                        Counter(const Counter& other) : count_ptr(other.count_ptr)
                        {
                            ++(*count_ptr);
                        }

                        ~Counter()
                        {
                            --(*count_ptr);
                        }

                        unsigned int* count_ptr;
                    };

                    {
                        Optional<Counter> opt1(&count);

                        {
                            Assert::AreEqual(1U, count);
                            Optional<Counter> opt2(&count);
                            Assert::AreEqual(2U, count);
                        }

                        Assert::AreEqual(1U, count);
                        opt1.Reset();
                        Assert::AreEqual(0U, count);
                    }

                    Assert::AreEqual(0U, count);
                }

                GLTFSDK_TEST_METHOD(OptionalTests, Swap)
                {
                    // Both lhs and rhs Optionals have values
                    {
                        Optional<char> optA('A');
                        Optional<char> optB('B');

                        Assert::AreEqual('A', optA.Get());
                        Assert::AreEqual('B', optB.Get());

                        Optional<char>::Swap(optA, optB); // After swapping optA should contain 'B' and optB should contain 'A'

                        Assert::AreEqual('B', optA.Get());
                        Assert::AreEqual('A', optB.Get());
                    }

                    // Only lhs Optional has a value
                    {
                        Optional<char> optA('A');
                        Optional<char> optB;

                        Assert::AreEqual('A', optA.Get());
                        Assert::IsFalse(optB.HasValue());

                        Optional<char>::Swap(optA, optB); // After swapping optA should be empty and optB should contain 'A'

                        Assert::IsFalse(optA.HasValue());
                        Assert::AreEqual('A', optB.Get());
                    }

                    // Only rhs Optional has a value
                    {
                        Optional<char> optA;
                        Optional<char> optB('B');

                        Assert::IsFalse(optA.HasValue());
                        Assert::AreEqual('B', optB.Get());

                        Optional<char>::Swap(optA, optB); // After swapping optA should contain 'B' and optB should be empty

                        Assert::AreEqual('B', optA.Get());
                        Assert::IsFalse(optB.HasValue());
                    }
                }

                GLTFSDK_TEST_METHOD(OptionalTests, AssignmentValueCopy)
                {
                    const std::string assignValue("Assign");

                    // Test assignment when Optional has no existing value
                    {
                        Optional<std::string> opt;

                        Assert::IsFalse(opt.HasValue());
                        opt = assignValue;
                        Assert::IsTrue(opt.HasValue());

                        Assert::AreEqual("Assign", opt.Get().c_str());
                    }

                    // Test assignment when Optional has an existing value
                    {
                        Optional<std::string> opt("Init");

                        Assert::IsTrue(opt.HasValue());
                        opt = assignValue;
                        Assert::IsTrue(opt.HasValue());

                        Assert::AreEqual("Assign", opt.Get().c_str());
                    }
                }

                GLTFSDK_TEST_METHOD(OptionalTests, AssignmentValueMove)
                {
                    // Test assignment when Optional has no existing value
                    {
                        Optional<std::string> opt;

                        Assert::IsFalse(opt.HasValue());
                        opt = std::string("Assign");
                        Assert::IsTrue(opt.HasValue());

                        Assert::AreEqual("Assign", opt.Get().c_str());
                    }

                    // Test assignment when Optional has an existing value
                    {
                        Optional<std::string> opt("Init");

                        Assert::IsTrue(opt.HasValue());
                        opt = std::string("Assign");
                        Assert::IsTrue(opt.HasValue());

                        Assert::AreEqual("Assign", opt.Get().c_str());
                    }
                }

                GLTFSDK_TEST_METHOD(OptionalTests, AssignmentOptionalCopy)
                {
                    // Test assignment when Optional has no existing value
                    {
                        Optional<std::string> opt1;
                        Optional<std::string> opt2("Assign");

                        Assert::IsFalse(opt1.HasValue());
                        Assert::IsTrue(opt2.HasValue());

                        opt1 = opt2;

                        Assert::IsTrue(opt1.HasValue());
                        Assert::IsTrue(opt2.HasValue());

                        Assert::AreEqual("Assign", opt1.Get().c_str());
                        Assert::AreEqual("Assign", opt2.Get().c_str());
                    }

                    // Test assignment when Optional has an existing value - assign no value
                    {
                        Optional<std::string> opt1("Init");
                        Optional<std::string> opt2;

                        Assert::IsTrue(opt1.HasValue());
                        Assert::IsFalse(opt2.HasValue());

                        opt1 = opt2;

                        Assert::IsFalse(opt1.HasValue());
                        Assert::IsFalse(opt2.HasValue());
                    }

                    // Test assignment when Optional has an existing value
                    {
                        Optional<std::string> opt1("Init");
                        Optional<std::string> opt2("Assign");

                        Assert::IsTrue(opt1.HasValue());
                        Assert::IsTrue(opt2.HasValue());

                        opt1 = opt2;

                        Assert::IsTrue(opt1.HasValue());
                        Assert::IsTrue(opt2.HasValue());

                        Assert::AreEqual("Assign", opt1.Get().c_str());
                        Assert::AreEqual("Assign", opt2.Get().c_str());
                    }

                    // Test self-assignment with no existing value
                    {
                        Optional<std::string> opt;

                        Assert::IsFalse(opt.HasValue());
                        opt = opt;
                        Assert::IsFalse(opt.HasValue());
                    }

                    // Test self-assignment with an existing value
                    {
                        Optional<std::string> opt("Init");

                        Assert::IsTrue(opt.HasValue());
                        opt = opt;
                        Assert::IsTrue(opt.HasValue());

                        Assert::AreEqual("Init", opt.Get().c_str());
                    }
                }

                GLTFSDK_TEST_METHOD(OptionalTests, AssignmentOptionalMove)
                {
                    // Test assignment when Optional has no existing value
                    {
                        Optional<std::string> opt1;
                        Optional<std::string> opt2("Assign");

                        Assert::IsFalse(opt1.HasValue());
                        Assert::IsTrue(opt2.HasValue());

                        opt1 = std::move(opt2);

                        Assert::IsTrue(opt1.HasValue());
                        Assert::IsFalse(opt2.HasValue());

                        Assert::AreEqual("Assign", opt1.Get().c_str());
                    }

                    // Test assignment when Optional has an existing value - assign no value
                    {
                        Optional<std::string> opt1("Init");
                        Optional<std::string> opt2;

                        Assert::IsTrue(opt1.HasValue());
                        Assert::IsFalse(opt2.HasValue());

                        opt1 = std::move(opt2);

                        Assert::IsFalse(opt1.HasValue());
                        Assert::IsFalse(opt2.HasValue());
                    }

                    // Test assignment when Optional has an existing value
                    {
                        Optional<std::string> opt1("Init");
                        Optional<std::string> opt2("Assign");

                        Assert::IsTrue(opt1.HasValue());
                        Assert::IsTrue(opt2.HasValue());

                        opt1 = std::move(opt2);

                        Assert::IsTrue(opt1.HasValue());
                        Assert::IsFalse(opt2.HasValue());

                        Assert::AreEqual("Assign", opt1.Get().c_str());
                    }


// Disable warnings (clang only) about self move-assignment so it is possible to test that the Optional type works as expected in this situation
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
#endif
                    // Test self-assignment with no existing value
                    {
                        Optional<std::string> opt;

                        Assert::IsFalse(opt.HasValue());
                        opt = std::move(opt);
                        Assert::IsFalse(opt.HasValue());
                    }

                    // Test self-assignment with an existing value
                    {
                        Optional<std::string> opt("Init");

                        Assert::IsTrue(opt.HasValue());
                        opt = std::move(opt);
                        Assert::IsTrue(opt.HasValue());

                        Assert::AreEqual("Init", opt.Get().c_str());
                    }
#ifdef __clang__
#pragma clang diagnostic pop
#endif
                }

                GLTFSDK_TEST_METHOD(OptionalTests, EqualTo)
                {
                    // lhs and rhs have no value
                    {
                        Optional<long> opt1;
                        Optional<long> opt2;

                        Assert::IsTrue(opt1 == opt2);
                    }

                    // only lhs has a value
                    {
                        Optional<long> opt1(1L);
                        Optional<long> opt2;

                        Assert::IsFalse(opt1 == opt2);
                    }

                    // only rhs has a value
                    {
                        Optional<long> opt1;
                        Optional<long> opt2(1L);

                        Assert::IsFalse(opt1 == opt2);
                    }

                    // lhs and rhs have the same value
                    {
                        Optional<long> opt1(1L);
                        Optional<long> opt2(1L);

                        Assert::IsTrue(opt1 == opt2);
                    }

                    // lhs and rhs have different values
                    {
                        Optional<long> opt1(1L);
                        Optional<long> opt2(2L);

                        Assert::IsFalse(opt1 == opt2);
                    }
                }

                GLTFSDK_TEST_METHOD(OptionalTests, NotEqualTo)
                {
                    // lhs and rhs have no value
                    {
                        Optional<long> opt1;
                        Optional<long> opt2;

                        Assert::IsFalse(opt1 != opt2);
                    }

                    // only lhs has a value
                    {
                        Optional<long> opt1(1L);
                        Optional<long> opt2;

                        Assert::IsTrue(opt1 != opt2);
                    }

                    // only rhs has a value
                    {
                        Optional<long> opt1;
                        Optional<long> opt2(1L);

                        Assert::IsTrue(opt1 != opt2);
                    }

                    // lhs and rhs have the same value
                    {
                        Optional<long> opt1(1L);
                        Optional<long> opt2(1L);

                        Assert::IsFalse(opt1 != opt2);
                    }

                    // lhs and rhs have different values
                    {
                        Optional<long> opt1(1L);
                        Optional<long> opt2(2L);

                        Assert::IsTrue(opt1 != opt2);
                    }
                }
            };
        }
    }
}
