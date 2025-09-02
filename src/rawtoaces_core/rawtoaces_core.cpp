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

//	=====================================================================
//	Calculate the chromaticity values based on cct
//
//	inputs:
//      const int: cct / correlated color temperature
//
//	outputs:
//		vector <double>: xy / chromaticity values
//

vector<double> cctToxy( const double &cctd )
{
    //        assert( cctd >= 4000 && cct <= 25000 );

    vector<double> xy( 2, 1.0 );
    if ( cctd >= 4002.15 && cctd <= 7003.77 )
        xy[0] =
            ( 0.244063 + 99.11 / cctd +
              2.9678 * 1000000 / ( std::pow( cctd, 2 ) ) -
              4.6070 * 1000000000 / ( std::pow( cctd, 3 ) ) );
    else
        xy[0] =
            ( 0.237040 + 247.48 / cctd +
              1.9018 * 1000000 / ( std::pow( cctd, 2 ) ) -
              2.0064 * 1000000000 / ( std::pow( cctd, 3 ) ) );

    xy[1] = -3.0 * ( std::pow( xy[0], 2 ) ) + 2.87 * xy[0] - 0.275;

    return xy;
}

//	=====================================================================
//	Calculate spectral power distribution(SPD) of CIE standard daylight
//  illuminant based on the requested Correlated Color Temperature
//	input value(s):
//
//      const int: cct / correlated color temperature
//      Spectrum &: spectrum / reference to Spectrum object to fill in
//

void calculate_daylight_SPD( const int &cct, Spectrum &spectrum )
{
    int inc = spectrum.shape.step;
    assert( ( s_series[53].wl - s_series[0].wl ) % inc == 0 );

    double cctd = 1.0;
    if ( cct >= 40 && cct <= 250 )
        cctd = cct * 100 * 1.4387752 / 1.438;
    else if ( cct >= 4000 && cct <= 25000 )
        cctd = cct * 1.0;
    else
    {
        fprintf(
            stderr,
            "The range of Correlated Color Temperature for "
            "Day Light should be from 4000 to 25000. \n" );
        exit( 1 );
    }

    spectrum.values.clear();

    vector<int>    wls0, wls1;
    vector<double> s00, s10, s20, s01, s11, s21;
    vector<double> xy = cctToxy( cctd );

    double m0 = 0.0241 + 0.2562 * xy[0] - 0.7341 * xy[1];
    double m1 = ( -1.3515 - 1.7703 * xy[0] + 5.9114 * xy[1] ) / m0;
    double m2 = ( 0.03000 - 31.4424 * xy[0] + 30.0717 * xy[1] ) / m0;

    FORI( 54 )
    {
        wls0.push_back( s_series[i].wl );
        s00.push_back( s_series[i].RGB[0] );
        s10.push_back( s_series[i].RGB[1] );
        s20.push_back( s_series[i].RGB[2] );
    }

    int size = ( s_series[53].wl - s_series[0].wl ) / inc + 1;
    FORI( size )
    wls1.push_back( s_series[0].wl + inc * i );

    s01 = interp1DLinear( wls0, wls1, s00 );
    clearVM( s00 );
    s11 = interp1DLinear( wls0, wls1, s10 );
    clearVM( s10 );
    s21 = interp1DLinear( wls0, wls1, s20 );
    clearVM( s20 );

    clearVM( wls0 );
    clearVM( wls1 );

    FORI( size )
    {
        int index = s_series[0].wl + inc * i;
        if ( index >= 380 && index <= 780 )
        {
            spectrum.values.push_back( s01[i] + m1 * s11[i] + m2 * s21[i] );
        }
    }

    clearVM( s01 );
    clearVM( s11 );
    clearVM( s21 );
}

//	=====================================================================
//    Generates blackbody curve(s) of a given temperature
//
//      const int: temp / temperature
//      Spectrum &: spectrum / reference to Spectrum object to fill in
//
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

    for ( int wav = 380; wav <= 780; wav += 5 )
    {
        double lambda = wav / 1e9;
        double c1     = 2 * bh * ( std::pow( bc, 2 ) );
        double c2     = ( bh * bc ) / ( bk * lambda * cct );
        spectrum.values.push_back(
            c1 * pi / ( std::pow( lambda, 5 ) * ( std::exp( c2 ) - 1 ) ) );
    }
}

void generate_illuminant(
    int               cct,
    const std::string type,
    bool              is_daylight,
    SpectralData     &illuminant )
{
    illuminant.data.clear();

    auto i = illuminant.data.emplace( "main", SpectralData::SpectralSet() );
    i.first->second.emplace_back(
        SpectralData::SpectralChannel( "power", Spectrum( 0 ) ) );

    if ( is_daylight )
    {
        illuminant.illuminant = type;
        calculate_daylight_SPD( cct, illuminant.data["main"].back().second );
    }
    else
    {
        illuminant.illuminant = type;
        calculate_blackbody_SPD( cct, illuminant.data["main"].back().second );
    }
}

// ------------------------------------------------------//

SpectralSolver::SpectralSolver(
    const std::vector<std::string> &search_directories )
    : _search_directories( search_directories )
{
    verbosity = 0;
    _IDT_matrix.resize( 3 );
    _WB_multipliers.resize( 3 );
    FORI( 3 )
    {
        _IDT_matrix[i].resize( 3 );
        _WB_multipliers[i]          = 1.0;
        FORJ( 3 ) _IDT_matrix[i][j] = neutral3[i][j];
    }
}

//	=====================================================================
//	Scale the Illuminant data using the max element of RGB code values
//
//	inputs:
//		Illum & Illuminant
//
//	outputs:
//		scaled Illuminant data set

void scaleLSC( const SpectralData &camera, SpectralData &illuminant )
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
            std::filesystem::path path( directory );
            path.append( file_path );

            if ( std::filesystem::exists( path ) )
            {
                return out_data.load( path.string() );
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

        if ( cmp_str( camera.manufacturer.c_str(), make.c_str() ) != 0 )
            continue;
        if ( cmp_str( camera.model.c_str(), model.c_str() ) != 0 )
            continue;
        return true;
    }
    return false;
}

bool SpectralSolver::find_illuminant( const std::string &type )
{
    assert( !type.empty() );

    // Daylight
    if ( std::tolower( type.front() ) == 'd' )
    {
        int               cct  = atoi( type.substr( 1 ).c_str() );
        const std::string type = "d" + std::to_string( cct );
        generate_illuminant( cct, type, true, illuminant );
        return true;
    }
    // Blackbody
    else if ( std::tolower( type.back() ) == 'k' )
    {
        int cct = atoi( type.substr( 0, type.length() - 1 ).c_str() );
        const std::string type = std::to_string( cct ) + "k";
        generate_illuminant( cct, type, false, illuminant );
        return true;
    }
    else
    {
        auto illuminant_files = collect_data_files( "illuminant" );

        for ( const auto &illuminant_file: illuminant_files )
        {
            if ( !illuminant.load( illuminant_file ) )
                continue;
            if ( cmp_str( illuminant.illuminant.c_str(), type.c_str() ) != 0 )
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
        for ( int i = 4000; i <= 25000; i += 500 )
        {
            SpectralData     &illuminant = _all_illuminants.emplace_back();
            const std::string type       = "d" + std::to_string( i / 100 );
            generate_illuminant( i, type, true, illuminant );
        }

        // Blackbody - pre-calculate
        for ( int i = 1500; i < 4000; i += 500 )
        {
            SpectralData     &illuminant = _all_illuminants.emplace_back();
            const std::string type       = std::to_string( i ) + "k";
            generate_illuminant( i, type, false, illuminant );
        }

        auto illuminant_files = collect_data_files( "illuminant" );

        for ( const auto &illuminant_file: illuminant_files )
        {
            SpectralData &illuminant = _all_illuminants.emplace_back();
            if ( !illuminant.load( illuminant_file ) )
            {
                _all_illuminants.pop_back();
                continue;
            }
        }
    }

    double sse = dmax;

    for ( auto &current_illuminant: _all_illuminants )
    {
        vector<double> wb_tmp  = calWB( camera, current_illuminant );
        double         sse_tmp = calSSE( wb_tmp, wb );

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

    _WB_multipliers = calWB( camera, illuminant );
    return true;
}

//	=====================================================================
//	Calculate the middle product based on the camera sensitivity data
//  and Illuminant/light source data
//
//	inputs:
//		N/A
//
//	outputs:
//		vector < double >: scaled vector by its maximum value

std::vector<double>
calCM( const SpectralData &camera, const SpectralData &illuminant )
{
    const Spectrum &camera_r = camera["R"];
    const Spectrum &camera_g = camera["G"];
    const Spectrum &camera_b = camera["B"];
    const Spectrum &illum    = illuminant["power"];

    double r = ( camera_r * illum ).integrate();
    double g = ( camera_g * illum ).integrate();
    double b = ( camera_b * illum ).integrate();

    double max = std::max( r, std::max( g, b ) );

    std::vector<double> result( 3 );
    result[0] = max / r;
    result[1] = max / g;
    result[2] = max / b;
    return result;
}

//	=====================================================================
//	Calculate the middle product based on the 190 patch / training data
//  and Illuminant/light source data
//
//	inputs:
//		N/A
//
//	outputs:
//		vector < vector<double> >: 2D vector (81 x 190)

std::vector<Spectrum>
calTI( const SpectralData &illuminant, const SpectralData &training_data )
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
calWB( const SpectralData &camera, SpectralData &illuminant )
{
    scaleLSC( camera, illuminant );

    const Spectrum &camera_r = camera["R"];
    const Spectrum &camera_g = camera["G"];
    const Spectrum &camera_b = camera["B"];
    const Spectrum &illum    = illuminant["power"];

    double r = ( camera_r * illum ).integrate();
    double g = ( camera_g * illum ).integrate();
    double b = ( camera_b * illum ).integrate();

    // Normalise to the green channel.
    std::vector<double> wb = { g / r, 1.0, g / b };
    return wb;
}

//	=====================================================================
//	Calculate CIE XYZ tristimulus values of scene adopted white
//  based on training color spectral radiances from CalTI() and color
//  adaptation matrix from CalCAT()
//
//	inputs:
//		vector< vector<double> > outcome of CalTI()
//
//	outputs:
//		vector < vector<double> >: 2D vector (190 x 3)

std::vector<std::vector<double>> calXYZ(
    const SpectralData          &observer,
    const SpectralData          &illuminant,
    const std::vector<Spectrum> &TI )
{
    assert( TI.size() > 0 );
    assert( TI[0].values.size() == 81 );

    vector<vector<double>> colXYZ( 3, vector<double>( TI.size(), 1.0 ) );

    std::vector<double>              w( XYZ_w, XYZ_w + 3 );
    std::vector<std::vector<double>> XYZ;

    const Spectrum &cmf_x = observer["X"];
    const Spectrum &cmf_y = observer["Y"];
    const Spectrum &cmf_z = observer["Z"];
    const Spectrum &illum = illuminant["power"];

    double scale = 1.0 / ( cmf_y * illum ).integrate();

    for ( auto &ti: TI )
    {
        auto &xyz = XYZ.emplace_back( 3 );
        xyz[0]    = ( ti * cmf_x ).integrate() * scale;
        xyz[1]    = ( ti * cmf_y ).integrate() * scale;
        xyz[2]    = ( ti * cmf_z ).integrate() * scale;
    }

    std::vector<double> ww( 3 );
    double              y = ( cmf_y * illum ).integrate();
    ww[0]                 = ( cmf_x * illum ).integrate() / y;
    ww[1]                 = 1.0;
    ww[2]                 = ( cmf_z * illum ).integrate() / y;

    XYZ = mulVector( XYZ, calculate_CAT( ww, w ) );

    return XYZ;
}

//	=====================================================================
//	Calculate white-balanced linearized camera system response (in RGB)
//  based on training color spectral radiances from CalTI() and white
//  balance factors from calWB(...)
//
//	inputs:
//		vector< vector<double> > outcome of CalTI()
//
//	outputs:
//		vector < vector<double> >: 2D vector (190 x 3)

std::vector<std::vector<double>> calRGB(
    const SpectralData          &camera,
    const SpectralData          &illuminant,
    const std::vector<double>   &WB_multipliers,
    const std::vector<Spectrum> &TI )
{
    assert( TI.size() > 0 );
    assert( TI[0].values.size() == 81 );

    const Spectrum &cam_r = camera["R"];
    const Spectrum &cam_g = camera["G"];
    const Spectrum &cam_b = camera["B"];
    const Spectrum &illum = illuminant["power"];

    std::vector<std::vector<double>> RGB;
    for ( auto &ti: TI )
    {
        auto &rgb = RGB.emplace_back( 3 );
        rgb[0]    = ( ti * cam_r ).integrate() * WB_multipliers[0];
        rgb[1]    = ( ti * cam_g ).integrate() * WB_multipliers[1];
        rgb[2]    = ( ti * cam_b ).integrate() * WB_multipliers[2];
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

    double BStart[6] = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };

    auto TI  = calTI( illuminant, training_data );
    auto RGB = calRGB( camera, illuminant, _WB_multipliers, TI );
    auto XYZ = calXYZ( observer, illuminant, TI );

    return curveFit( RGB, XYZ, BStart, verbosity, _IDT_matrix );
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
