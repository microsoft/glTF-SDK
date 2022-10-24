
###############################################################################
# Sets the name of the platform in the variable named by 'outPlatform'
# Possible values are "iOS", "iOSSimulator", "iOSSimulator64", "appleTV", "appleTVSimulator"
# Android values are of the form "android_<ANDROID_ABI>".
# Possible values of ANDROID_ABIs are found here : https://developer.android.com/ndk/guides/abis
# TODO: Modify the Android and iOS logic to calculate outPlatform the same way we do on Windows.
###############################################################################

function (GetGLTFPlatform outPlatform)

if(ANDROID_OS_PLATFORM)
    if(ANDROID_OS_PLATFORM STREQUAL "ANDROID")
        if( (ANDROID_ABI STREQUAL "x86") OR
            (ANDROID_ABI STREQUAL "x86_64") OR
            (ANDROID_ABI STREQUAL "arm64-v8a") OR
            (ANDROID_ABI STREQUAL "armeabi-v7a"))
            set(${outPlatform} "android_${ANDROID_ABI}" PARENT_SCOPE)
        else()
            message(FATAL_ERROR "Invalid ANDROID_ABI: ${ANDROID_ABI}")
        endif()
    else()
        message(FATAL_ERROR "Invalid ANDROID_OS_PLATFORM: ${ANDROID_OS_PLATFORM}")
    endif()
elseif (IOS_PLATFORM)

    if (IOS_PLATFORM STREQUAL "OS")
        set(${outPlatform} iOS PARENT_SCOPE)
    elseif (IOS_PLATFORM STREQUAL "SIMULATOR")
        set(${outPlatform} iOSSimulator PARENT_SCOPE)
    elseif (IOS_PLATFORM STREQUAL "SIMULATOR64")
        set(${outPlatform} iOSSimulator64 PARENT_SCOPE)
    elseif (IOS_PLATFORM STREQUAL "TVOS")
        set(${outPlatform} appleTV PARENT_SCOPE)
    elseif (IOS_PLATFORM STREQUAL "SIMULATOR_TVOS")
        set(${outPlatform} appleTVSimulator PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Invalid IOS_PLATFORM: ${IOS_PLATFORM}")
    endif()
elseif (MSVC)
    set(${outPlatform} "windows_${CMAKE_GENERATOR_PLATFORM}" PARENT_SCOPE)
else()
    # MacOS
    set(${outPlatform} macOS PARENT_SCOPE)
endif()

endfunction(GetGLTFPlatform)


###############################################################################
# Set the default install TARGETS (Built/Out/<Platform>/<Config>/<Project>)
# for the ARCHIVE, LIBRARY, RUNTIME, and BUNDLE folders
###############################################################################

function(CreateGLTFInstallTargets target platform)

    install(TARGETS ${target}
        ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/Built/Out/${platform}/$<CONFIG>/${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/Built/Out/${platform}/$<CONFIG>/${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/Built/Out/${platform}/$<CONFIG>/${PROJECT_NAME}
        BUNDLE DESTINATION ${CMAKE_SOURCE_DIR}/Built/Out/${platform}/$<CONFIG>/${PROJECT_NAME}
    )

    if (MSVC)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${PROJECT_NAME}.pdb DESTINATION ${CMAKE_SOURCE_DIR}/Built/Out/${platform}/$<CONFIG>/${PROJECT_NAME})
    endif()

endfunction(CreateGLTFInstallTargets)


###############################################################################
# Add the required properties and libraries to create an iOS Application
# Assumes that the Info.plist is in the same folder as the caller
###############################################################################

function(AddGLTFIOSAppProperties target)

    if(IOS_PLATFORM)

        # Locate system libraries on iOS
        find_library(UIKIT UIKit)
        find_library(FOUNDATION Foundation)
        find_library(MOBILECORESERVICES MobileCoreServices)
        find_library(CFNETWORK CFNetwork)
        find_library(SYSTEMCONFIGURATION SystemConfiguration)

        # link the frameworks located above
        target_link_libraries(${target} ${UIKIT})
        target_link_libraries(${target} ${FOUNDATION})
        target_link_libraries(${target} ${MOBILECORESERVICES})
        target_link_libraries(${target} ${CFNETWORK})
        target_link_libraries(${target} ${SYSTEMCONFIGURATION})

        set_target_properties(${target} PROPERTIES
            MACOSX_BUNDLE true
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/Info.plist"
            XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES

            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 9.0
            XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"

            # This is a temporary value that is a personal team (kaokuda) to confirm that these settings are working
            # When we get a real "Team" for development that we can all share then this should be changed.
            # Change this locally to your personal Team to simplify the build process.
            XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "BKXU2T37Q4"
        )

    endif()

endfunction(AddGLTFIOSAppProperties)
