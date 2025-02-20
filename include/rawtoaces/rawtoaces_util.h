// Copyright Contributors to the rawtoaces project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/rawtoaces

#ifndef _RAWTOACES_UTIL_H_
#define _RAWTOACES_UTIL_H_

#include <OpenImageIO/argparse.h>
#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>

namespace rta
{

/// An image converter converts an image read from a camera raw image file
/// into an ACESContainer compatible image.
class ImageConverter
{
public:
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
        /// the image. See `wbBox`. In this mode if an empty box if provided,
        /// white balancing is done by averaging over the whole image.
        Box,
        /// Use custom white balancing multipliers. This mode is useful if
        /// the white balancing coefficients are calculated by an external
        /// tool.
        Custom
    } wbMethod = WBMethod::Metadata;

    enum class MatrixMethod
    {
        /// Use the camera spectral sensitivity curves to solve for the colour
        /// conversion matrix. In this mode the illuminant is either provided
        /// directly in `illuminant` if `wbMethod` ==
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
    } matrixMethod = MatrixMethod::Spectral;

    /// An illuminant to use for white balancing and/or colour matrix
    /// calculation. Only used when `wbMethod` ==
    /// `WBMethod::Illuminant` or `matrixMethod` == `MatrixMethod::Spectral`.
    /// An illuminant can be provided as a black body correlated colour
    /// temperature, like `3200K`; or a D-series illuminant, like `D56`;
    /// or any other illuminant, in such case it must be present in the data
    /// folder.
    std::string illuminant;

    float headroom           = 6.0;
    int   wbBox[4]           = { 0 };
    float customWB[4]        = { 1.0, 1.0, 1.0, 1.0 };
    float customMatrix[3][3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };

    bool  no_auto_bright           = false;
    float adjust_maximum_threshold = 0.75;
    int   black_level              = -1;
    int   saturation_level         = -1;
    bool  half_size                = false;
    int   highlight_mode           = 0;
    int   flip                     = 0;
    int   cropbox[4]               = { 0, 0, 0, 0 };
    int   verbosity                = 0;

    bool        overwrite   = false;
    bool        create_dirs = false;
    std::string output_dir;

    /// Initialise the parser object with all the command line parameters
    /// used by this tool. The method also sets the help and usage strings.
    /// The parser object can be amended by the calling code afterwards if
    /// needed. This method is optional, all of the settings above can be
    /// modified directly.
    /// @param argParse
    ///    The command line parser object to be updated.
    void init_parser( OIIO::ArgParse &argParse );

    /// Initialise the converter settings from the command line parser object.
    /// Prior to calling this, first initialise the object via
    /// `ImageConverted::init_parser()`, and call
    /// `OIIO::ArgParse::parse_args()`.
    /// This method is optional, all of the settings above can be modified
    /// directly.
    /// @param argParse
    ///    the command line parser object
    /// @result
    ///    `true` if parsed successfully
    bool parse_params( const OIIO::ArgParse &argParse );

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

    /// Apply the colour space conversion matrix (or matrices) to convert the
    /// image buffer from the raw camera colour space to ACES.
    /// @param dst
    ///     Destination image buffer.
    /// @param src
    ///     Source image buffer, can be the same as `dst` for in-place
    ///     conversion.
    /// @result
    ///    `true` if converted successfully.
    bool apply_matrix(
        OIIO::ImageBuf &dst, const OIIO::ImageBuf &src, OIIO::ROI roi = {} );

    /// Apply the headroom scale to image buffer.
    /// @param dst
    ///     Destination image buffer.
    /// @param src
    ///     Source image buffer, can be the same as `dst` for in-place
    ///     conversion.
    /// @result
    ///    `true` if converted successfully.
    bool apply_scale(
        OIIO::ImageBuf &dst, const OIIO::ImageBuf &src, OIIO::ROI roi = {} );

    /// Make output file path and check if it is writable.
    /// @param path
    ///     A reference to a variable containing the input file path. The output file path gets generated
    ///     in-place.
    /// @result
    ///    `true` if the file can be written, e.g. the output directory exists, or creating directories
    ///     is allowed; the file does not exist or overwriting is allowed.
    bool
    make_output_path( std::string &path, const std::string &suffix = "_oiio" );

    /// Saves the image into ACES Container.
    /// @param output_filename
    ///     Full path to the file to be saved.
    /// @param buf
    ///     Image buffer to be saved.
    /// @result
    ///    `true` if saved successfully.
    bool save( const std::string &output_filename, const OIIO::ImageBuf &buf );

private:
    void prepareIDT_DNG( const OIIO::ImageSpec &imageSpec );
    void prepareIDT_nonDNG( const OIIO::ImageSpec &imageSpec );
    void prepareIDT_spectral(
        const OIIO::ImageSpec &imageSpec,
        bool                   calc_white_balance,
        bool                   calc_matrix );
    bool applyMatrix(
        const std::vector<std::vector<double>> &matrix,
        OIIO::ImageBuf                         &dst,
        const OIIO::ImageBuf                   &src,
        OIIO::ROI                               roi );

    bool _is_DNG;

    /// Make libraw read the raw photosites data without any processing.
    /// This is set if the requested demosaicing method is set `None`.
    bool _read_raw = false;

    std::vector<double>              _WB_mults;
    std::vector<std::vector<double>> _IDT_matrix;
    std::vector<std::vector<double>> _CAT_matrix;
};

} // namespace rta

#endif // _RAWTOACES_UTIL_H_
