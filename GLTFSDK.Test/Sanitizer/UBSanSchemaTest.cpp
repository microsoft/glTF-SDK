// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

// Standalone reproducer for CWE-476: null pointer dereference in RapidJSON
// schema validator's EndMissingDependentProperties().
//
// This test is compiled with Clang + UBSAN (-fsanitize=undefined) to detect
// undefined behavior that may not manifest as a crash on all platforms.
//
// The vulnerability: schema.h's EndMissingDependentProperties() calls
// GetInvalidSchemaPointer().GetAllocator() which dereferences a null
// allocator_ pointer when the schema pointer hasn't been initialized.
//
// This test creates a schema with a "dependencies" constraint and validates
// a document that violates it, exercising the vulnerable code path.

#include <cstdio>
#include <cstdlib>
#include <rapidjson/document.h>
#include <rapidjson/schema.h>

int main() {
    // Schema with a "dependencies" constraint: if "scene" is present,
    // "scenes" must also be present.
    const char* schemaJson = R"({
        "type": "object",
        "properties": {
            "scene": { "type": "integer" },
            "scenes": { "type": "array" }
        },
        "dependencies": {
            "scene": ["scenes"]
        }
    })";

    // Document that violates the dependency: has "scene" but NOT "scenes".
    const char* docJson = R"({ "scene": 0 })";

    rapidjson::Document schemaDoc;
    schemaDoc.Parse(schemaJson);
    if (schemaDoc.HasParseError()) {
        fprintf(stderr, "Schema parse error\n");
        return 1;
    }

    rapidjson::Document doc;
    doc.Parse(docJson);
    if (doc.HasParseError()) {
        fprintf(stderr, "Document parse error\n");
        return 1;
    }

    rapidjson::SchemaDocument schema(schemaDoc);
    rapidjson::SchemaValidator validator(schema);

    // This triggers EndMissingDependentProperties which, in unpatched code,
    // dereferences a null allocator pointer (undefined behavior).
    bool valid = doc.Accept(validator);

    if (!valid) {
        printf("Validation failed as expected: %s\n",
               validator.GetInvalidSchemaKeyword());
        printf("UBSAN test passed: no undefined behavior detected\n");
        return 0;
    }

    fprintf(stderr, "ERROR: Validation unexpectedly passed\n");
    return 1;
}
