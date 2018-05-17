// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <string>
#include <tuple>

namespace Microsoft
{
    namespace glTF
    {
        class MicrosoftGeneratorVersion
        {
        public:
            MicrosoftGeneratorVersion(const std::string& version);

            bool operator==(const MicrosoftGeneratorVersion& other) const;
            bool operator<(const MicrosoftGeneratorVersion& other) const;
            bool operator>(const MicrosoftGeneratorVersion& other) const;
            bool operator<=(const MicrosoftGeneratorVersion& other) const;
            bool operator>=(const MicrosoftGeneratorVersion& other) const;
            bool operator!=(const MicrosoftGeneratorVersion& other) const;

            bool IsMicrosoftGenerator() const;
            bool IsPrerelease() const;

        private:
            std::tuple<unsigned long, unsigned long, unsigned long, unsigned long> m_version;
            unsigned long m_prereleaseVersion;
            bool m_isPrerelease;
            bool m_isMicrosoftGenerator;
        };
    }
}
