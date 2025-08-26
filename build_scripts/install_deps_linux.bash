#!/usr/bin/env bash

set -ex

time sudo apt-get update

time sudo apt-get -q -f install -y \
    libimath-dev \
    libboost-dev \
    libboost-system-dev \
    libboost-test-dev \
    libceres-dev \
    nlohmann-json3-dev \
    libopencv-dev \
    openimageio-tools libopenimageio-dev
