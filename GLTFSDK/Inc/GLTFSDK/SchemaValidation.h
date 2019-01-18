// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/Schema.h>
#include <GLTFSDK/RapidJsonUtils.h>

namespace Microsoft
{
    namespace glTF
    {
        namespace Schema
        {
            void ValidateDocument(const rapidjson::Document& d, SchemaLocatorPtr schemaLocator = {}, SchemaFlags schemaFlags = SchemaFlags::None);
        }
    }
}
