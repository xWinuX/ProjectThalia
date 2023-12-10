# This Project is still W.I.P
# Split Engine

Vulkan & SDL based Engine 

## VCPKG
This project uses vcpkg to manage its dependencies. \
CMake will install a vcpkg instance inside the build folder and download the packages specified inside vcpkg.json. \
If you do not want that you can set the CMake option "SKIP_AUTOMATE_VCPKG" to true, but this also means you have to build all needed dependencies yourself.

## How to build

`cmake -G "Visual Studio 17 2022" -S . -B build -DCMAKE_BUILD_TYPE=<config>`

`cmake --build build --target SplitEngine --config <config>`
