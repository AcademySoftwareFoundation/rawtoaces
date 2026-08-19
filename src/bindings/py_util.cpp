// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "py_util.h"
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/ndarray.h>
#include <rawtoaces/image_converter.h>
#include "../misc/pragma.h"

using namespace rta::util;
using namespace nanobind::literals;

void util_bindings( nanobind::module_ &m )
{
    m.def(
        "collect_image_files",
        &collect_image_files,
        "path"_a,
        R"""(
        Collect all files from given `paths` into batches.
        For each path that is a directory, a batch is created in the returned 
        vector and filled with the file names. Invalid paths are skipped with
        an error message.
        
        First batch is reserved for all paths that are individual files. If no
        such paths are provided, the first batch will be empty.

        :param path: paths vector of paths to files or directories to process.
        :type path: list[str]
                
        :return: list of batches, where each batch contains files from one input 
            path.
        )""" );

    nanobind::class_<ImageConverter> image_converter( m, "ImageConverter", R"""(
        The ImageConverter class is the main interface for converting RAW images
        to ACES format. It provides a high-level API that handles the entire
        conversion pipeline.
        )""" );

    image_converter.def( nanobind::init<>(), R"""(
        Default constructor.
        )""" );

    image_converter.def_rw( "settings", &ImageConverter::settings, R"""(
        The conversion settings.)""" );
    image_converter.def_ro( "status", &ImageConverter::status, R"""(
        This property holds the error code from the most recent method call 
        that returns a bool.)""" );
    image_converter.def_rw(
        "last_error_message", &ImageConverter::last_error_message, R"""(
        Error message from the most recent method call that returned ``False``.
        )""" );
    image_converter.def(
        "process_image",
        &ImageConverter::process_image,
        "input_filename"_a,
        R"""(
        A convenience single-call method to process an image.
        
        :param input_filename: Full path to the file to be converted.
        :type input_filename: str
        
        :return: ``True`` if processed successfully.
        )""" );
    image_converter.def(
        "get_WB_multipliers", &ImageConverter::get_WB_multipliers, R"""(
        Get the solved white balance multipliers of the currently processed
        image. The multipliers become available after calling the ``configure``
        method.
        
        :return: a list containing 3 multiplier values.
        )""" );
    image_converter.def(
        "get_transform_matrix", &ImageConverter::get_transform_matrix, R"""(
        Get the solved colour transform matrix to be applied by 
        ``process_image``.
        The matrix becomes available after calling the ``configure`` method.
                
        :return: a list containing a 3x3 matrix.
        )""" );

    DISABLE_DEPRECATED_WARNINGS
    image_converter.def(
        "get_IDT_matrix",
        []( ImageConverter &converter ) {
            PyErr_WarnEx(
                PyExc_DeprecationWarning,
                "This method will be removed in v3. "
                "Use `get_transform_matrix()`.",
                1 );
            return converter.get_IDT_matrix();
        },
        R"""(
        Get the solved input transform matrix of the currently processed image.
        The matrix becomes available after calling the ``configure`` method.
        
        :return: a list containing a 3x3 matrix.
        
        .. deprecated:: 2.2.0
            will be removed in v3. Use ``get_transform_matrix()``.
        )""" );

    image_converter.def(
        "get_CAT_matrix",
        []( ImageConverter &converter ) {
            PyErr_WarnEx(
                PyExc_DeprecationWarning,
                "This method will be removed in v3. "
                "Use `get_transform_matrix()`.",
                1 );
            return converter.get_CAT_matrix();
        },
        R"""(
        Get the solved chromatic adaptation transform matrix of the currently
        processed image. The matrix becomes available after calling the 
        ``configure`` method.
        
        :return: a list containing a 3x3 matrix.
        
        .. deprecated:: 2.2.0
            will be removed in v3. Use ``get_transform_matrix()``.
        )""" );
    ENABLE_WARNINGS

    image_converter.def(
        "configure",
        []( ImageConverter &converter, const std::string &input_filename ) {
            OIIO::ParamValueList options;
            return converter.configure( input_filename, options );
        },
        "input_filename"_a,
        R"""(
        Configures the converter using the requested white balance and colour
        matrix method, and the metadata of the file provided in 
        ``input_filename``.
        
        This method loads the metadata from the given image file and
        initialises the options to give the OIIO raw image reader to
        decode the pixels.
        
        :param input_filename: A file name of the raw image file to read the 
            metadata from.
        :type input_filename: str
        :return: ``True`` if configured successfully.
        )""" );
    image_converter.def(
        "get_supported_formats", &ImageConverter::get_supported_formats, R"""(
        Collects all camera raw formats supported by this version.
        
        :return: List of all supported image formats.
        )""" );
    image_converter.def(
        "get_supported_illuminants",
        &ImageConverter::get_supported_illuminants,
        R"""(
        Collects all illuminants supported by this version.
        
        :return: List of all supported illuminants.
        )""" );
    image_converter.def(
        "get_supported_cameras", &ImageConverter::get_supported_cameras, R"""(
        Collects all camera models for which spectral sensitivity data is
        available in the database.
        
        :return: List containing camera model names.
        )""" );

    nanobind::class_<ImageConverter::Settings> settings(
        image_converter, "Settings", R"""(
        The structure containing all parameters needed to configure image
        conversion.
        )""" );

    settings.def( nanobind::init<>() );
    settings.def_rw( "WB_method", &ImageConverter::Settings::WB_method, R"""(
        The selected white-balancing method to use for conversion.
        )""" );
    settings.def_rw(
        "matrix_method",
        &ImageConverter::Settings::matrix_method,
        R"""(
        The selected colour transform matrix calculation method to use for
        conversion.
        )""" );
    settings.def_rw( "crop_mode", &ImageConverter::Settings::crop_mode, R"""(
        The selected cropping mode to use for conversion.
        )""" );
    settings.def_rw( "illuminant", &ImageConverter::Settings::illuminant, R"""(
        An illuminant to use for white balancing and/or colour matrix
        calculation. Only used when ``WB_method`` == ``WBMethod::Illuminant``
        and ``matrix_method`` == ``MatrixMethod::Spectral``.
                
        An illuminant can be provided as a black body correlated colour
        temperature, like ``3200K``; or a D-series illuminant, like ``D56``;
        or any other illuminant, in such case it must be present in the data
        folder.
        )""" );
    settings.def_rw( "headroom", &ImageConverter::Settings::headroom, R"""(
        Highlight headroom factor.
        )""" );
    settings.def_rw(
        "custom_camera_make",
        &ImageConverter::Settings::custom_camera_make,
        R"""(
        Camera manufacturer name to be used for spectral sensitivity
        curves lookup.
        )""" );
    settings.def_rw(
        "custom_camera_model",
        &ImageConverter::Settings::custom_camera_model,
        R"""(
        Camera model name to be used for spectral sensitivity curves lookup.
        )""" );
    settings.def_rw(
        "auto_bright",
        &ImageConverter::Settings::auto_bright,
        R"""(
        Enable automatic exposure adjustment.
        )""" );
    settings.def_rw(
        "adjust_maximum_threshold",
        &ImageConverter::Settings::adjust_maximum_threshold,
        R"""(
        Automatically lower the linearity threshold provided in the metadata by
        this scaling factor.
        )""" );
    settings.def_rw(
        "black_level",
        &ImageConverter::Settings::black_level,
        R"""(
        If >= 0, override the black level specified in the file metadata.
        )""" );
    settings.def_rw(
        "saturation_level", &ImageConverter::Settings::saturation_level, R"""(
        If >= 0, override the saturation level specified in the file metadata.
        )""" );
    settings.def_rw( "half_size", &ImageConverter::Settings::half_size, R"""(
        Decode the image at half size resolution.
        )""" );
    settings.def_rw(
        "highlight_mode", &ImageConverter::Settings::highlight_mode, R"""(
        Highlight recovery mode, as supported by OpenImageIO/Libraw: 
        0 = clip, 1 = unclip, 2 = blend, 3..9 = rebuild.
        )""" );
    settings.def_rw( "flip", &ImageConverter::Settings::flip, R"""(
        If not -1, override the orientation specified in the metadata.
        1..8 correspond to EXIF orientation codes
        (0 = none, 3 = 180 deg, 6 = 90 deg CCW, 8 = 90 deg CW.)
        )""" );
    settings.def_rw(
        "denoise_threshold", &ImageConverter::Settings::denoise_threshold, R"""(
        Wavelet denoising threshold.
        )""" );
    settings.def_rw( "scale", &ImageConverter::Settings::scale, R"""(
        Additional scaling factor to apply to the pixel values.
        )""" );
    settings.def_rw(
        "demosaic_algorithm",
        &ImageConverter::Settings::demosaic_algorithm,
        R"""(
        Demosaicing algorithm. 
        Supported options: ``linear``, ``VNG``, ``PPG``, ``AHD``, ``DCB``,
        ``DHT``, ``AAHD``.
        )""" );
    settings.def_rw(
        "bad_pixels_path", &ImageConverter::Settings::bad_pixels_path, R"""(
        A path to the file containing a list of bad bixels in libraw format:
        a plain text where each line describes a single bad pixel using
        three numbers separated by whitespace for the column, row and UNIX
        timestamp.
        )""" );
    settings.def_rw(
        "database_directories",
        &ImageConverter::Settings::database_directories,
        R"""(
        Directory containing rawtoaces spectral sensitivity and illuminant
        data files. Overrides the default search path and the
        ``RAWTOACES_DATA_PATH`` environment variable.
        )""" );
    settings.def_rw( "overwrite", &ImageConverter::Settings::overwrite, R"""(
        Allows overwriting existing files.
        )""" );
    settings.def_rw(
        "create_dirs",
        &ImageConverter::Settings::create_dirs,
        R"""(
        Create output directories if they don't exist.
        )""" );
    settings.def_rw(
        "compression",
        &ImageConverter::Settings::compression,
        R"""(
        "Output file compression type. Supported options: none, rle, "
            "zip, zips, piz, pxr24, b44, b44a, dwaa, dwab, htj2k256, htj2k32. "
            "(default: none)"
        )""" );
    settings.def_rw( "output_dir", &ImageConverter::Settings::output_dir, R"""(
        The directory to write the output files to.
        )""" );
    settings.def_rw( "use_timing", &ImageConverter::Settings::use_timing, R"""(
        Log the execution time of each step of image processing.
        )""" );
    settings.def_rw(
        "disable_cache",
        &ImageConverter::Settings::disable_cache,
        R"""(
        Disable caching.
        )""" );
    settings.def_rw(
        "disable_exiftool", &ImageConverter::Settings::disable_exiftool, R"""(
        Disable calling exiftool to fetch missing metadata.
        )""" );
    settings.def_rw( "verbosity", &ImageConverter::Settings::verbosity, R"""(
        Verbosity level.
        )""" );

    settings.def_rw(
        "lens_correction_types",
        &ImageConverter::Settings::lens_correction_types,
        R"""(
        The selected lens correction types.
        )""" );
    settings.def_rw(
        "require_lens_correction",
        &ImageConverter::Settings::require_lens_correction,
        R"""(
        If true, treat lens correction as mandatory. The conversion will
        fail if any of the correction types above is requested,
        but rawtoaces is unable to obtain a suitable lens model.
        )""" );
    settings.def_rw(
        "custom_lens_make", &ImageConverter::Settings::custom_lens_make, R"""(
        Lens manufacturer name to be used for lens correction data lookup.
        )""" );
    settings.def_rw(
        "custom_lens_model", &ImageConverter::Settings::custom_lens_model, R"""(
        Lens model name to be used for lens correction data lookup.
        )""" );
    settings.def_rw(
        "custom_aperture", &ImageConverter::Settings::custom_aperture, R"""(
        Aperture (f-number) to be user for lens correction.
        )""" );
    settings.def_rw(
        "custom_focal_length",
        &ImageConverter::Settings::custom_focal_length,
        R"""(
        Focal length to be user for lens correction.
        )""" );
    settings.def_rw(
        "custom_focus_distance",
        &ImageConverter::Settings::custom_focus_distance,
        R"""(
        Focus distance to be user for lens correction.
        )""" );

    settings.def_prop_rw(
        "WB_box",
        []( const ImageConverter::Settings &settings ) {
            std::vector<int> result( 4 );
            for ( size_t i = 0; i < 4; i++ )
                result[i] = settings.WB_box[i];
            return result;
        },
        []( ImageConverter::Settings &settings, const std::vector<int> &box ) {
            if ( box.size() == 4 )
            {
                settings.WB_box[0] = box[0];
                settings.WB_box[1] = box[1];
                settings.WB_box[2] = box[2];
                settings.WB_box[3] = box[3];
            }
            else
            {
                throw std::invalid_argument(
                    "The array must contain 4 elements." );
            }
        },
        R"""(
        Box to use for white balancing when ``WB_method`` == ``WBMethod::Box``.
        (default = (0,0,0,0) - full image)
        )""" );

    settings.def_prop_rw(
        "custom_WB",
        []( const ImageConverter::Settings &settings ) {
            std::vector<float> result( 4 );
            for ( size_t i = 0; i < 4; i++ )
                result[i] = settings.custom_WB[i];
            return result;
        },
        []( ImageConverter::Settings &settings,
            const std::vector<float> &custom_WB ) {
            if ( custom_WB.size() == 4 )
            {
                settings.custom_WB[0] = custom_WB[0];
                settings.custom_WB[1] = custom_WB[1];
                settings.custom_WB[2] = custom_WB[2];
                settings.custom_WB[3] = custom_WB[3];
            }
            else
            {
                throw std::invalid_argument(
                    "The array must contain 4 elements." );
            }
        },
        R"""(
        Custom white balance multipliers to be used when
        ``WB_method`` == ``WBMethod::Custom``.
        )""" );

    settings.def_prop_rw(
        "custom_matrix",
        []( const ImageConverter::Settings &settings ) {
            std::vector<std::vector<float>> result(
                3, std::vector<float>( 3 ) );
            for ( size_t i = 0; i < 3; i++ )
                for ( size_t j = 0; j < 3; j++ )
                    result[i][j] = settings.custom_matrix[i][j];
            return result;
        },
        []( ImageConverter::Settings              &settings,
            const std::vector<std::vector<float>> &matrix ) {
            if ( matrix.size() == 3 )
            {
                for ( size_t i = 0; i < 3; i++ )
                {
                    if ( matrix[i].size() == 3 )
                    {
                        for ( size_t j = 0; j < 3; j++ )
                            settings.custom_matrix[i][j] = matrix[i][j];
                    }
                    else
                    {
                        throw std::invalid_argument(
                            "Each row of the matrix must contain 3 elements." );
                    }
                }
            }
            else
            {
                throw std::invalid_argument(
                    "The matrix must contain 3 rows." );
            }
        },
        R"""(
        Custom camera RGB to XYZ matrix to be used when
        ``matrix_method`` == ``MatrixMethod::Custom``.
        )""" );

    settings.def_prop_rw(
        "crop_box",
        []( const ImageConverter::Settings &settings ) {
            std::vector<int> result( 4 );
            for ( size_t i = 0; i < 4; i++ )
                result[i] = settings.crop_box[i];
            return result;
        },
        []( ImageConverter::Settings &settings, const std::vector<int> &box ) {
            if ( box.size() == 4 )
            {
                settings.crop_box[0] = box[0];
                settings.crop_box[1] = box[1];
                settings.crop_box[2] = box[2];
                settings.crop_box[3] = box[3];
            }
            else
            {
                throw std::invalid_argument(
                    "The array must contain 4 elements." );
            }
        },
        R"""(
        Apply custom crop. If not specified (all values are zeroes),
        the default crop is applied, which should match the crop of the
        in-camera JPEG.
        )""" );

    settings.def_prop_rw(
        "chromatic_aberration",
        []( const ImageConverter::Settings &settings ) {
            std::vector<float> result( 2 );
            for ( size_t i = 0; i < 2; i++ )
                result[i] = settings.chromatic_aberration[i];
            return result;
        },
        []( ImageConverter::Settings &settings,
            const std::vector<float> &value ) {
            if ( value.size() == 2 )
            {
                settings.chromatic_aberration[0] = value[0];
                settings.chromatic_aberration[1] = value[1];
            }
            else
            {
                throw std::invalid_argument(
                    "The array must contain 2 elements." );
            }
        },
        R"""(
        Red and blue scale factors for chromatic aberration correction.
        )""" );

    nanobind::enum_<ImageConverter::Settings::WBMethod>(
        settings, "WBMethod", R"""(
        The enumerator containing all supported white-balancing methods.
        )""" )
        .value( "Metadata", ImageConverter::Settings::WBMethod::Metadata, R"""(
            Use the metadata provided in the image file. This mode is mostly
            usable with DNG files, as the information needed for conversion
            is mandatory in the DNG format.
            )""" )
        .value(
            "Illuminant", ImageConverter::Settings::WBMethod::Illuminant, R"""(
            White balance to a specified illuminant. See the ``illuminant``
            property for more information on the supported illuminants. This
            mode can only be used if spectral sensitivities are available for
            the camera.
            )""" )
        .value( "Box", ImageConverter::Settings::WBMethod::Box, R"""(
            Calculate white balance by averaging over a specified region of
            the image. See ``WB_box``. In this mode if an empty box is provided,
            white balancing is done by averaging over the whole image.
            )""" )
        .value( "Custom", ImageConverter::Settings::WBMethod::Custom, R"""(
            Use custom white balancing multipliers. This mode is useful if
            the white balancing coefficients are calculated by an external tool.
            )""" )
        .export_values();

    nanobind::enum_<ImageConverter::Settings::MatrixMethod>(
        settings, "MatrixMethod", R"""(
            The enumerator containing all supported colour transform matrix 
            calculation methods.
            )""" )
        .value( "Auto", ImageConverter::Settings::MatrixMethod::Auto, R"""(
            Automatically choose the best available matrix method.
            
            - If spectral sensitivity data for the camera is available, use ``Spectral``.
            - Otherwise, fall back to ``Metadata``.
            
            )""" )
        .value(
            "Spectral", ImageConverter::Settings::MatrixMethod::Spectral, R"""(
            Use the camera spectral sensitivity curves to solve for the colour
            conversion matrix. In this mode the illuminant is either provided
            directly in ``illuminant`` if ``WB_method`` == 
            ``WBMethod::Illuminant``, or the best illuminant is derived from the
            white balancing multipliers.
            )""" )
        .value(
            "Metadata", ImageConverter::Settings::MatrixMethod::Metadata, R"""(
            Use the metadata provided in the image file. This mode is mostly
            usable with DNG files, as the information needed for conversion
            is mandatory in the DNG format.
            )""" )
        .value( "Adobe", ImageConverter::Settings::MatrixMethod::Adobe, R"""(
            Use the Adobe colour matrix for the camera supplied in LibRaw.
            )""" )
        .value( "Custom", ImageConverter::Settings::MatrixMethod::Custom, R"""(
            Specify a custom matrix in `colourMatrix`. This mode is useful if
            the matrix is calculated by an external tool.
            )""" )
        .export_values();

    nanobind::enum_<ImageConverter::Settings::CropMode>(
        settings, "CropMode", R"""(
            The enumerator containing all supported cropping modes.
            )""" )
        .value( "Off", ImageConverter::Settings::CropMode::Off, R"""(
            Write out full sensor area.
            )""" )
        .value( "Soft", ImageConverter::Settings::CropMode::Soft, R"""(
            Write out full sensor area, mark the crop area as the display 
            window.
            )""" )
        .value( "Hard", ImageConverter::Settings::CropMode::Hard, R"""(
            Write out only the crop area.
            )""" )
        .export_values();

    nanobind::enum_<ImageConverter::Settings::LensCorrectionType>(
        settings, "LensCorrectionType", nanobind::is_flag(), R"""(
            The enumerator containing all supported lens correction types.
            )""" )
        .value(
            "Aberration",
            ImageConverter::Settings::LensCorrectionType::Aberration,
            R"""(
            Chromatic aberration
            )""" )
        .value(
            "Distortion",
            ImageConverter::Settings::LensCorrectionType::Distortion,
            R"""(
            Geeometric distortion
            )""" )
        .value(
            "Vignetting",
            ImageConverter::Settings::LensCorrectionType::Vignetting,
            R"""(
            Vignetting
            )""" )
        .export_values();

    nanobind::enum_<ImageConverter::Status>(
        image_converter, "Status", "Status codes for operation results." )
        .value( "Success", ImageConverter::Status::Success, R"""(
            "Operation completed successfully.
            )""" )
        .value(
            "DatabaseNotFound",
            ImageConverter::Status::DatabaseNotFound,
            R"""(
            Failed to locate spectral measurements database.
            )""" )
        .value( "FileExists", ImageConverter::Status::FileExists, R"""(
            Output file already exists and overwrite is not enabled.
            )""" )
        .value(
            "InputFileNotFound",
            ImageConverter::Status::InputFileNotFound,
            R"""(
            Input file does not exist.
            )""" )
        .value(
            "EmptyInputFilename",
            ImageConverter::Status::EmptyInputFilename,
            R"""(
            Empty input filename provided.
            )""" )
        .value(
            "FilesystemError",
            ImageConverter::Status::FilesystemError,
            R"""(
            Filesystem error occurred.
            )""" )
        .value(
            "OutputDirectoryError",
            ImageConverter::Status::OutputDirectoryError,
            R"""(
            Output directory does not exist and cannot be created.
            )""" )
        .value( "InvalidPath", ImageConverter::Status::InvalidPath, R"""(
            Invalid path format.
            )""" )
        .value(
            "ConfigurationError",
            ImageConverter::Status::ConfigurationError,
            R"""(
                Failed to configure the image reader.
            )""" )
        .value( "ReadError", ImageConverter::Status::ReadError, R"""(
            Failed to read the image file.
            )""" )
        .value(
            "LensCorrectionError",
            ImageConverter::Status::LensCorrectionError,
            R"""(
            Failed to apply lens correction.
            )""" )
        .value(
            "MatrixApplicationError",
            ImageConverter::Status::MatrixApplicationError,
            R"""(
                Failed to apply colour space conversion.
            )""" )
        .value(
            "ScaleApplicationError",
            ImageConverter::Status::ScaleApplicationError,
            R"""(
            Failed to apply scale.
            )""" )
        .value(
            "CropApplicationError",
            ImageConverter::Status::CropApplicationError,
            R"""(
            Failed to apply crop.
            )""" )
        .value( "WriteError", ImageConverter::Status::WriteError, R"""(
            Failed to save the output file.
            )""" )
        .value( "UnknownError", ImageConverter::Status::UnknownError, R"""(
            Unknown error.
            )""" )
        .export_values();
}
