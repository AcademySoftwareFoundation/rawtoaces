// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include "define.h"

#include <stdint.h>

using namespace std;

namespace rta
{
namespace core
{

struct trainSpec
{
    uint16_t       _wl;
    vector<double> _data;
};

struct CMF
{
    uint16_t _wl;
    double   _xbar;
    double   _ybar;
    double   _zbar;
};

struct RGBSen
{
    RGBSen(){};
    RGBSen( double R, double G, double B )
        : _RSen( R ), _GSen( G ), _BSen( B ){};

    double _RSen;
    double _GSen;
    double _BSen;
};

class Idt;

class Illum
{
    friend class Idt;

public:
    Illum();
    Illum( string type );
    ~Illum();

    void setIllumType( const string &type );
    void setIllumInc( const int &Inc );
    void setIllumIndex( const double &index );

    const vector<double> getIllumData() const;
    const string         getIllumType() const;
    const int            getIllumInc() const;
    const double         getIllumIndex() const;
    vector<double>       cctToxy( const double &cctd ) const;

    int readSPD( const string &path, const string &type );

    void calDayLightSPD( const int &cct );
    void calBlackBodySPD( const int &cct );

private:
    string         _type;
    int            _inc;
    double         _index;
    vector<double> _data;
};

class Spst
{
    friend class Idt;

public:
    Spst();
    Spst( const Spst &spstobject );
    Spst( char *brand, char *model, uint8_t increment, vector<RGBSen> rgbsen )
        : _brand( brand )
        , _model( model )
        , _increment( increment )
        , _rgbsen( rgbsen ){};
    ~Spst();

    void setBrand( const char *brand );
    void setModel( const char *model );
    void setWLIncrement( const int &inc );
    void setSensitivity( const vector<RGBSen> &rgbsen );

    const char          *getBrand() const;
    const char          *getModel() const;
    const uint8_t        getWLIncrement() const;
    const vector<RGBSen> getSensitivity() const;

    char *getBrand();
    char *getModel();

    int getWLIncrement();
    int loadSpst( const string &path, const char *maker, const char *model );

    vector<RGBSen> getSensitivity();

private:
    char          *_brand;
    char          *_model;
    int            _increment;
    int            _spstMaxCol;
    vector<RGBSen> _rgbsen;
};

class Idt
{
public:
    Idt();
    ~Idt();

    int
    loadCameraSpst( const string &path, const char *maker, const char *model );
    int loadIlluminant( const vector<string> &paths, string type = "na" );

    void loadTrainingData( const string &path );
    void loadCMF( const string &path );
    void chooseIllumSrc( const vector<double> &src, int highlight );
    void chooseIllumType( const char *type, int highlight );
    void setIlluminants( const Illum &Illuminant );
    void setVerbosity( const int verbosity );
    void scaleLSC( Illum &Illuminant );

    vector<double>         calCM();
    vector<double>         calWB( Illum &Illuminant, int highlight );
    vector<vector<double>> calTI() const;
    vector<vector<double>> calXYZ( const vector<vector<double>> &TI ) const;
    vector<vector<double>> calRGB( const vector<vector<double>> &TI ) const;

    int curveFit(
        const vector<vector<double>> &RGB,
        const vector<vector<double>> &XYZ,
        double                       *B );
    int calIDT();

    const Spst                   getCameraSpst() const;
    const Illum                  getBestIllum() const;
    const vector<trainSpec>      getTrainingSpec() const;
    const vector<Illum>          getIlluminants() const;
    const vector<CMF>            getCMF() const;
    const vector<vector<double>> getIDT() const;
    const vector<double>         getWB() const;
    const int                    getVerbosity() const;

private:
    Spst  _cameraSpst;
    Illum _bestIllum;
    int   _verbosity;

    vector<CMF>            _cmf;
    vector<trainSpec>      _trainingSpec;
    vector<Illum>          _Illuminants;
    vector<double>         _wb;
    vector<vector<double>> _idt;
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
        const vector<double> &uv, const vector<double> &uvt ) const;
    double lightSourceToColorTemp( const unsigned short tag ) const;
    double XYZToColorTemperature( const vector<double> &XYZ ) const;

    vector<double> XYZtoCameraWeightedMatrix(
        const double &mir, const double &mir1, const double &mir2 ) const;

    vector<double> findXYZtoCameraMtx( const vector<double> &neutralRGB ) const;
    vector<double> colorTemperatureToXYZ( const double &cct ) const;
    vector<double> matrixRGBtoXYZ( const double chromaticities[][2] ) const;

    vector<vector<double>> getDNGCATMatrix();
    vector<vector<double>> getDNGIDTMatrix();
    void                   getCameraXYZMtxAndWhitePoint();

private:
    vector<double> _cameraToXYZMtx;
    vector<double> _cameraXYZWhitePoint;
};

struct Objfun
{
    Objfun(
        const vector<vector<double>> &RGB,
        const vector<vector<double>> &outLAB )
        : _RGB( RGB ), _outLAB( outLAB )
    {}

    template <typename T> bool operator()( const T *B, T *residuals ) const;

    const vector<vector<double>> _RGB;
    const vector<vector<double>> _outLAB;
};

} // namespace core
} // namespace rta
