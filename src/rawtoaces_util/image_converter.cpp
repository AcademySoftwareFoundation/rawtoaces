// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include <rawtoaces/image_converter.h>
#include <rawtoaces/rawtoaces_core.h>
#include <rawtoaces/usage_timer.h>

#include <set>
#include <filesystem>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

namespace rta
{
namespace util
{

struct CameraIdentifier
{
    std::string make;
    std::string model;

    CameraIdentifier() = default;

    bool is_empty() const { return make.empty() && model.empty(); }

    operator std::string() const
    {
        return "make: '" + make + "', model: '" + model + "'";
    }

    friend std::string
    operator+( const std::string &lhs, const CameraIdentifier &rhs )
    {
        return lhs + static_cast<std::string>( rhs );
    }
};

/**
 * Checks if a file path is valid for processing and adds it to a batch list if appropriate.
 * 
 * This function validates that the given path points to a regular file or symbolic link,
 * filters out unwanted files (system files like .DS_Store and certain image formats like EXR and JPG),
 * and adds valid file paths to the provided batch vector for further processing.
 * 
 * @param path The filesystem path to check
 * @param batch Reference to a vector of strings to add valid file paths to
 * @return true if the file was processed (either added to batch or filtered out), 
 *         false if the file should be ignored
 */
void check_and_add_file(
    const std::filesystem::path &path, std::vector<std::string> &batch )
{
    bool is_regular_file = std::filesystem::is_regular_file( path ) ||
                           std::filesystem::is_symlink( path );
    if ( !is_regular_file )
    {
        std::cerr << "Not a regular file: " << path << std::endl;
        return;
    }

    static const std::set<std::string> ignore_filenames = { ".DS_Store" };
    std::string                        filename = path.filename().string();
    if ( ignore_filenames.count( filename ) > 0 )
        return;

    static const std::set<std::string> ignore_extensions = { ".exr",
                                                             ".jpg",
                                                             ".jpeg" };
    std::string extension = OIIO::Strutil::lower( path.extension().string() );
    if ( ignore_extensions.count( extension ) > 0 )
        return;

    batch.push_back( path.string() );
    return;
}

std::vector<std::vector<std::string>>
collect_image_files( const std::vector<std::string> &paths )
{
    std::vector<std::vector<std::string>> batches( 1 );

    for ( const auto &path: paths )
    {
        if ( !std::filesystem::exists( path ) )
        {
            std::cerr << "File or directory not found: " << path << std::endl;
            continue;
        }

        auto canonical_filename = std::filesystem::canonical( path );

        if ( std::filesystem::is_directory( path ) )
        {
            std::vector<std::string> &curr_batch = batches.emplace_back();
            auto it = std::filesystem::directory_iterator( path );

            for ( auto filename: it )
            {
                check_and_add_file( filename, curr_batch );
            }
        }
        else
        {
            check_and_add_file( path, batches[0] );
        }
    }

    return batches;
}

/// Gets the list of database paths for rawtoaces data files.
///
/// Checks environment variables (RAWTOACES_DATA_PATH or deprecated AMPAS_DATA_PATH)
/// and falls back to platform-specific default paths.
///
/// @return Vector of unique database directory paths
std::vector<std::string> database_paths()
{
    std::vector<std::string> result;

#if defined( WIN32 ) || defined( WIN64 )
    const std::string separator    = ";";
    const std::string default_path = ".";
#else
    const std::string separator   = ":";
    const std::string legacy_path = "/usr/local/include/rawtoaces/data";
    const std::string default_path =
        "/usr/local/share/rawtoaces/data" + separator + legacy_path;
#endif

    std::string path;

    const char *path_from_env = getenv( "RAWTOACES_DATA_PATH" );
    if ( !path_from_env )
    {
        // Fallback to the old environment variable.
        path_from_env = getenv( "AMPAS_DATA_PATH" );

        if ( path_from_env )
        {
            std::cerr << "Warning: The environment variable "
                      << "AMPAS_DATA_PATH is now deprecated. Please use "
                      << "RAWTOACES_DATA_PATH instead." << std::endl;
        }
    }

    if ( path_from_env )
    {
        path = path_from_env;
    }
    else
    {
        path = default_path;
    }

    OIIO::Strutil::split( path, result, separator );

    return result;
}

/// Get camera info (with make and model) from image metadata or custom settings.
///
/// Returns camera information using custom settings if provided, otherwise
/// extracts from image metadata. Returns empty CameraInfo if required metadata is missing.
///
/// @param spec Image specification containing metadata
/// @param settings Converter settings with optional custom camera info
/// @return CameraInfo struct with make and model, or empty if unavailable
CameraIdentifier get_camera_identifier(
    const OIIO::ImageSpec &spec, const ImageConverter::Settings &settings )
{
    std::string camera_make  = settings.custom_camera_make;
    std::string camera_model = settings.custom_camera_model;

    if ( camera_make.empty() )
    {
        camera_make = spec["cameraMake"];
        if ( camera_make.empty() )
        {
            std::cerr << "Missing the camera manufacturer name in the file "
                      << "metadata. You can provide a camera make using the "
                      << "--custom-camera-make parameter" << std::endl;
            return CameraIdentifier();
        }
    }

    if ( camera_model.empty() )
    {
        camera_model = spec["cameraModel"];
        if ( camera_model.empty() )
        {
            std::cerr << "Missing the camera model name in the file metadata. "
                      << "You can provide a camera model using the "
                      << "--custom-camera-model parameter" << std::endl;
            return CameraIdentifier();
        }
    }

    return { camera_make, camera_model };
}

void print_data_error( const std::string &data_type )
{
    std::cerr << "Failed to find " << data_type << "." << std::endl
              << "Please check the database search path "
              << "in RAWTOACES_DATABASE_PATH" << std::endl;
}

/// Prepares spectral transformation matrices for RAW to ACES conversion
///
/// This method initializes a spectral solver to find the appropriate camera data,
/// loads training and observer spectral data, determines the illuminant (either from
/// settings or by analyzing white balance multipliers), calculates white balance
/// coefficients, and computes the IDT matrix. The CAT (Chromatic Adaptation Transform) matrix is not used in spectral
/// mode as chromatic adaptation is embedded within the IDT (Input Device Transform) matrix.
///
/// @param image_spec OpenImageIO image specification containing metadata
/// @param settings ImageConverter settings including illuminant and verbosity
/// @param WB_multipliers Output white balance multipliers (3-element vector)
/// @param IDT_matrix Output Input Device Transform matrix (3x3 matrix)
/// @param CAT_matrix Output Chromatic Adaptation Transform matrix (cleared in spectral mode)
/// @return true if transformation matrices were successfully prepared, false otherwise
bool prepare_transform_spectral(
    const OIIO::ImageSpec            &image_spec,
    const ImageConverter::Settings   &settings,
    std::vector<double>              &WB_multipliers,
    std::vector<std::vector<double>> &IDT_matrix,
    std::vector<std::vector<double>> &CAT_matrix )
{
    // Step 1: Initialize and validate camera identification
    std::string lower_illuminant = OIIO::Strutil::lower( settings.illuminant );

    CameraIdentifier camera_identifier =
        get_camera_identifier( image_spec, settings );
    if ( camera_identifier.is_empty() )
        return false;

    bool success = false;

    // Step 2: Initialize spectral solver and find camera data
    core::SpectralSolver solver( settings.database_directories );
    solver.verbosity = settings.verbosity;

    success =
        solver.find_camera( camera_identifier.make, camera_identifier.model );
    if ( !success )
    {
        const std::string data_type =
            "spectral data for camera " + camera_identifier;
        print_data_error( data_type );
        return false;
    }

    // Step 3: Load training spectral data
    const std::string training_path = "training/training_spectral.json";
    success = solver.load_spectral_data( training_path, solver.training_data );
    if ( !success )
    {
        const std::string data_type = "training data '" + training_path + "'.";
        print_data_error( data_type );
        return false;
    }

    // Step 4: Load observer (CMF) spectral data
    const std::string observer_path = "cmf/cmf_1931.json";
    success = solver.load_spectral_data( observer_path, solver.observer );
    if ( !success )
    {
        const std::string data_type = "observer '" + observer_path + "'";
        print_data_error( data_type );
        return false;
    }

    // Step 5: Determine illuminant and calculate white balance
    if ( !lower_illuminant.empty() )
    {
        // Use specified illuminant from settings
        success = solver.find_illuminant( lower_illuminant );

        if ( !success )
        {
            const std::string data_type =
                "illuminant type = '" + lower_illuminant + "'";
            print_data_error( data_type );
            return false;
        }
    }

    if ( lower_illuminant.empty() )
    {
        // Auto-detect illuminant from white balance multipliers
        std::vector<double> tmp_wb_multipliers( 4 );

        if ( WB_multipliers.size() == 4 )
        {
            for ( int i = 0; i < 3; i++ )
                tmp_wb_multipliers[i] = WB_multipliers[i];
        }
        else
        {
            // Extract white balance from RAW metadata
            auto attr = image_spec.find_attribute(
                "raw:pre_mul", OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 4 ) );
            if ( attr )
            {
                for ( int i = 0; i < 4; i++ )
                    tmp_wb_multipliers[i] = attr->get_float_indexed( i );
            }
        }

        // Average green channels if 4-channel data
        if ( tmp_wb_multipliers[3] != 0 )
            tmp_wb_multipliers[1] =
                ( tmp_wb_multipliers[1] + tmp_wb_multipliers[3] ) / 2.0;
        tmp_wb_multipliers.resize( 3 );

        // Normalize white balance multipliers
        double min_val = *std::min_element(
            tmp_wb_multipliers.begin(), tmp_wb_multipliers.end() );

        if ( min_val > 0 && min_val != 1 )
            for ( int i = 0; i < 3; i++ )
                tmp_wb_multipliers[i] /= min_val;

        success = solver.find_illuminant( tmp_wb_multipliers );

        if ( !success )
        {
            std::cerr << "ERROR: Failed to find a suitable illuminant."
                      << std::endl;
            return false;
        }

        if ( settings.verbosity > 0 )
        {
            std::cerr << "Found illuminant: '" << solver.illuminant.illuminant
                      << "'." << std::endl;
        }
    }
    else
    {
        // Calculate white balance for specified illuminant
        success = solver.calculate_WB();

        if ( !success )
        {
            std::cerr << "ERROR: Failed to calculate the white balancing "
                      << "weights." << std::endl;
            return false;
        }

        WB_multipliers = solver.get_WB_multipliers();

        if ( settings.verbosity > 0 )
        {
            std::cerr << "White balance coefficients:" << std::endl;
            for ( auto &wb_multiplier: WB_multipliers )
            {
                std::cerr << wb_multiplier << " ";
            }
            std::cerr << std::endl;
        }
    }

    // Step 6: Calculate Input Device Transform (IDT) matrix
    success = solver.calculate_IDT_matrix();
    if ( !success )
    {
        std::cerr << "Failed to calculate the input transform matrix."
                  << std::endl;
        return false;
    }

    IDT_matrix = solver.get_IDT_matrix();

    if ( settings.verbosity > 0 )
    {
        std::cerr << "Input transform matrix:" << std::endl;
        for ( auto &row: IDT_matrix )
        {
            for ( auto &col: row )
            {
                std::cerr << col << " ";
            }
            std::cerr << std::endl;
        }
    }

    // Step 7: Clear CAT matrix (not used in spectral mode)
    // CAT is embedded in IDT in spectral mode
    CAT_matrix.resize( 0 );

    return true;
}

/// Prepares DNG transformation matrices for RAW to ACES conversion
///
/// This method extracts DNG metadata including baseline exposure, neutral RGB values,
/// and calibration matrices for two illuminants, then uses a MetadataSolver to calculate
/// the Input Device Transform (IDT) matrix. The Chromatic Adaptation Transform (CAT)
/// matrix is not applied for DNG files as chromatic adaptation is handled differently.
///
/// @param image_spec OpenImageIO image specification containing DNG metadata
/// @param settings ImageConverter settings including verbosity level
/// @param IDT_matrix Output Input Device Transform matrix (3x3 matrix)
/// @param CAT_matrix Output Chromatic Adaptation Transform matrix (cleared for DNG)
/// @return true if transformation matrices were successfully prepared, false otherwise
bool prepare_transform_DNG(
    const OIIO::ImageSpec            &image_spec,
    const ImageConverter::Settings   &settings,
    std::vector<std::vector<double>> &IDT_matrix,
    std::vector<std::vector<double>> &CAT_matrix )
{
    // Step 1: Extract basic DNG metadata
    core::Metadata metadata;

    metadata.baseline_exposure =
        image_spec.get_float_attribute( "raw:dng:baseline_exposure" );

    // Step 2: Extract neutral RGB values from camera multipliers
    metadata.neutral_RGB.resize( 3 );

    auto attr = image_spec.find_attribute(
        "raw:cam_mul", OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 4 ) );
    if ( attr )
    {
        for ( int i = 0; i < 3; i++ )
            metadata.neutral_RGB[i] = 1.0 / attr->get_float_indexed( i );
    }

    // Step 3: Extract calibration data for two illuminants
    for ( size_t k = 0; k < 2; k++ )
    {
        auto &calibration = metadata.calibration[k];
        calibration.XYZ_to_RGB_matrix.resize( 9 );
        calibration.camera_calibration_matrix.resize( 9 );

        auto index_string = std::to_string( k + 1 );

        // Extract illuminant type for this calibration
        auto key = "raw:dng:calibration_illuminant" + index_string;
        metadata.calibration[k].illuminant =
            static_cast<unsigned short>( image_spec.get_int_attribute( key ) );

        // Extract XYZ to RGB color matrix
        auto key1         = "raw:dng:color_matrix" + index_string;
        auto matrix1_attr = image_spec.find_attribute(
            key1, OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 12 ) );
        if ( matrix1_attr )
        {
            for ( int i = 0; i < 3; i++ )
            {
                for ( int j = 0; j < 3; j++ )
                {
                    calibration.XYZ_to_RGB_matrix[i * 3 + j] =
                        matrix1_attr->get_float_indexed( i * 3 + j );
                }
            }
        }

        // Extract camera calibration matrix
        auto key2         = "raw:dng:camera_calibration" + index_string;
        auto matrix2_attr = image_spec.find_attribute(
            key2, OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 16 ) );
        if ( matrix2_attr )
        {
            for ( int i = 0; i < 3; i++ )
            {
                for ( int j = 0; j < 3; j++ )
                {
                    calibration.camera_calibration_matrix[i * 3 + j] =
                        matrix2_attr->get_float_indexed( i * 4 + j );
                }
            }
        }
    }

    // Step 4: Calculate IDT matrix using metadata solver
    core::MetadataSolver solver( metadata );
    IDT_matrix = solver.calculate_IDT_matrix();

    if ( settings.verbosity > 0 )
    {
        std::cerr << "Input transform matrix:" << std::endl;
        for ( auto &IDT_matrix_row: IDT_matrix )
        {
            for ( auto &IDT_matrix_row_element: IDT_matrix_row )
            {
                std::cerr << IDT_matrix_row_element << " ";
            }
            std::cerr << std::endl;
        }
    }

    // Step 5: Clear CAT matrix (not used for DNG)
    // Do not apply CAT for DNG
    CAT_matrix.resize( 0 );
    return true;
}

void prepare_transform_nonDNG(
    std::vector<std::vector<double>> &IDT_matrix,
    std::vector<std::vector<double>> &CAT_matrix )
{
    // Do not apply IDT for non-DNG
    IDT_matrix.resize( 0 );

    CAT_matrix = rta::core::CAT_D65_to_ACES;
}

const char *HelpString =
    R"(Rawtoaces converts raw image files from a digital camera to 
the Academy Colour Encoding System (ACES) compliant images.
The process consists of two parts:
- the colour values get converted from the camera native colour 
space to the ACES AP0 (see "SMPTE ST 2065-1"), and 
- the image file gets converted from the camera native raw 
file format to the ACES Image Container file format 
(see "SMPTE ST 2065-4").

Rawtoaces supports the following white-balancing modes:
- "metadata" uses the white-balancing coefficients from the raw 
image file, provided by the camera.
- "illuminant" performs white balancing to the illuminant, 
provided in the "--illuminant" parameter. The list of the 
supported illuminants can be seen using the 
"--list-illuminants" parameter. This mode requires spectral 
sensitivity data for the camera model the image comes from. 
The list of cameras such data is available for, can be 
seen using the "--list-cameras" parameter. In addition to the named 
illuminants, which are stored under ${RAWTOACES_DATA_PATH}/illuminant, 
blackbody illuminants of a given colour temperature can me used (use 'K' 
suffix, i.e. '3200K'), as well as daylight illuminants (use the 'D' 
prefix, i.e. 'D65').
- "box" performs white-balancing to make the given region of 
the image appear neutral gray. The box position (origin and size) 
can be specified using the "--wb-box" parameter. In case no such 
parameter provided, the whole image is used for white-balancing.
- "custom" uses the custom white balancing coefficients 
provided using the -"custom-wb" parameter.

Rawtoaces supports the following methods of color matrix 
computation:
- "spectral" uses the camera sensor's spectral sensitivity data 
to compute the optimal matrix. This mode requires spectral 
sensitivity data for the camera model the image comes from. 
The list of cameras such data is available for, can be 
seen using the "--list-cameras" parameter.
- "metadata" uses the matrix (matrices) contained in the raw 
image file metadata. This mode works best with the images using 
the DNG format, as the DNG standard mandates the presense of 
such matrices.
- "Adobe" uses the Adobe coefficients provided by LibRaw. 
- "custom" uses a user-provided color conversion matrix. 
A matrix can be specified using the "--custom-mat" parameter.

The paths rawtoaces uses to search for the spectral sensitivity 
data can be specified in the RAWTOACES_DATA_PATH environment 
variable.
)";

const char *UsageString = R"(
    rawtoaces --wb-method METHOD --mat-method METHOD [PARAMS] path/to/dir/or/file ...
Examples: 
    rawtoaces --wb-method metadata --mat-method metadata raw_file.dng
    rawtoaces --wb-method illuminant --illuminant 3200K --mat-method spectral raw_file.cr3
)";

/// Validates command-line parameter consistency with selected processing mode
///
/// This template function ensures that command-line parameters are properly configured
/// for the selected processing mode. It validates parameter count, provides appropriate
/// warnings for missing or incorrect parameters, and executes callback functions based
/// on validation results. The function handles two main scenarios: when the parameter
/// is required for the current mode (is_correct_mode=true) and when it should not be
/// provided for the current mode (is_correct_mode=false).
///
/// @param mode_name Name of the mode being checked (e.g., "wb-method", "mat-method")
/// @param mode_value Value of the mode (e.g., "illuminant", "spectral", "metadata")
/// @param param_name Name of the parameter being validated (e.g., "illuminant", "custom-wb")
/// @param param_value Vector of parameter values to validate
/// @param correct_size Expected number of values for the parameter
/// @param default_value_message Message explaining default behavior when parameter is missing
/// @param is_correct_mode Whether the current mode matches the expected mode for this parameter
/// @param on_success Callback function to execute on successful validation
/// @param on_failure Callback function to execute on validation failure
/// @return true if parameter is valid for the current mode, false otherwise
template <typename T, typename F1, typename F2>
bool check_param(
    const std::string    &mode_name,
    const std::string    &mode_value,
    const std::string    &param_name,
    const std::vector<T> &param_value,
    size_t                correct_size,
    const std::string    &default_value_message,
    bool                  is_correct_mode,
    F1                    on_success,
    F2                    on_failure )
{
    if ( is_correct_mode )
    {
        if ( param_value.size() == correct_size )
        {
            on_success();
            return true;
        }
        else
        {
            bool param_not_provided =
                ( param_value.size() == 0 ) ||
                ( ( param_value.size() == 1 ) && ( param_value[0] == 0 ) );
            if ( param_not_provided )
            {
                std::cerr << "Warning: " << mode_name << " was set to \""
                          << mode_value << "\", but no \"--" << param_name
                          << "\" parameter provided. " << default_value_message
                          << std::endl;

                on_failure();
                return false;
            }

            std::cerr << "Warning: The parameter \"" << param_name
                      << "\" must have " << correct_size << " values. "
                      << default_value_message << std::endl;

            on_failure();
            return false;
        }
    }
    else
    {
        bool param_provided =
            ( param_value.size() > 1 ) ||
            ( ( param_value.size() == 1 ) && ( param_value[0] != 0 ) );
        if ( param_provided )
        {
            std::cerr << "Warning: the \"--" << param_name
                      << "\" parameter provided, but the " << mode_name
                      << " is different from \"" << mode_value << "\". "
                      << default_value_message << std::endl;

            on_failure();
            return false;
        }
        else
        {
            // Incorrect mode and no parameter. We don't want any mode-specific parameters parsed when we are in wrong mode.
            return true;
        }
    }
}

void ImageConverter::init_parser( OIIO::ArgParse &arg_parser )
{
    arg_parser.intro( HelpString );
    arg_parser.usage( UsageString );
    arg_parser.print_defaults( true );
    arg_parser.add_help( true );

#if OIIO_VERSION >= OIIO_MAKE_VERSION( 2, 4, 0 )
    arg_parser.add_version( RAWTOACES_VERSION );
#endif

    arg_parser.arg( "--wb-method" )
        .help(
            "White balance method. Supported options: metadata, illuminant, "
            "box, custom." )
        .metavar( "STR" )
        .defaultval( "metadata" )
        .action( OIIO::ArgParse::store() );

    arg_parser.arg( "--mat-method" )
        .help(
            "IDT matrix calculation method. Supported options: spectral, "
            "metadata, Adobe, custom." )
        .metavar( "STR" )
        .defaultval( "spectral" )
        .action( OIIO::ArgParse::store() );

    arg_parser.arg( "--illuminant" )
        .help( "Illuminant for white balancing. (default = D55)" )
        .metavar( "STR" )
        .action( OIIO::ArgParse::store() );

    arg_parser.arg( "--wb-box" )
        .help(
            "Box to use for white balancing. (default = (0,0,0,0) - full "
            "image)" )
        .nargs( 4 )
        .metavar( "X Y W H" )
        .action( OIIO::ArgParse::store<int>() );

    arg_parser.arg( "--custom-wb" )
        .help( "Custom white balance multipliers." )
        .nargs( 4 )
        .metavar( "R G B G" )
        .action( OIIO::ArgParse::store<float>() );

    arg_parser.arg( "--custom-mat" )
        .help( "Custom camera RGB to XYZ matrix." )
        .nargs( 9 )
        .metavar( "Rr Rg Rb Gr Gg Gb Br Bg Bb" )
        .action( OIIO::ArgParse::store<float>() );

    arg_parser.arg( "--custom-camera-make" )
        .help(
            "Camera manufacturer name to be used for spectral sensitivity "
            "curves lookup. "
            "If present, overrides the value stored in the file metadata." )
        .metavar( "STR" )
        .action( OIIO::ArgParse::store() );

    arg_parser.arg( "--custom-camera-model" )
        .help(
            "Camera model name to be used for spectral sensitivity "
            "curves lookup. "
            "If present, overrides the value stored in the file metadata." )
        .metavar( "STR" )
        .action( OIIO::ArgParse::store() );

    arg_parser.arg( "--headroom" )
        .help( "Highlight headroom factor." )
        .metavar( "VAL" )
        .defaultval( 6.0f )
        .action( OIIO::ArgParse::store<float>() );

    arg_parser.arg( "--scale" )
        .help( "Additional scaling factor to apply to the pixel values." )
        .metavar( "VAL" )
        .defaultval( 1.0f )
        .action( OIIO::ArgParse::store<float>() );

    arg_parser.separator( "General options:" );

    arg_parser.arg( "--overwrite" )
        .help(
            "Allows overwriting existing files. If not set, trying to write "
            "to an existing file will generate an error." )
        .action( OIIO::ArgParse::store_true() );

    arg_parser.arg( "--output-dir" )
        .help(
            "The directory to write the output files to. "
            "This gets applied to every input directory, so it is better to "
            "be used with a single input directory." )
        .metavar( "STR" )
        .action( OIIO::ArgParse::store() );

    arg_parser.arg( "--create-dirs" )
        .help( "Create output directories if they don't exist." )
        .action( OIIO::ArgParse::store_true() );

    arg_parser.separator( "Raw conversion options:" );

    arg_parser.arg( "--auto-bright" )
        .help( "Enable automatic exposure adjustment." )
        .action( OIIO::ArgParse::store_true() );

    arg_parser.arg( "--adjust-maximum-threshold" )
        .help(
            "Automatically lower the linearity threshold provided in the "
            "metadata by this scaling factor." )
        .metavar( "VAL" )
        .defaultval( 0.75f )
        .action( OIIO::ArgParse::store<float>() );

    arg_parser.arg( "--black-level" )
        .help( "If >= 0, override the black level." )
        .metavar( "VAL" )
        .defaultval( -1 )
        .action( OIIO::ArgParse::store<int>() );

    arg_parser.arg( "--saturation-level" )
        .help(
            "If not 0, override the level which appears to be saturated "
            "after normalisation." )
        .metavar( "VAL" )
        .defaultval( 0 )
        .action( OIIO::ArgParse::store<int>() );

    arg_parser.arg( "--chromatic-aberration" )
        .help(
            "Red and blue scale factors for chromatic aberration correction. "
            "The value of 1 means no correction." )
        .metavar( "R B" )
        .nargs( 2 )
        .defaultval( 1.0f )
        .action( OIIO::ArgParse::store<float>() );

    arg_parser.arg( "--half-size" )
        .help( "If present, decode image at half size resolution." )
        .action( OIIO::ArgParse::store_true() );

    arg_parser.arg( "--highlight-mode" )
        .help( "0 = clip, 1 = unclip, 2 = blend, 3..9 = rebuild." )
        .metavar( "VAL" )
        .defaultval( 0 )
        .action( OIIO::ArgParse::store<int>() );

    arg_parser.arg( "--crop-box" )
        .help(
            "Apply custom crop. If not present, the default crop is applied, "
            "which should match the crop of the in-camera JPEG." )
        .nargs( 4 )
        .metavar( "X Y W H" )
        .action( OIIO::ArgParse::store<int>() );

    arg_parser.arg( "--crop-mode" )
        .help(
            "Cropping mode. Supported options: 'none' (write out the full "
            "sensor area), 'soft' (write out full image, mark the crop as the "
            "display window), 'hard' (write out only the crop area)." )
        .metavar( "STR" )
        .defaultval( "soft" )
        .action( OIIO::ArgParse::store() );

    arg_parser.arg( "--flip" )
        .help(
            "If not 0, override the orientation specified in the metadata. "
            "1..8 correspond to EXIF orientation codes "
            "(3 = 180 deg, 6 = 90 deg CCW, 8 = 90 deg CW.)" )
        .metavar( "VAL" )
        .defaultval( 0 )
        .action( OIIO::ArgParse::store<int>() );

    arg_parser.arg( "--denoise-threshold" )
        .help( "Wavelet denoising threshold" )
        .metavar( "VAL" )
        .defaultval( 0 )
        .action( OIIO::ArgParse::store<int>() );

    arg_parser.arg( "--demosaic" )
        .help(
            "Demosaicing algorithm. Supported options: 'linear', 'VNG', 'PPG', "
            "'AHD', 'DCB', 'AHD-Mod', 'AFD', 'VCD', 'Mixed', 'LMMSE', 'AMaZE', "
            "'DHT', 'AAHD', 'AHD'." )
        .metavar( "STR" )
        .defaultval( "AHD" )
        .action( OIIO::ArgParse::store() );

    arg_parser.separator( "Benchmarking and debugging:" );

    arg_parser.arg( "--list-cameras" )
        .help( "Shows the list of cameras supported in spectral mode." )
        .action( OIIO::ArgParse::store_true() );

    arg_parser.arg( "--list-illuminants" )
        .help( "Shows the list of illuminants supported in spectral mode." )
        .action( OIIO::ArgParse::store_true() );

    arg_parser.arg( "--use-timing" )
        .help( "Log the execution time of each step of image processing." )
        .action( OIIO::ArgParse::store_true() );

    arg_parser.arg( "--verbose" )
        .help(
            "(-v) Print progress messages. "
            "Repeated -v will increase verbosity." )
        .action( [&]( OIIO::cspan<const char *> /* argv */ ) {
            settings.verbosity++;
        } );

    arg_parser.arg( "-v" ).hidden().action(
        [&]( OIIO::cspan<const char *> /* argv */ ) { settings.verbosity++; } );
}

bool ImageConverter::parse_parameters( const OIIO::ArgParse &arg_parser )
{
    settings.database_directories = database_paths();

    if ( arg_parser["list-cameras"].get<int>() )
    {
        auto cameras = supported_cameras();
        std::cout
            << std::endl
            << "Spectral sensitivity data is available for the following cameras:"
            << std::endl
            << OIIO::Strutil::join( cameras, "\n" ) << std::endl;
        exit( 0 );
    }

    if ( arg_parser["list-illuminants"].get<int>() )
    {
        auto illuminants = supported_illuminants();
        std::cout << std::endl
                  << "The following illuminants are supported:" << std::endl
                  << OIIO::Strutil::join( illuminants, "\n" ) << std::endl;
        exit( 0 );
    }

    std::string WB_method = arg_parser["wb-method"].get();

    if ( WB_method == "metadata" )
    {
        settings.WB_method = Settings::WBMethod::Metadata;
    }
    else if ( WB_method == "illuminant" )
    {
        settings.WB_method = Settings::WBMethod::Illuminant;
    }
    else if ( WB_method == "box" )
    {
        settings.WB_method = Settings::WBMethod::Box;
    }
    else if ( WB_method == "custom" )
    {
        settings.WB_method = Settings::WBMethod::Custom;
    }
    else
    {
        std::cerr
            << std::endl
            << "Unsupported white balancing method: '" << WB_method << "'. "
            << "The following methods are supported: metadata, illuminant, box, custom."
            << std::endl;

        return false;
    }

    std::string matrix_method = arg_parser["mat-method"].get();

    if ( matrix_method == "spectral" )
    {
        settings.matrix_method = Settings::MatrixMethod::Spectral;
    }
    else if ( matrix_method == "metadata" )
    {
        settings.matrix_method = Settings::MatrixMethod::Metadata;
    }
    else if ( matrix_method == "Adobe" )
    {
        settings.matrix_method = Settings::MatrixMethod::Adobe;
    }
    else if ( matrix_method == "custom" )
    {
        settings.matrix_method = Settings::MatrixMethod::Custom;
    }
    else
    {
        std::cerr
            << std::endl
            << "Unsupported matrix method: '" << matrix_method << "'. "
            << "The following methods are supported: spectral, metadata, Adobe, custom."
            << std::endl;

        return false;
    }

    settings.illuminant        = arg_parser["illuminant"].get();
    bool is_illuminant_defined = !settings.illuminant.empty();
    bool is_WB_method_illuminant =
        settings.WB_method == Settings::WBMethod::Illuminant;

    if ( is_WB_method_illuminant && !is_illuminant_defined )
    {
        std::cerr << "Warning: the white balancing method was set to "
                  << "\"illuminant\", but no \"--illuminant\" parameter "
                  << "provided. D55 will be used as default." << std::endl;

        std::string default_illuminant = "D55";
        settings.illuminant            = default_illuminant;
    }
    else if ( !is_WB_method_illuminant && is_illuminant_defined )
    {
        std::cerr << "Warning: the \"--illuminant\" parameter provided "
                  << "but the white balancing mode different from "
                  << "\"illuminant\" "
                  << "requested. The custom illuminant will be ignored."
                  << std::endl;
    }

    auto WB_box = arg_parser["wb-box"].as_vec<int>();
    check_param(
        "white balancing mode",
        "box",
        "wb-box",
        WB_box,
        4,
        "The box will be ignored.",
        settings.WB_method == Settings::WBMethod::Box,
        [&]() {
            for ( int i = 0; i < 4; i++ )
                settings.WB_box[i] = WB_box[i];
        },
        [&]() {
            for ( int i = 0; i < 4; i++ )
                settings.WB_box[i] = 0;
        } );

    auto custom_WB = arg_parser["custom-wb"].as_vec<float>();
    check_param(
        "white balancing mode",
        "custom",
        "custom-wb",
        custom_WB,
        4,
        "The scalers will be ignored. The default values of (1, 1, 1, 1) will be used",
        settings.WB_method == Settings::WBMethod::Custom,
        [&]() {
            for ( int i = 0; i < 4; i++ )
                settings.custom_WB[i] = custom_WB[i];
        },
        [&]() {
            for ( int i = 0; i < 4; i++ )
                settings.custom_WB[i] = 1.0;
        } );

    auto custom_matrix = arg_parser["custom-mat"].as_vec<float>();
    check_param(
        "matrix mode",
        "custom",
        "custom-mat",
        custom_matrix,
        9,
        "Identity matrix will be used",
        settings.matrix_method == Settings::MatrixMethod::Custom,
        [&]() {
            for ( int i = 0; i < 3; i++ )
                for ( int j = 0; j < 3; j++ )
                    settings.custom_matrix[i][j] = custom_matrix[i * 3 + j];
        },
        [&]() {
            for ( int i = 0; i < 3; i++ )
                for ( int j = 0; j < 3; j++ )
                    settings.custom_matrix[i][j] = i == j ? 1.0f : 0.0f;
        } );

    auto crop_box = arg_parser["crop-box"].as_vec<int>();
    if ( crop_box.size() == 4 )
    {
        for ( size_t i = 0; i < 4; i++ )
            settings.crop_box[i] = crop_box[i];
    }

    std::string crop_mode = arg_parser["crop-mode"].get();

    if ( crop_mode == "off" )
    {
        settings.crop_mode = Settings::CropMode::Off;
    }
    else if ( crop_mode == "soft" )
    {
        settings.crop_mode = Settings::CropMode::Soft;
    }
    else if ( crop_mode == "hard" )
    {
        settings.crop_mode = Settings::CropMode::Hard;
    }
    else
    {
        std::cerr << std::endl
                  << "Unsupported cropping mode: '" << crop_mode << "'. "
                  << "The following modes are supported: off, soft, hard."
                  << std::endl;

        return false;
    }

    auto chromatic_aberration =
        arg_parser["chromatic-aberration"].as_vec<int>();
    if ( chromatic_aberration.size() == 2 )
    {
        for ( size_t i = 0; i < 2; i++ )
            settings.chromatic_aberration[i] =
                static_cast<float>( chromatic_aberration[i] );
    }

    auto demosaic_algorithm = arg_parser["demosaic"].get();
    static std::set<std::string> demosaic_algorithms = {
        "linear", "VNG",   "PPG",   "AHD",   "DCB", "AHD-Mod", "AFD",
        "VCD",    "Mixed", "LMMSE", "AMaZE", "DHT", "AAHD",    "AHD"
    };

    if ( demosaic_algorithms.count( demosaic_algorithm ) != 1 )
    {
        std::cerr << std::endl
                  << "Unsupported demosaicing algorithm: '"
                  << demosaic_algorithm << "'. "
                  << "The following algorithms are supported: "
                  << OIIO::Strutil::join( demosaic_algorithms, ", " ) << "."
                  << std::endl;
        return false;
    }
    else
    {
        settings.demosaic_algorithm = demosaic_algorithm;
    }

    settings.custom_camera_make  = arg_parser["custom-camera-make"].get();
    settings.custom_camera_model = arg_parser["custom-camera-model"].get();

    settings.headroom    = arg_parser["headroom"].get<float>();
    settings.auto_bright = arg_parser["auto-bright"].get<int>();
    settings.adjust_maximum_threshold =
        arg_parser["adjust-maximum-threshold"].get<float>();
    settings.black_level      = arg_parser["black-level"].get<int>();
    settings.saturation_level = arg_parser["saturation-level"].get<int>();
    settings.half_size        = arg_parser["half-size"].get<int>();
    settings.highlight_mode   = arg_parser["highlight-mode"].get<int>();
    settings.flip             = arg_parser["flip"].get<int>();

    settings.scale             = arg_parser["scale"].get<float>();
    settings.denoise_threshold = arg_parser["denoise-threshold"].get<float>();

    settings.overwrite   = arg_parser["overwrite"].get<int>();
    settings.create_dirs = arg_parser["create-dirs"].get<int>();
    settings.output_dir  = arg_parser["output-dir"].get();
    settings.use_timing  = arg_parser["use-timing"].get<int>();

    // If an illuminant was requested, confirm that we have it in the database
    // an error out early, before we start loading any images.
    if ( settings.WB_method == Settings::WBMethod::Illuminant )
    {
        core::SpectralSolver solver( settings.database_directories );
        if ( !solver.find_illuminant( settings.illuminant ) )
        {
            std::cerr << std::endl
                      << "Error: No matching light source. "
                      << "Please find available options by "
                      << "\"rawtoaces --valid-illum\"." << std::endl;
            exit( -1 );
        }
    }

    return true;
}

std::vector<std::string> ImageConverter::supported_illuminants()
{
    std::vector<std::string> result;

    result.push_back( "Day-light (e.g., D60, D6025)" );
    result.push_back( "Blackbody (e.g., 3200K)" );

    rta::core::SpectralSolver solver( settings.database_directories );
    auto                      files = solver.collect_data_files( "illuminant" );
    for ( auto &file: files )
    {
        core::SpectralData data;
        if ( data.load( file, false ) )
        {
            result.push_back( data.illuminant );
        }
    }

    return result;
}

std::vector<std::string> ImageConverter::supported_cameras()
{
    std::vector<std::string> result;

    rta::core::SpectralSolver solver( settings.database_directories );
    auto                      files = solver.collect_data_files( "camera" );
    for ( auto &file: files )
    {
        core::SpectralData data;
        if ( data.load( file, false ) )
        {
            std::string name = data.manufacturer + " / " + data.model;
            result.push_back( name );
        }
    }

    return result;
}

/// Normalise the metadata in the cases where the OIIO attribute name
/// doesn't match the standard OpenEXR and/or ACES Container attribute name.
/// We only check the attribute names which are set by the raw input plugin.
///
/// @param spec ImageSpec to modify
void fix_metadata( OIIO::ImageSpec &spec )
{
    const std::map<std::string, std::string> standard_mapping = {
        { "Make", "cameraMake" }, { "Model", "cameraModel" }
    };

    for ( auto mapping_pair: standard_mapping )
    {
        auto &src_name = mapping_pair.first;
        auto &dst_name = mapping_pair.second;

        auto src_attribute = spec.find_attribute( src_name );
        auto dst_attribute = spec.find_attribute( dst_name );

        if ( dst_attribute == nullptr && src_attribute != nullptr )
        {
            auto type = src_attribute->type();
            if ( type.arraylen == 0 )
            {
                if ( type.basetype == OIIO::TypeDesc::STRING )
                    spec[dst_name] = src_attribute->get_string();
            }
            spec.erase_attribute( src_name );
        }
    }
}

bool ImageConverter::configure(
    const std::string &input_filename, OIIO::ParamValueList &options )
{
    options["raw:ColorSpace"]    = "XYZ";
    options["raw:use_camera_wb"] = 0;
    options["raw:use_auto_wb"]   = 0;

    OIIO::ImageSpec temp_spec;
    temp_spec.extra_attribs = options;

    OIIO::ImageSpec image_spec;
    auto image_input = OIIO::ImageInput::create( "raw", false, &temp_spec );
    bool result = image_input->open( input_filename, image_spec, temp_spec );
    if ( !result )
    {
        return false;
    }

    fix_metadata( image_spec );
    return configure( image_spec, options );
}

// TODO:
// Removed options comparing to v1.1:
// -P - bad pixels
// -K - dark frame
// -j - fuji-rotate
// -m - median filter
// -f - four-colour RGB
// -T - print Libraw-supported cameras
// -F - use big file
// -E - mmap-ed IO
// -s - image index in the file
// -G - green_matching() filter

bool ImageConverter::configure(
    const OIIO::ImageSpec &image_spec, OIIO::ParamValueList &options )
{
    options["raw:use_camera_wb"] = 0;
    options["raw:use_auto_wb"]   = 0;

    options["raw:auto_bright"]        = (int)settings.auto_bright;
    options["raw:adjust_maximum_thr"] = settings.adjust_maximum_threshold;
    options["raw:user_black"]         = settings.black_level;
    options["raw:user_sat"]           = settings.saturation_level;
    options["raw:half_size"]          = (int)settings.half_size;
    options["raw:user_flip"]          = settings.flip;
    options["raw:HighlightMode"]      = settings.highlight_mode;
    options["raw:Demosaic"]           = settings.demosaic_algorithm;
    options["raw:threshold"]          = settings.denoise_threshold;

    if ( settings.crop_box[2] != 0 && settings.crop_box[3] != 0 )
    {
        options.attribute(
            "raw:cropbox",
            OIIO::TypeDesc( OIIO::TypeDesc::INT, 4 ),
            settings.crop_box );
    }

    if ( settings.chromatic_aberration[0] != 1.0 &&
         settings.chromatic_aberration[1] != 1.0 )
    {
        options.attribute(
            "raw:aber",
            OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 2 ),
            settings.chromatic_aberration );
    }

    bool is_DNG =
        image_spec.extra_attribs.find( "raw:dng:version" )->get_int() > 0;

    switch ( settings.WB_method )
    {
        case Settings::WBMethod::Metadata: {
            float custom_WB[4];

            auto camera_multiplier_attribute = image_spec.find_attribute(
                "raw:cam_mul", OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 4 ) );
            if ( camera_multiplier_attribute )
            {
                for ( int i = 0; i < 4; i++ )
                {
                    custom_WB[i] =
                        camera_multiplier_attribute->get_float_indexed( i );
                }

                options.attribute(
                    "raw:user_mul",
                    OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 4 ),
                    custom_WB );

                _WB_multipliers.resize( 4 );
                for ( size_t i = 0; i < 4; i++ )
                    _WB_multipliers[i] = custom_WB[i];
            }
            break;
        }
        case Settings::WBMethod::Illuminant:
            // No configuration is required at this stage.
            break;
        case Settings::WBMethod::Box: {
            bool is_empty_box = settings.WB_box[2] == 0 ||
                                settings.WB_box[3] == 0;

            if ( is_empty_box )
            {
                // use whole image (auto white balancing)
                options["raw:use_auto_wb"] = 1;
            }
            else
            {
                int32_t WB_box[4];
                for ( int i = 0; i < 4; i++ )
                {
                    WB_box[i] = settings.WB_box[i];
                }
                options.attribute(
                    "raw:greybox",
                    OIIO::TypeDesc( OIIO::TypeDesc::INT, 4 ),
                    WB_box );
            }
            break;
        }
        case Settings::WBMethod::Custom:
            options.attribute(
                "raw:user_mul",
                OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 4 ),
                settings.custom_WB );

            _WB_multipliers.resize( 4 );
            for ( size_t i = 0; i < 4; i++ )
                _WB_multipliers[i] = settings.custom_WB[i];
            break;

        default:
            std::cerr
                << "ERROR: This white balancing method has not been configured "
                << "properly." << std::endl;
            return false;
    }

    switch ( settings.matrix_method )
    {
        case Settings::MatrixMethod::Spectral:
            options["raw:ColorSpace"]        = "raw";
            options["raw:use_camera_matrix"] = 0;
            break;
        case Settings::MatrixMethod::Metadata:
            options["raw:ColorSpace"]        = "XYZ";
            options["raw:use_camera_matrix"] = is_DNG ? 1 : 3;
            break;
        case Settings::MatrixMethod::Adobe:
            options["raw:ColorSpace"]        = "XYZ";
            options["raw:use_camera_matrix"] = 1;
            break;
        case Settings::MatrixMethod::Custom:
            options["raw:ColorSpace"]        = "raw";
            options["raw:use_camera_matrix"] = 0;

            _IDT_matrix.resize( 3 );
            for ( int i = 0; i < 3; i++ )
            {
                _IDT_matrix[i].resize( 3 );
                for ( int j = 0; j < 3; j++ )
                {
                    _IDT_matrix[i][j] = settings.custom_matrix[i][j];
                }
            }
            break;
        default:
            std::cerr
                << "ERROR: This matrix method has not been configured properly."
                << std::endl;
            return false;
    }

    bool spectral_white_balance =
        settings.WB_method == Settings::WBMethod::Illuminant;
    bool spectral_matrix =
        settings.matrix_method == Settings::MatrixMethod::Spectral;

    if ( spectral_white_balance || spectral_matrix )
    {
        if ( !prepare_transform_spectral(
                 image_spec,
                 settings,
                 _WB_multipliers,
                 _IDT_matrix,
                 _CAT_matrix ) )
        {
            std::cerr << "ERROR: the colour space transform has not been "
                      << "configured properly (spectral mode)." << std::endl;
            return false;
        }

        if ( spectral_white_balance )
        {
            float custom_WB[4];

            for ( size_t i = 0; i < _WB_multipliers.size(); i++ )
            {
                custom_WB[i] = static_cast<float>( _WB_multipliers[i] );
            }
            if ( _WB_multipliers.size() == 3 )
                custom_WB[3] = static_cast<float>( _WB_multipliers[1] );

            options.attribute(
                "raw:user_mul",
                OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 4 ),
                custom_WB );
        }
    }

    if ( settings.matrix_method == Settings::MatrixMethod::Metadata )
    {
        if ( is_DNG )
        {
            options["raw:use_camera_matrix"] = 1;
            options["raw:use_camera_wb"]     = 1;

            if ( !prepare_transform_DNG(
                     image_spec, settings, _IDT_matrix, _CAT_matrix ) )
            {
                std::cerr << "ERROR: the colour space transform has not been "
                          << "configured properly (metadata mode)."
                          << std::endl;
                return false;
            }
        }
        else
        {
            prepare_transform_nonDNG( _IDT_matrix, _CAT_matrix );
        }
    }
    else if ( settings.matrix_method == Settings::MatrixMethod::Adobe )
    {
        prepare_transform_nonDNG( _IDT_matrix, _CAT_matrix );
    }

    return true;
}

bool ImageConverter::load_image(
    const std::string          &path,
    const OIIO::ParamValueList &hints,
    OIIO::ImageBuf             &buffer )
{
    OIIO::ImageSpec image_spec;
    image_spec.extra_attribs = hints;
    buffer = OIIO::ImageBuf( path, 0, 0, nullptr, &image_spec, nullptr );

    return buffer.read(
        0, 0, 0, buffer.nchannels(), true, OIIO::TypeDesc::FLOAT );
}

bool apply_matrix(
    const std::vector<std::vector<double>> &matrix,
    OIIO::ImageBuf                         &dst,
    const OIIO::ImageBuf                   &src,
    OIIO::ROI                               roi )
{
    float M[4][4];

    size_t num_rows = matrix.size();

    if ( num_rows )
    {
        size_t num_columns = matrix[0].size();

        for ( size_t i = 0; i < num_rows; i++ )
        {
            for ( size_t j = 0; j < num_columns; j++ )
            {
                M[j][i] = static_cast<float>( matrix[i][j] );
            }

            for ( size_t j = num_columns; j < 4; j++ )
                M[j][i] = 0;
        }

        for ( size_t i = num_rows; i < 4; i++ )
        {
            for ( size_t j = 0; j < num_columns; j++ )
                M[j][i] = 0;
            for ( size_t j = num_columns; j < 4; j++ )
                M[j][i] = 1;
        }
    }

    return OIIO::ImageBufAlgo::colormatrixtransform( dst, src, M, false, roi );
}

bool ImageConverter::apply_matrix(
    OIIO::ImageBuf &dst, const OIIO::ImageBuf &src, OIIO::ROI roi )
{
    bool success = true;

    if ( !roi.defined() )
        roi = dst.roi();

    if ( _IDT_matrix.size() )
    {
        success = rta::util::apply_matrix( _IDT_matrix, dst, src, roi );
        if ( !success )
            return false;
    }

    if ( _CAT_matrix.size() )
    {
        success = rta::util::apply_matrix( _CAT_matrix, dst, dst, roi );
        if ( !success )
            return false;

        success = rta::util::apply_matrix( core::XYZ_to_ACES, dst, dst, roi );
        if ( !success )
            return false;
    }

    return success;
}

bool ImageConverter::apply_scale(
    OIIO::ImageBuf &dst, const OIIO::ImageBuf &src, OIIO::ROI /* roi */ )
{
    return OIIO::ImageBufAlgo::mul(
        dst, src, settings.headroom * settings.scale );
}

bool ImageConverter::apply_crop(
    OIIO::ImageBuf &dst, const OIIO::ImageBuf &src, OIIO::ROI /* roi */ )
{
    if ( settings.crop_mode == Settings::CropMode::Off )
    {
        if ( !OIIO::ImageBufAlgo::copy( dst, src ) )
        {
            return false;
        }
        dst.specmod().full_x      = dst.specmod().x;
        dst.specmod().full_y      = dst.specmod().y;
        dst.specmod().full_width  = dst.specmod().width;
        dst.specmod().full_height = dst.specmod().height;
    }
    else if ( settings.crop_mode == Settings::CropMode::Hard )
    {
        // OIIO can not currently crop in place.
        if ( &dst == &src )
        {
            OIIO::ImageBuf temp;
            if ( !OIIO::ImageBufAlgo::copy( temp, src ) )
            {
                return false;
            }

            if ( !OIIO::ImageBufAlgo::crop( dst, temp, temp.roi_full() ) )
            {
                return false;
            }
        }
        else
        {
            if ( !OIIO::ImageBufAlgo::crop( dst, src, src.roi_full() ) )
            {
                return false;
            }
        }
        dst.specmod().x      = 0;
        dst.specmod().y      = 0;
        dst.specmod().full_x = 0;
        dst.specmod().full_y = 0;
    }

    return true;
}

bool ImageConverter::make_output_path(
    std::string &path, const std::string &suffix )
{
    std::filesystem::path temp_path( path );

    temp_path.replace_extension();
    temp_path += suffix + ".exr";

    if ( !settings.output_dir.empty() )
    {
        auto new_directory = std::filesystem::path( settings.output_dir );

        auto filename      = temp_path.filename();
        auto old_directory = temp_path.remove_filename();

        new_directory = old_directory / new_directory;

        if ( !std::filesystem::exists( new_directory ) )
        {
            if ( settings.create_dirs )
            {
                if ( !std::filesystem::create_directory( new_directory ) )
                {
                    std::cerr << "ERROR: Failed to create directory "
                              << new_directory << "." << std::endl;
                    return false;
                }
            }
            else
            {
                std::cerr << "ERROR: The output directory " << new_directory
                          << " does not exist." << std::endl;
                return false;
            }
        }

        temp_path = std::filesystem::absolute( new_directory / filename );
    }

    if ( !settings.overwrite && std::filesystem::exists( temp_path ) )
    {
        std::cerr
            << "ERROR: file " << temp_path << " already exists. Use "
            << "--overwrite to allow overwriting existing files. Skipping "
            << "this file." << std::endl;
        return false;
    }

    path = temp_path.string();
    return true;
}

bool ImageConverter::save_image(
    const std::string &output_filename, const OIIO::ImageBuf &buf )
{
    // ST2065-4 demands these conditions met by an OpenEXR file:
    // - ACES AP0 chromaticities,
    // - acesImageContainerFlag present,
    // - no compression.

    const float chromaticities[] = { 0.7347f, 0.2653f, 0.0f,     1.0f,
                                     0.0001f, -0.077f, 0.32168f, 0.33767f };

    OIIO::ImageSpec image_spec = buf.spec();
    image_spec.set_format( OIIO::TypeDesc::HALF );
    image_spec["acesImageContainerFlag"] = 1;
    image_spec["compression"]            = "none";
    image_spec.attribute(
        "chromaticities",
        OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 8 ),
        chromaticities );

    auto image_output = OIIO::ImageOutput::create( "exr" );
    bool result       = image_output->open( output_filename, image_spec );
    if ( result )
    {
        result = buf.write( image_output.get() );
    }
    return result;
}

bool ImageConverter::process_image( const std::string &input_filename )
{
    std::string output_filename = input_filename;
    if ( !make_output_path( output_filename ) )
    {
        return ( false );
    }

    util::UsageTimer usage_timer;
    usage_timer.enabled = settings.use_timing;

    // ___ Configure transform ___

    usage_timer.reset();
    OIIO::ParamValueList hints;
    if ( !configure( input_filename, hints ) )
    {
        std::cerr << "Failed to configure the reader for the file: "
                  << input_filename << std::endl;
        return ( false );
    }
    usage_timer.print( input_filename, "configuring reader" );

    // ___ Load image ___

    usage_timer.reset();
    OIIO::ImageBuf buffer;
    if ( !load_image( input_filename, hints, buffer ) )
    {
        std::cerr << "Failed to read the file: " << input_filename << std::endl;
        return ( false );
    }
    usage_timer.print( input_filename, "reading image" );

    // ___ Apply matrix/matrices ___

    usage_timer.reset();
    if ( !apply_matrix( buffer, buffer ) )
    {
        std::cerr << "Failed to apply colour space conversion to the file: "
                  << input_filename << std::endl;
        return ( false );
    }
    usage_timer.print( input_filename, "applying transform matrix" );

    // ___ Apply scale ___

    usage_timer.reset();
    if ( !apply_scale( buffer, buffer ) )
    {
        std::cerr << "Failed to apply scale to the file: " << input_filename
                  << std::endl;
        return ( false );
    }
    usage_timer.print( input_filename, "applying scale" );

    // ___ Apply crop ___

    usage_timer.reset();
    if ( !apply_crop( buffer, buffer ) )
    {
        std::cerr << "Failed to apply crop to the file: " << input_filename
                  << std::endl;
        return ( false );
    }
    usage_timer.print( input_filename, "applying crop" );

    // ___ Save image ___

    usage_timer.reset();
    if ( !save_image( output_filename, buffer ) )
    {
        std::cerr << "Failed to save the file: " << output_filename
                  << std::endl;
        return ( false );
    }
    usage_timer.print( input_filename, "writing image" );

    return ( true );
}

const std::vector<double> &ImageConverter::get_WB_multipliers()
{
    return _WB_multipliers;
}

const std::vector<std::vector<double>> &ImageConverter::get_IDT_matrix()
{
    return _IDT_matrix;
}

const std::vector<std::vector<double>> &ImageConverter::get_CAT_matrix()
{
    return _CAT_matrix;
}

} //namespace util
} //namespace rta
