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
//            float * currentPtr = _CAT[i];
//            free(currentPtr);
//        }
//        
//        free(_CAT);
    }
    
    CIELab Idt::XYZt_2_Lab(vector<CIEXYZ> XYZt, CIEXYZ XYZw) {
        CIELab CLab = {1.0, 1.0, 1.0};
//        static CIELab lab = {1.0, 1.0, 1.0};
//        CIELab *lab = static_cast<CIELab>(malloc(1 * sizeof(CIELab)));
        
        return CLab;
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
//                cout << "R:" << float(tmp_sen.RSen) << "; " << "G: " << float(tmp_sen.GSen) << "; " << "B: " << float(tmp_sen.BSen) << "\n";
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
//            cout << float(_illuminate.data[i]) << "," << endl;
        
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
                
//                cout << "WL: " << _cmf[i].wl << " X:" << float(_cmf[i].xbar) << "; " << "Y: " << float(_cmf[i].ybar) << "; " << "Z: " << float(_cmf[i].zbar) << "\n";

                i += 1;
            }
        }
        
        fin.close();
    }
    
    const Spst Idt::getCameraSpst() const {
        return static_cast<const Spst>(_cameraSpst);
    }
    
    const illum Idt::getIllum() const {
        return static_cast<const illum>(_illuminate);
    }
    
    void Idt::normalDayLight(vector<float>& mul){
        float max = *max_element(mul.begin(), mul.end());
        
        transform(mul.begin(), mul.end(), mul.begin(), invertD);
        transform(mul.begin(), mul.end(), mul.begin(),
                  bind1st(multiplies<float>(), max));
        
        return;
    }
    
    vector<float> Idt::calCM() {
        vector<RGBSen> rgbsen = _cameraSpst.getSensitivity();
//        uint8_t size = rgbsen.size();
//        vector<float> R(size), G(size), B(size);
        vector< vector<float> > rgbsenV(3, vector<float>(rgbsen.size(), 1.0));

        
        FORI(rgbsen.size()){
//            R[i] = rgbsen[i].RSen;
//            G[i] = rgbsen[i].GSen;
//            B[i] = rgbsen[i].BSen;
            
            rgbsenV[0][i] = rgbsen[i].RSen;
            rgbsenV[1][i] = rgbsen[i].GSen;
            rgbsenV[2][i] = rgbsen[i].BSen;
        }
        
//        vector<float> CM(3, 1.0);
        
//        CM[0] = sumVector((R, _illuminate.data));
//        CM[1] = sumVector(mulVectorElement(G, _illuminate.data));
//        CM[2] = sumVector(mulVectorElement(B, _illuminate.data));
        
        vector<float> CM = mulVector(rgbsenV,
                                     _illuminate.data,
                                     rgbsenV.size(),
                                     _illuminate.data.size());
        
        normalDayLight(CM);
        
//        cout << CM[0] << "; "
//        << CM[1] << "; "
//        << CM[2] << endl;
        
        return CM;
    }
    
    void Idt::determineIllum(map< string, vector<float> >& illuCM, const float src[]) {
        float sse = numeric_limits<float>::max();
        vector<float> vsrc(src, src+3);
        
        for (map< string, vector<float> >::iterator it = illuCM.begin(); it!=illuCM.end(); ++it){
            float tmp = calSSE(it->second, vsrc);
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

        load_illuminate(_bestIllum);

        vector< vector<float> > TI = calTrainingIllum();
        calRGB(TI);
        calXYZ(TI);
//        curveFitting(calRGB(TI),  calXYZ(TI));
        
//        cout << "Hello, the best light source is: " << _bestIllum << endl;
        return;
    }
    
    vector< vector<float> > Idt::calTrainingIllum() const {
        assert(_illuminate.data.size() == 81 &&
               _trainingSpec[0].data.size() == 190);

        vector< vector<float> > TI(81, vector <float>(190));
        
       FORI(81) {
            vector<float> row(81, _illuminate.data[i]);
            FORJ(190){
                vector<float> col;
                for(int w=0; w < 81; w++){
                    col.push_back((_trainingSpec[w].data)[j]);
                }
                TI[i][j] = (sumVector(mulVectorElement(row, col)));
            }
        }
        
        return TI;
    }
    
    vector< vector<float> > Idt::calRGB(vector< vector<float> > TI) const {
        assert(TI.size() == 81);
        
        vector< vector<float> > transTI = transposeVec(TI, 81, 190);
        vector< vector<float> > colRGB(3, vector<float>(TI.size(), 1.0));
        
        FORI(81) {
            colRGB[0][i] = _cameraSpst._rgbsen[i].RSen;
            colRGB[1][i] = _cameraSpst._rgbsen[i].GSen;
            colRGB[2][i] = _cameraSpst._rgbsen[i].BSen;
        }

        vector< vector<float> > RGB = mulVector(transTI,
                                                colRGB,
                                                transTI.size(),
                                                colRGB.size());

        
//      cout << "RGB size: " << RGB.size() << "; " << "RGB.size[0] size: " << RGB.size[0].size() << endl;
        cout << "RGB[0][0]: " << float(RGB[0][0]) << " RGB[189][2]: " << float(RGB[189][2]) << endl;

        return RGB;
    }
    
    vector< vector<float> > Idt::calXYZ(vector< vector<float> > TI) const {
        assert(TI.size() == 81);
        
        vector< vector<float> > transTI = transposeVec(TI, 81, 190);
        vector< vector<float> > colXYZ(3, vector<float>(TI.size(), 1.0));

        FORI(81){
            colXYZ[0][i] = _cmf[i].xbar;
            colXYZ[1][i] = _cmf[i].ybar;
            colXYZ[2][i] = _cmf[i].zbar;
        }
        
        vector< vector<float> > XYZ = mulVector(transTI,
                                                colXYZ,
                                                transTI.size(),
                                                colXYZ.size());
        
//       cout << "XYZ size: " << XYZ.size() << "; " << "XYZ[0] size: " << XYZ[0].size() << endl;
        cout << "XYZ[0][0]: " << float(XYZ[0][0]) << " XYZ[189][2]: " << float(XYZ[189][2]) << endl;
        
        return XYZ;
    }
    
//    vector< vector<float> > Idt::calOutLAB(vector< vector<float> > XYZ) const
//    {
//        assert(XYZ.size() == 190);
//        const vector < vector< float > > XYZ_wv = repmat2d(XYZ_w, 3, 3);
//        vector< vector<float> > out_Lab = mulVector(XYZ, XYZ_wv, 190, 3);
//        
//        return out_Lab;
//        
//    }
//    
//    vector< vector<float> > Idt::calLAB(const vector < vector<float> > RGB,
//                                         const vector < vector<float> > XYZ,
//                                         const vector < vector<float> > B) const
//    {
//        assert(RGB.size() == XYZ.size());
//        float add = 16.0f/116.0f;
//        
//        vector< vector<float> > out_calc_XYZt = transposeVec(mulVector(B, transposeVec(RGB, 190, 3), 3, 190), 3, 190);
//        vector< vector<float> > tmp(190, vector<float>(3, 0.0f));
//        
//        FORI(190) {
//            FORJ(3)
//            {
//                tmp[i][j] = XYZ[i][j] / XYZ_w[j];
//                if (tmp[i][j] > e)
//                    tmp[i][j] = pow (tmp[i][j], 1.0f/3.0f);
//                else
//                    tmp[i][j] = k * tmp[i][j] + add;
//            }
//        }
//        
//        vector< vector<float> > out_calc_Lab(190, vector<float>(3, 0.0f));
//        FORI(190)
//        {
//            out_calc_Lab[i][0] = 116.0f * tmp[i][1] - 16.0f;
//            out_calc_Lab[i][1] = 500.0f * (tmp[i][0] - tmp[i][1]);
//            out_calc_Lab[i][2] = 200.0f * (tmp[i][1] - tmp[i][2]);
//        }
//        
//        return out_calc_Lab;
//    }
    
    void Idt::curveFitting(vector< vector<float> > RGB,
                           vector< vector<float> > XYZ) const
    {
        
        double B_start0, B_start3 = 1.0;
        double B_start1, B_start2, B_start4, B_start5 = 0.0;
        
        Problem problem;
        
        CostFunction* cost_function =
        new AutoDiffCostFunction<Objfun, 1, 1, 1, 1, 1, 1, 1>(new Objfun(RGB, XYZ));
        problem.AddResidualBlock(cost_function,
                                 new CauchyLoss(0.5),
                                 &B_start0,
                                 &B_start1,
                                 &B_start2,
                                 &B_start3,
                                 &B_start4,
                                 &B_start5);
        
//        double B_start[6] = {B_start0, B_start1, B_start2, B_start3, B_start4, B_start5};
//        
//        CostFunction* cost_function =
//        new AutoDiffCostFunction<Objfun, 1, 6>(new Objfun(RGB, XYZ));
//        problem.AddResidualBlock(cost_function,
//                                 new CauchyLoss(0.5),
//                                 B_start);
//        
        
//        Solver::Options options;
//        options.linear_solver_type = ceres::DENSE_QR;
//        options.minimizer_progress_to_stdout = true;
//        Solver::Summary summary;
//        Solve(options, &problem, &summary);
//        std::cout << summary.BriefReport() << "\n";
//        std::cout << "Initial m: " << 0.0 << " c: " << 0.0 << "\n";
//        std::cout << "Final   m: " << m << " c: " << c << "\n";
        
        return;
    }
}





