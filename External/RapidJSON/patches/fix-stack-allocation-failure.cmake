if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR must be defined")
endif()

set(STACK_FILE "${SOURCE_DIR}/include/rapidjson/internal/stack.h")
if(NOT EXISTS "${STACK_FILE}")
    message(FATAL_ERROR "stack.h not found at ${STACK_FILE}")
endif()

file(READ "${STACK_FILE}" content)

set(original [=[        stack_ = static_cast<char*>(allocator_->Realloc(stack_, GetCapacity(), newCapacity));]=])
set(replacement [=[        char* newStack = static_cast<char*>(allocator_->Realloc(stack_, GetCapacity(), newCapacity));
        if (!newStack)
        {
            throw std::bad_alloc();
        }
        stack_ = newStack;]=])
set(original_include "#include \"swap.h\"")
set(replacement_include "${original_include}\n#include <new>")

string(FIND "${content}" "${original}" original_pos)
string(FIND "${content}" "${replacement}" replacement_pos)
string(FIND "${content}" "${replacement_include}" include_pos)
if(original_pos EQUAL -1)
    if(NOT replacement_pos EQUAL -1 AND NOT include_pos EQUAL -1)
        message(STATUS "Stack allocation patch already applied")
        return()
    endif()
    message(FATAL_ERROR "Expected stack allocation not found in ${STACK_FILE}")
endif()
if(NOT replacement_pos EQUAL -1)
    message(FATAL_ERROR "Unexpected mixed stack allocation patch in ${STACK_FILE}")
endif()

string(FIND "${content}" "${original_include}" original_include_pos)
if(original_include_pos EQUAL -1)
    message(FATAL_ERROR "Expected stack include not found in ${STACK_FILE}")
endif()

string(REPLACE "${original}" "${replacement}" content "${content}")
if(include_pos EQUAL -1)
    string(REPLACE "${original_include}" "${replacement_include}" content "${content}")
endif()
file(WRITE "${STACK_FILE}" "${content}")
message(STATUS "Applied stack allocation patch")
