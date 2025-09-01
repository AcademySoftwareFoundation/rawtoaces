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
/// @param cct Correlated colour temperature of the requested illuminant (40-250 or 4000-25000 Kelvin)
/// @param spectrum Reference to a `Spectrum` object to fill with the calculated values
/// @pre cct is in valid range for daylight calculations
void calculate_daylight_SPD( const int &cct, Spectrum &spectrum );

/// Calculate spectral power distribution (SPD) of blackbody radiation at given temperature.
/// Generates a blackbody curve using Planck's law for the specified correlated color temperature.
/// The function calculates spectral power distribution across visible wavelengths (380-780nm).
///
/// @param cct Correlated colour temperature of the requested illuminant (1500-3999 Kelvin)
/// @param spectrum Reference to a `Spectrum` object to fill with the calculated values
/// @pre cct is in valid range for blackbody calculations (1500-3999)
void calculate_blackbody_SPD( const int &cct, Spectrum &spectrum );

/// Solve an input transform using spectral sensitivity curves of a camera.
class SpectralSolver
{
public:
    /// Initialize SpectralSolver with default settings.
    /// Sets up internal data structures including IDT matrix and white balance multipliers
    /// with neutral values. Initializes verbosity level to 0 for silent operation.
    SpectralSolver();

    /// Load camera sensitivity data from file and validate against manufacturer/model.
    /// Loads camera spectral sensitivity data from the specified file path and verifies
    /// that the loaded data matches the expected camera manufacturer and model from libraw.
    /// The function validates the data integrity before storing it in the internal camera data.
    ///
    /// @param path Path to the camera sensitivity file
    /// @param make Camera manufacturer name (from libraw metadata)
    /// @param model Camera model name (from libraw metadata)
    /// @return true if successfully loaded and validated, false otherwise
    /// @pre path, make, and model are non-empty strings
    bool load_camera(
        const std::string &path,
        const std::string &make,
        const std::string &model );

    /// Load illuminant data from files or generate standard illuminants.
    /// This function loads illuminant spectral data from specified file paths or generates
    /// standard daylight and blackbody illuminants if no specific type is provided. When a
    /// type is specified, it can handle daylight (e.g., "D50", "D65") or blackbody (e.g., "3200K")
    /// illuminants by parsing the type string and generating appropriate spectral data.
    ///
    /// @param paths Vector of file paths to illuminant data files
    /// @param type Type of light source (e.g., "D50", "D65", "3200K") or empty for auto-generation
    /// @return true if illuminants were successfully loaded or generated, false otherwise
    /// @pre paths vector contains valid file paths when type is empty
    bool load_illuminant(
        const std::vector<std::string> &paths, const std::string &type = "" );

    /// Load the training data for spectral calculations.
    /// This function loads training data from a file containing spectral information
    /// for color patches. The training data is used for calibrating and
    /// optimizing spectral calculations in the color pipeline.
    ///
    /// @param path Path to the training data file
    /// @return true if training data was successfully loaded, false otherwise
    /// @pre path points to a valid training data file
    bool load_training_data( const std::string &path );

    /// Load the Color Matching Functions data for standard observer.
    /// This function loads the standard observer color matching functions
    /// from a file. These functions define how the human eye perceives color across
    /// the visible spectrum and are essential for color space transformations.
    ///
    /// @param path Path to the Color Matching Functions data file
    /// @return true if observer data was successfully loaded, false otherwise
    /// @pre path points to a valid CMF data file
    bool load_observer( const std::string &path );

    /// Choose the best illuminant based on white balance coefficients from camera metadata.
    /// This function analyzes all available illuminants and selects the one that best matches
    /// the white balance coefficients read from the camera. It uses Sum of Squared Errors (SSE)
    /// to find the optimal match and automatically scales the white balance multipliers.
    ///
    /// @param wb_multipliers White balance coefficients from camera metadata
    /// @param highlight Highlight recovery mode for normalization
    void find_best_illuminant(
        const std::vector<double> &wb_multipliers, int highlight );

    /// Select a specific illuminant by type and calculate white balance multipliers.
    /// This function sets the best illuminant to a user-specified type and calculates
    /// the corresponding white balance multipliers for the camera-illuminant combination.
    /// The function automatically scales the white balance factors for normalization.
    ///
    /// @param type The illuminant type to select (must match first illuminant in list)
    /// @param highlight Highlight recovery mode for normalization
    void select_illuminant( const std::string &type, int highlight );

    /// Calculate the Input Device Transform (IDT) matrix using curve fitting optimization.
    /// This function computes the optimal IDT matrix by comparing camera RGB responses
    /// with target XYZ values across all training patches. It uses the Ceres optimization
    /// library to find the best 6-parameter transformation that minimizes color differences.
    /// The resulting IDT matrix transforms camera RGB values to standardized color space.
    ///
    /// @return true if IDT matrix was successfully calculated, false otherwise
    bool calculate_IDT_matrix();

    /// Get the best matching illuminant data that was determined during optimization.
    /// This function returns a reference to the illuminant that best matches the camera's
    /// white balance coefficients. The illuminant is selected based on spectral analysis
    /// and optimization results from the find_best_illuminant() or select_illuminant() calls.
    ///
    /// @return Reference to the best matching illuminant spectral data
    /// @pre illuminant data must be properly loaded with main section and power spectrum
    const SpectralData &get_best_illuminant() const;

    /// Get the computed Input Device Transform (IDT) matrix if calculation succeeded.
    /// This function returns a reference to the 3×3 IDT matrix that transforms camera
    /// RGB values to standardized color space. The matrix is computed by curve fitting
    /// optimization and represents the optimal color transformation for the camera under
    /// the specified illuminant conditions.
    ///
    /// @return Reference to the 3×3 IDT transformation matrix
    /// @pre calculate_IDT_matrix() must have been called successfully
    const std::vector<std::vector<double>> &get_IDT_matrix() const;

    /// Get the white balance multipliers if white balance calculation succeeded.
    /// This function returns a reference to the 3-element vector containing RGB white
    /// balance multipliers. These multipliers normalize the camera response to achieve
    /// proper white balance under the specified illuminant conditions and are computed
    /// by the calculate_WB() function during illuminant selection or optimization.
    ///
    /// @return Reference to the 3-element white balance multiplier vector [R, G, B]
    /// @pre white balance calculation must have been performed successfully
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
    /// across different lighting environments.
    ///
    /// @return 3×3 Color Adaptation Transform matrix
    /// @pre _metadata must contain valid camera calibration and neutral RGB data
    std::vector<std::vector<double>> calculate_CAT_matrix();

private:
    core::Metadata _metadata;
};

} // namespace core
} // namespace rta
