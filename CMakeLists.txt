cmake_minimum_required(VERSION 3.5)
project (GLTFSDK)

option(ENABLE_UNIT_TESTS "ENABLE_UNIT_TESTS" ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -D_DEBUG -DFEATURE_ASSERTS_ENABLED")
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.11)

if (WIN32)
    # Define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING to disable the warnings in the current version of Google Test (1.8.0)
    # TODO: Newer versions shouldn't have this problem.  Re-evaluate this when upgrading.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING")

    # Multithreaded Build
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/Build/CMake/Modules")


add_subdirectory(External/RapidJSON)
add_subdirectory(External/googletest)
add_subdirectory(GLTFSDK)

if(ENABLE_UNIT_TESTS)
    add_subdirectory(GLTFSDK.TestUtils)
    add_subdirectory(GLTFSDK.Test)
endif()
