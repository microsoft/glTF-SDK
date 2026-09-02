// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <GLTFSDK/Definitions.h>

#include <memory>
#include <string>

namespace Microsoft
{
    namespace glTF
    {
        class ISchemaLocator
        {
        public:
            virtual ~ISchemaLocator() = default;
            virtual const char* GetSchemaContent(
                const std::string& uri) const = 0;
        };

        void GLTFSDK_API ValidateDocumentAgainstSchema(
            const std::string& documentJson,
            const std::string& schemaUri,
            std::unique_ptr<const ISchemaLocator> schemaLocator);
    }
}
