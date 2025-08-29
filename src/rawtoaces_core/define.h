// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#ifndef _DEFINE_h__
#define _DEFINE_h__

#include <string>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <vector>

#ifndef WIN32
#    include <sys/stat.h>
#endif

#define INV_255 ( 1.0 / (double)255.0 )
#define INV_65535 ( 1.0 / (double)65535.0 )

#ifdef WIN32
// suppress sprintf-related warning. sprintf() is permitted in sample code
#    include <string.h>
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
//#    define snprintf _snprintf
#    define _CRT_SECURE_NO_WARNINGS
#    define cmp_str stricmp
#else
#    define cmp_str strcasecmp
#endif

#if !defined( TRUE )
#    define TRUE 1
#endif

#if !defined( FALSE )
#    define FALSE 0
#endif

#define FORI( val ) for ( int i = 0; i < val; i++ )
#define FORJ( val ) for ( int j = 0; j < val; j++ )
#define FORIJ( val1, val2 )                                                    \
    for ( int i = 0; i < val1; i++ )                                           \
        for ( int j = 0; j < val2; j++ )
#define countSize( a ) ( static_cast<int>( sizeof( a ) / sizeof( ( a )[0] ) ) )

namespace rta
{
namespace core
{

const double pi = 3.1416;
// 216.0/24389.0
const double e = 0.008856451679;
// (24389.0/27.0)/116.0
const double k = 7.787037037037;

// Planck's constant ([J*s] Joule-seconds)
const double bh = 6.626176 * 1e-34;
// Boltzmann constant ([J/K] Joules per Kelvin)
const double bk = 1.380662 * 1e-23;
// Speed of light ([m/s] meters per second)
const double bc = 2.99792458 * 1e8;

const double dmin = std::numeric_limits<double>::min();
const double dmax = std::numeric_limits<double>::max();

// clang-format off

static const double XYZ_w[3] = {0.952646074569846, 1.0,    1.00882518435159};
static const double d50  [3] = {0.9642,            1.0000, 0.8250};
static const double d60  [3] = {0.952646074569846, 1.0000, 1.00882518435159};
static const double d65  [3] = {0.9547,            1.0000, 1.0883};

static const double neutral3[3][3] = {
    {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 0.0, 1.0}
};

static const struct
{
    int    wl;
    double RGB[3];
} s_series[54] = {
    { 300,  {   0.04,  0.02, 0.0 } },
    { 310,	{   6.0,   4.5,  2.0 } },
    { 320,	{  29.6,  22.4,  4.0 } },
    { 330,	{  55.3,  42.0,  8.5 } },
    { 340,	{  57.3,  40.6,  7.8 } },
    { 350,	{  61.8,  41.6,  6.7 } },
    { 360,	{  61.5,  38.0,  5.3 } },
    { 370,	{  68.8,  42.4,  6.1 } },
    { 380,	{  63.4,  38.5,  3.0 } },
    { 390,	{  65.8,  35.0,  1.2 } },
    { 400,	{  94.8,  43.4, -1.1 } },
    { 410,	{ 104.8,  46.3, -0.5 } },
    { 420,	{ 105.9,  43.9,	-0.7 } },
    { 430,	{  96.8,  37.1,	-1.2 } },
    { 440,	{ 113.9,  36.7, -2.6 } },
    { 450,	{ 125.6,  35.9,	-2.9 } },
    { 460,	{ 125.5,  32.6,	-2.8 } },
    { 470,	{ 121.3,  27.9,	-2.6 } },
    { 480,	{ 121.3,  24.3,	-2.6 } },
    { 490,	{ 113.5,  20.1,	-1.8 } },
    { 500,	{ 113.1,  16.2,	-1.5 } },
    { 510,	{ 110.8,  13.2,	-1.3 } },
    { 520,	{ 106.5,   8.6, -1.2 } },
    { 530,	{ 108.8,   6.1, -1.0 } },
    { 540,	{ 105.3,   4.2,	-0.5 } },
    { 550,	{ 104.4,   1.9,	-0.3 } },
    { 560,	{ 100.0,   0.0,	 0.0 } },
    { 570,	{  96.0,  -1.6,	 0.2 } },
    { 580,	{  95.1,  -3.5,	 0.5 } },
    { 590,	{  89.1,  -3.5,	 2.1 } },
    { 600,	{  90.5,  -5.8,	 3.2 } },
    { 610,	{  90.3,  -7.2,	 4.1 } },
    { 620,	{  88.4,  -8.6,	 4.7 } },
    { 630,	{  84.0,  -9.5,	 5.1 } },
    { 640,	{  85.1, -10.9,	 6.7 } },
    { 650,	{  81.9, -10.7,	 7.3 } },
    { 660,	{  82.6, -12.0,	 8.6 } },
    { 670,	{  84.9, -14.0,	 9.8 } },
    { 680,	{  81.3, -13.6,	10.2 } },
    { 690,	{  71.9, -12.0,	 8.3 } },
    { 700,	{  74.3, -13.3,	 9.6 } },
    { 710,	{  76.4, -12.9,	 8.5 } },
    { 720,	{  63.3, -10.6,	 7.0 } },
    { 730,	{  71.7, -11.6,	 7.6 } },
    { 740,	{  77.0, -12.2,	 8.0 } },
    { 750,	{  65.2, -10.2,	 6.7 } },
    { 760,	{  47.7,  -7.8,	 5.2 } },
    { 770,	{  68.6, -11.2,	 7.4 } },
    { 780,	{  65.0, -10.4,	 6.8 } },
    { 790,	{  66.0, -10.6,	 7.0 } },
    { 800,	{  61.0,  -9.7,	 6.4 } },
    { 810,	{  53.3,  -8.3,	 5.5 } },
    { 820,	{  58.9,  -9.3,	 6.1 } },
    { 830,	{  61.9,  -9.8,	 6.5 } },
};

static const double XYZ_D65_acesrgb_3[3][3] = {
    {  1.0634731317028,    0.00639793641966071, -0.0157891874506841 },
    { -0.492082784686793,  1.36823709310019,     0.0913444629573544 },
    { -0.0028137154424595, 0.00463991165243123,  0.91649468506889   }
};

static const double XYZ_D65_acesrgb_4[4][4] = {
    { 1.0634731317028,     0.00639793641966071, -0.0157891874506841, 0.0 },
    { -0.492082784686793,  1.36823709310019,     0.0913444629573544, 0.0 },
    { -0.0028137154424595, 0.00463991165243123,  0.91649468506889,   0.0 },
    { 0.0,                 0.0,                  0.0,                1.0 }
};

static const double XYZ_acesrgb_3[3][3] = {
    {  1.0498110175, 0.0000000000, -0.0000974845 },
    { -0.4959030231, 1.3733130458,  0.0982400361 },
    {  0.0000000000, 0.0000000000,  0.9912520182 }
};

static const double XYZ_acesrgb_4[4][4] = {
    {  1.0498110175, 0.0000000000, -0.0000974845, 0.0 },
    { -0.4959030231, 1.3733130458,  0.0982400361, 0.0 },
    {  0.0000000000, 0.0000000000,  0.9912520182, 0.0 },
    {  0.0,          0.0,           0.0,          1.0 }
};

static const double acesrgb_XYZ_3[3][3] = {
    { 0.952552395938186, 0.0,                9.36786316604686e-05 },
    { 0.343966449765075, 0.728166096613485, -0.0721325463785608   },
    { 0.0,               0.0,                1.00882518435159     }
};

static const double chromaticitiesACES[4][2] = {
    { 0.73470,  0.26530 },
    { 0.00000,  1.00000 },
    { 0.00010, -0.07700 },
    { 0.32168,  0.33767 }
};

// Roberson UV Table
static const double Robertson_uvtTable[][3] = {
    { 0.18006,  0.26352,   -0.24341 },
    { 0.18066,  0.26589,   -0.25479 },
    { 0.18133,  0.26846,   -0.26876 },
    { 0.18208,  0.27119,   -0.28539 },
    { 0.18293,  0.27407,   -0.3047  },
    { 0.18388,  0.27709,   -0.32675 },
    { 0.18494,  0.28021,   -0.35156 },
    { 0.18611,  0.28342,   -0.37915 },
    { 0.18740,  0.28668,   -0.40955 },
    { 0.18880,  0.28997,   -0.44278 },
    { 0.19032,  0.29326,   -0.47888 },
    { 0.19462,  0.30141,   -0.58204 },
    { 0.19962,  0.30921,   -0.70471 },
    { 0.20525,  0.31647,   -0.84901 },
    { 0.21142,  0.32312,   -1.0182  },
    { 0.21807,  0.32909,   -1.2168  },
    { 0.22511,  0.33439,   -1.4512  },
    { 0.23247,  0.33904,   -1.7298  },
    { 0.24010,  0.34308,   -2.0637  },
    { 0.24792,  0.34655,   -2.4681  },
    { 0.25591,  0.34951,   -2.9641  },
    { 0.26400,  0.35200,   -3.5814  },
    { 0.27218,  0.35407,   -4.3633  },
    { 0.28039,  0.35577,   -5.3762  },
    { 0.28863,  0.35714,   -6.7262  },
    { 0.29685,  0.35823,   -8.5955  },
    { 0.30505,  0.35907,  -11.324   },
    { 0.31320,  0.35968,  -15.628   },
    { 0.32129,  0.36011,  -23.325   },
    { 0.32931,  0.36038,  -40.77    },
    { 0.33724,  0.36051, -116.45    }
};

// Roberson Mired Matrix
static const double RobertsonMired[] = {
      1.0e-10, 10.0,  20.0,  30.0,  40.0,  50.0,  60.0,  70.0,
     80.0,     90.0, 100.0, 125.0, 150.0, 175.0, 200.0, 225.0,
    250.0,    275.0, 300.0, 325.0, 350.0, 375.0, 400.0, 425.0,
    450.0,    475.0, 500.0, 525.0, 550.0, 575.0, 600.0
};

//  Color Adaptation Matrix - Bradford
static const double bradford[3][3] = {
    { 0.8951,  0.2664, -0.1614},
    {-0.7502,  1.7135,  0.0367},
    { 0.0389, -0.0685,  1.0296}
};

//  Color Adaptation Matrices - CAT02 (default)
static const std::vector<std::vector<double>> CAT02 = {
    {  0.7328, 0.4296, -0.1624 },
    { -0.7036, 1.6975,  0.0061 },
    {  0.0030, 0.0136,  0.9834 }
};

static const std::vector<std::vector<double>> CAT02_inv = {
    {  1.0961238208355142,    -0.27886900021828726,   0.18274517938277304 },
    {  0.45436904197535921,    0.47353315430741177,   0.072097803717229125 },
    { -0.0096276087384293551, -0.0056980312161134198, 1.0153256399545427 }
};

// clang-format on

// Function to Open Directories
inline std::vector<std::string> openDir( std::string path = "." )
{
    std::vector<std::string> paths;

    if ( std::filesystem::exists( path ) )
    {
        if ( std::filesystem::is_directory( path ) )
        {
            auto it = std::filesystem::directory_iterator( path );
            for ( auto filename: it )
            {
                if ( !std::filesystem::is_directory( filename ) )
                {
                    paths.push_back( filename.path().string() );
                }
            }
        }
    }

    return paths;
};

// Function to clear the memories occupied by vectors
template <typename T> inline void clearVM( std::vector<T> vct )
{
    std::vector<T>().swap( vct );
};

// Function to covert upper-case to lower-case
inline void lowerCase( char *tex )
{
    std::string tmp( tex );

    FORI( tmp.size() )
    tex[i] = tolower( tex[i] );
};

// Function to check if a value is numeric
inline bool isNumeric( const char *val )
{
    const std::string base = "0123456789E-.";
    std::string       input( val );

    return (
        input.find_first_not_of( base.substr( 0, base.size() ) ) ==
        std::string::npos );
};

// Function to check if a input is a alphabetic letter
inline bool isCTLetterDigit( const char c )
{
    return (
        ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c == '-' ) ||
        isdigit( c ) );
};

// Function to check if a string is a valid input
// to represent color temperature(s) (e.g., D60, 3200K)
inline bool isValidCT( std::string str )
{
    int    i      = 0;
    size_t length = str.length();

    if ( length == 0 )
        return false;

    bool first_is_D = std::tolower( str.front() ) == 'd';
    bool last_is_K  = std::tolower( str.back() ) == 'k';

    // other light sources
    if ( !first_is_D && !last_is_K )
    {
        while ( str[i] )
        {
            if ( !isCTLetterDigit( str[i] ) )
                return false;
            i++;
        }
    }
    // daylight ("D" + Numeric values)
    else if ( first_is_D && !last_is_K )
    {
        i = 1;
        while ( str[i] )
        {
            if ( !isdigit( str[i] ) )
                return false;
            i++;
        }
    }
    // daylight (Numeric values + "K")
    else if ( !first_is_D && last_is_K )
    {
        while ( str[i] && i < length - 1 )
        {
            if ( !isdigit( str[i] ) )
                return false;
            i++;
        }
    }
    // It is rarely for "D" and "K" appear
    // together as the first letter and
    // the last letter of the string
    else if ( first_is_D && last_is_K )
        return false;

    return true;
};

} // namespace core
} // namespace rta

#endif
