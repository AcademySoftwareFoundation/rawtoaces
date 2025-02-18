// Copyright Contributors to the rawtoaces project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/rawtoaces

#include <rawtoaces/rawtoaces_util.h>
#include <rawtoaces/define.h>
#include <rawtoaces/metadata.h>
#include <rawtoaces/rta.h>
#include <rawtoaces/mathOps.h>

#include "transform_cache.h"

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include <filesystem>

namespace rta
{

cache::TransformCache transform_cache;

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
    "seen using the \"--list-cameras\" parameter.\n"
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
    "    rawtoaces --wb-method metadata --mat-method metadata raw_file.cr3\n"
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

void ImageConverter::init_parser( OIIO::ArgParse &argParse ) const
{
    argParse.intro( HelpString );
    argParse.usage( UsageString );
    argParse.print_defaults( true );
    argParse.add_help( true );
    argParse.add_version( "TODO: VERSION NUMBER" );

    argParse.arg( "--wb-method" )
        .help(
            "White balance method. Supported options: metadata, illuminant, "
            "box, custom." )
        .metavar( "STR" )
        .defaultval( "metadata" )
        .action( OIIO::ArgParse::store() );

    argParse.arg( "--mat-method" )
        .help(
            "IDT matrix calculation method. Supported options: spectral, "
            "metadata, Adobe, custom." )
        .metavar( "STR" )
        .defaultval( "spectral" )
        .action( OIIO::ArgParse::store() );

    argParse.arg( "--illuminant" )
        .help( "Illuminant for white balancing. (default = D55)" )
        .metavar( "STR" )
        .action( OIIO::ArgParse::store() );

    argParse.arg( "--wb-box" )
        .help(
            "Box to use for white balancing. (default = (0,0,0,0) - full "
            "image)" )
        .nargs( 4 )
        .metavar( "X Y W H" )
        .action( OIIO::ArgParse::store<int>() );

    argParse.arg( "--custom-wb" )
        .help( "Custom white balance multipliers." )
        .nargs( 4 )
        .metavar( "R G B G" )
        .action( OIIO::ArgParse::store<float>() );

    argParse.arg( "--custom-mat" )
        .help( "Custom camera RGB to XYZ matrix." )
        .nargs( 9 )
        .metavar( "Rr Rg Rb Gr Gg Gb Br Bg Bb" )
        .action( OIIO::ArgParse::store<float>() );

    argParse.arg( "--headroom" )
        .help( "Highlight headroom factor." )
        .metavar( "VAL" )
        .defaultval( 6.0f )
        .action( OIIO::ArgParse::store<float>() );

    argParse.separator( "Raw conversion options:" );

    argParse.arg( "--no-auto-bright" )
        .help( "Disable automatic exposure adjustment." )
        .action( OIIO::ArgParse::store_true() );

    argParse.arg( "--adjust-maximum-threshold" )
        .help(
            "Automatically lower the linearity threshold provided in the "
            "metadata by this scaling factor." )
        .metavar( "VAL" )
        .defaultval( 0.75f )
        .action( OIIO::ArgParse::store<float>() );

    argParse.arg( "--black-level" )
        .help( "If >= 0, override the black level." )
        .metavar( "VAL" )
        .defaultval( -1 )
        .action( OIIO::ArgParse::store<int>() );

    argParse.arg( "--saturation-level" )
        .help(
            "If not 0, override the level which appears to be saturated "
            "after normalisation." )
        .metavar( "VAL" )
        .defaultval( 0 )
        .action( OIIO::ArgParse::store<int>() );

    argParse.arg( "--chromatic-aberration" )
        .help(
            "Red and blue scale factors for chromatic aberration correction. "
            "The value of 1 means no correction." )
        .metavar( "R B" )
        .nargs( 2 )
        .defaultval( 1.0f )
        .action( OIIO::ArgParse::store<float>() );

    argParse.arg( "--half-size" )
        .help( "If present, decode image at half size resolution." )
        .action( OIIO::ArgParse::store_true() );

    argParse.arg( "--highlight-mode" )
        .help( "0 = clip, 1 = unclip, 2 = blend, 3..9 = rebuild." )
        .metavar( "VAL" )
        .defaultval( 0 )
        .action( OIIO::ArgParse::store<int>() );

    argParse.arg( "--cropbox" )
        .help(
            "Apply custom crop. If not present, the default crop is applied, "
            "which should match the crop of the in-camera JPEG." )
        .nargs( 4 )
        .metavar( "X Y W H" )
        .action( OIIO::ArgParse::store<int>() );

    argParse.arg( "--flip" )
        .help(
            "If not 0, override the orientation specified in the metadata. "
            "1..8 correspond to EXIF orientation codes "
            "(3 = 180 deg, 6 = 90 deg CCW, 8 = 90 deg CW.)" )
        .metavar( "VAL" )
        .defaultval( 0 )
        .action( OIIO::ArgParse::store<int>() );

    argParse.arg( "--denoise_threshold" )
        .help( "Wavelet denoising threshold" )
        .metavar( "VAL" )
        .defaultval( 0 )
        .action( OIIO::ArgParse::store<int>() );

    argParse.separator( "Benchmarking and debugging:" );

    argParse.arg( "--list-cameras" )
        .help( "Shows the list of cameras supported in spectral mode." )
        .action( OIIO::ArgParse::store_true() );

    argParse.arg( "--list-illuminants" )
        .help( "Shows the list of illuminants supported in spectral mode." )
        .action( OIIO::ArgParse::store_true() );

    argParse.arg( "--verbose" )
        .help( "Verbosity level. 0 = off, 1 = info, 2 = debug." )
        .metavar( "VAL" )
        .defaultval( 0 )
        .action( OIIO::ArgParse::store<int>() );
}

bool ImageConverter::parse_params( const OIIO::ArgParse &argParse )
{
    if ( argParse["list-cameras"].get<int>() )
    {
        std::cout
            << "Spectral sensitivity data are available for the following "
            << "cameras:" << std::endl;

        Idt                      idt;
        auto                     paths = cache::collectDataFiles( "camera" );
        std::vector<std::string> cameras;
        for ( auto path: paths )
        {
            Spst spst;
            if ( spst.loadSpst( path, nullptr, nullptr ) )
            {
                cameras.push_back(
                    std::string( spst.getBrand() ) + " " + spst.getModel() );
            }
        }

        std::sort( cameras.begin(), cameras.end() );

        for ( auto s: cameras )
            std::cout << "- " << s << std::endl;
        std::cout << std::endl;
        exit( 0 );
    }

    if ( argParse["list-illuminants"].get<int>() )
    {
        std::cout << "The following illuminants are supported:" << std::endl;
        std::cout << "- The standard illuminant series D (e.g., D60, D6025)"
                  << std::endl;
        std::cout << "- Black-body radiation (e.g., 3200K)" << std::endl;

        Idt  idt;
        auto paths = cache::collectDataFiles( "illuminant" );
        std::vector<std::string> illuminants;
        for ( auto path: paths )
        {
            Illum illum;
            if ( illum.readSPD( path, "na" ) )
            {
                illuminants.push_back( illum.getIllumType() );
            }
        }

        std::sort( illuminants.begin(), illuminants.end() );

        for ( auto s: illuminants )
            std::cout << "- " << s << std::endl;
        std::cout << std::endl;
        exit( 0 );
    }

    verbosity = argParse["verbose"].get<int>();

    std::string wb_method = argParse["wb-method"].get();

    if ( wb_method == "metadata" )
    {
        wbMethod = WBMethod::Metadata;
    }
    else if ( wb_method == "illuminant" )
    {
        wbMethod = WBMethod::Illuminant;
    }
    else if ( wb_method == "box" )
    {
        wbMethod = WBMethod::Box;
    }
    else if ( wb_method == "custom" )
    {
        wbMethod = WBMethod::Custom;
    }
    else
    {
        std::cerr << std::endl
                  << "Unsupported white balancing method: \"" << wb_method
                  << "\"." << std::endl;

        return false;
    }

    std::string mat_method = argParse["mat-method"].get();

    if ( mat_method == "spectral" )
    {
        matrixMethod = MatrixMethod::Spectral;
    }
    else if ( mat_method == "metadata" )
    {
        matrixMethod = MatrixMethod::Metadata;
    }
    else if ( mat_method == "Adobe" )
    {
        matrixMethod = MatrixMethod::Adobe;
    }
    else if ( mat_method == "custom" )
    {
        matrixMethod = MatrixMethod::Custom;
    }
    else
    {
        std::cerr << std::endl
                  << "Unsupported matrix method: \"" << mat_method << "\"."
                  << std::endl;

        return false;
    }

    headroom = argParse["headroom"].get<float>();

    illuminant = argParse["illuminant"].get();

    if ( wbMethod == WBMethod::Illuminant )
    {
        if ( illuminant.empty() )
        {
            std::cerr << "Warning: the white balancing method was set to "
                      << "\"illuminant\", but no \"--illuminant\" parameter "
                      << "provided. " << illuminant << " will be used."
                      << std::endl;
        }
    }
    else
    {
        if ( !illuminant.empty() )
        {
            std::cerr << "Warning: the \"--illuminant\" parameter provided "
                      << "but the white balancing mode different from "
                      << "\"illuminant\" "
                      << "requested. The custom illuminant will be ignored."
                      << std::endl;
        }
    }

    auto box = argParse["wb-box"].as_vec<int>();
    check_param(
        "white balancing mode",
        "box",
        "wb-box",
        box,
        4,
        "The box will be ignored.",
        wbMethod == WBMethod::Box,
        [&]() {
            for ( int i = 0; i < 4; i++ )
                wbBox[i] = box[i];
        },
        [&]() {
            for ( int i = 0; i < 4; i++ )
                wbBox[i] = 0;
        } );

    auto custom_wb = argParse["custom-wb"].as_vec<float>();
    check_param(
        "white balancing mode",
        "custom",
        "custom-wb",
        custom_wb,
        4,
        "The scalers will be ignored. The default values of (1, 1, 1, 1) will be used",
        wbMethod == WBMethod::Custom,
        [&]() {
            for ( int i = 0; i < 4; i++ )
                customWB[i] = custom_wb[i];
        },
        [&]() {
            for ( int i = 0; i < 4; i++ )
                customWB[i] = 1.0;
        } );

    auto custom_mat = argParse["custom-mat"].as_vec<float>();
    check_param(
        "matrix mode",
        "custom",
        "custom-mat",
        custom_mat,
        9,
        "Identity matrix will be used",
        matrixMethod == MatrixMethod::Custom,
        [&]() {
            for ( int i = 0; i < 3; i++ )
                for ( int j = 0; j < 3; j++ )
                    customMatrix[i][j] = custom_mat[i * 3 + j];
        },
        [&]() {
            for ( int i = 0; i < 3; i++ )
                for ( int j = 0; j < 3; j++ )
                    customMatrix[i][j] = i == j ? 1.0 : 0.0;
        } );

    auto crop = argParse["cropbox"].as_vec<int>();
    if ( crop.size() == 4 )
    {
        for ( size_t i = 0; i < 4; i++ )
            cropbox[i] = crop[i];
    }

    no_auto_bright           = argParse["no-auto-bright"].get<int>();
    adjust_maximum_threshold = argParse["adjust-maximum-threshold"].get<int>();
    black_level              = argParse["black-level"].get<int>();
    saturation_level         = argParse["saturation-level"].get<int>();
    half_size                = argParse["half-size"].get<int>();
    highlight_mode           = argParse["highlight-mode"].get<int>();
    flip                     = argParse["flip"].get<int>();

    return true;
}

bool ImageConverter::configure(
    const OIIO::ImageSpec &imageSpec, OIIO::ParamValueList &options )
{
    options["raw:use_camera_wb"] = 0;
    options["raw:use_auto_wb"]   = 0;

    options["raw:auto_bright"]        = no_auto_bright ? 0 : 1;
    options["raw:adjust_maximum_thr"] = adjust_maximum_threshold;
    options["raw:user_black"]         = black_level;
    options["raw:user_sat"]           = saturation_level;
    options["raw:half_size"]          = (int)half_size;
    options["raw:user_flip"]          = flip;
    options["raw:HighlightMode"]      = highlight_mode;

    if ( cropbox[2] != 0 && cropbox[3] != 0 )
    {
        options.attribute(
            "raw:cropbox", OIIO::TypeDesc( OIIO::TypeDesc::INT, 4 ), cropbox );
    }

    _is_DNG = imageSpec.extra_attribs.find( "raw:dng:version" )->get_int() > 0;

    switch ( wbMethod )
    {
        case WBMethod::Metadata: {
            float user_mul[4];

            for ( int i = 0; i < 4; i++ )
            {
                user_mul[i] = imageSpec.find_attribute( "raw:cam_mul" )
                                  ->get_float_indexed( i );
            }

            options.attribute(
                "raw:user_mul",
                OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 4 ),
                user_mul );

            //            if ( _read_raw )
            {
                _WB_mults.resize( 4 );
                for ( size_t i = 0; i < 4; i++ )
                    _WB_mults[i] = user_mul[i];
            }
            break;
        }
        case WBMethod::Illuminant: {
            std::string lower_illuminant = OIIO::Strutil::lower( illuminant );
            if ( !isValidCT( lower_illuminant ) )
            {
                std::cerr << "Unrecognised illuminant \'" << illuminant << "\'"
                          << std::endl;
                return false;
            }

            break;
        }
        case WBMethod::Box:

            if ( wbBox[2] == 0 || wbBox[3] == 0 )
            {
                // Empty box, use whole image.
                options["raw:use_auto_wb"] = 1;
            }
            else
            {
                int32_t box[4];
                for ( int i = 0; i < 4; i++ )
                {
                    box[i] = wbBox[i];
                }
                options.attribute(
                    "raw:greybox",
                    OIIO::TypeDesc( OIIO::TypeDesc::INT, 4 ),
                    box );
            }
            break;

        case WBMethod::Custom:
            options.attribute(
                "raw:user_mul",
                OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 4 ),
                customWB );

            //            if ( _read_raw )
            {
                _WB_mults.resize( 4 );
                for ( size_t i = 0; i < 4; i++ )
                    _WB_mults[i] = customWB[i];
            }
            break;

        default:
            std::cerr
                << "ERROR: This white balancing method has not been configured "
                << "properly." << std::endl;
            exit( 1 );
    }

    switch ( matrixMethod )
    {
        case MatrixMethod::Spectral:
            options["raw:ColorSpace"]        = "raw";
            options["raw:use_camera_matrix"] = 0;
            break;
        case MatrixMethod::Metadata:
            options["raw:ColorSpace"]        = "XYZ";
            options["raw:use_camera_matrix"] = _is_DNG ? 1 : 3;
            break;
        case MatrixMethod::Adobe:
            options["raw:ColorSpace"]        = "XYZ";
            options["raw:use_camera_matrix"] = 1;
            break;
        case MatrixMethod::Custom:
            options["raw:ColorSpace"]        = "raw";
            options["raw:use_camera_matrix"] = 0;

            _IDT_matrix.resize( 3 );
            for ( int i = 0; i < 3; i++ )
            {
                _IDT_matrix[i].resize( 3 );
                for ( int j = 0; j < 3; j++ )
                {
                    _IDT_matrix[i][j] = customMatrix[i][j];
                }
            }
            break;
        default:
            std::cerr
                << "ERROR: This matrix method has not been configured properly."
                << std::endl;
            exit( 1 );
    }

    bool spectral_white_balance = wbMethod == WBMethod::Illuminant;
    bool spectral_matrix        = matrixMethod == MatrixMethod::Spectral;

    if ( spectral_white_balance || spectral_matrix )
    {
        prepareIDT_spectral(
            imageSpec, spectral_white_balance, spectral_matrix );

        if ( spectral_white_balance )
        {
            float user_mul[4];

            for ( int i = 0; i < _WB_mults.size(); i++ )
            {
                user_mul[i] = _WB_mults[i];
            }
            if ( _WB_mults.size() == 3 )
                user_mul[3] = _WB_mults[1];

            options.attribute(
                "raw:user_mul",
                OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 4 ),
                user_mul );
        }
    }

    if ( matrixMethod == MatrixMethod::Metadata ||
         matrixMethod == MatrixMethod::Adobe )
    {
        if ( _is_DNG )
        {
            options["raw:use_camera_matrix"] = 1;
            options["raw:use_camera_wb"]     = 1;

            prepareIDT_DNG( imageSpec );
        }
        else
        {
            prepareIDT_nonDNG( imageSpec );
        }
    }

    return true;
}

bool ImageConverter::configure(
    const std::string &input_filename, OIIO::ParamValueList &options )
{
    _read_raw = options.get_string( "raw:Demosaic" ) == "none";

    OIIO::ImageSpec imageSpec;

    options["raw:ColorSpace"]    = "XYZ";
    options["raw:use_camera_wb"] = 0;
    options["raw:use_auto_wb"]   = 0;

    OIIO::ImageSpec temp_spec;
    temp_spec.extra_attribs = options;

    auto imageInput = OIIO::ImageInput::create( "raw", false, &temp_spec );
    bool result     = imageInput->open( input_filename, imageSpec, temp_spec );
    if ( !result )
    {
        return false;
    }

    return configure( imageSpec, options );
}

bool ImageConverter::applyMatrix(
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
        success = applyMatrix( _IDT_matrix, dst, src, roi );
        if ( !success )
            return false;
    }

    if ( _CAT_matrix.size() )
    {
        success = applyMatrix( _CAT_matrix, dst, dst, roi );
        if ( !success )
            return false;

        success = OIIO::ImageBufAlgo::colormatrixtransform(
            dst, dst, XYZ_acesrgb_transposed_4, false, roi );
        if ( !success )
            return false;
    }

    return success;
}

bool ImageConverter::apply_scale(
    OIIO::ImageBuf &dst, const OIIO::ImageBuf &src, OIIO::ROI roi )
{
    return OIIO::ImageBufAlgo::mul( dst, src, headroom );
}

bool ImageConverter::save(
    const std::string &output_filename, const OIIO::ImageBuf &buf )
{
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
    result           = buf.write( imageOutput.get() );
    return result;
}

void ImageConverter::prepareIDT_spectral(
    const OIIO::ImageSpec &imageSpec,
    bool                   calc_white_balance,
    bool                   calc_matrix )
{
    std::string lower_illuminant = OIIO::Strutil::lower( illuminant );
    if ( lower_illuminant.empty() )
        lower_illuminant = "na";

    cache::TransformDescriptor descriptor;
    descriptor.camera_make  = imageSpec["Make"];
    descriptor.camera_model = imageSpec["Model"];

    if ( lower_illuminant == "na" )
    {
        std::vector<double> wb_multipliers( 4 );

        if ( _WB_mults.size() == 4 )
        {
            for ( int i = 0; i < 3; i++ )
                wb_multipliers[i] = _WB_mults[i];
        }
        else
        {
            auto attr = imageSpec.find_attribute( "raw:pre_mul" );
            for ( int i = 0; i < 4; i++ )
                wb_multipliers[i] = attr->get_float_indexed( i );
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

        descriptor.type = cache::TransformEntryType::Illum_from_WB;
        cache::WB &wb   = descriptor.value.emplace<cache::WB>();
        wb.value[0]     = wb_multipliers[0];
        wb.value[1]     = wb_multipliers[1];
        wb.value[2]     = wb_multipliers[2];
    }
    else
    {
        descriptor.type  = cache::TransformEntryType::WB_from_Illum;
        descriptor.value = lower_illuminant;
    }

    transform_cache.verbosity = verbosity;
    const cache::TransformCacheEntryData &wb_data =
        transform_cache.fetch( descriptor );

    if ( calc_white_balance )
    {
        auto p = std::get<std::pair<cache::WB, std::string>>( wb_data.value );
        const cache::WB &wb = p.first;
        _WB_mults.resize( 3 );
        _WB_mults[0] = wb.value[0];
        _WB_mults[1] = wb.value[1];
        _WB_mults[2] = wb.value[2];
    }

    if ( calc_matrix )
    {
        auto &p = std::get<std::pair<cache::WB, std::string>>( wb_data.value );
        std::string illum = p.second;

        cache::TransformDescriptor descriptor;
        descriptor.camera_make  = imageSpec["Make"];
        descriptor.camera_model = imageSpec["Model"];
        descriptor.type         = cache::TransformEntryType::Mat_from_Illum;
        descriptor.value        = illum;

        const cache::TransformCacheEntryData &mat_data =
            transform_cache.fetch( descriptor );
        const cache::Matrix33 &mat =
            std::get<cache::Matrix33>( mat_data.value );

        _IDT_matrix.resize( 3 );
        for ( size_t i = 0; i < 3; i++ )
        {
            _IDT_matrix[i].resize( 3 );
            for ( size_t j = 0; j < 3; j++ )
            {
                _IDT_matrix[i][j] = mat.value[i][j];
            }
        }

        _CAT_matrix.resize( 0 );
    }
}

void fetch_matrix(
    cache::TransformDescriptor       &descriptor,
    std::vector<std::vector<double>> &matrix )
{
    const cache::TransformCacheEntryData &mat_data =
        transform_cache.fetch( descriptor );
    const cache::Matrix33 &mat = std::get<cache::Matrix33>( mat_data.value );

    matrix.resize( 3 );
    for ( size_t i = 0; i < 3; i++ )
    {
        matrix[i].resize( 3 );
        for ( size_t j = 0; j < 3; j++ )
        {
            matrix[i][j] = mat.value[i][j];
        }
    }
}

void ImageConverter::prepareIDT_DNG( const OIIO::ImageSpec &imageSpec )
{
    cache::TransformDescriptor descriptor;
    descriptor.type         = cache::TransformEntryType::Mat_from_DNG;
    descriptor.camera_make  = imageSpec["Make"];
    descriptor.camera_model = imageSpec["Model"];
    Metadata &metadata      = descriptor.value.emplace<Metadata>();

    metadata.neutralRGB.resize( 3 );
    metadata.xyz2rgbMatrix1.resize( 9 );
    metadata.xyz2rgbMatrix2.resize( 9 );
    metadata.cameraCalibration1.resize( 9 );
    metadata.cameraCalibration2.resize( 9 );

    metadata.baselineExposure =
        imageSpec.get_float_attribute( "raw:dng:baseline_exposure" );
    metadata.calibrationIlluminant1 =
        imageSpec.get_int_attribute( "raw:dng:calibration_illuminant1" );
    metadata.calibrationIlluminant2 =
        imageSpec.get_int_attribute( "raw:dng:calibration_illuminant2" );

    for ( int i = 0; i < 3; i++ )
    {
        metadata.neutralRGB[i] =
            1.0 /
            imageSpec.find_attribute( "raw:cam_mul" )->get_float_indexed( i );

        for ( int j = 0; j < 3; j++ )
        {
            metadata.xyz2rgbMatrix1[i * 3 + j] =
                imageSpec.find_attribute( "raw:dng:color_matrix1" )
                    ->get_float_indexed( i * 3 + j );
            metadata.xyz2rgbMatrix2[i * 3 + j] =
                imageSpec.find_attribute( "raw:dng:color_matrix2" )
                    ->get_float_indexed( i * 3 + j );
            metadata.cameraCalibration1[i * 3 + j] =
                imageSpec.find_attribute( "raw:dng:camera_calibration1" )
                    ->get_float_indexed( i * 4 + j );
            metadata.cameraCalibration2[i * 3 + j] =
                imageSpec.find_attribute( "raw:dng:camera_calibration2" )
                    ->get_float_indexed( i * 4 + j );
        }
    }

    fetch_matrix( descriptor, _IDT_matrix );

    // Do not apply CAT for DNG
    _CAT_matrix.resize( 0 );
}

void ImageConverter::prepareIDT_nonDNG( const OIIO::ImageSpec &imageSpec )
{
    if ( _read_raw )
    {
        auto   mat  = imageSpec.find_attribute( "raw:cam_xyz" );
        size_t size = mat->type().arraylen;

        if ( size == 12 )
        {
            cache::TransformDescriptor descriptor;
            descriptor.type        = cache::TransformEntryType::Mat_from_nonDNG;
            descriptor.camera_make = imageSpec["Make"];
            descriptor.camera_model = imageSpec["Model"];

            auto &p =
                descriptor.value
                    .emplace<std::pair<cache::Vector3, cache::Matrix33>>();

            cache::Vector3  &wb   = p.first;
            cache::Matrix33 &mat2 = p.second;

            int idx = 0;
            for ( size_t row = 0; row < 3; row++ )
            {
                for ( size_t col = 0; col < 3; col++ )
                {
                    mat2.value[row][col] = mat->get_float_indexed( idx++ );
                }
            }

            fetch_matrix( descriptor, _IDT_matrix );
        }
    }
    else
    {
        _IDT_matrix.resize( 0 );
    }

    vector<double> dIV( d65, d65 + 3 );
    vector<double> dOV( d60, d60 + 3 );
    _CAT_matrix = getCAT( dIV, dOV );
}

} // namespace rta
