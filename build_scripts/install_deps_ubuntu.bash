#!/usr/bin/env bash

set -ex

time sudo apt-get update

time sudo apt-get -q -f install -y \
    nlohmann-json3-dev \
    libopencv-dev \
    openimageio-tools libopenimageio-dev \
    exiftool \

if [ "$RTA_ENABLE_EIGEN" == "ON" ];
then
    time sudo apt-get -q -f install -y \
        libeigen3-dev
fi

if [ "$RTA_ENABLE_CERES" == "ON" ];
then
    time sudo apt-get -q -f install -y \
        libceres-dev
fi

if [ "$RTA_ENABLE_LENSFUN" == "ON" ];
then
    time sudo apt-get -q -f install -y \
        liblensfun-dev \
        liblensfun-data-v1
fi


# Nanobind in apt is still v1.9, we need at least v2.2.
pip3 install pytest nanobind
