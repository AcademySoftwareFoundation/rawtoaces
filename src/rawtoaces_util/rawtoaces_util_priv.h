// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <filesystem>
#include <set>
#include <string>
#include <vector>
#include <OpenImageIO/imageio.h>
#include "../include/rawtoaces/image_converter.h"

// Contains the declarations of the private functions,
// exposed here for unit-testing.

namespace rta
{
namespace util
{
const std::set<std::string>
parse_raw_extensions( const std::string &extensionlist );
const std::set<std::string> supported_raw_extensions();
std::vector<std::string>
     database_paths( const std::string &override_path = "" );
void fix_metadata( OIIO::ImageSpec &spec );

std::string
combine_make_model( const std::string &make, const std::string &model );

bool check_input(
    const std::string      &input_filename,
    ImageConverter::Status &status,
    std::string             &error_message );

bool fetch_missing_metadata(
    const std::string              &input_path,
    const ImageConverter::Settings &settings,
    OIIO::ImageSpec                &spec,
    std::string                    &error_message );

bool prepare_transform_spectral(
    const OIIO::ImageSpec            &image_spec,
    const ImageConverter::Settings   &settings,
    std::vector<double>              &WB_multipliers,
    std::vector<std::vector<double>> &out_transform_matrix,
    std::string                      &error_message );

} // namespace util
} // namespace rta
