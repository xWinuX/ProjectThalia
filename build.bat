@echo off
if "%1"=="" (
  echo "Usage: build.bat <configuration>"
  exit /b 1
)

set CONFIG=%1

cmake -G "Visual Studio 17 2022" -S . -B build -DCMAKE_BUILD_TYPE=%CONFIG%
cmake --build build --target ProjectThalia --config %CONFIG%
build\bin\%CONFIG%\ProjectThalia.exe