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
    /// Status codes for operation results.
    enum class Status
    {
        /// Operation completed successfully.
        Success,
        /// Output file already exists and overwrite is not enabled.
        FileExists,
        /// Input file does not exist.
        InputFileNotFound,
        /// Empty input filename provided.
        EmptyInputFilename,
        /// Filesystem error occurred.
        FilesystemError,
        /// Output directory does not exist and cannot be created.
        OutputDirectoryError,
        /// Invalid path format.
        InvalidPath,
        /// Failed to configure the image reader.
        ConfigurationError,
        /// Failed to read the image file.
        ReadError,
        /// Failed to apply colour space conversion.
        MatrixApplicationError,
        /// Failed to apply scale.
        ScaleApplicationError,
        /// Failed to apply crop.
        CropApplicationError,
        /// Failed to save the output file.
        WriteError,
        /// Unknown error.
        UnknownError
    };

    /// The structure containing all parameters needed to configure image conversion.
    struct Settings
    {
        /// The enumerator containing all supported white-balancing methods.
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
        };

        /// The selected white-balancing method to use for conversion.
        WBMethod WB_method = WBMethod::Metadata;

        /// The enumerator containing all supported colour transform matrix calculation methods.
        enum class MatrixMethod
        {
            /// Automatically choose the best available matrix method.
            /// - If spectral sensitivity data for the camera is available,
            ///   use `Spectral`.
            /// - Otherwise, fall back to `Metadata`.
            Auto,
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
        };

        /// The selected colour transform matrix calculation method to use for conversion.
        MatrixMethod matrix_method = MatrixMethod::Auto;

        /// The enumerator containing all supported cropping modes.
        enum class CropMode
        {
            /// Write out full sensor area.
            Off,
            /// Write out full sensor area, mark the crop area as the display window.
            Soft,
            /// Write out only the crop area.
            Hard
        };

        /// The selected cropping mode to use for conversion.
        CropMode crop_mode = CropMode::Hard;

        /// An illuminant to use for white balancing and/or colour matrix
        /// calculation. Only used when `WB_method` ==
        /// `WBMethod::Illuminant` and `matrix_method` == `MatrixMethod::Spectral`.
        /// An illuminant can be provided as a black body correlated colour
        /// temperature, like `3200K`; or a D-series illuminant, like `D56`;
        /// or any other illuminant, in such case it must be present in the data
        /// folder.
        std::string illuminant;

        /// Highlight headroom factor.
        float headroom = 6.0;

        /// Box to use for white balancing when `WB_method` == `WBMethod::Box`.
        /// (default = (0,0,0,0) - full image)
        int WB_box[4] = { 0, 0, 0, 0 };

        /// Custom white balance multipliers to be used when
        /// `WB_method` == `WBMethod::Custom`.
        float custom_WB[4] = { 1.0, 1.0, 1.0, 1.0 };

        /// Custom camera RGB to XYZ matrix to be used when
        /// `matrix_method` == `MatrixMethod::Custom`.
        float custom_matrix[3][3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };

        /// Camera manufacturer name to be used for spectral sensitivity
        /// curves lookup.
        std::string custom_camera_make;

        /// Camera model name to be used for spectral sensitivity
        /// curves lookup.
        std::string custom_camera_model;

        ///////////////////////////
        // Libraw-specific options:

        /// Enable automatic exposure adjustment.
        bool auto_bright = false;

        /// Automatically lower the linearity threshold provided in the
        /// metadata by this scaling factor.
        float adjust_maximum_threshold = 0.75;

        /// If >= 0, override the black level specified in the file metadata.
        int black_level = -1;

        /// If >= 0, override the saturation level specified in the file
        ///  metadata.
        int saturation_level = -1;

        /// Decode the image at half size resolution.
        bool half_size = false;

        /// Highlight recovery mode, as supported by OpenImageIO/Libraw
        /// 0 = clip, 1 = unclip, 2 = blend, 3..9 = rebuild.
        int highlight_mode = 0;

        /// If not 0, override the orientation specified in the metadata.
        /// 1..8 correspond to EXIF orientation codes
        /// (3 = 180 deg, 6 = 90 deg CCW, 8 = 90 deg CW.)
        int flip = 0;

        /// Apply custom crop. If not specified (all values are zeroes),
        /// the default crop is applied, which should match the crop of the
        /// in-camera JPEG.
        int crop_box[4] = { 0, 0, 0, 0 };

        /// Red and blue scale factors for chromatic aberration correction.
        float chromatic_aberration[2] = { 1.0f, 1.0f };

        /// Wavelet denoising threshold.
        float denoise_threshold = 0;

        /// Additional scaling factor to apply to the pixel values.
        float scale = 1.0f;

        /// Demosaicing algorithm. Supported options: 'linear', 'VNG', 'PPG',
        /// 'AHD', 'DCB', 'AHD-Mod', 'AFD', 'VCD', 'Mixed', 'LMMSE', 'AMaZE',
        /// 'DHT', 'AAHD', 'AHD'.
        std::string demosaic_algorithm = "AHD";

        /////////////////
        // Global config:

        /// Directory containing rawtoaces spectral sensitivity and illuminant
        /// data files. Overrides the default search path and the
        /// RAWTOACES_DATA_PATH environment variable.
        std::vector<std::string> database_directories;

        /// Allows overwriting existing files.
        bool overwrite = false;

        /// Create output directories if they don't exist.
        bool create_dirs = false;

        /// The directory to write the output files to.
        std::string output_dir;

        //////////////
        // Diagnostic:

        /// Log the execution time of each step of image processing.
        bool use_timing = false;

        /// Disable caching.
        bool disable_cache = false;

        /// Disable calling exiftool to fetch missing metadata.
        bool disable_exiftool = false;

        /// Verbosity level.
        int verbosity = 0;
    };

    /// The conversion settings.
    Settings settings;

    /// This property holds the error code from the most recent method call that returns a bool. 
    Status status = Status::Success;

    /// Error message from the most recent method call that returned false.
    std::string last_error_message;

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

    /// Collects all camera raw formats supported by this version.
    std::vector<std::string> get_supported_formats() const;

    /// Collects all illuminants supported by this version.
    std::vector<std::string> get_supported_illuminants() const;

    /// Collects all camera models for which spectral sensitivity data is
    /// available in the database.
    std::vector<std::string> get_supported_cameras() const;

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
    /// @param options
    ///    Conversion hints to be passed to OIIO when reading an image file.
    ///    The list can be pre- or post- updated with other hints, unrelated to
    ///    the rawtoaces conversion.
    /// @result
    ///    `true` if configured successfully.
    bool configure(
        const OIIO::ImageSpec &imageSpec, OIIO::ParamValueList &options );

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
    const std::vector<double> &get_WB_multipliers() const;

    /// Get the solved input transform matrix of the currently processed image.
    /// The multipliers become available after calling either of the two
    /// `configure` methods.
    /// @result a reference to the matrix.
    const std::vector<std::vector<double>> &get_IDT_matrix() const;

    /// Get the solved chromatic adaptation transform matrix of the currently
    /// processed image. The multipliers become available after calling either
    /// of the two `configure` methods.
    /// @result a reference to the matrix.
    const std::vector<std::vector<double>> &get_CAT_matrix() const;

private:
    // Solved transform of the current image.
    std::vector<std::vector<double>> _idt_matrix;
    std::vector<std::vector<double>> _cat_matrix;
    std::vector<double>              _wb_multipliers;
};

} //namespace util
} //namespace rta
