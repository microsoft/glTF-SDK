// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/StreamCacheLRU.h>

using namespace glTF::UnitTest;

namespace
{
    class TestStreamReader : public Microsoft::glTF::IStreamReader
    {
    public:
        std::shared_ptr<std::istream> GetInputStream(const std::string& uri) const override
        {
            auto it = m_counts.find(uri);

            if (it == m_counts.end())
            {
                m_counts.insert({ uri, 1U });
            }
            else
            {
                it->second++;
            }

            return std::make_shared<std::stringstream>();
        }

        mutable std::unordered_map<std::string, size_t> m_counts;
    };
}

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(StreamCacheTest)
            {
                GLTFSDK_TEST_METHOD(StreamCacheTest, StreamReaderCacheGet)
                {
                    auto streamReader = std::make_shared<TestStreamReader>();
                    auto streamCache = MakeStreamReaderCache<StreamReaderCache>(streamReader);

                    auto stream = streamCache->Get("1");// Should populate the cache with a new stream

                    Assert::IsTrue(static_cast<bool>(stream));
                    Assert::AreEqual(size_t(1), streamReader->m_counts.size());
                    Assert::AreEqual(size_t(1), streamReader->m_counts["1"]);
                }

                GLTFSDK_TEST_METHOD(StreamCacheTest, StreamReaderCacheGetMultiple)
                {
                    auto streamReader = std::make_shared<TestStreamReader>();
                    auto streamCache = MakeStreamReaderCache<StreamReaderCache>(streamReader);

                    const std::string uri = "1";

                    auto stream1 = streamCache->Get(uri);// Should populate the cache with a new stream
                    Assert::AreEqual(size_t(1), streamCache->Size());
                    Assert::AreEqual(size_t(1), streamReader->m_counts[uri]);
                    auto stream2 = streamCache->Get(uri);// Should return the stream previously added to the cache - stream reader shouldn't be called
                    Assert::AreEqual(size_t(1), streamCache->Size());
                    Assert::AreEqual(size_t(1), streamReader->m_counts[uri]);

                    Assert::IsTrue(stream1 == stream2);
                }

                GLTFSDK_TEST_METHOD(StreamCacheTest, StreamReaderCacheSetGet)
                {
                    auto streamReader = std::make_shared<TestStreamReader>();
                    auto streamCache = MakeStreamReaderCache<StreamReaderCache>(streamReader);

                    auto stream1 = std::make_shared<std::stringstream>("Stream");
                    streamCache->Set("1", stream1);
                    auto stream2 = streamCache->Get("1");

                    Assert::IsTrue(stream1 == stream2);
                    Assert::IsTrue(streamReader->m_counts.empty());// Stream reader should not have been called
                }

                GLTFSDK_TEST_METHOD(StreamCacheTest, StreamReaderCacheSetMultiple)
                {
                    auto streamReader = std::make_shared<TestStreamReader>();
                    auto streamCache = MakeStreamReaderCache<StreamReaderCache>(streamReader);

                    auto streamDog = std::make_shared<std::stringstream>("Dog");
                    streamCache->Set("1", streamDog);
                    auto streamCat = std::make_shared<std::stringstream>("Cat");
                    streamCache->Set("2", streamCat);

                    Assert::AreEqual(size_t(2), streamCache->Size());
                    Assert::IsTrue(streamReader->m_counts.empty());

                    auto ssCached1 = std::dynamic_pointer_cast<std::stringstream>(streamCache->Get("1"));

                    Assert::IsTrue(ssCached1 == streamDog);
                    Assert::IsTrue(ssCached1->str() == "Dog");
                    Assert::IsTrue(streamReader->m_counts.empty());

                    auto ssCached2 = std::dynamic_pointer_cast<std::stringstream>(streamCache->Get("2"));

                    Assert::IsTrue(ssCached2 == streamCat);
                    Assert::IsTrue(ssCached2->str() == "Cat");
                    Assert::IsTrue(streamReader->m_counts.empty());
                }

                GLTFSDK_TEST_METHOD(StreamCacheTest, StreamReaderCacheErase)
                {
                    auto streamReader = std::make_shared<TestStreamReader>();
                    auto streamCache = MakeStreamReaderCache<StreamReaderCache>(streamReader);

                    streamCache->Get("1");
                    Assert::AreEqual(size_t(1), streamCache->Size());
                    streamCache->Erase("1");
                    Assert::AreEqual(size_t(0), streamCache->Size());
                    streamCache->Get("1");
                    Assert::AreEqual(size_t(1), streamCache->Size());

                    Assert::AreEqual(size_t(2), streamReader->m_counts["1"]);
                }

                GLTFSDK_TEST_METHOD(StreamCacheTest, StreamReaderCacheEraseFail)
                {
                    auto streamReader = std::make_shared<TestStreamReader>();
                    auto streamCache = MakeStreamReaderCache<StreamReaderCache>(streamReader);

                    Assert::ExpectException<GLTFException>([&streamCache]() { streamCache->Erase("1"); });
                }

                GLTFSDK_TEST_METHOD(StreamCacheTest, StreamReaderCacheLRUSetDuplicateKey)
                {
                    auto streamReader = std::make_shared<TestStreamReader>();
                    auto streamCache = MakeStreamReaderCache<StreamReaderCacheLRU>(streamReader);

                    auto ssOriginal = std::make_shared<std::stringstream>("Red");
                    auto ssDuplicate = std::make_shared<std::stringstream>("Yellow");

                    streamCache->Set("1", ssOriginal);
                    streamCache->Set("1", ssDuplicate);

                    auto ssCached = std::dynamic_pointer_cast<std::stringstream>(streamCache->Get("1"));

                    Assert::IsTrue(ssCached == ssDuplicate);
                    Assert::IsTrue(ssCached->str() == "Yellow");
                }

                GLTFSDK_TEST_METHOD(StreamCacheTest, StreamReaderCacheLRUSetMaxSize0)
                {
                    Assert::ExpectException<GLTFException>([]()
                    {
                        auto streamReader = std::make_shared<TestStreamReader>();
                        auto streamCache = MakeStreamReaderCache<StreamReaderCacheLRU>(streamReader, 0U);
                    });
                }

                GLTFSDK_TEST_METHOD(StreamCacheTest, StreamReaderCacheLRUSetMaxSize1)
                {
                    auto streamReader = std::make_shared<TestStreamReader>();
                    auto streamCache = MakeStreamReaderCache<StreamReaderCacheLRU>(streamReader, 1U);

                    auto stream1 = streamCache->Get("1");

                    Assert::AreEqual(size_t(1), streamReader->m_counts["1"]);
                    Assert::AreEqual(size_t(1), streamCache->Size());

                    auto stream2 = streamCache->Get("2");

                    Assert::AreEqual(size_t(1), streamReader->m_counts["2"]);
                    Assert::AreEqual(size_t(1), streamCache->Size());

                    auto stream3 = streamCache->Get("1");

                    Assert::AreEqual(size_t(2), streamReader->m_counts["1"]);
                    Assert::AreEqual(size_t(1), streamCache->Size());

                    Assert::IsFalse(stream1 == stream3);// The returned streams should be different as the cache can only hold a single entry
                }

                GLTFSDK_TEST_METHOD(StreamCacheTest, StreamReaderCacheLRUSetMaxSize2)
                {
                    auto streamReader = std::make_shared<TestStreamReader>();
                    auto streamCache = MakeStreamReaderCache<StreamReaderCacheLRU>(streamReader, 2U);

                    auto ss1 = std::make_shared<std::stringstream>("Apple");
                    auto ss2 = std::make_shared<std::stringstream>("Orange");
                    auto ss3 = std::make_shared<std::stringstream>("Pear");

                    streamCache->Set("1", ss1);
                    streamCache->Set("2", ss2);
                    streamCache->Set("3", ss3);

                    {
                        auto ss3Cached = std::dynamic_pointer_cast<std::stringstream>(streamCache->Get("3"));

                        Assert::IsTrue(ss3 == ss3Cached);
                        Assert::IsTrue(ss3Cached->str() == "Pear");
                    }

                    {
                        auto ss2Cached = std::dynamic_pointer_cast<std::stringstream>(streamCache->Get("2"));

                        Assert::IsTrue(ss2 == ss2Cached);
                        Assert::IsTrue(ss2Cached->str() == "Orange");
                    }

                    {
                        auto ss1Cached = std::dynamic_pointer_cast<std::stringstream>(streamCache->Get("1"));

                        Assert::IsTrue(ss1 != ss1Cached); // The max cache size is 2 - the 'Apple' stream should have been evicted from the cache
                        Assert::IsTrue(ss1Cached->str().empty());
                    }
                }
            };
        }
    }
}