// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <istream>
#include <memory>
#include <string>

namespace Microsoft
{
    namespace glTF
    {
        class IStreamReader
        {
        public:
            virtual ~IStreamReader() = default;
            virtual std::shared_ptr<std::istream> GetInputStream(const std::string& filename) const = 0;
        };
    }
}
