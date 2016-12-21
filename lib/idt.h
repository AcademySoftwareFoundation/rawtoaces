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

namespace idt {
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
        friend class Objfun;

        public:
            Idt();
            ~Idt();
        
            CIELab XYZt_2_Lab(vector<CIEXYZ> XYZt, CIEXYZ XYZw);
            float ** gen_final_idt(vector<float> B_final);
        
            void load_cameraspst_data(const string &path, const char * maker, const char * model);
            void load_illuminate(const string &path, const char * type="na");
            void load_training_spectral(const string &path);
            void load_CMF(const string &path);
            const Spst getCameraSpst() const;
            const illum getIllum() const;
        
            void determineIllum(map< string, vector<float> >& illuCM, const float src[]);
            void normalDayLight(vector<float>& mul);
        
            vector< float > calCM();
            vector< vector<float> > calTrainingIllum() const;
            vector< vector<float> > calRGB( vector< vector<float> > TI) const;
            vector< vector<float> > calXYZ( vector< vector<float> > TI) const;
//            vector< vector<float> > calOutLAB(vector< vector<float> > XYZ) const;
//            vector< vector<float> > calLAB(const vector < vector<float> > RGB,
//                                            const vector < vector<float> > XYZ,
//                                            const vector < vector<float> > B) const;
        
            void curveFitting(vector< vector<float> > RGB,
                              vector< vector<float> > XYZ) const;
        
        private:
            string  _outputEncoding;
            string  _bestIllum;
            Spst    _cameraSpst;
            illum   _illuminate;
        
            vector<CMF> _cmf;
            vector<trainSpec> _trainingSpec;
//            vector< vector<float> > _CAT;
        
            float _CAT[3][3];
    };
    
    class Objfun {
        public:
            Objfun(vector< vector<float> > RGB,
                   vector< vector<float> > XYZ): _RGB(RGB), _XYZ(XYZ) { }
        
            vector< vector<float> > calLAB(const vector < vector<float> > RGB,
                                           const vector < vector<float> > XYZ,
                                           const double * B) const
            {
                assert(RGB.size() == XYZ.size() && XYZ.size() == 190);
                float add = 16.0f/116.0f;
            
                vector < vector<float> > BV(3, vector<float>(3, 1.0));
                FORI(3)
                    FORJ(3) {
                        BV[i][j] = float(B[i*3+j]);
                    }
            
                vector< vector<float> > out_calc_XYZt = transposeVec(mulVector(BV, transposeVec(RGB, 190, 3), 3, 190), 3, 190);
                vector< vector<float> > tmp(190, vector<float>(3, 0.0f));
            
                FORI(190) {
                    FORJ(3)
                    {
                    tmp[i][j] = XYZ[i][j] / XYZ_w[j];
                    if (tmp[i][j] > e)
                        tmp[i][j] = pow (tmp[i][j], 1.0f/3.0f);
                    else
                        tmp[i][j] = k * tmp[i][j] + add;
                    }
                }
            
                vector< vector<float> > out_calc_Lab(190, vector<float>(3, 0.0f));
                FORI(190)
                {
                    out_calc_Lab[i][0] = 116.0f * tmp[i][1] - 16.0f;
                    out_calc_Lab[i][1] = 500.0f * (tmp[i][0] - tmp[i][1]);
                    out_calc_Lab[i][2] = 200.0f * (tmp[i][1] - tmp[i][2]);
                }
            
                return out_calc_Lab;
            }
        
            float calDistance(const vector < vector<float> > RGB,
                              const vector < vector<float> > XYZ,
                              const double * B) const
            {
                    assert(RGB.size() == XYZ.size() && XYZ.size() == 190);
                
                    vector< vector<float> > out_calc_Lab = calLAB(RGB, XYZ, B);
                
                    const vector < vector< float > > XYZ_wv = repmat2d(XYZ_w, 3, 3);
                    vector< vector<float> > out_Lab = mulVector(XYZ, XYZ_wv, 190, 3);
                
                    float dist = 0.0f;
                    FORI(190)
                        FORJ(3){
                            dist += pow((out_Lab[i][j] - out_calc_Lab[i][j]), 2);
//                            dist += (out_Lab[i][j] - out_calc_Lab[i][j]) << 1 ;
                        }
            
                    return pow(dist, 1.0f/2.0f);
            }
        
            template <typename T>
            bool operator()(const T* const B0,
                            const T* const B1,
                            const T* const B2,
                            const T* const B3,
                            const T* const B4,
                            const T* const B5,
                            T* residual) const {
                T* B;
                
                B[0] = B0[0];
                B[1] = B1[0];
                B[2] = T(1.0) - B0[0] - B1[0];
                B[3] = B2[0];
                B[4] = B3[0];
                B[5] = T(1.0) - B2[0] - B3[0];
                B[6] = B4[0];
                B[7] = B5[0];
                B[8] = T(1.0) - B4[0] - B5[0];
        
//                residual[0] = calDistance(_idt.calLAB2(_RGB, _XYZ, B));
                residual[0] = T(sqrt(10.0)) * (B0[0] - B2[0]) * (B1[0] - B3[0]) * (B4[0] - B5[0]);

        
//            template <typename T>
//            bool operator()(const T* const B_start,
//                            T* residual) const {
//                T* B;
//            
//                B[0] = (B_start[0])[0];
//                B[1] = (B_start[0])[1];
//                B[2] = T(1.0) - (B_start[0])[0] - (B_start[0])[1];
//                B[3] = (B_start[0])[2];
//                B[4] = (B_start[0])[3];
//                B[5] = T(1.0) -(B_start[0])[2] - (B_start[0])[3];
//                B[6] = (B_start[0])[4];
//                B[7] = (B_start[0])[5];
//                B[8] = T(1.0) - (B_start[0])[4] - (B_start[0])[5];
//            
////            residual[0] = T(calDistance(_idt.calLAB2(_RGB, _XYZ, B)));
//            residual[0] = T(sqrt(10.0)) * ((B_start[0])[0] - (B_start[0])[2]) * ((B_start[0])[1] - (B_start[0])[3]) * ((B_start[0])[4] - (B_start[0])[5]);


            return true;
        }
        
        private:
            const vector< vector<float> > _RGB;
            const vector< vector<float> > _XYZ;
        };
}

