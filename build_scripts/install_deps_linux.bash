#!/usr/bin/env bash

set -ex

time sudo apt-get update

time sudo apt-get -q -f install -y \
    libceres-dev \
    nlohmann-json3-dev \
    libopencv-dev \
    openimageio-tools libopenimageio-dev
