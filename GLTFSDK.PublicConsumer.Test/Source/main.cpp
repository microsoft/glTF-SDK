// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/ExtrasDocument.h>
#include <GLTFSDK/GLBResourceWriter.h>
#include <GLTFSDK/IStreamWriter.h>
#include <GLTFSDK/Schema.h>
#include <GLTFSDK/SchemaValidation.h>
#include <GLTFSDK/Serialize.h>

#include <memory>
#include <sstream>
#include <string>

namespace
{
    class MemoryStreamWriter final : public Microsoft::glTF::IStreamWriter
    {
    public:
        std::shared_ptr<std::ostream> GetOutputStream(
            const std::string&) const override
        {
            return std::make_shared<std::stringstream>();
        }
    };
}

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

    auto streamWriter = std::make_shared<const MemoryStreamWriter>();
    GLBResourceWriter glbWriter(streamWriter);
    std::stringstream glb(
        std::ios::in | std::ios::out | std::ios::binary);
    glbWriter.FlushStream(serialized, &glb);
    const std::string glbBytes = glb.str();

    return document.asset.version == "2.0" &&
        extras.HasMember("value") &&
        extras.GetMemberValueOrDefault<int>("value") == 7 &&
        extras.ToJson() == R"({"value":7})" &&
        glbBytes.size() >= 4U &&
        glbBytes.compare(0U, 4U, "glTF") == 0
        ? 0
        : 1;
}
