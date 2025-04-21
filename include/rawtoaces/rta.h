// SPDX-License-Identifier: apache-2.0
// Copyright Contributors to the rawtoaces Project.

#ifndef _RTA_h__
#define _RTA_h__

#include "define.h"

#include <stdint.h>
#include <libraw/libraw.h>

using namespace std;

namespace rta
{
struct CIEXYZ
{
    CIEXYZ(){};
    CIEXYZ( double X, double Y, double Z ) : _Xt( X ), _Yt( Y ), _Zt( Z ){};
    double _Xt;
    double _Yt;
    double _Zt;
};

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

class DNGIdt
{
public:
    DNGIdt();
    DNGIdt( libraw_rawdata_t R );
    virtual ~DNGIdt();

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
    vector<double> _cameraCalibration1DNG;
    vector<double> _cameraCalibration2DNG;
    vector<double> _cameraToXYZMtx;
    vector<double> _xyz2rgbMatrix1DNG;
    vector<double> _xyz2rgbMatrix2DNG;
    vector<double> _analogBalanceDNG;
    vector<double> _neutralRGBDNG;
    vector<double> _cameraXYZWhitePoint;
    vector<double> _calibrateIllum;
    double         _baseExpo;
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

} // namespace rta
#endif
