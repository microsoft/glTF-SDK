// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <GLTFSDK/StreamCache.h>

#include <list>

namespace Microsoft
{
    namespace glTF
    {
        // Adds 'Least Recently Used' (LRU) eviction functionality via a composition relationship with the StreamCache class
        template<typename TStream>
        class StreamCacheLRU : public IStreamCache<TStream>
        {
        public:
            template<typename Fn>
            StreamCacheLRU(Fn fnGenerate, size_t cacheMaxSize = std::numeric_limits<size_t>::max()) :
                cacheMaxSize(cacheMaxSize),
                m_cache([fnGenerate, this](const std::string& uri) { return Update(uri, fnGenerate(uri)); }),
                m_cacheList()
            {
                if (cacheMaxSize == 0U)
                {
                    throw GLTFException("LRU max cache size must be greater than zero");
                }
            }

            TStream Get(const std::string& uri) override
            {
                auto it = m_cache.Get(uri);

                // Sanity check that the list and cache sizes match
                if (m_cache.Size() != m_cacheList.size())
                {
                    throw GLTFException("Size mismatch between cache map and list");
                }

                // Ensure the returned stream and uri are now the 'most recently used'
                if (it != m_cacheList.begin())
                {
                    m_cacheList.splice(m_cacheList.begin(), m_cacheList, it);
                }

                return it->second;
            }

            TStream Set(const std::string& uri, TStream stream) override
            {
                // If the cache already contains an entry for 'uri' then remove it from the LRU list
                // before calling Update, otherwise the list will contain duplicate enties for 'uri'
                if (m_cache.Has(uri))
                {
                    m_cacheList.erase(m_cache.Get(uri));
                }

                // Update the LRU list then the cache with the new stream
                auto it = m_cache.Set(uri, Update(uri, std::move(stream)));

                // Sanity check that the list and cache sizes match
                if (m_cache.Size() != m_cacheList.size())
                {
                    throw GLTFException("Size mismatch between cache map and list");
                }

                return it->second;
            }

            size_t Size() const
            {
                return m_cache.Size();
            }

            const size_t cacheMaxSize;

        private:
            typedef std::list<std::pair<std::string, TStream>> StreamCacheLRUList;

            typename StreamCacheLRUList::iterator Update(const std::string& uri, TStream&& stream)
            {
                // Add the stream and uri to the front of the LRU list then erase the
                // least recently used entry from the cache and LRU list if necessary
                m_cacheList.emplace_front(uri, std::move(stream));

                // Remove the LRU stream from the cache if the cache's size equals the maximum size - stream has yet to be added to the cache
                if (m_cache.Size() == cacheMaxSize)
                {
                    m_cache.Erase(m_cacheList.back().first);
                }

                // Remove the LRU stream from the LRU list if the list's size exceeds the maximum size - stream has already been added to the LRU list
                if (m_cacheList.size() > cacheMaxSize)
                {
                    m_cacheList.pop_back();
                }

                return m_cacheList.begin();
            }

            StreamCache<typename StreamCacheLRUList::iterator> m_cache;
            StreamCacheLRUList m_cacheList;
        };

        typedef StreamCacheLRU<std::shared_ptr<std::istream>> StreamReaderCacheLRU;
        typedef StreamCacheLRU<std::shared_ptr<std::ostream>> StreamWriterCacheLRU;
    }
}
