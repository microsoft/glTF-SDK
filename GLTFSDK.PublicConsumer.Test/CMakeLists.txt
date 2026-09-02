cmake_minimum_required(VERSION 3.5)
project(GLTFSDK.PublicConsumer.Test LANGUAGES CXX)

if(NOT GLTFSDK_ROOT)
    message(FATAL_ERROR "GLTFSDK_ROOT must point to an installed configuration")
endif()

find_path(
    GLTFSDK_INCLUDE_DIR
    NAMES GLTFSDK/Deserialize.h
    PATHS "${GLTFSDK_ROOT}/Inc"
    NO_DEFAULT_PATH
)

find_library(
    GLTFSDK_LIBRARY
    NAMES GLTFSDK
    PATHS "${GLTFSDK_ROOT}"
    NO_DEFAULT_PATH
)

if(NOT GLTFSDK_INCLUDE_DIR OR NOT GLTFSDK_LIBRARY)
    message(FATAL_ERROR "GLTFSDK_ROOT does not contain installed headers and library")
endif()

add_executable(GLTFSDK.PublicConsumer.Test Source/main.cpp)
set_target_properties(
    GLTFSDK.PublicConsumer.Test
    PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
)

target_include_directories(
    GLTFSDK.PublicConsumer.Test
    PRIVATE "${GLTFSDK_INCLUDE_DIR}"
)

target_link_libraries(
    GLTFSDK.PublicConsumer.Test
    PRIVATE "${GLTFSDK_LIBRARY}"
)
