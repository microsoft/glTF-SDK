// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/RapidJsonUtils.h>

#include <memory>

namespace Microsoft
{
    namespace glTF
    {
        class ISchemaLocator
        {
        public:
            virtual ~ISchemaLocator() = default;
            virtual const char* GetSchemaContent(const std::string& uri) const = 0;
        };

        void ValidateDocumentAgainstSchema(const rapidjson::Document& d, const std::string& schemaUri, std::unique_ptr<const ISchemaLocator> schemaLocator);
    }
}
