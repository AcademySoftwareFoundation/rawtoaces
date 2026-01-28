#!/usr/bin/env bash

set -ex

# Configure vcpkg to use binary cache directory
# This allows vcpkg to automatically reuse pre-built packages from cache
export VCPKG_BINARY_SOURCES="clear;files,C:/vcpkg/binary-cache,readwrite"

# Baseline is pinned in vcpkg.json. The latest hash can be found with:
#   git ls-remote https://github.com/microsoft/vcpkg HEAD | cut -f1

# Install dependencies - vcpkg will automatically use binary cache if available
vcpkg install \
    --x-install-root="C:/vcpkg/installed" \
    --x-manifest-root="./build_scripts"

# Install pip and pytest to the vcpkg Python
# Since vcpkg Python doesn't include pip, install it first using ensurepip
VCPKG_PYTHON="C:/vcpkg/installed/x64-windows/tools/python3/python.exe"
"$VCPKG_PYTHON" -m ensurepip --upgrade
"$VCPKG_PYTHON" -m pip install pytest

curl --silent --location --output ./exiftool.zip https://sourceforge.net/projects/exiftool/files/exiftool-13.47_64.zip
unzip ./exiftool.zip
mv ./exiftool-13.47_64 ./exiftool
mv ./exiftool/exiftool\(-k\).exe ./exiftool/exiftool.exe
