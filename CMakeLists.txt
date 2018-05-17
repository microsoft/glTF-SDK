cmake_minimum_required(VERSION 3.5)
project (GLTFSDK)

option(ENABLE_UNIT_TESTS "ENABLE_UNIT_TESTS" ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -D_DEBUG -DFEATURE_ASSERTS_ENABLED")
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.11)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/Build/CMake/Modules")


add_subdirectory(External/RapidJSON)
add_subdirectory(External/googletest)
add_subdirectory(GLTFSDK)

if(ENABLE_UNIT_TESTS)
    add_subdirectory(GLTFSDK.TestUtils)
    add_subdirectory(GLTFSDK.Test)
endif()
