// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

// Contains the declarations of the private functions,
// exposed here for unit-testing.

namespace rta
{
namespace core
{

std::vector<double> cctToxy( const double &cctd );

void scaleLSC( const SpectralData &camera, SpectralData &illuminant );

std::vector<double>
calCM( const SpectralData &camera, const SpectralData &illuminant );

std::vector<Spectrum>
calTI( const SpectralData &illuminant, const SpectralData &training_data );

std::vector<double>
calWB( const SpectralData &camera, SpectralData &illuminant, int highlight );

std::vector<std::vector<double>> calXYZ(
    const SpectralData          &observer,
    const SpectralData          &illuminant,
    const std::vector<Spectrum> &TI );

std::vector<std::vector<double>> calRGB(
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

double ccttoMired( const double cct );

double robertsonLength(
    const std::vector<double> &uv, const std::vector<double> &uvt );

double lightSourceToColorTemp( const unsigned short tag );

double XYZToColorTemperature( const std::vector<double> &XYZ );

std::vector<double> XYZtoCameraWeightedMatrix(
    const double              &mired0,
    const double              &mired1,
    const double              &mired2,
    const std::vector<double> &matrix1,
    const std::vector<double> &matrix2 );

std::vector<double> colorTemperatureToXYZ( const double &cct );

std::vector<double> matrixRGBtoXYZ( const double chromaticities[][2] );

std::vector<double> findXYZtoCameraMtx(
    const Metadata &metadata, const std::vector<double> &neutralRGB );

void getCameraXYZMtxAndWhitePoint(
    const Metadata      &metadata,
    std::vector<double> &out_camera_to_XYZ_matrix,
    std::vector<double> &out_camera_XYZ_white_point );
} // namespace core
} // namespace rta
