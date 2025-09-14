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
vector<double> CCT_to_xy( const double &cct )
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

void calculate_daylight_SPD( const int &cct_input, Spectrum &spectrum )
{
    int step             = static_cast<int>( spectrum.shape.step );
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
    vector<double> xy = CCT_to_xy( cct );

    double m0 = 0.0241 + 0.2562 * xy[0] - 0.7341 * xy[1];
    double m1 = ( -1.3515 - 1.7703 * xy[0] + 5.9114 * xy[1] ) / m0;
    double m2 = ( 0.03000 - 31.4424 * xy[0] + 30.0717 * xy[1] ) / m0;

    for ( int i = 0; i < countSize( s_series ); i++ )
    {
        wavelengths.push_back( s_series[i].wl );
        s00.push_back( s_series[i].RGB[0] );
        s10.push_back( s_series[i].RGB[1] );
        s20.push_back( s_series[i].RGB[2] );
    }

    int num_wavelengths = wavelength_range / step + 1;
    for ( int i = 0; i < num_wavelengths; i++ )
    {
        wavelengths_interpolated.push_back( s_series[0].wl + step * i );
    }

    s01 = interp1DLinear( wavelengths, wavelengths_interpolated, s00 );
    s11 = interp1DLinear( wavelengths, wavelengths_interpolated, s10 );
    s21 = interp1DLinear( wavelengths, wavelengths_interpolated, s20 );

    for ( int i = 0; i < num_wavelengths; i++ )
    {
        int wavelength = s_series[0].wl + step * i;
        if ( wavelength >= 380 && wavelength <= 780 )
        {
            spectrum.values.push_back( s01[i] + m1 * s11[i] + m2 * s21[i] );
        }
    }
}

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
        double c2     = ( plancks_constant * light_speed ) /
                    ( boltzmann_constant * lambda * cct );
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
    int                cct,
    const std::string &type,
    bool               is_daylight,
    SpectralData      &illuminant )
{
    illuminant.data.clear();

    auto main_iter =
        illuminant.data.emplace( "main", SpectralData::SpectralSet() ).first;
    auto &main_spectral_set = main_iter->second;

    // Add the power channel and get a reference to it
    auto &power_spectrum = main_spectral_set
                               .emplace_back( SpectralData::SpectralChannel(
                                   "power", Spectrum( 0 ) ) )
                               .second;

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

SpectralSolver::SpectralSolver(
    const std::vector<std::string> &search_directories )
    : _search_directories( search_directories )
{
    verbosity = 0;
    _IDT_matrix.resize( 3 );
    _WB_multipliers.resize( 3 );
    for ( int i = 0; i < 3; i++ )
    {
        _IDT_matrix[i].resize( 3 );
        _WB_multipliers[i] = 1.0;
        for ( size_t j = 0; j < 3; j++ )
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
void scale_illuminant( const SpectralData &camera, SpectralData &illuminant )
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

/// Check if two strings are not equal, ignoring case differences.
/// @param str1 First string to compare
/// @param str2 Second string to compare
/// @return true if strings are different (case-insensitive), false if they match
bool is_not_equal_insensitive(
    const std::string &str1, const std::string &str2 )
{
    return cmp_str( str1.c_str(), str2.c_str() ) != 0;
}

std::vector<std::string>
SpectralSolver::collect_data_files( const std::string &type ) const
{
    std::vector<std::string> result;

    for ( const auto &directory: _search_directories )
    {
        if ( std::filesystem::is_directory( directory ) )
        {
            std::filesystem::path type_path( directory );
            type_path.append( type );
            if ( std::filesystem::exists( type_path ) )
            {
                auto it = std::filesystem::directory_iterator( type_path );
                for ( auto filename: it )
                {
                    auto path = filename.path();
                    if ( path.extension() == ".json" )
                    {
                        result.push_back( path.string() );
                    }
                }
            }
            else if ( verbosity > 0 )
            {
                std::cerr << "WARNING: Directory '" << type_path.string()
                          << "' does not exist." << std::endl;
            }
        }
        else if ( verbosity > 0 )
        {
            std::cerr << "WARNING: Database location '" << directory
                      << "' is not a directory." << std::endl;
        }
    }
    return result;
}

bool SpectralSolver::load_spectral_data(
    const std::string &file_path, SpectralData &out_data )
{
    std::filesystem::path path( file_path );

    if ( path.is_absolute() )
    {
        return out_data.load( file_path );
    }
    else
    {
        for ( const auto &directory: _search_directories )
        {
            std::filesystem::path search_path( directory );
            search_path.append( file_path );

            if ( std::filesystem::exists( search_path ) )
            {
                return out_data.load( search_path.string() );
            }
        }

        return false;
    }
}

bool SpectralSolver::find_camera(
    const std::string &make, const std::string &model )
{
    assert( !make.empty() );
    assert( !model.empty() );

    auto camera_files = collect_data_files( "camera" );

    for ( const auto &camera_file: camera_files )
    {
        camera.load( camera_file );

        if ( is_not_equal_insensitive( camera.manufacturer, make ) )
            continue;
        if ( is_not_equal_insensitive( camera.model, model ) )
            continue;
        return true;
    }
    return false;
}

bool SpectralSolver::find_illuminant( const std::string &type )
{
    assert( !type.empty() );

    bool starts_with_d = std::tolower( type.front() ) == 'd';
    bool ends_with_k   = std::tolower( type.back() ) == 'k';

    // daylight ("D" + Numeric values)
    bool is_daylight = starts_with_d && !ends_with_k;
    // blackbody (Numeric values + "K")
    bool is_blackbody = !starts_with_d && ends_with_k;

    if ( is_daylight )
    {
        int               cct             = atoi( type.substr( 1 ).c_str() );
        const std::string illuminant_type = "d" + std::to_string( cct );
        generate_illuminant( cct, illuminant_type, true, illuminant );
        return true;
    }
    else if ( is_blackbody )
    {
        int cct = atoi( type.substr( 0, type.length() - 1 ).c_str() );
        const std::string illuminant_type = std::to_string( cct ) + "k";
        generate_illuminant( cct, illuminant_type, false, illuminant );
        return true;
    }
    else
    {
        auto illuminant_files = collect_data_files( "illuminant" );

        for ( const auto &illuminant_file: illuminant_files )
        {
            if ( !illuminant.load( illuminant_file ) )
                continue;
            if ( is_not_equal_insensitive( illuminant.illuminant, type ) )
                continue;
            return true;
        }
    }

    return false;
}

bool SpectralSolver::find_illuminant( const vector<double> &wb )
{
    if ( camera.data.count( "main" ) == 0 ||
         camera.data.at( "main" ).size() != 3 )
    {
        std::cerr << "ERROR: camera needs to be initialised prior to calling "
                  << "SpectralSolver::find_illuminant()" << std::endl;
        return false;
    }

    if ( _all_illuminants.empty() )
    {
        // Daylight - pre-calculate
        for ( int cct = 4000; cct <= 25000; cct += 500 )
        {
            SpectralData     &illuminant_data = _all_illuminants.emplace_back();
            const std::string type = "d" + std::to_string( cct / 100 );
            generate_illuminant( cct, type, true, illuminant_data );
        }

        // Blackbody - pre-calculate
        for ( int cct = 1500; cct < 4000; cct += 500 )
        {
            SpectralData     &illuminant_data = _all_illuminants.emplace_back();
            const std::string type            = std::to_string( cct ) + "k";
            generate_illuminant( cct, type, false, illuminant_data );
        }

        auto illuminant_files = collect_data_files( "illuminant" );

        for ( const auto &illuminant_file: illuminant_files )
        {
            SpectralData &illuminant_data = _all_illuminants.emplace_back();
            if ( !illuminant_data.load( illuminant_file ) )
            {
                _all_illuminants.pop_back();
                continue;
            }
        }
    }

    // SSE: Sum of Squared Errors
    double sse = max_double_value;

    for ( auto &current_illuminant: _all_illuminants )
    {
        vector<double> wb_tmp  = _calculate_WB( camera, current_illuminant );
        double         sse_tmp = calculate_SSE( wb_tmp, wb );

        if ( sse_tmp < sse )
        {
            sse             = sse_tmp;
            illuminant      = current_illuminant;
            _WB_multipliers = wb_tmp;
        }
    }

    if ( verbosity > 1 )
        std::cerr << "The illuminant calculated to be the best match to the "
                  << "camera metadata is '" << illuminant.illuminant << "'."
                  << std::endl;

    return true;
}

bool SpectralSolver::calculate_WB()
{
    if ( camera.data.count( "main" ) == 0 ||
         camera.data.at( "main" ).size() != 3 )
    {
        std::cerr << "ERROR: camera needs to be initialised prior to calling "
                  << "SpectralSolver::calculate_WB()" << std::endl;
    }

    if ( illuminant.data.count( "main" ) == 0 ||
         illuminant.data.at( "main" ).size() != 1 )
    {
        std::cerr << "ERROR: illuminant needs to be initialised prior to "
                  << "calling SpectralSolver::calculate_WB()" << std::endl;
        return false;
    }

    _WB_multipliers = _calculate_WB( camera, illuminant );
    return true;
}

/// Calculate the middle product based on camera sensitivity and illuminant data.
/// This function computes the spectral integration of camera RGB channels with
/// the illuminant power spectrum, then scales the result by the maximum value
/// to normalize the output vector.
///
/// @param camera Camera sensitivity data containing RGB spectral information
/// @param illuminant Illuminant data containing power spectrum information
/// @return Vector of reciprocal RGB values scaled by the maximum component
std::vector<double>
calculate_CM( const SpectralData &camera, const SpectralData &illuminant )
{
    const Spectrum &camera_r            = camera["R"];
    const Spectrum &camera_g            = camera["G"];
    const Spectrum &camera_b            = camera["B"];
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
/// This function computes spectral transformations using the training data
/// and illuminant information. The result is a 2D vector representing spectral
/// transformations across the training patches under the specified illuminant.
///
/// @param illuminant Illuminant data containing power spectrum information
/// @param training_data Training data for spectral calculations
/// @return Vector of spectra, each containing wavelength samples
std::vector<Spectrum> calculate_TI(
    const SpectralData &illuminant, const SpectralData &training_data )
{
    std::vector<Spectrum> result;

    const Spectrum &illuminant_spectrum = illuminant["power"];
    for ( auto &[name, training_spectrum]: training_data.data.at( "main" ) )
    {
        result.push_back( training_spectrum * illuminant_spectrum );
    }

    return result;
}

/// Calculate white balance multipliers based on camera sensitivity and illuminant data.
/// This function computes RGB white balance multipliers by integrating camera spectral
/// sensitivity with illuminant power spectrum. The multipliers normalize the camera
/// response to achieve proper white balance under the specified illuminant conditions.
/// The function scales the illuminant to camera sensitivity data and normalizes to the green channel.
///
/// @param camera Camera sensitivity data containing RGB spectral information
/// @param illuminant Illuminant data (modified in-place by scale_illuminant)

/// @return Vector of 3 white balance multipliers [R, G, B] normalized to green channel
std::vector<double>
_calculate_WB( const SpectralData &camera, SpectralData &illuminant )
{
    scale_illuminant( camera, illuminant );

    const Spectrum &camera_r            = camera["R"];
    const Spectrum &camera_g            = camera["G"];
    const Spectrum &camera_b            = camera["B"];
    const Spectrum &illuminant_spectrum = illuminant["power"];

    double r = ( camera_r * illuminant_spectrum ).integrate();
    double g = ( camera_g * illuminant_spectrum ).integrate();
    double b = ( camera_b * illuminant_spectrum ).integrate();

    // Normalise to the green channel.
    std::vector<double> wb = { g / r, 1.0, g / b };
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
/// @return 2D vector containing XYZ values for each training patch
std::vector<std::vector<double>> calculate_XYZ(
    const SpectralData          &observer,
    const SpectralData          &illuminant,
    const std::vector<Spectrum> &training_illuminants )
{
    assert( training_illuminants.size() > 0 );
    assert( training_illuminants[0].values.size() == 81 );

    std::vector<double> reference_white_point(
        ACES_white_point_XYZ, ACES_white_point_XYZ + 3 );
    std::vector<std::vector<double>> XYZ;

    const Spectrum &observer_x          = observer["X"];
    const Spectrum &observer_y          = observer["Y"];
    const Spectrum &observer_z          = observer["Z"];
    const Spectrum &illuminant_spectrum = illuminant["power"];

    double scale = 1.0 / ( observer_y * illuminant_spectrum ).integrate();

    for ( auto &training_illuminant: training_illuminants )
    {
        auto &xyz = XYZ.emplace_back( 3 );
        xyz[0]    = ( training_illuminant * observer_x ).integrate() * scale;
        xyz[1]    = ( training_illuminant * observer_y ).integrate() * scale;
        xyz[2]    = ( training_illuminant * observer_z ).integrate() * scale;
    }

    std::vector<double> source_white_point( 3 );
    double              y = ( observer_y * illuminant_spectrum ).integrate();
    source_white_point[0] =
        ( observer_x * illuminant_spectrum ).integrate() / y;
    source_white_point[1] = 1.0;
    source_white_point[2] =
        ( observer_z * illuminant_spectrum ).integrate() / y;

    XYZ = mulVector(
        XYZ, calculate_CAT( source_white_point, reference_white_point ) );

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
/// @return 2D vector containing RGB values for each training patch
std::vector<std::vector<double>> calculate_RGB(
    const SpectralData          &camera,
    const std::vector<double>   &WB_multipliers,
    const std::vector<Spectrum> &training_illuminants )
{
    assert( training_illuminants.size() > 0 );
    assert( training_illuminants[0].values.size() == 81 );

    const Spectrum &camera_r = camera["R"];
    const Spectrum &camera_g = camera["G"];
    const Spectrum &camera_b = camera["B"];

    std::vector<std::vector<double>> RGB;
    for ( auto &training_illuminant: training_illuminants )
    {
        auto &rgb = RGB.emplace_back( 3 );
        rgb[0] =
            ( training_illuminant * camera_r ).integrate() * WB_multipliers[0];
        rgb[1] =
            ( training_illuminant * camera_g ).integrate() * WB_multipliers[1];
        rgb[2] =
            ( training_illuminant * camera_b ).integrate() * WB_multipliers[2];
    }

    return RGB;
}

/// Cost function object for IDT matrix optimization using Ceres solver.
/// This struct implements the objective function for curve fitting between camera RGB
/// responses and target LAB values. It's used to find the optimal 6-parameter IDT
/// matrix that minimizes the difference between predicted and actual color values
/// across all training patches.
struct IDTOptimizationCost
{
    IDTOptimizationCost(
        const std::vector<std::vector<double>> &RGB,
        const std::vector<std::vector<double>> &out_LAB )
        : _RGB( RGB ), _outLAB( out_LAB )
    {}

    template <typename T>
    bool operator()( const T *beta_params, T *residuals ) const;

    const std::vector<std::vector<double>> _RGB;
    const std::vector<std::vector<double>> _outLAB;
};

/// Perform curve fitting optimization to find optimal IDT matrix parameters.
/// This function uses the Ceres optimization library to find the best 6-parameter
/// IDT matrix that minimizes the difference between camera RGB responses and
/// target XYZ values across all training patches. The optimization process
/// iteratively adjusts the beta_params parameters to achieve the best color transformation.
///
/// @param RGB Camera RGB responses for training patches
/// @param XYZ Target XYZ values for training patches
/// @param beta_params Initial 6-element parameter array for IDT matrix (modified in-place)
/// @param verbosity Verbosity level for optimization output (0-3):
/// - 0: Silent (no output)
/// - 1: Brief optimization report and final IDT matrix
/// - 2: Full optimization report and progress output
/// - 3: Detailed progress with minimizer output to stdout
/// @param out_IDT_matrix Output IDT matrix computed from optimized parameters
/// @return true if optimization succeeded, false otherwise
bool curveFit(
    const std::vector<std::vector<double>> &RGB,
    const std::vector<std::vector<double>> &XYZ,
    double                                 *beta_params,
    int                                     verbosity,
    std::vector<std::vector<double>>       &out_IDT_matrix )
{
    Problem                problem;
    vector<vector<double>> out_LAB = XYZ_to_LAB( XYZ );

    CostFunction *cost_function =
        new AutoDiffCostFunction<IDTOptimizationCost, ceres::DYNAMIC, 6>(
            new IDTOptimizationCost( RGB, out_LAB ),
            int( RGB.size() * ( RGB[0].size() ) ) );

    problem.AddResidualBlock( cost_function, NULL, beta_params );

    ceres::Solver::Options options;
    options.linear_solver_type        = ceres::DENSE_QR;
    options.parameter_tolerance       = 1e-17;
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
        out_IDT_matrix[0][0] = beta_params[0];
        out_IDT_matrix[0][1] = beta_params[1];
        out_IDT_matrix[0][2] = 1.0 - beta_params[0] - beta_params[1];
        out_IDT_matrix[1][0] = beta_params[2];
        out_IDT_matrix[1][1] = beta_params[3];
        out_IDT_matrix[1][2] = 1.0 - beta_params[2] - beta_params[3];
        out_IDT_matrix[2][0] = beta_params[4];
        out_IDT_matrix[2][1] = beta_params[5];
        out_IDT_matrix[2][2] = 1.0 - beta_params[4] - beta_params[5];

        if ( verbosity > 1 )
        {
            printf( "The IDT matrix is ...\n" );
            for ( int i = 0; i < 3; i++ )
            {
                printf(
                    "   %f %f %f\n",
                    out_IDT_matrix[i][0],
                    out_IDT_matrix[i][1],
                    out_IDT_matrix[i][2] );
            }
        }

        return true;
    }

    delete cost_function;

    return false;
}

bool SpectralSolver::calculate_IDT_matrix()
{
    if ( camera.data.count( "main" ) == 0 ||
         camera.data.at( "main" ).size() != 3 )
    {
        std::cerr << "ERROR: camera needs to be initialised prior to calling "
                  << "SpectralSolver::calculate_IDT_matrix()" << std::endl;
        return false;
    }

    if ( illuminant.data.count( "main" ) == 0 ||
         illuminant.data.at( "main" ).size() != 1 )
    {
        std::cerr << "ERROR: illuminant needs to be initialised prior to "
                  << "calling SpectralSolver::calculate_IDT_matrix()"
                  << std::endl;
        return false;
    }

    if ( observer.data.count( "main" ) == 0 ||
         observer.data.at( "main" ).size() != 3 )
    {
        std::cerr << "ERROR: observer needs to be initialised prior to calling "
                  << "SpectralSolver::calculate_IDT_matrix()" << std::endl;
        return false;
    }

    if ( training_data.data.count( "main" ) == 0 ||
         training_data.data.at( "main" ).empty() )
    {
        std::cerr << "ERROR: training data needs to be initialised prior to "
                  << "calling SpectralSolver::calculate_IDT_matrix()"
                  << std::endl;
        return false;
    }

    double beta_params_start[6] = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };

    auto TI  = calculate_TI( illuminant, training_data );
    auto RGB = calculate_RGB( camera, _WB_multipliers, TI );
    auto XYZ = calculate_XYZ( observer, illuminant, TI );

    return curveFit( RGB, XYZ, beta_params_start, verbosity, _IDT_matrix );
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

const vector<double> &SpectralSolver::get_WB_multipliers() const
{
    return _WB_multipliers;
}

MetadataSolver::MetadataSolver( const core::Metadata &metadata )
    : _metadata( metadata )
{}

/// Convert Correlated Color Temperature (CCT) to Mired units.
/// This function converts color temperature from Kelvin to Mired scale, which is
/// commonly used in photography and lighting. Mired = 1,000,000 / CCT, providing
/// a more perceptually uniform scale for color temperature adjustments.
///
/// @param cct Correlated Color Temperature in Kelvin
/// @return Color temperature in Mired units
/// @pre cct must be positive and non-zero
double CCT_to_mired( const double cct )
{
    return 1.0E06 / cct;
}

/// Convert Mired units to Correlated Color Temperature (CCT).
/// This function converts color temperature from Mired scale back to Kelvin.
///
/// @param mired Color temperature in Mired units
/// @return Correlated color temperature in Kelvin
/// @pre mired must be positive and non-zero
double mired_to_CCT( const double mired )
{
    return 1.0E06 / mired;
}

/// Calculate the Robertson length for color temperature interpolation.
/// This function computes the distance between two points in CIE 1960 UCS color space
/// using the Robertson method. It's used for interpolating color adaptation matrices
/// between different color temperatures during color space transformations.
///
/// @param source_uv Source point coordinates in CIE 1960 UCS space [u, v]
/// @param target_uvt Target point coordinates in CIE 1960 UCS space with temperature [u, v, t]
/// @return Distance between the two points in UCS color space
/// @pre source_uv.size() >= 2, target_uvt.size() >= 3
double robertson_length(
    const vector<double> &source_uv, const vector<double> &target_uvt )
{
    double         t    = target_uvt[2];
    double         sign = t < 0 ? -1.0 : t > 0 ? 1.0 : 0.0;
    vector<double> slope( 2 );
    slope[0] = -sign / std::sqrt( 1 + t * t );
    slope[1] = t * slope[0];

    vector<double> target_uv( target_uvt.begin(), target_uvt.begin() + 2 );
    return cross2d_scalar( slope, subVectors( source_uv, target_uv ) );
}

/// Convert EXIF light source tag to correlated color temperature.
/// This function maps EXIF light source tags to their corresponding color temperatures
/// in Kelvin. It handles both standard EXIF values (0-22) and extended values (≥32768).
/// Extended values are converted by subtracting 32768 from the tag value.
///
/// @param tag EXIF light source tag value
/// @return Correlated color temperature in Kelvin
/// @pre tag is a valid EXIF light source identifier
double light_source_to_color_temp( const unsigned short tag )
{

    if ( tag >= 32768 )
        return ( static_cast<double>( tag ) ) - 32768.0;

    uint16_t exif_light_source_temperature_map[][2] = {
        { 0, 5500 },  { 1, 5500 },  { 2, 3500 },  { 3, 3400 },
        { 10, 5550 }, { 17, 2856 }, { 18, 4874 }, { 19, 6774 },
        { 20, 5500 }, { 21, 6500 }, { 22, 7500 }
    };

    for ( int i = 0; i < countSize( exif_light_source_temperature_map ); i++ )
    {
        if ( exif_light_source_temperature_map[i][0] ==
             static_cast<uint16_t>( tag ) )
        {
            return ( static_cast<double>(
                exif_light_source_temperature_map[i][1] ) );
        }
    }

    return 5500.0;
}

/// Convert XYZ values to correlated color temperature using Robertson method.
/// This function estimates the color temperature from XYZ values by interpolating
/// between known color temperature points in CIE 1960 UCS space. It uses the Robertson
/// method to find the closest color temperature match based on the UV coordinates.
///
/// @param XYZ XYZ color values [X, Y, Z]
/// @return Correlated color temperature in Kelvin
double XYZ_to_color_temperature( const vector<double> &XYZ )
{
    vector<double> uv                  = XYZ_to_uv( XYZ );
    int            num_robertson_table = countSize( robertson_uvt_table );
    int            i;

    double mired;
    double distance_this = 0.0, distance_prev = 0.0;

    for ( i = 0; i < num_robertson_table; i++ )
    {
        vector<double> robertson(
            robertson_uvt_table[i],
            robertson_uvt_table[i] + countSize( robertson_uvt_table[i] ) );
        distance_this = robertson_length( uv, robertson );
        if ( distance_this <= 0.0 )
        {
            break;
        }
        distance_prev = distance_this;
    }

    if ( i <= 0 )
        mired = robertson_mired_table[0];
    else if ( i >= num_robertson_table )
        mired = robertson_mired_table[num_robertson_table - 1];
    else
        mired =
            robertson_mired_table[i - 1] +
            distance_prev *
                ( robertson_mired_table[i] - robertson_mired_table[i - 1] ) /
                ( distance_prev - distance_this );

    double cct = mired_to_CCT( mired );
    cct        = std::max( 2000.0, std::min( 50000.0, cct ) );

    return cct;
}

/// Calculate weighted interpolation between two camera matrices based on Mired values.
/// This function performs linear interpolation between two camera transformation matrices
/// based on the position of a target Mired value between two reference Mired values.
/// The interpolation weight is calculated as (mired_start - mired_target) / (mired_start - mired_end),
/// ensuring smooth transitions between different color temperature calibration points.
///
/// @param mired_target Target Mired value for interpolation
/// @param mired_start First reference Mired value (start of interpolation range)
/// @param mired_end Second reference Mired value (end of interpolation range)
/// @param matrix_start First camera transformation matrix
/// @param matrix_end Second camera transformation matrix
/// @return Interpolated camera transformation matrix
/// @pre mired_start != mired_end to avoid division by zero
vector<double> XYZ_to_camera_weighted_matrix(
    const double              &mired_target,
    const double              &mired_start,
    const double              &mired_end,
    const std::vector<double> &matrix_start,
    const std::vector<double> &matrix_end )
{

    double weight = std::max(
        0.0,
        std::min(
            1.0,
            ( mired_start - mired_target ) / ( mired_start - mired_end ) ) );
    vector<double> result = subVectors( matrix_end, matrix_start );
    scaleVector( result, weight );
    result = addVectors( result, matrix_start );

    return result;
}

/// Find the optimal XYZ to camera transformation matrix using iterative optimization.
/// This function determines the best camera transformation matrix by iteratively
/// searching through Mired values to find the one that minimizes the error between
/// predicted and actual neutral RGB values. It uses a binary search approach with
/// error minimization to find the optimal color temperature calibration point.
///
/// The function interpolates between two calibration matrices based on the estimated
/// optimal Mired value, ensuring accurate color transformations for the given
/// neutral RGB reference values.
///
/// @param metadata Camera metadata containing calibration information and matrices
/// @param neutral_RGB Reference neutral RGB values for optimization
/// @return Optimized XYZ to camera transformation matrix
/// @pre metadata must contain valid calibration data with at least two illuminants
vector<double> find_XYZ_to_camera_matrix(
    const Metadata &metadata, const vector<double> &neutral_RGB )
{

    if ( metadata.calibration[0].illuminant == 0 )
    {
        fprintf( stderr, " No calibration illuminants were found. \n " );
        return metadata.calibration[0].XYZ_to_RGB_matrix;
    }

    if ( neutral_RGB.size() == 0 )
    {
        fprintf( stderr, " no neutral RGB values were found. \n " );
        return metadata.calibration[0].XYZ_to_RGB_matrix;
    }

    double cct1 =
        light_source_to_color_temp( metadata.calibration[0].illuminant );
    double cct2 =
        light_source_to_color_temp( metadata.calibration[1].illuminant );

    double mir1 = CCT_to_mired( cct1 );
    double mir2 = CCT_to_mired( cct2 );

    double max_mired = CCT_to_mired( 2000.0 );
    double min_mired = CCT_to_mired( 50000.0 );

    const std::vector<double> &matrix_start =
        metadata.calibration[0].XYZ_to_RGB_matrix;
    const std::vector<double> &matrix_end =
        metadata.calibration[1].XYZ_to_RGB_matrix;

    double low_mired =
        std::max( min_mired, std::min( max_mired, std::min( mir1, mir2 ) ) );
    double high_mired =
        std::max( min_mired, std::min( max_mired, std::max( mir1, mir2 ) ) );
    double mirStep = std::max( 5.0, ( high_mired - low_mired ) / 50.0 );

    double current_mired = 0.0, last_mired = 0.0, estimated_mired = 0.0,
           current_error = 0.0, last_error = 0.0, smallest_error = 0.0;

    for ( current_mired = low_mired; current_mired < high_mired;
          current_mired += mirStep )
    {
        current_error =
            current_mired -
            CCT_to_mired( XYZ_to_color_temperature( mulVector(
                invertV( XYZ_to_camera_weighted_matrix(
                    current_mired, mir1, mir2, matrix_start, matrix_end ) ),
                neutral_RGB ) ) );

        if ( std::fabs( current_error - 0.0 ) <= 1e-09 )
        {
            estimated_mired = current_mired;
            break;
        }
        if ( std::fabs( current_mired - low_mired - 0.0 ) > 1e-09 &&
             current_error * last_error <= 0.0 )
        {
            estimated_mired = current_mired +
                              ( current_error / ( current_error - last_error ) *
                                ( current_mired - last_mired ) );
            break;
        }
        if ( std::fabs( current_mired - low_mired ) <= 1e-09 ||
             std::fabs( current_error ) < std::fabs( smallest_error ) )
        {
            estimated_mired = current_mired;
            smallest_error  = current_error;
        }

        last_error = current_error;
        last_mired = current_mired;
    }

    return XYZ_to_camera_weighted_matrix(
        estimated_mired, mir1, mir2, matrix_start, matrix_end );
}

/// Convert correlated color temperature to CIE XYZ color values.
/// This function estimates the XYZ color coordinates corresponding to a given
/// correlated color temperature by interpolating between known color temperature
/// points in the Robertson table. It converts the temperature to Mired units
/// and finds the closest match in the pre-computed color temperature data.
///
/// @param cct Correlated color temperature in Kelvin
/// @return Vector of 3 XYZ color values [X, Y, Z]
/// @pre cct should be in the valid range supported by the Robertson table
vector<double> color_temperature_to_XYZ( const double &cct )
{

    double         mired = CCT_to_mired( cct );
    vector<double> uv( 2, 1.0 );

    int num_robertson_table = countSize( robertson_uvt_table );
    int i;

    for ( i = 0; i < num_robertson_table; i++ )
    {
        if ( robertson_mired_table[i] >= mired )
            break;
    }

    if ( i <= 0 )
    {
        uv = vector<double>(
            robertson_uvt_table[0], robertson_uvt_table[0] + 2 );
    }
    else if ( i >= num_robertson_table )
    {
        uv = vector<double>(
            robertson_uvt_table[num_robertson_table - 1],
            robertson_uvt_table[num_robertson_table - 1] + 2 );
    }
    else
    {
        double weight =
            ( mired - robertson_mired_table[i - 1] ) /
            ( robertson_mired_table[i] - robertson_mired_table[i - 1] );

        vector<double> uv1(
            robertson_uvt_table[i], robertson_uvt_table[i] + 2 );
        scaleVector( uv1, weight );

        vector<double> uv2(
            robertson_uvt_table[i - 1], robertson_uvt_table[i - 1] + 2 );
        scaleVector( uv2, 1.0 - weight );

        uv = addVectors( uv1, uv2 );
    }

    return uv_to_XYZ( uv );
}

/// Calculate RGB to XYZ transformation matrix from chromaticity coordinates.
/// This function constructs a 3×3 transformation matrix that converts RGB values
/// to CIE XYZ color space. It takes the xy chromaticity coordinates for red,
/// green, blue primaries and white point, converts them to XYZ, and then
/// calculates the appropriate scaling factors to ensure proper color reproduction.
///
/// The resulting matrix is used in color space transformations and color
/// adaptation calculations, particularly for converting between different
/// RGB color spaces and the standardized CIE XYZ color space.
///
/// @param chromaticities Array of 4 xy chromaticity coordinates [R, G, B, W]
/// @return 3×3 RGB to XYZ transformation matrix as a flattened vector
/// @pre chromaticities must contain exactly 4 xy coordinate pairs
vector<double> matrix_RGB_to_XYZ( const double chromaticities[][2] )
{
    vector<double> red_XYZ =
        xy_to_XYZ( vector<double>( chromaticities[0], chromaticities[0] + 2 ) );
    vector<double> green_XYZ =
        xy_to_XYZ( vector<double>( chromaticities[1], chromaticities[1] + 2 ) );
    vector<double> blue_XYZ =
        xy_to_XYZ( vector<double>( chromaticities[2], chromaticities[2] + 2 ) );
    vector<double> white_XYZ =
        xy_to_XYZ( vector<double>( chromaticities[3], chromaticities[3] + 2 ) );

    vector<double> rgb_matrix( 9 );
    for ( int i = 0; i < 3; i++ )
    {
        rgb_matrix[0 + i * 3] = red_XYZ[i];
        rgb_matrix[1 + i * 3] = green_XYZ[i];
        rgb_matrix[2 + i * 3] = blue_XYZ[i];
    }

    scaleVector( white_XYZ, 1.0 / white_XYZ[1] );

    vector<double> channel_gains =
        mulVector( invertV( rgb_matrix ), white_XYZ, 3 );
    vector<double> color_matrix =
        mulVector( rgb_matrix, diagV( channel_gains ), 3 );

    return color_matrix;
}

/// Calculate camera XYZ transformation matrix and white point from metadata.
/// This function computes the camera-to-XYZ transformation matrix and the
/// corresponding white point in XYZ color space. It uses the camera's neutral
/// RGB values to find the optimal transformation matrix through iterative
/// optimization, then calculates the white point either from the neutral RGB
/// values or from the calibration illuminant's color temperature.
///
/// The function also applies baseline exposure compensation and normalizes
/// the white point to ensure proper color scaling in the transformation pipeline.
///
/// @param metadata Camera metadata containing calibration and exposure information
/// @param out_camera_to_XYZ_matrix Output camera to XYZ transformation matrix
/// @param out_camera_XYZ_white_point Output camera white point in XYZ space
/// @pre metadata must contain valid calibration data and neutral RGB values
void get_camera_XYZ_matrix_and_white_point(
    const Metadata      &metadata,
    std::vector<double> &out_camera_to_XYZ_matrix,
    std::vector<double> &out_camera_XYZ_white_point )
{
    out_camera_to_XYZ_matrix =
        invertV( find_XYZ_to_camera_matrix( metadata, metadata.neutral_RGB ) );
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
        out_camera_XYZ_white_point = color_temperature_to_XYZ(
            light_source_to_color_temp( metadata.calibration[0].illuminant ) );
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
    get_camera_XYZ_matrix_and_white_point(
        _metadata, camera_to_XYZ_matrix, camera_XYZ_white_point );
    vector<double> output_RGB_to_XYZ_matrix =
        matrix_RGB_to_XYZ( chromaticitiesACES );
    vector<double> output_XYZ_white_point =
        mulVector( output_RGB_to_XYZ_matrix, deviceWhiteV );
    vector<vector<double>> CAT_matrix =
        calculate_CAT( camera_XYZ_white_point, output_XYZ_white_point );

    return CAT_matrix;
}

vector<vector<double>> MetadataSolver::calculate_IDT_matrix()
{
    // 1. Obtains the CAT matrix for white point adaptation
    vector<vector<double>> CAT_matrix = calculate_CAT_matrix();

    // 2. Converts the CAT matrix to a flattened format for matrix multiplication
    vector<double> XYZ_D65_acesrgb( 9 ), CAT( 9 );
    for ( size_t i = 0; i < 3; i++ )
        for ( size_t j = 0; j < 3; j++ )
        {
            XYZ_D65_acesrgb[i * 3 + j] = XYZ_D65_acesrgb_3[i][j];
            CAT[i * 3 + j]             = CAT_matrix[i][j];
        }

    // 3. Multiplies the D65 ACES RGB to XYZ matrix with the CAT matrix
    vector<double> matrix = mulVector( XYZ_D65_acesrgb, CAT, 3 );

    // 4. Reshapes the result into a 3×3 transformation matrix
    vector<vector<double>> DNG_IDT_matrix( 3, vector<double>( 3 ) );
    for ( size_t i = 0; i < 3; i++ )
        for ( size_t j = 0; j < 3; j++ )
            DNG_IDT_matrix[i][j] = matrix[i * 3 + j];

    // 5. Validates the matrix properties (non-zero determinant)
    assert( std::fabs( sumVectorM( DNG_IDT_matrix ) - 0.0 ) > 1e-09 );

    return DNG_IDT_matrix;
}

/// Cost function operator for Ceres optimization of IDT matrix parameters.
/// This function computes the residual errors between target LAB values and
/// calculated LAB values from camera RGB responses transformed by candidate
/// IDT matrix parameters. It's used by the Ceres optimization library to
/// iteratively find the optimal 6-parameter IDT matrix that minimizes
/// color differences across all training patches.
///
/// The function transforms camera RGB values using candidate IDT parameters beta_params,
/// converts the result to XYZ using ACES RGB primaries, then to LAB color space,
/// and computes the difference from target LAB values as residuals.
///
/// @param beta_params 6-element array of IDT matrix parameters [b00, b01, b02, b10, b11, b12]
/// @param residuals Output array of LAB differences
/// @return true (required by Ceres interface)
/// @pre _RGB must contain camera RGB responses
/// @pre _outLAB must contain target LAB values
template <typename T>
bool IDTOptimizationCost::operator()( const T *beta_params, T *residuals ) const
{
    vector<vector<T>> RGB_copy( 190, vector<T>( 3 ) );
    for ( size_t i = 0; i < 190; i++ )
        for ( size_t j = 0; j < 3; j++ )
            RGB_copy[i][j] = T( _RGB[i][j] );

    vector<vector<T>> out_calc_LAB =
        XYZ_to_LAB( getCalcXYZt( RGB_copy, beta_params ) );
    for ( size_t i = 0; i < 190; i++ )
        for ( size_t j = 0; j < 3; j++ )
            residuals[i * 3 + j] = _outLAB[i][j] - out_calc_LAB[i][j];

    return true;
}

} // namespace core
} // namespace rta
