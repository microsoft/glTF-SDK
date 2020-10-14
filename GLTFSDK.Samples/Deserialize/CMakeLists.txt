cmake_minimum_required(VERSION 3.5)
INCLUDE( CheckCXXCompilerFlag )

project (Deserialize)

include(GLTFPlatform)
GetGLTFPlatform(Platform)

file(GLOB source_files
    "${CMAKE_CURRENT_LIST_DIR}/Source/main.cpp"
)

# Check if the compiler supports c++17 and add flags (to avoid using std::experimental::filesystem)
CHECK_CXX_COMPILER_FLAG( "-std=c++11" COMPILER_SUPPORTS_CXX17 )
if (COMPILER_SUPPORTS_CXX17)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

set(STD_FILESYSTEM "stdc++fs")

add_executable(Deserialize ${source_files})

if (MSVC)
    # Generate PDB files in all configurations, not just Debug (/Zi)
    # Set warning level to 4 (/W4)
    target_compile_options(Deserialize PRIVATE "/Zi;/W4;/EHsc")

    # Make sure that all PDB files on Windows are installed to the output folder.  By default, only the debug build does this.
    set_target_properties(Deserialize PROPERTIES COMPILE_PDB_NAME "Deserialize" COMPILE_PDB_OUTPUT_DIRECTORY "${RUNTIME_OUTPUT_DIRECTORY}")
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(Deserialize
        PRIVATE "-Wunguarded-availability"
        PRIVATE "-Wall"
        PRIVATE "-Werror"
        PUBLIC "-Wno-unknown-pragmas")
endif()

target_link_libraries(Deserialize
    GLTFSDK
    ${STD_FILESYSTEM}
)

CreateGLTFInstallTargets(Deserialize ${Platform})
