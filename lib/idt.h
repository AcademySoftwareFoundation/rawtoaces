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
    class Spst {
        public:
            Spst();
            Spst(Spst& spstobject) : _brand(spstobject._brand),
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
        
            ~Spst(){};
        
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
            void setSensitivity(vector<RGBSen> rgbsen);
        
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
        
            // Matrix converting ACES RGB relative exposure values to CIE XYZ tristimulus values.
            float ** aces_3_XYZt_mat();
        
            // If output encoding is XYZt by default w is the CIE XYZ tristimulus values of adopted white.
            // ls -> Spectral power distribution of the illuminant or source.
            // wl -> Wavelenghts corresponding to SPEC.
            float * XYZt_illum(lightsrc &ls);
        
            // Returns a 3x3 Von Kries chromatic adaptation transform matrix
            float ** calc_cat_mat(illum src, illum desc, float CAT[3][3]);
        
            // Converts from CIE XYZ tristimulus values to CIE L*a*b*
            CIELab XYZt_2_Lab(vector<CIEXYZ> XYZt, CIEXYZ XYZw);
            float ** gen_final_idt(vector<float> B_final);
        
            void load_cameraspst_data(const string &path, const char * maker, const char * model);
            void load_training_spectral(const string &path);
            void load_CMF(const string &path);
        
        private:
            string _outputEncoding;
            lightsrc _lightSource;
            Spst     _cameraSpst;
            vector<trainSpec> _trainingSpec;
            vector<CMF> _cmf;
        
            vector<float> _encodingWhite;
            vector<float> _WB_start;
        
            float _CAT[3][3];
    };

}

