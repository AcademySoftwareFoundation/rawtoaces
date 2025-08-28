#!/usr/bin/env bash

set -ex

vcpkg install \
    ceres:x64-windows \
    nlohmann-json:x64-windows \
    openimageio:x64-windows
