cmake_minimum_required(VERSION 3.5)
project(valijson VERSION 1.0.6 LANGUAGES NONE)

add_library(valijson INTERFACE)
add_library(ValiJSON::valijson ALIAS valijson)

target_include_directories(valijson SYSTEM
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_compile_definitions(valijson
    INTERFACE VALIJSON_USE_EXCEPTIONS=1
)

target_link_libraries(valijson
    INTERFACE nlohmann_json::nlohmann_json
)
