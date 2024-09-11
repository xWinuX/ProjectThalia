> This project is W.I.P stuff will be missing or not work
# Split Engine

Vulkan & SDL based Engine 

## VCPKG
This project uses vcpkg to manage its dependencies.

## How to build

### Generate project files
`cmake.exe -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=<PathToVCPKGInstance>\scripts\buildsystems\vcpkg.cmake -S <SourcePath> -B <BuildPath>`

### Build Project
`cmake.exe --build <BuildPath> --target ALL_BUILD --config <Debug/Release/RelWithDebInfo>`
