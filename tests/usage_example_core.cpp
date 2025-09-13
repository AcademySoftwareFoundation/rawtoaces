// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include <filesystem>
#include <OpenImageIO/unittest.h>
#include <rawtoaces/rawtoaces_core.h>

// This file contains some usage examples of the core library.
// It has only very little unit test functionality to keep the code clean.

// A path to the rawtoaces database,
// typically in /usr/local/share/rawtoaces/data.
// We just use the copy in the current repo.
#define DATA_PATH "../_deps/rawtoaces_data-src/data/"

/// A helper function to configure the spectral solver. Typically, only the camera
/// data file path, and its make and model would change per image.
/// Every other bit of data should be the same.
void configure_spectral_solver( rta::core::SpectralSolver &solver )
{
    const std::string camera_make  = "nikon";
    const std::string camera_model = "d200";

    // Spectral curves to use.
    const std::string observer_path = "cmf/cmf_1931.json";
    const std::string training_path = "training/training_spectral.json";

    solver.find_camera( camera_make, camera_model );
    solver.load_spectral_data( observer_path, solver.observer );
    solver.load_spectral_data( training_path, solver.training_data );
}

/// Test the spectral solver, using the white balance weights either from
/// the image file's metadata, or custom weights.
void test_SpectralSolver_multipliers()
{
    // Input parameters.
    const std::vector<double> white_balance = { 1.79488, 1, 1.39779 };

    // Step 0:  Configure the solver.
    const std::vector<std::string> database_path = { DATA_PATH };
    rta::core::SpectralSolver      solver( database_path );
    configure_spectral_solver( solver );

    // Step 1: Find the best suitable illuminant for the given white-balancing
    // weights.
    solver.find_illuminant( white_balance );

    // Step 2: Solve the transform matrix.
    solver.calculate_IDT_matrix();

    // Step 3: Get the solved matrix.
    const std::vector<std::vector<double>> &solved_IDT =
        solver.get_IDT_matrix();

    // Check the results.
    const std::vector<std::vector<double>> true_IDT = {
        { 0.713439, 0.221480, 0.065082 },
        { 0.064818, 1.076460, -0.141278 },
        { 0.039568, -0.140956, 1.101387 }
    };

    for ( size_t row = 0; row < 3; row++ )
        for ( size_t col = 0; col < 3; col++ )
            OIIO_CHECK_EQUAL_THRESH(
                solved_IDT[row][col], true_IDT[row][col], 1e-5 );
}

/// Test the spectral solver, white-balancing to a specific illuminant.
void test_SpectralSolver_illuminant()
{
    // Input parameters.
    const std::string illuminant = "d55";

    // Step 0: Configure the solver.
    const std::vector<std::string> database_path = { DATA_PATH };
    rta::core::SpectralSolver      solver( database_path );
    configure_spectral_solver( solver );

    // Step 1: Select the provided illuminant.
    solver.find_illuminant( illuminant );

    // Step 2: Calculate the white-balancing weights.
    solver.calculate_WB();

    // Step 3: Get the solved WB weights.
    const std::vector<double> &solved_WB = solver.get_WB_multipliers();

    // Step 4: Solve the transform matrix.
    solver.calculate_IDT_matrix();

    // Step 5: Get the solved matrix.
    const std::vector<std::vector<double>> &solved_IDT =
        solver.get_IDT_matrix();

    // Check the results.
    const std::vector<double> true_WB = { 1.79488, 1, 1.39779 };
    for ( size_t row = 0; row < 3; row++ )
        OIIO_CHECK_EQUAL_THRESH( solved_WB[row], true_WB[row], 1e-5 );

    const std::vector<std::vector<double>> true_IDT = {
        { 0.713428, 0.221535, 0.065037 },
        { 0.064829, 1.076544, -0.141372 },
        { 0.039572, -0.140962, 1.101390 }
    };
    for ( size_t row = 0; row < 3; row++ )
        for ( size_t col = 0; col < 3; col++ )
            OIIO_CHECK_EQUAL_THRESH(
                solved_IDT[row][col], true_IDT[row][col], 1e-5 );
}

/// A helper function to init the metadata object.
/// Normally the values come from a DNG file metadata.
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

/// Test the metadata solver.
void test_MetadataSolver()
{
    // Step 0: Init the metadata.
    rta::core::Metadata metadata;
    init_metadata( metadata );

    // Step 1: Init the solver.
    rta::core::MetadataSolver solver( metadata );

    // Step 2: Solve the transform matrix.
    const std::vector<std::vector<double>> solved_IDT =
        solver.calculate_IDT_matrix();

    // Check the results.
    const std::vector<std::vector<double>> true_IDT = {
        { 1.053647, 0.003904, 0.004908 },
        { -0.489956, 1.361479, 0.102084 },
        { -0.002450, 0.006050, 1.013916 }
    };
    for ( size_t row = 0; row < 3; row++ )
        for ( size_t col = 0; col < 3; col++ )
            OIIO_CHECK_EQUAL_THRESH(
                solved_IDT[row][col], true_IDT[row][col], 1e-5 );
}

int main( int, char ** )
{
    test_SpectralSolver_multipliers();
    test_SpectralSolver_illuminant();
    test_MetadataSolver();

    return unit_test_failures;
}
