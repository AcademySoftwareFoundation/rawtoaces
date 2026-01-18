#!/usr/bin/env bash

set -ex

sudo yum install --setopt=tsflags=nodocs -y eigen3-devel ceres-solver-devel json-devel perl-Image-ExifTool

sudo python -m pip install nanobind pytest
