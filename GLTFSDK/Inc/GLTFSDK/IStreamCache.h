// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <istream>
#include <memory>
#include <ostream>
#include <string>

namespace Microsoft
{
    namespace glTF
    {
        template<typename TStream>
        class IStreamCache
        {
        public:
            virtual ~IStreamCache() = default;

            // Returns a stream from the cache
            //
            // Note: Calls to Get must always succeed and return a valid stream. For a given uri
            // Implementations of IStreamCache must automatically populate the cache with a stream
            // if one does not already exist
            virtual TStream Get(const std::string& uri) = 0;

            // Explicitly populate the cache with the specified stream
            virtual TStream Set(const std::string& uri, TStream stream) = 0;
        };

        typedef IStreamCache<std::shared_ptr<std::istream>> IStreamReaderCache;
        typedef IStreamCache<std::shared_ptr<std::ostream>> IStreamWriterCache;
    }
}
