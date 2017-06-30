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

#ifndef _RTA_h__
#define _RTA_h__

// # C++ 11:201103L, C++ 97:199711L
// #define null_ptr (__cplusplus > 201103L ? (nullptr) : 0)
#define null_ptr nullptr

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <stdint.h>
#include <math.h>
#include <cstring>
#include <half.h>
#include <ctype.h>
#include <stdlib.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

#include "mathOps.h"

using namespace std;
using namespace ceres;
using namespace boost::property_tree;

namespace rta {
    struct CIEXYZ {
        CIEXYZ() {};
        CIEXYZ( double X, double Y, double Z ) : _Xt(X),
                                                 _Yt(Y),
                                                 _Zt(Z){ };
        double _Xt;
        double _Yt;
        double _Zt;
    };
    
    struct trainSpec {
        uint16_t _wl;
        vector <double> _data;
    };
    
    struct CMF {
        uint16_t _wl;
        double _xbar;
        double _ybar;
        double _zbar;
    };
    
    struct RGBSen {
        RGBSen() {};
        RGBSen( double R, double G, double B ) : _RSen(R),
                                                 _GSen(G),
                                                 _BSen(B){ };
        
        double _RSen;
        double _GSen;
        double _BSen;
    };

    
    class Idt;

    class Illum {
        friend class Idt;
        
        public:
            Illum();
            Illum( string type );
            ~Illum();

            int loadSPD( string path, string type );
            vector <double> cctToxy( const int cct ) const;
            void calDayLightSPD( const int cct );
            
//        private:
            string _type;
            uint8_t _inc;
            double _index;
            vector <double> _data;
    };
    
    class Idt;
    
    class Spst {
        friend class Idt;
        
        public:
            Spst();
            Spst( const Spst& spstobject );
            Spst( char * brand,
                  char * model,
                  uint8_t increment,
                  vector<RGBSen> rgbsen ) : _brand(brand),
                                            _model(model),
                                            _increment(increment),
                                            _rgbsen(rgbsen){ };
            ~Spst();

            void setBrand ( const char * brand );
            void setModel ( const char * model );
            void setWLIncrement ( const uint8_t inc );
            void setSensitivity ( const vector < RGBSen > rgbsen );
        
            const char * getBrand() const;
            const char * getModel() const;
            const uint8_t getWLIncrement() const;
            const vector < RGBSen > & getSensitivity() const;
        
            char * getBrand();
            char * getModel();
        
            int getWLIncrement();
            int loadSpst( string path,
                          const char * maker,
                          const char * model );
        
            vector < RGBSen > & getSensitivity();

        private:
            char * _brand;
            char * _model;
            int _increment;
            int _spstMaxCol;
            vector < RGBSen > _rgbsen;
    };

    class Idt {
        public:
            Idt();
            ~Idt();
        
            int loadCameraSpst( const string &path,
                                const char * maker,
                                const char * model );
            int loadIlluminant( const string &path,
                                const string type="na" );
            void loadTrainingData( const string &path );
            void loadCMF( const string &path );
        
            void chooseIllumSrc( unordered_map < string, vector<double> >& illuCM,
                                 vector<double>& src );
            void chooseIllumType( unordered_map < string, vector<double> >& illuCM,
                                  const char * type );
            void setVerbosity( int verbosity );
            void scaleLSC();
        
            vector< double > calCM();
            vector< vector<double> > calTI() const;
            vector< double > calWB( int highlight );
            vector< vector<double> > calCAT( vector<double> src,
                                             vector<double> des ) const;
            vector< vector<double> > calXYZ( vector< vector<double> > TI ) const;
            vector< vector<double> > calRGB( vector< vector<double> > TI ) const;
        
            int curveFit( vector< vector<double> > RGB,
                          vector< vector<double> > XYZ,
                          double * BStart );
            int calIDT();
        
            const Spst getCameraSpst() const;
            const Illum getIlluminant() const;
            const int getVerbosity() const;
            const vector< vector <double> > getIDT() const;
            const vector< double > getWB() const;

            Spst getCameraSpst();
            Illum getIlluminant();
            int getVerbosity();
        
        private:
            Spst    _cameraSpst;
            Illum   _Illuminant;
            string  _bestIllum;
            int     _verbosity;
        
            vector < CMF > _cmf;
            vector < trainSpec > _trainingSpec;
            vector < double > _wb;
            vector < vector< double > > _idt;
    };
    
    
    class DNG {
        public:
            DNG();
            ~DNG();
        
            vector<double> xyToXYZ( const vector<double>& xy );
            vector<double> uvToXYZ( const vector<double>& uv );
            vector<double> uvToxy( const vector<double>& uv );
            vector<double> XYZTouv( const vector<double> &XYZ) ;
            vector<double> XYZtoCameraWeightedMatrix( const float &mir,
                                                      const float &mir1,
                                                      const float &mir2 );
            vector<double> colorTemperatureToXYZ( const double cct ) const;
            vector<double> findXYZtoCameraMtx( const vector<double> neutralRGB ) const;
            vector<double> matrixRGBtoXYZ( const vector< vector<double> > chromaticities ) const;
            vector<double> matrixChromaticAdaptation( const vector<double> &whiteFrom,
                                                      const vector<double> &whiteTo );
        

            double ccttoMired( const double cct ) const;
            double robertsonLength( const vector<double> &uv,
                                    const vector<double>& uvt ) const;
            double lightSourceToColorTemp( const unsigned short tag ) const;
            double XYZToColorTemperature( const vector<double> &XYZ );
        
            void prepareMatrices();
            void getCameraXYZMtxAndWhitePoint( double baseExpo );
        
        private:
            vector<double> CAT;
    };
    
    struct Objfun {
            Objfun(vector< vector<double> > RGB,
                   vector< vector<double> > outLAB): _RGB(RGB), _outLAB(outLAB) { }
        
            template<typename T>
            bool operator() (const T* B,
                             T * residuals) const
            {
                vector < vector <T> > RGBJet(190, vector< T >(3));
                FORIJ(190, 3) RGBJet[i][j] = T(_RGB[i][j]);
            
                vector < vector <T> > outCalcLAB = XYZtoLAB(getCalcXYZt(RGBJet, B));
                FORIJ(190, 3) residuals[i * 3 + j] = _outLAB[i][j] - outCalcLAB[i][j];

                return true;
           }
        
            const vector< vector<double> > _RGB;
            const vector< vector<double> > _outLAB;
   };
    
}
#endif
