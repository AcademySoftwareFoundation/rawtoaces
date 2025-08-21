// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <rawtoaces/spectral_data.h>

namespace rta
{
namespace core
{

std::vector<double> cctToxy( const double &cctd );

void calDayLightSPD( const int &cct, Spectrum &spectrum );
void calBlackBodySPD( const int &cct, Spectrum &spectrum );

class Idt
{
public:
    Idt();

    int loadCameraSpst(
        const std::string &path,
        const std::string &make,
        const std::string &model );
    int loadIlluminant(
        const std::vector<std::string> &paths, std::string type = "na" );

    void loadTrainingData( const std::string &path );
    void loadCMF( const std::string &path );
    void chooseIllumSrc( const std::vector<double> &src, int highlight );
    void chooseIllumType( const std::string &type, int highlight );
    void setIlluminants( const SpectralData &illuminant );
    void setVerbosity( const int verbosity );
    void scaleLSC( SpectralData &illuminant );

    std::vector<double>   calCM();
    std::vector<double>   calWB( SpectralData &illuminant, int highlight );
    std::vector<Spectrum> calTI() const;
    std::vector<std::vector<double>>
    calXYZ( const std::vector<Spectrum> &TI ) const;
    std::vector<std::vector<double>>
    calRGB( const std::vector<Spectrum> &TI ) const;

    int curveFit(
        const std::vector<std::vector<double>> &RGB,
        const std::vector<std::vector<double>> &XYZ,
        double                                 *B );
    int calIDT();

    const SpectralData                    &getCameraSpst() const;
    const SpectralData                    &getBestIllum() const;
    const SpectralData                    &getTrainingSpec() const;
    const std::vector<SpectralData>       &getIlluminants() const;
    const SpectralData                    &getCMF() const;
    const std::vector<std::vector<double>> getIDT() const;
    const std::vector<double>              getWB() const;
    const int                              getVerbosity() const;

private:
    SpectralData              _camera;
    SpectralData              _best_illuminant;
    SpectralData              _observer;
    SpectralData              _training_data;
    std::vector<SpectralData> _Illuminants;

    int _verbosity;

    std::vector<double>              _wb;
    std::vector<std::vector<double>> _idt;
};

struct Metadata
{
    struct Calibration
    {
        unsigned short      illuminant = 0;
        std::vector<double> cameraCalibrationMatrix;
        std::vector<double> xyz2rgbMatrix;

        friend bool operator==( const Calibration &c1, const Calibration &c2 )
        {
            if ( c1.illuminant != c2.illuminant )
                return false;
            if ( c1.cameraCalibrationMatrix != c2.cameraCalibrationMatrix )
                return false;
            if ( c1.xyz2rgbMatrix != c2.xyz2rgbMatrix )
                return false;

            return true;
        }

        friend bool operator!=( const Calibration &c1, const Calibration &c2 )
        {
            return !( c1 == c2 );
        }
    } calibration[2];

    std::vector<double> neutralRGB;
    double              baselineExposure = 0.0;

    friend bool operator==( const Metadata &m1, const Metadata &m2 )
    {
        if ( m1.calibration[0] != m2.calibration[0] )
            return false;
        if ( m1.calibration[1] != m2.calibration[1] )
            return false;
        if ( m1.baselineExposure != m2.baselineExposure )
            return false;
        if ( m1.neutralRGB != m2.neutralRGB )
            return false;

        return true;
    }

    friend bool operator!=( const Metadata &m1, const Metadata &m2 )
    {
        return !( m1 == m2 );
    }
};

class DNGIdt
{
    core::Metadata _metadata;

public:
    DNGIdt( const core::Metadata &metadata );

    double ccttoMired( const double cct ) const;
    double robertsonLength(
        const std::vector<double> &uv, const std::vector<double> &uvt ) const;
    double lightSourceToColorTemp( const unsigned short tag ) const;
    double XYZToColorTemperature( const std::vector<double> &XYZ ) const;

    std::vector<double> XYZtoCameraWeightedMatrix(
        const double &mir, const double &mir1, const double &mir2 ) const;

    std::vector<double>
    findXYZtoCameraMtx( const std::vector<double> &neutralRGB ) const;
    std::vector<double> colorTemperatureToXYZ( const double &cct ) const;
    std::vector<double>
    matrixRGBtoXYZ( const double chromaticities[][2] ) const;

    std::vector<std::vector<double>> getDNGCATMatrix();
    std::vector<std::vector<double>> getDNGIDTMatrix();
    void                             getCameraXYZMtxAndWhitePoint();

private:
    std::vector<double> _cameraToXYZMtx;
    std::vector<double> _cameraXYZWhitePoint;
};

struct Objfun
{
    Objfun(
        const std::vector<std::vector<double>> &RGB,
        const std::vector<std::vector<double>> &outLAB )
        : _RGB( RGB ), _outLAB( outLAB )
    {}

    template <typename T> bool operator()( const T *B, T *residuals ) const;

    const std::vector<std::vector<double>> _RGB;
    const std::vector<std::vector<double>> _outLAB;
};

} // namespace core
} // namespace rta
