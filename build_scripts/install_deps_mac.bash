#!/usr/bin/env bash

set -ex

brew install ceres-solver nlohmann-json openimageio nanobind robin-map

python3 -m pip install --break-system-packages pytest
