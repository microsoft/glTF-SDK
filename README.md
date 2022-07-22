[![Build Status](https://gltfsdk.visualstudio.com/build/_apis/build/status/Microsoft.glTF-SDK)](https://gltfsdk.visualstudio.com/build/_build/latest?definitionId=1)

# Microsoft glTF SDK - A C++ Deserializer/Serializer for glTF

* Windows/macOS/Android Compatible
* glTF & glB support, including embedded Base64 support
* Flexibility and ease of integration to any 3D engine
* Strongly typed extension support
* Built-in schema validation during deserialization
* Low memory overhead
* Utilities for converting input data of any type (e.g. float/uint8/uint16 RGB/RGBA color) into a single type such as float RGBA color

# Building

Navigate to the root of the repository.
Note: This folder name is more of a guideline but is useful if you're using cmake to build for multiple platforms. You can also name the path something more meaningful, as long as it's in the Built folder because git ignores that folder.

```
mkdir Built && cd Built
```
Pick one of the following depending on your environment.

```
# Windows - remove the curly braces and pick one the architures.
# This will create a .sln in the current directory for Visual Studio that you can open on the IDE and build or edit.
cmake ../ -G "Visual Studio 16 2019" -A "{x64 || Win32 || ARM || ARM64}"
#or
cmake ../ -G "Visual Studio 17 2022" -A "{x64 || Win32 || ARM || ARM64}"

# MacOs
# This will create necessary xcode files in the current directory that you can open on the IDE and build or edit.
cmake ../ -G Xcode

# Ubuntu
cmake ../ -G "Unix Makefiles"
# or
cmake ../ -G Ninja
```
Install - Pick either Debug or Release.
Install files will to written to the `Built\Out` directory located at the root.
```
cmake --build . --target install --config "${Debug || Release}" 
```

# VCPKG

To add glTF-SDK with vcpkg run the following.

```
./vcpkg install ms-gltf
```

# Nuget Packages

* [Microsoft.glTF.CPP](https://www.nuget.org/packages/Microsoft.glTF.CPP/)
* [Microsoft.glTF.macOS.CPP](https://www.nuget.org/packages/Microsoft.glTF.macOS.CPP/)
* [Microsoft.glTF.iOS.CPP](https://www.nuget.org/packages/Microsoft.glTF.iOS.CPP/)
* [Microsoft.glTF.Android.CPP](https://www.nuget.org/packages/Microsoft.glTF.Android.CPP/)

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
