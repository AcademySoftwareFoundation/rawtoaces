// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

// Contains the declarations of the private functions,
// exposed here for unit-testing.

namespace rta
{
namespace core
{

std::vector<double> CCT_to_xy( const double &cctd );

void scale_illuminant( const SpectralData &camera, SpectralData &illuminant );

std::vector<double>
calculate_CM( const SpectralData &camera, const SpectralData &illuminant );

std::vector<Spectrum> calculate_TI(
    const SpectralData &illuminant, const SpectralData &training_data );

std::vector<double>
_calculate_WB( const SpectralData &camera, SpectralData &illuminant );

std::vector<std::vector<double>> calculate_XYZ(
    const SpectralData          &observer,
    const SpectralData          &illuminant,
    const std::vector<Spectrum> &TI );

std::vector<std::vector<double>> calculate_RGB(
    const SpectralData          &camera,
    const SpectralData          &illuminant,
    const std::vector<double>   &WB_multipliers,
    const std::vector<Spectrum> &TI );

bool curveFit(
    const std::vector<std::vector<double>> &RGB,
    const std::vector<std::vector<double>> &XYZ,
    double                                 *B,
    int                                     verbosity,
    std::vector<std::vector<double>>       &out_IDT_matrix );

double CCT_to_mired( const double cct );
double mired_to_CCT( const double mired );

double robertson_length(
    const std::vector<double> &uv, const std::vector<double> &uvt );

double light_source_to_color_temp( const unsigned short tag );

double XYZ_to_color_temperature( const std::vector<double> &XYZ );

std::vector<double> XYZ_to_camera_weighted_matrix(
    const double              &mired0,
    const double              &mired1,
    const double              &mired2,
    const std::vector<double> &matrix1,
    const std::vector<double> &matrix2 );

std::vector<double> color_temperature_to_XYZ( const double &cct );

std::vector<double> matrix_RGB_to_XYZ( const double chromaticities[][2] );

std::vector<double> find_XYZ_to_camera_matrix(
    const Metadata &metadata, const std::vector<double> &neutralRGB );

void get_camera_XYZ_matrix_and_white_point(
    const Metadata      &metadata,
    std::vector<double> &out_camera_to_XYZ_matrix,
    std::vector<double> &out_camera_XYZ_white_point );
} // namespace core
} // namespace rta
