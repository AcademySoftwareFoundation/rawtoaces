#!/usr/bin/env bash

set -ex

# Configure vcpkg to use binary cache directory
# This allows vcpkg to automatically reuse pre-built packages from cache
export VCPKG_BINARY_SOURCES="clear;files,C:/vcpkg/binary-cache,readwrite"

# Add/update baseline in vcpkg.json
vcpkg x-update-baseline \
    --add-initial-baseline \
    --x-manifest-root="./build_scripts"

# Install dependencies - vcpkg will automatically use binary cache if available
vcpkg install \
    --x-install-root="C:/vcpkg/installed" \
    --x-manifest-root="./build_scripts"

# Install pip and pytest to the vcpkg Python
# Since vcpkg Python doesn't include pip, install it first using ensurepip
VCPKG_PYTHON="C:/vcpkg/installed/x64-windows/tools/python3/python.exe"
"$VCPKG_PYTHON" -m ensurepip --upgrade
"$VCPKG_PYTHON" -m pip install pytest
