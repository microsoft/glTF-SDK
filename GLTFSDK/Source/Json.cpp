// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "Internal/Json.h"

#include <GLTFSDK/Exceptions.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iterator>
#include <limits>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{
    static_assert(
        NLOHMANN_JSON_VERSION_MAJOR == 3,
        "glTF-SDK requires nlohmann/json 3.12.0");
    static_assert(
        NLOHMANN_JSON_VERSION_MINOR == 12,
        "glTF-SDK requires nlohmann/json 3.12.0");
    static_assert(
        NLOHMANN_JSON_VERSION_PATCH == 0,
        "glTF-SDK requires nlohmann/json 3.12.0");

    using JsonValue = Microsoft::glTF::Internal::JsonValue;

    bool StartsWithUtf8Bom(const std::string& bytes, std::size_t offset = 0U)
    {
        return bytes.size() >= offset + 3U &&
            static_cast<unsigned char>(bytes[offset]) == 0xEFU &&
            static_cast<unsigned char>(bytes[offset + 1U]) == 0xBBU &&
            static_cast<unsigned char>(bytes[offset + 2U]) == 0xBFU;
    }

    bool StartsWithUnsupportedBom(const std::string& bytes)
    {
        if (bytes.size() >= 4U)
        {
            const auto first = static_cast<unsigned char>(bytes[0]);
            const auto second = static_cast<unsigned char>(bytes[1]);
            const auto third = static_cast<unsigned char>(bytes[2]);
            const auto fourth = static_cast<unsigned char>(bytes[3]);

            if ((first == 0x00U && second == 0x00U &&
                 third == 0xFEU && fourth == 0xFFU) ||
                (first == 0xFFU && second == 0xFEU &&
                 third == 0x00U && fourth == 0x00U))
            {
                return true;
            }
        }

        return bytes.size() >= 2U &&
            ((static_cast<unsigned char>(bytes[0]) == 0xFEU &&
              static_cast<unsigned char>(bytes[1]) == 0xFFU) ||
             (static_cast<unsigned char>(bytes[0]) == 0xFFU &&
              static_cast<unsigned char>(bytes[1]) == 0xFEU));
    }

    class StrictParseCallback
    {
    public:
        bool operator()(
            int depth,
            JsonValue::parse_event_t event,
            JsonValue& parsed)
        {
            if (event == JsonValue::parse_event_t::object_start ||
                event == JsonValue::parse_event_t::array_start)
            {
                const auto level = static_cast<std::size_t>(depth) + 1U;
                if (level > Microsoft::glTF::Internal::MaxJsonNestingDepth)
                {
                    throw Microsoft::glTF::GLTFException(
                        "The document exceeds the maximum JSON nesting depth "
                        "of 256");
                }

                if (event == JsonValue::parse_event_t::object_start)
                {
                    m_objectKeys.emplace_back();
                }
            }
            else if (event == JsonValue::parse_event_t::key)
            {
                const auto& key =
                    parsed.get_ref<const JsonValue::string_t&>();
                if (m_objectKeys.empty() ||
                    !m_objectKeys.back().insert(key).second)
                {
                    throw Microsoft::glTF::GLTFException(
                        "The document contains a duplicate object member: " +
                        key);
                }
            }
            else if (event == JsonValue::parse_event_t::object_end)
            {
                m_objectKeys.pop_back();
            }

            return true;
        }

    private:
        std::vector<std::unordered_set<std::string>> m_objectKeys;
    };

    template<typename Destination>
    bool TryGetSignedInteger(const JsonValue& value, Destination& result)
    {
        static_assert(
            std::numeric_limits<Destination>::is_signed,
            "Destination must be signed");

        if (value.is_number_unsigned())
        {
            const auto source =
                value.get_ref<const JsonValue::number_unsigned_t&>();
            if (source >
                static_cast<JsonValue::number_unsigned_t>(
                    std::numeric_limits<Destination>::max()))
            {
                return false;
            }

            result = static_cast<Destination>(source);
            return true;
        }

        if (!value.is_number_integer())
        {
            return false;
        }

        const auto source =
            value.get_ref<const JsonValue::number_integer_t&>();
        if (source < static_cast<JsonValue::number_integer_t>(
                         std::numeric_limits<Destination>::min()) ||
            source > static_cast<JsonValue::number_integer_t>(
                         std::numeric_limits<Destination>::max()))
        {
            return false;
        }

        result = static_cast<Destination>(source);
        return true;
    }

    template<typename Destination>
    bool TryGetUnsignedInteger(const JsonValue& value, Destination& result)
    {
        static_assert(
            !std::numeric_limits<Destination>::is_signed,
            "Destination must be unsigned");

        JsonValue::number_unsigned_t source = 0U;
        if (value.is_number_unsigned())
        {
            source = value.get_ref<const JsonValue::number_unsigned_t&>();
        }
        else if (value.is_number_integer())
        {
            const auto signedSource =
                value.get_ref<const JsonValue::number_integer_t&>();
            if (signedSource < 0)
            {
                return false;
            }
            source = static_cast<JsonValue::number_unsigned_t>(signedSource);
        }
        else
        {
            return false;
        }

        if (source >
            static_cast<JsonValue::number_unsigned_t>(
                std::numeric_limits<Destination>::max()))
        {
            return false;
        }

        result = static_cast<Destination>(source);
        return true;
    }

    long double GetNumericValue(const JsonValue& value)
    {
        if (value.is_number_unsigned())
        {
            return static_cast<long double>(
                value.get_ref<const JsonValue::number_unsigned_t&>());
        }
        if (value.is_number_integer())
        {
            return static_cast<long double>(
                value.get_ref<const JsonValue::number_integer_t&>());
        }
        return static_cast<long double>(
            value.get_ref<const JsonValue::number_float_t&>());
    }

    void ValidateSerializableJson(const JsonValue& value)
    {
        Microsoft::glTF::Internal::ValidateJsonNestingDepth(value);

        std::vector<const JsonValue*> pending;
        pending.push_back(&value);
        while (!pending.empty())
        {
            const JsonValue* current = pending.back();
            pending.pop_back();

            if (current->is_number_float() &&
                !std::isfinite(
                    current->get_ref<const JsonValue::number_float_t&>()))
            {
                throw Microsoft::glTF::GLTFException(
                    "Cannot serialize a non-finite JSON number");
            }

            if (current->is_structured())
            {
                for (auto iterator = current->begin();
                     iterator != current->end();
                     ++iterator)
                {
                    pending.push_back(&*iterator);
                }
            }
        }
    }

    void NormalizeNumberExponents(std::string& json)
    {
        bool inString = false;
        bool escaped = false;

        for (std::size_t index = 0U; index < json.size(); ++index)
        {
            const char character = json[index];
            if (inString)
            {
                if (escaped)
                {
                    escaped = false;
                }
                else if (character == '\\')
                {
                    escaped = true;
                }
                else if (character == '"')
                {
                    inString = false;
                }
                continue;
            }

            if (character == '"')
            {
                inString = true;
                continue;
            }

            if (character != 'e' && character != 'E')
            {
                continue;
            }

            std::size_t digitStart = index + 1U;
            if (digitStart < json.size() &&
                (json[digitStart] == '+' || json[digitStart] == '-'))
            {
                ++digitStart;
            }

            std::size_t firstNonZero = digitStart;
            while (firstNonZero < json.size() &&
                   json[firstNonZero] == '0')
            {
                ++firstNonZero;
            }

            if (firstNonZero > digitStart &&
                firstNonZero < json.size() &&
                std::isdigit(
                    static_cast<unsigned char>(json[firstNonZero])) != 0)
            {
                json.erase(digitStart, firstNonZero - digitStart);
            }
            else if (firstNonZero > digitStart + 1U)
            {
                json.erase(
                    digitStart,
                    firstNonZero - digitStart - 1U);
            }
        }
    }

    std::string DecodeJsonPointerToken(const std::string& token)
    {
        std::string decoded;
        decoded.reserve(token.size());

        for (std::size_t index = 0U; index < token.size(); ++index)
        {
            if (token[index] != '~')
            {
                decoded.push_back(token[index]);
                continue;
            }

            if (++index >= token.size())
            {
                throw Microsoft::glTF::GLTFException(
                    "Invalid JSON Pointer escape sequence");
            }

            if (token[index] == '0')
            {
                decoded.push_back('~');
            }
            else if (token[index] == '1')
            {
                decoded.push_back('/');
            }
            else
            {
                throw Microsoft::glTF::GLTFException(
                    "Invalid JSON Pointer escape sequence");
            }
        }

        return decoded;
    }

    bool TryParseCanonicalArrayIndex(
        const std::string& token,
        std::size_t& result)
    {
        if (token.empty())
        {
            return false;
        }
        if (token == "0")
        {
            result = 0U;
            return true;
        }
        if (token[0] < '1' || token[0] > '9')
        {
            return false;
        }

        std::size_t value = 0U;
        for (const char character : token)
        {
            if (character < '0' || character > '9')
            {
                return false;
            }

            const std::size_t digit =
                static_cast<std::size_t>(character - '0');
            if (value >
                (std::numeric_limits<std::size_t>::max() - digit) / 10U)
            {
                throw Microsoft::glTF::GLTFException(
                    "JSON Pointer array index is out of range");
            }
            value = value * 10U + digit;
        }

        result = value;
        return true;
    }

    std::size_t ParseArrayIndex(const std::string& token)
    {
        std::size_t result = 0U;
        if (!TryParseCanonicalArrayIndex(token, result))
        {
            throw Microsoft::glTF::GLTFException(
                "JSON Pointer array index is not canonical");
        }
        return result;
    }

    JsonValue CreateContainerForToken(const std::string& token)
    {
        std::size_t unused = 0U;
        return TryParseCanonicalArrayIndex(token, unused)
            ? JsonValue::array()
            : JsonValue::object();
    }

    void AssignCompatibleJsonValue(JsonValue& target, JsonValue value)
    {
        if (!Microsoft::glTF::Internal::
                AreJsonAssignmentCategoriesCompatible(target, value))
        {
            throw Microsoft::glTF::GLTFException(
                "JSON value cannot change compatibility category");
        }
        target = std::move(value);
    }

    template<typename ValueType>
    ValueType* FindJsonPointerImpl(
        ValueType& root,
        const std::vector<std::string>& tokens)
    {
        ValueType* current = &root;
        for (const auto& token : tokens)
        {
            if (current->is_object())
            {
                const auto member = current->find(token);
                if (member == current->end())
                {
                    return nullptr;
                }
                current = &member.value();
            }
            else if (current->is_array())
            {
                const std::size_t index = ParseArrayIndex(token);
                if (index >= current->size())
                {
                    return nullptr;
                }
                current = &(*current)[index];
            }
            else
            {
                throw Microsoft::glTF::GLTFException(
                    "JSON Pointer cannot traverse a scalar value");
            }
        }

        return current;
    }

    void SetJsonPointerImpl(
        JsonValue& root,
        const std::vector<std::string>& tokens,
        JsonValue value)
    {
        if (tokens.empty())
        {
            AssignCompatibleJsonValue(root, std::move(value));
            return;
        }

        if (root.is_null())
        {
            root = CreateContainerForToken(tokens.front());
        }
        else if (!root.is_structured())
        {
            throw Microsoft::glTF::GLTFException(
                "JSON Pointer cannot traverse a scalar value");
        }

        JsonValue* current = &root;
        for (std::size_t tokenIndex = 0U;
             tokenIndex < tokens.size();
             ++tokenIndex)
        {
            const bool finalToken = tokenIndex + 1U == tokens.size();
            const std::string& token = tokens[tokenIndex];

            if (current->is_object())
            {
                auto member = current->find(token);
                if (member == current->end())
                {
                    Microsoft::glTF::Internal::SetJsonMember(
                        *current,
                        token,
                        finalToken
                            ? std::move(value)
                            : CreateContainerForToken(
                                tokens[tokenIndex + 1U]));
                    member = current->find(token);
                }
                else if (finalToken)
                {
                    AssignCompatibleJsonValue(
                        member.value(), std::move(value));
                }

                if (finalToken)
                {
                    return;
                }

                JsonValue& next = member.value();
                if (next.is_null())
                {
                    next = CreateContainerForToken(
                        tokens[tokenIndex + 1U]);
                }
                else if (!next.is_structured())
                {
                    throw Microsoft::glTF::GLTFException(
                        "JSON Pointer cannot traverse a scalar value");
                }
                current = &next;
            }
            else if (current->is_array())
            {
                const std::size_t index = ParseArrayIndex(token);
                if (index >= current->max_size())
                {
                    throw Microsoft::glTF::GLTFException(
                        "JSON Pointer array index is out of range");
                }
                while (current->size() <= index)
                {
                    current->push_back(JsonValue());
                }

                JsonValue& next = (*current)[index];
                if (finalToken)
                {
                    AssignCompatibleJsonValue(next, std::move(value));
                    return;
                }

                if (next.is_null())
                {
                    next = CreateContainerForToken(
                        tokens[tokenIndex + 1U]);
                }
                else if (!next.is_structured())
                {
                    throw Microsoft::glTF::GLTFException(
                        "JSON Pointer cannot traverse a scalar value");
                }
                current = &next;
            }
            else
            {
                throw Microsoft::glTF::GLTFException(
                    "JSON Pointer cannot traverse a scalar value");
            }
        }
    }
}

const char* Microsoft::glTF::Internal::GetJsonLibraryVersion()
{
    return "3.12.0";
}

Microsoft::glTF::Internal::JsonValue
Microsoft::glTF::Internal::ParseJson(
    const std::string& bytes,
    bool ignoreByteOrderMark)
{
    if (StartsWithUnsupportedBom(bytes))
    {
        throw GLTFException("The document is not UTF-8 JSON");
    }

    std::size_t offset = 0U;
    if (StartsWithUtf8Bom(bytes))
    {
        if (!ignoreByteOrderMark)
        {
            throw GLTFException(
                "The document contains a UTF-8 byte order mark");
        }
        offset = 3U;
        if (StartsWithUtf8Bom(bytes, offset))
        {
            throw GLTFException(
                "The document contains more than one UTF-8 byte order mark");
        }
    }

    try
    {
        StrictParseCallback callback;
        return JsonValue::parse(
            bytes.begin() + static_cast<std::ptrdiff_t>(offset),
            bytes.end(),
            callback,
            true,
            false);
    }
    catch (const GLTFException&)
    {
        throw;
    }
    catch (const JsonValue::exception&)
    {
        throw GLTFException(
            "The document is invalid due to bad JSON formatting");
    }
}

Microsoft::glTF::Internal::JsonValue
Microsoft::glTF::Internal::ParseJson(
    std::istream& stream,
    bool ignoreByteOrderMark)
{
    if (!stream.good() && !stream.eof())
    {
        throw GLTFException("The JSON input stream is not readable");
    }

    const std::string bytes{
        std::istreambuf_iterator<char>(stream),
        std::istreambuf_iterator<char>()};

    if (stream.bad() || (stream.fail() && !stream.eof()))
    {
        throw GLTFException("Failed to read the JSON input stream");
    }

    return ParseJson(bytes, ignoreByteOrderMark);
}

bool Microsoft::glTF::Internal::IsJsonNull(const JsonValue& value)
{
    return value.is_null();
}

bool Microsoft::glTF::Internal::IsJsonObject(const JsonValue& value)
{
    return value.is_object();
}

bool Microsoft::glTF::Internal::IsJsonArray(const JsonValue& value)
{
    return value.is_array();
}

bool Microsoft::glTF::Internal::IsJsonBoolean(const JsonValue& value)
{
    return value.is_boolean();
}

bool Microsoft::glTF::Internal::IsJsonString(const JsonValue& value)
{
    return value.is_string();
}

bool Microsoft::glTF::Internal::IsJsonNumber(const JsonValue& value)
{
    return value.is_number();
}

bool Microsoft::glTF::Internal::IsJsonSignedInteger(const JsonValue& value)
{
    return value.is_number_integer() && !value.is_number_unsigned();
}

bool Microsoft::glTF::Internal::IsJsonUnsignedInteger(const JsonValue& value)
{
    return value.is_number_unsigned();
}

bool Microsoft::glTF::Internal::IsJsonFloatingPoint(const JsonValue& value)
{
    return value.is_number_float();
}

Microsoft::glTF::Internal::JsonCategory
Microsoft::glTF::Internal::GetJsonCategory(const JsonValue& value)
{
    if (value.is_null())
    {
        return JsonCategory::Null;
    }
    if (value.is_boolean())
    {
        return JsonCategory::Boolean;
    }
    if (value.is_number())
    {
        return JsonCategory::Number;
    }
    if (value.is_string())
    {
        return JsonCategory::String;
    }
    if (value.is_object())
    {
        return JsonCategory::Object;
    }
    return JsonCategory::Array;
}

bool Microsoft::glTF::Internal::AreJsonAssignmentCategoriesCompatible(
    const JsonValue& currentValue,
    const JsonValue& replacementValue)
{
    return currentValue.is_null() ||
        GetJsonCategory(currentValue) == GetJsonCategory(replacementValue);
}

void Microsoft::glTF::Internal::RequireJsonObject(
    const JsonValue& value,
    const std::string& error)
{
    if (!value.is_object())
    {
        throw InvalidGLTFException(error);
    }
}

void Microsoft::glTF::Internal::RequireJsonArray(
    const JsonValue& value,
    const std::string& error)
{
    if (!value.is_array())
    {
        throw InvalidGLTFException(error);
    }
}

const Microsoft::glTF::Internal::JsonValue*
Microsoft::glTF::Internal::FindJsonMember(
    const JsonValue& object,
    const std::string& name)
{
    RequireJsonObject(object, "JSON value must be an object");
    const auto member = object.find(name);
    return member == object.end() ? nullptr : &member.value();
}

Microsoft::glTF::Internal::JsonValue*
Microsoft::glTF::Internal::FindJsonMember(
    JsonValue& object,
    const std::string& name)
{
    RequireJsonObject(object, "JSON value must be an object");
    const auto member = object.find(name);
    return member == object.end() ? nullptr : &member.value();
}

const Microsoft::glTF::Internal::JsonValue&
Microsoft::glTF::Internal::RequireJsonMember(
    const JsonValue& object,
    const std::string& name,
    const std::string& error)
{
    const auto* member = FindJsonMember(object, name);
    if (member == nullptr)
    {
        throw InvalidGLTFException(error);
    }
    return *member;
}

std::vector<std::string>
Microsoft::glTF::Internal::GetJsonObjectMemberNames(
    const JsonValue& object,
    const std::string& error)
{
    RequireJsonObject(object, error);
    std::vector<std::string> names;
    names.reserve(object.size());
    for (auto iterator = object.begin();
         iterator != object.end();
         ++iterator)
    {
        names.push_back(iterator.key());
    }
    return names;
}

std::size_t Microsoft::glTF::Internal::GetJsonArraySize(
    const JsonValue& array)
{
    RequireJsonArray(array, "JSON value must be an array");
    return array.size();
}

const Microsoft::glTF::Internal::JsonValue&
Microsoft::glTF::Internal::GetJsonArrayElement(
    const JsonValue& array,
    std::size_t index,
    const std::string& error)
{
    RequireJsonArray(array, error);
    if (index >= array.size())
    {
        throw InvalidGLTFException(error);
    }
    return array[index];
}

Microsoft::glTF::Internal::JsonValue&
Microsoft::glTF::Internal::GetJsonArrayElement(
    JsonValue& array,
    std::size_t index,
    const std::string& error)
{
    RequireJsonArray(array, error);
    if (index >= array.size())
    {
        throw InvalidGLTFException(error);
    }
    return array[index];
}

bool Microsoft::glTF::Internal::TryGetJsonBoolean(
    const JsonValue& value,
    bool& result)
{
    if (!value.is_boolean())
    {
        return false;
    }
    result = value.get_ref<const JsonValue::boolean_t&>();
    return true;
}

bool Microsoft::glTF::Internal::TryGetJsonInt32(
    const JsonValue& value,
    std::int32_t& result)
{
    return TryGetSignedInteger(value, result);
}

bool Microsoft::glTF::Internal::TryGetJsonUInt32(
    const JsonValue& value,
    std::uint32_t& result)
{
    return TryGetUnsignedInteger(value, result);
}

bool Microsoft::glTF::Internal::TryGetJsonInt64(
    const JsonValue& value,
    std::int64_t& result)
{
    return TryGetSignedInteger(value, result);
}

bool Microsoft::glTF::Internal::TryGetJsonUInt64(
    const JsonValue& value,
    std::uint64_t& result)
{
    return TryGetUnsignedInteger(value, result);
}

bool Microsoft::glTF::Internal::TryGetJsonSize(
    const JsonValue& value,
    std::size_t& result)
{
    return TryGetUnsignedInteger(value, result);
}

bool Microsoft::glTF::Internal::TryGetJsonFloat(
    const JsonValue& value,
    float& result)
{
    if (!value.is_number())
    {
        return false;
    }

    const long double source = GetNumericValue(value);
    if (!std::isfinite(source))
    {
        return false;
    }

    const float converted = static_cast<float>(source);
    if (!std::isfinite(converted) ||
        (source != 0.0L && converted == 0.0F))
    {
        return false;
    }

    result = converted;
    return true;
}

bool Microsoft::glTF::Internal::TryGetJsonDouble(
    const JsonValue& value,
    double& result)
{
    if (!value.is_number())
    {
        return false;
    }

    const long double source = GetNumericValue(value);
    const double converted = static_cast<double>(source);
    if (!std::isfinite(source) || !std::isfinite(converted))
    {
        return false;
    }

    result = converted;
    return true;
}

bool Microsoft::glTF::Internal::TryGetJsonString(
    const JsonValue& value,
    std::string& result)
{
    if (!value.is_string())
    {
        return false;
    }
    result = value.get_ref<const JsonValue::string_t&>();
    return true;
}

Microsoft::glTF::Internal::JsonValue
Microsoft::glTF::Internal::CreateJsonNull()
{
    return JsonValue();
}

Microsoft::glTF::Internal::JsonValue
Microsoft::glTF::Internal::CreateJsonObject()
{
    return JsonValue::object();
}

Microsoft::glTF::Internal::JsonValue
Microsoft::glTF::Internal::CreateJsonArray()
{
    return JsonValue::array();
}

Microsoft::glTF::Internal::JsonValue
Microsoft::glTF::Internal::CreateJsonBoolean(bool value)
{
    return JsonValue(value);
}

Microsoft::glTF::Internal::JsonValue
Microsoft::glTF::Internal::CreateJsonInt32(std::int32_t value)
{
    return JsonValue(static_cast<JsonValue::number_integer_t>(value));
}

Microsoft::glTF::Internal::JsonValue
Microsoft::glTF::Internal::CreateJsonUInt32(std::uint32_t value)
{
    return JsonValue(static_cast<JsonValue::number_unsigned_t>(value));
}

Microsoft::glTF::Internal::JsonValue
Microsoft::glTF::Internal::CreateJsonInt64(std::int64_t value)
{
    return JsonValue(static_cast<JsonValue::number_integer_t>(value));
}

Microsoft::glTF::Internal::JsonValue
Microsoft::glTF::Internal::CreateJsonUInt64(std::uint64_t value)
{
    return JsonValue(static_cast<JsonValue::number_unsigned_t>(value));
}

Microsoft::glTF::Internal::JsonValue
Microsoft::glTF::Internal::CreateJsonSize(std::size_t value)
{
    return JsonValue(static_cast<JsonValue::number_unsigned_t>(value));
}

Microsoft::glTF::Internal::JsonValue
Microsoft::glTF::Internal::CreateJsonFloat(float value)
{
    if (!std::isfinite(value))
    {
        throw GLTFException("Cannot store a non-finite JSON number");
    }
    return JsonValue(static_cast<JsonValue::number_float_t>(value));
}

Microsoft::glTF::Internal::JsonValue
Microsoft::glTF::Internal::CreateJsonDouble(double value)
{
    if (!std::isfinite(value))
    {
        throw GLTFException("Cannot store a non-finite JSON number");
    }
    return JsonValue(static_cast<JsonValue::number_float_t>(value));
}

Microsoft::glTF::Internal::JsonValue
Microsoft::glTF::Internal::CreateJsonString(const std::string& value)
{
    return JsonValue(value);
}

Microsoft::glTF::Internal::JsonValue
Microsoft::glTF::Internal::CreateJsonString(const char* value)
{
    if (value == nullptr)
    {
        throw GLTFException("Cannot store a null JSON string");
    }
    return JsonValue(value);
}

void Microsoft::glTF::Internal::SetJsonMember(
    JsonValue& object,
    const std::string& name,
    JsonValue value)
{
    RequireJsonObject(object, "JSON value must be an object");
    const auto member = object.find(name);
    if (member == object.end())
    {
        object.emplace(name, std::move(value));
    }
    else
    {
        member.value() = std::move(value);
    }
}

void Microsoft::glTF::Internal::AppendJsonValue(
    JsonValue& array,
    JsonValue value)
{
    RequireJsonArray(array, "JSON value must be an array");
    array.push_back(std::move(value));
}

std::size_t Microsoft::glTF::Internal::GetJsonNestingDepth(
    const JsonValue& value)
{
    if (!value.is_structured())
    {
        return 0U;
    }

    std::size_t maximumDepth = 1U;
    std::vector<std::pair<const JsonValue*, std::size_t>> pending;
    pending.emplace_back(&value, 1U);

    while (!pending.empty())
    {
        const auto current = pending.back();
        pending.pop_back();
        maximumDepth = std::max(maximumDepth, current.second);

        for (auto iterator = current.first->begin();
             iterator != current.first->end();
             ++iterator)
        {
            if (iterator->is_structured())
            {
                pending.emplace_back(
                    &*iterator,
                    current.second + 1U);
            }
        }
    }

    return maximumDepth;
}

void Microsoft::glTF::Internal::ValidateJsonNestingDepth(
    const JsonValue& value)
{
    if (GetJsonNestingDepth(value) > MaxJsonNestingDepth)
    {
        throw GLTFException(
            "The document exceeds the maximum JSON nesting depth of 256");
    }
}

std::string Microsoft::glTF::Internal::WriteJson(
    const JsonValue& value,
    bool pretty)
{
    try
    {
        ValidateSerializableJson(value);
        std::string result = value.dump(
            pretty ? 4 : -1,
            ' ',
            false,
            JsonValue::error_handler_t::strict);
        NormalizeNumberExponents(result);
        return result;
    }
    catch (const GLTFException&)
    {
        throw;
    }
    catch (const JsonValue::exception&)
    {
        throw GLTFException("Failed to serialize JSON document");
    }
}

std::vector<std::string>
Microsoft::glTF::Internal::TokenizeJsonPointer(
    const std::string& pointer)
{
    std::vector<std::string> tokens;
    if (pointer.empty())
    {
        return tokens;
    }
    if (pointer[0] != '/')
    {
        throw GLTFException(
            "JSON Pointer must be empty or begin with '/'");
    }

    std::size_t tokenStart = 1U;
    while (tokenStart <= pointer.size())
    {
        const std::size_t delimiter = pointer.find('/', tokenStart);
        const std::size_t tokenEnd =
            delimiter == std::string::npos ? pointer.size() : delimiter;
        tokens.push_back(DecodeJsonPointerToken(
            pointer.substr(tokenStart, tokenEnd - tokenStart)));

        if (delimiter == std::string::npos)
        {
            break;
        }
        tokenStart = delimiter + 1U;
    }

    return tokens;
}

const Microsoft::glTF::Internal::JsonValue*
Microsoft::glTF::Internal::FindJsonPointer(
    const JsonValue& root,
    const std::string& pointer)
{
    return FindJsonPointerImpl(root, TokenizeJsonPointer(pointer));
}

Microsoft::glTF::Internal::JsonValue*
Microsoft::glTF::Internal::FindJsonPointer(
    JsonValue& root,
    const std::string& pointer)
{
    return FindJsonPointerImpl(root, TokenizeJsonPointer(pointer));
}

void Microsoft::glTF::Internal::SetJsonPointer(
    JsonValue& root,
    const std::string& pointer,
    JsonValue value)
{
    const auto tokens = TokenizeJsonPointer(pointer);

    try
    {
        JsonValue candidate = root;
        SetJsonPointerImpl(candidate, tokens, std::move(value));
        ValidateJsonNestingDepth(candidate);
        root = std::move(candidate);
    }
    catch (const GLTFException&)
    {
        throw;
    }
    catch (const JsonValue::exception&)
    {
        throw GLTFException("Failed to update JSON Pointer");
    }
}
