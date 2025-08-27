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

//  =====================================================================
//  Prepare the matching between string flags and single character flag
//
//  inputs:
//      N/A
//
//  outputs:
//      N/A : keys should be prepared and loaded

void create_key( unordered_map<string, char> &keys )
{
    keys["--help"]          = 'I';
    keys["--version"]       = 'V';
    keys["--cameras"]       = 'T';
    keys["--wb-method"]     = 'R';
    keys["--mat-method"]    = 'p';
    keys["--headroom"]      = 'M';
    keys["--valid-illums"]  = 'z';
    keys["--valid-cameras"] = 'Q';
    keys["-c"]              = 'c';
    keys["-C"]              = 'C';
    keys["-k"]              = 'k';
    keys["-S"]              = 'S';
    keys["-n"]              = 'n';
    keys["-H"]              = 'H';
    keys["-t"]              = 't';
    keys["-W"]              = 'W';
    keys["-b"]              = 'b';
    keys["-q"]              = 'q';
    keys["-h"]              = 'h';
    keys["-s"]              = 's';
    keys["-B"]              = 'B';
    keys["-v"]              = 'v';
    keys["-F"]              = 'F';
    keys["-d"]              = 'd';
    keys["-I"]              = 'I';
    keys["-V"]              = 'V';
};

//  =====================================================================
//  Print usage / help message
//
//  inputs:
//      const char * : name of the program (i.e., rawtoaces)
//
//  outputs:
//      N/A

void ImageConverter::usage( const char *prog )
{
    printf( "%s - convert RAW digital camera files to ACES\n", prog );
    printf( "\n" );
    printf( "Usage:\n" );
    printf( "  %s file ...\n", prog );
    printf( "  %s [options] file\n", prog );
    printf( "  %s --help\n", prog );
    printf( "  %s --version\n", prog );
    printf( "\n" );
    printf(
        "IDT options:\n"
        "  --help                  Show this screen\n"
        "  --version               Show version\n"
        "  --wb-method [0-4]       White balance factor calculation method\n"
        "                            0=white balance using file metadata \n"
        "                            1=white balance using user specified illuminant [str] \n"
        "                            2=Average the whole image for white balance\n"
        "                            3=Average a grey box for white balance <x y w h>\n"
        "                            4=Use custom white balance  <r g b g>\n"
        "                            (default = 0)\n"
        "  --mat-method [0-3]      IDT matrix calculation method\n"
        "                            0=Calculate matrix from camera spec sens\n"
        "                            1=Use file metadata color matrix\n"
        "                            2=Use adobe coeffs included in libraw\n"
        "                            3=Use custom matrix <m1r m1g m1b m2r m2g m2b m3r m3g m3b>\n"
        "                            (default = 0)\n"
        "                            (default = /usr/local/include/rawtoaces/data/camera)\n"
        "  --headroom float        Set highlight headroom factor (default = 6.0)\n"
        "  --valid-illums          Show a list of illuminants\n"
        "  --valid-cameras         Show a list of cameras/models with available\n"
        "                          spectral sensitivity datasets\n"
        "\n"
        "Raw conversion options:\n"
        "  -c float                Set adjust maximum threshold (default = 0.75)\n"
        "  -C <r b>                Correct chromatic aberration\n"
        "  -k <num>                Set the darkness level\n"
        "  -S <num>                Set the saturation level\n"
        "  -n <num>                Set threshold for wavelet denoising\n"
        "  -H [0-9]                Highlight mode (0=clip, 1=unclip, 2=blend, 3+=rebuild) (default = 0)\n"
        "  -t [0-7]                Flip image (0=none, 3=180, 5=90CCW, 6=90CW)\n"
        "  -W                      Don't automatically brighten the image\n"
        "  -b <num>                Adjust brightness (default = 1.0)\n"
        "  -q [0-12]                Demosaicing algorithm (0=linear, 1=VNG, 2=PPG, 3=AHD, 4=DCB, 5=AHD-Mod, 6=AFD, 7=VCD, 8=Mixed, 9=LMMSE, 10=AMaZE, 11=DHT, 12=AAHD)\n"
        "  -h                      Half-size color image (twice as fast as \"-q 0\")\n"
        "  -B <x y w h>            Use cropbox\n"
        "\n"
        "Benchmarking options:\n"
        "  -v                      Verbose: print progress messages (repeated -v will add verbosity)\n"
        "  -d                      Detailed timing report\n" );
    exit( -1 );
};

//  =====================================================================

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
    char              separator    = ':';
    const std::string default_path = "/usr/local/share/rawtoaces/data";
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

    if ( path.empty() )
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
    camera_make = spec["cameraMake"];
    if ( camera_make.empty() )
    {
        std::cerr << "Missing the camera manufacturer name in the file "
                  << "metadata." << std::endl;
        return false;
    }

    camera_model = spec["cameraModel"];
    if ( camera_model.empty() )
    {
        std::cerr << "Missing the camera model name in the file metadata. "
                  << std::endl;
        return false;
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
    core::Idt                      &solver,
    const std::vector<std::string> &directories,
    const std::string              &camera_make,
    const std::string              &camera_model,
    const std::string              &illuminant = "" )
{
    bool success = true;

    auto camera_files = collect_data_files( directories, "camera" );
    for ( auto &camera_file: camera_files )
    {
        success = solver.loadCameraSpst(
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
        solver.loadTrainingData( found_training_data[0] );
    }

    std::vector<std::string> found_cmf_files =
        find_files( "cmf/cmf_1931.json", directories );
    if ( found_cmf_files.size() )
    {
        solver.loadCMF( found_cmf_files[0] );
    }

    auto illuminant_files = collect_data_files( directories, "illuminant" );
    if ( illuminant.empty() )
    {
        solver.loadIlluminant( illuminant_files );
    }
    else
    {
        solver.loadIlluminant( illuminant_files, illuminant );
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
    core::Idt solver;
    solver.setVerbosity( verbosity );
    if ( !configure_solver(
             solver, directories, camera_make, camera_model, "" ) )
    {
        return false;
    }

    solver.chooseIllumSrc( wb_mults, highlight );
    out_illuminant = solver.getBestIllum().illuminant;
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
    core::Idt solver;
    solver.setVerbosity( verbosity );
    if ( !configure_solver(
             solver, directories, camera_make, camera_model, illuminant ) )
    {
        return false;
    }

    solver.chooseIllumType( illuminant, highlight );
    out_WB = solver.getWB();
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
    core::Idt solver;
    solver.setVerbosity( verbosity );
    if ( !configure_solver(
             solver, directories, camera_make, camera_model, illuminant ) )
    {
        return false;
    }

    solver.chooseIllumType( illuminant, highlight );
    if ( !solver.calIDT() )
    {
        return false;
    }

    out_matrix = solver.getIDT();
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

    std::string camera_make, camera_model;
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
    std::string camera_make, camera_model;
    if ( !fetch_camera_make_and_model( imageSpec, camera_make, camera_model ) )
        return false;

    core::Metadata metadata;

    metadata.baselineExposure =
        imageSpec.get_float_attribute( "raw:dng:baseline_exposure" );

    metadata.neutralRGB.resize( 3 );

    auto attr = find_and_check_attribute(
        imageSpec, "raw:cam_mul", OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 4 ) );
    if ( attr )
    {
        for ( int i = 0; i < 3; i++ )
            metadata.neutralRGB[i] = 1.0 / attr->get_float_indexed( i );
    }

    for ( size_t k = 0; k < 2; k++ )
    {
        auto &calibration = metadata.calibration[k];
        calibration.xyz2rgbMatrix.resize( 9 );
        calibration.cameraCalibrationMatrix.resize( 9 );

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
                    calibration.xyz2rgbMatrix[i * 3 + j] =
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
                    calibration.cameraCalibrationMatrix[i * 3 + j] =
                        matrix2_attr->get_float_indexed( i * 4 + j );
                }
            }
        }
    }

    core::DNGIdt solver( metadata );
    IDT_matrix = solver.getDNGIDTMatrix();

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

int ImageConverter::configure_settings( int argc, char const *const argv[] )
{
    settings.database_directories = database_paths();

    char *cp, *sp;
    int   arg;

    static unordered_map<string, char> keys;
    create_key( keys );

    for ( arg = 1; arg < argc; )
    {
        string key( argv[arg] );

        if ( key[0] != '-' )
        {
            break;
        }

        arg++;

        char opt = keys[key];

        if ( !opt )
        {
            fprintf(
                stderr, "\nNon-recognizable flag - \"%s\"\n", key.c_str() );
            exit( -1 );
        }

        if ( ( cp = strchr( sp = (char *)"HcnbksStqmBC", opt ) ) != 0 )
        {
            for ( int i = 0; i < "111111111142"[cp - sp] - '0'; i++ )
            {
                if ( !isdigit( argv[arg + i][0] ) )
                {
                    fprintf(
                        stderr,
                        "\nError: Non-numeric argument to "
                        "\"%s\"\n",
                        key.c_str() );
                    exit( -1 );
                }
            }
        }

        switch ( opt )
        {
            case 'I': usage( argv[0] ); break;
            case 'V': printf( "%s\n", VERSION ); break;
            case 'v': settings.verbosity++; break;
            case 'c':
                settings.adjust_maximum_threshold = (float)atof( argv[arg++] );
                break;
            case 'n':
                settings.denoise_threshold = (float)atof( argv[arg++] );
                break;
            case 'b':
                settings.scale = (float)atof( argv[arg++] );
                break;
                //            case 'P': OUT.bad_pixels = argv[arg++]; break;
                //            case 'K': OUT.dark_frame = argv[arg++]; break;
            case 'C': {
                settings.aberration[0] = 1.0 / atof( argv[arg++] );
                settings.aberration[1] = 1.0 / atof( argv[arg++] );
                break;
            }
            case 'k': settings.black_level = atoi( argv[arg++] ); break;
            case 'S': settings.saturation_level = atoi( argv[arg++] ); break;
            case 't': settings.flip = atoi( argv[arg++] ); break;
            case 'q':
                switch ( atoi( argv[arg++] ) )
                {
                    case 0: settings.demosaic_algorithm = "linear"; break;
                    case 1: settings.demosaic_algorithm = "VNG"; break;
                    case 2: settings.demosaic_algorithm = "PPG"; break;
                    case 3: settings.demosaic_algorithm = "AHD"; break;
                    case 4: settings.demosaic_algorithm = "DCB"; break;
                    case 5: settings.demosaic_algorithm = "AHD-Mod"; break;
                    case 6: settings.demosaic_algorithm = "AFD"; break;
                    case 7: settings.demosaic_algorithm = "VCD"; break;
                    case 8: settings.demosaic_algorithm = "Mixed"; break;
                    case 9: settings.demosaic_algorithm = "LMMSE"; break;
                    case 10: settings.demosaic_algorithm = "AMaZE"; break;
                    case 11: settings.demosaic_algorithm = "DHT"; break;
                    case 12: settings.demosaic_algorithm = "AAHD"; break;
                    default: settings.demosaic_algorithm = "AHD"; break;
                }
                break;
                //            case 'm': OUT.med_passes = atoi( argv[arg++] ); break;
            case 'h':
                settings.half_size = true;
                //                // no break:  "-h" implies "-f"
                //            case 'f': OUT.four_color_rgb = 1;
                break;
            case 'B':
                FORI( 4 ) settings.cropbox[i] = atoi( argv[arg++] );
                break;
                //            case 'j': OUT.use_fuji_rotate = 0; break;
            case 'W':
                settings.auto_bright = false;
                break;
                //            case 'F': _opts.use_bigfile = 1; break;
            case 'd': settings.use_timing = 1; break;
            case 'Q': {
                // gather a list of cameras supported
                auto cameras = supported_cameras();
                std::cerr << std::endl
                          << "The following cameras' "
                          << "sensitivity data is available:" << std::endl;
                for ( const auto &camera: cameras )
                {
                    std::cerr << std::endl << camera;
                }
                std::cerr << std::endl;
                break;
            }
            case 'z': {
                // gather a list of illuminants supported
                auto illuminants = supported_illuminants();
                std::cerr << std::endl
                          << "The following illuminants "
                          << "are available:" << std::endl;
                for ( const auto &illuminant: illuminants )
                {
                    std::cerr << std::endl << illuminant;
                }
                std::cerr << std::endl;
                break;
            }
            case 'M': settings.headroom = atof( argv[arg++] ); break;
            case 'H': {
                settings.highlight_mode = atoi( argv[arg++] );
                break;
            }
            case 'p': {
                int mat_method = atoi( argv[arg++] );
                if ( mat_method > 3 || mat_method < 0 )
                {
                    fprintf(
                        stderr,
                        "\nError: Invalid argument to "
                        "\"%s\" \n",
                        key.c_str() );
                    exit( -1 );
                }

                settings.matrixMethod = Settings::MatrixMethod( mat_method );
                if ( settings.matrixMethod == Settings::MatrixMethod::Custom )
                {
                    float custom_buffer[9] = { 0.0 };
                    bool  flag             = false;
                    FORI( 9 )
                    {
                        if ( isalpha( argv[arg][0] ) )
                        {
                            fprintf(
                                stderr,
                                "\nError: Non-numeric argument to "
                                "\"%s %i\" \n",
                                key.c_str(),
                                settings.matrixMethod );
                            exit( -1 );
                        }
                        custom_buffer[i] =
                            static_cast<float>( atof( argv[arg++] ) );
                        if ( i == 8 )
                        {
                            flag = true;
                        }
                    }

                    if ( flag )
                    {
                        FORI( 3 )
                        {
                            FORJ( 3 )
                            {
                                settings.customMatrix[i][j] =
                                    custom_buffer[i * 3 + j];
                            }
                        }
                    }
                }
                break;
            }
            case 'R': {
                std::string flag = std::string( argv[arg] );
                FORI( flag.size() )
                {
                    if ( !isdigit( flag[i] ) )
                    {
                        fprintf(
                            stderr,
                            "\nNon-recognizable argument to "
                            "\"--wb-method\".\n" );
                        exit( -1 );
                    }
                }

                int wb_method = atoi( argv[arg++] );

                if ( wb_method > 4 || wb_method < 0 )
                {
                    fprintf(
                        stderr,
                        "\nError: Invalid argument to "
                        "\"%s\" \n",
                        key.c_str() );
                    exit( -1 );
                }

                settings.wbMethod = Settings::WBMethod( wb_method );

                switch ( wb_method )
                {
                    case 0:
                        settings.wbMethod = Settings::WBMethod::Metadata;
                        break;
                    case 1:
                        settings.wbMethod = Settings::WBMethod::Illuminant;
                        settings.illuminant =
                            OIIO::Strutil::lower( argv[arg++] );

                        if ( !rta::core::isValidCT( settings.illuminant ) )
                        {
                            fprintf(
                                stderr,
                                "\nError: white balance method 1 requires a valid "
                                "illuminant (e.g., D60, 3200K) to be specified\n" );
                            exit( -1 );
                        }
                        break;
                    case 2: settings.wbMethod = Settings::WBMethod::Box; break;
                    case 3:
                        settings.wbMethod = Settings::WBMethod::Box;

                        FORI( 4 )
                        {
                            if ( !isdigit( argv[arg][0] ) )
                            {
                                fprintf(
                                    stderr,
                                    "\nError: Non-numeric argument to "
                                    "\"%s %i\" \n",
                                    key.c_str(),
                                    settings.wbMethod );
                                exit( -1 );
                            }
                            settings.wbBox[i] =
                                static_cast<float>( atof( argv[arg++] ) );
                        }
                        break;
                    case 4:
                        settings.wbMethod = Settings::WBMethod::Custom;

                        FORI( 4 )
                        {
                            if ( !isdigit( argv[arg][0] ) )
                            {
                                fprintf(
                                    stderr,
                                    "\nError: Non-numeric argument to "
                                    "\"%s %i\" \n",
                                    key.c_str(),
                                    settings.wbMethod );
                                exit( -1 );
                            }
                            settings.customWB[i] =
                                static_cast<float>( atof( argv[arg++] ) );
                        }
                        break;
                    default:
                        fprintf(
                            stderr,
                            "\nError: Invalid argument to "
                            "\"%s\" \n",
                            key.c_str() );
                        exit( -1 );
                }

                break;
            }
            default:
                fprintf(
                    stderr, "\nError: Unknown option \"%s\".\n", key.c_str() );
                exit( -1 );
        }
    }

    if ( settings.wbMethod == Settings::WBMethod::Illuminant )
    {
        auto paths =
            collect_data_files( settings.database_directories, "illuminant" );
        core::Idt solver;
        if ( !solver.loadIlluminant( paths, settings.illuminant ) )
        {
            std::cerr << std::endl
                      << "Error: No matching light source. "
                      << "Please find available options by "
                      << "\"rawtoaces --valid-illum\"." << std::endl;
            exit( -1 );
        }
    }

    return arg;
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
