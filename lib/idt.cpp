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
    Spst::Spst() {
        _brand = null_ptr;
        _model = null_ptr;
        _increment = 5;
        
        for (int i=0; i<81; i++) {
            _rgbsen.push_back(RGBSen());
        }
    }
    
    Spst::~Spst() {
        delete _brand;
        delete _model;
        
        vector< RGBSen >().swap(_rgbsen);
    }
    
    const char * Spst::getBrand() const {
        return _brand;
    }
    
    const char * Spst::getModel() const {
        return _model;
    }
    
    const uint8_t Spst::getWLIncrement() const {
        return _increment;
    }
    
    const vector <RGBSen> & Spst::getSensitivity() const {
        return _rgbsen;
    }
    
    char * Spst::getBrand() {
        return _brand;
    }
    
    char * Spst::getModel() {
        return _model;
    }
    
    uint8_t Spst::getWLIncrement() {
        return _increment;
    }
    
    vector <RGBSen> & Spst::getSensitivity() {
        return _rgbsen;
    }
    
    void Spst::setBrand(const char * brand) {
        assert(brand != null_ptr);
        uint8_t len = strlen(brand);
        
        assert(len < 64);
        
        if(len > 64)
            len = 64;
        
        _brand = (char *)malloc(len+1);
        memset(_brand, 0x0, len);
        memcpy(_brand, brand, len);
        _brand[len] = '\0';
        
        return;
    }
    
    void Spst::setModel(const char * model) {
        assert(model != null_ptr);
        uint8_t len = strlen(model);
        
        assert(len < 64);
        
        if(len > 64)
            len = 64;
        
        _model = (char *)malloc(len+1);
        memset(_model, 0x0, len);
        memcpy(_model, model, len);
        _model[len] = '\0';
  
        return;
    }
    
    void Spst::setWLIncrement(uint8_t inc){
        _increment = inc;
        
        return;
    }
    
    void Spst::setSensitivity(const vector<RGBSen> rgbsen){
        FORI(rgbsen.size()){
            _rgbsen[i] = rgbsen[i];
        }
        
        return;
    }

    Idt::Idt() {
        _outputEncoding = "ACES";
        _bestIllum = " ";
//        *_CAT[0] = CATMatrix[0][0];
        
        for (int i=0; i<81; i++) {
            _trainingSpec.push_back(trainSpec());
            _cmf.push_back(CMF());
        }
    }
    
    Idt::~Idt() {
        vector< CMF >().swap(_cmf);
        vector< trainSpec >().swap(_trainingSpec);
        
//        FORI(3) {
//            double * currentPtr = _CAT[i];
//            free(currentPtr);
//        }
//        
//        free(_CAT);
    }
    
    void Idt::load_cameraspst_data(const string & path, const char * maker, const char * model) {
        ifstream fin;
        fin.open(path);
        uint8_t line = 0;
        
        if(!fin.good()) {
            fprintf(stderr, "The file may not exist.\n");
            exit(EXIT_FAILURE);
        }
        
        vector <RGBSen> rgbsen;
        
        while(!fin.eof()){
            char buffer[512];
            fin.getline(buffer, 512);
            
            if (!buffer[0]) {
                continue;
            }
            
            RGBSen tmp_sen;
            
            char* token[3] = {};
            token[0] = strtok(buffer, " ,");
//            assert(token[0]);
            
            if(line == 0) {
                if (cmp_str(maker, static_cast<const char *>(token[0]))) {
                    fin.close();
                    return;
                }
                _cameraSpst.setBrand(static_cast<const char *>(token[0]));
            }
            else if(line == 1) {
                if (cmp_str(model, static_cast<const char *>(token[0]))) {
                    fin.close();
                    return;
                }
                _cameraSpst.setModel(static_cast<const char *>(token[0]));
            }
            else if(line == 2)
                _cameraSpst.setWLIncrement(static_cast<uint8_t>(atoi(token[0])));
            else {
                tmp_sen.RSen = atof(token[0]);
            
                token[1] = strtok(null_ptr, " ,");
                tmp_sen.GSen = atof(token[1]);
            
                token[2] = strtok(null_ptr, " ,");
                tmp_sen.BSen = atof(token[2]);
//                cout << "R:" << double(tmp_sen.RSen) << "; " << "G: " << double(tmp_sen.GSen) << "; " << "B: " << double(tmp_sen.BSen) << "\n";
                rgbsen.push_back(tmp_sen);
            }
            line++;
        }
        
        if(line != 84) {
            fprintf(stderr, "The increment should be 5nm from 380nm to 780nm.\n");
            exit(EXIT_FAILURE);
        }
        
        _cameraSpst.setSensitivity(rgbsen);
        
        fin.close();
    }
    
    void Idt::load_illuminate(const string &path, const char * type) {
        ifstream fin;
        fin.open(path);
        
        uint8_t line = 0;
        
        if(!fin.good()) {
            fprintf(stderr, "The file may not exist.\n");
            exit(EXIT_FAILURE);
        }
        
        if((_illuminate.data).size() != 0)
            (_illuminate.data).clear();
        
        while((!fin.eof())){
            char buffer[128];
            fin.getline(buffer, 128);
            
            if (!buffer[0]) {
                continue;
            }
            
            char* token;
            token = strtok(buffer, " ");
            //            assert(token);
            
            if(line == 0) {
                if (cmp_str(type, "na") &&
                    cmp_str(type, static_cast<const char *>(token))) {
                    fin.close();
                    return;
                }
                _illuminate.type = static_cast<string>(token);
            }
            else if(line == 1) {
                 _illuminate.inc = atoi(token);
            }
            else {
                 _illuminate.data.push_back(atof(token));
            }
            
            line += 1;
        }
        
//        cout << "Type: " << string(_illuminate.type) << "; " << "Inc: " << int(_illuminate.inc) << "; "<< "Data: " << "\n";
//        for (int i = 0; i < _illuminate.data.size(); i++)
//            cout << double(_illuminate.data[i]) << "," << endl;
        
        fin.close();
        
        return;
    }
    
    void Idt::load_training_spectral(const string &path) {
        ifstream fin;
        fin.open(path);
        
        uint8_t i = 0;
        uint16_t wl = 380;
        
        if(!fin.good()) {
            fprintf(stderr, "The file may not exist.\n");
            exit(EXIT_FAILURE);
        }
        
        while((!fin.eof())){
            char buffer[4096];
            fin.getline(buffer, 4096);
            
            if (!buffer[0]) {
                continue;
            }
            
            char* token[190] = {};
            token[0] = strtok(buffer, " ,");
//            assert(token[0]);
            
            _trainingSpec[i].wl = wl;
            _trainingSpec[i].data.push_back(atof(token[0]));

            for (uint8_t n = 1; n < 190; n++){
                token[n] = strtok(null_ptr, " ,");

                if(token[n]) {
                    _trainingSpec[i].data.push_back(atof(token[n]));
                }
                else {
                    fprintf(stderr, "The training spectral sensitivity file may need to be looked at\n");
                    exit(EXIT_FAILURE);
                }
            }
            i += 1;
            wl += 5;
        }
        
        fin.close();
    }
    
    void Idt::load_CMF(const string &path) {
        ifstream fin;
        fin.open(path);
        
        int i = 0;
        
        if(!fin.good()) {
            fprintf(stderr, "The file may not exist.\n");
            exit(EXIT_FAILURE);
        }
        
        while(!fin.eof()){
            char buffer[512];
            fin.getline(buffer, 512);
            
            if (!buffer[0]) {
                continue;
            }
            
            char* token[4] = {};
            token[0] = strtok(buffer, " ,");
//            assert(token[0]);
            _cmf[i].wl = (uint16_t)atoi(token[0]);
            
            if (!(_cmf[i].wl % 5)) {
                for (int n = 1; n < 4; n++){
                    token[n] = strtok(null_ptr, " ,");
                    if(token[n] && n == 1) {
                        _cmf[i].xbar = atof(token[n]);
                    }
                    else if(token[n] && n == 2) {
                        _cmf[i].ybar = atof(token[n]);
                    }
                    else if(token[n] && n == 3) {
                        _cmf[i].zbar = atof(token[n]);
                    }
                    else {
                        fprintf(stderr, "The color matching function file may need to be looked at\n");
                        exit(EXIT_FAILURE);
                    }
                }
                
//                cout << "WL: " << _cmf[i].wl << " X:" << double(_cmf[i].xbar) << "; " << "Y: " << double(_cmf[i].ybar) << "; " << "Z: " << double(_cmf[i].zbar) << "\n";

                i += 1;
            }
        }
        
        fin.close();
    }
    
    const Spst Idt::get_cameraspst() const {
        return static_cast<const Spst>(_cameraSpst);
    }
    
    const illum Idt::get_illum() const {
        return static_cast<const illum>(_illuminate);
    }
    
    void Idt::scale_day_light(vector<double>& mul){
        double max = *max_element(mul.begin(), mul.end());
        
        transform(mul.begin(), mul.end(), mul.begin(), invertD);
        transform(mul.begin(), mul.end(), mul.begin(),
                  bind1st(multiplies<double>(), max));
        
        return;
    }
    
    vector<double> Idt::cal_CM() {
        vector<RGBSen> rgbsen = _cameraSpst.getSensitivity();
        vector< vector<double> > rgbsenV(3, vector<double>(rgbsen.size(), 1.0));

        
        FORI(rgbsen.size()){
            rgbsenV[0][i] = rgbsen[i].RSen;
            rgbsenV[1][i] = rgbsen[i].GSen;
            rgbsenV[2][i] = rgbsen[i].BSen;
        }
        
        vector<double> CM = mulVector(rgbsenV,
                                      _illuminate.data,
                                      rgbsenV.size(),
                                      _illuminate.data.size());
        
        scale_day_light(CM);
        
//        cout << CM[0] << "; "
//        << CM[1] << "; "
//        << CM[2] << endl;
        
        return CM;
    }
    
    void Idt::choose_illum(map< string, vector<double> >& illuCM, const double src[]) {
        double sse = numeric_limits<double>::max();
        vector<double> vsrc(src, src+3);
        
        for (map< string, vector<double> >::iterator it = illuCM.begin(); it!=illuCM.end(); ++it){
            double tmp = calSSE(it->second, vsrc);
            if (sse > tmp) {
                sse = tmp;
                _bestIllum = it->first;
            }
        }
        
        if((_illuminate.data).size() != 0){
            _illuminate.type = "";
            _illuminate.inc = 5;
            _illuminate.data.clear();
        }

        return;
    }
    
    vector< vector<double> > Idt::cal_TI() const {
        assert(_illuminate.data.size() == 81 &&
               _trainingSpec[0].data.size() == 190);

        vector< vector<double> > TI(81, vector <double>(190));
        FORI(81) {
            vector<double> row(81, _illuminate.data[i]);
            FORJ(190){
                vector<double> col;
                for(int w=0; w < 81; w++){
                    col.push_back((_trainingSpec[w].data)[j]);
                }
                TI[i][j] = (sumVector(mulVectorElement(row, col)));
            }
        }
        
        return TI;
    }
    
    vector< vector<double> > Idt::cal_RGB(vector< vector<double> > TI) const {
        assert(TI.size() == 81);
        
        vector< vector<double> > transTI = transposeVec(TI, 81, 190);
        vector< vector<double> > colRGB(3, vector<double>(TI.size(), 1.0));
        
        FORI(81) {
            colRGB[0][i] = _cameraSpst._rgbsen[i].RSen;
            colRGB[1][i] = _cameraSpst._rgbsen[i].GSen;
            colRGB[2][i] = _cameraSpst._rgbsen[i].BSen;
        }

        vector< vector<double> > RGB = mulVector(transTI,
                                                colRGB,
                                                transTI.size(),
                                                colRGB.size());

        
//      cout << "RGB size: " << RGB.size() << "; " << "RGB.size[0] size: " << RGB.size[0].size() << endl;
//        cout << "RGB[0][0]: " << double(RGB[0][0]) << " RGB[189][2]: " << double(RGB[189][2]) << endl;

        return RGB;
    }
    
    vector< vector<double> > Idt::cal_XYZ(vector< vector<double> > TI) const {
        assert(TI.size() == 81);
        
        vector< vector<double> > transTI = transposeVec(TI, 81, 190);
        vector< vector<double> > colXYZ(3, vector<double>(TI.size(), 1.0));

        FORI(81){
            colXYZ[0][i] = _cmf[i].xbar;
            colXYZ[1][i] = _cmf[i].ybar;
            colXYZ[2][i] = _cmf[i].zbar;
        }
        
        vector< vector<double> > XYZ = mulVector(transTI,
                                                 colXYZ,
                                                 transTI.size(),
                                                 colXYZ.size());
        
//       cout << "XYZ size: " << XYZ.size() << "; " << "XYZ[0] size: " << XYZ[0].size() << endl;
//        cout << "XYZ[0][0]: " << double(XYZ[0][0]) << " XYZ[189][2]: " << double(XYZ[189][2]) << endl;
        
        return XYZ;
    }
    
    void Idt::curve_fit(vector< vector<double> > RGB,
                                vector< vector<double> > XYZ,
                                double * B)
    {
        
        Problem problem;
        
//        CostFunction* cost_function =
//        new AutoDiffCostFunction<Objfun, 1, 1, 1, 1, 1, 1, 1>(new Objfun(RGB, XYZ));
//        problem.AddResidualBlock(cost_function,
//                                 new CauchyLoss(0.5),
//                                 &B_start0,
//                                 &B_start1,
//                                 &B_start2,
//                                 &B_start3,
//                                 &B_start4,
//                                 &B_start5);
        
        CostFunction* cost_function =
                new NumericDiffCostFunction<Objfun, CENTRAL, 1, 6>(new Objfun(RGB, XYZ),
                                                                   TAKE_OWNERSHIP);
        problem.AddResidualBlock(cost_function,
                                 new CauchyLoss(0.5),
                                 B);
        
        Solver::Options options;
        options.linear_solver_type = ceres::DENSE_QR;
        options.minimizer_progress_to_stdout = true;
        Solver::Summary summary;
        Solve(options, &problem, &summary);
        std::cout << summary.BriefReport() << "\n";
//        std::cout << summary.FullReport() << "\n";
        
        _CAT[0][0] = B[0];
        _CAT[0][1] = B[1];
        _CAT[0][2] = 1.0 - B[0] - B[1];
        _CAT[1][0] = B[2];
        _CAT[1][1] = B[3];
        _CAT[1][2] = 1.0 - B[2] - B[3];
        _CAT[2][0] = B[4];
        _CAT[2][1] = B[5];
        _CAT[2][2] = 1.0 - B[4] - B[5];
        
        FORI(3)
            FORJ(3)
                printf("%f ", _CAT[i][j]);
        
        return;
    }
    
    void Idt::get_final_idt() {
        load_illuminate(_bestIllum);
        double B_start[6] = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};

        vector< vector<double> > TI = cal_TI();
        
        //        vector< vector<double> > RGB = cal_RGB(TI);
        //        vector< vector<double> > XYZ = cal_XYZ(TI);
        //        vector< vector<double> > out_Lab = mulVector(XYZ, repmat2d(XYZ_w, 3, 3), 190, 3);
        
        vector < vector<double> > BV(3, vector<double>(3));
        
        BV[0][0] = B_start[0];
        BV[0][1] = B_start[1];
        BV[0][2] = 1.0 - B_start[0] - B_start[1];
        BV[1][0] = B_start[2];
        BV[1][1] = B_start[3];
        BV[1][2] = 1.0 - B_start[2] - B_start[3];
        BV[2][0] = B_start[4];
        BV[2][1] = B_start[5];
        BV[2][2] = 1.0 - B_start[4] - B_start[5];
        
        vector< vector<double> > out_calc_XYZt = transposeVec(mulVector(BV,
                                                                        rotateVec(transposeVec(cal_RGB(TI), 190, 3)),
                                                                        3, 190),
                                                              3, 190);
        
//        vector< vector<double> > out_calc_XYZt = mulVector(cal_RGB(TI), BV, 190, 3);
        
        FORI(190) {
            FORJ(3){
                printf(" %f ", out_calc_XYZt[i][j]);
            }
            printf(" \n");
        }

        curve_fit(cal_RGB(TI), cal_XYZ(TI), B_start);
        
        return;
    };

}





