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

/// Calculate spectral power distribution (SPD) of CIE standard daylight illuminant.
/// The function generates the spectral power distribution for a daylight illuminant
/// based on the requested correlated color temperature using CIE standard formulas.
///
/// @param cct Correlated colour temperature of the requested illuminant either in Kelvin (in range of 4000-25000), or in short form from an illuminant name, e.g. 55 for D55 (in range of 40-250).
/// @param spectrum Reference to a `Spectrum` object to fill with the calculated values
/// @pre cct is in valid range for daylight calculations
void calculate_daylight_SPD( const int &cct, Spectrum &spectrum );

/// Calculate spectral power distribution (SPD) of blackbody radiation at given temperature.
/// Generates a blackbody curve using Planck's law for the specified correlated color temperature.
/// The function calculates spectral power distribution across wavelengths defined by Spectrum object.
///
/// @param cct Correlated colour temperature of the requested illuminant (1500-3999 Kelvin)
/// @param spectrum Reference to a `Spectrum` object to fill with the calculated values
/// @pre cct is in valid range for blackbody calculations (1500-3999)
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

    /// Initialize SpectralSolver with database search path.
    /// Sets up internal data structures including IDT matrix and white balance multipliers
    /// with neutral values. Initializes verbosity level to 0 for silent operation.
    /// Takes the database search path as an optional parameter for finding spectral data files.
    ///
    /// @param search_directories optional database search path for spectral data files
    SpectralSolver( const std::vector<std::string> &search_directories = {} );

    /// A helper method collecting spectral data files of a given type from the database.
    /// This function searches through the configured search directories to find all
    /// spectral data files matching the specified type (e.g., "camera", "illuminant").
    /// It searches for type subdirectories at the top level of each directory and returns JSON files matching the type.
    ///
    /// @param type data type of the files to search for (e.g., "camera", "illuminant", "cmf")
    /// @return a collection of file paths found in the database
    std::vector<std::string>
    collect_data_files( const std::string &type ) const;

    /// A helper method loading the spectral data for a file at the given path.
    /// This function loads spectral data from a file, handling both absolute and relative paths.
    /// For relative paths, it searches through all configured search directories.
    ///
    /// @param file_path the path to the file to load. If the path is relative,
    /// all locations in the search path will be searched in.
    /// @param out_data the `SpectralData` object to be filled with the loaded data.
    /// @return `true` if loaded successfully, `false` otherwise
    bool
    load_spectral_data( const std::string &file_path, SpectralData &out_data );

    /// Load spectral sensitivity data for a camera by searching the database.
    /// This function searches through camera data files in the database to find
    /// a match for the specified camera manufacturer and model. It loads the
    /// spectral sensitivity data into the camera member variable.
    ///
    /// @param make the camera make to search for
    /// @param model the camera model to search for
    /// @return `true` if loaded successfully, `false` otherwise
    bool find_camera( const std::string &make, const std::string &model );

    /// Find spectral power distribution data of an illuminant of the given type.
    /// This function can handle both built-in illuminant types (e.g., "d55", "3200k")
    /// and custom illuminants stored in the database. For built-in types, it generates
    /// the spectral data using standard formulas.
    ///
    /// @param type illuminant type. Can be one of the built-in types,
    /// e.g. `d55`, `3200k`, or a custom illuminant stored in the database.
    /// @return `true` if loaded successfully, `false` otherwise
    bool find_illuminant( const std::string &type );

    /// Find the illuminant best matching the given white-balancing multipliers.
    /// This function analyzes all available illuminants and selects the one that best matches
    /// the white balance coefficients. It uses Sum of Squared Errors (SSE) to find the
    /// optimal match and automatically scales the white balance multipliers.
    ///
    /// @param wb_multipliers white-balancing multipliers to match
    /// @return `true` if loaded successfully, `false` otherwise
    bool find_illuminant( const std::vector<double> &wb_multipliers );

    /// Calculate the white-balance multipliers for the given configuration.
    /// This function computes RGB white balance multipliers by integrating camera spectral
    /// sensitivity with illuminant power spectrum. The multipliers normalize the camera
    /// response to achieve proper white balance under the specified illuminant conditions.
    /// The `camera` and `illuminant` data have to be configured prior to this call.
    ///
    /// @return `true` if calculated successfully, `false` otherwise
    /// @pre camera and illuminant data must be properly loaded
    bool calculate_WB();

    /// Calculate an input transform matrix using curve fitting optimization.
    /// This function computes the optimal IDT matrix by comparing camera RGB responses
    /// with target XYZ values across all training patches.
    /// The `camera`, `illuminant`, `observer` and `training_data` have to be configured prior to this call.
    ///
    /// @return `true` if calculated successfully, `false` otherwise
    /// @pre camera, illuminant, observer, and training_data must be properly loaded
    bool calculate_IDT_matrix();

    /// Get the matrix calculated using `calculate_IDT_matrix()`.
    /// This function returns a reference to the 3×3 IDT matrix that transforms camera
    /// RGB values to standardized color space. The matrix is computed by curve fitting
    /// optimization and represents the optimal color transformation for the camera under
    /// the specified illuminant conditions.
    ///
    /// @return a reference to the 3×3 IDT transformation matrix
    /// @pre calculate_IDT_matrix() must have been called successfully
    const std::vector<std::vector<double>> &get_IDT_matrix() const;

    /// Get the white-balance multipliers calculated using `find_illuminant()` or `calculate_WB()`.
    /// This function returns a reference to the 3-element vector containing RGB white
    /// balance multipliers. These multipliers scale the camera response to achieve
    /// proper white balance under the specified illuminant conditions.
    ///
    /// @return a reference to the 3-element white balance multiplier vector [R, G, B]
    /// @pre white balance calculation must have been performed successfully
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
    /// Initialize the solver using DNG metadata.
    /// Creates a MetadataSolver instance with the provided camera metadata
    /// for calculating IDT and CAT matrices.
    ///
    /// @param metadata DNG metadata containing camera calibration and exposure information
    MetadataSolver( const core::Metadata &metadata );

    /// Calculate the Input Device Transform (IDT) matrix for DNG color space conversion.
    /// This function computes the final IDT matrix that transforms camera RGB values
    /// to ACES RGB color space. It combines the Color Adaptation Transform (CAT) matrix
    /// with the D65 ACES RGB to XYZ transformation matrix to create a complete
    /// camera-to-ACES transformation pipeline.
    ///
    /// @return 3×3 Input Device Transform matrix for DNG to ACES conversion
    /// @pre _metadata must contain valid camera calibration data
    /// @pre calculate_CAT_matrix() must return a valid CAT matrix
    std::vector<std::vector<double>> calculate_IDT_matrix();

    /// Calculate the Color Adaptation Transform (CAT) matrix for color space conversion.
    /// This function computes the CAT matrix needed to transform colors from the camera's
    /// white point to the target ACES RGB white point. It first obtains the camera's
    /// XYZ transformation matrix and white point, then creates the target ACES RGB to XYZ
    /// matrix, and finally calculates the color adaptation transform between the two
    /// white points using the Bradford or CAT02 method.
    ///
    /// The CAT matrix is essential for maintaining color appearance when converting
    /// between different illuminant conditions, ensuring that colors look consistent
    /// across different lighting environments. Strictly speaking, this matrix is not
    /// required for image processing, as it is embedded in the IDT, see `calculate_IDT_matrix`.
    ///
    /// @return 3×3 Color Adaptation Transform matrix
    /// @pre _metadata must contain valid camera calibration and neutral RGB data
    std::vector<std::vector<double>> calculate_CAT_matrix();

private:
    core::Metadata _metadata;
};

} // namespace core
} // namespace rta
