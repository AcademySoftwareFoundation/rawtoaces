#!/usr/bin/env bash

set -ex

vcpkg x-update-baseline \
    --add-initial-baseline \
    --x-manifest-root="./build_scripts"
vcpkg install \
    --x-install-root="C:/vcpkg/installed" \
    --x-manifest-root="./build_scripts"
