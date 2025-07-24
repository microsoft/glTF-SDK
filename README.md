[![Build Status](https://github.com/microsoft/glTF-SDK/actions/workflows/ci.yml/badge.svg)](https://github.com/microsoft/glTF-SDK/actions/workflows/ci.yml)

# Microsoft glTF SDK - A C++ Deserializer/Serializer for glTF

* Windows/macOS/Android Compatible
* glTF & glB support, including embedded Base64 support
* Flexibility and ease of integration to any 3D engine
* Strongly typed extension support
* Built-in schema validation during deserialization
* Low memory overhead
* Utilities for converting input data of any type (e.g. float/uint8/uint16 RGB/RGBA color) into a single type such as float RGBA color

# Project setup and build

This quick overview will help you get started developing in the glTF SDK
repository. We support development on Windows, macOS, and Linux. This overview is intended
for developers familiar with common native development practices.

## **CMake Configure and Build**

**Required Tools:** [git](https://git-scm.com/), [CMake (version 3.X.X)](https://cmake.org/), [powershell](https://learn.microsoft.com/en-us/powershell/scripting/install/installing-powershell?view=powershell-7.5)

The first step for all development environments and targets is to clone the repo. Use a
git-enabled terminal to follow the steps below.

```
git clone https://github.com/microsoft/glTF-SDK.git
```

glTF SDK build system is based on CMake, which customarily uses a separate
build directory. Build directory location is up to you, but we highly recommend using
the `Built` directory from the repository root. The `.gitignore` file is set up to
ignore this `Built` directory.

**NOTE:** We don't currently support usage of CMake 4.0 or higher.

Use the following command to output a platform specific project using CMake:

```
cmake -B Built
```

Use the following command to build the binaries and install: 

```
cmake --build ./Built --target install --config Debug
```

This will install all binaries produced by this repo into a "Built/Out". This include the static library, the Serialize and Deserialize applications as well as the GLTFSDK.Test test application.


## **Running tests**

To run the test application and save the results just use the following command from the GLTFSDK.Test installation folder:

```
.\GLTFSDK.Test.exe --gtest_output=xml:GLTFSDK.Test.log
```

# Trademarks

glTF is a trademark of The Khronos Group Inc.

# Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.microsoft.com.

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.
