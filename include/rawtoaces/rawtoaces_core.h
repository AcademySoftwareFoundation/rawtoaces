// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <rawtoaces/spectral_data.h>

namespace rta
{
namespace core
{

/// Calculate spectral power distribution of a daylight illuminant of given CCT
/// - parameter cct: correlated colour temperature of the requested illuminant.
/// - parameter spectrum: a reference to a `Spectrum` object to full with the
/// calculated values.
void calculate_daylight_SPD( const int &cct, Spectrum &spectrum );

/// Calculate spectral power distribution of a blackbody illuminant of given CCT
/// - parameter cct: correlated colour temperature of the requested illuminant.
/// - parameter spectrum: a reference to a `Spectrum` object to full with the
/// calculated values.
void calculate_blackbody_SPD( const int &cct, Spectrum &spectrum );

/// Solve an input transform using spectral sensitivity curves of a camera.
class SpectralSolver
{
public:
    SpectralSolver();

    /// Load spectral sensitivity data for a camera.
    /// - parameter path: a path to the data file
    /// - parameter make: the camera make to verify the loaded data against.
    /// - parameter model: the camera model to verify the loaded data against.
    /// - returns `true` if loaded successfully.
    bool load_camera(
        const std::string &path,
        const std::string &make,
        const std::string &model );

    /// Load spectral power distribution data for an illuminant.
    /// If an illuminant type specified, try to find the matching data,
    /// otherwise load all known illuminants.
    /// - parameter paths: a set of data file paths to the data file
    /// - parameter type: illuminant type to load.
    /// - returns `true` if loaded successfully.
    bool load_illuminant(
        const std::vector<std::string> &paths, const std::string &type = "" );

    /// Load spectral reflectivity data for a training set (a colour chart).
    /// - parameter path: a path to the data file
    /// - returns `true` if loaded successfully.
    bool load_training_data( const std::string &path );

    /// Load spectral sensitivity data for an observer
    /// (colour matching functions).
    /// - parameter path: a path to the data file
    /// - returns `true` if loaded successfully.
    bool load_observer( const std::string &path );

    /// Find the illuminant best matching the given white-balancing multipliers.
    /// See `get_best_illuminant()` to access the result.
    /// - parameter wb_multipliers: white-balancing multipliers to match.
    /// - parameter highlight: the highlight recovery mode, used for
    /// normalisation.
    void find_best_illuminant(
        const std::vector<double> &wb_multipliers, int highlight );

    /// Select an illuminant of a given type.
    /// See `get_best_illuminant()` to access the result.
    /// - parameter type: illuminant type to select.
    /// - parameter highlight: the highlight recovery mode, used for
    /// normalisation.
    void select_illuminant( const std::string &type, int highlight );

    /// Calculate an input transform matrix.
    /// See `get_IDT_matrix()` to access the result.
    /// - returns `true` if calculated successfully.
    bool calculate_IDT_matrix();

    /// Get the illuminant configured using `find_best_illuminant()` or
    /// `select_illuminant()`.
    /// - returns a reference to the illuminant.
    const SpectralData &get_best_illuminant() const;

    /// Get the matrix calculated using `calculate_IDT_matrix()`.
    /// - returns a reference to the matrix.
    const std::vector<std::vector<double>> &get_IDT_matrix() const;

    /// Get the white-balance multipliers calculated using
    /// `find_best_illuminant()` or `select_illuminant()`.
    /// - returns a reference to the multipliers.
    const std::vector<double> &get_WB_multipliers() const;

    int verbosity = 0;

private:
    SpectralData              _camera;
    SpectralData              _best_illuminant;
    SpectralData              _observer;
    SpectralData              _training_data;
    std::vector<SpectralData> _illuminants;

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
    /// - parameter metadata: DNG metadata
    MetadataSolver( const core::Metadata &metadata );

    /// Calculate an input transform matrix.
    /// - returns: calculated matrix
    std::vector<std::vector<double>> calculate_IDT_matrix();

    /// Calculate a chromatic adaptation transform matrix. Strictly speaking,
    /// this matrix is not required for image processing, as it is embedded in
    /// the IDT, see `calculate_IDT_matrix`.
    /// - returns: calculated matrix
    std::vector<std::vector<double>> calculate_CAT_matrix();

private:
    core::Metadata _metadata;
};

} // namespace core
} // namespace rta
