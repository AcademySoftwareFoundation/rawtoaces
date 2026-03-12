#!/usr/bin/env bash

set -ex

brew update

brew install \
    ceres-solver \
    nlohmann-json \
    openimageio \
    nanobind \
    robin-map \
    exiftool \
    lensfun

python3 -m pip install --break-system-packages pytest
