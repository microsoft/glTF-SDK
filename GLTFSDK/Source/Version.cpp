// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/Version.h>

#include <GLTFSDK/Exceptions.h>

#include <set>
#include <regex>
#include <sstream>

using namespace Microsoft::glTF;

Version::Version(const char* version) : Version(AsTuple(version))
{
}

Version::Version(const std::string& version) : Version(version.c_str())
{
}

std::string Version::AsString() const
{
    std::stringstream ss;

    ss << major;
    ss << ".";
    ss << minor;

    return ss.str();
}

bool Version::operator==(const Version& rhs) const
{
    return major == rhs.major
        && minor == rhs.minor;
}

bool Version::operator!=(const Version& rhs) const
{
    return !operator==(rhs);
}

std::tuple<uint32_t, uint32_t> Version::AsTuple(const char* version)
{
    std::cmatch match_results;

    if (version && std::regex_match(version, match_results, std::regex("^([0-9]+)\\.([0-9]+)$")))
    {
        // Use std::istringstream rather than std::stoul to get consistent behaviour on both LLP64 and LP64 platforms
        uint32_t major = std::numeric_limits<uint32_t>::max();
        uint32_t minor = std::numeric_limits<uint32_t>::max();

        std::istringstream ssMajor(match_results.str(1));

        if (!(ssMajor >> major))
        {
            throw GLTFException("Invalid version number - major value would fall outside the range of the result type");
        }

        std::istringstream ssMinor(match_results.str(2));

        if (!(ssMinor >> minor))
        {
            throw GLTFException("Invalid version number - minor value would fall outside the range of the result type");
        }

        return { major, minor };
    }
    else
    {
        throw GLTFException("Invalid version number");
    }
}

bool Microsoft::glTF::IsMinVersionRequirementSatisfied(const Version& minVersion, std::initializer_list<Version> supported)
{
    if (supported.size() == 0U)
    {
        throw GLTFException("List of supported versions cannot be empty");
    }

    auto fnCompare = [](const Version& lhs, const Version& rhs)
    {
        return (lhs.major < rhs.major) || ((lhs.major == rhs.major) && (lhs.minor < rhs.minor));
    };

    std::set<Version, decltype(fnCompare)> versions(
        supported.begin(),
        supported.end(),
        fnCompare);

    auto it = versions.lower_bound(minVersion);

    if (it != versions.end())
    {
        return it->major == minVersion.major;
    }

    return false;
}

bool Microsoft::glTF::IsMinVersionRequirementSatisfied(const std::string& minVersion, std::initializer_list<Version> supported)
{
    if (supported.size() == 0U)
    {
        throw GLTFException("List of supported versions cannot be empty");
    }

    return minVersion.empty() || IsMinVersionRequirementSatisfied(Version(minVersion), supported);
}
