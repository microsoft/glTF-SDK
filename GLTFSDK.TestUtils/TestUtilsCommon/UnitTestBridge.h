// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#ifndef UnitTestBridge_h
#define UnitTestBridge_h

// You can define USE_GOOGLE_TEST=0 in your project to make UnitTestBridge be a
// wrapper for VisualStudio CppUnitTestFramework instead of Google Test.
#ifndef USE_GOOGLE_TEST
#define USE_GOOGLE_TEST 1
#endif

#if USE_GOOGLE_TEST

/*
With Google Test you do not need to declare test classes, just test methods as below.  Your
test methods are actually classes, though, and so can inherit from a base class with the
GLTFSDK_TEST_METHOD_F macro.

GLTFSDK_TEST_METHOD(AllTheTests, Test0)
{
Assert::IsTrue(true);
}

class SharedTestCode : public GLTFSDK_TEST_BASE
{
public:
void foo() {}
};

GLTFSDK_TEST_METHOD_F(SharedTestCode, Test1)
{
foo();
Assert::IsTrue(true);
}

Google Test does not need test classes like VS CppUnitTest.  GLTFSDK_TEST_CLASS macros
exist only for the purpose of having a single unit test source file that can be compiled
for Google Test or VS CppUnitTest by setting USE_GOOGLE_TEST in your project defines.

Note that you can't use public or private, etc., in your GLTFSDK_TEST_CLASSes because they
are merely namespaces if compiling for Google Test.  VS still finds your tests just fine.

GLTFSDK_TEST_CLASS(MyTestClass)
{
GLTFSDK_TEST_METHOD(MyTestClass, TestMethod0)
{
...
}
};

class SharedClass : public GLTFSDK_TEST_BASE
{
public:
void bar() {}
};

GLTFSDK_TEST_CLASS_F(MyOtherTestClass, SharedClass)
{
GLTFSDK_TEST_METHOD(MyOtherTestClass, TestMethod1)
{
bar();
...
}
};

Google Test fixtures (hence the *_F on the macros) have some standard methods you can override.

class TestFixture : public GLTFSDK_TEST_BASE
{
public:
TestFixture();                      // initialization code
~TestFixture();                     // cleanup, no exceptions allowed

virtual void SetUp() override;      // code will execute right before each test method
virtual void TearDown() override;   // code will execute right after each test method
};

// In order for VS UnitTests to call SetUp and TearDown you need to use the
// GLTFSDK_TEST_METHOD_INITIALIZE and GLTFSDK_TEST_METHOD_CLEANUP macros in
// your unit test class.

class TestFixture : public GLTFSDK_TEST_BASE
{
public:
virtual void SetUp() override
{
// do all the things
}
};

GLTFSDK_TEST_CLASS_F(TestFixture)
{
GLTFSDK_TEST_METHOD_INITIALIZE(TestFixture_Init);

GLTFSDK_TEST_METHOD_F(TestFixture, Test_TestA)
{
// SetUp will have been called before we get here.
}
};

To set a filter on a test method you can use the GLTFSDK_TEST_METHOD_FILTER or
GLTFSDK_TEST_METHOD_FILTER_F macros and pass the filter as the third argument. DO NOT quote
the filter. If GTest is enabled, the macro(s) will append __ (double underscores) followed
by the filter value to the method name else two method attributes will be used to set the owner
and a trait named "Filter" to the filter value.

GLTFSDK_TEST_CLASS(MyTestClass)
{
GLTFSDK_TEST_METHOD_FILTER(MyTestClass, TestMethod0, LOCAL)
{
...
}
};
*/

#include <gtest/gtest.h>
#include <iostream>

#define GLTFSDK_TEST_CLASS(CLASSNAME) namespace CLASSNAME
#define GLTFSDK_TEST_CLASS_F(INHERITCLASS) namespace INHERITCLASS ## space

#define GLTFSDK_TEST_METHOD_INTERNAL(TESTNAME, METHOD) TEST(TESTNAME, METHOD)
#define GLTFSDK_TEST_METHOD_INTERNAL_F(CLASS, METHOD) TEST_F(CLASS, METHOD)

#define GLTFSDK_TEST_METHOD_FILTER(TESTNAME, METHOD, FILTER) GLTFSDK_TEST_METHOD_INTERNAL(TESTNAME, METHOD ## __ ## FILTER)
#define GLTFSDK_TEST_METHOD_FILTER_F(CLASS, METHOD, FILTER) GLTFSDK_TEST_METHOD_INTERNAL_F(CLASS, METHOD ## __ ## FILTER)

#define BEGIN_TEST_CLASS_ATTRIBUTE()
#define TEST_CLASS_ATTRIBUTE(a, b)
#define END_TEST_CLASS_ATTRIBUTE()

#define GLTFSDK_TEST_METHOD_INITIALIZE(METHODNAME)
#define GLTFSDK_TEST_METHOD_CLEANUP(METHODNAME)

#else

#include <CppUnitTest.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#define GLTFSDK_TEST_CLASS(CLASSNAME) TEST_CLASS(CLASSNAME)
#define GLTFSDK_TEST_CLASS_F(INHERITCLASS) TEST_CLASS(INHERITCLASS##class), INHERITCLASS

#define GLTFSDK_TEST_METHOD_INTERNAL(TESTNAME, METHOD) TEST_METHOD(METHOD)
#define GLTFSDK_TEST_METHOD_INTERNAL_F(CLASS, METHOD) TEST_METHOD(METHOD)
#define GLTFSDK_TEST_METHOD_FILTER_ATTRIBUTE_INTERNAL(METHOD, FILTER)\
    BEGIN_TEST_METHOD_ATTRIBUTE(METHOD) \
        TEST_OWNER (L#FILTER) \
        TEST_METHOD_ATTRIBUTE (L"Filter", L#FILTER) \
    END_TEST_METHOD_ATTRIBUTE() \

#define GLTFSDK_TEST_METHOD_FILTER(TESTNAME, METHOD, FILTER) \
    GLTFSDK_TEST_METHOD_FILTER_ATTRIBUTE_INTERNAL(METHOD, FILTER) \
    GLTFSDK_TEST_METHOD_INTERNAL(TESTNAME, METHOD)

#define GLTFSDK_TEST_METHOD_FILTER_F(CLASS, METHOD, FILTER) \
    GLTFSDK_TEST_METHOD_FILTER_ATTRIBUTE_INTERNAL(METHOD, FILTER) \
    GLTFSDK_TEST_METHOD_INTERNAL_F(CLASS, METHOD)

namespace glTF
{
    namespace UnitTest = Microsoft::VisualStudio::CppUnitTestFramework;
}

namespace testing
{
    class Test
    {
    public:
        virtual void SetUp() {}
        virtual void TearDown() {}
    };
}

#define GLTFSDK_TEST_METHOD_INITIALIZE(METHODNAME) \
    TEST_METHOD_INITIALIZE(METHODNAME) { SetUp(); }

#define GLTFSDK_TEST_METHOD_CLEANUP(METHODNAME) \
    TEST_METHOD_CLEANUP(METHODNAME) { TearDown(); }

#endif

#define GLTFSDK_TEST_METHOD(TESTNAME, METHOD) GLTFSDK_TEST_METHOD_FILTER(TESTNAME, METHOD, none)
#define GLTFSDK_TEST_METHOD_F(CLASS, METHOD) GLTFSDK_TEST_METHOD_FILTER_F(CLASS, METHOD, none)

#define GLTFSDK_TEST_BASE ::testing::Test

#if USE_GOOGLE_TEST
namespace Microsoft
{
    namespace VisualStudio
    {
        namespace CppUnitTestFramework
        {
            template <typename Q> static std::wstring ToString(const Q& q) {}
            template <typename Q> static std::wstring ToString(const Q* q) {}
            template <typename Q> static std::wstring ToString(Q* q) {}
        }
    }
}

namespace glTF
{
    namespace UnitTest
    {
        namespace Assert
        {
            inline void Fail(const wchar_t* message = nullptr)
            {
                FAIL() << message;
            }

            inline void IsTrue(bool b, wchar_t const* message = nullptr)
            {
                EXPECT_TRUE(b) << message;
            }

            inline void IsFalse(bool b, wchar_t const* message = nullptr)
            {
                EXPECT_FALSE(b) << message;
            }

            template <typename T>
            inline void IsNull(const T* actual, wchar_t const* message = nullptr)
            {
                EXPECT_TRUE(actual == nullptr) << message;
            }

            template <typename T>
            inline void IsNotNull(const T* actual, wchar_t const* message = nullptr)
            {
                EXPECT_FALSE(actual == nullptr) << message;
            }

            template <typename T>
            inline void AreEqual(T a, T b, T tolerance, wchar_t const* message = nullptr)
            {
                double diff = (double)(a - b);
                if (diff < 0) diff = -diff;
                EXPECT_LE(diff, tolerance) << message;
            }

            inline void AreEqual(int a, int b, double tolerance, wchar_t const* message = nullptr)
            {
                double diff = (double)(a - b);
                if (diff < 0) diff = -diff;
                EXPECT_LE(diff, tolerance) << message;
            }

            template <typename T>
            inline void AreEqual(T a, T b, wchar_t const* message = nullptr)
            {
                EXPECT_EQ(a, b) << message;
            }

            inline void AreEqual(std::size_t a, unsigned int b, wchar_t const* message = nullptr)
            {
                std::size_t c = static_cast<std::size_t>(b);
                EXPECT_EQ(a, c) << message;
            }

            template <>
            inline void AreEqual<char const*>(char const* a, char const* b, wchar_t const* message)
            {
                EXPECT_STREQ(a, b) << message;
            }

            template <>
            inline void AreEqual<char*>(char* a, char* b, wchar_t const* message)
            {
                EXPECT_STREQ(a, b) << message;
            }

            template <>
            inline void AreEqual<wchar_t const*>(wchar_t const* a, wchar_t const* b, wchar_t const* message)
            {
                EXPECT_STREQ(a, b) << message;
            }

            template <>
            inline void AreEqual<wchar_t*>(wchar_t* a, wchar_t* b, wchar_t const* message)
            {
                EXPECT_STREQ(a, b) << message;
            }

            template <typename T>
            inline void AreNotEqual(T a, T b, wchar_t const* message = nullptr)
            {
                EXPECT_NE(a, b) << message;
            }

            inline void AreNotEqual(std::size_t a, unsigned int b, wchar_t const* message = nullptr)
            {
                std::size_t c = static_cast<std::size_t>(b);
                EXPECT_NE(a, c) << message;
            }

            template <>
            inline void AreNotEqual<char const*>(char const* a, char const* b, wchar_t const* message)
            {
                EXPECT_STRNE(a, b) << message;
            }

            template <>
            inline void AreNotEqual<char*>(char* a, char* b, wchar_t const* message)
            {
                EXPECT_STRNE(a, b) << message;
            }

            template <>
            inline void AreNotEqual<wchar_t const*>(wchar_t const* a, wchar_t const* b, wchar_t const* message)
            {
                EXPECT_STRNE(a, b) << message;
            }

            template <>
            inline void AreNotEqual<wchar_t*>(wchar_t* a, wchar_t* b, wchar_t const* message)
            {
                EXPECT_STRNE(a, b) << message;
            }

            template <typename T>
            inline void AreSame(const T& expected, const T& actual, wchar_t const* message = nullptr)
            {
                EXPECT_TRUE(&expected == &actual) << message;
            }

            template <typename T>
            inline void AreNotSame(const T& notExpected, const T& actual, wchar_t const* message = nullptr)
            {
                EXPECT_FALSE(&notExpected == &actual) << message;
            }

            template <typename _EXPECTEDEXCEPTION, typename _FUNCTOR>
            inline void ExpectException(_FUNCTOR functor, wchar_t const* message = nullptr)
            {
                EXPECT_THROW(functor(), _EXPECTEDEXCEPTION) << message;
            }

            template <typename _EXPECTEDEXCEPTION, typename _RETURNTYPE>
            inline void ExpectException(_RETURNTYPE(*func)(), wchar_t const* message = nullptr)
            {
                EXPECT_THROW(func(), _EXPECTEDEXCEPTION) << message;
            }
        }

        namespace Logger
        {
            inline void WriteMessage(wchar_t const* message)
            {
                std::cout << message << '\n';
            }

            inline void WriteMessage(char const* message)
            {
                std::cout << message << '\n';
            }
        }
    }
}
#endif // USE_GOOGLE_TEST

#endif /* UnitTestBridge_h */
