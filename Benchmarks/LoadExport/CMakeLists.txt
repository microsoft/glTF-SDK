cmake_minimum_required(VERSION 3.5)
project(GLTFSDK.LoadExportBenchmarks)

add_executable(GLTFSDK.LoadExportBenchmarks
    LoadExportBenchmarks.cpp
)

target_link_libraries(GLTFSDK.LoadExportBenchmarks
    PRIVATE GLTFSDK
)

if(MSVC)
    target_compile_options(GLTFSDK.LoadExportBenchmarks PRIVATE "/W4;/EHsc")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(GLTFSDK.LoadExportBenchmarks PRIVATE "-Wall;-Wextra;-Werror")
endif()
