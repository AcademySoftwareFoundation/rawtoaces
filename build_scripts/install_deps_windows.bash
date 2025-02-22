#!/usr/bin/env bash

set -ex

vcpkg install \
    libraw:x64-windows \
    ceres:x64-windows \
    imath:x64-windows \
    openimageio:x64-windows \
    nlohmann-json:x64-windows
