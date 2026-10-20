// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/RapidJsonUtils.h>

#include <algorithm>
#include <limits>
#include <new>
#include <stdexcept>

using namespace glTF::UnitTest;

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            namespace
            {
                class ReservationAllocator : public rapidjson::CrtAllocator
                {
                public:
                    bool observe = false;
                    size_t requestedSize = 0;

                    void* Realloc(void* original, size_t originalSize, size_t newSize)
                    {
                        if (observe)
                        {
                            requestedSize = newSize;
                            throw std::bad_alloc();
                        }
                        return rapidjson::CrtAllocator::Realloc(original, originalSize, newSize);
                    }
                };

                template<typename Encoding>
                void CheckReservations()
                {
                    using Ch = typename Encoding::Ch;
                    using Schema = rapidjson::GenericSchemaDocument<rapidjson::GenericValue<Encoding>>;
                    using Validator = rapidjson::GenericSchemaValidator<Schema, rapidjson::BaseReaderHandler<Encoding>, ReservationAllocator>;

                    rapidjson::GenericDocument<Encoding> document;
                    document.SetObject();
                    Schema schema(document);
                    const auto maxLength = (std::numeric_limits<rapidjson::SizeType>::max)();
                    const size_t maxTokenLength = (static_cast<size_t>((std::numeric_limits<std::ptrdiff_t>::max)()) / sizeof(Ch) - 1) / 2;
                    const auto boundary = static_cast<rapidjson::SizeType>((std::min)(maxTokenLength, static_cast<size_t>(maxLength - 1)));
                    const rapidjson::SizeType lengths[] = { 0, 1, boundary, boundary + 1, maxLength / 2, maxLength / 2 + 1, maxLength };

                    for (auto length : lengths)
                    {
                        ReservationAllocator allocator;
                        Validator validator(schema, &allocator, 1024, 0);
                        Assert::IsTrue(validator.StartObject());
                        allocator.observe = true;

                        // Observe the reservation without allocating or reading a large string.
                        const Ch key[] = { 0 };
                        const uint64_t expectedSize = (1 + static_cast<uint64_t>(length) * 2) * sizeof(Ch);
                        if (expectedSize > static_cast<uint64_t>((std::numeric_limits<std::ptrdiff_t>::max)()))
                        {
                            Assert::ExpectException<std::length_error>([&] { validator.Key(key, length, false); });
                            Assert::AreEqual(size_t(0), allocator.requestedSize);
                        }
                        else
                        {
                            Assert::ExpectException<std::bad_alloc>([&] { validator.Key(key, length, false); });
                            Assert::AreEqual(static_cast<size_t>(expectedSize), allocator.requestedSize);
                        }
                    }
                }
            }

            GLTFSDK_TEST_CLASS(SchemaValidationTests)
            {
                GLTFSDK_TEST_METHOD(SchemaValidationTests, MemberReservations)
                {
                    CheckReservations<rapidjson::UTF8<>>();
                }

                GLTFSDK_TEST_METHOD(SchemaValidationTests, WideMemberReservations)
                {
                    CheckReservations<rapidjson::UTF16<>>();
                }

                GLTFSDK_TEST_METHOD(SchemaValidationTests, EscapedMemberPaths)
                {
                    rapidjson::Document schemaJson;
                    schemaJson.Parse(R"({"type":"object","additionalProperties":{"type":"integer"}})");
                    rapidjson::SchemaDocument schema(schemaJson);

                    for (const std::string& key : { std::string(), std::string("field"), std::string("a~/"), std::string(512, '/') })
                    {
                        rapidjson::Document document(rapidjson::kObjectType);
                        rapidjson::Value name;
                        name.SetString(key.c_str(), static_cast<rapidjson::SizeType>(key.size()), document.GetAllocator());
                        document.AddMember(name, false, document.GetAllocator());

                        rapidjson::SchemaValidator validator(schema);
                        Assert::IsFalse(document.Accept(validator));
                        std::string expected = "/";
                        for (char ch : key)
                        {
                            expected += ch == '~' ? "~0" : ch == '/' ? "~1" : std::string(1, ch);
                        }
                        rapidjson::StringBuffer pointer;
                        validator.GetInvalidDocumentPointer().Stringify(pointer);
                        Assert::AreEqual(expected, std::string(pointer.GetString()));
                    }
                }

                GLTFSDK_TEST_METHOD(SchemaValidationTests, AdditionalMembers)
                {
                    auto document = Deserialize(R"({"asset":{"version":"2.0"},"":null,"field/~":{"nested":[1,2,3]}})");
                    Assert::AreEqual(std::string("2.0"), document.asset.version);
                }
            };
        }
    }
}
