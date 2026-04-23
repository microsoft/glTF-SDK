# Check if the RapidJSON target has already been defined
if (TARGET RapidJSON)
  message(AUTHOR_WARNING "RapidJSON target already defined, skipping")
  return()
endif()

# Download and unpack RapidJSON at configure time
configure_file(CMakeRapidJSONDownload.txt.in ${CMAKE_BINARY_DIR}/RapidJSON-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/RapidJSON-download )
if(result)
  message(FATAL_ERROR "CMake step for RapidJSON failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/RapidJSON-download )
if(result)
  message(FATAL_ERROR "Build step for RapidJSON failed: ${result}")
endif()

SET(RAPIDJSON_BUILD_TESTS  OFF CACHE BOOL "Build rapidjson perftests and unittests." FORCE)

# Add RapidJSON directly to our build.
add_subdirectory(${CMAKE_BINARY_DIR}/RapidJSON-src
                 ${CMAKE_BINARY_DIR}/RapidJSON-build
                 EXCLUDE_FROM_ALL)

# Get include directories directly from the target created by add_subdirectory.
# Previous versions used find_package(RapidJSON REQUIRED CONFIG) here, but newer
# RapidJSON versions generate a config that requires a build-step-generated targets
# file. Since we use BUILD_COMMAND "", we get the include dirs from the target directly.
# Note: newer RapidJSON only sets INSTALL_INTERFACE, so we must add BUILD_INTERFACE
# include dirs explicitly for subdirectory usage.
set(RapidJSON_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/RapidJSON-src/include")
target_include_directories(RapidJSON INTERFACE
    $<BUILD_INTERFACE:${RapidJSON_INCLUDE_DIRS}>
)
