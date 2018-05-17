// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <tuple>
#include <string>

#include <cstdint>

namespace Microsoft
{
    namespace glTF
    {
        struct Version
        {
            uint32_t major;
            uint32_t minor;

            constexpr Version(uint32_t major, uint32_t minor) : major(major), minor(minor)
            {
            }

            constexpr Version(const std::tuple<uint32_t, uint32_t>& version) : Version(std::get<0>(version), std::get<1>(version))
            {
            }

            explicit Version(const char* version);
            explicit Version(const std::string& version);

            std::string AsString() const;

            bool operator==(const Version& rhs) const;
            bool operator!=(const Version& rhs) const;

            static std::tuple<uint32_t, uint32_t> AsTuple(const char* version);
        };

        namespace Versions
        {
            constexpr Version v2_0 = { 2U, 0U };
        }

        bool IsMinVersionRequirementSatisfied(const Version& minVersion, std::initializer_list<Version> supported = { Versions::v2_0 });
        bool IsMinVersionRequirementSatisfied(const std::string& minVersion, std::initializer_list<Version> supported = { Versions::v2_0 });
    }
}
