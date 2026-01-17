// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include <filesystem>
#include <OpenImageIO/unittest.h>
#include <rawtoaces/rawtoaces_core.h>
#include "test_utils.h"

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
