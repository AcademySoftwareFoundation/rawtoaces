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

namespace idt {
    const char * Spst::getBrand() const {
        return this->_brand;
    }
    
    const char * Spst::getModel() const {
        return this->_model;
    }
    
    const vector <RGBSen>& Spst::getSensitivity() const {
        return this->_sensitivity;
    }
    
    char * Spst::getBrand() {
        return this->_brand;
    }
    
    char * Spst::getModel() {
        return this->_model;
    }
    
    vector <RGBSen>& Spst::getSensitivity() {
        return this->_sensitivity;
    }

    Idt::Idt() {
        _outputEncoding = "ACES";
        _encodingWhite = vector <float> (0.0, 3);
        _WB_start = vector <float> (0.0, 3);
        *_CAT[0] = CATMatrix[0][0];
    }
    
    float ** Idt::aces_3_XYZt_mat() {
        return 0;
    }

    
    float * Idt::XYZt_illum(lightsrc &ls) {
        float* wl = ls.wavelength;
        return 0;
    }
    
    float ** Idt::calc_cat_mat(illum src,
                          illum desc,
                          float CAT[3][3]) {
        
        float **vkmat = new float*[3];
        
        for(uint8_t i=0; i<3; i++){
            vkmat[i] = new float[3];
        }
        
        return vkmat;
    }
    
    CIELab Idt::XYZt_2_Lab(vector<CIEXYZ> XYZt, CIEXYZ XYZw) {
        CIELab CLab = {1.0, 1.0, 1.0};
//        static CIELab lab = {1.0, 1.0, 1.0};
//        CIELab *lab = static_cast<CIELab>(malloc(1 * sizeof(CIELab)));
        
        return CLab;
    }
    
//    void Idt::readspstdata(const string & path){
//        ifstream fin;
//        fin.open(path);
//        
//        int i = 0;
//        
//        if(!fin.good()) {
//            debug("The file may not exist.\n");
//            exit(EXIT_FAILURE);
//        }
//        
//        vector <RGBSen> rgbsen;
//        
//        while(!fin.eof()){
//            char buffer[512];
//            fin.getline(buffer, 512);
//            
//            char* token[5] = {};
//            token[0] = strtok(buffer, " ,-");
//            assert(token[0]);
//            
//            for (int n = 0; n < 3; n++){
//                token[n] = strtok(null_ptr, " ,-");
//                if(token[n] && n == 2) {
//                    rgbsen[i].RSen = atof(token[n]);
//                }
//                else if(token[n] && n == 3) {
//                    rgbsen[i].GSen = atof(token[n]);
//                }
//                else if(token[n] && n == 4) {
//                    rgbsen[i].BSen = atof(token[n]);
//                }
//                else if(!token[n]) {
////                    continue;
//                    debug("The spectral sensitivity file may need to be looked at\n");
//                    exit(EXIT_FAILURE);
//                }
//            }
//            
//            fin.close();
//
//            Spst * tmp = new Spst(token[0], token[1], 10, rgbsen);
//            spsts[i++] = tmp;
//        }
//    }
    
    void Idt::load_training_spectral(const char * path){
        ifstream fin;
        fin.open(path);
        
        int i = 0;
        
        if(!fin.good()) {
            debug("The file may not exist.\n");
            exit(EXIT_FAILURE);
        }
        
        while(!fin.eof()){
            char buffer[4096];
            fin.getline(buffer, 4096);
            
            char* token[121] = {};
            token[0] = strtok(buffer, " ,-");
            cout << token[0] << endl;
            assert(token[0]);
            
            // pay attention here
//            _trainingSpec[i].wl = atof(token[0]);

//            for (int n = 1; n < 121; n++){
//                token[n] = strtok(null_ptr, " ,-");
//                if(token[n]) {
//                    this->_trainingSpec[i].data[n-1] = atof(token[n]);
//                }
//                else {
//                    debug("The training spectral sensitivity file may need to be looked at\n");
//                    exit(EXIT_FAILURE);
//                }
//            }
//            i += 1;
        }
        
        fin.close();
    }
    
    void Idt::load_CMF(const char * path){
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
            
            char* token[4] = {};
            token[0] = strtok(buffer, " ,-");
            assert(token[0]);
            this->_trainingSpec[i].wl = atof(token[0]);
            
            for (int n = 1; n < 4; n++){
                token[n] = strtok(null_ptr, " ,-");
                if(token[n] && n == 1) {
                    this->_cmf[i].xbar = atof(token[n]);
                }
                else if(token[n] && n == 2) {
                    this->_cmf[i].ybar = atof(token[n]);
                }
                else if(token[n] && n == 3) {
                    this->_cmf[i].zbar = atof(token[n]);
                }
                else {
                    debug("The color matching function file may need to be looked at\n");
                    exit(EXIT_FAILURE);
                }
            }
            i += 1;
        }
        
        fin.close();
    }
    
    
    // Non-class functions
    template <typename T>
    vector <T> repmat(vector <T> data, uint8_t row, uint8_t col) {
        vector <T> out(0.0, data.size()*row*col);
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                out[i*col + j] = data[i];
            }
        }
        
        return out;
    }
    
    cameraDataPath& cameraPathsFinder() {
        static cameraDataPath cdp;
        static bool firstTime = 1;
        
        if(firstTime)
        {
            vector <string>& cPaths = cdp.paths;
            
            string path;
            const char* env = getenv("RAWTOACES_CAMERA_PATH");
            if (env)
                path = env;
            
            if (path == "") {
                #if defined (WIN32) || defined (WIN64)
                path = ".";
                cdp.os = "WIN";
                #else
                path = ".:/usr/local/lib/RAWTOACES:/usr/local" PACKAGE "-" VERSION "/lib/RAWTOACES";
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
                
                if(find(cPaths.begin(), cPaths.end(), pathItem) == cPaths.end())
                    cPaths.push_back(pathItem);
                
                pos = end + 1;
            }
        }
        return cdp;
    }
}





