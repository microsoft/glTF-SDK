// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <memory>
#include <ostream>
#include <string>

namespace Microsoft
{
    namespace glTF
    {
        class IStreamWriter
        {
        public:
            virtual ~IStreamWriter() = default;
            virtual std::shared_ptr<std::ostream> GetOutputStream(const std::string& filename) const = 0;
        };
    }
}