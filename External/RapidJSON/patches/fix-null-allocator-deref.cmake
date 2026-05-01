# fix-null-allocator-deref.cmake
#
# Patches RapidJSON schema.h to fix CWE-476: null pointer dereference in
# EndMissingDependentProperties(). Replaces calls to
# GetInvalidSchemaPointer().GetAllocator() (which dereferences a null
# allocator_ pointer) with GetStateAllocator() (which is always initialized).
#
# Usage: cmake -DSOURCE_DIR=<path> -P fix-null-allocator-deref.cmake

if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR must be defined")
endif()

set(SCHEMA_FILE "${SOURCE_DIR}/include/rapidjson/schema.h")

if(NOT EXISTS "${SCHEMA_FILE}")
    message(FATAL_ERROR "schema.h not found at ${SCHEMA_FILE}")
endif()

file(READ "${SCHEMA_FILE}" content)

string(FIND "${content}" "&GetInvalidSchemaPointer().GetAllocator()" match_pos)
if(match_pos EQUAL -1)
    message(STATUS "Patch already applied or pattern not found in schema.h")
    return()
endif()

string(REPLACE
    "&GetInvalidSchemaPointer().GetAllocator()"
    "&GetStateAllocator()"
    content "${content}")

file(WRITE "${SCHEMA_FILE}" "${content}")
message(STATUS "Patched schema.h: replaced GetInvalidSchemaPointer().GetAllocator() with GetStateAllocator()")
