# PhotonRenderer
This is a OpenGL Renderer with support for PBR materials and image based lighting (IBL).

## Dependencies
- ASSIMP
- FREETYPE
- GLEW
- GLFW3
- GLM
- IMGUI
- STB

## Installation
Installation was tested on Win10 x64 with Visual Studio 2017 and 2019. It's recommended to use VCPKG for installation.

Requirenments: 
- CMake
- Git
- Visual Studio 2017 or 2019

### Build dependencies
Download and install [VCPKG](https://github.com/microsoft/vcpkg). Set environment variable VCPKG_DEFAULT_TRIPLET=x64-windows.

Install dependencies:
```
./vcpkg.exe install assimp freetype glew glfw3 glm stb
```

### Building
Open CMake-GUI
- set source path to ./src
- set build path to ./build
- press configure and select your visual studio version and the platform x64
- select specify toolchain file for cross-compiling
- set toolchain file to: <vcpkg_root>/scripts/buildsystems/vcpkg.cmake
- press generate and open solution
- select configuration Release x64 and Build Solution

## Running
If you run it from Visual Studio you need to set the Working Directory to the Folder where the exe file is located.
