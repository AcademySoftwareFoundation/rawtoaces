// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <rawtoaces/spectral_data.h>

namespace rta
{
namespace core
{

// clang-format off

// NOLINTBEGIN
/// Deprecated, not currently used and will be removed in v3.0.
static const std::vector<std::vector<double>> XYZ_to_ACES = {
    {  1.0498110175, 0.0000000000, -0.0000974845 },
    { -0.4959030231, 1.3733130458,  0.0982400361 },
    {  0.0000000000, 0.0000000000,  0.9912520182 }
};
// NOLINTEND

// NOLINTBEGIN
/// Colour adaptation from D65 to the ACES white point
/// Deprecated, not currently used and will be removed in v3.0.
static const std::vector<std::vector<double> > CAT_D65_to_ACES = {
    {  1.0097583639200136,      0.0050178093846550455, -0.015058389092388141  },
    {  0.0036602813378778347,   1.0030138169214682,    -0.0059802329456399824 },
    { -0.00029980928869024906, -0.0010516909063249997,  0.92820279627476576   }
};
// NOLINTEND

// clang-format on

/// Calculate spectral power distribution (SPD) of CIE standard daylight.
/// The function generates the spectral power distribution for a daylight
/// illuminant based on the requested correlated color temperature using CIE
/// standard formulas.
///
/// @param cct Correlated colour temperature of the requested illuminant either
/// in Kelvin (in range of 4000-25000), or in short form from an illuminant
/// name, e.g. 55 for D55 (in range of 40-250).
/// @param spectrum Reference to a `Spectrum` object to fill with the calculated
/// values
/// @pre cct is in valid range for daylight calculations
void calculate_daylight_SPD( const int &cct, Spectrum &spectrum );

/// Calculate spectral power distribution (SPD) of blackbody radiation at given
/// temperature. Generates a blackbody curve using Planck's law for the
/// specified correlated color temperature. The function calculates spectral
/// power distribution across wavelengths defined by Spectrum object.
///
/// @param kelvin Correlated colour temperature of the requested illuminant
/// (1500-3999 Kelvin)
/// @param spectrum Reference to a `Spectrum` object to fill with the calculated
/// values
/// @pre kelvin is in valid range for blackbody calculations (1500-3999)
void calculate_blackbody_SPD( const int &kelvin, Spectrum &spectrum );

/// Base class for a colour space transform solver.
class TransformSolver
{
public:
    /// Initialise the initial state of the solver.
    /// Sets the `transform_matrix` to an identity matrix and verbosity level
    /// to zero for silent operation.
    TransformSolver();

    virtual ~TransformSolver() = default;

    /// Calculate the transform matrix. Each subclass must implement this
    /// method.
    /// @return `true` on success.
    virtual bool calculate_transform() = 0;

    /// 3×3 colour transform matrix populated by `calculate_transform()` (starts
    /// as identity from the base `TransformSolver` constructor).
    std::vector<std::vector<double>> transform_matrix;

    /// Error message from the most recent method call that returned false.
    std::string last_error_message;

    /// Verbosity level.
    int verbosity;
};

/// Solve an input transform using spectral sensitivity curves of a camera.
class SpectralSolver : public TransformSolver
{
public:
    /// The camera spectral data. Can be either assigned directly, loaded
    /// in place via `solver.camera.load()`, or found via
    /// `solver.find_camera()`.
    SpectralData camera;

    /// The illuminant spectral data. Can be either assigned directly, loaded
    /// in place via `solver.illuminant.load()`, or found via
    /// `solver.find_illuminant()`.
    SpectralData illuminant;

    /// The observer spectral data. Can be either assigned directly, or loaded
    /// in place via `solver.observer.load()`.
    SpectralData observer;

    /// The training set spectral data. Can be either assigned directly, or loaded
    /// in place via `solver.training_data.load()`.
    SpectralData training_data;

    /// Initialize SpectralSolver with database search path.
    /// Sets white balance multipliers to neutral (1, 1, 1) and relies on the
    /// base `TransformSolver` constructor for an identity `transform_matrix` and
    /// zero verbosity. Takes the database search path as an optional parameter
    /// for finding spectral data files.
    ///
    /// @param search_directories optional database search path for spectral
    /// data files
    SpectralSolver( const std::vector<std::string> &search_directories = {} );

    /// A helper method collecting spectral data files of a given type from the
    /// database. This function searches through the configured search
    /// directories to find all spectral data files matching the specified type
    /// (e.g., "camera", "illuminant"). It searches for type subdirectories at
    /// the top level of each directory and returns JSON files matching the
    /// type.
    ///
    /// @param type data type of the files to search for (e.g., "camera",
    /// "illuminant", "cmf")
    /// @return a collection of file paths found in the database
    std::vector<std::string>
    collect_data_files( const std::string &type ) const;

    /// A helper method collecting aliases of a given type from the database.
    /// This function searches through the configured search directories to
    /// find all make/model aliases of the specified type ("camera" is the only
    /// currently supported type).
    ///
    /// @param type type of the aliases to search for (e.g., "camera")
    /// @param make_aliases output parameter referencing the map
    /// to be filled with the found make aliases.
    /// @param model_aliases output parameter referencing the map
    /// to be filled with the found model aliases.
    /// @return `true` on success.
    bool collect_aliases(
        const std::string                  &type,
        std::map<std::string, std::string> &make_aliases,
        std::map<
            std::string,
            std::map<std::string, std::pair<std::string, std::string>>>
            &model_aliases ) const;

    /// A helper method loading the spectral data for a file at the given path.
    /// This function loads spectral data from a file, handling both absolute
    /// and relative paths. For relative paths, it searches through all
    /// configured search directories.
    ///
    /// @param file_path the path to the file to load. If the path is relative,
    /// all locations in the search path will be searched in.
    /// @param out_data the `SpectralData` object to be filled with the loaded
    /// data.
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

    /// Find spectral power distribution data of an illuminant of the given
    /// type. This function can handle both built-in illuminant types (e.g.,
    /// "d55", "3200k") and custom illuminants stored in the database. For
    /// built-in types, it generates the spectral data using standard formulas.
    ///
    /// @param type illuminant type. Can be one of the built-in types,
    /// e.g. `d55`, `3200k`, or a custom illuminant stored in the database.
    /// @return `true` if loaded successfully, `false` otherwise
    bool find_illuminant( const std::string &type );

    /// Find the illuminant best matching the given white-balancing multipliers.
    /// This function analyzes all available illuminants and selects the one
    /// that best matches the white balance coefficients. It uses Sum of Squared
    /// Errors (SSE) to find the optimal match and automatically scales the
    /// white balance multipliers.
    ///
    /// @param wb_multipliers white-balancing multipliers to match
    /// @return `true` if loaded successfully, `false` otherwise
    bool find_illuminant( const std::vector<double> &wb_multipliers );

    /// Calculate the white-balance multipliers for the given configuration.
    /// This function computes RGB white balance multipliers by integrating
    /// camera spectral sensitivity with illuminant power spectrum. The
    /// multipliers normalize the camera response to achieve proper white
    /// balance under the specified illuminant conditions.
    /// The `camera` and `illuminant` data have to be configured prior to this
    /// call.
    ///
    /// @return `true` if calculated successfully, `false` otherwise
    /// @pre camera and illuminant data must be properly loaded
    bool calculate_WB();

    // clang-format off

    /// Calculate an input transform matrix using curve fitting optimization.
    /// This function computes the optimal IDT matrix by comparing camera RGB
    /// responses with target XYZ values across all training patches.
    /// The `camera`, `illuminant`, `observer` and `training_data` have to be
    /// configured prior to this call.
    ///
    /// @return `true` if calculated successfully, `false` otherwise
    /// @pre camera, illuminant, observer, and training_data must be properly
    /// loaded
    [[deprecated(
        "This method will be removed in v3. "
        "Use `calculate_transform()`" )]]
    bool calculate_IDT_matrix();

    /// Get the matrix from the last successful spectral solve (same data as
    /// `transform_matrix`).
    /// This function returns a reference to the 3×3 IDT matrix that transforms
    /// camera RGB values to standardized color space. The matrix is computed by
    /// curve fitting optimization and represents the optimal color
    /// transformation for the camera under the specified illuminant conditions.
    ///
    /// @return a reference to the 3×3 IDT transformation matrix
    /// @pre `calculate_transform()` or deprecated `calculate_IDT_matrix()`
    /// completed successfully
    [[deprecated(
        "This method will be removed in v3. "
        "Use `transform_matrix`" )]]
    const std::vector<std::vector<double>> &get_IDT_matrix() const;

    // clang-format on

    /// Get the white-balance multipliers calculated using `find_illuminant()`
    /// or `calculate_WB()`. This function returns a reference to the 3-element
    /// vector containing RGB white balance multipliers. These multipliers scale
    /// the camera response to achieve proper white balance under the specified
    /// illuminant conditions.
    ///
    /// @return a reference to the 3-element white balance multiplier vector
    /// [R, G, B]
    /// @pre white balance calculation must have been performed successfully
    const std::vector<double> &get_WB_multipliers() const;

    /// Solve the spectral IDT via curve fitting and store the 3×3 matrix in
    /// `transform_matrix`. Compares camera RGB responses with target XYZ values
    /// across training patches. Requires `camera`, `illuminant`, `observer`, and
    /// `training_data` to be configured prior to this call.
    ///
    /// @return `true` if calculated successfully, `false` otherwise
    /// @pre camera, illuminant, observer, and training_data must be properly
    /// loaded
    bool calculate_transform() override;

private:
    std::vector<std::string>  _search_directories;
    std::vector<SpectralData> _all_illuminants;

    std::vector<double> _wb_multipliers;
};

/// DNG metadata required to calculate an input transform.
struct Metadata
{
    /// A calibration data set structure. Contains calibration matrices for
    /// a given light source.
    struct Calibration
    {
        /// EXIF light source tag.
        /// The values [0..22] encode predefined light sources, the extended
        /// values (≥32768) can be used to encode an arbitrary CCT.
        unsigned short illuminant = 0;
        /// Camera calibration matrix.
        std::vector<double> camera_calibration_matrix;
        /// XYZ to camera RGB colour transform matrix.
        std::vector<double> XYZ_to_RGB_matrix;
    };

    /// Calibration datasets. Currently two sets are supported.
    struct Calibration calibration[2];

    /// Neutral RGB values. The colour channels in the camera RGB colour space
    /// representing a neutral colour.
    std::vector<double> neutral_RGB;

    /// Exposure calibration multiplier.
    double baseline_exposure = 0.0;
};

/// Solve an input transform using the metadata stored in DNG files.
class MetadataSolver : public TransformSolver
{
public:
    /// Initialize the solver using DNG metadata.
    /// Creates a MetadataSolver instance with the provided camera metadata
    /// for calculating IDT and CAT matrices.
    ///
    /// @param metadata DNG metadata containing camera calibration and exposure
    /// information
    MetadataSolver( const core::Metadata &metadata );

    // clang-format off

    /// Calculate the Input Device Transform (IDT) matrix for DNG color space
    /// conversion. This function computes the final IDT matrix that transforms
    /// camera RGB values to ACES RGB color space. It combines the Color
    /// Adaptation Transform (CAT) matrix with the D65 ACES RGB to XYZ
    /// transformation matrix to create a complete camera-to-ACES transformation
    /// pipeline.
    ///
    /// @return 3×3 Input Device Transform matrix for DNG to ACES conversion
    /// @pre _metadata must contain valid camera calibration data
    /// @note CAT is computed internally; `calculate_CAT_matrix()` does not need
    /// to be called first.
    [[deprecated(
        "This method will be removed in v3. "
        "Use `calculate_transform()`" )]]
    std::vector<std::vector<double>> calculate_IDT_matrix();

    /// Calculate the Color Adaptation Transform (CAT) matrix for color space
    /// conversion. This function computes the CAT matrix needed to transform
    /// colors from the camera's white point to the target ACES RGB white point.
    /// It first obtains the camera's XYZ transformation matrix and white point,
    /// then creates the target ACES RGB to XYZ matrix, and finally calculates
    /// the color adaptation transform between the two white points using the
    /// Bradford or CAT02 method.
    ///
    /// The CAT matrix is essential for maintaining color appearance when
    /// converting between different illuminant conditions, ensuring that colors
    /// look consistent across different lighting environments. Strictly
    /// speaking, this matrix is not required for image processing, as it is
    /// embedded in the IDT; see `calculate_transform()` or deprecated
    /// `calculate_IDT_matrix()`.
    ///
    /// @return 3×3 Color Adaptation Transform matrix
    /// @pre _metadata must contain valid camera calibration and neutral RGB
    /// data
    [[deprecated(
        "This method will be removed in v3. "
        "Use `calculate_transform()`" )]]
    std::vector<std::vector<double>> calculate_CAT_matrix();

    // clang-format on

    /// Calculate the Input Device Transform (IDT) matrix for DNG color space
    /// conversion. This function computes the final IDT matrix that transforms
    /// camera RGB values to ACES RGB color space. It combines the Color
    /// Adaptation Transform (CAT) matrix with the D65 ACES RGB to XYZ
    /// transformation matrix to create a complete camera-to-ACES transformation
    /// pipeline.
    ///
    /// On success, writes the same 3×3 matrix as deprecated `calculate_IDT_matrix()`
    /// into `transform_matrix` and returns `true`.
    ///
    /// @return `true` on success
    /// @pre _metadata must contain valid camera calibration data
    bool calculate_transform() override;

private:
    core::Metadata _metadata;
};

/// Fixed transform for XYZ data with a D65 white point: chromatic adaptation
/// from D65 toward the ACES white point, combined with XYZ D60 to ACES.
class XYZD65Solver : public TransformSolver
{
public:
    /// Sets `transform_matrix` to the built-in D65 adaptation × XYZ D60→ACES
    /// product. Always succeeds if invoked.
    /// @return `true`
    bool calculate_transform() override;
};

} // namespace core
} // namespace rta
