// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include <rawtoaces/acesrender.h>
#include <rawtoaces/rawtoaces_core.h>
#include <rawtoaces/mathOps.h>
#include <rawtoaces/usage_timer.h>

#include <filesystem>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

namespace rta
{
namespace util
{

std::vector<std::string> collect_data_files(
    const std::vector<std::string> &directories, const std::string &type )
{
    std::vector<std::string> result;

    for ( const auto &path: directories )
    {
        if ( std::filesystem::is_directory( path ) )
        {
            auto type_path = path + "/" + type;
            if ( std::filesystem::exists( type_path ) )
            {
                auto it = std::filesystem::directory_iterator( type_path );
                for ( auto filename2: it )
                {
                    auto p = filename2.path();
                    if ( filename2.path().extension() == ".json" )
                    {
                        result.push_back( filename2.path().string() );
                    }
                }
            }
        }
    }
    return result;
}

bool check_and_add_file(
    const std::filesystem::path &path, std::vector<std::string> &batch )
{
    if ( std::filesystem::is_regular_file( path ) ||
         std::filesystem::is_symlink( path ) )
    {
        static const std::set<std::string> ignore_filenames = { ".DS_Store" };
        std::string                        filename = path.filename().string();
        if ( ignore_filenames.count( filename ) > 0 )
            return false;

        static const std::set<std::string> ignore_extensions = {
            ".exr", ".EXR", ".jpg", ".JPG", ".jpeg", ".JPEG"
        };
        std::string extension = path.extension().string();
        if ( ignore_extensions.count( extension ) > 0 )
            return false;

        batch.push_back( path.string() );
    }
    else
    {
        std::cerr << "Not a regular file: " << path << std::endl;
    }
    return true;
}

bool collect_image_files(
    const std::string &path, std::vector<std::vector<std::string>> &batches )
{
    if ( !std::filesystem::exists( path ) )
    {
        return false;
    }

    auto canonical_filename = std::filesystem::canonical( path );

    if ( std::filesystem::is_directory( path ) )
    {
        std::vector<std::string> &curr_batch = batches.emplace_back();
        auto it = std::filesystem::directory_iterator( path );

        for ( auto filename2: it )
        {
            check_and_add_file( filename2, curr_batch );
        }
    }
    else
    {
        check_and_add_file( path, batches[0] );
    }

    return true;
}

// Adapted from define.h pathsFinder()
std::vector<std::string> database_paths()
{
    std::vector<std::string> result;

#if defined( WIN32 ) || defined( WIN64 )
    const std::string separator    = ";";
    const std::string default_path = ".";
#else
    char              separator = ':';
    const std::string default_path =
        "/usr/local/share/rawtoaces/data"
        ":/usr/local/include/rawtoaces/data"; // Old path for back compatibility
#endif

    std::string path;

    const char *env = getenv( "RAWTOACES_DATA_PATH" );
    if ( !env )
    {
        // Fallback to the old environment variable.
        env = getenv( "AMPAS_DATA_PATH" );

        if ( env )
        {
            std::cerr << "Warning: The environment variable "
                      << "AMPAS_DATA_PATH is now deprecated. Please use "
                      << "RAWTOACES_DATA_PATH instead." << std::endl;
        }
    }

    if ( env )
    {
        path = env;
    }
    else
    {
        path = default_path;
    }

    size_t pos = 0;

    while ( pos < path.size() )
    {
        size_t end = path.find( separator, pos );

        if ( end == std::string::npos )
            end = path.size();

        std::string pathItem = path.substr( pos, end - pos );

        if ( find( result.begin(), result.end(), pathItem ) == result.end() )
            result.push_back( pathItem );

        pos = end + 1;
    }

    return result;
};

bool fetch_camera_make_and_model(
    const OIIO::ImageSpec &spec,
    std::string           &camera_make,
    std::string           &camera_model )
{
    if ( camera_make.empty() )
    {
        camera_make = spec["cameraMake"];
        if ( camera_make.empty() )
        {
            std::cerr << "Missing the camera manufacturer name in the file "
                      << "metadata. You can provide a camera make using the "
                      << "--custom-camera-make parameter" << std::endl;
            return false;
        }
    }

    if ( camera_model.empty() )
    {
        camera_model = spec["cameraModel"];
        if ( camera_model.empty() )
        {
            std::cerr << "Missing the camera model name in the file metadata. "
                      << "You can provide a camera make using the "
                      << "--custom-camera-make parameter" << std::endl;
            return false;
        }
    }

    return true;
}

std::vector<std::string> find_files(
    const std::string &file_path, const std::vector<std::string> &search_paths )
{
    std::vector<std::string> found_files;

    for ( auto &search_path: search_paths )
    {
        std::string full_path = search_path + "/" + file_path;

        if ( std::filesystem::exists( full_path ) )
            found_files.push_back( full_path );
    }

    return found_files;
}

bool configure_solver(
    core::SpectralSolver           &solver,
    const std::vector<std::string> &directories,
    const std::string              &camera_make,
    const std::string              &camera_model,
    const std::string              &illuminant = "" )
{
    bool success = false;

    auto camera_files = collect_data_files( directories, "camera" );
    for ( auto &camera_file: camera_files )
    {
        success = solver.load_camera(
            camera_file, camera_make.c_str(), camera_model.c_str() );
        if ( success )
            break;
    }

    if ( !success )
    {
        std::cerr << "Failed to find spectral data for camera " << camera_make
                  << " " << camera_model << std::endl;
        return false;
    }

    std::vector<std::string> found_training_data =
        find_files( "training/training_spectral.json", directories );
    if ( found_training_data.size() )
    {
        // loading training data (190 patches)
        solver.load_training_data( found_training_data[0] );
    }

    std::vector<std::string> found_cmf_files =
        find_files( "cmf/cmf_1931.json", directories );
    if ( found_cmf_files.size() )
    {
        solver.load_observer( found_cmf_files[0] );
    }

    auto illuminant_files = collect_data_files( directories, "illuminant" );
    if ( illuminant.empty() )
    {
        solver.load_illuminant( illuminant_files );
    }
    else
    {
        solver.load_illuminant( illuminant_files, illuminant );
    }

    return true;
}

bool solve_illuminant_from_WB(
    const std::vector<std::string> &directories,
    const std::string              &camera_make,
    const std::string              &camera_model,
    const std::vector<double>      &wb_mults,
    float                           highlight,
    bool                            verbosity,
    std::string                    &out_illuminant )
{
    core::SpectralSolver solver;
    solver.verbosity = verbosity;
    if ( !configure_solver(
             solver, directories, camera_make, camera_model, "" ) )
    {
        return false;
    }

    solver.find_best_illuminant( wb_mults, highlight );
    out_illuminant = solver.get_best_illuminant().illuminant;
    return true;
}

bool solve_WB_from_illuminant(
    const std::vector<std::string> &directories,
    const std::string              &camera_make,
    const std::string              &camera_model,
    const std::string              &illuminant,
    float                           highlight,
    bool                            verbosity,
    std::vector<double>            &out_WB )
{
    core::SpectralSolver solver;
    solver.verbosity = verbosity;
    if ( !configure_solver(
             solver, directories, camera_make, camera_model, illuminant ) )
    {
        return false;
    }

    solver.select_illuminant( illuminant, highlight );
    out_WB = solver.get_WB_multipliers();
    return true;
}

bool solve_matrix_from_illuminant(
    const std::vector<std::string>   &directories,
    const std::string                &camera_make,
    const std::string                &camera_model,
    const std::string                &illuminant,
    float                             highlight,
    bool                              verbosity,
    std::vector<std::vector<double>> &out_matrix )
{
    core::SpectralSolver solver;
    solver.verbosity = verbosity;
    if ( !configure_solver(
             solver, directories, camera_make, camera_model, illuminant ) )
    {
        return false;
    }

    solver.select_illuminant( illuminant, highlight );
    if ( !solver.calculate_IDT_matrix() )
    {
        return false;
    }

    out_matrix = solver.get_IDT_matrix();
    return true;
}

/// Check if an attribute of a given name exists
/// and has the type we are expecting.
const OIIO::ParamValue *find_and_check_attribute(
    const OIIO::ImageSpec &imageSpec,
    const std::string     &name,
    OIIO::TypeDesc         type )
{
    auto attr = imageSpec.find_attribute( name );
    if ( attr )
    {
        auto attr_type = attr->type();
        if ( attr_type == type )
        {
            return attr;
        }
    }
    return nullptr;
}

bool prepare_transform_spectral(
    const OIIO::ImageSpec            &imageSpec,
    const ImageConverter::Settings   &settings,
    std::vector<double>              &WB_multipliers,
    std::vector<std::vector<double>> &IDT_matrix,
    std::vector<std::vector<double>> &CAT_matrix )
{
    std::string lower_illuminant = OIIO::Strutil::lower( settings.illuminant );

    std::string camera_make  = settings.custom_camera_make;
    std::string camera_model = settings.custom_camera_model;
    if ( !fetch_camera_make_and_model( imageSpec, camera_make, camera_model ) )
        return false;

    bool        success = false;
    std::string best_illuminant;

    if ( lower_illuminant.empty() )
    {
        std::vector<double> wb_multipliers( 4 );

        if ( WB_multipliers.size() == 4 )
        {
            for ( int i = 0; i < 3; i++ )
                wb_multipliers[i] = WB_multipliers[i];
        }
        else
        {
            auto attr = find_and_check_attribute(
                imageSpec,
                "raw:pre_mul",
                OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 4 ) );
            if ( attr )
            {
                for ( int i = 0; i < 4; i++ )
                    wb_multipliers[i] = attr->get_float_indexed( i );
            }
        }

        if ( wb_multipliers[3] != 0 )
            wb_multipliers[1] = ( wb_multipliers[1] + wb_multipliers[3] ) / 2.0;
        wb_multipliers.resize( 3 );

        float min_val = std::numeric_limits<float>::max();
        for ( int i = 0; i < 3; i++ )
            if ( min_val > wb_multipliers[i] )
                min_val = wb_multipliers[i];

        if ( min_val > 0 && min_val != 1 )
            for ( int i = 0; i < 3; i++ )
                wb_multipliers[i] /= min_val;

        success = solve_illuminant_from_WB(
            settings.database_directories,
            camera_make,
            camera_model,
            wb_multipliers,
            settings.highlight_mode,
            settings.verbosity,
            best_illuminant );

        if ( !success )
        {
            std::cerr << "ERROR: Failed to find a suitable illuminant."
                      << std::endl;
            return false;
        }

        if ( settings.verbosity > 0 )
        {
            std::cerr << "Found illuminant: " << best_illuminant << std::endl;
        }
    }
    else
    {
        best_illuminant = lower_illuminant;
        success         = solve_WB_from_illuminant(
            settings.database_directories,
            camera_make,
            camera_model,
            lower_illuminant,
            settings.highlight_mode,
            settings.verbosity,
            WB_multipliers );

        if ( !success )
        {
            std::cerr << "ERROR: Failed to calculate the white balancing "
                      << "weights." << std::endl;
            return false;
        }

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

    success = solve_matrix_from_illuminant(
        settings.database_directories,
        camera_make,
        camera_model,
        best_illuminant,
        settings.highlight_mode,
        settings.verbosity,
        IDT_matrix );

    if ( !success )
    {
        std::cerr << "Failed to calculate the input transform matrix."
                  << std::endl;
        return false;
    }

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

    // CAT is embedded in IDT in spectral mode
    CAT_matrix.resize( 0 );

    return true;
}

bool prepare_transform_DNG(
    const OIIO::ImageSpec            &imageSpec,
    const ImageConverter::Settings   &settings,
    std::vector<std::vector<double>> &IDT_matrix,
    std::vector<std::vector<double>> &CAT_matrix )
{
    std::string camera_make  = settings.custom_camera_make;
    std::string camera_model = settings.custom_camera_model;
    if ( !fetch_camera_make_and_model( imageSpec, camera_make, camera_model ) )
        return false;

    core::Metadata metadata;

    metadata.baseline_exposure =
        imageSpec.get_float_attribute( "raw:dng:baseline_exposure" );

    metadata.neutral_RGB.resize( 3 );

    auto attr = find_and_check_attribute(
        imageSpec, "raw:cam_mul", OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 4 ) );
    if ( attr )
    {
        for ( int i = 0; i < 3; i++ )
            metadata.neutral_RGB[i] = 1.0 / attr->get_float_indexed( i );
    }

    for ( size_t k = 0; k < 2; k++ )
    {
        auto &calibration = metadata.calibration[k];
        calibration.XYZ_to_RGB_matrix.resize( 9 );
        calibration.camera_calibration_matrix.resize( 9 );

        auto index_string = std::to_string( k + 1 );

        auto key = "raw:dng:calibration_illuminant" + index_string;
        metadata.calibration[k].illuminant = imageSpec.get_int_attribute( key );

        auto key1         = "raw:dng:color_matrix" + index_string;
        auto matrix1_attr = find_and_check_attribute(
            imageSpec, key1, OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 12 ) );
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

        auto key2         = "raw:dng:camera_calibration" + index_string;
        auto matrix2_attr = find_and_check_attribute(
            imageSpec, key2, OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 16 ) );
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

    core::MetadataSolver solver( metadata );
    IDT_matrix = solver.calculate_IDT_matrix();

    if ( settings.verbosity > 0 )
    {
        std::cerr << "Input transform matrix:" << std::endl;
        for ( auto &i: IDT_matrix )
        {
            for ( auto &j: i )
            {
                std::cerr << j << " ";
            }
            std::cerr << std::endl;
        }
    }

    // Do not apply CAT for DNG
    CAT_matrix.resize( 0 );
    return true;
}

bool prepare_transform_nonDNG(
    const OIIO::ImageSpec            &imageSpec,
    const ImageConverter::Settings   &settings,
    std::vector<std::vector<double>> &IDT_matrix,
    std::vector<std::vector<double>> &CAT_matrix )
{
    // Do not apply IDT for non-DNG
    IDT_matrix.resize( 0 );

    CAT_matrix = rta::core::getCAT(
        rta::core::D65_white_XYZ, rta::core::ACES_white_XYZ );

    return true;
}

const char *HelpString =
    "Rawtoaces converts raw image files from a digital camera to "
    "the Academy Colour Encoding System (ACES) compliant images.\n"
    "The process consists of two parts:\n"
    "- the colour values get converted from the camera native colour "
    "space to the ACES AP0 (see \"SMPTE ST 2065-1\"), and \n"
    "- the image file gets converted from the camera native raw "
    "file format to the ACES Image Container file format "
    "(see \"SMPTE ST 2065-4\").\n"
    "\n"
    "Rawtoaces supports the following white-balancing modes:\n"
    "- \"metadata\" uses the white-balancing coefficients from the raw "
    "image file, provided by the camera.\n"
    "- \"illuminant\" performs white balancing to the illuminant, "
    "provided in the \"--illuminant\" parameter. The list of the "
    "supported illuminants can be seen using the "
    "\"--list-illuminants\" parameter. This mode requires spectral "
    "sensitivity data for the camera model the image comes from. "
    "The list of cameras such data is available for, can be "
    "seen using the \"--list-cameras\" parameter. In addition to the named "
    "illuminants, which are stored under ${RAWTOACES_DATA_PATH}/illuminant, "
    "blackbody illuminants of a given colour temperature can me used (use 'K' "
    "suffix, i.e. '3200K'), as well as daylight illuminants (use the 'D' "
    "prefix, i.e. 'D65').\n"
    "- \"box\" performs white-balancing to make the given region of "
    "the image appear neutral gray. The box position (origin and size) "
    "can be specified using the \"--wb-box\" parameter. In case no such "
    "parameter provided, the whole image is used for white-balancing.\n"
    "- \"custom\" uses the custom white balancing coefficients "
    "provided using the -\"custom-wb\" parameter.\n"
    "\n"
    "Rawtoaces supports the following methods of color matrix "
    "computation:\n"
    "- \"spectral\" uses the camera sensor's spectral sensitivity data "
    "to compute the optimal matrix. This mode requires spectral "
    "sensitivity data for the camera model the image comes from. "
    "The list of cameras such data is available for, can be "
    "seen using the \"--list-cameras\" parameter.\n"
    "- \"metadata\" uses the matrix (matrices) contained in the raw "
    "image file metadata. This mode works best with the images using "
    "the DNG format, as the DNG standard mandates the presense of "
    "such matrices.\n"
    "- \"Adobe\" uses the Adobe coefficients provided by LibRaw. \n"
    "- \"custom\" uses a user-provided color conversion matrix. "
    "A matrix can be specified using the \"--custom-mat\" parameter.\n"
    "\n"
    "The paths rawtoaces uses to search for the spectral sensitivity "
    "data can be specified in the RAWTOACES_DATA_PATH environment "
    "variable.\n";

const char *UsageString =
    "\n"
    "    rawtoaces --wb-method METHOD --mat-method METHOD [PARAMS] "
    "path/to/dir/or/file ...\n"
    "Examples: \n"
    "    rawtoaces --wb-method metadata --mat-method metadata raw_file.dng\n"
    "    rawtoaces --wb-method illuminant --illuminant 3200K --mat-method "
    "spectral raw_file.cr3\n";

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
            if ( ( param_value.size() == 0 ) ||
                 ( ( param_value.size() == 1 ) && ( param_value[0] == 0 ) ) )
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
        if ( ( param_value.size() > 1 ) ||
             ( ( param_value.size() == 1 ) && ( param_value[0] != 0 ) ) )
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
        .action(
            [&]( OIIO::cspan<const char *> argv ) { settings.verbosity++; } );

    arg_parser.arg( "-v" ).hidden().action(
        [&]( OIIO::cspan<const char *> argv ) { settings.verbosity++; } );
}

bool ImageConverter::parse_parameters( const OIIO::ArgParse &arg_parser )
{
    settings.database_directories = database_paths();

    if ( arg_parser["list-cameras"].get<int>() )
    {
        auto cameras = supported_cameras();
        std::cout << std::endl
                  << "Spectral sensitivity data are available for the "
                  << "following cameras:" << std::endl;
        for ( const auto &camera: cameras )
        {
            std::cerr << std::endl << camera;
        }
        std::cerr << std::endl;
    }

    if ( arg_parser["list-illuminants"].get<int>() )
    {
        // gather a list of illuminants supported
        auto illuminants = supported_illuminants();
        std::cerr << std::endl
                  << "The following illuminants are supported:" << std::endl;
        for ( const auto &illuminant: illuminants )
        {
            std::cerr << std::endl << illuminant;
        }
        std::cerr << std::endl;
    }

    std::string wb_method = arg_parser["wb-method"].get();

    if ( wb_method == "metadata" )
    {
        settings.wbMethod = Settings::WBMethod::Metadata;
    }
    else if ( wb_method == "illuminant" )
    {
        settings.wbMethod = Settings::WBMethod::Illuminant;
    }
    else if ( wb_method == "box" )
    {
        settings.wbMethod = Settings::WBMethod::Box;
    }
    else if ( wb_method == "custom" )
    {
        settings.wbMethod = Settings::WBMethod::Custom;
    }
    else
    {
        std::cerr << std::endl
                  << "Unsupported white balancing method: \"" << wb_method
                  << "\"." << std::endl;

        return false;
    }

    std::string mat_method = arg_parser["mat-method"].get();

    if ( mat_method == "spectral" )
    {
        settings.matrixMethod = Settings::MatrixMethod::Spectral;
    }
    else if ( mat_method == "metadata" )
    {
        settings.matrixMethod = Settings::MatrixMethod::Metadata;
    }
    else if ( mat_method == "Adobe" )
    {
        settings.matrixMethod = Settings::MatrixMethod::Adobe;
    }
    else if ( mat_method == "custom" )
    {
        settings.matrixMethod = Settings::MatrixMethod::Custom;
    }
    else
    {
        std::cerr << std::endl
                  << "Unsupported matrix method: \"" << mat_method << "\"."
                  << std::endl;

        return false;
    }

    settings.illuminant = arg_parser["illuminant"].get();

    if ( settings.wbMethod == Settings::WBMethod::Illuminant )
    {
        if ( settings.illuminant.empty() )
        {
            std::cerr << "Warning: the white balancing method was set to "
                      << "\"illuminant\", but no \"--illuminant\" parameter "
                      << "provided. D55 will be used as default." << std::endl;
            settings.illuminant = "D55"; // Set default illuminant
        }
    }
    else
    {
        if ( !settings.illuminant.empty() )
        {
            std::cerr << "Warning: the \"--illuminant\" parameter provided "
                      << "but the white balancing mode different from "
                      << "\"illuminant\" "
                      << "requested. The custom illuminant will be ignored."
                      << std::endl;
        }
    }

    auto box = arg_parser["wb-box"].as_vec<int>();
    check_param(
        "white balancing mode",
        "box",
        "wb-box",
        box,
        4,
        "The box will be ignored.",
        settings.wbMethod == Settings::WBMethod::Box,
        [&]() {
            for ( int i = 0; i < 4; i++ )
                settings.wbBox[i] = box[i];
        },
        [&]() {
            for ( int i = 0; i < 4; i++ )
                settings.wbBox[i] = 0;
        } );

    auto custom_wb = arg_parser["custom-wb"].as_vec<float>();
    check_param(
        "white balancing mode",
        "custom",
        "custom-wb",
        custom_wb,
        4,
        "The scalers will be ignored. The default values of (1, 1, 1, 1) will be used",
        settings.wbMethod == Settings::WBMethod::Custom,
        [&]() {
            for ( int i = 0; i < 4; i++ )
                settings.customWB[i] = custom_wb[i];
        },
        [&]() {
            for ( int i = 0; i < 4; i++ )
                settings.customWB[i] = 1.0;
        } );

    auto custom_mat = arg_parser["custom-mat"].as_vec<float>();
    check_param(
        "matrix mode",
        "custom",
        "custom-mat",
        custom_mat,
        9,
        "Identity matrix will be used",
        settings.matrixMethod == Settings::MatrixMethod::Custom,
        [&]() {
            for ( int i = 0; i < 3; i++ )
                for ( int j = 0; j < 3; j++ )
                    settings.customMatrix[i][j] = custom_mat[i * 3 + j];
        },
        [&]() {
            for ( int i = 0; i < 3; i++ )
                for ( int j = 0; j < 3; j++ )
                    settings.customMatrix[i][j] = i == j ? 1.0 : 0.0;
        } );

    auto crop = arg_parser["crop-box"].as_vec<int>();
    if ( crop.size() == 4 )
    {
        for ( size_t i = 0; i < 4; i++ )
            settings.cropbox[i] = crop[i];
    }

    std::string cropping_mode = arg_parser["crop-mode"].get();

    if ( cropping_mode == "off" )
    {
        settings.crop_mode = Settings::CropMode::Off;
    }
    else if ( cropping_mode == "soft" )
    {
        settings.crop_mode = Settings::CropMode::Soft;
    }
    else if ( cropping_mode == "hard" )
    {
        settings.crop_mode = Settings::CropMode::Hard;
    }
    else
    {
        std::cerr << std::endl
                  << "Unsupported cropping mode: \"" << cropping_mode << "\"."
                  << std::endl;

        return false;
    }

    auto aberration = arg_parser["chromatic-aberration"].as_vec<int>();
    if ( aberration.size() == 2 )
    {
        for ( size_t i = 0; i < 2; i++ )
            settings.aberration[i] = aberration[i];
    }

    auto                         demosaic       = arg_parser["demosaic"].get();
    static std::set<std::string> demosaic_algos = {
        "linear", "VNG",   "PPG",   "AHD",   "DCB", "AHD-Mod", "AFD",
        "VCD",    "Mixed", "LMMSE", "AMaZE", "DHT", "AAHD",    "AHD"
    };

    if ( demosaic_algos.count( demosaic ) != 1 )
    {
        std::cerr << std::endl
                  << "ERROR: unsupported demosaicing algorithm '" << demosaic
                  << ". The following methods are supported: "
                  << "'linear', 'VNG', 'PPG', 'AHD', 'DCB', 'AHD-Mod', 'AFD', "
                  << "'VCD', 'Mixed', 'LMMSE', 'AMaZE', 'DHT', 'AAHD', 'AHD'."
                  << std::endl;
        return false;
    }
    else
    {
        settings.demosaic_algorithm = demosaic;
    }

    settings.custom_camera_make  = arg_parser["custom-camera-make"].get();
    settings.custom_camera_model = arg_parser["custom-camera-model"].get();

    settings.headroom    = arg_parser["headroom"].get<float>();
    settings.auto_bright = arg_parser["auto-bright"].get<int>();
    settings.adjust_maximum_threshold =
        arg_parser["adjust-maximum-threshold"].get<int>();
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
    if ( settings.wbMethod == Settings::WBMethod::Illuminant )
    {
        auto paths =
            collect_data_files( settings.database_directories, "illuminant" );
        core::SpectralSolver solver;
        if ( !solver.load_illuminant( paths, settings.illuminant ) )
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

    auto files =
        collect_data_files( settings.database_directories, "illuminant" );
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

    auto files = collect_data_files( settings.database_directories, "camera" );
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
void fix_metadata( OIIO::ImageSpec &spec )
{
    const std::map<std::string, std::string> standard_mapping = {
        { "Make", "cameraMake" }, { "Model", "cameraModel" }
    };

    for ( auto i: standard_mapping )
    {
        auto &src_name = i.first;
        auto &dst_name = i.second;

        auto src_attribute = spec.find_attribute( src_name );
        auto dst_attribute = spec.find_attribute( dst_name );

        if ( dst_attribute == nullptr && src_attribute != nullptr )
        {
            auto type = src_attribute->type();
            if ( type.arraylen == 0 )
            {
                if ( type.basetype == OIIO::TypeDesc::STRING )
                    spec[dst_name] = src_attribute->get_string();
                else if ( type.basetype == OIIO::TypeDesc::FLOAT )
                    spec[dst_name] = src_attribute->get_float();
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

    OIIO::ImageSpec imageSpec;
    auto imageInput = OIIO::ImageInput::create( "raw", false, &temp_spec );
    bool result     = imageInput->open( input_filename, imageSpec, temp_spec );
    if ( !result )
    {
        return false;
    }

    fix_metadata( imageSpec );
    return configure( imageSpec, options );
}

bool ImageConverter::configure(
    const OIIO::ImageSpec &imageSpec, OIIO::ParamValueList &options )
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

    if ( settings.cropbox[2] != 0 && settings.cropbox[3] != 0 )
    {
        options.attribute(
            "raw:cropbox",
            OIIO::TypeDesc( OIIO::TypeDesc::INT, 4 ),
            settings.cropbox );
    }

    if ( settings.aberration[0] != 1.0 && settings.aberration[1] != 1.0 )
    {
        options.attribute(
            "raw:aber",
            OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 2 ),
            settings.aberration );
    }

    bool is_DNG =
        imageSpec.extra_attribs.find( "raw:dng:version" )->get_int() > 0;

    switch ( settings.wbMethod )
    {
        case Settings::WBMethod::Metadata: {
            float user_mul[4];

            auto attr = find_and_check_attribute(
                imageSpec,
                "raw:cam_mul",
                OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 4 ) );
            if ( attr )
            {
                for ( int i = 0; i < 4; i++ )
                {
                    user_mul[i] = attr->get_float_indexed( i );
                }

                options.attribute(
                    "raw:user_mul",
                    OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 4 ),
                    user_mul );

                _WB_multipliers.resize( 4 );
                for ( size_t i = 0; i < 4; i++ )
                    _WB_multipliers[i] = user_mul[i];
            }
            break;
        }
        case Settings::WBMethod::Illuminant:
            // No configuration is required at this stage.
            break;
        case Settings::WBMethod::Box:

            if ( settings.wbBox[2] == 0 || settings.wbBox[3] == 0 )
            {
                // Empty box, use whole image.
                options["raw:use_auto_wb"] = 1;
            }
            else
            {
                int32_t box[4];
                for ( int i = 0; i < 4; i++ )
                {
                    box[i] = settings.wbBox[i];
                }
                options.attribute(
                    "raw:greybox",
                    OIIO::TypeDesc( OIIO::TypeDesc::INT, 4 ),
                    box );
            }
            break;

        case Settings::WBMethod::Custom:
            options.attribute(
                "raw:user_mul",
                OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 4 ),
                settings.customWB );

            _WB_multipliers.resize( 4 );
            for ( size_t i = 0; i < 4; i++ )
                _WB_multipliers[i] = settings.customWB[i];
            break;

        default:
            std::cerr
                << "ERROR: This white balancing method has not been configured "
                << "properly." << std::endl;
            return false;
    }

    switch ( settings.matrixMethod )
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
                    _IDT_matrix[i][j] = settings.customMatrix[i][j];
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
        settings.wbMethod == Settings::WBMethod::Illuminant;
    bool spectral_matrix =
        settings.matrixMethod == Settings::MatrixMethod::Spectral;

    if ( spectral_white_balance || spectral_matrix )
    {
        if ( !prepare_transform_spectral(
                 imageSpec,
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
            float user_mul[4];

            for ( size_t i = 0; i < _WB_multipliers.size(); i++ )
            {
                user_mul[i] = _WB_multipliers[i];
            }
            if ( _WB_multipliers.size() == 3 )
                user_mul[3] = _WB_multipliers[1];

            options.attribute(
                "raw:user_mul",
                OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 4 ),
                user_mul );
        }
    }

    if ( settings.matrixMethod == Settings::MatrixMethod::Metadata )
    {
        if ( is_DNG )
        {
            options["raw:use_camera_matrix"] = 1;
            options["raw:use_camera_wb"]     = 1;

            if ( !prepare_transform_DNG(
                     imageSpec, settings, _IDT_matrix, _CAT_matrix ) )
            {
                std::cerr << "ERROR: the colour space transform has not been "
                          << "configured properly (metadata mode)."
                          << std::endl;
                return false;
            }
        }
        else
        {
            if ( !prepare_transform_nonDNG(
                     imageSpec, settings, _IDT_matrix, _CAT_matrix ) )
            {
                std::cerr << "ERROR: the colour space transform has not been "
                          << "configured properly (Adobe mode)." << std::endl;
                return false;
            }
        }
    }
    else if ( settings.matrixMethod == Settings::MatrixMethod::Adobe )
    {
        if ( !prepare_transform_nonDNG(
                 imageSpec, settings, _IDT_matrix, _CAT_matrix ) )
        {
            std::cerr << "ERROR: the colour space transform has not been "
                      << "configured properly (Adobe mode)." << std::endl;
            return false;
        }
    }

    return true;
}

bool ImageConverter::load_image(
    const std::string          &path,
    const OIIO::ParamValueList &hints,
    OIIO::ImageBuf             &buffer )
{
    OIIO::ImageSpec imageSpec;
    imageSpec.extra_attribs = hints;
    buffer = OIIO::ImageBuf( path, 0, 0, nullptr, &imageSpec, nullptr );

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

    size_t n = matrix.size();

    if ( n )
    {
        size_t m = matrix[0].size();

        for ( size_t i = 0; i < n; i++ )
        {
            for ( size_t j = 0; j < m; j++ )
            {
                M[j][i] = matrix[i][j];
            }

            for ( size_t j = m; j < 4; j++ )
                M[j][i] = 0;
        }

        for ( size_t i = n; i < 4; i++ )
        {
            for ( size_t j = 0; j < m; j++ )
                M[j][i] = 0;
            for ( size_t j = m; j < 4; j++ )
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
    OIIO::ImageBuf &dst, const OIIO::ImageBuf &src, OIIO::ROI roi )
{
    return OIIO::ImageBufAlgo::mul(
        dst, src, settings.headroom * settings.scale );
}

bool ImageConverter::apply_crop(
    OIIO::ImageBuf &dst, const OIIO::ImageBuf &src, OIIO::ROI roi )
{
    if ( !OIIO::ImageBufAlgo::copy( dst, src ) )
    {
        return false;
    }
    dst.specmod().full_x      = dst.specmod().x;
    dst.specmod().full_y      = dst.specmod().y;
    dst.specmod().full_width  = dst.specmod().width;
    dst.specmod().full_height = dst.specmod().height;

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

    const float chromaticities[] = { 0.7347, 0.2653, 0,       1,
                                     0.0001, -0.077, 0.32168, 0.33767 };

    OIIO::ImageSpec imageSpec = buf.spec();
    imageSpec.set_format( OIIO::TypeDesc::HALF );
    imageSpec["acesImageContainerFlag"] = 1;
    imageSpec["compression"]            = "none";
    imageSpec.attribute(
        "chromaticities",
        OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 8 ),
        chromaticities );

    auto imageOutput = OIIO::ImageOutput::create( "exr" );
    bool result      = imageOutput->open( output_filename, imageSpec );
    if ( result )
    {
        result = buf.write( imageOutput.get() );
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
