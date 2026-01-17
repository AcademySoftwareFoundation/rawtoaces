// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#endif

#include "test_utils.h"
#include <OpenImageIO/unittest.h>

#include "../src/rawtoaces_util/colour_transforms.h"
#include <rawtoaces/rawtoaces_core.h>

namespace
{
const std::vector<double> k_wb_multipliers = { 1.5, 1.0, 1.2 };
} // namespace

/// Verifies cache hit/miss logging for illuminant lookup by WB multipliers.
void test_fetch_illuminant_from_multipliers_cache_hit()
{
    std::cout << std::endl
              << "test_fetch_illuminant_from_multipliers_cache_hit()"
              << std::endl;

    TestFixture fixture;
    auto &test_dir = fixture.with_camera( "CacheBrand", "CacheModel" ).build();

    rta::core::SpectralSolver solver( { test_dir.get_database_path() } );

    std::string illuminant;

    std::string output = capture_stderr( [&]() {
        bool success = rta::util::fetch_illuminant_from_multipliers(
            "CacheBrand",
            "CacheModel",
            k_wb_multipliers,
            solver,
            1,
            false,
            illuminant );
        OIIO_CHECK_ASSERT( success );

        success = rta::util::fetch_illuminant_from_multipliers(
            "CacheBrand",
            "CacheModel",
            k_wb_multipliers,
            solver,
            1,
            false,
            illuminant );
        OIIO_CHECK_ASSERT( success );
    } );

    ASSERT_CONTAINS(
        output,
        "Cache (illuminant from WB): not found. Calculating a new entry." );
    ASSERT_CONTAINS( output, "Cache (illuminant from WB): found in cache!" );
}

/// Ensures invalid camera spectral data causes illuminant auto-detect failure.
void test_fetch_illuminant_from_multipliers_invalid_camera_data()
{
    std::cout << std::endl
              << "test_fetch_illuminant_from_multipliers_invalid_camera_data()"
              << std::endl;

    TestFixture fixture;
    auto       &test_dir = fixture
                         .with_camera(
                             "InvalidCamera",
                             "BadData",
                             nlohmann::json::array( { "R", "G", "B", "D" } ) )
                         .build();

    rta::core::SpectralSolver solver( { test_dir.get_database_path() } );

    std::string illuminant;

    bool        success;
    std::string output = capture_stderr( [&]() {
        success = rta::util::fetch_illuminant_from_multipliers(
            "InvalidCamera",
            "BadData",
            k_wb_multipliers,
            solver,
            1,
            false,
            illuminant );
    } );

    OIIO_CHECK_ASSERT( !success );
    ASSERT_CONTAINS( output, "SpectralSolver::find_illuminant()" );
}

/// Ensures cache-disable flag bypasses cache hits for illuminant lookup.
void test_fetch_illuminant_from_multipliers_cache_disabled()
{
    std::cout << std::endl
              << "test_fetch_illuminant_from_multipliers_cache_disabled()"
              << std::endl;

    TestFixture fixture;
    auto       &test_dir =
        fixture.with_camera( "NoCacheBrand", "NoCacheModel" ).build();

    rta::core::SpectralSolver solver( { test_dir.get_database_path() } );

    std::string illuminant;

    std::string output = capture_stderr( [&]() {
        bool success = rta::util::fetch_illuminant_from_multipliers(
            "NoCacheBrand",
            "NoCacheModel",
            k_wb_multipliers,
            solver,
            1,
            true,
            illuminant );
        OIIO_CHECK_ASSERT( success );

        success = rta::util::fetch_illuminant_from_multipliers(
            "NoCacheBrand",
            "NoCacheModel",
            k_wb_multipliers,
            solver,
            1,
            true,
            illuminant );
        OIIO_CHECK_ASSERT( success );
    } );

    ASSERT_CONTAINS( output, "Cache (illuminant from WB): disabled." );
    ASSERT_NOT_CONTAINS(
        output, "Cache (illuminant from WB): found in cache!" );
}

/// Verifies WB multipliers calculation and cache hit for illuminant lookup.
void test_fetch_multipliers_from_illuminant_cache_hit()
{
    std::cout << std::endl
              << "test_fetch_multipliers_from_illuminant_cache_hit()"
              << std::endl;

    TestFixture fixture;
    auto       &test_dir =
        fixture.with_camera( "CacheBrand2", "CacheModel2" ).build();

    rta::core::SpectralSolver solver( { test_dir.get_database_path() } );

    std::vector<double> wb_multipliers;
    std::string         output = capture_stderr( [&]() {
        bool success = rta::util::fetch_multipliers_from_illuminant(
            "CacheBrand2",
            "CacheModel2",
            "D65",
            solver,
            1,
            false,
            wb_multipliers );
        OIIO_CHECK_ASSERT( success );
        OIIO_CHECK_EQUAL( wb_multipliers.size(), 3 );

        success = rta::util::fetch_multipliers_from_illuminant(
            "CacheBrand2",
            "CacheModel2",
            "D65",
            solver,
            1,
            false,
            wb_multipliers );
        OIIO_CHECK_ASSERT( success );
    } );

    rta::core::SpectralSolver solver_expected(
        { test_dir.get_database_path() } );
    OIIO_CHECK_ASSERT(
        solver_expected.find_camera( "CacheBrand2", "CacheModel2" ) );
    OIIO_CHECK_ASSERT( solver_expected.find_illuminant( "D65" ) );
    OIIO_CHECK_ASSERT( solver_expected.calculate_WB() );

    const auto &expected = solver_expected.get_WB_multipliers();
    OIIO_CHECK_EQUAL( expected.size(), 3 );
    for ( size_t i = 0; i < 3; i++ )
    {
        OIIO_CHECK_EQUAL_THRESH( wb_multipliers[i], expected[i], 1e-7 );
    }

    ASSERT_CONTAINS(
        output,
        "Cache (WB from illuminant): not found. Calculating a new entry." );
    ASSERT_CONTAINS( output, "Cache (WB from illuminant): found in cache!" );
    ASSERT_CONTAINS( output, "White balance coefficients:" );
}

/// Ensures WB multipliers are cleared and error is logged on failure.
void test_fetch_multipliers_from_illuminant_failure_clears_output()
{
    std::cout
        << std::endl
        << "test_fetch_multipliers_from_illuminant_failure_clears_output()"
        << std::endl;

    TestFixture fixture;
    auto       &test_dir = fixture
                         .with_camera(
                             "BadWB",
                             "BadWBModel",
                             nlohmann::json::array( { "R", "G", "B", "D" } ) )
                         .with_illuminant( "4200" )
                         .build();

    rta::core::SpectralSolver solver( { test_dir.get_database_path() } );

    std::vector<double> wb_multipliers = { 9.0, 9.0, 9.0 };

    bool        success;
    std::string output = capture_stderr( [&]() {
        success = rta::util::fetch_multipliers_from_illuminant(
            "BadWB", "BadWBModel", "4200", solver, 1, false, wb_multipliers );
    } );

    OIIO_CHECK_ASSERT( !success );
    OIIO_CHECK_EQUAL( wb_multipliers.size(), 0 );
    ASSERT_CONTAINS(
        output, "ERROR: Failed to calculate the white balancing weights." );
}

/// Verifies IDT matrix calculation and cache hit for illuminant lookup.
void test_fetch_matrix_from_illuminant_cache_hit()
{
    std::cout << std::endl
              << "test_fetch_matrix_from_illuminant_cache_hit()" << std::endl;

    TestFixture fixture;
    auto       &test_dir =
        fixture.with_camera( "CacheBrand3", "CacheModel3" ).build();

    rta::core::SpectralSolver solver( { test_dir.get_database_path() } );

    std::vector<std::vector<double>> matrix;
    std::string                      output = capture_stderr( [&]() {
        bool success = rta::util::fetch_matrix_from_illuminant(
            "CacheBrand3", "CacheModel3", "D65", solver, 1, false, matrix );
        OIIO_CHECK_ASSERT( success );

        success = rta::util::fetch_matrix_from_illuminant(
            "CacheBrand3", "CacheModel3", "D65", solver, 1, false, matrix );
        OIIO_CHECK_ASSERT( success );
    } );

    rta::core::SpectralSolver solver_expected(
        { test_dir.get_database_path() } );
    OIIO_CHECK_ASSERT(
        solver_expected.find_camera( "CacheBrand3", "CacheModel3" ) );
    OIIO_CHECK_ASSERT( solver_expected.load_spectral_data(
        "training/training_spectral.json", solver_expected.training_data ) );
    OIIO_CHECK_ASSERT( solver_expected.load_spectral_data(
        "cmf/cmf_1931.json", solver_expected.observer ) );
    OIIO_CHECK_ASSERT( solver_expected.find_illuminant( "D65" ) );
    OIIO_CHECK_ASSERT( solver_expected.calculate_WB() );
    OIIO_CHECK_ASSERT( solver_expected.calculate_IDT_matrix() );

    const auto &expected = solver_expected.get_IDT_matrix();
    OIIO_CHECK_EQUAL( matrix.size(), 3 );
    OIIO_CHECK_EQUAL( matrix[0].size(), 3 );
    for ( size_t row = 0; row < 3; row++ )
    {
        for ( size_t col = 0; col < 3; col++ )
        {
            OIIO_CHECK_EQUAL_THRESH(
                matrix[row][col], expected[row][col], 1e-7 );
        }
    }

    ASSERT_CONTAINS(
        output,
        "Cache (matrix from illuminant): not found. Calculating a new entry." );
    ASSERT_CONTAINS(
        output, "Cache (matrix from illuminant): found in cache!" );
    ASSERT_CONTAINS( output, "Input Device Transform (IDT) matrix:" );
}

/// Verifies DNG-metadata IDT matrix values and cache hit behavior.
void test_fetch_matrix_from_metadata_cache_hit_and_values()
{
    std::cout << std::endl
              << "test_fetch_matrix_from_metadata_cache_hit_and_values()"
              << std::endl;

    rta::core::Metadata metadata;
    init_metadata( metadata );

    std::vector<std::vector<double>> matrix;
    std::string                      output = capture_stderr( [&]() {
        rta::util::fetch_matrix_from_metadata( metadata, 1, false, matrix );
        rta::util::fetch_matrix_from_metadata( metadata, 1, false, matrix );
    } );

    OIIO_CHECK_EQUAL( matrix.size(), 3 );
    OIIO_CHECK_EQUAL( matrix[0].size(), 3 );
    ASSERT_CONTAINS(
        output,
        "Cache (matrix from DNG metadata): not found. Calculating a new entry." );
    ASSERT_CONTAINS(
        output, "Cache (matrix from DNG metadata): found in cache!" );
    ASSERT_CONTAINS( output, "Input Device Transform (IDT) matrix:" );

    rta::core::MetadataSolver solver( metadata );
    auto                      expected = solver.calculate_IDT_matrix();
    for ( size_t row = 0; row < 3; row++ )
    {
        for ( size_t col = 0; col < 3; col++ )
        {
            OIIO_CHECK_EQUAL_THRESH(
                matrix[row][col], expected[row][col], 1e-7 );
        }
    }
}

/// Ensures missing camera data fails during illuminant-from-WB.
void test_fetch_illuminant_from_multipliers_missing_camera()
{
    std::cout << std::endl
              << "test_fetch_illuminant_from_multipliers_missing_camera()"
              << std::endl;

    TestFixture fixture;
    auto       &test_dir = fixture.build();

    rta::core::SpectralSolver solver( { test_dir.get_database_path() } );

    std::string illuminant;

    bool        success;
    std::string output = capture_stderr( [&]() {
        success = rta::util::fetch_illuminant_from_multipliers(
            "MissingMake",
            "MissingModel",
            k_wb_multipliers,
            solver,
            1,
            true,
            illuminant );
    } );

    OIIO_CHECK_ASSERT( !success );
    ASSERT_CONTAINS(
        output,
        "Failed to find spectral data for camera make: 'MissingMake', model: 'MissingModel'." );
}

/// Ensures IDT matrix fetch fails when WB calculation fails.
void test_fetch_matrix_from_illuminant_calculate_wb_failure()
{
    std::cout << std::endl
              << "test_fetch_matrix_from_illuminant_calculate_wb_failure()"
              << std::endl;

    TestFixture fixture;
    auto       &test_dir = fixture
                         .with_camera(
                             "BadWBMatrix",
                             "BadWBMatrixModel",
                             nlohmann::json::array( { "R", "G", "B", "D" } ) )
                         .build();

    rta::core::SpectralSolver solver( { test_dir.get_database_path() } );

    std::vector<std::vector<double>> matrix;
    bool                             success;
    std::string                      output = capture_stderr( [&]() {
        success = rta::util::fetch_matrix_from_illuminant(
            "BadWBMatrix", "BadWBMatrixModel", "D65", solver, 1, true, matrix );
    } );

    OIIO_CHECK_ASSERT( !success );
    ASSERT_CONTAINS(
        output,
        "ERROR: camera needs to be initialised prior to calling SpectralSolver::calculate_WB()" );
    ASSERT_CONTAINS(
        output, "Failed to calculate the input transform matrix." );
}

int main( int, char ** )
{
    test_fetch_illuminant_from_multipliers_cache_hit();
    test_fetch_illuminant_from_multipliers_invalid_camera_data();
    test_fetch_illuminant_from_multipliers_cache_disabled();
    test_fetch_multipliers_from_illuminant_cache_hit();
    test_fetch_multipliers_from_illuminant_failure_clears_output();
    test_fetch_matrix_from_illuminant_cache_hit();
    test_fetch_matrix_from_metadata_cache_hit_and_values();
    test_fetch_illuminant_from_multipliers_missing_camera();
    test_fetch_matrix_from_illuminant_calculate_wb_failure();

    return unit_test_failures;
}
