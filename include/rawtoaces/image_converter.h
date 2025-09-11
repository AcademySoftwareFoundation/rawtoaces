// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/argparse.h>

namespace rta
{
namespace util
{

/// Collect all files from given `paths` into batches.
/// For each path that is a directory, entries are created in the returned batches
/// and fill it with the file names. Invalid paths are skipped with an error message.
/// First batch is reserved for all paths that are files. If no such paths are provided,
/// first batch will be empty.
///
/// @param paths vector of paths to files or directories to process.
/// @return vector of batches, where each batch contains files from one input path.
std::vector<std::vector<std::string>>
collect_image_files( const std::vector<std::string> &paths );

class ImageConverter
{
public:
    struct Settings
    {
        /// The  white balancing method to use for conversion can be specified
        ///
        enum class WBMethod
        {
            /// Use the metadata provided in the image file. This mode is mostly
            /// usable with DNG files, as the information needed for conversion
            /// is mandatory in the DNG format.
            Metadata,
            /// White balance to a specified illuminant. See the `illuminant`
            /// property for more information on the supported illuminants. This
            /// mode can only be used if spectral sensitivities are available for
            /// the camera.
            Illuminant,
            /// Calculate white balance by averaging over a specified region of
            /// the image. See `WB_box`. In this mode if an empty box if provided,
            /// white balancing is done by averaging over the whole image.
            Box,
            /// Use custom white balancing multipliers. This mode is useful if
            /// the white balancing coefficients are calculated by an external
            /// tool.
            Custom
        } WB_method = WBMethod::Metadata;

        enum class MatrixMethod
        {
            /// Use the camera spectral sensitivity curves to solve for the colour
            /// conversion matrix. In this mode the illuminant is either provided
            /// directly in `illuminant` if `WB_method` ==
            /// `WBMethod::Illuminant`, or the best illuminant is derived from the
            /// white balancing multipliers.
            Spectral,
            /// Use the metadata provided in the image file. This mode is mostly
            /// usable with DNG files, as the information needed for conversion
            /// is mandatory in the DNG format.
            Metadata,
            /// Use the Adobe colour matrix for the camera supplied in LibRaw.
            Adobe,
            /// Specify a custom matrix in `colourMatrix`. This mode is useful if
            /// the matrix is calculated by an external tool.
            Custom
        } matrix_method = MatrixMethod::Spectral;

        /// Cropping mode.
        enum class CropMode
        {
            /// Write out full sensor area.
            Off,
            /// Write out full sensor area, mark the crop area as the display window.
            Soft,
            /// Write out only the crop area.
            Hard
        } crop_mode = CropMode::Hard;

        /// An illuminant to use for white balancing and/or colour matrix
        /// calculation. Only used when `WB_method` ==
        /// `WBMethod::Illuminant` and `matrix_method` == `MatrixMethod::Spectral`.
        /// An illuminant can be provided as a black body correlated colour
        /// temperature, like `3200K`; or a D-series illuminant, like `D56`;
        /// or any other illuminant, in such case it must be present in the data
        /// folder.
        std::string illuminant;

        float headroom            = 6.0;
        int   WB_box[4]           = { 0 };
        float custom_WB[4]        = { 1.0, 1.0, 1.0, 1.0 };
        float custom_matrix[3][3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };

        std::string custom_camera_make;
        std::string custom_camera_model;

        // Libraw-specific options:
        bool        auto_bright              = false;
        float       adjust_maximum_threshold = 0.75;
        int         black_level              = -1;
        int         saturation_level         = -1;
        bool        half_size                = false;
        int         highlight_mode           = 0;
        int         flip                     = 0;
        int         crop_box[4]              = { 0, 0, 0, 0 };
        float       chromatic_aberration[2]  = { 1.0f, 1.0f };
        float       denoise_threshold        = 0;
        float       scale                    = 1.0f;
        std::string demosaic_algorithm       = "AHD";

        // Global config:
        std::vector<std::string> database_directories;
        bool                     overwrite   = false;
        bool                     create_dirs = false;
        std::string              output_dir;

        // Diagnostic:
        bool use_timing = false;
        int  verbosity  = 0;

    } settings;

    /// Initialise the parser object with all the command line parameters
    /// used by this tool. The method also sets the help and usage strings.
    /// The parser object can be amended by the calling code afterwards if
    /// needed. This method is optional, all of the settings above can be
    /// modified directly.
    /// @param arg_parser The command line parser object to be updated.
    void init_parser( OIIO::ArgParse &arg_parser );

    /// Initialise the converter settings from the command line parser object.
    /// Prior to calling this, first initialise the object via
    /// `ImageConverted::init_parser()`, and call
    /// `OIIO::ArgParse::parse_args()`.
    /// This method is optional, all of the settings above can be modified
    /// directly.
    /// @param arg_parser the command line parser object
    /// @result `true` if parsed successfully
    bool parse_parameters( const OIIO::ArgParse &arg_parser );

    /// Collects all illuminants supported by this version.
    std::vector<std::string> supported_illuminants();

    /// Collects all camera models for which spectral sensitivity data is
    /// available in the database.
    std::vector<std::string> supported_cameras();

    /// Configures the converter using the requested white balance and colour
    /// matrix method, and the metadata of the file provided in `input_file`.
    /// This method loads the metadata from the given image file and
    /// initialises the options to give the OIIO raw image reader to
    /// decode the pixels.
    /// @param input_filename
    ///    A file name of the raw image file to read the metadata from.
    /// @param options
    ///    Conversion hints to be passed to OIIO when reading an image file.
    ///    The list can be pre- or post- updated with other hints, unrelated to
    ///    the rawtoaces conversion.
    /// @result
    ///    `true` if configured successfully.
    bool configure(
        const std::string &input_filename, OIIO::ParamValueList &options );

    /// Configures the converter using the requested white balance and colour
    /// matrix method, and the metadata of the given OIIO::ImageSpec object.
    /// Use this method if you already have an image read from file to save
    /// on disk operations.
    /// @param imageSpec
    ///    An image spec obtained from OIIO::ImageInput or OIIO::ImageBuf.
    /// @param hints
    ///    Conversion hints to be passed to OIIO when reading an image file.
    ///    The list can be pre- or post- updated with other hints, unrelated to
    ///    the rawtoaces conversion.
    /// @result
    ///    `true` if configured successfully.
    bool
    configure( const OIIO::ImageSpec &imageSpec, OIIO::ParamValueList &hints );

    /// Load an image from a given `path` into a `buffer` using the `hints`
    /// calculated by the `configure` method. The hints can be manually
    /// modified prior to invoking this method.
    bool load_image(
        const std::string          &path,
        const OIIO::ParamValueList &hints,
        OIIO::ImageBuf             &buffer );

    /// Apply the colour space conversion matrix (or matrices) to convert the
    /// image buffer from the raw camera colour space to ACES.
    /// @param dst
    ///     Destination image buffer.
    /// @param src
    ///     Source image buffer, can be the same as `dst` for in-place
    ///     conversion.
    /// @result
    ///    `true` if applied successfully.
    bool apply_matrix(
        OIIO::ImageBuf &dst, const OIIO::ImageBuf &src, OIIO::ROI roi = {} );

    /// Apply the headroom scale to image buffer.
    /// @param dst
    ///     Destination image buffer.
    /// @param src
    ///     Source image buffer, can be the same as `dst` for in-place
    ///     conversion.
    /// @result
    ///    `true` if applied successfully.
    bool apply_scale(
        OIIO::ImageBuf &dst, const OIIO::ImageBuf &src, OIIO::ROI roi = {} );

    /// Apply the cropping mode as specified in crop_mode.
    /// @param dst
    ///     Destination image buffer.
    /// @param src
    ///     Source image buffer, can be the same as `dst` for in-place
    ///     conversion.
    /// @result
    ///    `true` if applied successfully.
    bool apply_crop(
        OIIO::ImageBuf &dst, const OIIO::ImageBuf &src, OIIO::ROI roi = {} );

    /// Make output file path and check if it is writable.
    /// @param path
    ///     A reference to a variable containing the input file path. The output file path gets generated
    ///     in-place.
    /// @param suffix
    ///     A suffix to add to the file name.
    /// @result
    ///    `true` if the file can be written, e.g. the output directory exists, or creating directories
    ///     is allowed; the file does not exist or overwriting is allowed.
    bool
    make_output_path( std::string &path, const std::string &suffix = "_aces" );

    /// Saves the image into ACES Container.
    /// @param output_filename
    ///     Full path to the file to be saved.
    /// @param buf
    ///     Image buffer to be saved.
    /// @result
    ///    `true` if saved successfully.
    bool
    save_image( const std::string &output_filename, const OIIO::ImageBuf &buf );

    /// A convenience single-call method to process an image. This is equivalent to calling the following
    /// methods sequentially: `make_output_path`->`configure`->`apply_matrix`->
    /// `apply_scale`->`apply_crop`->`save`.
    /// @param input_filename
    ///     Full path to the file to be converted.
    /// @result
    ///    `true` if processed successfully.
    bool process_image( const std::string &input_filename );

    /// Get the solved white balance multipliers of the currently processed
    /// image. The multipliers become available after calling either of the
    /// two `configure` methods.
    /// @result a reference to the multipliers vector.
    const std::vector<double> &get_WB_multipliers();

    /// Get the solved input transform matrix of the currently processed image.
    /// The multipliers become available after calling either of the two
    /// `configure` methods.
    /// @result a reference to the matrix.
    const std::vector<std::vector<double>> &get_IDT_matrix();

    /// Get the solved chromatic adaptation transform matrix of the currently
    /// processed image. The multipliers become available after calling either
    /// of the two `configure` methods.
    /// @result a reference to the matrix.
    const std::vector<std::vector<double>> &get_CAT_matrix();

private:
    // Solved transform of the current image.
    std::vector<std::vector<double>> _IDT_matrix;
    std::vector<std::vector<double>> _CAT_matrix;
    std::vector<double>              _WB_multipliers;
};

} //namespace util
} //namespace rta
