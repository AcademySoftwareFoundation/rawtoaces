// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "colour_transforms.h"
#include "transform_cache.h"

#include <iostream>
#include <assert.h>

namespace rta
{
namespace util
{

bool configure_spectral_solver(
    core::SpectralSolver &solver,
    const std::string    &camera_make,
    const std::string    &camera_model,
    const std::string    &illuminant,
    bool                  load_observer,
    bool                  load_training_data,
    std::string          &error_message )
{
    bool success;

    success = solver.find_camera( camera_make, camera_model );
    if ( !success )
    {
        error_message = "Failed to find spectral data for camera make: '" +
                       camera_make + "', model: '" + camera_model +
                       "'. Please check the database search path in RAWTOACES_DATABASE_PATH";
        return false;
    }

    if ( load_training_data )
    {
        const std::string training_path = "training/training_spectral.json";
        success =
            solver.load_spectral_data( training_path, solver.training_data );
        if ( !success )
        {
            error_message = "Failed to find training data '" + training_path +
                           "'. Please check the database search path in RAWTOACES_DATABASE_PATH";
            return false;
        }
    }

    if ( load_observer )
    {
        const std::string observer_path = "cmf/cmf_1931.json";
        success = solver.load_spectral_data( observer_path, solver.observer );
        if ( !success )
        {
            error_message = "Failed to find observer '" + observer_path +
                           "'. Please check the database search path in RAWTOACES_DATABASE_PATH";
            return false;
        }
    }

    if ( !illuminant.empty() )
    {
        success = solver.find_illuminant( illuminant );

        if ( !success )
        {
            error_message = "Failed to find illuminant type '" + illuminant +
                           "'. Please check the database search path in RAWTOACES_DATABASE_PATH";
            return false;
        }
    }

    return true;
}

bool solve_illuminant_from_multipliers(
    const std::string          &camera_make,
    const std::string          &camera_model,
    const std::vector<double>  &wb_multipliers,
    core::SpectralSolver       &solver,
    cache::IlluminantAndWBData &cache_data,
    std::string                &error_message )
{
    if ( !configure_spectral_solver(
             solver, camera_make, camera_model, "", false, false, error_message ) )
    {
        return false;
    }

    if ( !solver.find_illuminant( wb_multipliers ) )
    {
        error_message = "Failed to find illuminant from white balance multipliers";
        return false;
    }

    const auto &multipliers = solver.get_WB_multipliers();
    cache_data.first        = solver.illuminant.type;
    cache_data.second[0]    = multipliers[0];
    cache_data.second[1]    = multipliers[1];
    cache_data.second[2]    = multipliers[2];

    return true;
}

bool fetch_illuminant_from_multipliers(
    const std::string         &camera_make,
    const std::string         &camera_model,
    const std::vector<double> &wb_multipliers,
    core::SpectralSolver      &solver,
    int                        verbosity,
    bool                       disable_cache,
    std::string               &out_illuminant,
    std::string               &error_message )
{
    assert( wb_multipliers.size() == 3 );

    cache::CameraAndWBDescriptor descriptor = {
        camera_make,
        camera_model,
        { wb_multipliers[0], wb_multipliers[1], wb_multipliers[2] }
    };

    auto &illuminant_from_WB_cache     = cache::get_illuminant_from_WB_cache();
    illuminant_from_WB_cache.verbosity = verbosity;
    illuminant_from_WB_cache.disabled  = disable_cache;

    std::string solve_error;
    const auto &entry = illuminant_from_WB_cache.fetch(
        descriptor, [&]( cache::IlluminantAndWBData &cache_data ) {
            return solve_illuminant_from_multipliers(
                camera_make, camera_model, wb_multipliers, solver, cache_data, solve_error );
        } );

    bool success = entry.first;
    if ( !success && !solve_error.empty() )
    {
        error_message = solve_error;
    }

    if ( success )
    {
        out_illuminant = entry.second.first;

        if ( verbosity > 0 )
        {
            std::cerr << "Found illuminant: '" << out_illuminant << "'."
                      << std::endl;
        }
    }
    return success;
}

bool solve_multipliers_from_illuminant(
    const std::string           &camera_make,
    const std::string           &camera_model,
    const std::string           &in_illuminant,
    core::SpectralSolver        &solver,
    cache::WBFromIlluminantData &cache_data,
    std::string                 &error_message )
{
    if ( !configure_spectral_solver(
             solver, camera_make, camera_model, in_illuminant, false, false, error_message ) )
    {
        return false;
    }

    if ( !solver.calculate_WB() )
    {
        error_message = "Failed to calculate white balance multipliers";
        return false;
    }

    const auto &multipliers = solver.get_WB_multipliers();
    cache_data[0]           = multipliers[0];
    cache_data[1]           = multipliers[1];
    cache_data[2]           = multipliers[2];

    return true;
}

bool fetch_multipliers_from_illuminant(
    const std::string    &camera_make,
    const std::string    &camera_model,
    const std::string    &in_illuminant,
    core::SpectralSolver &solver,
    int                   verbosity,
    bool                  disable_cache,
    std::vector<double>  &out_multipliers,
    std::string          &error_message )
{
    cache::CameraAndIlluminantDescriptor descriptor = { camera_make,
                                                        camera_model,
                                                        in_illuminant };

    auto &WB_from_illuminant_cache     = cache::get_WB_from_illuminant_cache();
    WB_from_illuminant_cache.verbosity = verbosity;
    WB_from_illuminant_cache.disabled  = disable_cache;

<<<<<<< HEAD
    const auto &entry = WB_from_illuminant_cache.fetch(
=======
    std::string solve_error;
<<<<<<< HEAD
    const auto &entry = cache::WB_from_illuminant_cache.fetch(
>>>>>>> a174d5c (improve test coverage: add error_message parameter, fix Python tests, add main.cpp error handling tests)
=======
    const auto &entry = WB_from_illuminant_cache.fetch(
>>>>>>> f1e9945 ([analysis] fix static analyser warnings (#239))
        descriptor, [&]( cache::WBFromIlluminantData &cache_data ) {
            return solve_multipliers_from_illuminant(
                camera_make, camera_model, in_illuminant, solver, cache_data, solve_error );
        } );

    bool success = entry.first;
    if ( !success && !solve_error.empty() )
    {
        error_message = solve_error;
    }

    if ( success )
    {
        out_multipliers.resize( 3 );
        out_multipliers[0] = entry.second[0];
        out_multipliers[1] = entry.second[1];
        out_multipliers[2] = entry.second[2];

        if ( verbosity > 0 )
        {
            std::cerr << "White balance coefficients:" << std::endl;
            for ( auto &wb_multiplier: out_multipliers )
            {
                std::cerr << wb_multiplier << " ";
            }
            std::cerr << std::endl;
        }
    }
    else
    {
        out_multipliers.resize( 0 );
    }

    return success;
}

bool solve_matrix_from_illuminant(
    const std::string    &camera_make,
    const std::string    &camera_model,
    const std::string    &in_illuminant,
    core::SpectralSolver &solver,
    cache::MatrixData    &cache_data )
{
    std::string error_msg;
    if ( !configure_spectral_solver(
             solver, camera_make, camera_model, in_illuminant, true, true, error_msg ) )
    {
        return false;
    }

    if ( !solver.calculate_WB() )
    {
        return false;
    }

    if ( !solver.calculate_IDT_matrix() )
    {
        return false;
    }

    const auto &matrix = solver.get_IDT_matrix();
    for ( size_t row = 0; row < 3; row++ )
        for ( size_t col = 0; col < 3; col++ )
            cache_data[row][col] = matrix[row][col];

    return true;
}

bool fetch_matrix_from_illuminant(
    const std::string                &camera_make,
    const std::string                &camera_model,
    const std::string                &in_illuminant,
    core::SpectralSolver             &solver,
    int                               verbosity,
    bool                              disable_cache,
    std::vector<std::vector<double>> &out_matrix,
    std::string                       &error_message )
{
    cache::CameraAndIlluminantDescriptor descriptor = { camera_make,
                                                        camera_model,
                                                        in_illuminant };

    auto &matrix_from_illuminant_cache =
        cache::get_matrix_from_illuminant_cache();
    matrix_from_illuminant_cache.verbosity = verbosity;
    matrix_from_illuminant_cache.disabled  = disable_cache;

    const auto &entry = matrix_from_illuminant_cache.fetch(
        descriptor, [&]( cache::MatrixData &cache_data ) {
            return solve_matrix_from_illuminant(
                camera_make, camera_model, in_illuminant, solver, cache_data );
        } );

    bool success = entry.first;
    if ( !success )
    {
        // Try to get error message from solve_matrix_from_illuminant if available
        if ( error_message.empty() )
        {
            error_message = "Failed to calculate IDT matrix from illuminant";
        }
        return false;
    }

    const auto &matrix = entry.second;
    out_matrix.resize( 3 );
    for ( size_t row = 0; row < 3; row++ )
    {
        out_matrix[row].resize( 3 );
        for ( size_t col = 0; col < 3; col++ )
            out_matrix[row][col] = matrix[row][col];
    }

    if ( verbosity > 0 )
    {
        std::cerr << "Input Device Transform (IDT) matrix:" << std::endl;
        for ( auto &row: out_matrix )
        {
            std::cerr << "  ";
            for ( auto &col: row )
            {
                std::cerr << col << " ";
            }
            std::cerr << std::endl;
        }
    }

    return true;
}

void solve_matrix_from_metadata(
    const core::Metadata &metadata, cache::MatrixData &cache_data )
{
    core::MetadataSolver solver( metadata );
    auto                 matrix = solver.calculate_IDT_matrix();

    for ( size_t row = 0; row < 3; row++ )
        for ( size_t col = 0; col < 3; col++ )
            cache_data[row][col] = matrix[row][col];
}

void fetch_matrix_from_metadata(
    const core::Metadata             &metadata,
    int                               verbosity,
    bool                              disable_cache,
    std::vector<std::vector<double>> &out_matrix )
{
    cache::MetadataDescriptor descriptor = metadata;

    auto &matrix_from_dng_metadata_cache =
        cache::get_matrix_from_dng_metadata_cache();
    matrix_from_dng_metadata_cache.verbosity = verbosity;
    matrix_from_dng_metadata_cache.disabled  = disable_cache;

    const auto &entry = matrix_from_dng_metadata_cache.fetch(
        descriptor, [&]( cache::MatrixData &cache_data ) {
            solve_matrix_from_metadata( metadata, cache_data );
            return true;
        } );

    const auto &matrix = entry.second;
    out_matrix.resize( 3 );
    for ( size_t row = 0; row < 3; row++ )
    {
        out_matrix[row].resize( 3 );
        for ( size_t col = 0; col < 3; col++ )
            out_matrix[row][col] = matrix[row][col];
    }

    if ( verbosity > 0 )
    {
        std::cerr << "Input Device Transform (IDT) matrix:" << std::endl;
        for ( auto &row: out_matrix )
        {
            std::cerr << "  ";
            for ( auto &col: row )
            {
                std::cerr << col << " ";
            }
            std::cerr << std::endl;
        }
    }
}

} // namespace util
} // namespace rta
