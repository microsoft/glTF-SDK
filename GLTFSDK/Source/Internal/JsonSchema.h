#pragma once

#include "Json.h"

#include <memory>
#include <string>

namespace Microsoft
{
    namespace glTF
    {
        class ISchemaLocator;

        namespace Internal
        {
            class Draft4ValidationSession
            {
            public:
                Draft4ValidationSession(
                    std::unique_ptr<const ISchemaLocator> schemaLocator,
                    std::string rootSchemaUri);
                ~Draft4ValidationSession();

                Draft4ValidationSession(
                    const Draft4ValidationSession&) = delete;
                Draft4ValidationSession& operator=(
                    const Draft4ValidationSession&) = delete;

                void Validate(const JsonValue& document);

            private:
                struct Impl;
                std::unique_ptr<Impl> m_impl;
            };

            void ValidateJsonAgainstSchema(
                const JsonValue& document,
                const std::string& schemaUri,
                std::unique_ptr<const ISchemaLocator> schemaLocator);
        }
    }
}
