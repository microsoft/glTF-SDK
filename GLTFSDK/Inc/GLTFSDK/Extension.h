// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <memory>

namespace Microsoft
{
    namespace glTF
    {
        class Extension
        {
        public:
            virtual ~Extension() = default;

            virtual std::unique_ptr<Extension> Clone() const = 0;
            virtual bool IsEqual(const Extension& other) const = 0;

            bool operator==(const Extension& rhs) const;
            bool operator!=(const Extension& rhs) const;

        protected:
            Extension() = default;
        };
    }
}
