#!/usr/bin/env bash

set -ex

# Configure vcpkg to use binary cache directory
# This allows vcpkg to automatically reuse pre-built packages from cache
export VCPKG_BINARY_SOURCES="clear;files,C:/vcpkg/binary-cache,readwrite"

# Baseline is pinned in vcpkg.json. The latest hash can be found with:
#   git ls-remote https://github.com/microsoft/vcpkg HEAD | cut -f1

FEATURES=""

if [ "$RTA_ENABLE_LENSFUN" == "ON" ]; then
    FEATURES="$FEATURES --x-feature=lensfun"
fi

if [ "$RTA_ENABLE_CERES" == "ON" ]; then
    FEATURES="$FEATURES --x-feature=ceres"
elif  [ "$RTA_ENABLE_EIGEN" == "ON" ]; then
    FEATURES="$FEATURES --x-feature=eigen"
fi

# Install dependencies - vcpkg will automatically use binary cache if available
vcpkg install \
    --x-install-root="C:/vcpkg/installed" \
    --x-manifest-root="./build_scripts" \
    $FEATURES

# Install pip and pytest to the vcpkg Python
# Since vcpkg Python doesn't include pip, install it first using ensurepip
VCPKG_PYTHON="C:/vcpkg/installed/x64-windows/tools/python3/python.exe"
"$VCPKG_PYTHON" -m ensurepip --upgrade
"$VCPKG_PYTHON" -m pip install pytest

curl --silent --location --output ./exiftool_version.txt https://sourceforge.net/projects/exiftool/files/ver.txt
exiftool_version=`cat ./exiftool_version.txt`

curl --silent --location --output ./exiftool.zip https://sourceforge.net/projects/exiftool/files/exiftool-${exiftool_version}_64.zip
unzip ./exiftool.zip

mv ./exiftool-${exiftool_version}_64 ./exiftool
mv ./exiftool/exiftool\(-k\).exe ./exiftool/exiftool.exe
