#!/bin/bash

# RAWtoACES GUI Build Script
# Cross-platform friendly: does not auto-install packages; uses existing Qt6 if found.

set -e

echo "🚀 RAWtoACES GUI Build Script"
echo "=============================="

OS="$OSTYPE"
QT6_PATH=""

if [[ "$OS" == "darwin"* ]]; then
  echo "✅ Detected macOS"
  if command -v brew &>/dev/null; then
    QT6_PATH=$(brew --prefix qt6 2>/dev/null || true)
    if [[ -n "$QT6_PATH" ]]; then
      echo "✅ Using Qt6 at: $QT6_PATH"
    else
      echo "ℹ️  Qt6 prefix not detected via Homebrew; relying on CMake"
    fi
  else
    echo "ℹ️  Homebrew not found; relying on CMake to locate Qt6"
  fi
elif [[ "$OS" == "linux-gnu"* ]]; then
  echo "✅ Detected Linux"
  if command -v qmake6 &>/dev/null || command -v qtpaths6 &>/dev/null; then
    echo "✅ Qt6 tools detected in PATH"
  else
    echo "ℹ️  Qt6 not detected. Please install qt6-base-dev and qt6-tools-dev (Debian/Ubuntu) or qt6-qtbase-devel and qt6-qttools-devel (RHEL/Fedora)."
  fi
else
  echo "ℹ️  This script targets macOS/Linux. On Windows, use PowerShell and CMake with your Qt6 prefix."
  echo "Example:"
  echo "  cmake -S gui -B gui/build -G Ninja -DCMAKE_PREFIX_PATH=\"C:/Qt/6.x.x/msvc2019_64\""
  echo "  cmake --build gui/build --config Release"
fi

# Check rawtoaces availability
echo "🔍 Checking RAWTOACES installation..."
if [[ -n "${RAWTOACES_BIN}" ]]; then
  echo "✅ Using RAWTOACES from RAWTOACES_BIN: ${RAWTOACES_BIN}"
elif command -v rawtoaces &>/dev/null; then
  echo "✅ RAWTOACES found at: $(which rawtoaces)"
else
  echo "❌ RAWTOACES not found in PATH"
  echo "Please install RAWTOACES first by following the main README"
  echo "The GUI requires the core RAWTOACES libraries to be installed"
  exit 1
fi

# Build directory
echo "📁 Creating build directory..."
mkdir -p build
cd build

# Configure
echo "⚙️  Configuring with CMake..."
if [[ -n "$QT6_PATH" ]]; then
  cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$QT6_PATH" -DCMAKE_INSTALL_PREFIX="/usr/local"
else
  cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/usr/local"
fi

# Build
echo "🔨 Building RAWtoACES GUI..."
cmake --build . --parallel $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Check outputs
if [[ -f "rawtoaces-gui" ]] || [[ -f "rawtoaces-gui.exe" ]] || [[ -f "rawtoaces-gui.app/Contents/MacOS/rawtoaces-gui" ]]; then
  echo "✅ Build successful!"
  echo ""
  echo "🎉 RAWtoACES GUI has been built successfully!"
  echo ""
  echo "To run the application:"
  echo "  ./rawtoaces-gui"
  echo ""
  echo "To install system-wide:"
  echo "  sudo cmake --install ."
  echo ""
  echo "Tip: Set RAWTOACES_BIN if rawtoaces is not on PATH (e.g., export RAWTOACES_BIN=/usr/local/bin/rawtoaces)"
else
  echo "❌ Build failed!"
  echo "Check the build output above for errors."
  exit 1
fi
