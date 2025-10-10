#!/usr/bin/env bash

set -ex

cd ./build_scripts
vcpkg x-update-baseline --add-initial-baseline
vcpkg install
cd ..
