# Keep schema token reservations representable by the document stack.

if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR must be defined")
endif()

set(SCHEMA_FILE "${SOURCE_DIR}/include/rapidjson/schema.h")
if(NOT EXISTS "${SCHEMA_FILE}")
    message(FATAL_ERROR "schema.h not found at ${SCHEMA_FILE}")
endif()

file(READ "${SCHEMA_FILE}" content)

set(original [=[        documentStack_.template Reserve<Ch>(1 + len * 2);]=])
set(replacement [=[        const size_t available = (static_cast<size_t>((std::numeric_limits<std::ptrdiff_t>::max)()) - documentStack_.GetSize()) / sizeof(Ch);
        if (available == 0 || len > (available - 1) / 2)
            throw std::length_error("Schema document path is too long");
        documentStack_.template Reserve<Ch>(1 + static_cast<size_t>(len) * 2);]=])
set(original_include "#include <cmath> // abs, floor")
set(replacement_include "${original_include}\n#include <limits>\n#include <stdexcept>")

string(FIND "${content}" "${original}" original_pos)
string(FIND "${content}" "${replacement}" replacement_pos)
string(FIND "${content}" "${replacement_include}" replacement_include_pos)
if(original_pos EQUAL -1)
    if(NOT replacement_pos EQUAL -1 AND NOT replacement_include_pos EQUAL -1)
        message(STATUS "Schema reservation patch already applied")
        return()
    endif()
    message(FATAL_ERROR "Expected schema reservation not found in ${SCHEMA_FILE}")
endif()

string(FIND "${content}" "${original_include}" include_pos)
if(include_pos EQUAL -1)
    message(FATAL_ERROR "Expected schema include not found in ${SCHEMA_FILE}")
endif()

string(REPLACE "${original}" "${replacement}" content "${content}")
if(replacement_include_pos EQUAL -1)
    string(REPLACE "${original_include}" "${replacement_include}" content "${content}")
endif()
file(WRITE "${SCHEMA_FILE}" "${content}")
message(STATUS "Applied schema reservation patch")
