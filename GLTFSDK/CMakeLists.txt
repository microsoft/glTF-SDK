cmake_minimum_required(VERSION 3.5)
project (GLTFSDK)

include(GLTFPlatform)
GetGLTFPlatform(Platform)

function(add_msvc_security_flags target)
	# enable /Qspectre compiler flag in Visual Studio 2017 version 15.5.5 and later.
	if(MSVC_VERSION GREATER_EQUAL 1913)
		target_compile_options(${target} PRIVATE "/Qspectre")
	endif()
	# enable /ZH:SHA_256 compiler flag in Visual Studio 2019 version 16.4 and later.
	if(MSVC_VERSION GREATER_EQUAL 1924)
		target_compile_options(${target} PRIVATE "/ZH:SHA_256")
	endif()
	# The /guard:ehcont option is available in Visual Studio 2019 version 16.7 and later. The feature is supported for 64-bit processes on a 64-bit OS.
	if((MSVC_VERSION GREATER_EQUAL 1928) AND (CMAKE_SIZEOF_VOID_P EQUAL 8))
		target_compile_options(${target} PRIVATE "$<$<NOT:$<CONFIG:DEBUG>>:/guard:ehcont>")
		target_link_options(${target} PRIVATE "$<$<NOT:$<CONFIG:DEBUG>>:/guard:ehcont>")
	endif()
endfunction()

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

    add_msvc_security_flags(GLTFSDK)

    # Make sure that all PDB files on Windows are installed to the output folder with the libraries. By default, only the debug build does this.
    set_target_properties(GLTFSDK PROPERTIES COMPILE_PDB_NAME "GLTFSDK" COMPILE_PDB_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_DIRECTORY}")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(GLTFSDK
        PRIVATE "-Wunguarded-availability"
        PRIVATE "-Wall"
        PRIVATE "-Werror"
        PUBLIC "-Wno-unknown-pragmas"
        PRIVATE "-fstack-protector-strong"
        PRIVATE "-Wformat"
        PRIVATE "-Wformat-security"
        PRIVATE "-Werror=format-security")
endif()

target_include_directories(GLTFSDK
    PUBLIC "${CMAKE_CURRENT_LIST_DIR}/Inc"
    PRIVATE "${CMAKE_SOURCE_DIR}/Built/Int"
    PRIVATE "${CMAKE_BINARY_DIR}/GeneratedFiles"
)

CreateGLTFInstallTargets(GLTFSDK ${Platform})
