#!/usr/bin/env bash

set -ex

brew update

brew install \
    nlohmann-json \
    openimageio \
    nanobind \
    robin-map \
    exiftool

if [ "$RTA_ENABLE_EIGEN" == "ON" ];
then
    brew install eigen
fi

if [ "$RTA_ENABLE_CERES" == "ON" ];
then
    brew install ceres-solver
fi

if [ "$RTA_ENABLE_LENSFUN" == "ON" ];
then
    brew install lensfun
fi


python3 -m pip install --break-system-packages pytest
