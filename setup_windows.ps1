# Windows PowerShell Setup Script for rawtoaces C++ Project
# This script assumes you have Visual Studio and CMake installed

Write-Host "Setting up rawtoaces development environment on Windows..." -ForegroundColor Green

# Check if CMake is available
try {
    $cmakeVersion = cmake --version 2>$null
    if ($LASTEXITCODE -ne 0) {
        throw "CMake not found"
    }
    Write-Host "CMake found: $($cmakeVersion[0])" -ForegroundColor Green
} catch {
    Write-Host "ERROR: CMake is not installed or not in PATH" -ForegroundColor Red
    Write-Host "Please install CMake from https://cmake.org/download/" -ForegroundColor Yellow
    Write-Host "Make sure to add CMake to system PATH during installation" -ForegroundColor Yellow
    exit 1
}

# Check if Visual Studio compiler is available
try {
    # Try to find and set up Visual Studio environment
    $vcvarsPath = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    if (Test-Path $vcvarsPath) {
        Write-Host "Found Visual Studio 2022 Community" -ForegroundColor Green
        # Set up Visual Studio environment
        cmd /c "`"$vcvarsPath`" && set" | ForEach-Object {
            if ($_ -match "^([^=]+)=(.*)$") {
                [Environment]::SetEnvironmentVariable($matches[1], $matches[2])
            }
        }
        Write-Host "Visual Studio environment configured" -ForegroundColor Green
    } else {
        throw "Visual Studio not found"
    }
} catch {
    Write-Host "ERROR: Visual Studio C++ compiler not found" -ForegroundColor Red
    Write-Host "Please install Visual Studio Community with C++ workload" -ForegroundColor Yellow
    Write-Host "Or run this from a Visual Studio Developer Command Prompt" -ForegroundColor Yellow
    exit 1
}

Write-Host "CMake and Visual Studio compiler found. Setting up vcpkg..." -ForegroundColor Green

# Set up vcpkg
if (-not (Test-Path "vcpkg")) {
    Write-Host "Cloning vcpkg..." -ForegroundColor Yellow
    git clone https://github.com/Microsoft/vcpkg.git
    Set-Location vcpkg
    git checkout 2024.01.12
} else {
    Write-Host "vcpkg directory already exists" -ForegroundColor Yellow
    Set-Location vcpkg
}

# Bootstrap vcpkg
Write-Host "Bootstrapping vcpkg..." -ForegroundColor Yellow
& .\bootstrap-vcpkg.bat
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to bootstrap vcpkg" -ForegroundColor Red
    exit 1
}

# Set vcpkg environment
$env:VCPKG_ROOT = Get-Location
$env:VCPKG_DEFAULT_TRIPLET = "x64-windows"

Write-Host "Installing required packages with vcpkg..." -ForegroundColor Yellow
Write-Host "This may take a while..." -ForegroundColor Yellow

# Install all required dependencies
$packages = @(
    "libraw:x64-windows",
    "ceres:x64-windows", 
    "imath:x64-windows",
    "boost-system:x64-windows",
    "boost-foreach:x64-windows",
    "boost-filesystem:x64-windows",
    "boost-test:x64-windows",
    "boost-property-tree:x64-windows"
)

foreach ($package in $packages) {
    Write-Host "Installing $package..." -ForegroundColor Cyan
    & .\vcpkg.exe install $package
    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERROR: Failed to install $package" -ForegroundColor Red
        exit 1
    }
}

# Install aces-container (this might need to be built from source)
Write-Host "Installing aces-container..." -ForegroundColor Yellow
git clone https://github.com/ampas/aces_container.git
Set-Location aces_container
New-Item -ItemType Directory -Force -Path "build" | Out-Null
Set-Location build
cmake .. -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"
cmake --build . --config Release
cmake --install . --config Release
Set-Location ../..

Write-Host "Setting up rawtoaces project..." -ForegroundColor Green
Set-Location ..\rawtoaces

# Create build directory
if (-not (Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
}
Set-Location build

# Configure with CMake
Write-Host "Configuring project with CMake..." -ForegroundColor Yellow
cmake .. -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release

# Build the project
Write-Host "Building rawtoaces..." -ForegroundColor Yellow
cmake --build . --config Release

Write-Host "Setup complete! The rawtoaces executable should be in the build\Release directory." -ForegroundColor Green
Write-Host "You can now run: .\build\Release\rawtoaces.exe --help" -ForegroundColor Cyan
