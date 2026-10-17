@echo off
REM Windows Setup Script for rawtoaces C++ Project
REM This script assumes you have Visual Studio and CMake installed

echo Setting up rawtoaces development environment on Windows...

REM Check if CMake is available
cmake --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: CMake is not installed or not in PATH
    echo Please install CMake from https://cmake.org/download/
    echo Make sure to add CMake to system PATH during installation
    pause
    exit /b 1
)

REM Check if Visual Studio compiler is available
cl >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: Visual Studio C++ compiler not found
    echo Please install Visual Studio Community with C++ workload
    echo Or run this from a Visual Studio Developer Command Prompt
    pause
    exit /b 1
)

echo CMake and Visual Studio compiler found. Setting up vcpkg...

REM Set up vcpkg
if not exist "vcpkg" (
    echo Cloning vcpkg...
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    git checkout 2024.01.12
) else (
    echo vcpkg directory already exists
    cd vcpkg
)

REM Bootstrap vcpkg
echo Bootstrapping vcpkg...
call bootstrap-vcpkg.bat
if %errorlevel% neq 0 (
    echo ERROR: Failed to bootstrap vcpkg
    pause
    exit /b 1
)

REM Set vcpkg environment
set VCPKG_ROOT=%CD%
set VCPKG_DEFAULT_TRIPLET=x64-windows

echo Installing required packages with vcpkg...
echo This may take a while...

REM Install all required dependencies
vcpkg install libraw:x64-windows
vcpkg install ceres:x64-windows
vcpkg install imath:x64-windows
vcpkg install boost-system:x64-windows
vcpkg install boost-foreach:x64-windows
vcpkg install boost-filesystem:x64-windows
vcpkg install boost-test:x64-windows
vcpkg install boost-property-tree:x64-windows

REM Install aces-container (this might need to be built from source)
echo Installing aces-container...
git clone https://github.com/ampas/aces_container.git
cd aces_container
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
cmake --build . --config Release
cmake --install . --config Release
cd ../..

echo Setting up rawtoaces project...
cd ..\rawtoaces

REM Create build directory
if not exist "build" mkdir build
cd build

REM Configure with CMake
echo Configuring project with CMake...
cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake -DCMAKE_BUILD_TYPE=Release

REM Build the project
echo Building rawtoaces...
cmake --build . --config Release

echo Setup complete! The rawtoaces executable should be in the build\Release directory.
pause

