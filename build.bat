@echo off
if "%1"=="" (
  echo "Usage: build.bat <configuration>"
  exit /b 1
)

set CONFIG=%1

cmake -G "Visual Studio 17 2022" -S . -B build -DCMAKE_BUILD_TYPE=%CONFIG%
if %ERRORLEVEL% neq 0 (
  echo "CMake configuration failed. Exiting."
  exit /b %ERRORLEVEL%
)

cmake --build build --target ProjectThalia --config %CONFIG% --verbose
if %ERRORLEVEL% neq 0 (
  echo "Build failed. Exiting."
  exit /b %ERRORLEVEL%
)

cd build\bin\%CONFIG%
ProjectThalia.exe