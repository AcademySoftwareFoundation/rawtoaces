#!/usr/bin/env bash

set -ex

vcpkg install \
    ceres:x64-windows \
    imath:x64-windows \
    boost-system:x64-windows \
    boost-foreach:x64-windows \
    boost-test:x64-windows \
    boost-property-tree:x64-windows \
    nlohmann-json:x64-windows \
    openimageio:x64-windows
