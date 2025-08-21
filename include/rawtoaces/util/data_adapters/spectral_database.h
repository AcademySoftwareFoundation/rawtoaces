// Copyright Contributors to the rawtoaces project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/rawtoaces

#pragma once

#include <vector>
#include <map>

#include <rawtoaces/core/spectral_data.h>

namespace rta
{
namespace util
{

// Function to check if a string is a valid input
// to represent color temperature(s) (e.g., D60, 3200K)
bool is_valid_CT( std::string str );

class SpectralDatabase
{
public:
    int verbosity = 0;

    std::string find_file( const std::string &filename );

    std::vector<std::string> collect_data_files( const std::string &type );

    std::vector<core::SpectralData> load_all_cameras();

    bool find_camera(
        const std::string  &make,
        const std::string  &model,
        core::SpectralData &out );

    std::vector<core::SpectralData> load_all_illuminants();

    bool load_illuminant( const std::string &type, core::SpectralData &out );

    core::SpectralData find_best_illuminant(
        const core::SpectralData  &camera,
        const std::vector<double> &wb,
        int                        kelvin_step = 500 );
};

} // namespace util
} // namespace rta
