cmake_minimum_required(VERSION 3.5)
project (GLTFSDK)

include(GLTFPlatform)
GetGLTFPlatform(Platform)

file(GLOB source_files
    "${CMAKE_CURRENT_LIST_DIR}/Source/*.cpp"
)

file(GLOB schema_deps
    "${CMAKE_CURRENT_LIST_DIR}/schema/*"
)

find_program(POWERSHELL_PATH NAMES pwsh powershell NO_PACKAGE_ROOT_PATH NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_CMAKE_FIND_ROOT_PATH)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/GeneratedFiles/SchemaJson.h
    COMMAND ${POWERSHELL_PATH} -ExecutionPolicy Bypass "${CMAKE_CURRENT_LIST_DIR}/GenerateSchemaJsonHeader.ps1" -outPath "${CMAKE_BINARY_DIR}/GeneratedFiles"
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    DEPENDS "${schema_deps}"
)

add_library(GLTFSDK ${source_files} ${CMAKE_BINARY_DIR}/GeneratedFiles/SchemaJson.h)

if (MSVC)
    # Generate PDB files in all configurations, not just Debug (/Zi)
    # Set warning level to 4 (/W4)
    target_compile_options(GLTFSDK PRIVATE "/Zi;/W4;/EHsc")

    # Make sure that all PDB files on Windows are installed to the output folder with the libraries. By default, only the debug build does this.
    set_target_properties(GLTFSDK PROPERTIES COMPILE_PDB_NAME "GLTFSDK" COMPILE_PDB_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_DIRECTORY}")
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(GLTFSDK
        PRIVATE "-Wunguarded-availability"
        PRIVATE "-Wall"
        PRIVATE "-Werror"
        PUBLIC "-Wno-unknown-pragmas")
endif()

target_include_directories(GLTFSDK
    PUBLIC "${CMAKE_CURRENT_LIST_DIR}/Inc"
    PRIVATE "${CMAKE_SOURCE_DIR}/Built/Int"
    PRIVATE "${CMAKE_BINARY_DIR}/GeneratedFiles"
)

target_link_libraries(GLTFSDK
    RapidJSON
)

CreateGLTFInstallTargets(GLTFSDK ${Platform})
