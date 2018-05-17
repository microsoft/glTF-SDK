cmake_minimum_required(VERSION 3.5)
project (GLTFSDK.Test)

include(GLTFPlatform)
GetGLTFPlatform(Platform)

file(GLOB source_files
    "${CMAKE_CURRENT_LIST_DIR}/Source/*"
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
)

add_executable(GLTFSDK.Test ${source_files})

if (MSVC)
    # Generate PDB files in all configurations, not just Debug (/Zi)
    # Set warning level to 4 (/W4)
    target_compile_options(GLTFSDK.Test PRIVATE "/Zi;/W4;/EHsc")

    # Make sure that all PDB files on Windows are installed to the output folder.  By default, only the debug build does this.
    set_target_properties(GLTFSDK.Test PROPERTIES COMPILE_PDB_NAME "GLTFSDK.Test" COMPILE_PDB_OUTPUT_DIRECTORY "${RUNTIME_OUTPUT_DIRECTORY}")
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(GLTFSDK.Test
        PRIVATE "-Wunguarded-availability"
        PRIVATE "-Wall"
        PRIVATE "-Werror"
        PUBLIC "-Wno-unknown-pragmas")
endif()

target_include_directories(GLTFSDK.Test
    PUBLIC "${CMAKE_CURRENT_LIST_DIR}/Inc"
    PRIVATE "${CMAKE_SOURCE_DIR}/Built/Int"
    PRIVATE "${CMAKE_CURRENT_LIST_DIR}"
)

target_link_libraries(GLTFSDK.Test
    GLTFSDK
    gtest_main
    RapidJSON
    GLTFSDK.TestUtils
)

add_custom_command(TARGET GLTFSDK.Test
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_CURRENT_LIST_DIR}/Resources" "${PROJECT_BINARY_DIR}/$<CONFIG>/Resources/"
)

AddGLTFIOSAppProperties(GLTFSDK.Test)
CreateGLTFInstallTargets(GLTFSDK.Test ${Platform})

install(FILES ${PROJECT_BINARY_DIR}/$<CONFIG>/Resources/ DESTINATION ${CMAKE_SOURCE_DIR}/Built/Out/${Platform}/$<CONFIG>/${PROJECT_NAME}/Resources)
