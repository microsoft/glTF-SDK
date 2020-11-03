cmake_minimum_required(VERSION 3.5)
project (GLTFSDK)

option(ENABLE_UNIT_TESTS "ENABLE_UNIT_TESTS" ON)
if(ENABLE_UNIT_TESTS)
    enable_testing()
endif()

# Disable the samples on macOS, iOS, and Android since the experimental features they use
# do not yet build with XCode or clang on these platforms.
if(APPLE OR ANDROID_OS_PLATFORM)
    option(ENABLE_SAMPLES "ENABLE_SAMPLES" OFF)
else()
    option(ENABLE_SAMPLES "ENABLE_SAMPLES" ON)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -D_DEBUG -DFEATURE_ASSERTS_ENABLED")
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.11)

if (WIN32)
    # Define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING to disable the warnings in the current version of Google Test (1.8.0)
    # TODO: Newer versions shouldn't have this problem.  Re-evaluate this when upgrading.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING /D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING")

    # Multithreaded Build
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/Build/CMake/Modules")

find_package(RapidJSON CONFIG)
if(NOT RapidJSON_FOUND)
    add_subdirectory(External/RapidJSON)
endif()
add_subdirectory(GLTFSDK)

if(RapidJSON_FOUND)
    target_include_directories(GLTFSDK
    PUBLIC
        $<BUILD_INTERFACE:${RAPIDJSON_INCLUDE_DIRS}>
    )
else()
    target_link_libraries(GLTFSDK
    PRIVATE
        RapidJSON
    )
endif()

if(ENABLE_UNIT_TESTS)
    find_package(GTest CONFIG REQUIRED)
    # add_subdirectory(External/googletest)
    add_subdirectory(GLTFSDK.TestUtils)
    add_subdirectory(GLTFSDK.Test)
endif()

if(ENABLE_SAMPLES)
    add_subdirectory(GLTFSDK.Samples)
endif()
