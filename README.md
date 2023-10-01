# ProjectThalia

Vulkan & SDL based 2D Sandbox 

## VCPKG
This project uses vcpkg to manage its dependencies. \
CMake will install a vcpkg instance inside the build folder and download the packages specified inside vcpkg.json. \
If you do not want that you can set the CMake option "SKIP_AUTOMATE_VCPKG" to true, but this also means you have to build all needed dependencies yourself.

## How to build

You can either run the build.bat like this: \
`cmd.exe /c build.bat <config>`

Or do it manually like this: \
`cmake -G "Visual Studio 17 2022" -S . -B build -DCMAKE_BUILD_TYPE=<config>`

`cmake --build build --target ProjectThalia --config <config>`