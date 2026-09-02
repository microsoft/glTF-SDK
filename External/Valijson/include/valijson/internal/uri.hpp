#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include <valijson/internal/optional.hpp>
#include <valijson/internal/regex.hpp>

namespace valijson {
namespace internal {
namespace uri {

inline opt::optional<size_t> parseScheme(const std::string &uri)
{
    static const char *marker = "://";
    const size_t position = uri.find(marker);

    return position == std::string::npos
        ? opt::optional<size_t>()
        : opt::make_optional<size_t>(position + 3U);
}

inline bool isUriAbsolute(const std::string &uri)
{
    return static_cast<bool>(parseScheme(uri));
}

inline bool isUrn(const std::string &documentUri)
{
    static const internal::regex pattern(
        "^((urn)|(URN)):(?!urn:)([a-zA-Z0-9][a-zA-Z0-9-]{1,31})(:[-a-zA-Z0-9\\\\._~%!$&'()\\/*+,;=]+)+(\\?[-a-zA-Z0-9\\\\._~%!$&'()\\/*+,;:=]+){0,1}(#[-a-zA-Z0-9\\\\._~%!$&'()\\/*+,;:=]+){0,1}$");

    return internal::regex_match(documentUri, pattern);
}

inline std::string resolveRelativeUri(
        const std::string &resolutionScope,
        const std::string &relativeUri)
{
    if (relativeUri.empty()) {
        return resolutionScope;
    }

    if (isUriAbsolute(relativeUri) || isUrn(relativeUri)) {
        return relativeUri;
    }

    const opt::optional<size_t> schemeEnd = parseScheme(resolutionScope);
    std::string schemeAndAuthority;
    std::string basePath;
    if (schemeEnd) {
        const size_t authorityEnd = resolutionScope.find('/', *schemeEnd);
        schemeAndAuthority = resolutionScope.substr(0, authorityEnd);
        const std::string::size_type pathStart = schemeAndAuthority.size();
        basePath = pathStart < resolutionScope.size()
                ? resolutionScope.substr(pathStart)
                : "/";
    } else {
        basePath = resolutionScope;
    }

    const std::string::size_type baseFragmentPos = basePath.find('#');
    if (baseFragmentPos != std::string::npos) {
        basePath.erase(baseFragmentPos);
    }

    const std::string::size_type baseQueryPos = basePath.find('?');
    if (baseQueryPos != std::string::npos) {
        basePath.erase(baseQueryPos);
    }

    if (relativeUri[0] == '#' || relativeUri[0] == '?') {
        return schemeAndAuthority + basePath + relativeUri;
    }

    std::string relativePath = relativeUri;
    std::string suffix;
    const std::string::size_type relativeFragmentPos = relativePath.find('#');
    const std::string::size_type relativeQueryPos = relativePath.find('?');
    const std::string::size_type suffixPos =
            relativeFragmentPos == std::string::npos ? relativeQueryPos :
            relativeQueryPos == std::string::npos ? relativeFragmentPos :
            std::min(relativeFragmentPos, relativeQueryPos);
    if (suffixPos != std::string::npos) {
        suffix = relativePath.substr(suffixPos);
        relativePath.erase(suffixPos);
    }

    std::string mergedPath;
    if (!relativePath.empty() && relativePath[0] == '/') {
        mergedPath = relativePath;
    } else {
        const std::string::size_type lastSlashPos = basePath.find_last_of('/');
        mergedPath = lastSlashPos == std::string::npos
                ? relativePath
                : basePath.substr(0, lastSlashPos + 1) + relativePath;
    }

    const bool absolutePath = !mergedPath.empty() && mergedPath[0] == '/';
    std::vector<std::string> segments;
    std::string segment;
    for (const char character : mergedPath) {
        if (character == '/') {
            if (segment == "..") {
                if (!segments.empty() && segments.back() != "..") {
                    segments.pop_back();
                } else if (!absolutePath) {
                    segments.push_back(segment);
                }
            } else if (!segment.empty() && segment != ".") {
                segments.push_back(segment);
            }
            segment.clear();
        } else {
            segment += character;
        }
    }

    if (segment == "..") {
        if (!segments.empty() && segments.back() != "..") {
            segments.pop_back();
        } else if (!absolutePath) {
            segments.push_back(segment);
        }
    } else if (!segment.empty() && segment != ".") {
        segments.push_back(segment);
    }

    std::string normalisedPath = absolutePath ? "/" : "";
    for (std::vector<std::string>::const_iterator iterator = segments.begin();
            iterator != segments.end(); ++iterator) {
        if (iterator != segments.begin()) {
            normalisedPath += "/";
        }
        normalisedPath += *iterator;
    }

    if (!mergedPath.empty() && mergedPath[mergedPath.size() - 1U] == '/' &&
            (normalisedPath.empty() ||
             normalisedPath[normalisedPath.size() - 1U] != '/')) {
        normalisedPath += "/";
    }

    return schemeAndAuthority + normalisedPath + suffix;
}

} // namespace uri
} // namespace internal
} // namespace valijson
