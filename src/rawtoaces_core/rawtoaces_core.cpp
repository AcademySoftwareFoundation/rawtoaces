// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include <rawtoaces/rawtoaces_core.h>
#include "rawtoaces_core_priv.h"
#include "mathOps.h"
#include "define.h"

using namespace ceres;

namespace rta
{
namespace core
{


/// Calculate the chromaticity values (x, y) based on correlated color temperature (CCT).
/// The function converts a correlated color temperature to CIE 1931 chromaticity coordinates
/// using empirical formulas for different temperature ranges.
/// 
/// @param cct The correlated color temperature in Kelvin
/// @return A vector containing [x, y] chromaticity coordinates
vector<double> cct_to_XY( const double &cct )
{
    double x;
    if ( cct >= 4002.15 && cct <= 7003.77 )
    {
        x =
            ( 0.244063 + 99.11 / cct +
              2.9678 * 1000000 / ( std::pow( cct, 2 ) ) -
              4.6070 * 1000000000 / ( std::pow( cct, 3 ) ) );
    }
    else
    {
        x =
            ( 0.237040 + 247.48 / cct +
              1.9018 * 1000000 / ( std::pow( cct, 2 ) ) -
              2.0064 * 1000000000 / ( std::pow( cct, 3 ) ) );
    }

    double y = -3.0 * ( std::pow( x, 2 ) ) + 2.87 * x - 0.275;

    return { x, y };
}


/// Calculate spectral power distribution (SPD) of CIE standard daylight illuminant.
/// The function generates the spectral power distribution for a daylight illuminant
/// based on the requested correlated color temperature using CIE standard formulas.
/// 
/// @param cct_input The correlated color temperature in Kelvin (40-250 or 4000-25000)
/// @param spectrum Reference to Spectrum object to fill with calculated SPD values
/// @pre cct_input is in valid range for daylight calculations
void calculate_daylight_SPD( const int &cct_input, Spectrum &spectrum )
{
    int step = spectrum.shape.step;
    int wavelength_range = s_series[53].wl - s_series[0].wl;
    assert( wavelength_range % step == 0 );

    double cct;
    if ( cct_input >= 40 && cct_input <= 250 )
        cct = cct_input * 100 * 1.4387752 / 1.438;
    else if ( cct_input >= 4000 && cct_input <= 25000 )
        cct = cct_input * 1.0;
    else
    {
        fprintf(
            stderr,
            "The range of Correlated Color Temperature for "
            "Day Light should be from 4000 to 25000. \n" );
        exit( 1 );
    }

    spectrum.values.clear();

    vector<int>    wavelengths, wavelengths_interpolated;
    vector<double> s00, s10, s20, s01, s11, s21;
    vector<double> xy = cct_to_XY( cct );

    double m0 = 0.0241 + 0.2562 * xy[0] - 0.7341 * xy[1];
    double m1 = ( -1.3515 - 1.7703 * xy[0] + 5.9114 * xy[1] ) / m0;
    double m2 = ( 0.03000 - 31.4424 * xy[0] + 30.0717 * xy[1] ) / m0;

    FORI( countSize( s_series ) )
    {
        wavelengths.push_back( s_series[i].wl );
        s00.push_back( s_series[i].RGB[0] );
        s10.push_back( s_series[i].RGB[1] );
        s20.push_back( s_series[i].RGB[2] );
    }

    int num_wavelengths = wavelength_range / step + 1;
    FORI( num_wavelengths )
    {
        wavelengths_interpolated.push_back( s_series[0].wl + step * i );
    }

    s01 = interp1DLinear( wavelengths, wavelengths_interpolated, s00 );
    clear_vector_memory( s00 );
    s11 = interp1DLinear( wavelengths, wavelengths_interpolated, s10 );
    clear_vector_memory( s10 );
    s21 = interp1DLinear( wavelengths, wavelengths_interpolated, s20 );
    clear_vector_memory( s20 );

    clear_vector_memory( wavelengths );
    clear_vector_memory( wavelengths_interpolated );

    FORI( num_wavelengths )
    {
        int wavelength = s_series[0].wl + step * i;
        if ( wavelength >= 380 && wavelength <= 780 )
        {
            spectrum.values.push_back( s01[i] + m1 * s11[i] + m2 * s21[i] );
        }
    }

    clear_vector_memory( s01 );
    clear_vector_memory( s11 );
    clear_vector_memory( s21 );
}

/// Calculate spectral power distribution (SPD) of blackbody radiation at given temperature.
/// Generates a blackbody curve using Planck's law for the specified correlated color temperature.
/// The function calculates spectral power distribution across visible wavelengths (380-780nm).
/// 
/// @param cct The correlated color temperature in Kelvin (1500-3999)
/// @param spectrum Reference to Spectrum object to fill with calculated SPD values
/// @pre cct is in valid range for blackbody calculations (1500-3999)
void calculate_blackbody_SPD( const int &cct, Spectrum &spectrum )
{
    if ( cct < 1500 || cct >= 4000 )
    {
        fprintf(
            stderr,
            "The range of Color Temperature for BlackBody "
            "should be from 1500 to 3999. \n" );
        exit( 1 );
    }

    spectrum.values.clear();

    for ( int wavelength = 380; wavelength <= 780; wavelength += 5 )
    {
        double lambda = wavelength / 1e9;
        double c1     = 2 * plancks_constant * ( std::pow( light_speed, 2 ) );
        double c2     = ( plancks_constant * light_speed ) / ( boltzmann_constant * lambda * cct );
        spectrum.values.push_back(
            c1 * pi / ( std::pow( lambda, 5 ) * ( std::exp( c2 ) - 1 ) ) );
    }
}

/// Generate illuminant spectral data based on type and temperature.
/// Creates spectral power distribution data for either daylight or blackbody illuminants
/// depending on the specified type and correlated color temperature. The function
/// automatically selects the appropriate calculation method (daylight vs blackbody).
/// 
/// @param cct The correlated color temperature in Kelvin
/// @param type Type of light source (e.g. "d50", "d65", "d75", "A", "B", "C", "D50", "D65", "D75")
/// @param is_daylight True if the light source is a daylight source, false if it is a blackbody source
/// @param illuminant Reference to SpectralData object to fill with generated illuminant data
/// @pre cct is in valid range for the specified illuminant type
void generate_illuminant(
    int               cct,
    const std::string type,
    bool              is_daylight,
    SpectralData     &illuminant )
{
    illuminant.data.clear();

    auto main_iter = illuminant.data.emplace( "main", SpectralData::SpectralSet() ).first;
    auto& main_spectral_set = main_iter->second;

    // Add the power channel and get a reference to it
    auto& power_spectrum = main_spectral_set.emplace_back(
        SpectralData::SpectralChannel( "power", Spectrum( 0 ) ) ).second;

    illuminant.illuminant = type;
    if ( is_daylight )
    {
        calculate_daylight_SPD( cct, power_spectrum );
    }
    else
    {
        calculate_blackbody_SPD( cct, power_spectrum );
    }
}

SpectralSolver::SpectralSolver()
{
    verbosity = 0;
    _IDT_matrix.resize( 3 );
    _WB_multipliers.resize( 3 );
    FORI( 3 )
    {
        _IDT_matrix[i].resize( 3 );
        _WB_multipliers[i]          = 1.0;
        FORJ( 3 )
        {
            _IDT_matrix[i][j] = neutral3[i][j];
        }
    }
}

/// Scale the illuminant (Light Source) to camera sensitivity data using the maximum RGB channel.
/// This function normalizes the illuminant spectral data by scaling it based on the camera's
/// most sensitive RGB channel. The scaling ensures proper integration between camera sensitivity
/// and illuminant data for accurate color calculations.
/// 
/// @param camera Camera sensitivity data containing RGB channel information
/// @param illuminant Light source data to be scaled (modified in-place)
/// @pre camera contains valid RGB channel data and illuminant contains power spectrum data
void scale_LSC( const SpectralData &camera, SpectralData &illuminant )
{
    double max_R = camera["R"].max();
    double max_G = camera["G"].max();
    double max_B = camera["B"].max();

    std::string max_channel;

    if ( max_R >= max_G && max_R >= max_B )
        max_channel = "R";
    else if ( max_G >= max_R && max_G >= max_B )
        max_channel = "G";
    else
        max_channel = "B";

    const Spectrum &camera_spectrum     = camera[max_channel];
    Spectrum       &illuminant_spectrum = illuminant["power"];

    double scale = 1.0 / ( camera_spectrum * illuminant_spectrum ).integrate();
    illuminant_spectrum *= scale;
}

bool is_not_equal_insensitive( const std::string &str1, const std::string &str2 )
{
    return cmp_str( str1.c_str(), str2.c_str() ) != 0;
}

//	=====================================================================
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
bool SpectralSolver::load_camera(
    const std::string &path, const std::string &make, const std::string &model )
{
    assert( !path.empty() );
    assert( !make.empty() );
    assert( !model.empty() );

    if ( !_camera.load( path ) )
        return false;
    if ( is_not_equal_insensitive( _camera.manufacturer, make ) )
        return false;
    if ( is_not_equal_insensitive( _camera.model, model ) )
        return false;

    return true;
}

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
bool SpectralSolver::load_illuminant(
    const std::vector<std::string> &paths, const std::string &type )
{
    if ( _illuminants.size() > 0 )
        _illuminants.clear();

    if ( !type.empty() )
    {
        bool is_daylight = std::tolower( type.front() ) == 'd';
        bool is_blackbody = std::tolower( type.back() ) == 'k';
        if ( is_daylight )
        {
            int               cct        = atoi( type.substr( 1 ).c_str() );
            const std::string type       = "d" + std::to_string( cct );
            SpectralData     &illuminant = _illuminants.emplace_back();
            generate_illuminant( cct, type, true, illuminant );
            return true;
        }
        else if ( is_blackbody )
        {
            int cct = atoi( type.substr( 0, type.length() - 1 ).c_str() );
            const std::string type       = std::to_string( cct ) + "k";
            SpectralData     &illuminant = _illuminants.emplace_back();
            generate_illuminant( cct, type, false, illuminant );
            return true;
        }
        else
        {
            FORI( paths.size() )
            {
                SpectralData &illuminant = _illuminants.emplace_back();
                bool is_invalid_illum = !illuminant.load( paths[i] ) || is_not_equal_insensitive( illuminant.illuminant, type );
                if ( is_invalid_illum )
                {
                    _illuminants.pop_back();
                }
                else
                {
                    return true;
                }
            }
        }
    }
    else
    {
        // Daylight - pre-calculate
        for ( int cct = 4000; cct <= 25000; cct += 500 )
        {
            SpectralData     &illuminant = _illuminants.emplace_back();
            const std::string type       = "d" + std::to_string( cct / 100 );
            generate_illuminant( cct, type, true, illuminant );
        }

        // Blackbody - pre-calculate
        for ( int cct = 1500; cct < 4000; cct += 500 )
        {
            SpectralData     &illuminant = _illuminants.emplace_back();
            const std::string type       = std::to_string( cct ) + "k";
            generate_illuminant( cct, type, false, illuminant );
        }

        FORI( paths.size() )
        {
            SpectralData &illuminant = _illuminants.emplace_back();
            bool is_invalid_illum = !illuminant.load( paths[i] ) || is_not_equal_insensitive( illuminant.illuminant, type );
            if ( is_invalid_illum )
            {
                _illuminants.pop_back();
            }
        }
    }

    return ( _illuminants.size() > 0 );
}

/// Load the 190-patch training data for spectral calculations.
/// This function loads training data from a file containing spectral information
/// for 190 color patches. The training data is used for calibrating and
/// optimizing spectral calculations in the color pipeline.
/// 
/// @param path Path to the 190-patch training data file
/// @return true if training data was successfully loaded, false otherwise
/// @pre path points to a valid training data file
bool SpectralSolver::load_training_data( const string &path )
{
    return _training_data.load( path );
}

/// Load the CIE 1931 Color Matching Functions data for standard observer.
/// This function loads the CIE 1931 2° standard observer color matching functions
/// from a file. These functions define how the human eye perceives color across
/// the visible spectrum and are essential for color space transformations.
/// 
/// @param path Path to the CIE 1931 Color Matching Functions data file
/// @return true if observer data was successfully loaded, false otherwise
/// @pre path points to a valid CIE 1931 CMF data file
bool SpectralSolver::load_observer( const string &path )
{
    return _observer.load( path );
}

/// Choose the best illuminant based on white balance coefficients from camera metadata.
/// This function analyzes all available illuminants and selects the one that best matches
/// the white balance coefficients read from the camera. It uses Sum of Squared Errors (SSE)
/// to find the optimal match and automatically scales the white balance multipliers.
/// 
/// @param src White balance coefficients from camera metadata
/// @param highlight Highlight recovery mode for normalization
void SpectralSolver::find_best_illuminant(
    const vector<double> &src, int highlight )
{
    // SSE: Sum of Squared Errors
    double sse = max_double_value;

    FORI( _illuminants.size() )
    {
        vector<double> wb_tmp  = calculate_WB( _camera, _illuminants[i], highlight );
        double         sse_tmp = calculate_SSE( wb_tmp, src );

        if ( sse_tmp < sse )
        {
            sse              = sse_tmp;
            _best_illuminant = _illuminants[i];
            _WB_multipliers  = wb_tmp;
        }
    }

    if ( verbosity > 1 )
    {
        printf(
            "The illuminant calculated to be the best match to the camera metadata is %s\n",
            _best_illuminant.illuminant.c_str() );
    }

    // scale back the WB factor
    double factor = _WB_multipliers[1];
    assert( factor != 0.0 );
    FORI( _WB_multipliers.size() ) _WB_multipliers[i] /= factor;
}

/// Select a specific illuminant by type and calculate white balance multipliers.
/// This function sets the best illuminant to a user-specified type and calculates
/// the corresponding white balance multipliers for the camera-illuminant combination.
/// The function automatically scales the white balance factors for normalization.
/// 
/// @param type The illuminant type to select (must match first illuminant in list)
/// @param highlight Highlight recovery mode for normalization
void SpectralSolver::select_illuminant( const std::string &type, int highlight )
{
    assert( type == _illuminants[0].illuminant );

    _best_illuminant = _illuminants[0];
    _WB_multipliers  = calculate_WB( _camera, _best_illuminant, highlight );

    // scale back the WB factor
    double factor = _WB_multipliers[1];
    assert( factor != 0.0 );
    FORI( _WB_multipliers.size() ) _WB_multipliers[i] /= factor;

    return;
}

/// Calculate the middle product based on camera sensitivity and illuminant data.
/// This function computes the spectral integration of camera RGB channels with
/// the illuminant power spectrum, then scales the result by the maximum value
/// to normalize the output vector.
/// 
/// @param camera Camera sensitivity data containing RGB spectral information
/// @param illuminant Illuminant data containing power spectrum information
/// @return Vector of scaled RGB values normalized by the maximum component
std::vector<double>
calculate_CM( const SpectralData &camera, const SpectralData &illuminant )
{
    const Spectrum &camera_r = camera["R"];
    const Spectrum &camera_g = camera["G"];
    const Spectrum &camera_b = camera["B"];
    const Spectrum &illuminant_spectrum = illuminant["power"];

    double r = ( camera_r * illuminant_spectrum ).integrate();
    double g = ( camera_g * illuminant_spectrum ).integrate();
    double b = ( camera_b * illuminant_spectrum ).integrate();

    double max = std::max( r, std::max( g, b ) );

    std::vector<double> result( 3 );
    result[0] = max / r;
    result[1] = max / g;
    result[2] = max / b;
    return result;
}

/// Calculate the middle product based on training data and illuminant data.
/// This function computes spectral transformations using the 190-patch training data
/// and illuminant information. The result is a 2D vector representing spectral
/// transformations across the training patches under the specified illuminant.
/// 
/// @param illuminant Illuminant data containing power spectrum information
/// @param training_data 190-patch training data for spectral calculations
/// @return Vector of 190 spectra, each containing 81 wavelength samples
std::vector<Spectrum>
calculate_TI( const SpectralData &illuminant, const SpectralData &training_data )
{
    std::vector<Spectrum> result;

    const Spectrum &illuminant_spectrum = illuminant["power"];
    for ( auto &[name, training_spectrum]: training_data.data.at( "main" ) )
    {
        result.push_back( training_spectrum * illuminant_spectrum );
    }

    return result;
}

//	=====================================================================
//	Calculate White Balance based on the Illuminant data and
//  highlight mode used in pre-processing with "libraw"
//
//	inputs:
//      Illum: Illuminant
//      int: highlight
//
//	outputs:
//		vector: wb(R, G, B)

std::vector<double>
calculate_WB( const SpectralData &camera, SpectralData &illuminant, int highlight )
{
    scale_LSC( camera, illuminant );

    const Spectrum &camera_r = camera["R"];
    const Spectrum &camera_g = camera["G"];
    const Spectrum &camera_b = camera["B"];
    const Spectrum &illuminant_spectrum    = illuminant["power"];

    double r = ( camera_r * illuminant_spectrum ).integrate();
    double g = ( camera_g * illuminant_spectrum ).integrate();
    double b = ( camera_b * illuminant_spectrum ).integrate();

    std::vector<double> wb = { 1.0 / r, 1.0 / g, 1.0 / b };

    if ( highlight )
    {
        scale_vector_max( wb );
    }
    else
    {
        scale_vector_min( wb );
    }

    return wb;
}

/// Calculate CIE XYZ tristimulus values from training illuminant data.
/// This function computes XYZ tristimulus values for each training patch based on
/// the training illuminant data (TI) and applies color adaptation transformation.
/// The result represents how the training patches appear in CIE XYZ color space
/// under the specified illuminant conditions.
/// 
/// @param observer CIE 1931 color matching functions (X, Y, Z)
/// @param illuminant Illuminant data containing power spectrum information
/// @param training_illuminants Training patches transformed by illuminant (from calculate_TI)
/// @return 2D vector (190 x 3) containing XYZ values for each training patch
std::vector<std::vector<double>> calculate_XYZ(
    const SpectralData          &observer,
    const SpectralData          &illuminant,
    const std::vector<Spectrum> &training_illuminants )
{
    assert( training_illuminants.size() > 0 );
    assert( training_illuminants[0].values.size() == 81 );

    std::vector<double>              reference_white_point( XYZ_white_point, XYZ_white_point + 3 );
    std::vector<std::vector<double>> XYZ;

    const Spectrum &cmf_x = observer["X"];
    const Spectrum &cmf_y = observer["Y"];
    const Spectrum &cmf_z = observer["Z"];
    const Spectrum &illuminant_spectrum = illuminant["power"];

    double scale = 1.0 / ( cmf_y * illuminant_spectrum ).integrate();

    for ( auto &training_illuminant: training_illuminants )
    {
        auto &xyz = XYZ.emplace_back( 3 );
        xyz[0]    = ( training_illuminant * cmf_x ).integrate() * scale;
        xyz[1]    = ( training_illuminant * cmf_y ).integrate() * scale;
        xyz[2]    = ( training_illuminant * cmf_z ).integrate() * scale;
    }

    std::vector<double> source_white_point( 3 );
    double              y = ( cmf_y * illuminant_spectrum ).integrate();
    source_white_point[0] = ( cmf_x * illuminant_spectrum ).integrate() / y;
    source_white_point[1] = 1.0;
    source_white_point[2] = ( cmf_z * illuminant_spectrum ).integrate() / y;

    XYZ = mulVector( XYZ, calculate_CAT( source_white_point, reference_white_point ) );

    return XYZ;
}

/// Calculate white-balanced linearized camera RGB responses from training illuminant data.
/// This function computes RGB camera responses for each training patch under the specified
/// illuminant, applying white balance multipliers to normalize the responses. The result
/// represents how the camera would record each training patch in RGB color space.
/// 
/// @param camera Camera sensitivity data containing RGB spectral information
/// @param illuminant Illuminant data containing power spectrum information
/// @param WB_multipliers White balance multipliers from calculate_WB function
/// @param training_illuminants Training patches transformed by illuminant (from calculate_TI)
/// @return 2D vector (190 x 3) containing RGB values for each training patch
std::vector<std::vector<double>> calculate_RGB(
    const SpectralData          &camera,
    const SpectralData          &illuminant,
    const std::vector<double>   &WB_multipliers,
    const std::vector<Spectrum> &training_illuminants )
{
    assert( training_illuminants.size() > 0 );
    assert( training_illuminants[0].values.size() == 81 );

    const Spectrum &camera_r = camera["R"];
    const Spectrum &camera_g = camera["G"];
    const Spectrum &camera_b = camera["B"];
    const Spectrum &illuminant_spectrum = illuminant["power"];

    std::vector<std::vector<double>> RGB;
    for ( auto &training_illuminant: training_illuminants )
    {
        auto &rgb = RGB.emplace_back( 3 );
        rgb[0]    = ( training_illuminant * camera_r ).integrate() * WB_multipliers[0];
        rgb[1]    = ( training_illuminant * camera_g ).integrate() * WB_multipliers[1];
        rgb[2]    = ( training_illuminant * camera_b ).integrate() * WB_multipliers[2];
    }

    return RGB;
}

struct Objfun
{
    Objfun(
        const std::vector<std::vector<double>> &RGB,
        const std::vector<std::vector<double>> &outLAB )
        : _RGB( RGB ), _outLAB( outLAB )
    {}

    template <typename T> bool operator()( const T *B, T *residuals ) const;

    const std::vector<std::vector<double>> _RGB;
    const std::vector<std::vector<double>> _outLAB;
};

//	=====================================================================
//	Process curve fit between XYZ and RGB data with initial set of B
//  values.
//
//	inputs:
//		vector< vector<double> >: RGB
//      vector< vector<double> >: XYZ
//      double * :                B (6 elements)
//
//	outputs:
//      boolean: if succeed, _idt should be filled with values
//               that minimize the distance between RGB and XYZ
//               through updated B.

bool curveFit(
    const std::vector<std::vector<double>> &RGB,
    const std::vector<std::vector<double>> &XYZ,
    double                                 *B,
    int                                     verbosity,
    std::vector<std::vector<double>>       &out_IDT_matrix )
{
    Problem                problem;
    vector<vector<double>> outLAB = XYZtoLAB( XYZ );

    CostFunction *cost_function =
        new AutoDiffCostFunction<Objfun, ceres::DYNAMIC, 6>(
            new Objfun( RGB, outLAB ), int( RGB.size() * ( RGB[0].size() ) ) );

    problem.AddResidualBlock( cost_function, NULL, B );

    ceres::Solver::Options options;
    options.linear_solver_type  = ceres::DENSE_QR;
    options.parameter_tolerance = 1e-17;
    //        options.gradient_tolerance = 1e-17;
    options.function_tolerance        = 1e-17;
    options.min_line_search_step_size = 1e-17;
    options.max_num_iterations        = 300;

    if ( verbosity > 2 )
        options.minimizer_progress_to_stdout = true;

    ceres::Solver::Summary summary;
    ceres::Solve( options, &problem, &summary );

    if ( verbosity > 1 )
        std::cout << summary.BriefReport() << std::endl;
    else if ( verbosity >= 2 )
        std::cout << summary.FullReport() << std::endl;

    if ( summary.num_successful_steps )
    {
        out_IDT_matrix[0][0] = B[0];
        out_IDT_matrix[0][1] = B[1];
        out_IDT_matrix[0][2] = 1.0 - B[0] - B[1];
        out_IDT_matrix[1][0] = B[2];
        out_IDT_matrix[1][1] = B[3];
        out_IDT_matrix[1][2] = 1.0 - B[2] - B[3];
        out_IDT_matrix[2][0] = B[4];
        out_IDT_matrix[2][1] = B[5];
        out_IDT_matrix[2][2] = 1.0 - B[4] - B[5];

        if ( verbosity > 1 )
        {
            printf( "The IDT matrix is ...\n" );
            FORI( 3 )
            printf(
                "   %f %f %f\n",
                out_IDT_matrix[i][0],
                out_IDT_matrix[i][1],
                out_IDT_matrix[i][2] );
        }

        return true;
    }

    delete cost_function;

    return false;
}

//	=====================================================================
//	Calculate IDT matrix by calling curveFit(...)
//
//	inputs:
//         N/A
//
//	outputs: through curveFit(...)
//      boolean: if succeed, _idt should be filled with values
//               that minimize the distance between RGB and XYZ
//               through updated B.

bool SpectralSolver::calculate_IDT_matrix()
{
    double BStart[6] = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };

    auto TI  = calculate_TI( _best_illuminant, _training_data );
    auto RGB = calculate_RGB( _camera, _best_illuminant, _WB_multipliers, TI );
    auto XYZ = calculate_XYZ( _observer, _best_illuminant, TI );

    return curveFit( RGB, XYZ, BStart, verbosity, _IDT_matrix );
}

//	=====================================================================
//  Get the Best Illuminant data / light source that was loaded from
//  the file
//
//	inputs:
//         N/A
//
//	outputs:
//      const SpectralData: Illuminant data that has the closest match

const SpectralData &SpectralSolver::get_best_illuminant() const
{
    assert( _best_illuminant.data.count( "main" ) == 1 );
    assert( _best_illuminant.data.at( "main" ).size() == 1 );
    assert( _best_illuminant["power"].values.size() > 0 );

    return _best_illuminant;
}

//	=====================================================================
//  Get Idt matrix if CalIDT() succeeds
//
//	inputs:
//         N/A
//
//	outputs:
//      const vector< vector < double > >: _idt matrix (3 x 3)

const vector<vector<double>> &SpectralSolver::get_IDT_matrix() const
{
    return _IDT_matrix;
}

//	=====================================================================
//  Get white balanced if calWB(...) succeeds
//
//	inputs:
//         N/A
//
//	outputs:
//      const vector< double >: _wb vector (1 x 3)

const vector<double> &SpectralSolver::get_WB_multipliers() const
{
    return _WB_multipliers;
}

// ------------------------------------------------------//

MetadataSolver::MetadataSolver( const core::Metadata &metadata )
    : _metadata( metadata )
{}

double ccttoMired( const double cct )
{
    return 1.0E06 / cct;
}

double robertsonLength( const vector<double> &uv, const vector<double> &uvt )
{

    double         t    = uvt[2];
    double         sign = t < 0 ? -1.0 : t > 0 ? 1.0 : 0.0;
    vector<double> slope( 2 );
    slope[0] = -sign / std::sqrt( 1 + t * t );
    slope[1] = t * slope[0];

    vector<double> uvr( uvt.begin(), uvt.begin() + 2 );
    return cross2( slope, subVectors( uv, uvr ) );
}

double lightSourceToColorTemp( const unsigned short tag )
{

    if ( tag >= 32768 )
        return ( static_cast<double>( tag ) ) - 32768.0;

    uint16_t LightSourceEXIFTagValues[][2] = {
        { 0, 5500 },  { 1, 5500 },  { 2, 3500 },  { 3, 3400 },
        { 10, 5550 }, { 17, 2856 }, { 18, 4874 }, { 19, 6774 },
        { 20, 5500 }, { 21, 6500 }, { 22, 7500 }
    };

    FORI( countSize( LightSourceEXIFTagValues ) )
    {
        if ( LightSourceEXIFTagValues[i][0] == static_cast<uint16_t>( tag ) )
        {
            return ( static_cast<double>( LightSourceEXIFTagValues[i][1] ) );
        }
    }

    return 5500.0;
}

double XYZToColorTemperature( const vector<double> &XYZ )
{
    vector<double> uv      = XYZTouv( XYZ );
    int            Nrobert = countSize( Robertson_uvtTable );
    int            i;

    double mired;
    double RDthis = 0.0, RDprevious = 0.0;

    for ( i = 0; i < Nrobert; i++ )
    {
        vector<double> robertson(
            Robertson_uvtTable[i],
            Robertson_uvtTable[i] + countSize( Robertson_uvtTable[i] ) );
        if ( ( RDthis = robertsonLength( uv, robertson ) ) <= 0.0 )
            break;
        RDprevious = RDthis;
    }
    if ( i <= 0 )
        mired = RobertsonMired[0];
    else if ( i >= Nrobert )
        mired = RobertsonMired[Nrobert - 1];
    else
        mired = RobertsonMired[i - 1] +
                RDprevious * ( RobertsonMired[i] - RobertsonMired[i - 1] ) /
                    ( RDprevious - RDthis );

    double cct = 1.0e06 / mired;
    cct        = std::max( 2000.0, std::min( 50000.0, cct ) );

    return cct;
}

vector<double> XYZtoCameraWeightedMatrix(
    const double              &mired0,
    const double              &mired1,
    const double              &mired2,
    const std::vector<double> &matrix1,
    const std::vector<double> &matrix2 )
{

    double weight = std::max(
        0.0, std::min( 1.0, ( mired1 - mired0 ) / ( mired1 - mired2 ) ) );
    vector<double> result = subVectors( matrix2, matrix1 );
    scaleVector( result, weight );
    result = addVectors( result, matrix1 );

    return result;
}

vector<double>
findXYZtoCameraMtx( const Metadata &metadata, const vector<double> &neutralRGB )
{

    if ( metadata.calibration[0].illuminant == 0 )
    {
        fprintf( stderr, " No calibration illuminants were found. \n " );
        return metadata.calibration[0].XYZ_to_RGB_matrix;
    }

    if ( neutralRGB.size() == 0 )
    {
        fprintf( stderr, " no neutral RGB values were found. \n " );
        return metadata.calibration[0].XYZ_to_RGB_matrix;
    }

    double cct1 = lightSourceToColorTemp( metadata.calibration[0].illuminant );
    double cct2 = lightSourceToColorTemp( metadata.calibration[1].illuminant );

    double mir1 = ccttoMired( cct1 );
    double mir2 = ccttoMired( cct2 );

    double maxMir = ccttoMired( 2000.0 );
    double minMir = ccttoMired( 50000.0 );

    const std::vector<double> &matrix1 =
        metadata.calibration[0].XYZ_to_RGB_matrix;
    const std::vector<double> &matrix2 =
        metadata.calibration[1].XYZ_to_RGB_matrix;

    double lomir =
        std::max( minMir, std::min( maxMir, std::min( mir1, mir2 ) ) );
    double himir =
        std::max( minMir, std::min( maxMir, std::max( mir1, mir2 ) ) );
    double mirStep = std::max( 5.0, ( himir - lomir ) / 50.0 );

    double mir = 0.0, lastMired = 0.0, estimatedMired = 0.0, lerror = 0.0,
           lastError = 0.0, smallestError = 0.0;

    for ( mir = lomir; mir < himir; mir += mirStep )
    {
        lerror = mir - ccttoMired( XYZToColorTemperature( mulVector(
                           invertV( XYZtoCameraWeightedMatrix(
                               mir, mir1, mir2, matrix1, matrix2 ) ),
                           neutralRGB ) ) );

        if ( std::fabs( lerror - 0.0 ) <= 1e-09 )
        {
            estimatedMired = mir;
            break;
        }
        if ( std::fabs( mir - lomir - 0.0 ) > 1e-09 &&
             lerror * lastError <= 0.0 )
        {
            estimatedMired =
                mir + ( lerror / ( lerror - lastError ) * ( mir - lastMired ) );
            break;
        }
        if ( std::fabs( mir - lomir ) <= 1e-09 ||
             std::fabs( lerror ) < std::fabs( smallestError ) )
        {
            estimatedMired = mir;
            smallestError  = lerror;
        }

        lastError = lerror;
        lastMired = mir;
    }

    return XYZtoCameraWeightedMatrix(
        estimatedMired, mir1, mir2, matrix1, matrix2 );
}

vector<double> colorTemperatureToXYZ( const double &cct )
{

    double         mired = 1.0e06 / cct;
    vector<double> uv( 2, 1.0 );

    int Nrobert = countSize( Robertson_uvtTable );
    int i;

    for ( i = 0; i < Nrobert; i++ )
    {
        if ( RobertsonMired[i] >= mired )
            break;
    }

    if ( i <= 0 )
    {
        uv = vector<double>( Robertson_uvtTable[0], Robertson_uvtTable[0] + 2 );
    }
    else if ( i >= Nrobert )
    {
        uv = vector<double>(
            Robertson_uvtTable[Nrobert - 1],
            Robertson_uvtTable[Nrobert - 1] + 2 );
    }
    else
    {
        double weight = ( mired - RobertsonMired[i - 1] ) /
                        ( RobertsonMired[i] - RobertsonMired[i - 1] );

        vector<double> uv1( Robertson_uvtTable[i], Robertson_uvtTable[i] + 2 );
        scaleVector( uv1, weight );

        vector<double> uv2(
            Robertson_uvtTable[i - 1], Robertson_uvtTable[i - 1] + 2 );
        scaleVector( uv2, 1.0 - weight );

        uv = addVectors( uv1, uv2 );
    }

    return uvToXYZ( uv );
}

vector<double> matrixRGBtoXYZ( const double chromaticities[][2] )
{
    vector<double> rXYZ =
        xyToXYZ( vector<double>( chromaticities[0], chromaticities[0] + 2 ) );
    vector<double> gXYZ =
        xyToXYZ( vector<double>( chromaticities[1], chromaticities[1] + 2 ) );
    vector<double> bXYZ =
        xyToXYZ( vector<double>( chromaticities[2], chromaticities[2] + 2 ) );
    vector<double> wXYZ =
        xyToXYZ( vector<double>( chromaticities[3], chromaticities[3] + 2 ) );

    vector<double> rgbMtx( 9 );
    FORI( 3 )
    {
        rgbMtx[0 + i * 3] = rXYZ[i];
        rgbMtx[1 + i * 3] = gXYZ[i];
        rgbMtx[2 + i * 3] = bXYZ[i];
    }

    scaleVector( wXYZ, 1.0 / wXYZ[1] );

    vector<double> channelgains = mulVector( invertV( rgbMtx ), wXYZ, 3 );
    vector<double> colorMatrix  = mulVector( rgbMtx, diagV( channelgains ), 3 );

    return colorMatrix;
}

void getCameraXYZMtxAndWhitePoint(
    const Metadata      &metadata,
    std::vector<double> &out_camera_to_XYZ_matrix,
    std::vector<double> &out_camera_XYZ_white_point )
{
    out_camera_to_XYZ_matrix =
        invertV( findXYZtoCameraMtx( metadata, metadata.neutral_RGB ) );
    assert( std::fabs( sumVector( out_camera_to_XYZ_matrix ) - 0.0 ) > 1e-09 );

    scaleVector(
        out_camera_to_XYZ_matrix, std::pow( 2.0, metadata.baseline_exposure ) );

    if ( metadata.neutral_RGB.size() > 0 )
    {
        out_camera_XYZ_white_point =
            mulVector( out_camera_to_XYZ_matrix, metadata.neutral_RGB );
    }
    else
    {
        out_camera_XYZ_white_point = colorTemperatureToXYZ(
            lightSourceToColorTemp( metadata.calibration[0].illuminant ) );
    }

    scaleVector(
        out_camera_XYZ_white_point, 1.0 / out_camera_XYZ_white_point[1] );
    assert( sumVector( out_camera_XYZ_white_point ) != 0 );

    return;
}

vector<vector<double>> MetadataSolver::calculate_CAT_matrix()
{
    vector<double>      deviceWhiteV( 3, 1.0 );
    std::vector<double> camera_to_XYZ_matrix;
    std::vector<double> camera_XYZ_white_point;
    getCameraXYZMtxAndWhitePoint(
        _metadata, camera_to_XYZ_matrix, camera_XYZ_white_point );
    vector<double> outputRGBtoXYZMtx = matrixRGBtoXYZ( chromaticitiesACES );
    vector<double> outputXYZWhitePoint =
        mulVector( outputRGBtoXYZMtx, deviceWhiteV );
    vector<vector<double>> chadMtx =
        calculate_CAT( camera_XYZ_white_point, outputXYZWhitePoint );

    return chadMtx;
}

vector<vector<double>> MetadataSolver::calculate_IDT_matrix()
{
    vector<vector<double>> chadMtx = calculate_CAT_matrix();
    vector<double>         XYZ_D65_acesrgb( 9 ), CAT( 9 );
    FORIJ( 3, 3 )
    {
        XYZ_D65_acesrgb[i * 3 + j] = XYZ_D65_acesrgb_3[i][j];
        CAT[i * 3 + j]             = chadMtx[i][j];
    }

    vector<double>         matrix = mulVector( XYZ_D65_acesrgb, CAT, 3 );
    vector<vector<double>> DNGIDTMatrix( 3, vector<double>( 3 ) );
    FORIJ( 3, 3 ) DNGIDTMatrix[i][j] = matrix[i * 3 + j];

    //        vector < double > outRGBWhite = mulVector ( DNGIDTMatrix,
    //                                                    mulVector ( invertV ( _cameraToXYZMtx ),
    //                                                                _cameraXYZWhitePoint ) );

    //        double max_value = *std::max_element ( outRGBWhite.begin(), outRGBWhite.end() );
    //        scaleVector ( outRGBWhite, 1.0 / max_value );
    //        vector < double > absdif = subVectors ( outRGBWhite, deviceWhiteV );
    //
    //        FORI ( absdif.size() ) absdif[i] = std::fabs ( absdif[i] );
    //        max_value = *std::max_element ( absdif.begin(), absdif.end() );
    //
    //        if ( max_value >= 0.0001 )
    //            fprintf(stderr, "WARNING: The neutrals should come out white balanced.\n");

    assert( std::fabs( sumVectorM( DNGIDTMatrix ) - 0.0 ) > 1e-09 );

    return DNGIDTMatrix;
}

template <typename T> bool Objfun::operator()( const T *B, T *residuals ) const
{
    vector<vector<T>> RGBJet( 190, vector<T>( 3 ) );
    FORIJ( 190, 3 ) RGBJet[i][j] = T( _RGB[i][j] );

    vector<vector<T>> outCalcLAB         = XYZtoLAB( getCalcXYZt( RGBJet, B ) );
    FORIJ( 190, 3 ) residuals[i * 3 + j] = _outLAB[i][j] - outCalcLAB[i][j];

    return true;
}

} // namespace core
} // namespace rta
