// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/MicrosoftGeneratorVersion.h>

#include <GLTFSDK/Constants.h>

#include <regex>

using namespace Microsoft::glTF;

MicrosoftGeneratorVersion::MicrosoftGeneratorVersion(const std::string& version) :
    m_version(std::make_tuple(0, 0, 0, 0)),
    m_prereleaseVersion(0),
    m_isPrerelease(false),
    m_isMicrosoftGenerator(false)
{
    if (!version.empty())
    {
        std::string msftGeneratorName(MSFT_GLTF_EXPORTER_NAME);

        //check whether the version name starts with the generator name
        auto versionLocal(version);
        if (versionLocal.size() >= msftGeneratorName.size() &&
            std::equal(msftGeneratorName.begin(), msftGeneratorName.end(), versionLocal.begin()))
        {
            m_isMicrosoftGenerator = true;
            versionLocal = versionLocal.substr(msftGeneratorName.size());
        }

        unsigned long major = 0, minor = 0, revision = 0, build = 0, prerelease = 0;

        const std::regex regex3("(\\d+)(\\.)(\\d+)(\\.)(\\d+)");
        const std::regex regex4("(\\d+)(\\.)(\\d+)(\\.)(\\d+)(\\.)(\\d+)");
        const std::regex regex3beta("(\\d+)(\\.)(\\d+)(\\.)(\\d+)-b(\\d+)");
        const std::regex regex4beta("(\\d+)(\\.)(\\d+)(\\.)(\\d+)(\\.)(\\d+)-b(\\d+)");

        std::smatch match;
        bool didMatch = true;
        if (std::regex_search(versionLocal, match, regex4beta))
        {
            build = std::stoul(match[7]);
            prerelease = std::stoul(match[8]);
            m_isPrerelease = true;
        }
        else if (std::regex_search(versionLocal, match, regex3beta))
        {
            prerelease = std::stoul(match[6]);
            m_isPrerelease = true;
        }
        else if (std::regex_search(versionLocal, match, regex4))
        {
            build = std::stoul(match[7]);
        }
        else if (!std::regex_search(versionLocal, match, regex3))
        {
            didMatch = false;
        }

        //parse the commmon components (major.minor.revision)
        if (didMatch)
        {
            major = std::stoul(match[1]);
            minor = std::stoul(match[3]);
            revision = std::stoul(match[5]);
            m_version = std::make_tuple(major, minor, revision, build);
            m_prereleaseVersion = prerelease;
        }
    }
}

bool MicrosoftGeneratorVersion::operator==(const MicrosoftGeneratorVersion& other) const
{
    return m_version == other.m_version
        && m_isPrerelease == other.m_isPrerelease
        && m_prereleaseVersion == other.m_prereleaseVersion;
}

bool MicrosoftGeneratorVersion::operator<(const MicrosoftGeneratorVersion& other) const
{
    if (m_isPrerelease)
    {
        if (other.m_isPrerelease && m_version == other.m_version)
        {
            return m_prereleaseVersion < other.m_prereleaseVersion;
        }
        return m_version <= other.m_version;
    }

    if (other.m_isPrerelease)
    {
        return m_version < other.m_version;
    }

    return (m_version < other.m_version);
}

bool MicrosoftGeneratorVersion::operator>(const MicrosoftGeneratorVersion& other) const
{
    return other.operator<(*this);
}

bool MicrosoftGeneratorVersion::operator<=(const MicrosoftGeneratorVersion& other) const
{
    return operator<(other) || operator==(other);
}

bool MicrosoftGeneratorVersion::operator>=(const MicrosoftGeneratorVersion& other) const
{
    return operator>(other) || operator==(other);
}

bool MicrosoftGeneratorVersion::operator!=(const MicrosoftGeneratorVersion& other) const
{
    return !operator==(other);
}

bool MicrosoftGeneratorVersion::IsMicrosoftGenerator() const
{
    return m_isMicrosoftGenerator;
}

bool MicrosoftGeneratorVersion::IsPrerelease() const
{
    return m_isPrerelease;
}