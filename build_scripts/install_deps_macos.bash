#!/usr/bin/env bash

# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the rawtoaces project.

set -ex

brew update

brew install \
    nlohmann-json \
    openimageio \
    nanobind \
    robin-map \
    exiftool

if [ "$RTA_ENABLE_EIGEN" != "OFF" ];
then
    brew install eigen
fi

if [ "$RTA_ENABLE_CERES" != "OFF" ];
then
    brew install ceres-solver
fi

if [ "$RTA_ENABLE_LENSFUN" != "OFF" ];
then
    brew install lensfun
fi


# Use OpenImageIO's Homebrew Python so its module and pytest are available to
# the same interpreter. python3 on PATH may belong to the runner instead.
# Publish the major.minor version; FindPython matches any patch of that minor.
HOMEBREW_PYTHON="$(brew deps openimageio | sed -n '/^python@/p')"
RTA_PYTHON_VERSION="${HOMEBREW_PYTHON#python@}"
HOMEBREW_PYTHON_EXECUTABLE="$(brew --prefix "$HOMEBREW_PYTHON")/bin/${HOMEBREW_PYTHON/@/}"
"$HOMEBREW_PYTHON_EXECUTABLE" -m pip install --break-system-packages pytest
if [ -n "${GITHUB_ENV:-}" ]; then
    echo "RTA_PYTHON_VERSION=$RTA_PYTHON_VERSION" >> "$GITHUB_ENV"
fi
