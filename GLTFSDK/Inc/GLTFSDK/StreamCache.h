// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <GLTFSDK/Exceptions.h>
#include <GLTFSDK/IStreamCache.h>
#include <GLTFSDK/IStreamReader.h>
#include <GLTFSDK/IStreamWriter.h>

#include <functional>
#include <unordered_map>

namespace Microsoft
{
    namespace glTF
    {
        template<typename TStream>
        class StreamCache : public IStreamCache<TStream>
        {
        public:
            template<typename Fn>
            StreamCache(Fn fnGenerate) : m_cacheMap(), m_cacheFn(fnGenerate)
            {
            }

            TStream Get(const std::string& uri) override
            {
                auto it = m_cacheMap.find(uri);

                if (it == m_cacheMap.end())
                {
                    // Populate the cache with a new entry for 'uri' (acquired by calling the user supplied functor m_cacheFn)
                    return Set(uri, m_cacheFn(uri));
                }
                else
                {
                    return it->second;
                }
            }

            TStream Set(const std::string& uri, TStream stream) override
            {
                return m_cacheMap[uri] = std::move(stream);
            }

            bool Has(const std::string& uri)
            {
                return m_cacheMap.find(uri) != m_cacheMap.end();
            }

            void Erase(const std::string& uri)
            {
                auto it = m_cacheMap.find(uri);

                if (it != m_cacheMap.end())
                {
                    m_cacheMap.erase(it);
                }
                else
                {
                    throw GLTFException("Passed key doesn't exist in the stream cache");
                }
            }

            size_t Size() const
            {
                return m_cacheMap.size();
            }

        protected:
            std::unordered_map<std::string, TStream> m_cacheMap;

        private:
            std::function<TStream(const std::string&)> m_cacheFn;
        };

        typedef StreamCache<std::shared_ptr<std::istream>> StreamReaderCache;
        typedef StreamCache<std::shared_ptr<std::ostream>> StreamWriterCache;

        template<typename TStreamCache, typename ...TArgs>
        std::unique_ptr<TStreamCache> MakeStreamReaderCache(std::shared_ptr<const IStreamReader> streamReader, TArgs&& ...args)
        {
            static_assert(std::is_base_of<IStreamReaderCache, TStreamCache>::value, "Template parameter TStreamCache must derive from IStreamReaderCache");

            return std::make_unique<TStreamCache>([streamReaderLocal = std::move(streamReader)](const std::string& uri)
            {
                return streamReaderLocal->GetInputStream(uri);
            }, std::forward<TArgs>(args)...);
        }

        template<typename TStreamCache, typename ...TArgs>
        std::unique_ptr<TStreamCache> MakeStreamWriterCache(std::shared_ptr<const IStreamWriter> streamWriter, TArgs&& ...args)
        {
            static_assert(std::is_base_of<IStreamWriterCache, TStreamCache>::value, "Template parameter TStreamCache must derive from IStreamWriterCache");

            return std::make_unique<TStreamCache>([streamWriterLocal = std::move(streamWriter)](const std::string& uri)
            {
                return streamWriterLocal->GetOutputStream(uri);
            }, std::forward<TArgs>(args)...);
        }
    }
}
