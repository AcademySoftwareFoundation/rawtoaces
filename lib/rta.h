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

#include "mathOps.h"

using namespace std;
using namespace ceres;

namespace rta {
    class Idt;
    class Spst {
        friend class Idt;
        
        public:
            Spst();
            Spst(const Spst& spstobject) : _brand(spstobject._brand),
                                           _model(spstobject._model),
                                           _increment(spstobject._increment),
                                           _rgbsen(spstobject._rgbsen){ };
        
            Spst(char * brand,
                 char * model,
                 uint8_t increment,
                 vector<RGBSen> rgbsen) : _brand(brand),
                                          _model(model),
                                          _increment(increment),
                                          _rgbsen(rgbsen){ };
        
            ~Spst();
        
            const char * getBrand() const;
            const char * getModel() const;
            const uint8_t getWLIncrement() const;
            const vector <RGBSen> & getSensitivity() const;
        
            char * getBrand();
            char * getModel();
            uint8_t getWLIncrement();
            vector<RGBSen> & getSensitivity();
        
            void setBrand(const char * brand);
            void setModel(const char * model);
            void setWLIncrement(const uint8_t inc);
            void setSensitivity(const vector<RGBSen> rgbsen);
        
        private:
            char * _brand;
            char * _model;
            uint8_t _increment;
            uint8_t _spstMaxCol;
            vector<RGBSen> _rgbsen;
    };

    class Idt {
        public:
            Idt();
            ~Idt();
        
            bool loadCameraSpst(const string &path,
                                const char * maker,
                                const char * model);
            bool loadIlluminate(const string &path,
                                const char * type="unknown");
            void loadTrainingData(const string &path);
            void loadCMF(const string &path);
        
            void chooseIlluminate(map< string,
                                  vector<double> >& illuCM,
                                  vector<double>& src);
            void scaleLSC();
            void calWB(const char * illumType);
        
            vector< double > calCM();
            vector< vector<double> > calTI() const;
            vector< vector<double> > calCAT(vector<double> src,
                                            vector<double> des) const;
            vector< vector<double> > calXYZ(vector< vector<double> > TI) const;
            vector< vector<double> > calRGB(vector< vector<double> > TI) const;
        
            bool curveFit(vector< vector<double> > RGB,
                          vector< vector<double> > XYZ,
                          double * BStart);
            bool calIDT();
        
            const Spst getCameraSpst() const;
            const illum getIlluminate() const;

            const vector< vector<double> > getIDT() const;
            const vector< double > getWB() const;


        private:
            string  _outputEncoding;
//            string  _illumType;
//            string  _cameraPath;
            string  _bestIllum;
            Spst    _cameraSpst;
            illum   _illuminate;
        
            vector< CMF > _cmf;
            vector< trainSpec > _trainingSpec;
            vector< double > _wb;
            vector< vector< double > > _idt;
    };
    
    
    class DNG {
        public:
            DNG();
            ~DNG();
        
            vector<double> xyToXYZ(const vector<double>& xy);
            vector<double> uvToXYZ(const vector<double>& uv);
            vector<double> uvToxy(const vector<double>& uv);
            vector<double> XYZTouv(const vector<double> &XYZ);
            vector<double> XYZtoCameraWeightedMatrix(const float &mir,
                                                     const float &mir1,
                                                     const float &mir2);
            vector<double> colorTemperatureToXYZ(const double cct) const;
            vector<double> findXYZtoCameraMtx(const vector<double> neutralRGB) const;
            vector<double> matrixRGBtoXYZ(const vector< vector<double> > chromaticities) const;
            vector<double> matrixChromaticAdaptation(const vector<double> &whiteFrom,
                                                     const vector<double> &whiteTo);
        

            double ccttoMired(const double cct) const;
            double robertsonLength(const vector<double> &uv,
                                   const vector<double>& uvt) const;
            double lightSourceToColorTemp(const unsigned short tag) const;
            double XYZToColorTemperature(const vector<double> &XYZ);
        
            void prepareMatrices();
            void getCameraXYZMtxAndWhitePoint(double baseExpo);

        
        private:
            vector<double> CAT;
    };
    
    class Objfun {
        public:
            Objfun(vector< vector<double> > RGB,
                   vector< vector<double> > XYZ,
                   vector< vector<double> > M): _RGB(RGB), _XYZ(XYZ), _M(M) { }
        
            vector < vector<double> > XYZtoLAB( const vector < vector<double> >& XYZ ) const
            {
                assert(XYZ.size() == 190);
                double add = 16.0/116.0;
                
                vector< vector<double> > tmpXYZ(190, vector<double>(3, 1.0));
                FORI(190) {
                    FORJ(3)
                    {
                        tmpXYZ[i][j] = XYZ[i][j] / XYZ_w[j];
                        if (tmpXYZ[i][j] > e)
                            tmpXYZ[i][j] = std::pow(tmpXYZ[i][j], 1.0/3.0);
                        else
                            tmpXYZ[i][j] = k * tmpXYZ[i][j] + add;
                    }
                }
            
                vector< vector<double> > outCalcLab(190, vector<double>(3, 1.0));
                FORI(190)
                {
                    outCalcLab[i][0] = 116.0 * tmpXYZ[i][1]  - 16.0;
                    outCalcLab[i][1] = 500.0 * (tmpXYZ[i][0] - tmpXYZ[i][1]);
                    outCalcLab[i][2] = 200.0 * (tmpXYZ[i][1] - tmpXYZ[i][2]);
                }
                
                clearVM(tmpXYZ);
                
                return outCalcLab;
            }
        
            vector< vector<double> > getCalcXYZt(const vector < vector<double> > RGB,
                                                 const double * const B) const
            {
                assert(RGB.size() == 190);
                vector < vector<double> > BV( 3, vector< double >(3) );
            
                BV[0][0] = B[0];
                BV[0][1] = B[1];
                BV[0][2] = 1.0 - B[0] - B[1];
                BV[1][0] = B[2];
                BV[1][1] = B[3];
                BV[1][2] = 1.0 - B[2] - B[3];
                BV[2][0] = B[4];
                BV[2][1] = B[5];
                BV[2][2] = 1.0 - B[4] - B[5];
            
                vector< vector<double> > outCalcXYZt = transposeVec(mulVector(mulVector(_M, BV),
                                                                    transposeVec(transposeVec(RGB))));
                clearVM(BV);
                return outCalcXYZt;
            }
        
            double findDistance(const vector < vector<double> > RGB,
                                const vector < vector<double> > outLAB,
                                const double * const B) const
            {
                assert(RGB.size() == 190);
                vector < vector<double> > BV(3, vector<double>(3));
                
                BV[0][0] = B[0];
                BV[0][1] = B[1];
                BV[0][2] = 1.0 - B[0] - B[1];
                BV[1][0] = B[2];
                BV[1][1] = B[3];
                BV[1][2] = 1.0 - B[2] - B[3];
                BV[2][0] = B[4];
                BV[2][1] = B[5];
                BV[2][2] = 1.0 - B[4] - B[5];
                
                vector< vector<double> > outCalcXYZt = transposeVec(mulVector(mulVector(_M, BV),
                                                                              transposeVec(transposeVec(RGB))));
                vector< vector<double> > outCalcLAB = XYZtoLAB(outCalcXYZt);

                double dist = 0.0;
                double maxD = numeric_limits<double>::min();
                
                FORI(190) {
                    double deltaE = 0.0;
                    FORJ(3) {
                        deltaE += std::pow((outLAB[i][j] - outCalcLAB[i][j]), 2.0);
                    }
                    
                    if (maxD < std::pow(deltaE, 1.0/2.0))
                        maxD = std::pow(deltaE, 1.0/2.0);
                    
                    dist += std::pow(deltaE, 1.0/2.0);
                }
                
//                printf("Max: %f, \n", maxD);
//                printf("Mean: %f, \n", dist/190.0);

                clearVM(BV);
                clearVM(outCalcXYZt);
                clearVM(outCalcLAB);

                return dist;
            }
        
            bool operator()(const double* const B,
                            double* residual) const
            {
//                residual[0] = findDistance(_RGB, XYZtoLAB(_XYZ), B);
                
                vector < vector<double> > outLAB = XYZtoLAB(_XYZ);
                vector < vector<double> > outCalcLAB = XYZtoLAB(getCalcXYZt(_RGB, B));

                FORI(190) {
                    residual[i] = 0.0;
                    FORJ(3) {
                        residual[i] += std::pow((outLAB[i][j] - outCalcLAB[i][j]), 2.0);
//                        residual[i*3+j] = outLAB[i][j] - outCalcLAB[i][j];
                    }
                    residual[i] = std::pow(residual[i], 1.0/4.0);
                }
                
                return true;
            }
        
        private:
            const vector< vector<double> > _RGB;
            const vector< vector<double> > _XYZ;
            const vector< vector<double> > _M;
        };
}
#endif
