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

#include "idt.h"
#include "assert.h"

#if 0
    #define debug(x) (cout << x << endl)
#else
    #define debug(x)
#endif


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

namespace idt {
    const char * Spst::showBrand() {
        return this->_brand;
    }
    
    const char * Spst::showModel() {
        return this->_model;
    }
    
    const valarray <float> Spst::showSensitivity() {
        return this->_sensitivity;
    }

    Idt::Idt() {
        _outputEncoding = "ACES";
        _encodingWhite = valarray <float> (0.0, 3);
        _B_start = valarray <float> (0.0, 3);
        *_CAT[0] = CATMatrix[0][0];
    }
    
    float ** Idt::aces_3_XYZt_mat() {
        return 0;
    }

    
    float * Idt::XYZt_illum(lightsrc &ls) {
        valarray<float> wl = ls.wavelength;
        return 0;
    }
    
    float ** calc_cat_mat(illum src,
                          illum desc,
                          float CAT[3][3]) {
        
        float **vkmat = new float*[3];
        
        for(uint8_t i=0; i<3; i++){
            vkmat[i] = new float[3];
        }
        
        return vkmat;
    }
    
    CIELab Idt::XYZt_2_Lab(valarray<CIEXYZ> XYZt, CIEXYZ XYZw) {
        CIELab lab = {1.0, 1.0, 1.0};
//        static CIELab lab = {1.0, 1.0, 1.0};
//        CIELab *lab = static_cast<CIELab>(malloc(1 * sizeof(CIELab)));
        
        return lab;
    }
    
    void Idt::readspstdata(const string & path){
        ifstream fin;
        fin.open(path);
        
        int i = 0;
        
        if(!fin.good()) {
            debug("The file may not exist.\n");
            exit(EXIT_FAILURE);
        }
        
        while(!fin.eof()){
            char buffer[512];
            fin.getline(buffer, 512);
            
            const char* token[43] = {};
            token[0] = strtok(buffer, " ,-");
            assert(token[0]);
            
            valarray <float> sensitivity;
            
            for (int n = 1; n < 43; n++){
                token[n] = strtok(nullptr, " ,-");
                if(!token[n] && n >= 2) {
                    sensitivity[n-2] = atof(token[n]);
                }
                else if(!token[n]) {
//                    continue;
                    debug("The spectral sensitivity file may need to be looked at\n");
                    exit(EXIT_FAILURE);
                }
            }
            
            Spst * tmp = new Spst(token[0], token[1], 10, (const valarray<float>)sensitivity);
            spsts[i] = tmp;
        }
    }
    
    template <typename T>
    valarray <T> repmat(valarray <T> data, uint8_t row, uint8_t col) {
        valarray <T> out(0.0, data.size()*row*col);
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                out[i*col + j] = data[i];
            }
        }
        
        return out;
    }
}





