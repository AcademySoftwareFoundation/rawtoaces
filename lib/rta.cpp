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

#include "rta.h"

namespace rta {
    Spst::Spst() {
        _brand = null_ptr;
        _model = null_ptr;
        _increment = 5;
        _spstMaxCol = -1;
        
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
        
        FORI(81) {
            _trainingSpec.push_back(trainSpec());
            _cmf.push_back(CMF());
        }
        
        _idt.resize(3);
        FORI(3) {
            _idt[i].resize(3);
            FORJ(3)
                _idt[i][j] = neutral3[i][j];
        }
    }
    
    Idt::~Idt() {
        vector< CMF >().swap(_cmf);
        vector< trainSpec >().swap(_trainingSpec);
        vector< vector<double> >().swap(_idt);
        
//        FORI(3) {
//            double * currentPtr = _idt[i];
//            free(currentPtr);
//        }
      
//        free(_idt);
    }
    
    void Idt::scaleLSC(){
        assert(_cameraSpst._spstMaxCol >= 0
               && (_illuminate.data).size() != 0);
        
        vector<double> colMax(81, 1.0);
        switch(_cameraSpst._spstMaxCol){
            case 0:
                FORI(81) colMax[i] = _cameraSpst._rgbsen[i].RSen;
                break;
            case 1:
                FORI(81) colMax[i] = _cameraSpst._rgbsen[i].GSen;
                break;
            case 2:
                FORI(81) colMax[i] = _cameraSpst._rgbsen[i].BSen;
                break;
            default:
                return;
        }
        
        scaleVector(_illuminate.data,
                    1.0/sumVector(mulVectorElement(_illuminate.data, colMax)));
        
//        clearVM(colMax);
    }
    
    void Idt::loadCameraSpst(const string & path, const char * maker, const char * model) {
        ifstream fin;
        fin.open(path);
        uint8_t line = 0;
        
        if(!fin.good()) {
            fprintf(stderr, "The file may not exist.\n");
            exit(EXIT_FAILURE);
        }
        
        vector <RGBSen> rgbsen;
        vector<double> max(3, numeric_limits<double>::min());
        
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
                
                if(tmp_sen.RSen > max[0])
                    max[0] = tmp_sen.RSen;
                if(tmp_sen.GSen > max[1])
                    max[1] = tmp_sen.GSen;
                if(tmp_sen.BSen > max[2])
                    max[2] = tmp_sen.BSen;
                
                rgbsen.push_back(tmp_sen);
            }
            line++;
        }
        
        if(line != 84) {
            fprintf(stderr, "The increment should be 5nm from 380nm to 780nm.\n");
            exit(EXIT_FAILURE);
        }
        
        _cameraSpst._spstMaxCol = max_element(max.begin(), max.end()) - max.begin();
        _cameraSpst.setSensitivity(rgbsen);
        
        fin.close();
    }
    
    void Idt::loadIlluminate(const string &path, const char * type) {
        ifstream fin;
        fin.open(path);
        
        uint8_t line = 0;
        int wl = 380;
        
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
                
                if(wl == 550) {
                    _illuminate.index = atof(token);
                }
                
                wl += _illuminate.inc;
            }
            
            line += 1;
        }

        fin.close();
        
        return;
    }
    
    void Idt::loadTrainingData(const string &path) {
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
    
    void Idt::loadCMF(const string &path) {
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
            
            if (!(_cmf[i].wl % 5)
                && _cmf[i].wl >= 380
                && _cmf[i].wl <= 780) {
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
    
    const Spst Idt::getCameraSpst() const {
        return static_cast<const Spst>(_cameraSpst);
    }
    
    const illum Idt::getIlluminate() const {
        return static_cast<const illum>(_illuminate);
    }
    
    vector<double> Idt::calCM() {
        vector<RGBSen> rgbsen = _cameraSpst.getSensitivity();
        vector< vector<double> > rgbsenV(3, vector<double>(rgbsen.size(), 1.0));
        
        FORI(rgbsen.size()){
            rgbsenV[0][i] = rgbsen[i].RSen;
            rgbsenV[1][i] = rgbsen[i].GSen;
            rgbsenV[2][i] = rgbsen[i].BSen;
        }
        
        vector<double> CM = mulVector(rgbsenV,
                                      _illuminate.data);
        scaleVectorD(CM);
        
        return CM;
    }
    
    void Idt::chooseIlluminate(map< string, vector<double> >& illuCM,
                               vector<double>& src) {
        double sse = numeric_limits<double>::max();
//        vector<double> vsrc(src, src+3);
        
        for (map< string, vector<double> >::iterator it = illuCM.begin(); it!=illuCM.end(); ++it){
            double tmp = calSSE(it->second, src);
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
    
    vector< vector<double> > Idt::calTI() const {
        assert(_illuminate.data.size() == 81 &&
               _trainingSpec[0].data.size() == 190);

        vector< vector<double> > TI(81, vector<double>(190));
        
//        vector<double> illumData(_illuminate.data);
//        scaleVector(illumData, 1.0/_illuminate.index);
        
        FORI(81)
            FORJ(190)
                TI[i][j] = _illuminate.data[i] * (_trainingSpec[i].data)[j];
        
        
        return TI;
    }
    
    vector< vector<double> > Idt::calCAT(vector<double> src, vector<double> des) const {
        assert(src.size() == des.size());
        
        vector < vector <double> > vect(3, vector<double>(3));
        FORI(3) {
            FORJ(3) {
                vect[i][j] = cat02[i][j];
            }
        }
        vector< double > wSRC = mulVector(src, vect);
        vector< double > wDES = mulVector(des, vect);
        vector< vector<double> > vkm = solveVM(vect, diagVM(divVectorElement(wDES,wSRC)));
        vkm = mulVector(vkm, transposeVec(vect));
        
        clearVM(wSRC);
        clearVM(wDES);

        return vkm;
    }
    
    vector< vector<double> > Idt::calRGB(vector< vector<double> > TI) const {
        assert(TI.size() == 81);
        
        vector< vector<double> > transTI = transposeVec(TI);
        vector< vector<double> > colRGB(3, vector<double>(TI.size(), 1.0));
        
        FORI(81) {
            colRGB[0][i] = _cameraSpst._rgbsen[i].RSen;
            colRGB[1][i] = _cameraSpst._rgbsen[i].GSen;
            colRGB[2][i] = _cameraSpst._rgbsen[i].BSen;
        }

        vector<double> b = mulVector(colRGB, _illuminate.data);
        FORI(b.size()) {
            b[i] = invertD(b[i]);
        }
        
        vector< vector<double> > RGB = transposeVec(mulVector(colRGB, transTI));
        FORI(RGB.size())
            RGB[i] = mulVectorElement(b, RGB[i]);
        
//        FORI(190) {
//            FORJ(3) {
//                printf("%f, ", RGB[i][j]);
//            }
//            printf("\n");
//        }
        
        clearVM(b);

        return RGB;
    }
    
    vector< vector<double> > Idt::calXYZ(vector< vector<double> > TI) const {
        assert(TI.size() == 81);
        
        vector< vector<double> > transTI = transposeVec(TI);
        vector< vector<double> > colXYZ(3, vector<double>(TI.size(), 1.0));

        FORI(81){
            colXYZ[0][i] = _cmf[i].xbar;
            colXYZ[1][i] = _cmf[i].ybar;
            colXYZ[2][i] = _cmf[i].zbar;
        }
        
        vector< vector<double> > XYZ = transposeVec(mulVector(colXYZ,
                                                              transTI));
        
        FORI(XYZ.size())
            scaleVector(XYZ[i],
                        1.0/sumVector(mulVectorElement(colXYZ[1],_illuminate.data)));
        
        vector <double> ww = mulVector(colXYZ, _illuminate.data);
        scaleVector(ww, (1.0/ww[1]));
        vector <double> w(XYZ_w, XYZ_w+3);

        XYZ = mulVector(XYZ, calCAT(ww, w));
        
//        FORI(190) {
//            FORJ(3) {
//                printf("%f, ", XYZ[i][j]);
//            }
//            printf("\n");
//        }
        
        clearVM(ww);
        clearVM(w);

        return XYZ;
    }
    
    bool Idt::curveFit(vector< vector<double> > RGB,
                                vector< vector<double> > XYZ,
                                double * B)
    {
        Problem problem;
        vector < vector <double> > M(3, vector<double>(3));
        FORI(3)
            FORJ(3)
                M[i][j] = acesrgb_XYZ_3[i][j];
        
        CostFunction* cost_function =
                new NumericDiffCostFunction<Objfun, CENTRAL, 1, 6>(new Objfun(RGB, XYZ, M),
                                                                   TAKE_OWNERSHIP);
        problem.AddResidualBlock(cost_function,
                                 new CauchyLoss(0.5),
                                 B);
        
        Solver::Options options;
        options.linear_solver_type = ceres::DENSE_QR;
        options.minimizer_progress_to_stdout = false;
        options.parameter_tolerance = 1e-17;
//        options.gradient_tolerance = 1e-17;
        options.function_tolerance = 1e-17;
        options.max_num_iterations = 100;
        options.minimizer_type = LINE_SEARCH;
        
        Solver::Summary summary;
        Solve(options, &problem, &summary);
//        std::cout << summary.BriefReport() << "\n";
        std::cout << summary.FullReport() << "\n";
        
        if (summary.num_successful_steps) {
            
            _idt[0][0] = B[0];
            _idt[0][1] = B[1];
            _idt[0][2] = 1.0 - B[0] - B[1];
            _idt[1][0] = B[2];
            _idt[1][1] = B[3];
            _idt[1][2] = 1.0 - B[2] - B[3];
            _idt[2][0] = B[4];
            _idt[2][1] = B[5];
            _idt[2][2] = 1.0 - B[4] - B[5];
            
//            _idt = mulVector(M, _idt);

            FORI(3) {
                FORJ(3) {
                    printf("%f ", _idt[i][j]);
                }
                printf("\n");
            }
            
            return 1;
        }
        
        return 0;
    }
    
    void Idt::calIDT() {
        loadIlluminate(_bestIllum);
        scaleLSC();
        
        cout << "The best illuminate is: " << _bestIllum << endl;
        
        double BStart[6] = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
        vector< vector<double> > TI = calTI();
        
        curveFit(calRGB(TI), calXYZ(TI), BStart);
        clearVM(TI);
    }
    
    vector< vector<double> > Idt::getIDT() const {
        return _idt;
    }
}





