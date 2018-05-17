cmake_minimum_required(VERSION 3.9)

if (NOT GLTFSDK_FOUND)
    set(GLTFSDK_FOUND TRUE)
    add_library(GLTFSDK STATIC IMPORTED GLOBAL)
    set_target_properties(GLTFSDK PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/build/native/include")
    set_target_properties(GLTFSDK PROPERTIES IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_LIST_DIR}/build/native/lib/x86_64/Debug/static/libGLTFSDK.a")
    set_target_properties(GLTFSDK PROPERTIES IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_LIST_DIR}/build/native/lib/x86_64/Release/static/libGLTFSDK.a")
    set_target_properties(GLTFSDK PROPERTIES IMPORTED_LOCATION_RELWITHDEBINFO "${CMAKE_CURRENT_LIST_DIR}/build/native/lib/x86_64/Release/static/libGLTFSDK.a")
    set_target_properties(GLTFSDK PROPERTIES IMPORTED_LOCATION_MINSIZEREL "${CMAKE_CURRENT_LIST_DIR}/build/native/lib/x86_64/Release/static/libGLTFSDK.a")
    # Default location for other build configurations defaults to Debug
    set_target_properties(GLTFSDK PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/build/native/lib/x86_64/Debug/static/libGLTFSDK.a")
endif()