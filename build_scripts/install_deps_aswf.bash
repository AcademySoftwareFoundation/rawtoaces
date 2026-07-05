#!/usr/bin/env bash

set -ex

conan profile detect --force
conan install --requires=tsl-robin-map/1.4.0 --generator CMakeDeps --generator CMakeToolchain --output-folder=build_deps --build=missing --settings compiler.cppstd=gnu17
python -m pip install pytest
sudo yum install --setopt=tsflags=nodocs -y perl-Image-ExifTool
