#!/usr/bin/env bash

set -ex

time sudo apt-get update

time sudo apt-get -q -f install -y \
    libceres-dev \
    nlohmann-json3-dev \
    libopencv-dev \
    openimageio-tools libopenimageio-dev \
    exiftool \
    liblensfun-dev \
    liblensfun-data-v1

# Nanobind in apt is still v1.9, we need at least v2.2.
pip3 install pytest nanobind
