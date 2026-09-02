#pragma once

#include <nlohmann/json.hpp>

#include <cstddef>
#include <cstdint>
#include <istream>
#include <string>
#include <vector>

namespace Microsoft
{
    namespace glTF
    {
        namespace Internal
        {
            using JsonValue = nlohmann::ordered_json;

            constexpr std::size_t MaxJsonNestingDepth = 256U;

            enum class JsonCategory
            {
                Null,
                Boolean,
                Number,
                String,
                Object,
                Array
            };

            const char* GetJsonLibraryVersion();

            JsonValue ParseJson(
                const std::string& bytes,
                bool ignoreByteOrderMark = false);
            JsonValue ParseJson(
                std::istream& stream,
                bool ignoreByteOrderMark = false);

            bool IsJsonNull(const JsonValue& value);
            bool IsJsonObject(const JsonValue& value);
            bool IsJsonArray(const JsonValue& value);
            bool IsJsonBoolean(const JsonValue& value);
            bool IsJsonString(const JsonValue& value);
            bool IsJsonNumber(const JsonValue& value);
            bool IsJsonSignedInteger(const JsonValue& value);
            bool IsJsonUnsignedInteger(const JsonValue& value);
            bool IsJsonFloatingPoint(const JsonValue& value);

            JsonCategory GetJsonCategory(const JsonValue& value);
            bool AreJsonAssignmentCategoriesCompatible(
                const JsonValue& currentValue,
                const JsonValue& replacementValue);

            void RequireJsonObject(
                const JsonValue& value,
                const std::string& error);
            void RequireJsonArray(
                const JsonValue& value,
                const std::string& error);

            const JsonValue* FindJsonMember(
                const JsonValue& object,
                const std::string& name);
            JsonValue* FindJsonMember(
                JsonValue& object,
                const std::string& name);
            const JsonValue& RequireJsonMember(
                const JsonValue& object,
                const std::string& name,
                const std::string& error);
            std::vector<std::string> GetJsonObjectMemberNames(
                const JsonValue& object,
                const std::string& error);

            std::size_t GetJsonArraySize(const JsonValue& array);
            const JsonValue& GetJsonArrayElement(
                const JsonValue& array,
                std::size_t index,
                const std::string& error);
            JsonValue& GetJsonArrayElement(
                JsonValue& array,
                std::size_t index,
                const std::string& error);

            bool TryGetJsonBoolean(const JsonValue& value, bool& result);
            bool TryGetJsonInt32(const JsonValue& value, std::int32_t& result);
            bool TryGetJsonUInt32(const JsonValue& value, std::uint32_t& result);
            bool TryGetJsonInt64(const JsonValue& value, std::int64_t& result);
            bool TryGetJsonUInt64(const JsonValue& value, std::uint64_t& result);
            bool TryGetJsonSize(const JsonValue& value, std::size_t& result);
            bool TryGetJsonFloat(const JsonValue& value, float& result);
            bool TryGetJsonDouble(const JsonValue& value, double& result);
            bool TryGetJsonString(const JsonValue& value, std::string& result);

            JsonValue CreateJsonNull();
            JsonValue CreateJsonObject();
            JsonValue CreateJsonArray();
            JsonValue CreateJsonBoolean(bool value);
            JsonValue CreateJsonInt32(std::int32_t value);
            JsonValue CreateJsonUInt32(std::uint32_t value);
            JsonValue CreateJsonInt64(std::int64_t value);
            JsonValue CreateJsonUInt64(std::uint64_t value);
            JsonValue CreateJsonSize(std::size_t value);
            JsonValue CreateJsonFloat(float value);
            JsonValue CreateJsonDouble(double value);
            JsonValue CreateJsonString(const std::string& value);
            JsonValue CreateJsonString(const char* value);

            void SetJsonMember(
                JsonValue& object,
                const std::string& name,
                JsonValue value);
            void AppendJsonValue(JsonValue& array, JsonValue value);

            std::size_t GetJsonNestingDepth(const JsonValue& value);
            void ValidateJsonNestingDepth(const JsonValue& value);

            std::string WriteJson(const JsonValue& value, bool pretty = false);

            std::vector<std::string> TokenizeJsonPointer(
                const std::string& pointer);
            const JsonValue* FindJsonPointer(
                const JsonValue& root,
                const std::string& pointer);
            JsonValue* FindJsonPointer(
                JsonValue& root,
                const std::string& pointer);
            void SetJsonPointer(
                JsonValue& root,
                const std::string& pointer,
                JsonValue value);
        }
    }
}
