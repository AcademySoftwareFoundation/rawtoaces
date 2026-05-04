// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include <filesystem>
#include <OpenImageIO/unittest.h>

#include <rawtoaces/rawtoaces_core.h>
#include "../src/rawtoaces_core/define.h"
#include "../src/rawtoaces_core/mathOps.h"
#include "../src/rawtoaces_core/rawtoaces_core_priv.h"
#include "test_utils.h"

void testIDT_CcttoMired()
{
    double cct_kelvin = 6500.0;
    double mired      = rta::core::kelvin_to_mired( cct_kelvin );
    OIIO_CHECK_EQUAL_THRESH( mired, 153.8461538462, 1e-5 );
}

void testIDT_RobertsonLength()
{
    double              uv[]  = { 0.2042589852, 0.3196233991 };
    double              uvt[] = { 0.1800600000, 0.2635200000, -0.2434100000 };
    std::vector<double> uvVector( uv, uv + 2 );
    std::vector<double> uvtVector( uvt, uvt + 3 );
    double rLength = rta::core::robertson_length( uvVector, uvtVector );
    OIIO_CHECK_EQUAL_THRESH( rLength, 0.060234937, 1e-5 );
}

void testIDT_LightSourceToColorTemp()
{
    unsigned short tag = 17;
    double         ct  = rta::core::light_source_to_color_temp( tag );
    OIIO_CHECK_EQUAL_THRESH( ct, 2856.0, 1e-5 );
}

void testIDT_LightSourceToColorTemp_Extended()
{
    /// Test extended value (tag >= 32768): tag 32768 should return 0.0
    unsigned short tag = 32768;
    double         ct  = rta::core::light_source_to_color_temp( tag );
    OIIO_CHECK_EQUAL_THRESH( ct, 0.0, 1e-5 );

    /// Test extended value: tag 37768 (32768 + 5000) should return 5000.0
    tag = 37768;
    ct  = rta::core::light_source_to_color_temp( tag );
    OIIO_CHECK_EQUAL_THRESH( ct, 5000.0, 1e-5 );

    /// Test extended value: tag 40000 (32768 + 7232) should return 7232.0
    tag = 40000;
    ct  = rta::core::light_source_to_color_temp( tag );
    OIIO_CHECK_EQUAL_THRESH( ct, 7232.0, 1e-5 );
}

void testIDT_LightSourceToColorTemp_Default()
{
    /// Test unknown tag values fall back to daylight default (5500K).
    unsigned short tag = 999;
    double         ct  = rta::core::light_source_to_color_temp( tag );
    OIIO_CHECK_EQUAL_THRESH( ct, 5500.0, 1e-5 );
}

static const std::vector<double> k_calibration_0_xyz_to_rgb = {
    1.3119699954986572,   -0.49678999185562134, 0.011559999547898769,
    -0.41723001003265381, 1.4423700571060181,   0.045279998332262039,
    0.067230001091957092, 0.21709999442100525,  0.72650998830795288
};

static const std::vector<double> k_calibration_1_xyz_to_rgb = {
    1.0088499784469604,    -0.27351000905036926, -0.082580000162124634,
    -0.48996999859809875,  1.3444099426269531,   0.11174000054597855,
    -0.064060002565383911, 0.32997000217437744,  0.5391700267791748
};

static const std::vector<double> k_identity_xyz_to_rgb = { 1.0, 0.0, 0.0,
                                                           0.0, 1.0, 0.0,
                                                           0.0, 0.0, 1.0 };

void testIDT_XYZToColorTemperature()
{
    double              XYZ[3] = { 0.9731171910, 1.0174927152, 0.9498565880 };
    std::vector<double> XYZVector( XYZ, XYZ + 3 );
    double              cct = rta::core::XYZ_to_color_temperature( XYZVector );

    OIIO_CHECK_EQUAL_THRESH( cct, 5564.6648479019, 1e-5 );
}

void testIDT_XYZToColorTemperature_UpperClamp()
{
    /// UV exactly at the first Robertson entry should clamp to 50000K.
    const double        huge_cct = 1.0e16;
    std::vector<double> XYZ = rta::core::color_temperature_to_XYZ( huge_cct );
    double              cct = rta::core::XYZ_to_color_temperature( XYZ );

    OIIO_CHECK_EQUAL_THRESH( cct, 50000.0, 1e-5 );
}

void testIDT_XYZToColorTemperature_LowerClamp()
{
    /// UV slightly above the Robertson table should clamp to 2000K.
    std::vector<double> XYZ =
        rta::core::uv_to_XYZ( std::vector<double>{ 0.34, 0.361 } );
    double cct = rta::core::XYZ_to_color_temperature( XYZ );

    OIIO_CHECK_EQUAL_THRESH( cct, 2000.0, 1e-5 );
}

void testIDT_XYZtoCameraWeightedMatrix()
{
    rta::core::Metadata metadata;
    init_metadata( metadata );
    rta::core::MetadataSolver *di = new rta::core::MetadataSolver( metadata );
    double mirs[3]      = { 158.8461538462, 350.1400560224, 153.8461538462 };
    double matrix[3][3] = { { 1.0165710542, -0.2791973987, -0.0801820653 },
                            { -0.4881171650, 1.3469051835, 0.1100471308 },
                            { -0.0607157824, 0.3270949763, 0.5439419519 } };

    const std::vector<double> &matrix1 =
        metadata.calibration[0].XYZ_to_RGB_matrix;
    const std::vector<double> &matrix2 =
        metadata.calibration[1].XYZ_to_RGB_matrix;

    std::vector<std::vector<double>> result =
        rta::core::XYZ_to_camera_weighted_matrix(
            mirs[0], mirs[1], mirs[2], matrix1, matrix2 );
    delete di;

    OIIO_CHECK_EQUAL( result.size(), 3 );
    for ( size_t row = 0; row < 3; row++ )
    {
        OIIO_CHECK_EQUAL( result[row].size(), 3 );
        for ( size_t col = 0; col < 3; col++ )
        {
            OIIO_CHECK_EQUAL_THRESH( result[row][col], matrix[row][col], 1e-5 );
        }
    }
}

void check_DNG_matrix(
    const rta::core::Metadata              &metadata,
    std::vector<double>                    &neutralRGB,
    bool                                    expected_result,
    const std::vector<std::vector<double>> &expected_matrix,
    const std::string                      &expected_output )
{
    std::vector<std::vector<double>> camera_to_XYZ_matrix;
    bool                             result;
    std::string                      stderr_output = capture_stderr( [&]() {
        result = rta::core::find_camera_to_XYZ_matrix(
            metadata, neutralRGB, camera_to_XYZ_matrix );
    } );

    OIIO_CHECK_EQUAL( result, expected_result );
    if ( !expected_output.empty() )
        OIIO_CHECK_EQUAL( stderr_output, expected_output );

    if ( result )
    {
        OIIO_CHECK_EQUAL( camera_to_XYZ_matrix.size(), 3 );
        for ( size_t row = 0; row < 3; row++ )
        {
            OIIO_CHECK_EQUAL( camera_to_XYZ_matrix[row].size(), 3 );
            for ( size_t col = 0; col < 3; col++ )
            {
                OIIO_CHECK_EQUAL_THRESH(
                    camera_to_XYZ_matrix[row][col],
                    expected_matrix[row][col],
                    1e-5 );
            }
        }
    }
    else
    {
        OIIO_CHECK_EQUAL( camera_to_XYZ_matrix.size(), 0 );
    }
}

void testIDT_FindCameraToXYZMtx()
{
    rta::core::Metadata metadata;
    init_metadata( metadata );
    std::vector<double> neutralRGB = { 0.6289999865,
                                       1.0000000000,
                                       0.7904000305 };

    std::vector<std::vector<double>> expected_matrix = {
        { 1.0616656923, -0.3124143737, -0.0661770211 },
        { -0.4772957633, 1.3614785395, 0.1001599918 },
        { -0.0411839968, 0.3103035015, 0.5718121924 }
    };

    expected_matrix = rta::core::invertVM( expected_matrix );

    check_DNG_matrix( metadata, neutralRGB, true, expected_matrix, "" );
}

void testIDT_FindCameraToXYZMtx_NoIlluminant()
{
    rta::core::Metadata metadata;
    init_metadata( metadata );
    metadata.calibration[0].illuminant = 0;

    std::vector<double> neutralRGB = { 0.5, 0.5, 0.5 };

    // The expected result is the inverse of the first camera matrix
    auto m1 = metadata.calibration[0].XYZ_to_RGB_matrix;
    auto m2 = rta::core::stack_rows( m1, 3 );
    auto m3 = rta::core::invertVM( m2 );

    check_DNG_matrix(
        metadata,
        neutralRGB,
        true,
        m3,
        "No calibration illuminants were found.\n" );
}

void testIDT_FindCameraToXYZMtx_EmptyNeutral()
{
    rta::core::Metadata metadata;
    init_metadata( metadata );

    std::vector<double> neutralRGB = {};

    // The expected result is the inverse of the first camera matrix
    auto m1 = metadata.calibration[0].XYZ_to_RGB_matrix;
    auto m2 = rta::core::stack_rows( m1, 3 );
    auto m3 = rta::core::invertVM( m2 );

    check_DNG_matrix(
        metadata, neutralRGB, true, m3, "No neutral RGB values were found.\n" );
}

void testIDT_FindCameraToXYZMtx_ExactMatchMired()
{
    rta::core::Metadata metadata;
    init_metadata( metadata );
    metadata.calibration[0].XYZ_to_RGB_matrix = k_identity_xyz_to_rgb;
    metadata.calibration[1].XYZ_to_RGB_matrix = k_identity_xyz_to_rgb;
    metadata.calibration[1].illuminant        = 32768 + 10000;

    std::vector<double> neutral_RGB = { 0.97347064038736957,
                                        1.0,
                                        1.4953965764168315 };

    check_DNG_matrix(
        metadata,
        neutral_RGB,
        true,
        rta::core::stack_rows( k_identity_xyz_to_rgb, 3 ),
        "" );
}

void testIDT_FindCameraToXYZMtx_Fail()
{
    std::vector<double> non_invertible_mat = { 1, 0, 0, 1, 0, 0, 1, 0, 0 };

    std::vector<std::vector<double>> empty_mat;

    rta::core::Metadata metadata;
    init_metadata( metadata );
    metadata.calibration[0].XYZ_to_RGB_matrix = non_invertible_mat;
    metadata.calibration[1].XYZ_to_RGB_matrix = non_invertible_mat;
    metadata.calibration[1].illuminant        = 32768 + 10000;

    std::vector<double> neutral_RGB = { 0.97347064038736957,
                                        1.0,
                                        1.4953965764168315 };

    check_DNG_matrix(
        metadata,
        neutral_RGB,
        false,
        empty_mat,
        "Failed to find a suitable illuminant.\n" );
}
void testIDT_ColorTemperatureToXYZ()
{
    double              cct    = 6500.0;
    double              XYZ[3] = { 0.3135279229, 0.3235340821, 0.3629379950 };
    std::vector<double> result = rta::core::color_temperature_to_XYZ( cct );

    for ( int i = 0; i < countSize( XYZ ); i++ )
        OIIO_CHECK_EQUAL_THRESH( result[i], XYZ[i], 1e-5 );
}

void testIDT_ColorTemperatureToXYZ_ClampHighMired()
{
    /// Mired below the Robertson table should clamp to the last entry.
    const double        cct    = 200.0;
    std::vector<double> result = rta::core::color_temperature_to_XYZ( cct );
    std::vector<double> expected =
        rta::core::uv_to_XYZ( std::vector<double>{ 0.33724, 0.36051 } );

    for ( int i = 0; i < countSize( expected ); i++ )
        OIIO_CHECK_EQUAL_THRESH( result[i], expected[i], 1e-5 );
}

void testIDT_GetCameraXYZWhitePoint_UsesIlluminantWhenNeutralEmpty()
{
    rta::core::Metadata metadata;
    init_metadata( metadata );
    metadata.neutral_RGB.clear();

    std::vector<std::vector<double>> camera_to_XYZ;
    std::vector<double>              camera_XYZ_white_point;
    std::string                      stderr_output = capture_stderr( [&]() {
        rta::core::get_camera_XYZ_matrix_and_white_point(
            metadata, camera_to_XYZ, camera_XYZ_white_point );
    } );

    // Expect illuminant 17 fallback (normalized Y=1.0) when neutral_RGB is empty.
    std::vector<double> expected = { 1.098445424569, 1.0, 0.355920076967 };
    OIIO_CHECK_EQUAL( stderr_output, "No neutral RGB values were found.\n" );
    for ( int i = 0; i < countSize( expected ); i++ )
        OIIO_CHECK_EQUAL_THRESH( camera_XYZ_white_point[i], expected[i], 1e-5 );
}

void testIDT_MatrixRGBtoXYZ()
{
    double XYZ[9] = { 0.952552395938, 0.000000000000, 0.000093678632,
                      0.343966449765, 0.728166096613, -0.072132546379,
                      0.000000000000, 0.000000000000, 1.008825184352 };
    std::vector<double> result =
        rta::core::matrix_RGB_to_XYZ( rta::core::chromaticitiesACES );

    for ( int i = 0; i < countSize( XYZ ); i++ )
        OIIO_CHECK_EQUAL_THRESH( result[i], XYZ[i], 1e-5 );
}

void testIDT_GetDNGCATMatrix()
{
    rta::core::Metadata metadata;
    init_metadata( metadata );
    rta::core::MetadataSolver *di = new rta::core::MetadataSolver( metadata );
    double matrix[3][3] = { { 0.9907763427, -0.0022862289, 0.0209908807 },
                            { -0.0017882434, 0.9941341374, 0.0083008330 },
                            { 0.0003777587, 0.0015609315, 1.1063201101 } };
    std::vector<std::vector<double>> result = di->calculate_CAT_matrix();

    delete di;

    for ( size_t i = 0; i < 3; i++ )
        for ( size_t j = 0; j < 3; j++ )
            OIIO_CHECK_EQUAL_THRESH( result[i][j], matrix[i][j], 1e-5 );
}

void testIDT_GetDNGIDTMatrix()
{
    rta::core::Metadata metadata;
    init_metadata( metadata );
    rta::core::MetadataSolver *di = new rta::core::MetadataSolver( metadata );
    double matrix[3][3] = { { 1.0536466144, 0.0039044182, 0.0049084502 },
                            { -0.4899562165, 1.3614787986, 0.1020844728 },
                            { -0.0024498461, 0.0060497128, 1.0139159537 } };
    std::vector<std::vector<double>> result = di->calculate_IDT_matrix();

    delete di;

    for ( size_t i = 0; i < 3; i++ )
        for ( size_t j = 0; j < 3; j++ )
            OIIO_CHECK_EQUAL_THRESH( result[i][j], matrix[i][j], 1e-5 );
}

int main( int, char ** )
{
    testIDT_CcttoMired();
    testIDT_RobertsonLength();
    testIDT_LightSourceToColorTemp();
    testIDT_LightSourceToColorTemp_Extended();
    testIDT_LightSourceToColorTemp_Default();
    testIDT_XYZToColorTemperature();
    testIDT_XYZToColorTemperature_UpperClamp();
    testIDT_XYZToColorTemperature_LowerClamp();
    testIDT_XYZtoCameraWeightedMatrix();
    testIDT_FindCameraToXYZMtx();
    testIDT_FindCameraToXYZMtx_NoIlluminant();
    testIDT_FindCameraToXYZMtx_EmptyNeutral();
    testIDT_FindCameraToXYZMtx_ExactMatchMired();
    testIDT_FindCameraToXYZMtx_Fail();
    testIDT_ColorTemperatureToXYZ();
    testIDT_ColorTemperatureToXYZ_ClampHighMired();
    testIDT_GetCameraXYZWhitePoint_UsesIlluminantWhenNeutralEmpty();
    testIDT_MatrixRGBtoXYZ();
    testIDT_GetDNGCATMatrix();
    testIDT_GetDNGIDTMatrix();

    return unit_test_failures;
}
