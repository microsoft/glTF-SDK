set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Linux)

# Address and undefined-behavior sanitizer flags applied to vcpkg
# dependencies so they are built with the same instrumentation as
# the main project.
set(VCPKG_CXX_FLAGS "-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer")
set(VCPKG_C_FLAGS "-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer")
set(VCPKG_LINKER_FLAGS "-fsanitize=address,undefined")

# Let CC / CXX env vars propagate so vcpkg uses the same compiler
# (clang) as the main project.
set(VCPKG_ENV_PASSTHROUGH_UNTRACKED CC CXX)
