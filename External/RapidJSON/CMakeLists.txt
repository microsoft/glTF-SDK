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

# Set the RapidJSONConfig.cmake path and make find_package to work in config mode explicitly.
set(RapidJSON_DIR "${CMAKE_BINARY_DIR}/RapidJSON-build" CACHE LOCATION "Specific configuration file location")
find_package(RapidJSON REQUIRED CONFIG)

add_library(RapidJSON INTERFACE IMPORTED GLOBAL)
set_target_properties(RapidJSON PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${RapidJSON_INCLUDE_DIRS})
