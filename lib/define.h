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
#ifndef _DEFINE_h__
#define _DEFINE_h__

#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <stdexcept>
#include <vector>
#include <map>
#include <numeric>
#include <limits>
#include <string>
#include <stdio.h>
#include <ctype.h>
#include <dirent.h>
#include <half.h>
#include <Eigen/Core>
#include <glog/logging.h>
#include <ceres/ceres.h>

#ifndef WIN32
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#endif

#define INV_255 (1.0/(double) 255.0)
#define INV_65535 (1.0/(double) 65535.0)
#define FILEPATH ("/usr/local/include/rawtoaces/data/")

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

#define sign(x)		((x) > 0 ? 1 : ( (x) < 0 ? (0-1) : 0))
#define FORI(val) for (int i=0; i < val; i++)
#define FORJ(val) for (int j=0; j < val; j++)
#define FORIJ(val1, val2) for (int i=0; i < val1; i++) for (int j=0; j < val2; j++)

//typedef half   float16_t;
typedef float  float32_t;
typedef double float64_t;

using namespace std;
using ceres::AutoDiffCostFunction;
using ceres::CostFunction;
using ceres::CauchyLoss;
using ceres::Problem;
using ceres::Solve;
using ceres::Solver;

struct option {
    int ret;
    int use_bigfile;
    int use_timing;
    int use_camera_path;
    int use_illum;
    int use_Mul;
    int use_mat;
    int use_wb;
    int use_mmap;
    int msize;
    int verbosity;
    int highlight;
    
    char * cameraSenPath;
    char * illumType;
    
    float scale;
    vector <string> EnvPaths, cEnvPaths, iEnvPaths;
};

struct CIEXYZ {
    double Xt;
    double Yt;
    double Zt;
    
    CIEXYZ() {};
    CIEXYZ( double X, double Y, double Z ) : Xt(X),
                                             Yt(Y),
                                             Zt(Z){ };
};

struct trainSpec {
    uint16_t wl;
    vector <double> data;
};

struct CMF {
    uint16_t wl;
    double xbar;
    double ybar;
    double zbar;
};

struct RGBSen {
    double RSen;
    double GSen;
    double BSen;
    
    RGBSen() {};
    RGBSen( double r, double g, double b ) : RSen(r),
                                             GSen(g),
                                             BSen(b){ };
};

struct illum {
    string path;
    string type;
    uint8_t inc;
    double index;
    vector <double> data;
};

struct dataPath {
    string os;
    vector <string> paths;
};

const double e = 216.0/24389.0;
const double k = (24389.0/27.0)/116.0;
const double dmin = numeric_limits<double>::min();
const double dmax = numeric_limits<double>::max();

static map < const string, char > keys;
static const double XYZ_w[3] = {0.952646074569846, 1.0, 1.00882518435159};
static const double d50[3] = {0.9642, 1.0000, 0.8250};
static const double d60[3] = {0.952646074569846, 1.0000, 1.00882518435159};
static const char *lightS[12] = { "3200k", "cie15-a", "cie15-c",
                                  "cie15-d50", "cie15-d55", "cie15-d65",
                                  "cie15-d75", "d40", "d45",
                                  "d50", "d60", "iso7589" };

static const double neutral3[3][3] = {
    {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 0.0, 1.0}
};

static const double XYZ_acesrgb_3[3][3] = {
    { 1.0634731317028,      0.00639793641966071,   -0.0157891874506841 },
    { -0.492082784686793,   1.36823709310019,      0.0913444629573544  },
    { -0.0028137154424595,  0.00463991165243123,   0.91649468506889    }
};

static const double XYZ_acesrgb_4[4][4] = {
    { 1.0634731317028,      0.00639793641966071,   -0.0157891874506841,     0.0 },
    { -0.492082784686793,   1.36823709310019,      0.0913444629573544,      0.0 },
    { -0.0028137154424595,  0.00463991165243123,   0.91649468506889,        0.0 },
    { 0.0,                  0.0,                   0.0,                     1.0 }

};

static const double acesrgb_XYZ_3[3][3] = {
    { 0.952552395938186,    0.0,                   9.36786316604686e-05 },
    { 0.343966449765075,    0.728166096613485,     -0.0721325463785608  },
    { 0.0,                             0.0,        1.00882518435159   }
};

//  Color Adaptation Matrices - Bradford
static const double bradford[3][3] = {
    {0.8951,  0.2664, -0.1614},
    {-0.7502, 1.7135,  0.0367},
    {0.0389,  -0.0685, 1.0296}
};

//  Color Adaptation Matrices - Cat02 (default)
static const double cat02[3][3] = {
    {0.7328,  0.4296,  -0.1624},
    {-0.7036, 1.6975,  0.0061 },
    {0.0030,  0.0136,  0.9834 }
};

// Function to Open Directories
inline vector<string> openDir (string path = ".") {
    DIR *    dir;
    dirent * pDir;
    struct stat fStat;
    vector <string> fPaths;
    
    dir = opendir(path.c_str());
    
    while ((pDir = readdir(dir))) {
        string fPath = path + "/" + pDir->d_name;
        if (stat(fPath.c_str(), &fStat))
            continue;
        if (S_ISDIR( fStat.st_mode ))
            continue;
        fPaths.push_back(fPath);
    }
    
    return fPaths;
};

// Function to clear the memories occupied by vectors
template<typename T>
inline void clearVM (vector<T> vct) {
    vector< T >().swap(vct);
};

// Function to print out the version number
inline void printVS (const vector <string> vs) {
    assert (vs.size() > 0);
    printf("\nThe following options are available:\n\n");
    
    FORI ( vs.size() ) printf("%s\n", vs[i].c_str());
    printf ("\n");
};

// Function to covert upper-case to lower-case
inline void lowerCase (char * tex)
{
    string tmp(tex);
    
    FORI(tmp.size())
        tex[i] = tolower(tex[i]);
};

// Function to check if a value is numeric
inline bool isNumeric ( const char * val )
{
    string base = "0123456789E-.";
    string input(val);
    
    return (input.find_first_not_of(base.substr(0, base.size())) == string::npos);
};

// Function to get environment variable for camera data
inline dataPath& pathsFinder ( )
{
    static dataPath cdp;
    static bool firstTime = 1;
    
    if(firstTime)
    {
        string path;
        const char * env;
        
        vector <string>& PATHs = cdp.paths;
        env = getenv("AMPAS_DATA_PATH");
        
        if (env) path = env;
        
        if ( path == "" ) {
#if defined (WIN32) || defined (WIN64)
            path = ".";
            cdp.os = "WIN";
#else
            path = "/usr/local/include/rawtoaces/data:/usr/local/" PACKAGE "-" VERSION "/include/rawtoaces/data";
            cdp.os = "UNIX";
#endif
        }
        
        size_t pos = 0;
        
        while (pos < path.size()){
#if defined (WIN32) || defined (WIN64)
            size_t end = path.find(';', pos);
#else
            size_t end = path.find(':', pos);
#endif
            
            if (end == string::npos)
                end = path.size();
            
            string pathItem = path.substr(pos, end-pos);
            
            if (find(PATHs.begin(), PATHs.end(), pathItem) == PATHs.end())
                PATHs.push_back(pathItem);
            
            pos = end + 1;
        }
    }
    return cdp;
};

#endif
