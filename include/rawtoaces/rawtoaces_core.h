// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <rawtoaces/spectral_data.h>

namespace rta
{
namespace core
{

// clang-format off

static const std::vector<std::vector<double>> XYZ_to_ACES = {
    {  1.0498110175, 0.0000000000, -0.0000974845 },
    { -0.4959030231, 1.3733130458,  0.0982400361 },
    {  0.0000000000, 0.0000000000,  0.9912520182 }
};

/// Colour adaptation from D65 to the ACES white point
static const std::vector<std::vector<double> > CAT_D65_to_ACES = {
    {  1.0097583639200136,      0.0050178093846550455, -0.015058389092388141  },
    {  0.0036602813378778347,   1.0030138169214682,    -0.0059802329456399824 },
    { -0.00029980928869024906, -0.0010516909063249997,  0.92820279627476576   }
};

// clang-format on

/// Calculate spectral power distribution of a daylight illuminant of given CCT
/// @param cct correlated colour temperature of the requested illuminant.
/// @param spectrum a reference to a `Spectrum` object to full with the
/// calculated values.
void calculate_daylight_SPD( const int &cct, Spectrum &spectrum );

/// Calculate spectral power distribution of a blackbody illuminant of given CCT
/// @param cct correlated colour temperature of the requested illuminant.
/// @param spectrum a reference to a `Spectrum` object to full with the
/// calculated values.
void calculate_blackbody_SPD( const int &cct, Spectrum &spectrum );

/// Solve an input transform using spectral sensitivity curves of a camera.
class SpectralSolver
{
public:
    /// The camera spectral data. Can be either assigned directly, loaded
    /// in-place place via `solver.camera.load()`, or found via
    /// `solver.find_camera()`.
    SpectralData camera;

    /// The illuminant spectral data. Can be either assigned directly, loaded
    /// in-place place via `solver.illuminant.load()`, or found via
    /// `solver.find_illuminant()`.
    SpectralData illuminant;

    /// The observer spectral data. Can be either assigned directly, or loaded
    /// in-place place via `solver.observer.load()`.
    SpectralData observer;

    /// The training set spectral data. Can be either assigned directly, or loaded
    /// in-place place via `solver.training_data.load()`.
    SpectralData training_data;

    /// The constructor. Takes the database search path as an optional
    /// parameter.
    /// @param search_directories optional database search path.
    SpectralSolver( const std::vector<std::string> &search_directories = {} );

    /// A helper method collecting of spectral data files of a given type from
    /// the database.
    /// @param type data type of the files to search for.
    /// @result a collection of files found in the database.
    std::vector<std::string>
    collect_data_files( const std::string &type ) const;

    /// A helper method loading the spectral data for a file at the given path.
    /// @param file_path the path to the file to load. If the path is relative,
    /// all locations in the search path will be searched in.
    /// @param out_data the `SpectralData` object to be filled with the loaded
    /// data.
    /// @result `true` is loaded successfully.
    bool
    load_spectral_data( const std::string &file_path, SpectralData &out_data );

    /// Load spectral sensitivity data for a camera.
    /// @param make the camera make to search for.
    /// @param model the camera model to search for.
    /// @result `true` if loaded successfully.
    bool find_camera( const std::string &make, const std::string &model );

    /// Find spectral power distribution data of an illuminant of the given
    /// type.
    /// @param type illuminant type. Can be one of the built-in types,
    /// e.g. `d55`, `3200k`, or a custom illuminant stored in the database.
    /// @result `true` if loaded successfully.
    bool find_illuminant( const std::string &type );

    /// Find the illuminant best matching the given white-balancing multipliers.
    /// @param wb_multipliers white-balancing multipliers to match.
    /// @result `true` if loaded successfully.
    bool find_illuminant( const std::vector<double> &wb_multipliers );

    /// Calculate the white-balance multipliers for the given configuration.
    /// The `camera`, and `illuminant` data have to be configured prior to this
    /// call. See `get_WB_multipliers()` to access the result.
    /// @result `true` if calculated successfully.
    bool calculate_WB();

    /// Calculate an input transform matrix. The `camera`,  `illuminant`,
    /// `observer` and `training_data` have to be configured prior to this
    /// call. See `get_IDT_matrix()` to access the result.
    /// @result `true` if calculated successfully.
    bool calculate_IDT_matrix();

    /// Get the matrix calculated using `calculate_IDT_matrix()`.
    /// @result a reference to the matrix.
    const std::vector<std::vector<double>> &get_IDT_matrix() const;

    /// Get the white-balance multipliers calculated using
    /// `find_best_illuminant()` or `select_illuminant()`.
    /// @result a reference to the multipliers.
    const std::vector<double> &get_WB_multipliers() const;

    int verbosity = 0;

private:
    std::vector<std::string>  _search_directories;
    std::vector<SpectralData> _all_illuminants;

    std::vector<double>              _WB_multipliers;
    std::vector<std::vector<double>> _IDT_matrix;
};

/// DNG metadata required to calculate an input transform.
struct Metadata
{
    /// A calibration data set. Currently two sets are supported.
    struct Calibration
    {
        unsigned short      illuminant = 0;
        std::vector<double> camera_calibration_matrix;
        std::vector<double> XYZ_to_RGB_matrix;
    } calibration[2];

    std::vector<double> neutral_RGB;
    double              baseline_exposure = 0.0;
};

/// Solve an input transform using the metadata stored in DNG files.
class MetadataSolver
{
public:
    /// Initialise the solver using DNG metadata.
    /// @param metadata DNG metadata
    MetadataSolver( const core::Metadata &metadata );

    /// Calculate an input transform matrix.
    /// @result calculated matrix
    std::vector<std::vector<double>> calculate_IDT_matrix();

    /// Calculate a chromatic adaptation transform matrix. Strictly speaking,
    /// this matrix is not required for image processing, as it is embedded in
    /// the IDT, see `calculate_IDT_matrix`.
    /// @result calculated matrix
    std::vector<std::vector<double>> calculate_CAT_matrix();

private:
    core::Metadata _metadata;
};

} // namespace core
} // namespace rta
