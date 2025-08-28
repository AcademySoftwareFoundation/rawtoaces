// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <filesystem>
#include <boost/test/tools/floating_point_comparison.hpp>

#include <rawtoaces/rawtoaces_core.h>
#include <rawtoaces/define.h>
#include "../src/rawtoaces_core/rawtoaces_core_priv.h"

BOOST_AUTO_TEST_CASE( TestIDT_CcttoMired )
{
    double cct   = 6500.0;
    double mired = rta::core::ccttoMired( cct );
    BOOST_CHECK_CLOSE( mired, 153.8461538462, 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIDT_RobertsonLength )
{
    double              uv[]  = { 0.2042589852, 0.3196233991 };
    double              uvt[] = { 0.1800600000, 0.2635200000, -0.2434100000 };
    std::vector<double> uvVector( uv, uv + 2 );
    std::vector<double> uvtVector( uvt, uvt + 3 );
    double rLength = rta::core::robertsonLength( uvVector, uvtVector );
    BOOST_CHECK_CLOSE( rLength, 0.060234937, 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIDT_LightSourceToColorTemp )
{
    unsigned short tag = 17;
    double         ct  = rta::core::lightSourceToColorTemp( tag );
    BOOST_CHECK_CLOSE( ct, 2856.0, 1e-5 );
};

void init_metadata( rta::core::Metadata &metadata )
{
    metadata.baseline_exposure = 2.4;

    metadata.neutral_RGB = { 0.6289999865031245, 1, 0.79040003045288199 };

    metadata.calibration[0].illuminant = 17;
    metadata.calibration[1].illuminant = 21;

    metadata.calibration[0].XYZ_to_RGB_matrix = {
        1.3119699954986572,   -0.49678999185562134, 0.011559999547898769,
        -0.41723001003265381, 1.4423700571060181,   0.045279998332262039,
        0.067230001091957092, 0.21709999442100525,  0.72650998830795288
    };

    metadata.calibration[1].XYZ_to_RGB_matrix = {
        1.0088499784469604,    -0.27351000905036926, -0.082580000162124634,
        -0.48996999859809875,  1.3444099426269531,   0.11174000054597855,
        -0.064060002565383911, 0.32997000217437744,  0.5391700267791748
    };
}

BOOST_AUTO_TEST_CASE( TestIDT_XYZToColorTemperature )
{
    double              XYZ[3] = { 0.9731171910, 1.0174927152, 0.9498565880 };
    std::vector<double> XYZVector( XYZ, XYZ + 3 );
    double              cct = rta::core::XYZToColorTemperature( XYZVector );
    BOOST_CHECK_CLOSE( cct, 5564.6648479019, 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIDT_XYZtoCameraWeightedMatrix )
{
    rta::core::Metadata metadata;
    init_metadata( metadata );
    rta::core::MetadataSolver *di = new rta::core::MetadataSolver( metadata );
    double mirs[3]   = { 158.8461538462, 350.1400560224, 153.8461538462 };
    double matrix[9] = { 1.0165710542,  -0.2791973987, -0.0801820653,
                         -0.4881171650, 1.3469051835,  0.1100471308,
                         -0.0607157824, 0.3270949763,  0.5439419519 };

    const std::vector<double> &matrix1 =
        metadata.calibration[0].XYZ_to_RGB_matrix;
    const std::vector<double> &matrix2 =
        metadata.calibration[1].XYZ_to_RGB_matrix;

    std::vector<double> result = rta::core::XYZtoCameraWeightedMatrix(
        mirs[0], mirs[1], mirs[2], matrix1, matrix2 );
    delete di;

    FORI( countSize( matrix ) )
    BOOST_CHECK_CLOSE( result[i], matrix[i], 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIDT_FindXYZtoCameraMtx )
{
    rta::core::Metadata metadata;
    init_metadata( metadata );
    rta::core::MetadataSolver *di = new rta::core::MetadataSolver( metadata );
    double neutralRGB[3] = { 0.6289999865, 1.0000000000, 0.7904000305 };
    double matrix[9]     = { 1.0616656923,  -0.3124143737, -0.0661770211,
                             -0.4772957633, 1.3614785395,  0.1001599918,
                             -0.0411839968, 0.3103035015,  0.5718121924 };
    std::vector<double> neutralRGBVector( neutralRGB, neutralRGB + 3 );
    std::vector<double> result =
        rta::core::findXYZtoCameraMtx( metadata, neutralRGBVector );

    delete di;

    FORI( countSize( matrix ) )
    BOOST_CHECK_CLOSE( result[i], matrix[i], 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIDT_ColorTemperatureToXYZ )
{
    double              cct    = 6500.0;
    double              XYZ[3] = { 0.3135279229, 0.3235340821, 0.3629379950 };
    std::vector<double> result = rta::core::colorTemperatureToXYZ( cct );

    FORI( countSize( XYZ ) )
    BOOST_CHECK_CLOSE( result[i], XYZ[i], 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIDT_MatrixRGBtoXYZ )
{
    double XYZ[9] = { 0.952552395938, 0.000000000000, 0.000093678632,
                      0.343966449765, 0.728166096613, -0.072132546379,
                      0.000000000000, 0.000000000000, 1.008825184352 };
    std::vector<double> result =
        rta::core::matrixRGBtoXYZ( rta::core::chromaticitiesACES );

    FORI( countSize( XYZ ) )
    BOOST_CHECK_CLOSE( result[i], XYZ[i], 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIDT_GetDNGCATMatrix )
{
    rta::core::Metadata metadata;
    init_metadata( metadata );
    rta::core::MetadataSolver *di = new rta::core::MetadataSolver( metadata );
    double matrix[3][3] = { { 0.9907763427, -0.0022862289, 0.0209908807 },
                            { -0.0017882434, 0.9941341374, 0.0083008330 },
                            { 0.0003777587, 0.0015609315, 1.1063201101 } };
    std::vector<std::vector<double>> result = di->calculate_CAT_matrix();

    delete di;

    FORIJ( 3, 3 )
    BOOST_CHECK_CLOSE( result[i][j], matrix[i][j], 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIDT_GetDNGIDTMatrix )
{
    rta::core::Metadata metadata;
    init_metadata( metadata );
    rta::core::MetadataSolver *di = new rta::core::MetadataSolver( metadata );
    double matrix[3][3] = { { 1.0536466144, 0.0039044182, 0.0049084502 },
                            { -0.4899562165, 1.3614787986, 0.1020844728 },
                            { -0.0024498461, 0.0060497128, 1.0139159537 } };
    std::vector<std::vector<double>> result = di->calculate_IDT_matrix();

    delete di;

    FORIJ( 3, 3 )
    BOOST_CHECK_CLOSE( result[i][j], matrix[i][j], 1e-5 );
};
