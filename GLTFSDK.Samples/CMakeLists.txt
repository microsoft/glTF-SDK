cmake_minimum_required(VERSION 3.5)

# Visual Studio 2026 (MSVC 14.5x) removed the <experimental/filesystem> header that
# these samples fall back to under C++14. Build the samples as C++17 so they use the
# standard <filesystem> header instead (see each sample's Source/main.cpp). The GLTFSDK
# library itself continues to target C++14.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Deserialize)
add_subdirectory(Serialize)
