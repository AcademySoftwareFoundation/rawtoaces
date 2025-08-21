// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include <rawtoaces/rawtoaces_core.h>
#include <rawtoaces/mathOps.h>
#include <rawtoaces/define.h> // for cmp_str

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

void calDayLightSPD( const int &cct, Spectrum &spectrum )
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
void calBlackBodySPD( const int &cct, Spectrum &spectrum )
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
        calDayLightSPD( cct, illuminant.data["main"].back().second );
    }
    else
    {
        illuminant.illuminant = type;
        calBlackBodySPD( cct, illuminant.data["main"].back().second );
    }
}

// ------------------------------------------------------//

Idt::Idt()
{
    _verbosity = 0;
    _idt.resize( 3 );
    _wb.resize( 3 );
    FORI( 3 )
    {
        _idt[i].resize( 3 );
        _wb[i]               = 1.0;
        FORJ( 3 ) _idt[i][j] = neutral3[i][j];
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

void Idt::scaleLSC( SpectralData &illuminant )
{
    double max_R = _camera["R"].max();
    double max_G = _camera["G"].max();
    double max_B = _camera["B"].max();

    std::string max_channel;

    if ( max_R >= max_G && max_R >= max_B )
        max_channel = "R";
    else if ( max_G >= max_R && max_G >= max_B )
        max_channel = "G";
    else
        max_channel = "B";

    const Spectrum &camera_spectrum     = _camera[max_channel];
    Spectrum       &illuminant_spectrum = illuminant["power"];

    double scale = 1.0 / ( camera_spectrum * illuminant_spectrum ).integrate();
    illuminant_spectrum *= scale;
}

//	=====================================================================
//	Load the Camera Sensitivty data
//
//	inputs:
//		const std::string &: path to the camera sensitivity file
//      const std::string &: camera maker  (from libraw)
//      const std::string &: camera model  (from libraw)
//
//	outputs:
//		int: If successfully parsed, _cameraSpst will be filled and return 1;
//               Otherwise, return 0

int Idt::loadCameraSpst(
    const std::string &path, const std::string &make, const std::string &model )
{
    assert( !path.empty() );
    assert( !make.empty() );
    assert( !model.empty() );

    if ( !_camera.load( path ) )
        return 0;
    if ( cmp_str( _camera.manufacturer.c_str(), make.c_str() ) != 0 )
        return 0;
    if ( cmp_str( _camera.model.c_str(), model.c_str() ) != 0 )
        return 0;

    return 1;
}

//	=====================================================================
//	Load the Illuminant data
//
//	inputs:
//		string: paths to various Illuminant data files
//      string: type of light source if user specifies
//
//	outputs:
//		int: If successfully parsed, _bestIllum will be filled and return 1;
//               Otherwise, return 0

int Idt::loadIlluminant( const vector<string> &paths, string type )
{
    //        assert ( paths.size() > 0 && !type.empty() );

    if ( _Illuminants.size() > 0 )
        _Illuminants.clear();

    if ( type.compare( "na" ) != 0 )
    {

        // Daylight
        if ( type[0] == 'd' )
        {
            int               cct        = atoi( type.substr( 1 ).c_str() );
            const std::string type       = "d" + std::to_string( cct );
            SpectralData     &illuminant = _Illuminants.emplace_back();
            generate_illuminant( cct, type, true, illuminant );
            return 1;
        }
        // Blackbody
        else if ( type[type.length() - 1] == 'k' )
        {
            int cct = atoi( type.substr( 0, type.length() - 1 ).c_str() );
            const std::string type       = std::to_string( cct ) + "k";
            SpectralData     &illuminant = _Illuminants.emplace_back();
            generate_illuminant( cct, type, false, illuminant );
            return 1;
        }
        else
        {
            FORI( paths.size() )
            {
                SpectralData &illuminant = _Illuminants.emplace_back();
                if ( !illuminant.load( paths[i] ) ||
                     illuminant.illuminant != type )
                {
                    _Illuminants.pop_back();
                }
                else
                {
                    return 1;
                }
            }
        }
    }
    else
    {
        // Daylight - pre-calculate
        for ( int i = 4000; i <= 25000; i += 500 )
        {
            SpectralData     &illuminant = _Illuminants.emplace_back();
            const std::string type       = "d" + std::to_string( i / 100 );
            generate_illuminant( i, type, true, illuminant );
        }

        // Blackbody - pre-calculate
        for ( int i = 1500; i < 4000; i += 500 )
        {
            SpectralData     &illuminant = _Illuminants.emplace_back();
            const std::string type       = std::to_string( i ) + "k";
            generate_illuminant( i, type, false, illuminant );
        }

        FORI( paths.size() )
        {
            SpectralData &illuminant = _Illuminants.emplace_back();
            if ( !illuminant.load( paths[i] ) || illuminant.illuminant != type )
            {
                _Illuminants.pop_back();
            }
        }
    }

    return ( _Illuminants.size() > 0 );
}

//	=====================================================================
//	Load the 190-patch training data
//
//	inputs:
//		string : path to the 190-patch training data
//
//	outputs:
//		_trainingSpec: If successfully parsed, _trainingSpec will be filled

void Idt::loadTrainingData( const string &path )
{
    struct stat st;
    assert( !stat( path.c_str(), &st ) );

    _training_data.load( path );
}

//	=====================================================================
//	Load the CIE 1931 Color Matching Functions data
//
//	inputs:
//		string : path to the CIE 1931 Color Matching Functions data
//
//	outputs:
//		_cmf: If successfully parsed, _cmf will be filled

void Idt::loadCMF( const string &path )
{
    struct stat st;
    assert( !stat( path.c_str(), &st ) );

    _observer.load( path );
}

//	=====================================================================
//	Push new Illuminant to further process Spectral Power Data
//
//	inputs:
//      Illum: Illuminant
//
//	outputs:
//		N/A:   _Illuminants should have one more element

void Idt::setIlluminants( const SpectralData &Illuminant )
{
    _Illuminants.push_back( Illuminant );
}

//	=====================================================================
//	Set Verbosity value for the length of IDT generation status message
//
//	inputs:
//      int: verbosity
//
//	outputs:
//		int: _verbosity

void Idt::setVerbosity( const int verbosity )
{
    _verbosity = verbosity;
}

//	=====================================================================
//	Choose the best Light Source based on White Balance Coefficients from
//  the camera read by libraw according to a given set of coefficients
//
//	inputs:
//		Map: Key: path to the Light Source data;
//           Value: Light Source x Camera Sensitivity
//      Vector: White Balance Coefficients
//
//	outputs:
//		Illum: the best _Illuminant

void Idt::chooseIllumSrc( const vector<double> &src, int highlight )
{
    double sse = dmax;

    FORI( _Illuminants.size() )
    {
        vector<double> wb_tmp  = calWB( _Illuminants[i], highlight );
        double         sse_tmp = calSSE( wb_tmp, src );

        printf( "%s, %f \n", _Illuminants[i].illuminant.c_str(), sse_tmp );
        printf( "%f, %f, %f\n ", wb_tmp[0], wb_tmp[1], wb_tmp[2] );

        if ( sse_tmp < sse )
        {
            sse              = sse_tmp;
            _best_illuminant = _Illuminants[i];
            _wb              = wb_tmp;
        }
    }

    if ( _verbosity > 1 )
        printf(
            "The illuminant calculated to be the best match to the camera metadata is %s\n",
            _best_illuminant.illuminant.c_str() );

    // scale back the WB factor
    double factor = _wb[1];
    assert( factor != 0.0 );
    FORI( _wb.size() ) _wb[i] /= factor;

    return;
}

//	=====================================================================
//	Choose the best Light Source based on White Balance Coefficients from
//  the camera read by libraw according to user-specified illuminant
//
//	inputs:
//		Map: Key: path to the Light Source data;
//           Value: Light Source x Camera Sensitivity
//      String: Light Source Name
//
//	outputs:
//		Illum: the best _Illuminant

void Idt::chooseIllumType( const std::string &type, int highlight )
{
    assert( type == _Illuminants[0].illuminant );

    _best_illuminant = _Illuminants[0];
    _wb              = calWB( _best_illuminant, highlight );

    //		if (_verbosity > 1)
    //            printf ( "The specified light source is: %s\n",
    //                     _bestIllum._type.c_str() );

    // scale back the WB factor
    double factor = _wb[1];
    assert( factor != 0.0 );
    FORI( _wb.size() ) _wb[i] /= factor;

    return;
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

vector<double> Idt::calCM()
{
    Spectrum &camera_r   = _camera["R"];
    Spectrum &camera_g   = _camera["G"];
    Spectrum &camera_b   = _camera["B"];
    Spectrum &illuminant = _best_illuminant["power"];

    double r = ( camera_r * illuminant ).integrate();
    double g = ( camera_g * illuminant ).integrate();
    double b = ( camera_b * illuminant ).integrate();

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

vector<Spectrum> Idt::calTI() const
{
    std::vector<Spectrum> result;

    const Spectrum &illuminant_spectrum = _best_illuminant["power"];
    for ( auto &[name, training_spectrum]: _training_data.data.at( "main" ) )
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

vector<double> Idt::calWB( SpectralData &illuminant, int highlight )
{
    scaleLSC( illuminant );

    const Spectrum &camera_r = _camera["R"];
    const Spectrum &camera_g = _camera["G"];
    const Spectrum &camera_b = _camera["B"];
    const Spectrum &illum    = illuminant["power"];

    double r = ( camera_r * illum ).integrate();
    double g = ( camera_g * illum ).integrate();
    double b = ( camera_b * illum ).integrate();

    std::vector<double> wb = { 1.0 / r, 1.0 / g, 1.0 / b };

    if ( !highlight )
        scaleVectorMin( wb );
    else
        scaleVectorMax( wb );

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

vector<vector<double>> Idt::calXYZ( const vector<Spectrum> &TI ) const
{
    assert( TI.size() > 0 );
    assert( TI[0].values.size() == 81 );

    vector<vector<double>> colXYZ( 3, vector<double>( TI.size(), 1.0 ) );

    std::vector<double>              w( XYZ_w, XYZ_w + 3 );
    std::vector<std::vector<double>> XYZ;

    const Spectrum &cmf_x = _observer["X"];
    const Spectrum &cmf_y = _observer["Y"];
    const Spectrum &cmf_z = _observer["Z"];
    const Spectrum &illum = _best_illuminant["power"];

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

    XYZ = mulVector( XYZ, getCAT( ww, w ) );

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

vector<vector<double>> Idt::calRGB( const vector<Spectrum> &TI ) const
{
    assert( TI.size() > 0 );
    assert( TI[0].values.size() == 81 );

    const Spectrum &cam_r = _camera["R"];
    const Spectrum &cam_g = _camera["G"];
    const Spectrum &cam_b = _camera["B"];
    const Spectrum &illum = _best_illuminant["power"];

    std::vector<std::vector<double>> RGB;
    for ( auto &ti: TI )
    {
        auto &rgb = RGB.emplace_back( 3 );
        rgb[0]    = ( ti * cam_r ).integrate() * _wb[0];
        rgb[1]    = ( ti * cam_g ).integrate() * _wb[1];
        rgb[2]    = ( ti * cam_b ).integrate() * _wb[2];
    }

    return RGB;
}

//	=====================================================================
//	Process cureve fit between XYZ and RGB data with initial set of B
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

int Idt::curveFit(
    const vector<vector<double>> &RGB,
    const vector<vector<double>> &XYZ,
    double                       *B )
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

    if ( _verbosity > 2 )
        options.minimizer_progress_to_stdout = true;

    ceres::Solver::Summary summary;
    ceres::Solve( options, &problem, &summary );

    if ( _verbosity > 1 )
        std::cout << summary.BriefReport() << std::endl;
    else if ( _verbosity >= 2 )
        std::cout << summary.FullReport() << std::endl;

    if ( summary.num_successful_steps )
    {
        _idt[0][0] = B[0];
        _idt[0][1] = B[1];
        _idt[0][2] = 1.0 - B[0] - B[1];
        _idt[1][0] = B[2];
        _idt[1][1] = B[3];
        _idt[1][2] = 1.0 - B[2] - B[3];
        _idt[2][0] = B[4];
        _idt[2][1] = B[5];
        _idt[2][2] = 1.0 - B[4] - B[5];

        if ( _verbosity > 1 )
        {
            printf( "The IDT matrix is ...\n" );
            FORI( 3 )
            printf( "   %f %f %f\n", _idt[i][0], _idt[i][1], _idt[i][2] );
        }

        return 1;
    }

    delete cost_function;

    return 0;
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

int Idt::calIDT()
{

    double           BStart[6] = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };
    vector<Spectrum> TI        = calTI();

    return curveFit( calRGB( TI ), calXYZ( TI ), BStart );
}

//	=====================================================================
//  Get camera sensitivity data that was loaded from the file
//
//	inputs:
//         N/A
//
//	outputs:
//      const SpectralData: camera sensitivity data that was loaded from the file

const SpectralData &Idt::getCameraSpst() const
{
    return _camera;
}

//	=====================================================================
//  Get Illuminant data / light source that was loaded from the file
//
//	inputs:
//         N/A
//
//	outputs:
//      const vector < SpectralData >: Illuminant data that was loaded from
//      the file

const vector<SpectralData> &Idt::getIlluminants() const
{
    return _Illuminants;
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

const SpectralData &Idt::getBestIllum() const
{
    assert( _best_illuminant.data.count( "main" ) == 1 );
    assert( _best_illuminant.data.at( "main" ).size() == 1 );
    assert( _best_illuminant["power"].values.size() > 0 );

    return _best_illuminant;
}

//	=====================================================================
//	Get Verbosity value for the length of IDT generation status message
//
//	inputs:
//      N/A
//
//	outputs:
//		int: _verbosity (const)

const int Idt::getVerbosity() const
{
    return _verbosity;
}

//	=====================================================================
//  Get Spectral Training Data that was loaded from the file
//
//	inputs:
//         N/A
//
//	outputs:
//      const SpectralData: Spectral Training data that was loaded
//      from the file

const SpectralData &Idt::getTrainingSpec() const
{
    return _training_data;
}

//	=====================================================================
//  Get Color Matching Function Data that was loaded from the file
//
//	inputs:
//         N/A
//
//	outputs:
//      const SpectralData: Color Matching Function data that was loaded from
//      the file

const SpectralData &Idt::getCMF() const
{
    return _observer;
}

//	=====================================================================
//  Get Idt matrix if CalIDT() succeeds
//
//	inputs:
//         N/A
//
//	outputs:
//      const vector< vector < double > >: _idt matrix (3 x 3)

const vector<vector<double>> Idt::getIDT() const
{
    return _idt;
}

//	=====================================================================
//  Get white balanced if calWB(...) succeeds
//
//	inputs:
//         N/A
//
//	outputs:
//      const vector< double >: _wb vector (1 x 3)

const vector<double> Idt::getWB() const
{
    return _wb;
}

// ------------------------------------------------------//

DNGIdt::DNGIdt( const core::Metadata &metadata ) : _metadata( metadata )
{}

double DNGIdt::ccttoMired( const double cct ) const
{
    return 1.0E06 / cct;
}

double DNGIdt::robertsonLength(
    const vector<double> &uv, const vector<double> &uvt ) const
{

    double         t    = uvt[2];
    double         sign = t < 0 ? -1.0 : t > 0 ? 1.0 : 0.0;
    vector<double> slope( 2 );
    slope[0] = -sign / std::sqrt( 1 + t * t );
    slope[1] = t * slope[0];

    vector<double> uvr( uvt.begin(), uvt.begin() + 2 );
    return cross2( slope, subVectors( uv, uvr ) );
}

double DNGIdt::lightSourceToColorTemp( const unsigned short tag ) const
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

double DNGIdt::XYZToColorTemperature( const vector<double> &XYZ ) const
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

vector<double> DNGIdt::XYZtoCameraWeightedMatrix(
    const double &mir0, const double &mir1, const double &mir2 ) const
{

    double weight =
        std::max( 0.0, std::min( 1.0, ( mir1 - mir0 ) / ( mir1 - mir2 ) ) );
    vector<double> result = subVectors(
        _metadata.calibration[1].xyz2rgbMatrix,
        _metadata.calibration[0].xyz2rgbMatrix );
    scaleVector( result, weight );
    result = addVectors( result, _metadata.calibration[0].xyz2rgbMatrix );

    return result;
}

vector<double>
DNGIdt::findXYZtoCameraMtx( const vector<double> &neutralRGB ) const
{

    if ( _metadata.calibration[0].illuminant == 0 )
    {
        fprintf( stderr, " No calibration illuminants were found. \n " );
        return _metadata.calibration[0].xyz2rgbMatrix;
    }

    if ( neutralRGB.size() == 0 )
    {
        fprintf( stderr, " no neutral RGB values were found. \n " );
        return _metadata.calibration[0].xyz2rgbMatrix;
    }

    double cct1 = lightSourceToColorTemp( _metadata.calibration[0].illuminant );
    double cct2 = lightSourceToColorTemp( _metadata.calibration[1].illuminant );

    double mir1 = ccttoMired( cct1 );
    double mir2 = ccttoMired( cct2 );

    double maxMir = ccttoMired( 2000.0 );
    double minMir = ccttoMired( 50000.0 );

    double lomir =
        std::max( minMir, std::min( maxMir, std::min( mir1, mir2 ) ) );
    double himir =
        std::max( minMir, std::min( maxMir, std::max( mir1, mir2 ) ) );
    double mirStep = std::max( 5.0, ( himir - lomir ) / 50.0 );

    double mir = 0.0, lastMired = 0.0, estimatedMired = 0.0, lerror = 0.0,
           lastError = 0.0, smallestError = 0.0;

    for ( mir = lomir; mir < himir; mir += mirStep )
    {
        lerror =
            mir - ccttoMired( XYZToColorTemperature( mulVector(
                      invertV( XYZtoCameraWeightedMatrix( mir, mir1, mir2 ) ),
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

    return XYZtoCameraWeightedMatrix( estimatedMired, mir1, mir2 );
}

vector<double> DNGIdt::colorTemperatureToXYZ( const double &cct ) const
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

vector<double> DNGIdt::matrixRGBtoXYZ( const double chromaticities[][2] ) const
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

void DNGIdt::getCameraXYZMtxAndWhitePoint()
{
    _cameraToXYZMtx = invertV( findXYZtoCameraMtx( _metadata.neutralRGB ) );
    assert( std::fabs( sumVector( _cameraToXYZMtx ) - 0.0 ) > 1e-09 );

    scaleVector( _cameraToXYZMtx, std::pow( 2.0, _metadata.baselineExposure ) );

    if ( _metadata.neutralRGB.size() > 0 )
    {
        _cameraXYZWhitePoint =
            mulVector( _cameraToXYZMtx, _metadata.neutralRGB );
    }
    else
    {
        _cameraXYZWhitePoint = colorTemperatureToXYZ(
            lightSourceToColorTemp( _metadata.calibration[0].illuminant ) );
    }

    scaleVector( _cameraXYZWhitePoint, 1.0 / _cameraXYZWhitePoint[1] );
    assert( sumVector( _cameraXYZWhitePoint ) != 0 );

    return;
}

vector<vector<double>> DNGIdt::getDNGCATMatrix()
{
    vector<double> deviceWhiteV( 3, 1.0 );
    getCameraXYZMtxAndWhitePoint();
    vector<double> outputRGBtoXYZMtx = matrixRGBtoXYZ( chromaticitiesACES );
    vector<double> outputXYZWhitePoint =
        mulVector( outputRGBtoXYZMtx, deviceWhiteV );
    vector<vector<double>> chadMtx =
        getCAT( _cameraXYZWhitePoint, outputXYZWhitePoint );

    return chadMtx;
}

vector<vector<double>> DNGIdt::getDNGIDTMatrix()
{
    vector<vector<double>> chadMtx = getDNGCATMatrix();
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
