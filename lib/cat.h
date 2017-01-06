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

#include "define.h"

using namespace std;
using namespace ceres;

namespace cat {
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
            void setWLIncrement(uint8_t inc);
            void setSensitivity(const vector<RGBSen> rgbsen);
        
        private:
            char * _brand;
            char * _model;
            uint8_t _increment;
            vector<RGBSen> _rgbsen;
    };

    class Idt {
        public:
            Idt();
            ~Idt();
        
            void loadCameraSpst(const string &path, const char * maker, const char * model);
            void loadIlluminate(const string &path, const char * type="NA");
            void loadTrainingData(const string &path);
            void loadCMF(const string &path);
            void scaleDaylight(vector<double>& mul);
            void chooseIlluminate(map< string, vector<double> >& illuCM, const double src[]);
        
            const Spst getCameraSpst() const;
            const illum getIlluminate() const;
        
            vector< double > calCM();
            vector< vector<double> > calTI() const;
            vector< vector<double> > calRGB( vector< vector<double> > TI) const;
            vector< vector<double> > calXYZ( vector< vector<double> > TI) const;
        
            void curveFit(vector< vector<double> > RGB,
                                  vector< vector<double> > XYZ,
                                  double * BStart);
            void getIdt();
        
        private:
            string  _outputEncoding;
            string  _bestIllum;
            Spst    _cameraSpst;
            illum   _illuminate;
        
            vector<CMF> _cmf;
            vector<trainSpec> _trainingSpec;
            double _CAT[3][3];
    };
    
    class Objfun {
        public:
            Objfun(vector< vector<double> > RGB,
                   vector< vector<double> > XYZ): _RGB(RGB), _XYZ(XYZ) { }
        
            vector< vector<double> > calLAB(const vector < vector<double> > RGB,
                                            const vector < vector<double> > B) const
            {
                assert(RGB.size() == 190);
                double add = 16.0/116.0;
                
                vector< vector<double> > outCalcXYZt = transposeVec(mulVector(B,
                                                                    transposeVec(transposeVec(RGB))));
//                vector< vector<double> > outCalcXYZt = mulVector(RGB, B);

                FORI(190) {
                    FORJ(3)
                    {
//                        printf("%f ", outCalcXYZt[i][j]);
                        outCalcXYZt[i][j] = outCalcXYZt[i][j] / XYZ_w[j];
                        if (outCalcXYZt[i][j] > e)
                            outCalcXYZt[i][j] = std::pow(outCalcXYZt[i][j], 1.0/3.0);
                        else
                            outCalcXYZt[i][j] = k * outCalcXYZt[i][j] + add;
                    }
//                    printf("\n");
                }
            
                vector< vector<double> > outCalcLab(190, vector<double>(3, 1.0));
                FORI(190)
                {
                    outCalcLab[i][0] = 116.0 * outCalcXYZt[i][1]  - 16.0;
                    outCalcLab[i][1] = 500.0 * (outCalcXYZt[i][0] - outCalcXYZt[i][1]);
                    outCalcLab[i][2] = 200.0 * (outCalcXYZt[i][1] - outCalcXYZt[i][2]);
                }
            
                return outCalcLab;
            }
        
        
            double findDistance(const vector < vector<double> > RGB,
                                const vector < vector<double> > XYZ,
                                const double * const B) const
            {
                assert(RGB.size() == XYZ.size() && XYZ.size() == 190);
                
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
                
                vector< vector<double> > outLAB = mulVector(XYZ, repmat2d(XYZ_w, 3, 3));
                vector< vector<double> > outCalcLAB = calLAB(RGB, BV);

                double dist = 0.0;
                FORI(190) {
                    FORJ(3){
//                        printf("%f ", outLAB[i][j]);
//                        dist += std::pow((outLAB[i][j] - outCalcLAB[i][j]), 2.0);
                        dist += outLAB[i][j] - outCalcLAB[i][j];
                    }
//                    printf("\n");
                }
                
//                cout << dist << endl;
//                return std::pow(dist, 1.0/2.0);
                return dist;
                
            }

//            double simpleFunction(const double* B) const
//            {
//                double BD[9];
//                
//                BD[0] = B[0];
//                BD[1] = B[1];
//                BD[2] = 1.0 - B[0] - B[1];
//                BD[3] = B[2];
//                BD[4] = B[3];
//                BD[5] = 1.0 - B[2] - B[3];
//                BD[6] = B[4];
//                BD[7] = B[5];
//                BD[8] = 1.0 - B[4] - B[5];
//
//                double tmp = 1.0 - std::pow(BD[0], 2) + BD[1] + BD[2] - BD[5] + BD[3] + (1.0 - BD[4]) + BD[6] - BD[7] + BD[8];
//
//                return tmp;
//            }
        
            bool operator()(const double* const B,
                            double* residual) const {
                
//                std::cout << double(B[0]) << ", " << double(B[5]) << ", " << "\n";
                residual[0] = findDistance(_RGB, _XYZ, B);
//                residual[0] = simpleFunction(B);
                
                return true;
            }
        
        private:
            const vector< vector<double> > _RGB;
            const vector< vector<double> > _XYZ;
        };
}

