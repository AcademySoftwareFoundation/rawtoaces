// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#endif

#include <filesystem>
#include <OpenImageIO/unittest.h>

#include "../src/rawtoaces_core/mathOps.h"
#include <rawtoaces/rawtoaces_core.h>
#include "../src/rawtoaces_core/rawtoaces_core_priv.h"
#include "test_utils.h"

#define DATA_PATH "../_deps/rawtoaces_data-src/data/"

// --- Helper: fingerprint via std::hash<double> (deterministic same platform) ---
#include <functional>
#include <sstream>
static std::string compute_fingerprint(const std::vector<std::vector<double>>& data) {
    std::hash<double> hasher;
    std::ostringstream oss;
    oss << std::hex;
    for (const auto& row : data)
        for (double v : row)
            oss << hasher(v);
    return oss.str();
}

void testIDT_LoadCameraSpst()
{
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

    const std::string channels[3] = { "R", "G", "B" };
    for ( size_t i = 0; i < 3; i++ )
    {
        const rta::core::Spectrum &spectrum = camera[channels[i]];
        OIIO_CHECK_EQUAL( spectrum.shape.first, 380 );
        OIIO_CHECK_EQUAL( spectrum.shape.last, 780 );
        OIIO_CHECK_EQUAL( spectrum.shape.step, 5 );
        OIIO_CHECK_EQUAL( spectrum.values.size(), 81 );

        // Spot-check: first 3 and last 3 rows per channel
        if (i == 0) { // R channel
            OIIO_CHECK_EQUAL_THRESH( spectrum.values[0],  0.000188205, 1e-5 );
            OIIO_CHECK_EQUAL_THRESH( spectrum.values[1],  0.000440222, 1e-5 );
            OIIO_CHECK_EQUAL_THRESH( spectrum.values[2],  0.001561591, 1e-5 );
            OIIO_CHECK_EQUAL_THRESH( spectrum.values[78], 0.000384839, 1e-5 );
            OIIO_CHECK_EQUAL_THRESH( spectrum.values[79], 0.000286597, 1e-5 );
            OIIO_CHECK_EQUAL_THRESH( spectrum.values[80], 0.000269169, 1e-5 );
        } else if (i == 1) { // G channel
            OIIO_CHECK_EQUAL_THRESH( spectrum.values[0],  8.59E-05, 1e-5 );
            OIIO_CHECK_EQUAL_THRESH( spectrum.values[1],  0.000166118, 1e-5 );
            OIIO_CHECK_EQUAL_THRESH( spectrum.values[2],  0.00046321, 1e-5 );
            OIIO_CHECK_EQUAL_THRESH( spectrum.values[78], 0.000125687, 1e-5 );
            OIIO_CHECK_EQUAL_THRESH( spectrum.values[79], 0.000104774, 1e-5 );
            OIIO_CHECK_EQUAL_THRESH( spectrum.values[80], 0.000138887, 1e-5 );
        } else { // B channel
            OIIO_CHECK_EQUAL_THRESH( spectrum.values[0],  9.58E-05, 1e-5 );
            OIIO_CHECK_EQUAL_THRESH( spectrum.values[1],  0.000258734, 1e-5 );
            OIIO_CHECK_EQUAL_THRESH( spectrum.values[2],  0.001181466, 1e-5 );
            OIIO_CHECK_EQUAL_THRESH( spectrum.values[78], 8.94E-05, 1e-5 );
            OIIO_CHECK_EQUAL_THRESH( spectrum.values[79], 6.92E-05, 1e-5 );
            OIIO_CHECK_EQUAL_THRESH( spectrum.values[80], 0.000126057, 1e-5 );
        }
    }
}

void testIDT_LoadIlluminant()
{
    std::filesystem::path absolutePath = std::filesystem::absolute(
        DATA_PATH "illuminant/iso7589_stutung_380_780_5.json" );

    bool                    result;
    rta::core::SpectralData illuminant;
    result = illuminant.load( absolutePath.string() );
    OIIO_CHECK_ASSERT( result );

    OIIO_CHECK_EQUAL( illuminant.type, "ISO7589" );
    OIIO_CHECK_EQUAL( illuminant["power"].shape.step, 5 );

    std::vector<double> &illumTestData = illuminant["power"].values;
    OIIO_CHECK_EQUAL( illumTestData.size(), 81 );
    // Spot-check: first 3 and last 3
    OIIO_CHECK_EQUAL_THRESH( illumTestData[0],  0.04, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumTestData[1],  0.05, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumTestData[2],  0.06, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumTestData[78], 0.975, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumTestData[79], 0.980, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumTestData[80], 0.985, 1e-5 );
}

void testIDT_LoadTrainingData()
{
    std::filesystem::path absolutePath = std::filesystem::absolute(
        DATA_PATH "training/training_spectral.json" );

    bool                    result;
    rta::core::SpectralData training_data;
    result = training_data.load( absolutePath.string() );
    OIIO_CHECK_ASSERT( result );

    // Spot-check structure
    OIIO_CHECK_EQUAL( training_data.data.count("patch1"), 1 );
    OIIO_CHECK_EQUAL( training_data.data.count("patch2"), 1 );
    OIIO_CHECK_EQUAL( training_data.data.count("patch3"), 1 );
    OIIO_CHECK_EQUAL( training_data["patch1"].values.size(), 81 );
    OIIO_CHECK_EQUAL( training_data["patch2"].values.size(), 81 );
    OIIO_CHECK_EQUAL( training_data["patch3"].values.size(), 81 );

    // Spot-check: first 3 and last 3 for each patch
    OIIO_CHECK_EQUAL_THRESH( training_data["patch1"].values[0], 0.0501123560, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( training_data["patch1"].values[1], 0.0350998380, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( training_data["patch1"].values[2], 0.0262312100, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( training_data["patch1"].values[78], 0.00080075, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( training_data["patch1"].values[79], 0.000743396, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( training_data["patch1"].values[80], 0.000690079, 1e-5 );

    OIIO_CHECK_EQUAL_THRESH( training_data["patch2"].values[0], 0.0706580560, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( training_data["patch2"].values[1], 0.0801336960, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( training_data["patch2"].values[2], 0.0850925650, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( training_data["patch2"].values[78], 0.000289166, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( training_data["patch2"].values[79], 0.000268454, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( training_data["patch2"].values[80], 0.0002492, 1e-5 );

    OIIO_CHECK_EQUAL_THRESH( training_data["patch3"].values[0], 0.1696284150, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( training_data["patch3"].values[1], 0.3047719550, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( training_data["patch3"].values[2], 0.5160895950, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( training_data["patch3"].values[78], 0.000231302, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( training_data["patch3"].values[79], 0.000214686, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( training_data["patch3"].values[80], 0.000199288, 1e-5 );
}

void testIDT_LoadCMF()
{
    std::filesystem::path absolutePath =
        std::filesystem::absolute( DATA_PATH "cmf/cmf_1931.json" );

    bool                    result;
    rta::core::SpectralData spectral_data;
    result = spectral_data.load( absolutePath.string() );
    OIIO_CHECK_ASSERT( result );

    OIIO_CHECK_EQUAL( spectral_data.data.count("X"), 1 );
    OIIO_CHECK_EQUAL( spectral_data.data.count("Y"), 1 );
    OIIO_CHECK_EQUAL( spectral_data.data.count("Z"), 1 );
    OIIO_CHECK_EQUAL( spectral_data["X"].values.size(), 401 );
    OIIO_CHECK_EQUAL( spectral_data["Y"].values.size(), 401 );
    OIIO_CHECK_EQUAL( spectral_data["Z"].values.size(), 401 );

    // Spot-check: first 3 and last 3 for X,Y,Z (step=5, indices 0,1,2 and 398,399,400)
    // X channel
    OIIO_CHECK_EQUAL_THRESH( spectral_data["X"].values[0],   0.001368, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( spectral_data["X"].values[1],   0.002236, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( spectral_data["X"].values[2],   0.004243, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( spectral_data["X"].values[398], 0.000117413, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( spectral_data["X"].values[399], 0.000109552, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( spectral_data["X"].values[400], 0.000102225, 1e-5 );
    // Y channel
    OIIO_CHECK_EQUAL_THRESH( spectral_data["Y"].values[0],   3.90E-05, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( spectral_data["Y"].values[1],   6.40E-05, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( spectral_data["Y"].values[2],   0.00012, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( spectral_data["Y"].values[398], 4.24E-05, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( spectral_data["Y"].values[399], 3.96E-05, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( spectral_data["Y"].values[400], 3.69E-05, 1e-5 );
    // Z channel
    OIIO_CHECK_EQUAL_THRESH( spectral_data["Z"].values[0],   0.006450001, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( spectral_data["Z"].values[1],   0.01054999, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( spectral_data["Z"].values[2],   0.02005001, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( spectral_data["Z"].values[398], 0.0, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( spectral_data["Z"].values[399], 0.0, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( spectral_data["Z"].values[400], 0.0, 1e-5 );
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
    rta::core::SpectralData illuminant;
    load_file( "illuminant/iso7589_stutung_380_780_5.json", illuminant );

    rta::core::SpectralData camera;
    load_file( "camera/Nikon_D200_380_780_5.json", camera );

    scale_illuminant( camera, illuminant );

    const std::vector<double> illumDataScaled = illuminant["power"].values;

    OIIO_CHECK_EQUAL( illumDataScaled.size(), 81 );
    OIIO_CHECK_EQUAL( illuminant.type, "ISO7589" );
    OIIO_CHECK_EQUAL( illuminant["power"].shape.step, 5 );
    // Spot-check: first 3 and last 3
    OIIO_CHECK_EQUAL_THRESH( illumDataScaled[0],  0.00546219526, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumDataScaled[1],  0.00682774407, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumDataScaled[2],  0.00819329289, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumDataScaled[78], 0.13450655825, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumDataScaled[79], 0.13553071986, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumDataScaled[80], 0.13655488148, 1e-5 );
}

void testIDT_CalCM()
{
    rta::core::SpectralData illuminant;
    load_file( "illuminant/iso7589_stutung_380_780_5.json", illuminant );

    rta::core::SpectralData camera;
    load_file( "camera/ARRI_D21_380_780_5.json", camera );

    std::vector<double> CM_test = calculate_CM( camera, illuminant );

    OIIO_CHECK_EQUAL_THRESH( CM_test[0], 1.0000000000f, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( CM_test[1], 1.4418439699f, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( CM_test[2], 1.8703081160f, 1e-5 );
}

void testIDT_CalWB()
{
    rta::core::SpectralData illuminant;
    load_file( "illuminant/iso7589_stutung_380_780_5.json", illuminant );

    rta::core::SpectralData camera;
    load_file( "camera/Nikon_D200_380_780_5.json", camera );

    std::vector<double> WB_test = _calculate_WB( camera, illuminant );

    OIIO_CHECK_EQUAL_THRESH( WB_test[0], 1.1397265, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( WB_test[1], 1.0000000, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( WB_test[2], 2.3240151, 1e-5 );
}

void testIDT_ChooseIllumSrc()
{
    rta::core::SpectralSolver solver( { DATA_PATH } );
    load_camera_helper( solver, "nikon", "d200", "", true, false );

    float               wb[3] = { 1.0, 1.0, 1.0 };
    std::vector<double> wbv( wb, wb + 3 );
    solver.find_illuminant( wbv );

    const auto         &best_illuminant = solver.illuminant;
    std::string         illumType_Test  = best_illuminant.type;
    std::vector<double> illumData_Test  = best_illuminant["power"].values;

    OIIO_CHECK_EQUAL( illumType_Test, "d45" );
    OIIO_CHECK_EQUAL( illumData_Test.size(), 81 );
    // Spot-check: first 3 and last 3
    OIIO_CHECK_EQUAL_THRESH( illumData_Test[0],  0.0106671894, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumData_Test[1],  0.0120341268, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumData_Test[2],  0.0134010642, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumData_Test[78], 0.0539319463, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumData_Test[79], 0.0523097995, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumData_Test[80], 0.0505829525, 1e-5 );
}

void testIDT_ChooseIllumType()
{
    rta::core::SpectralSolver solver( { DATA_PATH } );
    load_camera_helper( solver, "nikon", "d200", "iso7589", true, false );

    float               wb[3] = { 1.0, 1.0, 1.0 };
    std::vector<double> wbv( wb, wb + 3 );
    solver.calculate_WB();

    const auto         &best_illuminant = solver.illuminant;
    std::string         illumType_Test  = best_illuminant.type;
    std::vector<double> illumData_Test  = best_illuminant["power"].values;

    OIIO_CHECK_EQUAL( illumType_Test, "ISO7589" );
    OIIO_CHECK_EQUAL( illumData_Test.size(), 81 );
    // Spot-check: first 3 and last 3
    OIIO_CHECK_EQUAL_THRESH( illumData_Test[0],  0.0054621953, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumData_Test[1],  0.0068277441, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumData_Test[2],  0.0081932929, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumData_Test[78], 0.1345065583, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumData_Test[79], 0.1355307199, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( illumData_Test[80], 0.1365548815, 1e-5 );
}

void testIDT_CalTI()
{
    rta::core::SpectralData camera;
    load_file( "camera/Nikon_D200_380_780_5.json", camera );

    rta::core::SpectralData illuminant;
    load_file( "illuminant/iso7589_stutung_380_780_5.json", illuminant );

    rta::core::SpectralData training_data;
    load_file( "training/training_spectral.json", training_data );

    scale_illuminant( camera, illuminant );
    auto TI_test = calculate_TI( illuminant, training_data );

    OIIO_CHECK_EQUAL( TI_test.size(), 190 );
    for (size_t i = 0; i < 190; i++)
        OIIO_CHECK_EQUAL( TI_test[i].values.size(), 81 );

    // Fingerprint check on the entire TI output
    std::vector<std::vector<double>> ti_data;
    for (const auto& s : TI_test)
        ti_data.push_back(s.values);
    std::string fp = compute_fingerprint(ti_data);
    OIIO_CHECK_EQUAL( fp, "PLACEHOLDER_TI_FINGERPRINT" );
}

void testIDT_CalXYZ()
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

    auto TI       = calculate_TI( illuminant, training_data );
    auto XYZ_test = calculate_XYZ( observer, illuminant, TI );

    OIIO_CHECK_EQUAL( XYZ_test.size(), 190 );
    for (size_t i = 0; i < 190; i++)
        OIIO_CHECK_EQUAL( XYZ_test[i].size(), 3 );

    std::string fp = compute_fingerprint(XYZ_test);
    OIIO_CHECK_EQUAL( fp, "PLACEHOLDER_XYZ_FINGERPRINT" );
}

void testIDT_CalRGB()
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
    auto WB       = _calculate_WB( camera, illuminant );
    auto TI       = calculate_TI( illuminant, training_data );
    auto RGB_test = calculate_RGB( camera, WB, TI );

    OIIO_CHECK_EQUAL( RGB_test.size(), 190 );
    for (size_t i = 0; i < 190; i++)
        OIIO_CHECK_EQUAL( RGB_test[i].size(), 3 );

    std::string fp = compute_fingerprint(RGB_test);
    OIIO_CHECK_EQUAL( fp, "PLACEHOLDER_RGB_FINGERPRINT" );
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

    double BStart[6] = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };

    std::vector<std::vector<double>> IDT_test( 3, std::vector<double>( 3 ) );

    OIIO_CHECK_ASSERT( rta::core::curveFit( RGB, XYZ, BStart, 0, IDT_test ) );

    OIIO_CHECK_EQUAL_THRESH( IDT_test[0][0], 0.7447691479f, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( IDT_test[0][1], 0.1434200377f, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( IDT_test[0][2], 0.1118108144f, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( IDT_test[1][0], 0.0451759890f, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( IDT_test[1][1], 1.0082622042f, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( IDT_test[1][2], -0.0534381932f, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( IDT_test[2][0], 0.0247144012f, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( IDT_test[2][1], -0.1245524896f, 1e-5 );
    OIIO_CHECK_EQUAL_THRESH( IDT_test[2][2], 1.0998380884f, 1e-5 );
}

void testIDT_CalIDT()
{
    rta::core::SpectralSolver solver( { DATA_PATH } );
    load_camera_helper( solver, "arri", "d21", "iso7589", true, true );
    solver.calculate_WB();

    OIIO_CHECK_ASSERT( solver.calculate_IDT_matrix() );
    std::vector<std::vector<double>> IDT_test = solver.get_IDT_matrix();

    OIIO_CHECK_EQUAL( IDT_test.size(), 3 );
    OIIO_CHECK_EQUAL_THRESH( IDT_test[0][0], 1.0915120600f, 1e-4 );
    OIIO_CHECK_EQUAL_THRESH( IDT_test[0][1], -0.2516916464f, 1e-4 );
    OIIO_CHECK_EQUAL_THRESH( IDT_test[0][2], 0.1601795864f, 1e-4 );
    OIIO_CHECK_EQUAL_THRESH( IDT_test[1][0], -0.0089998772f, 1e-4 );
    OIIO_CHECK_EQUAL_THRESH( IDT_test[1][1], 1.2147199060f, 1e-4 );
    OIIO_CHECK_EQUAL_THRESH( IDT_test[1][2], -0.2057200288f, 1e-4 );
    OIIO_CHECK_EQUAL_THRESH( IDT_test[2][0], -0.1312667887f, 1e-4 );
    OIIO_CHECK_EQUAL_THRESH( IDT_test[2][1], -0.7361633199f, 1e-4 );
    OIIO_CHECK_EQUAL_THRESH( IDT_test[2][2], 1.8674301085f, 1e-4 );
}

static void check_calculate_IDT_matrix_error(
    rta::core::SpectralSolver &solver, const std::string &expected_error )
{
    bool        success = solver.calculate_IDT_matrix();
    std::string output  = solver.last_error_message;

    OIIO_CHECK_ASSERT( !success );
    ASSERT_CONTAINS( output, expected_error );
}

const std::string expected_error_camera_not_initialized =
    "Camera needs to be initialised prior to calling "
    "SpectralSolver::calculate_IDT_matrix().";

void testIDT_CalIDT_Camera_Not_Initialized()
{
    std::cout << std::endl
              << "testIDT_CalIDT_Camera_Not_Initialized()" << std::endl;

    rta::core::SpectralSolver solver( { DATA_PATH } );
    bool result = solver.find_illuminant( "iso7589" );
    OIIO_CHECK_ASSERT( result );
    result = solver.load_spectral_data(
        "training/training_spectral.json", solver.training_data );
    OIIO_CHECK_ASSERT( result );
    result = solver.load_spectral_data( "cmf/cmf_1931.json", solver.observer );
    OIIO_CHECK_ASSERT( result );

    check_calculate_IDT_matrix_error(
        solver, expected_error_camera_not_initialized );
}

void testIDT_CalIDT_Camera_Wrong_Size()
{
    std::cout << std::endl << "testIDT_CalIDT_Camera_Wrong_Size()" << std::endl;

    rta::core::SpectralSolver solver( { DATA_PATH } );
    bool result = solver.find_camera( "arri", "d21" );
    OIIO_CHECK_ASSERT( result );
    solver.camera.data["main"].pop_back();
    OIIO_CHECK_EQUAL( solver.camera.data["main"].size(), 2 );
    result = solver.find_illuminant( "iso7589" );
    OIIO_CHECK_ASSERT( result );
    result = solver.load_spectral_data(
        "training/training_spectral.json", solver.training_data );
    OIIO_CHECK_ASSERT( result );
    result = solver.load_spectral_data( "cmf/cmf_1931.json", solver.observer );
    OIIO_CHECK_ASSERT( result );
    solver.calculate_WB();

    check_calculate_IDT_matrix_error(
        solver, expected_error_camera_not_initialized );
}

const std::string expected_error_illuminant_not_initialized =
    "Illuminant needs to be initialised prior to "
    "calling SpectralSolver::calculate_IDT_matrix().";

void testIDT_CalIDT_Illuminant_Not_Initialized()
{
    std::cout << std::endl
              << "testIDT_CalIDT_Illuminant_Not_Initialized()" << std::endl;

    rta::core::SpectralSolver solver( { DATA_PATH } );
    bool result = solver.find_camera( "arri", "d21" );
    OIIO_CHECK_ASSERT( result );
    result = solver.load_spectral_data(
        "training/training_spectral.json", solver.training_data );
    OIIO_CHECK_ASSERT( result );
    result = solver.load_spectral_data( "cmf/cmf_1931.json", solver.observer );
    OIIO_CHECK_ASSERT( result );
    solver.calculate_WB();

    check_calculate_IDT_matrix_error(
        solver, expected_error_illuminant_not_initialized );
}

void testIDT_CalIDT_Illuminant_Wrong_Size()
{
    std::cout << std::endl
              << "testIDT_CalIDT_Illuminant_Wrong_Size()" << std::endl;

    rta::core::SpectralSolver solver( { DATA_PATH } );
    bool result = solver.find_camera( "arri", "d21" );
    OIIO_CHECK_ASSERT( result );
    result = solver.find_illuminant( "iso7589" );
    OIIO_CHECK_ASSERT( result );
    solver.illuminant.data["main"].emplace_back(
        std::make_pair( "extra", rta::core::Spectrum() ) );
    OIIO_CHECK_EQUAL( solver.illuminant.data["main"].size(), 2 );
    result = solver.load_spectral_data(
        "training/training_spectral.json", solver.training_data );
    OIIO_CHECK_ASSERT( result );
    result = solver.load_spectral_data( "cmf/cmf_1931.json", solver.observer );
    OIIO_CHECK_ASSERT( result );
    solver.calculate_WB();

    check_calculate_IDT_matrix_error(
        solver, expected_error_illuminant_not_initialized );
}

const std::string expected_error_observer_not_initialized =
    "Observer needs to be initialised prior to calling "
    "SpectralSolver::calculate_IDT_matrix().";

void testIDT_CalIDT_Observer_Not_Initialized()
{
    std::cout << std::endl
              << "testIDT_CalIDT_Observer_Not_Initialized()" << std::endl;

    rta::core::SpectralSolver solver( { DATA_PATH } );
    load_camera_helper( solver, "arri", "d21", "iso7589", true, false );
    solver.observer.data.clear();
    solver.calculate_WB();

    check_calculate_IDT_matrix_error(
        solver, expected_error_observer_not_initialized );
}

void testIDT_CalIDT_Observer_Wrong_Size()
{
    std::cout << std::endl
              << "testIDT_CalIDT_Observer_Wrong_Size()" << std::endl;

    rta::core::SpectralSolver solver( { DATA_PATH } );
    load_camera_helper( solver, "arri", "d21", "iso7589", true, false );
    bool result =
        solver.load_spectral_data( "cmf/cmf_1931.json", solver.observer );
    OIIO_CHECK_ASSERT( result );
    solver.observer.data["main"].pop_back();
    OIIO_CHECK_EQUAL( solver.observer.data["main"].size(), 2 );
    solver.calculate_WB();

    check_calculate_IDT_matrix_error(
        solver, expected_error_observer_not_initialized );
}

const std::string expected_error_training_data_not_initialized =
    "Training data needs to be initialised prior to "
    "calling SpectralSolver::calculate_IDT_matrix().";

void testIDT_CalIDT_Training_Data_Not_Initialized()
{
    std::cout << std::endl
              << "testIDT_CalIDT_Training_Data_Not_Initialized()" << std::endl;

    rta::core::SpectralSolver solver( { DATA_PATH } );
    load_camera_helper( solver, "arri", "d21", "iso7589", false, true );
    solver.training_data.data.clear();
    solver.calculate_WB();

    check_calculate_IDT_matrix_error(
        solver, expected_error_training_data_not_initialized );
}

void testIDT_CalIDT_Training_Data_Empty()
{
    std::cout << std::endl
              << "testIDT_CalIDT_Training_Data_Empty()" << std::endl;

    rta::core::SpectralSolver solver( { DATA_PATH } );
    load_camera_helper( solver, "arri", "d21", "iso7589", false, true );
    bool result = solver.load_spectral_data(
        "training/training_spectral.json", solver.training_data );
    OIIO_CHECK_ASSERT( result );
    solver.training_data.data["main"].clear();
    OIIO_CHECK_EQUAL( solver.training_data.data.count( "main" ), 1 );
    OIIO_CHECK_EQUAL( solver.training_data.data["main"].empty(), true );
    solver.calculate_WB();

    check_calculate_IDT_matrix_error(
        solver, expected_error_training_data_not_initialized );
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
    testIDT_CalIDT_Camera_Not_Initialized();
    testIDT_CalIDT_Camera_Wrong_Size();
    testIDT_CalIDT_Illuminant_Not_Initialized();
    testIDT_CalIDT_Illuminant_Wrong_Size();
    testIDT_CalIDT_Observer_Not_Initialized();
    testIDT_CalIDT_Observer_Wrong_Size();
    testIDT_CalIDT_Training_Data_Not_Initialized();
    testIDT_CalIDT_Training_Data_Empty();

    return unit_test_failures;
}