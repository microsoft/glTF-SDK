# fix-string-type-conversion.cmake
#
# Makes the pointer cast in RapidJSON's generic memory Stack explicit.
# Stack::Bottom<T>() returns the internal raw byte buffer (char* stack_)
# reinterpret_cast directly to T*. When T is a wider element type (e.g. a
# wide-character type) this is a narrow-to-wide pointer conversion that some
# compilers and static analysis tools diagnose as a potentially
# misaligned/narrowing cast.
#
# Routing the cast through void* makes the "treat this buffer as raw storage"
# intent explicit. It is address-identical and preserves the exact behavior of
# the generic memory stack, while avoiding the direct narrow-to-wide character
# pointer conversion diagnostic.
#
# Usage: cmake -DSOURCE_DIR=<path> -P fix-string-type-conversion.cmake

if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR must be defined")
endif()

set(STACK_FILE "${SOURCE_DIR}/include/rapidjson/internal/stack.h")

if(NOT EXISTS "${STACK_FILE}")
    message(FATAL_ERROR "stack.h not found at ${STACK_FILE}")
endif()

file(READ "${STACK_FILE}" content)

string(FIND "${content}" "reinterpret_cast<T*>(static_cast<void*>(stack_))" already_pos)
if(NOT already_pos EQUAL -1)
    message(STATUS "Patch already applied in stack.h")
    return()
endif()

string(FIND "${content}" "reinterpret_cast<T*>(stack_)" match_pos)
if(match_pos EQUAL -1)
    message(STATUS "Pattern not found in stack.h; nothing to patch")
    return()
endif()

string(REPLACE
    "reinterpret_cast<T*>(stack_)"
    "reinterpret_cast<T*>(static_cast<void*>(stack_))"
    content "${content}")

file(WRITE "${STACK_FILE}" "${content}")
message(STATUS "Patched stack.h: routed Stack::Bottom char* -> T* cast through void*")
