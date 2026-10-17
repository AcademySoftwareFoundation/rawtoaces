# Simple Windows Setup for rawtoaces

Since the automated setup is complex, here's a simpler step-by-step approach:

## Current Status ✅
- ✅ CMake 4.1.1 - Working
- ✅ Visual Studio 2022 Community - Working
- ✅ aces-container - Downloaded

## Next Steps

### Option 1: Use Pre-built Dependencies (Recommended)

1. **Download pre-built libraries:**
   - Download Boost: https://www.boost.org/users/download/
   - Download Eigen: https://eigen.tuxfamily.org/index.php?title=Main_Page
   - Download Imath: https://github.com/AcademySoftwareFoundation/Imath
   - Download Ceres: http://ceres-solver.org/installation.html
   - Download LibRaw: https://www.libraw.org/download

2. **Extract and configure:**
   ```cmd
   # Set environment variables for library paths
   set BOOST_ROOT=C:\path\to\boost
   set EIGEN3_ROOT=C:\path\to\eigen
   set IMATH_ROOT=C:\path\to\imath
   set CERES_ROOT=C:\path\to\ceres
   set LIBRAW_ROOT=C:\path\to\libraw
   ```

### Option 2: Use Chocolatey Package Manager

1. **Install Chocolatey (run as Administrator):**
   ```powershell
   Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
   ```

2. **Install dependencies:**
   ```cmd
   choco install boost eigen imath ceres-solver libraw
   ```

### Option 3: Manual Build (Advanced)

1. **Build each dependency from source:**
   - This requires more time but gives you full control
   - Each library has its own build instructions

## Quick Test

Once you have the dependencies, try:

```cmd
cd F:\Projects\coding\rawtoaces\build
cmake .. -DCMAKE_PREFIX_PATH="C:\path\to\dependencies"
cmake --build . --config Release
```

## Alternative: Use Docker

If the Windows setup is too complex, you can use Docker:

```cmd
docker build -f Dockerfile -t rawtoaces:latest .
docker run -it --rm -v %CD%:/workspace rawtoaces:latest
```

## What We've Accomplished

✅ **CMake**: Working (version 4.1.1)  
✅ **Visual Studio**: Working (version 19.44.35215)  
✅ **aces-container**: Downloaded  
🔄 **Other dependencies**: Need to be installed

The project is ready to build once the remaining dependencies are installed.

