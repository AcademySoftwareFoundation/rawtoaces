#!/usr/bin/env bash

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
HOMEBREW_PYTHON="$(brew deps openimageio | sed -n '/^python@/p')"
RTA_CI_PYTHON_EXECUTABLE="$(brew --prefix "$HOMEBREW_PYTHON")/bin/${HOMEBREW_PYTHON/@/}"
"$RTA_CI_PYTHON_EXECUTABLE" -m pip install --break-system-packages pytest
if [ -n "${GITHUB_ENV:-}" ]; then
    echo "RTA_CI_PYTHON_EXECUTABLE=$RTA_CI_PYTHON_EXECUTABLE" >> "$GITHUB_ENV"
fi
