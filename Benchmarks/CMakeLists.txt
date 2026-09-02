cmake_minimum_required(VERSION 3.5)
project(GLTFSDK.JsonBenchmarks)

add_executable(GLTFSDK.JsonBenchmarks
    Source/JsonBenchmarks.cpp
)

target_link_libraries(GLTFSDK.JsonBenchmarks
    PRIVATE
        GLTFSDK
        ValiJSON::valijson
)

target_include_directories(GLTFSDK.JsonBenchmarks
    PRIVATE "${CMAKE_SOURCE_DIR}/GLTFSDK/Source"
)

target_compile_definitions(GLTFSDK.JsonBenchmarks
    PRIVATE GLTFSDK_BENCHMARK_ASSET_DIR="${CMAKE_CURRENT_LIST_DIR}/Assets"
)

if(MSVC)
    target_compile_options(GLTFSDK.JsonBenchmarks PRIVATE "/W4;/EHsc")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(GLTFSDK.JsonBenchmarks PRIVATE "-Wall;-Wextra;-Werror")
endif()

add_subdirectory(LoadExport)
