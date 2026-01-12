#! /bin/bash
# Execute from the build directory of the CLI tool

set -ex
export RAWTOACES_DATA_PATH="$(realpath "../../_deps/rawtoaces_data-src/data")"

image_dir="../../../tests/materials"
image_file1="${image_dir}/BatteryPark.NEF"
image_file2="${image_dir}/blackmagic_cinema_camera_cinemadng.dng"

./rawtoaces --overwrite ${image_file1} --wb-method metadata --mat-method spectral
./rawtoaces --overwrite ${image_file1} --wb-method illuminant --illuminant 3200K --mat-method spectral
./rawtoaces --overwrite ${image_file1} --wb-method illuminant --illuminant D55 --mat-method spectral
./rawtoaces --overwrite ${image_file1} --wb-method illuminant --illuminant ISO7589 --mat-method spectral
./rawtoaces --overwrite ${image_file1} --wb-method custom --custom-wb 2.0 1.0 1.5 --mat-method spectral
./rawtoaces --overwrite ${image_file1} --wb-method metadata --mat-method metadata
./rawtoaces --overwrite ${image_file1} --wb-method metadata --mat-method custom --custom-mat 1 0 0 0 1 0 0 0 1
./rawtoaces --overwrite ${image_file2} --wb-method metadata --mat-method metadata
./rawtoaces --overwrite ${image_file2} --wb-method metadata --mat-method custom --custom-mat 1 0 0 0 1 0 0 0 1
