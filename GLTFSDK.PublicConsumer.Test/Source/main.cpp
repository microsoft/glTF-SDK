// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/ExtrasDocument.h>
#include <GLTFSDK/Schema.h>
#include <GLTFSDK/SchemaValidation.h>
#include <GLTFSDK/Serialize.h>

#include <string>

int main()
{
    using namespace Microsoft::glTF;

    const std::string input = R"({"asset":{"version":"2.0"}})";
    const Document document = Deserialize(input);
    const std::string serialized = Serialize(document);

    ValidateDocumentAgainstSchema(
        serialized,
        SCHEMA_URI_GLTF,
        GetDefaultSchemaLocator(SchemaFlags::None));

    ExtrasDocument extras;
    extras.SetMemberValue("value", 7);

    return document.asset.version == "2.0" &&
        extras.HasMember("value") &&
        extras.GetMemberValueOrDefault<int>("value") == 7 &&
        extras.ToJson() == R"({"value":7})"
        ? 0
        : 1;
}
