# ProjectThalia

Vulkan & SDL based 2D Sandbox 

## VCPKG
This project uses vcpkg to manage its dependencies, if you haven't already installed it get started [here](https://vcpkg.io/en/getting-started)

## How to build

In the root folder of the project execute the following command to generate the build files \
`cmake -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=<path to your vcpkg installation>\scripts\buildsystems\vcpkg.cmake -S . -B cmake-build-debug`

After this you can build the project like this \
`cmake --build cmake-build-debug --target ProjectThalia --config <Debug|Release>`