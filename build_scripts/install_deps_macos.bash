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


# OpenImageIO's Homebrew Python is keg-only, so `python3` on PATH can be a
# different interpreter from the framework FindPython selects. Install pytest
# there, and export Python_ROOT_DIR so FindPython uses that prefix before
# frameworks or PATH. CMake reads this hint from the environment.
python_formula="$(brew deps --formula openimageio | awk '/^python@/{ print; exit }')"
if [ -z "$python_formula" ]; then
    echo "OpenImageIO has no Homebrew python@ dependency" >&2
    exit 1
fi
python_prefix="$(brew --prefix "$python_formula")"
"${python_prefix}/bin/python3" -m pip install --break-system-packages pytest
if [ -n "${GITHUB_ENV:-}" ]; then
    echo "Python_ROOT_DIR=${python_prefix}" >> "$GITHUB_ENV"
fi
