# Windows Setup Guide for rawtoaces

This guide will help you set up your Windows machine to compile and run the rawtoaces C++ project.

## Prerequisites

Before starting, you need to install the following tools:

### 1. Visual Studio Community (Free)
- Download from: https://visualstudio.microsoft.com/vs/community/
- During installation, select the **"Desktop development with C++"** workload
- This includes:
  - MSVC C++ compiler
  - Windows SDK
  - CMake (usually included)

### 2. CMake (if not included with Visual Studio)
- Download from: https://cmake.org/download/
- Choose the Windows installer
- **Important**: Check "Add CMake to system PATH" during installation

### 3. Git (if not already installed)
- Download from: https://git-scm.com/download/win
- Use default installation options

## Quick Setup (Automated)

Once you have the prerequisites installed, you can use the automated setup scripts:

### Option 1: Batch Script
```cmd
# Run in Command Prompt or PowerShell
setup_windows.bat
```

### Option 2: PowerShell Script
```powershell
# Run in PowerShell (may need to enable execution policy)
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
.\setup_windows.ps1
```

## Manual Setup (Step by Step)

If you prefer to set up manually or if the automated scripts don't work:

### Step 1: Open Developer Command Prompt
- Search for "Developer Command Prompt for VS" in Start Menu
- Or open "x64 Native Tools Command Prompt for VS"

### Step 2: Set up vcpkg
```cmd
cd F:\Projects\coding
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
git checkout 2024.01.12
bootstrap-vcpkg.bat
```

### Step 3: Install Dependencies
```cmd
# Set environment variables
set VCPKG_ROOT=%CD%
set VCPKG_DEFAULT_TRIPLET=x64-windows

# Install required packages
vcpkg install libraw:x64-windows
vcpkg install ceres:x64-windows
vcpkg install imath:x64-windows
vcpkg install boost-system:x64-windows
vcpkg install boost-foreach:x64-windows
vcpkg install boost-filesystem:x64-windows
vcpkg install boost-test:x64-windows
vcpkg install boost-property-tree:x64-windows
```

### Step 4: Install aces-container
```cmd
cd ..
git clone https://github.com/ampas/aces_container.git
cd aces_container
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
cmake --build . --config Release
cmake --install . --config Release
```

### Step 5: Build rawtoaces
```cmd
cd ..\..\rawtoaces
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

## Verification

After successful setup, you should have:
- `build\Release\rawtoaces.exe` - The main executable
- All required libraries installed via vcpkg

Test the installation:
```cmd
.\build\Release\rawtoaces.exe --help
```

## Troubleshooting

### Common Issues:

1. **"cmake is not recognized"**
   - Install CMake and add it to PATH
   - Restart your command prompt

2. **"cl is not recognized"**
   - Use Developer Command Prompt for Visual Studio
   - Or run `vcvarsall.bat x64` in regular command prompt

3. **vcpkg bootstrap fails**
   - Make sure you're using a stable version (2024.01.12)
   - Check that Visual Studio is properly installed

4. **Package installation fails**
   - Some packages take a long time to compile (especially ceres)
   - Make sure you have enough disk space (several GB)
   - Check your internet connection

5. **CMake configuration fails**
   - Verify all dependencies are installed
   - Check that the vcpkg toolchain file path is correct
   - Make sure you're using the correct triplet (x64-windows)

### Getting Help:
- Check the project's GitHub issues: https://github.com/ampas/rawtoaces/issues
- vcpkg documentation: https://vcpkg.readthedocs.io/
- CMake documentation: https://cmake.org/documentation/

## Next Steps

Once setup is complete, you can:
1. Test with sample RAW files
2. Explore the command-line options
3. Integrate into your workflow

The rawtoaces tool supports various RAW formats and provides multiple conversion methods for optimal results.

