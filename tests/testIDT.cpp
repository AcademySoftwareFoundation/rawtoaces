// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#endif

#include <filesystem>
#include <OpenImageIO/unittest.h>

#include "../src/rawtoaces_core/core_math.h"
#include <rawtoaces/rawtoaces_core.h>
#include "../src/rawtoaces_core/rawtoaces_core_priv.h"
#include "test_utils.h"
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <vector>
#include "../src/misc/pragma.h"

#define DATA_PATH "../_deps/rawtoaces_data-src/data/"

// FNV-1a 64-bit over IEEE754 little-endian bytes — deterministic in CI.
static std::string fingerprint_doubles( const double *values, size_t count )
{
    const uint64_t FNV_OFFSET = 14695981039346656037ull;
    const uint64_t FNV_PRIME  = 1099511628211ull;
    uint64_t       hash       = FNV_OFFSET;
    for ( size_t i = 0; i < count; i++ )
    {
        double        v = values[i];
        unsigned char bytes[sizeof( double )];
        std::memcpy( bytes, &v, sizeof( double ) );
        for ( size_t b = 0; b < sizeof( double ); b++ )
        {
            hash ^= bytes[b];
            hash *= FNV_PRIME;
        }
    }
    std::ostringstream oss;
    oss << std::hex << std::setfill( '0' ) << std::setw( 16 ) << hash;
    return oss.str();
}

static std::string fingerprint_doubles( const std::vector<double> &values )
{
    return fingerprint_doubles( values.data(), values.size() );
}


void testIDT_LoadCameraSpst()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    std::filesystem::path absolutePath =
        std::filesystem::absolute( DATA_PATH "camera/ARRI_D21_380_780_5.json" );

    rta::core::SpectralData camera;
    bool                    result;

    result = camera.load( absolutePath.string() );
    OIIO_CHECK_ASSERT( result );
    OIIO_CHECK_EQUAL( camera.manufacturer, "ARRI" );
    OIIO_CHECK_EQUAL( camera.model, "D21" );
    OIIO_CHECK_EQUAL( camera.data.size(), 1 );
    OIIO_CHECK_EQUAL( camera.data.count( "main" ), 1 );
    OIIO_CHECK_EQUAL( camera.data.at( "main" ).size(), 3 );

    // Spot-check first/last samples per channel (full table is in DATA_PATH JSON).
    const std::string channels[3] = { "R", "G", "B" };
    const double head[3][2] = {
        { 0.000188205, 0.000440222 },
        { 8.59e-05, 0.000166118 },
        { 9.58e-05, 0.000258734 }
    };
    const double tail[3][2] = {
        { 0.000149065, 3.71e-05 },
        { 7.26e-05, 0 },
        { 5.84e-05, 2.7e-06 }
    };
    for ( size_t i = 0; i < 3; i++ )
    {
        const rta::core::Spectrum &spectrum = camera[channels[i]];
        OIIO_CHECK_EQUAL( spectrum.shape.first, 380 );
        OIIO_CHECK_EQUAL( spectrum.shape.last, 780 );
        OIIO_CHECK_EQUAL( spectrum.shape.step, 5 );
        OIIO_CHECK_EQUAL( spectrum.values.size(), 81 );
        OIIO_CHECK_EQUAL_THRESH( spectrum.values[0], head[i][0], 1e-5 );
        OIIO_CHECK_EQUAL_THRESH( spectrum.values[1], head[i][1], 1e-5 );
        OIIO_CHECK_EQUAL_THRESH( spectrum.values[79], tail[i][0], 1e-5 );
        OIIO_CHECK_EQUAL_THRESH( spectrum.values[80], tail[i][1], 1e-5 );
    }
}

void testIDT_LoadIlluminant()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    std::filesystem::path absolutePath = std::filesystem::absolute(
        DATA_PATH "illuminant/iso7589_stutung_380_780_5.json" );

    bool                    result;
    rta::core::SpectralData illuminant;
    result = illuminant.load( absolutePath.string() );
    OIIO_CHECK_ASSERT( result );

    // Spot-check illuminant fixture samples (full curve is on disk).
    OIIO_CHECK_EQUAL( illuminant.type, "ISO7589" );
    OIIO_CHECK_EQUAL( illuminant["power"].shape.step, 5 );

    std::vector<double> &illumTestData = illuminant["power"].values;
    OIIO_CHECK_EQUAL( illumTestData.size(), 81 );
    OIIO_CHECK_EQUAL_THRESH( illumTestData[0], 0.04, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumTestData[1], 0.05, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumTestData[40], 0.555, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumTestData[79], 0.9925, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumTestData[80], 1, 1e-5 );
}

void testIDT_LoadTrainingData()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    std::filesystem::path absolutePath = std::filesystem::absolute(
        DATA_PATH "training/training_spectral.json" );

    bool                    result;
    rta::core::SpectralData training_data;
    result = training_data.load( absolutePath.string() );
    OIIO_CHECK_ASSERT( result );

    // Spot-check training spectral patches (fixture JSON is authoritative).
    OIIO_CHECK_EQUAL( training_data["patch1"].values.size(), 81 );
    OIIO_CHECK_EQUAL( training_data["patch2"].values.size(), 81 );
    OIIO_CHECK_EQUAL( training_data["patch3"].values.size(), 81 );
    OIIO_CHECK_EQUAL_THRESH( training_data["patch1"].values[0], 0.06, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( training_data["patch2"].values[0], 0.0649, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( training_data["patch3"].values[0], 0.1365, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( training_data["patch1"].values[40], 0.017, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( training_data["patch2"].values[40], 0.0906, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( training_data["patch3"].values[40], 0.7788, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( training_data["patch1"].values[80], 0.8866, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( training_data["patch2"].values[80], 0.0808, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( training_data["patch3"].values[80], 0.7809, 1e-5 );
}

void testIDT_LoadCMF()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    std::filesystem::path absolutePath =
        std::filesystem::absolute( DATA_PATH "cmf/cmf_1931.json" );

    bool                    result;
    rta::core::SpectralData spectral_data;
    result = spectral_data.load( absolutePath.string() );
    OIIO_CHECK_ASSERT( result );

    // Spot-check CMF at a few wavelengths (1 nm source; loader exposes 5 nm).
    OIIO_CHECK_EQUAL( spectral_data["X"].values.size(), 81 );
    OIIO_CHECK_EQUAL( spectral_data["Y"].values.size(), 81 );
    OIIO_CHECK_EQUAL( spectral_data["Z"].values.size(), 81 );
    OIIO_CHECK_EQUAL_THRESH( spectral_data["X"].values[0], 0.001368, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( spectral_data["Y"].values[0], 3.9e-05, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( spectral_data["Z"].values[0], 0.006450001, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( spectral_data["X"].values[40], 0.9163, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( spectral_data["Y"].values[40], 0.87, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( spectral_data["Z"].values[40], 0.001650001, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( spectral_data["X"].values[80], 4.15e-05, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( spectral_data["Y"].values[80], 1.5e-05, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( spectral_data["Z"].values[80], 0, 1e-5 );
}


void load_camera_helper(
    rta::core::SpectralSolver &solver,
    const std::string         &camera_make,
    const std::string         &camera_model,
    const std::string         &illuminant_name,
    bool                       load_training,
    bool                       load_observer )
{

    {
        bool result = solver.find_camera( camera_make, camera_model );
        OIIO_CHECK_ASSERT( result );
    }

    if ( !illuminant_name.empty() )
    {
        bool result = solver.find_illuminant( illuminant_name );
        OIIO_CHECK_ASSERT( result );
    }

    if ( load_training )
    {
        bool result = solver.load_spectral_data(
            "training/training_spectral.json", solver.training_data );
        OIIO_CHECK_ASSERT( result );
    }

    if ( load_observer )
    {
        bool result =
            solver.load_spectral_data( "cmf/cmf_1931.json", solver.observer );
        OIIO_CHECK_ASSERT( result );
    }
}

void load_file( const std::string &path, rta::core::SpectralData &data )
{
    std::filesystem::path full_path =
        std::filesystem::absolute( DATA_PATH + path );
    OIIO_CHECK_ASSERT( data.load( full_path.string() ) );
}

void testIDT_scaleLSC()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::core::SpectralData illuminant;
    load_file( "illuminant/iso7589_stutung_380_780_5.json", illuminant );

    rta::core::SpectralData camera;
    load_file( "camera/Nikon_D200_380_780_5.json", camera );

    scale_illuminant( camera, illuminant );

    const std::vector<double> illumDataScaled = illuminant["power"].values;

    OIIO_CHECK_EQUAL( illumDataScaled.size(), 81 );
    OIIO_CHECK_EQUAL( illuminant.type, "ISO7589" );
    OIIO_CHECK_EQUAL( illuminant["power"].shape.step, 5 );
    OIIO_CHECK_EQUAL_THRESH( illumDataScaled[0], 0.00546219526, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumDataScaled[1], 0.00682774407, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumDataScaled[40], 0.07578795922, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumDataScaled[79], 0.1355307199, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumDataScaled[80], 0.1365548815, 1e-5 );
    OIIO_CHECK_EQUAL( fingerprint_doubles( illumDataScaled ), "0c3e6122ab096e39" );
}

void testIDT_CalCM()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::core::SpectralData illuminant;
    load_file( "illuminant/iso7589_stutung_380_780_5.json", illuminant );

    rta::core::SpectralData camera;
    load_file( "camera/ARRI_D21_380_780_5.json", camera );

    std::vector<double> CM_test = calculate_CM( camera, illuminant );

    float CM[81] = { 1.0000000000f, 1.4418439699f, 1.8703081160f };

    for ( int i = 0; i < 3; i++ )
        OIIO_CHECK_EQUAL_THRESH( CM[i], CM_test[i], 1e-5 );
}

void testIDT_CalWB()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::core::SpectralData illuminant;
    load_file( "illuminant/iso7589_stutung_380_780_5.json", illuminant );

    rta::core::SpectralData camera;
    load_file( "camera/Nikon_D200_380_780_5.json", camera );

    std::vector<double> WB_test = _calculate_WB( camera, illuminant );

    double WB[3] = { 1.1397265, 1.0000000, 2.3240151 };
    for ( size_t i = 0; i < WB_test.size(); i++ )
    {
        OIIO_CHECK_EQUAL_THRESH( WB[i], WB_test[i], 1e-5 );
    }
}

void testIDT_ChooseIllumSrc()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::core::SpectralSolver solver( { DATA_PATH } );
    load_camera_helper( solver, "nikon", "d200", "", true, false );

    float               wb[3] = { 1.0, 1.0, 1.0 };
    std::vector<double> wbv( wb, wb + 3 );
    solver.find_illuminant( wbv );

    const auto         &best_illuminant = solver.illuminant;
    std::string         illumType_Test  = best_illuminant.type;
    std::vector<double> illumData_Test  = best_illuminant["power"].values;

    OIIO_CHECK_EQUAL( illumType_Test, "d45" );
    // Spot-check chosen illuminant curve + fingerprint.
    OIIO_CHECK_EQUAL( illumData_Test.size(), 81 );
    OIIO_CHECK_EQUAL_THRESH( illumData_Test[0], 0.0106671894, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumData_Test[40], 0.0601212712, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumData_Test[80], 0.0523097995, 1e-5 );
    OIIO_CHECK_EQUAL( fingerprint_doubles( illumData_Test ), "97f50d9b25c5a977" );
}

void testIDT_ChooseIllumType()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::core::SpectralSolver solver( { DATA_PATH } );
    load_camera_helper( solver, "nikon", "d200", "iso7589", true, false );

    float               wb[3] = { 1.0, 1.0, 1.0 };
    std::vector<double> wbv( wb, wb + 3 );
    solver.calculate_WB();

    const auto         &best_illuminant = solver.illuminant;
    std::string         illumType_Test  = best_illuminant.type;
    std::vector<double> illumData_Test  = best_illuminant["power"].values;

    OIIO_CHECK_EQUAL( illumType_Test, "ISO7589" );
    // Spot-check chosen illuminant curve + fingerprint.
    OIIO_CHECK_EQUAL( illumData_Test.size(), 81 );
    OIIO_CHECK_EQUAL_THRESH( illumData_Test[0], 0.0054621953, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumData_Test[40], 0.0757879592, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumData_Test[80], 0.1365548815, 1e-5 );
    OIIO_CHECK_EQUAL( fingerprint_doubles( illumData_Test ), "a3556bde9c77577c" );
}

void testIDT_CalTI()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::core::SpectralData camera;
    load_file( "camera/Nikon_D200_380_780_5.json", camera );

    rta::core::SpectralData illuminant;
    load_file( "illuminant/iso7589_stutung_380_780_5.json", illuminant );

    rta::core::SpectralData training_data;
    load_file( "training/training_spectral.json", training_data );

    scale_illuminant( camera, illuminant );
    auto TI_test = calculate_TI( illuminant, training_data );

    // Spot-checks + full-matrix fingerprint for TI (was an 81x190 literal).
    OIIO_CHECK_EQUAL( TI_test.size(), 190 );
    OIIO_CHECK_EQUAL( TI_test[0].values.size(), 81 );
    OIIO_CHECK_EQUAL_THRESH( TI_test[0].values[0], 0.0003277317, 1e-4 );
    OIIO_CHECK_EQUAL_THRESH( TI_test[0].values[40], 0.0012883953, 1e-4 );
    OIIO_CHECK_EQUAL_THRESH( TI_test[0].values[80], 0.1210695579, 1e-4 );
    OIIO_CHECK_EQUAL_THRESH( TI_test[189].values[0], 0, 1e-4 );
    OIIO_CHECK_EQUAL_THRESH( TI_test[189].values[80], 0.0936083713, 1e-4 );
    OIIO_CHECK_EQUAL_THRESH( TI_test[95].values[40], 0.018901517, 1e-4 );

    std::vector<double> ti_flat;
    ti_flat.reserve( 81 * 190 );
    for ( size_t j = 0; j < 81; j++ )
        for ( size_t i = 0; i < 190; i++ )
            ti_flat.push_back( TI_test[i].values[j] );
    OIIO_CHECK_EQUAL( fingerprint_doubles( ti_flat ), "756057abf4cd85e6" );
}

void testIDT_CalXYZ()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::core::SpectralData camera;
    load_file( "camera/Nikon_D200_380_780_5.json", camera );

    rta::core::SpectralData illuminant;
    load_file( "illuminant/iso7589_stutung_380_780_5.json", illuminant );

    rta::core::SpectralData training_data;
    load_file( "training/training_spectral.json", training_data );

    rta::core::SpectralData observer;
    load_file( "cmf/cmf_1931.json", observer );

    scale_illuminant( camera, illuminant );

    auto TI       = calculate_TI( illuminant, training_data );
    auto XYZ_test = calculate_XYZ( observer, illuminant, TI );

    // Spot-checks + fingerprint for XYZ transform output.
    OIIO_CHECK_EQUAL( XYZ_test.size(), 190 );
    OIIO_CHECK_EQUAL( XYZ_test[0].size(), 3 );
    OIIO_CHECK_EQUAL_THRESH( XYZ_test[0][0], 0.0179976319, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( XYZ_test[0][1], 0.0180404631, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( XYZ_test[0][2], 0.0195495429, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( XYZ_test[189][0], 0.460324829, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( XYZ_test[189][1], 0.3206987361, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( XYZ_test[189][2], 0.3960834297, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( XYZ_test[95][1], 0.2698135334, 1e-5 );

    std::vector<double> xyz_flat;
    xyz_flat.reserve( 190 * 3 );
    for ( size_t i = 0; i < 190; i++ )
        for ( size_t j = 0; j < 3; j++ )
            xyz_flat.push_back( XYZ_test[i][j] );
    OIIO_CHECK_EQUAL( fingerprint_doubles( xyz_flat ), "929acb75bdb781ae" );
}

void testIDT_CalRGB()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::core::SpectralData camera;
    load_file( "camera/Nikon_D200_380_780_5.json", camera );

    rta::core::SpectralData illuminant;
    load_file( "illuminant/iso7589_stutung_380_780_5.json", illuminant );

    rta::core::SpectralData training_data;
    load_file( "training/training_spectral.json", training_data );

    rta::core::SpectralData observer;
    load_file( "cmf/cmf_1931.json", observer );

    scale_illuminant( camera, illuminant );
    auto WB       = _calculate_WB( camera, illuminant );
    auto TI       = calculate_TI( illuminant, training_data );
    auto RGB_test = calculate_RGB( camera, WB, TI );

    // Spot-checks + fingerprint for RGB transform output.
    OIIO_CHECK_EQUAL( RGB_test.size(), 190 );
    OIIO_CHECK_EQUAL( RGB_test[0].size(), 3 );
    OIIO_CHECK_EQUAL_THRESH( RGB_test[0][0], 0.0202216733, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( RGB_test[0][1], 0.0193805976, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( RGB_test[0][2], 0.02422774, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( RGB_test[189][0], 0.5608623445, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( RGB_test[189][1], 0.2394587589, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( RGB_test[189][2], 0.3637261707, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( RGB_test[95][1], 0.3177618117, 1e-5 );

    std::vector<double> rgb_flat;
    rgb_flat.reserve( 190 * 3 );
    for ( size_t i = 0; i < 190; i++ )
        for ( size_t j = 0; j < 3; j++ )
            rgb_flat.push_back( RGB_test[i][j] );
    OIIO_CHECK_EQUAL( fingerprint_doubles( rgb_flat ), "d11b8987022e8e0b" );
}

void testIDT_CurveFit()
{
    rta::core::SpectralData camera;
    load_file( "camera/Nikon_D200_380_780_5.json", camera );

    rta::core::SpectralData illuminant;
    load_file( "illuminant/iso7589_stutung_380_780_5.json", illuminant );

    rta::core::SpectralData training_data;
    load_file( "training/training_spectral.json", training_data );

    rta::core::SpectralData observer;
    load_file( "cmf/cmf_1931.json", observer );

    scale_illuminant( camera, illuminant );
    auto WB  = _calculate_WB( camera, illuminant );
    auto TI  = calculate_TI( illuminant, training_data );
    auto XYZ = calculate_XYZ( observer, illuminant, TI );
    auto RGB = calculate_RGB( camera, WB, TI );

    std::vector<std::vector<double>> IDT_test( 3, std::vector<double>( 3 ) );

    OIIO_CHECK_ASSERT( rta::core::curveFit( RGB, XYZ, 0, IDT_test ) );

    float IDT[3][3] = { { 0.7447691479f, 0.1434200377f, 0.1118108144f },
                        { 0.0451759890f, 1.0082622042f, -0.0534381932f },
                        { 0.0247144012f, -0.1245524896f, 1.0998380884f } };

    for ( size_t i = 0; i < 3; i++ )
        for ( size_t j = 0; j < 3; j++ )
            OIIO_CHECK_EQUAL_THRESH( IDT[i][j], IDT_test[i][j], 1e-5 );
}

void testIDT_CalIDT()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::core::SpectralSolver solver( { DATA_PATH } );
    load_camera_helper( solver, "arri", "d21", "iso7589", true, true );
    solver.calculate_WB();

    DISABLE_DEPRECATED_WARNINGS
    OIIO_CHECK_ASSERT( solver.calculate_IDT_matrix() );
    std::vector<std::vector<double>> IDT_test = solver.get_IDT_matrix();
    ENABLE_WARNINGS

    float IDT[3][3] = { { 1.0915120600f, -0.2516916464f, 0.1601795864f },
                        { -0.0089998772f, 1.2147199060f, -0.2057200288f },
                        { -0.1312667887f, -0.7361633199f, 1.8674301085f } };

    for ( size_t i = 0; i < 3; i++ )
        for ( size_t j = 0; j < 3; j++ )
            OIIO_CHECK_EQUAL_THRESH( IDT[i][j], IDT_test[i][j], 1e-4 );
}

void testIDT_calculate_transform()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::core::SpectralSolver solver( { DATA_PATH } );
    load_camera_helper( solver, "arri", "d21", "iso7589", true, true );
    solver.calculate_WB();

    OIIO_CHECK_ASSERT( solver.calculate_transform() );
    std::vector<std::vector<double>> &IDT_test = solver.transform_matrix;

    float IDT[3][3] = { { 1.0915120600f, -0.2516916464f, 0.1601795864f },
                        { -0.0089998772f, 1.2147199060f, -0.2057200288f },
                        { -0.1312667887f, -0.7361633199f, 1.8674301085f } };

    for ( size_t i = 0; i < 3; i++ )
        for ( size_t j = 0; j < 3; j++ )
            OIIO_CHECK_EQUAL_THRESH( IDT[i][j], IDT_test[i][j], 1e-4 );
}

/// Helper function to test that calculate_transform returns false and sets expected error
static void check_calculate_transform_matrix_error(
    rta::core::SpectralSolver &solver, const std::string &expected_error )
{
    bool        success = solver.calculate_transform();
    std::string output  = solver.last_error_message;

    OIIO_CHECK_ASSERT( !success );
    ASSERT_CONTAINS( output, expected_error );
}

const std::string expected_error_camera_not_initialized =
    "Camera needs to be initialised prior to calling "
    "SpectralSolver::calculate_transform().";

/// Tests that calculate_transform returns false and sets error when camera is not initialized
void testIDT_calculate_transform_Camera_Not_Initialized()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::core::SpectralSolver solver( { DATA_PATH } );
    /// Camera is not initialized - leave it empty
    /// Initialize other components
    bool result = solver.find_illuminant( "iso7589" );
    OIIO_CHECK_ASSERT( result );
    result = solver.load_spectral_data(
        "training/training_spectral.json", solver.training_data );
    OIIO_CHECK_ASSERT( result );
    result = solver.load_spectral_data( "cmf/cmf_1931.json", solver.observer );
    OIIO_CHECK_ASSERT( result );
    /// Cannot call calculate_WB() because camera is not initialized

    check_calculate_transform_matrix_error(
        solver, expected_error_camera_not_initialized );
}

/// Tests that calculate_transform returns false and sets error when camera has wrong size
void testIDT_calculate_transform_Camera_Wrong_Size()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::core::SpectralSolver solver( { DATA_PATH } );
    /// Initialize camera with wrong size (2 channels instead of 3)
    bool result = solver.find_camera( "arri", "d21" );
    OIIO_CHECK_ASSERT( result );
    /// Remove one channel to make size != 3
    solver.camera.data["main"].pop_back();
    OIIO_CHECK_EQUAL( solver.camera.data["main"].size(), 2 );
    /// Initialize other components
    result = solver.find_illuminant( "iso7589" );
    OIIO_CHECK_ASSERT( result );
    result = solver.load_spectral_data(
        "training/training_spectral.json", solver.training_data );
    OIIO_CHECK_ASSERT( result );
    result = solver.load_spectral_data( "cmf/cmf_1931.json", solver.observer );
    OIIO_CHECK_ASSERT( result );
    solver.calculate_WB();

    check_calculate_transform_matrix_error(
        solver, expected_error_camera_not_initialized );
}

const std::string expected_error_illuminant_not_initialized =
    "Illuminant needs to be initialised prior to "
    "calling SpectralSolver::calculate_transform().";

/// Tests that calculate_transform returns false and sets error when illuminant is not initialized
void testIDT_calculate_transform_Illuminant_Not_Initialized()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::core::SpectralSolver solver( { DATA_PATH } );
    /// Initialize camera
    bool result = solver.find_camera( "arri", "d21" );
    OIIO_CHECK_ASSERT( result );
    /// Illuminant is not initialized - leave it empty
    /// Initialize other components
    result = solver.load_spectral_data(
        "training/training_spectral.json", solver.training_data );
    OIIO_CHECK_ASSERT( result );
    result = solver.load_spectral_data( "cmf/cmf_1931.json", solver.observer );
    OIIO_CHECK_ASSERT( result );
    solver.calculate_WB();

    check_calculate_transform_matrix_error(
        solver, expected_error_illuminant_not_initialized );
}

/// Tests that calculate_transform returns false and sets error when illuminant has wrong size
void testIDT_calculate_transform_Illuminant_Wrong_Size()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::core::SpectralSolver solver( { DATA_PATH } );
    /// Initialize camera
    bool result = solver.find_camera( "arri", "d21" );
    OIIO_CHECK_ASSERT( result );
    /// Initialize illuminant with wrong size (2 channels instead of 1)
    result = solver.find_illuminant( "iso7589" );
    OIIO_CHECK_ASSERT( result );
    /// Add an extra channel to make size != 1
    solver.illuminant.data["main"].emplace_back(
        std::make_pair( "extra", rta::core::Spectrum() ) );
    OIIO_CHECK_EQUAL( solver.illuminant.data["main"].size(), 2 );
    /// Initialize other components
    result = solver.load_spectral_data(
        "training/training_spectral.json", solver.training_data );
    OIIO_CHECK_ASSERT( result );
    result = solver.load_spectral_data( "cmf/cmf_1931.json", solver.observer );
    OIIO_CHECK_ASSERT( result );
    solver.calculate_WB();

    check_calculate_transform_matrix_error(
        solver, expected_error_illuminant_not_initialized );
}

const std::string expected_error_observer_not_initialized =
    "Observer needs to be initialised prior to calling "
    "SpectralSolver::calculate_transform().";

/// Tests that calculate_transform returns false and sets error when observer is not initialized
void testIDT_calculate_transform_Observer_Not_Initialized()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::core::SpectralSolver solver( { DATA_PATH } );
    /// Initialize camera, illuminant, but not observer
    load_camera_helper( solver, "arri", "d21", "iso7589", true, false );
    /// Clear observer data to simulate uninitialized state
    solver.observer.data.clear();
    solver.calculate_WB();

    check_calculate_transform_matrix_error(
        solver, expected_error_observer_not_initialized );
}

/// Tests that calculate_transform returns false and sets error when observer has wrong size
void testIDT_calculate_transform_Observer_Wrong_Size()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::core::SpectralSolver solver( { DATA_PATH } );
    /// Initialize camera, illuminant
    load_camera_helper( solver, "arri", "d21", "iso7589", true, false );
    /// Initialize observer with wrong size (2 channels instead of 3)
    bool result =
        solver.load_spectral_data( "cmf/cmf_1931.json", solver.observer );
    OIIO_CHECK_ASSERT( result );
    /// Remove one channel to make size != 3
    solver.observer.data["main"].pop_back();
    OIIO_CHECK_EQUAL( solver.observer.data["main"].size(), 2 );
    solver.calculate_WB();

    check_calculate_transform_matrix_error(
        solver, expected_error_observer_not_initialized );
}

const std::string expected_error_training_data_not_initialized =
    "Training data needs to be initialised prior to "
    "calling SpectralSolver::calculate_transform().";

/// Tests that calculate_transform returns false and sets error when training data is not initialized
void testIDT_calculate_transform_Training_Data_Not_Initialized()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::core::SpectralSolver solver( { DATA_PATH } );
    /// Initialize camera, illuminant, observer, but not training_data
    load_camera_helper( solver, "arri", "d21", "iso7589", false, true );
    /// Clear training_data to simulate uninitialized state
    solver.training_data.data.clear();
    solver.calculate_WB();

    check_calculate_transform_matrix_error(
        solver, expected_error_training_data_not_initialized );
}

/// Tests that calculate_transform returns false and sets error when training data is empty
void testIDT_calculate_transform_Training_Data_Empty()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::core::SpectralSolver solver( { DATA_PATH } );
    /// Initialize camera, illuminant, observer
    load_camera_helper( solver, "arri", "d21", "iso7589", false, true );
    /// Initialize training_data but make it empty
    bool result = solver.load_spectral_data(
        "training/training_spectral.json", solver.training_data );
    OIIO_CHECK_ASSERT( result );
    /// Clear the "main" set to make it empty (but keep the key)
    solver.training_data.data["main"].clear();
    OIIO_CHECK_EQUAL( solver.training_data.data.count( "main" ), 1 );
    OIIO_CHECK_EQUAL( solver.training_data.data["main"].empty(), true );
    solver.calculate_WB();

    check_calculate_transform_matrix_error(
        solver, expected_error_training_data_not_initialized );
}

void test_compare_solvers()
{
    // Test that all 4 types of solver [with/without eigen] x [with/without ceres]
    // calculate the same transform matrix.
    // Calculates and compares the four matrices over 48 different colour
    // temperatures.

    if ( !rta::core::math::has_eigen() && !rta::core::math::has_ceres() )
        return;

    std::cout << std::endl << __FUNCTION__ << std::endl;

    size_t eigen_steps = rta::core::math::has_eigen() ? 2 : 1;
    size_t ceres_steps = rta::core::math::has_ceres() ? 2 : 1;

    rta::core::SpectralData camera;
    load_file( "camera/Nikon_D200_380_780_5.json", camera );

    rta::core::SpectralData observer;
    load_file( "cmf/cmf_1931.json", observer );

    rta::core::SpectralData training_data;
    load_file( "training/training_spectral.json", training_data );

    std::vector<std::vector<double>> matrices[4];

    for ( int cct = 1500; cct <= 25000; cct += 500 )
    {
        std::string cct_string = std::to_string( cct );

        if ( cct < 4000 )
            cct_string = cct_string + "K";
        else
            cct_string = "D" + cct_string;

        for ( size_t use_eigen = 0; use_eigen < eigen_steps; use_eigen++ )
        {
            for ( size_t use_ceres = 0; use_ceres < ceres_steps; use_ceres++ )
            {
                rta::core::math::use_eigen = use_eigen;
                rta::core::math::use_ceres = use_ceres;

                rta::core::SpectralSolver solver;
                solver.camera        = camera;
                solver.observer      = observer;
                solver.training_data = training_data;
                OIIO_CHECK_ASSERT( solver.find_illuminant( cct_string ) );
                OIIO_CHECK_ASSERT( solver.calculate_WB() );
                OIIO_CHECK_ASSERT( solver.calculate_transform() );

                size_t index    = ( use_ceres << 1 ) + use_eigen;
                matrices[index] = solver.transform_matrix;

                if ( index != 0 )
                {
                    const auto &mat1 = matrices[0];
                    const auto &mat2 = matrices[index];

                    for ( size_t row = 0; row < 3; row++ )
                        for ( size_t col = 0; col < 3; col++ )
                            OIIO_CHECK_EQUAL_THRESH(
                                mat1[row][col], mat2[row][col], 1e-6 );
                }
            }
        }
    }
}

int main( int, char ** )
{
    testIDT_LoadCameraSpst();
    testIDT_LoadIlluminant();
    testIDT_LoadTrainingData();
    testIDT_LoadCMF();
    testIDT_scaleLSC();
    testIDT_CalCM();
    testIDT_CalWB();
    testIDT_ChooseIllumSrc();
    testIDT_ChooseIllumType();
    testIDT_CalTI();
    testIDT_CalXYZ();
    testIDT_CalRGB();
    testIDT_CurveFit();
    testIDT_CalIDT();
    testIDT_calculate_transform();
    testIDT_calculate_transform_Camera_Not_Initialized();
    testIDT_calculate_transform_Camera_Wrong_Size();
    testIDT_calculate_transform_Illuminant_Not_Initialized();
    testIDT_calculate_transform_Illuminant_Wrong_Size();
    testIDT_calculate_transform_Observer_Not_Initialized();
    testIDT_calculate_transform_Observer_Wrong_Size();
    testIDT_calculate_transform_Training_Data_Not_Initialized();
    testIDT_calculate_transform_Training_Data_Empty();

    test_compare_solvers();

    return unit_test_failures;
}
