///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Academy of Motion Picture Arts and Sciences 
// ("A.M.P.A.S."). Portions contributed by others as indicated.
// All rights reserved.
// 
// A worldwide, royalty-free, non-exclusive right to copy, modify, create
// derivatives, and use, in source and binary forms, is hereby granted, 
// subject to acceptance of this license. Performance of any of the 
// aforementioned acts indicates acceptance to be bound by the following 
// terms and conditions:
//
//  * Copies of source code, in whole or in part, must retain the 
//    above copyright notice, this list of conditions and the 
//    Disclaimer of Warranty.
//
//  * Use in binary form must retain the above copyright notice, 
//    this list of conditions and the Disclaimer of Warranty in the
//    documentation and/or other materials provided with the distribution.
//
//  * Nothing in this license shall be deemed to grant any rights to 
//    trademarks, copyrights, patents, trade secrets or any other 
//    intellectual property of A.M.P.A.S. or any contributors, except 
//    as expressly stated herein.
//
//  * Neither the name "A.M.P.A.S." nor the name of any other 
//    contributors to this software may be used to endorse or promote 
//    products derivative of or based on this software without express 
//    prior written permission of A.M.P.A.S. or the contributors, as 
//    appropriate.
// 
// This license shall be construed pursuant to the laws of the State of 
// California, and any disputes related thereto shall be subject to the 
// jurisdiction of the courts therein.
//
// Disclaimer of Warranty: THIS SOFTWARE IS PROVIDED BY A.M.P.A.S. AND 
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, 
// BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS 
// FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT ARE DISCLAIMED. IN NO 
// EVENT SHALL A.M.P.A.S., OR ANY CONTRIBUTORS OR DISTRIBUTORS, BE LIABLE 
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, RESITUTIONARY, 
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
// THE POSSIBILITY OF SUCH DAMAGE.
//
// WITHOUT LIMITING THE GENERALITY OF THE FOREGOING, THE ACADEMY 
// SPECIFICALLY DISCLAIMS ANY REPRESENTATIONS OR WARRANTIES WHATSOEVER 
// RELATED TO PATENT OR OTHER INTELLECTUAL PROPERTY RIGHTS IN THE ACADEMY 
// COLOR ENCODING SYSTEM, OR APPLICATIONS THEREOF, HELD BY PARTIES OTHER 
// THAN A.M.P.A.S., WHETHER DISCLOSED OR UNDISCLOSED.
///////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <stdexcept>
#include <valarray>
#include <vector>
#include <map>
#include <numeric>
#include <limits>
#include <string>
#include <stdio.h>
#include <ctype.h>
#include <dirent.h>

#ifndef WIN32
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#endif

#define INV_255 (1.0/(double) 255.0)
#define INV_65535 (1.0/(double) 65535.0)
#define FILEPATH ("/usr/local/include/RAWTOACES/data/")

#ifdef WIN32
// suppress sprintf-related warning. sprintf() is permitted in sample code
#include <string.h>
#include <windows.h>
#define snprintf _snprintf
#define _CRT_SECURE_NO_WARNINGS
#define cmp_str stricmp
#else
#define cmp_str strcasecmp
#endif

#if !defined(TRUE)
#define TRUE 1
#endif

#if !defined(FALSE)
#define FALSE 0
#endif

#if 0
#define debug(x) (fprintf(stderr, x));

#else
#define debug(x)
#endif

#define sign(x)		((x) > 0 ? 1 : ( (x) < 0 ? (0-1) : 0))
#define countSize(a)	(sizeof(a) / sizeof((a)[0]))
#define FORL(val) for (int i=0; i < val; i++)

typedef half   float16_t;
typedef float  float32_t;
typedef double float64_t;

using namespace std;

valarray<float>  cameraCalibration1DNG = valarray<float>(1.0f, 9);
valarray<float>  cameraCalibration2DNG = valarray<float>(1.0f, 9);
valarray<float>  cameraToXYZMtx        = valarray<float>(1.0f, 9);
valarray<float>  xyz2rgbMatrix1DNG     = valarray<float>(1.0f, 9);
valarray<float>  xyz2rgbMatrix2DNG     = valarray<float>(1.0f, 9);
valarray<float>  analogBalanceDNG      = valarray<float>(1.0f, 3);
valarray<float>  neutralRGBDNG         = valarray<float>(1.0f, 3);
valarray<float>  cameraXYZWhitePoint   = valarray<float>(1.0f, 3);
valarray<float>  calibrateIllum        = valarray<float>(1.0f, 2);

struct CIEXYZ {
    float Xt;
    float Yt;
    float Zt;
};

struct CIELab {
    float L;
    float a;
    float b;
};

struct trainSpec {
    uint16_t wl;
    vector <float> data;
};

struct CMF {
    uint16_t wl;
    float xbar;
    float ybar;
    float zbar;
};

struct RGBSen {
    float RSen;
    float GSen;
    float BSen;
};

struct light {
    CIEXYZ XYZt;
    string src;
    string des;
};

struct illum {
    string type;
    uint8_t inc;
    vector <float> data;
};

struct cameraDataPath {
    string os;
    vector <string> paths;
};

template<typename T>
struct square
{
    T operator()(const T& val1, const T& val2) const
    {
        return (val1 + val2*val2);
    }
};

static const double XYZ_acesrgb_3[3][3] = {
    { 1.0634731317028,      0.00639793641966071,   -0.0157891874506841 },
    { -0.492082784686793,   1.36823709310019,      0.0913444629573544  },
    { -0.0028137154424595,  0.00463991165243123,   0.91649468506889    }
};

//static const double XYZ_acesrgb_3[3][3] = {
//    { 1.01584,   -0.01773,   0.04637 },
//    { -0.50780,  1.39129,    0.11917 },
//    { 0.00846,   -0.01404,   1.21907 }
//};

static const double XYZ_acesrgb_4[4][4] = {
    { 1.0634731317028,      0.00639793641966071,   -0.0157891874506841,     0.0 },
    { -0.492082784686793,   1.36823709310019,      0.0913444629573544,      0.0 },
    { -0.0028137154424595,  0.00463991165243123,   0.91649468506889,        0.0 },
    { 0.0,                  0.0,                   0.0,                     1.0 }

};

static const float chromaticitiesACES[4][2] = {
    { 0.73470f,     0.26530f  },
    { 0.00000f,     1.00000f  },
    { 0.00010f,     -0.07700f },
    { 0.32168f,     0.33767f  }
};

static const float Robertson_uvtTable[][3] = {
    { 0.18006f, 0.26352f, -0.24341f},
    { 0.18066f, 0.26589f, -0.25479f},
    { 0.18133f, 0.26846f, -0.26876f},
    { 0.18208f, 0.27119f, -0.28539f},
    { 0.18293f, 0.27407f, -0.3047f},
    { 0.18388f, 0.27709f, -0.32675f},
    { 0.18494f, 0.28021f, -0.35156f},
    { 0.18611f, 0.28342f, -0.37915f},
    { 0.18740f, 0.28668f, -0.40955f},
    { 0.18880f, 0.28997f, -0.44278f},
    { 0.19032f, 0.29326f, -0.47888f},
    { 0.19462f, 0.30141f, -0.58204f},
    { 0.19962f, 0.30921f, -0.70471f},
    { 0.20525f, 0.31647f, -0.84901f},
    { 0.21142f, 0.32312f, -1.0182f},
    { 0.21807f, 0.32909f, -1.2168f},
    { 0.22511f, 0.33439f, -1.4512f},
    { 0.23247f, 0.33904f, -1.7298f},
    { 0.24010f, 0.34308f, -2.0637f},
    { 0.24792f, 0.34655f, -2.4681f},
    { 0.25591f, 0.34951f, -2.9641f},
    { 0.26400f, 0.35200f, -3.5814f},
    { 0.27218f, 0.35407f, -4.3633f},
    { 0.28039f, 0.35577f, -5.3762f},
    { 0.28863f, 0.35714f, -6.7262f},
    { 0.29685f, 0.35823f, -8.5955f},
    { 0.30505f, 0.35907f, -11.324f},
    { 0.31320f, 0.35968f, -15.628f},
    { 0.32129f, 0.36011f, -23.325f},
    { 0.32931f, 0.36038f, -40.77f},
    { 0.33724f, 0.36051f, -116.45f}
};

static const float deviceWhite[3] = {1.0, 1.0, 1.0};

// Different Color Adaptation Matrices
static const float bradford[3][3] = {
    {0.8951,  0.2664, -0.1614},
    {-0.7502, 1.7135,  0.0367},
    {0.0389,  -0.0685, 1.0296}
};

static const float cat02[3][3] = {
    {0.7328,  0.4296,  -0.1624},
    {-0.7036, 1.6975,  0.0061 },
    {0.0030,  0.0136,  0.9834 }
};

static const float reillum[3][3] = {
    {1.6160, -0.3591, -0.2569},
    {-0.9542, 1.8731,  0.0811},
    {0.0170,  -0.0333, 1.0163}
};

static const float CATMatrix[3][3] = {
    { 1.0634731317028,      0.00639793641966071,   -0.0157891874506841 },
    { -0.492082784686793,   1.36823709310019,      0.0913444629573544  },
    { -0.0028137154424595,  0.00463991165243123,   0.91649468506889    }
};

