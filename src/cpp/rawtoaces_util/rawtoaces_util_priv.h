// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <filesystem>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <OpenImageIO/imageio.h>
#include <rawtoaces/image_converter.h>

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
    std::string            &error_message );

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
    std::string                      &out_found_illuminant,
    std::string                      &error_message );

/// Apply a colour transform matrix to an image buffer.
/// Exposed for unit-testing of the double-application guard.
bool apply_matrix(
    const std::vector<std::vector<double>> &matrix,
    OIIO::ImageBuf                         &dst,
    const OIIO::ImageBuf                   &src,
    std::string                            &error_message );

void remove_diagnostic_attributes( OIIO::ImageSpec &image_spec );

namespace exiftool
{
bool execute( const std::string &command, std::stringstream &stream );
} // namespace exiftool

} // namespace util
} // namespace rta
