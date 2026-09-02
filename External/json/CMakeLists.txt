cmake_minimum_required(VERSION 3.5)
project(nlohmann_json VERSION 3.12.0 LANGUAGES NONE)

add_library(nlohmann_json INTERFACE)
add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)

target_include_directories(nlohmann_json SYSTEM
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
)
