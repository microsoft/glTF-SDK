# fix-writer-size-overflow.cmake
#
# Patches RapidJSON writer.h to fix CWE-190: 32-bit integer overflow in the
# reservation computed by Writer::WriteString.
#
# WriteString reserves output space with:
#     PutReserve(*os_, 2 + length * 6);   // "\uxxxx..."
#     PutReserve(*os_, 2 + length * 12);  // "\uxxxx\uyyyy..."
# where 'length' is a 32-bit rapidjson::SizeType. For very large strings the
# 'length * 6' / 'length * 12' multiplication wraps around in 32-bit arithmetic
# before it is widened to the size_t parameter of PutReserve, producing an
# undersized reservation. The subsequent PutUnsafe/PushUnsafe writes then run
# past the end of the allocation (heap out-of-bounds write).
#
# The reservation parameter of PutReserve is size_t, so forcing the whole
# expression to be evaluated in size_t makes the multiplication use the target
# pointer width (64-bit on the affected 64-bit builds), eliminating the wrap
# and yielding the true required size (a genuinely huge string then fails the
# allocation cleanly instead of silently under-reserving). The result is
# value-identical for all in-range lengths.
#
# Usage: cmake -DSOURCE_DIR=<path> -P fix-writer-size-overflow.cmake

if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR must be defined")
endif()

set(WRITER_FILE "${SOURCE_DIR}/include/rapidjson/writer.h")

if(NOT EXISTS "${WRITER_FILE}")
    message(FATAL_ERROR "writer.h not found at ${WRITER_FILE}")
endif()

file(READ "${WRITER_FILE}" content)

string(FIND "${content}" "static_cast<size_t>(2) + static_cast<size_t>(length) * 6" already_pos)
if(NOT already_pos EQUAL -1)
    message(STATUS "Patch already applied in writer.h")
    return()
endif()

set(patched FALSE)

if(NOT "${content}" MATCHES "PutReserve\\(\\*os_, 2 \\+ length \\* 6\\)")
    message(STATUS "Pattern 'length * 6' not found in writer.h")
else()
    string(REPLACE
        "PutReserve(*os_, 2 + length * 6)"
        "PutReserve(*os_, static_cast<size_t>(2) + static_cast<size_t>(length) * 6)"
        content "${content}")
    set(patched TRUE)
endif()

if(NOT "${content}" MATCHES "PutReserve\\(\\*os_, 2 \\+ length \\* 12\\)")
    message(STATUS "Pattern 'length * 12' not found in writer.h")
else()
    string(REPLACE
        "PutReserve(*os_, 2 + length * 12)"
        "PutReserve(*os_, static_cast<size_t>(2) + static_cast<size_t>(length) * 12)"
        content "${content}")
    set(patched TRUE)
endif()

if(patched)
    file(WRITE "${WRITER_FILE}" "${content}")
    message(STATUS "Patched writer.h: widened WriteString reserve computation to size_t")
else()
    message(STATUS "writer.h reserve patterns not found; nothing to patch")
endif()
