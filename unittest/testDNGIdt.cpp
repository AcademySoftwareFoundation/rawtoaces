// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <filesystem>
#include <boost/test/tools/floating_point_comparison.hpp>

#include <libraw/libraw.h>

#include <rawtoaces/rawtoaces_core.h>

using namespace std;
using namespace rta;
using namespace rta::core;

BOOST_AUTO_TEST_CASE( TestIDT_CcttoMired )
{
    Metadata metadata;
    DNGIdt  *di    = new DNGIdt( metadata );
    double   cct   = 6500.0;
    double   mired = di->ccttoMired( cct );
    delete di;

    BOOST_CHECK_CLOSE( mired, 153.8461538462, 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIDT_RobertsonLength )
{
    Metadata       metadata;
    DNGIdt        *di    = new DNGIdt( metadata );
    double         uv[]  = { 0.2042589852, 0.3196233991 };
    double         uvt[] = { 0.1800600000, 0.2635200000, -0.2434100000 };
    vector<double> uvVector( uv, uv + 2 );
    vector<double> uvtVector( uvt, uvt + 3 );

    double rLength = di->robertsonLength( uvVector, uvtVector );
    delete di;

    BOOST_CHECK_CLOSE( rLength, 0.060234937, 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIDT_LightSourceToColorTemp )
{
    Metadata       metadata;
    DNGIdt        *di  = new DNGIdt( metadata );
    unsigned short tag = 17;
    double         ct  = di->lightSourceToColorTemp( tag );
    delete di;

    BOOST_CHECK_CLOSE( ct, 2856.0, 1e-5 );
};

void init_metadata( const LibRaw &libraw, Metadata &metadata )
{
    metadata.neutralRGB.resize( 3 );
    metadata.calibration[0].xyz2rgbMatrix.resize( 9 );
    metadata.calibration[1].xyz2rgbMatrix.resize( 9 );
    metadata.calibration[0].cameraCalibrationMatrix.resize( 9 );
    metadata.calibration[1].cameraCalibrationMatrix.resize( 9 );

    const libraw_colordata_t &color = libraw.imgdata.rawdata.color;

#if LIBRAW_VERSION >= LIBRAW_MAKE_VERSION( 0, 20, 0 )
    metadata.baselineExposure =
        static_cast<double>( color.dng_levels.baseline_exposure );
#else
    metadata.baselineExposure = static_cast<double>( color.baseline_exposure );
#endif

    for ( int i = 0; i < 3; i++ )
    {
        metadata.neutralRGB[i] = 1.0 / static_cast<double>( color.cam_mul[i] );
    }

    for ( int k = 0; k < 2; k++ )
    {
        metadata.calibration[k].illuminant =
            static_cast<double>( color.dng_color[k].illuminant );

        for ( int i = 0; i < 3; i++ )
        {
            for ( int j = 0; j < 3; j++ )
            {
                metadata.calibration[k].xyz2rgbMatrix[i * 3 + j] =
                    color.dng_color[k].colormatrix[i][j];
                metadata.calibration[k].cameraCalibrationMatrix[i * 3 + j] =
                    color.dng_color[k].calibration[i][j];
            }
        }
    }
}

BOOST_AUTO_TEST_CASE( TestIDT_XYZToColorTemperature )
{
    LibRaw                rawProcessor;
    std::filesystem::path pathToRaw = std::filesystem::absolute(
        "../../unittest/materials/blackmagic_cinema_camera_cinemadng.dng" );
    int ret = rawProcessor.open_file( ( pathToRaw.string() ).c_str() );
    ret     = rawProcessor.unpack();

    Metadata metadata;
    init_metadata( rawProcessor, metadata );
    DNGIdt        *di     = new DNGIdt( metadata );
    double         XYZ[3] = { 0.9731171910, 1.0174927152, 0.9498565880 };
    vector<double> XYZVector( XYZ, XYZ + 3 );
    double         cct = di->XYZToColorTemperature( XYZVector );

    rawProcessor.recycle();
    delete di;

    BOOST_CHECK_CLOSE( cct, 5564.6648479019, 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIDT_XYZtoCameraWeightedMatrix )
{
    LibRaw                rawProcessor;
    std::filesystem::path pathToRaw = std::filesystem::absolute(
        "../../unittest/materials/blackmagic_cinema_camera_cinemadng.dng" );
    int ret = rawProcessor.open_file( ( pathToRaw.string() ).c_str() );
    ret     = rawProcessor.unpack();

    Metadata metadata;
    init_metadata( rawProcessor, metadata );
    DNGIdt        *di      = new DNGIdt( metadata );
    double         mirs[3] = { 158.8461538462, 350.1400560224, 153.8461538462 };
    double         matrix[9] = { 1.0165710542,  -0.2791973987, -0.0801820653,
                                 -0.4881171650, 1.3469051835,  0.1100471308,
                                 -0.0607157824, 0.3270949763,  0.5439419519 };
    vector<double> result =
        di->XYZtoCameraWeightedMatrix( mirs[0], mirs[1], mirs[2] );
    rawProcessor.recycle();
    delete di;

    FORI( countSize( matrix ) )
    BOOST_CHECK_CLOSE( result[i], matrix[i], 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIDT_FindXYZtoCameraMtx )
{
    LibRaw                rawProcessor;
    std::filesystem::path pathToRaw = std::filesystem::absolute(
        "../../unittest/materials/blackmagic_cinema_camera_cinemadng.dng" );
    int ret = rawProcessor.open_file( ( pathToRaw.string() ).c_str() );
    ret     = rawProcessor.unpack();

    Metadata metadata;
    init_metadata( rawProcessor, metadata );
    DNGIdt        *di            = new DNGIdt( metadata );
    double         neutralRGB[3] = { 0.6289999865, 1.0000000000, 0.7904000305 };
    double         matrix[9] = { 1.0616656923,  -0.3124143737, -0.0661770211,
                                 -0.4772957633, 1.3614785395,  0.1001599918,
                                 -0.0411839968, 0.3103035015,  0.5718121924 };
    vector<double> neutralRGBVector( neutralRGB, neutralRGB + 3 );
    vector<double> result = di->findXYZtoCameraMtx( neutralRGBVector );

    rawProcessor.recycle();
    delete di;

    FORI( countSize( matrix ) )
    BOOST_CHECK_CLOSE( result[i], matrix[i], 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIDT_ColorTemperatureToXYZ )
{
    Metadata       metadata;
    DNGIdt        *di     = new DNGIdt( metadata );
    double         cct    = 6500.0;
    double         XYZ[3] = { 0.3135279229, 0.3235340821, 0.3629379950 };
    vector<double> result = di->colorTemperatureToXYZ( cct );
    delete di;

    FORI( countSize( XYZ ) )
    BOOST_CHECK_CLOSE( result[i], XYZ[i], 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIDT_MatrixRGBtoXYZ )
{
    Metadata       metadata;
    DNGIdt        *di     = new DNGIdt( metadata );
    double         XYZ[9] = { 0.952552395938, 0.000000000000, 0.000093678632,
                              0.343966449765, 0.728166096613, -0.072132546379,
                              0.000000000000, 0.000000000000, 1.008825184352 };
    vector<double> result = di->matrixRGBtoXYZ( chromaticitiesACES );
    delete di;

    FORI( countSize( XYZ ) )
    BOOST_CHECK_CLOSE( result[i], XYZ[i], 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIDT_GetDNGCATMatrix )
{

    LibRaw                rawProcessor;
    std::filesystem::path pathToRaw = std::filesystem::absolute(
        "../../unittest/materials/blackmagic_cinema_camera_cinemadng.dng" );
    int ret = rawProcessor.open_file( ( pathToRaw.string() ).c_str() );
    ret     = rawProcessor.unpack();

    Metadata metadata;
    init_metadata( rawProcessor, metadata );
    DNGIdt *di           = new DNGIdt( metadata );
    double  matrix[3][3] = { { 0.9907763427, -0.0022862289, 0.0209908807 },
                             { -0.0017882434, 0.9941341374, 0.0083008330 },
                             { 0.0003777587, 0.0015609315, 1.1063201101 } };
    vector<vector<double>> result = di->getDNGCATMatrix();

    rawProcessor.recycle();
    delete di;

    FORIJ( 3, 3 )
    BOOST_CHECK_CLOSE( result[i][j], matrix[i][j], 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIDT_GetDNGIDTMatrix )
{

    LibRaw                rawProcessor;
    std::filesystem::path pathToRaw = std::filesystem::absolute(
        "../../unittest/materials/blackmagic_cinema_camera_cinemadng.dng" );
    int ret = rawProcessor.open_file( ( pathToRaw.string() ).c_str() );
    ret     = rawProcessor.unpack();

    Metadata metadata;
    init_metadata( rawProcessor, metadata );
    DNGIdt *di           = new DNGIdt( metadata );
    double  matrix[3][3] = { { 1.0536466144, 0.0039044182, 0.0049084502 },
                             { -0.4899562165, 1.3614787986, 0.1020844728 },
                             { -0.0024498461, 0.0060497128, 1.0139159537 } };
    vector<vector<double>> result = di->getDNGIDTMatrix();

    rawProcessor.recycle();
    delete di;

    FORIJ( 3, 3 )
    BOOST_CHECK_CLOSE( result[i][j], matrix[i][j], 1e-5 );
};
