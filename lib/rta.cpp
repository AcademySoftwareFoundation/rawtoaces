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
            _rgbsen.push_back( RGBSen() );
        }
    }
    
    Spst::Spst ( const Spst& spstobject ) {
        assert ( spstobject._brand != null_ptr
                && spstobject._model != null_ptr );
        
        uint8_t lenb = strlen(spstobject._brand);
        assert(lenb < 64);
        
        if(lenb > 64)
            lenb = 64;
        
        _brand = (char *) malloc(lenb+1);
        memset(_brand, 0x0, lenb);
        memcpy(_brand, spstobject._brand, lenb);
        _brand[lenb] = '\0';
        
        uint8_t lenm = strlen(spstobject._model);
        assert(lenm < 64);
        
        if(lenm > 64)
            lenm = 64;
        
        _model = (char *) malloc(lenm+1);
        memset(_model, 0x0, lenm);
        memcpy(_model, spstobject._model, lenm);
        _model[lenm] = '\0';
        
        _increment = spstobject._increment;
        _spstMaxCol = spstobject._spstMaxCol;
        _rgbsen = spstobject._rgbsen;
    }
    
    Spst::~Spst() {
        delete _brand;
        delete _model;
        
        vector< RGBSen >().swap( _rgbsen );
    }
    
    //	=====================================================================
    //	Fetch the brand of camera
    //
    //	inputs:
    //      N/A
    //
    //	outputs:
    //		const char *: the name of camera brand
    
    const char * Spst::getBrand() const {
        return _brand;
    }
    
    //	=====================================================================
    //	Fetch the model of the camera
    //
    //	inputs:
    //      N/A
    //
    //	outputs:
    //		const char *: the model of the camera
    
    const char * Spst::getModel() const {
        return _model;
    }
    
    //	=====================================================================
    //	Fetch wavelength increment value of the camera sensitivity
    //
    //	inputs:
    //      N/A
    //
    //	outputs:
    //		const uint8_t: Wavelength increment value (e.g., 5nm, 10nm) of
    //                     the camera sensitivity
    
    const uint8_t Spst::getWLIncrement() const {
        return _increment;
    }
    
    //	=====================================================================
    //	Fetch the sensitivity data of the camera (reading from the file)
    //
    //	inputs:
    //      N/A
    //
    //	outputs:
    //		const vector <RGBSen>: the sensitivity (in vector) of the camera
    
    const vector <RGBSen> & Spst::getSensitivity() const {
        return _rgbsen;
    }
    
    //	=====================================================================
    //	Fetch the brand of camera
    //
    //	inputs:
    //      N/A
    //
    //	outputs:
    //		char *: the name of camera brand

    char * Spst::getBrand() {
        return _brand;
    }
    
    //	=====================================================================
    //	Fetch the model of the camera
    //
    //	inputs:
    //      N/A
    //
    //	outputs:
    //	    char *: the model of the camera

    char * Spst::getModel() {
        return _model;
    }
    
    //	=====================================================================
    //	Fetch the wavelength increment value of the camera sensitivity
    //
    //	inputs:
    //      N/A
    //
    //	outputs:
    //	    uint8_t: Wavelength increment value (e.g., 5nm, 10nm) of the
    //               camera's sensitivity

    uint8_t Spst::getWLIncrement() {
        return _increment;
    }
    
    //	=====================================================================
    //	Fetch the sensitivity data of the camera (reading from the file)
    //
    //	inputs:
    //      N/A
    //
    //	outputs:
    //		vector <RGBSen>: the sensitivity (in vector) of the camera
    
    vector <RGBSen> & Spst::getSensitivity() {
        return _rgbsen;
    }
    
    //	=====================================================================
    //	Set the brand of camera
    //
    //	inputs:
    //      const char *: brand (read from the file or
    //                    the meta-data from libraw)
    //
    //	outputs:
    //		void: _brand (private member)
    
    void Spst::setBrand ( const char * brand ) {
        assert(brand != null_ptr);
        uint8_t len = strlen(brand);
        
        assert(len < 64);
        
        if(len > 64) len = 64;
        
        _brand = (char *) malloc(len+1);
        memset(_brand, 0x0, len);
        memcpy(_brand, brand, len);
        _brand[len] = '\0';
        
        return;
    }
    
    //	=====================================================================
    //	Set the model of camera
    //
    //	inputs:
    //      const char *: model (read from the file or
    //                    the meta-data from libraw)
    //
    //	outputs:
    //		void: _brand (private member)
    
    void Spst::setModel ( const char * model ) {
        assert(model != null_ptr);
        uint8_t len = strlen(model);
        
        assert(len < 64);
        
        if(len > 64) len = 64;
        
        _model = (char *)malloc(len+1);
        memset(_model, 0x0, len);
        memcpy(_model, model, len);
        _model[len] = '\0';
  
        return;
    }
    
    //	=====================================================================
    //	Set the wavelength increment value of the camera sensitivity
    //
    //	inputs:
    //      uint8_t: inc (read from the file)
    //
    //	outputs:
    //		void: _increment (private member)
    
    void Spst::setWLIncrement ( const uint8_t inc ) {
        _increment = inc;
        
        return;
    }
    
    //	=====================================================================
    //	Set the sensitivity data of the camera (reading from the file)
    //
    //	inputs:
    //      const vector<RGBSen>: rgbsen (read from the file)
    //
    //	outputs:
    //		void: _rgbsen (private member)

    void Spst::setSensitivity ( const vector<RGBSen> rgbsen ) {
        FORI(rgbsen.size()) _rgbsen[i] = rgbsen[i];
        
        return;
    }
    
    Idt::Idt() {
        _bestIllum = "";
        _verbosity = 0;
        
        FORI(81) {
            _trainingSpec.push_back(trainSpec());
            _cmf.push_back(CMF());
        }
        
        _idt.resize(3);
        _wb.resize(3);
        FORI(3) {
            _idt[i].resize(3);
            _wb[i] = 1.0;
            FORJ(3) _idt[i][j] = neutral3[i][j];
        }
    }
    
    Idt::~Idt() {
        vector < CMF >().swap(_cmf);
        vector < trainSpec >().swap(_trainingSpec);
        vector < double >().swap(_wb);
        vector < vector<double> >().swap(_idt);
    }
    
    //	=====================================================================
    //	Scale the Illuminant data using the max element of RGB code values
    //
    //	inputs:
    //		N/A
    //	
    //	outputs:
    //		scaled _Illuminant data set
    
    void Idt::scaleLSC() {
        assert(_cameraSpst._spstMaxCol >= 0
               && (_Illuminant.data).size() != 0);
        
        uint16_t size =_cameraSpst._rgbsen.size();
        vector < double > colMax(size, 1.0);
        switch (_cameraSpst._spstMaxCol){
            case 0:
                FORI(size) colMax[i] = _cameraSpst._rgbsen[i].RSen;
                break;
            case 1:
                FORI(size) colMax[i] = _cameraSpst._rgbsen[i].GSen;
                break;
            case 2:
                FORI(size) colMax[i] = _cameraSpst._rgbsen[i].BSen;
                break;
            default:
                return;
        }
        
        scaleVector(_Illuminant.data,
                    1.0/sumVector(mulVectorElement(_Illuminant.data, colMax)));
    }
    
    //	=====================================================================
    //	Load the Camera Sensitivty data
    //
    //	inputs:
    //		const string: path to the camera sensitivity file
    //      const char *: camera maker  (from libraw)
    //      const char *: camera model  (from libraw)
    //      const int: ss_path  (from user input "--ss-path")
    //
    //	outputs:
    //		boolean: If successufully parsed, _cameraSpst will be filled and return 1;
    //               Otherwise, return 0
    
    int Idt::loadCameraSpst ( const string & path,
                              const char * maker,
                              const char * model,
                              const int ss_path ) {
        vector <RGBSen> rgbsen;
        vector <double> max(3, dmin);
        
        try
        {
            // using libraries from boost::property_tree
            ptree pt;
            read_json ( path, pt );
            
            const char * cmaker = (pt.get<string>( "header.manufacturer" )).c_str();
            if (!ss_path && cmp_str(maker, cmaker)) return 0;
                _cameraSpst.setBrand(cmaker);
            
            const char * cmodel = (pt.get<string>( "header.model" )).c_str();
            if (!ss_path && cmp_str(model, cmodel)) return 0;
                _cameraSpst.setModel(cmaker);
            
            vector <int> wavs;
            int inc;

            BOOST_FOREACH ( ptree::value_type &row, pt.get_child ( "spectral_data.data.main" ) )
            {
                wavs.push_back(atoi((row.first).c_str()));

                if ( wavs.size() == 2 )
                    inc = wavs[1] - wavs[0];
                else if ( wavs.size() > 2 &&
                          wavs[wavs.size()-1] - wavs[wavs.size()-2] != inc ) {
                    fprintf(stderr, "Please double check the Camera "
                            "Sensitivity data (e.g. the increment "
                            "should be uniform from 380nm to 780nm).\n");
                    exit(1);
                }
                
                if ( wavs[wavs.size()-1] < 380 ||
                     wavs[wavs.size()-1] % 5 )
                    continue;
                else if ( wavs[wavs.size()-1] > 780 )
                    break;

                vector < double > data;
                BOOST_FOREACH ( ptree::value_type &cell, row.second )
                    data.push_back ( cell.second.get_value<double>() );
                
                // ensure there are three components
                assert(data.size() == 3);
                
                RGBSen tmp_sen ( data[0], data[1], data[2] );
                
                if (tmp_sen.RSen > max[0]) max[0] = tmp_sen.RSen;
                if (tmp_sen.GSen > max[1]) max[1] = tmp_sen.GSen;
                if (tmp_sen.BSen > max[2]) max[2] = tmp_sen.BSen;
                
//                printf("\"%i\": [ %18.13f,  %18.13f,  %18.13f ], \n", wavs[wavs.size()-1], data[0], data[1], data[2] );
                
                rgbsen.push_back (tmp_sen);
            }
            _cameraSpst.setWLIncrement (inc);
        }
        catch ( std::exception const& e )
        {
            std::cerr << e.what() << std::endl;
        }
        
        // it can be updated if there is a broader spectrum
        // (e.g., 300nm-800nm) or a smaller increment values (e.g, 1nm)
        if ( rgbsen.size() != 81 ) {
            fprintf(stderr, "Please double check the Camera "
                            "Sensitivity data (e.g. the increment "
                            "should be uniform from 380nm to 780nm).\n");
            exit(-1);
        }
        
        _cameraSpst._spstMaxCol = max_element (max.begin(), max.end()) - max.begin();
        _cameraSpst.setSensitivity (rgbsen);
        
        return 1;
    }
    
    //	=====================================================================
    //	Load the Illuminant data
    //
    //	inputs:
    //		const string: path to the Illuminant data file
    //      const char *: type of light source if user specifies
    //
    //	outputs:
    //		boolean: If successufully parsed, _Illuminant will be filled and return 1;
    //               Otherwise, return 0

    
    int Idt::loadIlluminant ( const string &path,
                              const string type ) {
        
        if ((_Illuminant.data).size() != 0) {
            _Illuminant.type = "";
            _Illuminant.inc = 5;
            _Illuminant.data.clear();
        }
        
        try
        {
            // using libraries from boost::property_tree
            ptree pt;
            read_json ( path, pt );
            
            const string stype = pt.get<string>( "header.illuminant" );
            if ( type.compare(stype) != 0
                 && type.compare("unknown") != 0 )
            {
                return 0;
            }
            _Illuminant.type = stype;
            
            vector <int> wavs;
            int inc;
            
            BOOST_FOREACH ( ptree::value_type &row, pt.get_child ( "spectral_data.data.main" ) )
            {
                wavs.push_back(atoi((row.first).c_str()));
                
                if ( wavs.size() == 2 )
                    inc = wavs[1] - wavs[0];
                else if ( wavs.size() > 2 &&
                         wavs[wavs.size()-1] - wavs[wavs.size()-2] != inc ) {
                    fprintf ( stderr, "Please double check the Light "
                                      "Source data (e.g. the increment "
                                      "should be uniform from 380nm to 780nm).\n" );
                    exit(-1);
                }
                
                if ( wavs[wavs.size()-1] < 380 ||
                     wavs[wavs.size()-1] % 5 )
                    continue;
                else if ( wavs[wavs.size()-1] > 780 )
                    break;

                BOOST_FOREACH ( ptree::value_type &cell, row.second ) {
                    _Illuminant.data.push_back(cell.second.get_value<double>());
                    if ( wavs[wavs.size()-1] == 550 )
                        _Illuminant.index = cell.second.get_value<double>();
                }

//                printf("\"%i\": [ %18.13f ], \n", wavs[wavs.size()-1], _Illuminant.data[_Illuminant.data.size()-1] );
            }
        }
        catch ( std::exception const& e )
        {
            std::cerr << e.what() << std::endl;
        }
        
        if ( _Illuminant.data.size() != 81 ) {
            fprintf ( stderr, "Please double check the Light "
                              "Source data (e.g. the increment "
                              "should be 5nm from 380nm to 780nm).\n" );
            exit(1);
        }
        
        return 1;
    }
    
    //	=====================================================================
    //	Load the 190-patch training data
    //
    //	inputs:
    //		path to the 190-patch training data
    //
    //	outputs:
    //		_trainingSpec: If successufully parsed, _trainingSpec will be filled
    
    void Idt::loadTrainingData ( const string &path ) {
        struct stat st;
        assert (!stat( path.c_str(), &st ));
        
        if ( _trainingSpec.size() > 0 ) {
            FORI (_trainingSpec.size())
                _trainingSpec[i].data.clear();
        }
        
        try
        {
            // using libraries from boost::property_tree
            ptree pt;
            read_json ( path, pt );
            
            int i = 0;
            
            BOOST_FOREACH ( ptree::value_type &row, pt.get_child ( "spectral_data.data.main" ) )
            {
                _trainingSpec[i].wl = atoi((row.first).c_str());

                BOOST_FOREACH ( ptree::value_type &cell, row.second )
                    _trainingSpec[i].data.push_back(cell.second.get_value<double>());
                
                assert(_trainingSpec[i].data.size() == 190);
                
//                printf("\"%i\": [", _trainingSpec[i].wl);
//                FORJ (190) printf("%18.13f,", _trainingSpec[i].data[j] );
//                printf("], \n");
                
                i += 1;
            }
        }
        catch ( std::exception const& e )
        {
            std::cerr << e.what() << std::endl;
        }
    }
    
    //	=====================================================================
    //	Load the CIE 1931 Color Matching Functions data
    //
    //	inputs:
    //		path to the CIE 1931 Color Matching Functions data
    //
    //	outputs:
    //		_cmf: If successufully parsed, _cmf will be filled
    
    void Idt::loadCMF ( const string &path ) {
        struct stat st;
        assert (!stat( path.c_str(), &st ));
        
        if ( _cmf.size() > 0 )
            _cmf.clear();
        
        try
        {
            // using libraries from boost::property_tree
            ptree pt;
            read_json ( path, pt );
            
            int i = 0;
            BOOST_FOREACH ( ptree::value_type &row, pt.get_child ( "spectral_data.data.main" ) )
            {
                _cmf[i].wl = atoi((row.first).c_str());
                
                if ( _cmf[i].wl < 380 ||
                    _cmf[i].wl % 5 )
                     continue;
                else if ( _cmf[i].wl > 780 )
                    break;
                
                vector < double > data;
                BOOST_FOREACH ( ptree::value_type &cell, row.second )
                    data.push_back ( cell.second.get_value<double>() );
                
                assert(data.size() == 3);
                _cmf[i].xbar = data[0];
                _cmf[i].ybar = data[1];
                _cmf[i].zbar = data[2];
                
//                printf("\"%i\": [ %18.13f,  %18.13f,  %18.13f ], \n", _cmf[i].wl, data[0], data[1], data[2] );
                
                i += 1;
            }
        }
        catch ( std::exception const& e )
        {
            std::cerr << e.what() << std::endl;
        }
    }
    
    //	=====================================================================
    //	Set Verbosity value for the length of IDT generation status message
    //
    //	inputs:
    //      int: verbosity
    //
    //	outputs:
    //		int: _verbosity
    
    void Idt::setVerbosity ( int verbosity ) {
        _verbosity = verbosity;
    }
    
    //	=====================================================================
    //	Choose the best Light Source based on White Balance Coefficients from
    //  the camera read by libraw according to a given set of coefficients
    //
    //	inputs:
    //		Map: Key: path to the Light Source data;
    //           Value: Light Source x Camera Sensitivity
    //      Vector: White Balance Coefficients
    //
    //	outputs:
    //		illum: the best _Illuminant
    
    void Idt::chooseIllumSrc ( map < string, vector<double> >& illuCM,
                               vector<double>& src) {
        double sse = dmax;
        
        for ( map< string, vector<double> >::iterator it = illuCM.begin(); it != illuCM.end(); ++it ) {
            double tmp = calSSE(it->second, src);
            
//            printf ("%s, %f\n", it->first.c_str(), tmp);
//            FORI(3) printf ("%f, ", it->second[i]);
//            printf ("\n");
            
            if (sse > tmp) {
                sse = tmp;
                _bestIllum = it->first;
                _wb = it->second;
            }
        }
        
        if (_verbosity > 1)
        	printf ( "The illuminant calculated to be the best match to the camera metadata is %s\n",
                	 _bestIllum.c_str() );

        if ( loadIlluminant( _bestIllum ) )
            scaleLSC();
        
        return;
    }
    
    
    //	=====================================================================
    //	Choose the best Light Source based on White Balance Coefficients from
    //  the camera read by libraw according to user-specified illuminant
    //
    //	inputs:
    //		Map: Key: path to the Light Source data;
    //           Value: Light Source x Camera Sensitivity
    //      String: Light Source Name
    //
    //	outputs:
    //		illum: the best _Illuminant
    
    void Idt::chooseIllumType ( map < string, vector<double> >& illuCM,
                                const string type ) {
        
        _bestIllum = illuCM.begin()->first;
        _wb = illuCM.begin()->second;

		if (_verbosity > 1)
        printf ( "The specified light source is: %s\n",
                 _bestIllum.c_str() );
        
        if ( loadIlluminant( _bestIllum, type ) )
            scaleLSC();
        
        return;
    }
    
    //	=====================================================================
    //	Calculate the middle product based on the camera sensitivity data
    //  and Illuminant/light source data
    //
    //	inputs:
    //		N/A
    //
    //	outputs:
    //		vector < double >: scaled vector by its maximum value

    vector<double> Idt::calCM() {
        vector < RGBSen > rgbsen = _cameraSpst.getSensitivity();
        vector< vector < double > > rgbsenV (3, vector < double > ( rgbsen.size(), 1.0));
        
        FORI( rgbsen.size() ){
            rgbsenV[0][i] = rgbsen[i].RSen;
            rgbsenV[1][i] = rgbsen[i].GSen;
            rgbsenV[2][i] = rgbsen[i].BSen;
        }
        
        vector<double> CM = mulVector ( rgbsenV, _Illuminant.data );
        scaleVectorD(CM);
        
        return CM;
    }
    
    //	=====================================================================
    //	Calculate the middle product based on the 190 patch / training data
    //  and Illuminant/light source data
    //
    //	inputs:
    //		N/A
    //
    //	outputs:
    //		vector < vector<double> >: 2D vector (81 x 190)
    
    vector< vector<double> > Idt::calTI() const {
        assert(_Illuminant.data.size() == 81 &&
               _trainingSpec[0].data.size() == 190);

        vector< vector<double> > TI(_Illuminant.data.size(), vector<double>(190));
        FORIJ(_Illuminant.data.size(), _trainingSpec[0].data.size())
            TI[i][j] = _Illuminant.data[i] * (_trainingSpec[i].data)[j];
        
        return TI;
    }
    
    //	=====================================================================
    //	Calculate the CAT matrix to compensate for difference between scene
    //  adopted white chromaticity and ACES neutral chromaticity (either
    //  CAT02 or bradford is used)
    //
    //	inputs:
    //		vector<double> source
    //      vector<double> destination
    //
    //	outputs:
    //		vector < vector<double> >: 2D vector (3 x 3)
    
    vector< vector<double> > Idt::calCAT ( vector<double> src,
                                           vector<double> des ) const {
        assert(src.size() == des.size());
        
        vector < vector <double> > vect(3, vector<double>(3));
        FORIJ(3, 3) vect[i][j] = cat02[i][j];
        
        vector< double > wSRC = mulVector ( src, vect );
        vector< double > wDES = mulVector ( des, vect );
        vector< vector<double> > vkm = solveVM(vect, diagVM ( divVectorElement (wDES, wSRC) ));
        vkm = mulVector(vkm, transposeVec(vect));
        
        clearVM (wSRC);
        clearVM (wDES);

        return vkm;
    }
    
    //	=====================================================================
    //	Calculate White Balance based on the best Illuminant data and
    //  highlight mode used in pre-processing with "libraw"
    //
    //	inputs:
    //      int: highlight
    //
    //	outputs:
    //		vector: wb(R, G, B)
    
    vector < double > Idt::calWB( int highlight ) {
        assert( _Illuminant.data.size() == 81
               && _cameraSpst._rgbsen.size() > 0 );
        
        scaleLSC();
        
        vector < vector < double > > colRGB (3, vector <double> (_Illuminant.data.size(), 1.0));
        
        FORI(_Illuminant.data.size()) {
            colRGB[0][i] = _cameraSpst._rgbsen[i].RSen;
            colRGB[1][i] = _cameraSpst._rgbsen[i].GSen;
            colRGB[2][i] = _cameraSpst._rgbsen[i].BSen;
        }
        
        vector< double > wb = mulVector ( colRGB, _Illuminant.data );
        clearVM(colRGB);
        
        FORI(wb.size()) wb[i] = invertD(wb[i]);
        
        if ( !highlight )
            scaleVectorMin (wb);
        else
            scaleVectorMax (wb);
        
        return wb;
    }
    
    //	=====================================================================
    //	Calculate CIE XYZ tristimulus values of scene adopted white
    //  based on training color spectral radiances from CalTI() and color
    //  adaptation matrix from CalCAT()
    //
    //	inputs:
    //		vector< vector<double> > outcome of CalTI()
    //
    //	outputs:
    //		vector < vector<double> >: 2D vector (190 x 3)
    
    vector< vector<double> > Idt::calXYZ (vector < vector < double > > TI ) const {
        assert(TI.size() == 81);
        
        vector< vector<double> > transTI = transposeVec(TI);
        vector< vector<double> > colXYZ(3, vector<double>(TI.size(), 1.0));

        FORI(TI.size()){
            colXYZ[0][i] = _cmf[i].xbar;
            colXYZ[1][i] = _cmf[i].ybar;
            colXYZ[2][i] = _cmf[i].zbar;
        }
        
        vector< vector<double> > XYZ = transposeVec(mulVector(colXYZ,
                                                              transTI));
        
        FORI(XYZ.size())
            scaleVector(XYZ[i],
                        1.0 / sumVector(mulVectorElement(colXYZ[1],_Illuminant.data)));
        
        vector <double> ww = mulVector(colXYZ, _Illuminant.data);
        scaleVector(ww, (1.0/ww[1]));
        vector <double> w(XYZ_w, XYZ_w+3);

        XYZ = mulVector(XYZ, calCAT(ww, w));
        
        clearVM(ww);
        clearVM(w);

        return XYZ;
    }
    
    //	=====================================================================
    //	Calculate white-balanced linearized camera system response (in RGB)
    //  based on training color spectral radiances from CalTI() and white
    //  balance factors from calWB(...)
    //
    //	inputs:
    //		vector< vector<double> > outcome of CalTI()
    //
    //	outputs:
    //		vector < vector<double> >: 2D vector (190 x 3)
    
    vector< vector<double> > Idt::calRGB ( vector < vector< double > > TI ) const {
        assert(TI.size() == 81);
        
        vector< vector<double> > transTI = transposeVec(TI);
        vector< vector<double> > colRGB(3, vector<double>(TI.size(), 1.0));
        
        FORI(TI.size()) {
            colRGB[0][i] = _cameraSpst._rgbsen[i].RSen;
            colRGB[1][i] = _cameraSpst._rgbsen[i].GSen;
            colRGB[2][i] = _cameraSpst._rgbsen[i].BSen;
        }
        
//        vector< vector<double> > RGB = transposeVec(mulVector(colRGB, transTI));
        vector< vector<double> > RGB = mulVector(transTI, colRGB);

        FORI(RGB.size())
            RGB[i] = mulVectorElement(_wb, RGB[i]);
        
        clearVM(transTI);
        clearVM(colRGB);
        
        return RGB;
    }
    
    //	=====================================================================
    //	Process cureve fit between XYZ and RGB data with initial set of B
    //  values.
    //
    //	inputs:
    //		vector< vector<double> >: RGB
    //      vector< vector<double> >: XYZ
    //      double * :                B (6 elements)
    //
    //	outputs:
    //      boolean: if succeed, _idt should be filled with values
    //               that minimize the distance between RGB and XYZ
    //               through updated B.
    
    int Idt::curveFit ( vector< vector<double> > RGB,
                        vector< vector<double> > XYZ,
                        double * B ) {
        Problem problem;
        vector < vector <double> > outLAB = XYZtoLAB(XYZ);
        
//        CostFunction* cost_function =
//                new NumericDiffCostFunction<Objfun, CENTRAL, 190, 6>(new Objfun(RGB, XYZ, M),
//                                                                     TAKE_OWNERSHIP);
        
        CostFunction* cost_function =
            new AutoDiffCostFunction<Objfun, DYNAMIC, 6>(new Objfun(RGB, outLAB), RGB.size()*(RGB[0].size()));
        
        problem.AddResidualBlock ( cost_function,
                                   NULL,
                                   B );
        
        ceres::Solver::Options options;
        options.linear_solver_type = ceres::DENSE_QR;
        options.parameter_tolerance = 1e-17;
        options.gradient_tolerance = 1e-17;
        options.function_tolerance = 1e-17;
        options.min_line_search_step_size = 1e-17;
        options.max_num_iterations = 300;
        
        if (_verbosity > 2)
            options.minimizer_progress_to_stdout = true;
        
        ceres::Solver::Summary summary;
        ceres::Solve(options, &problem, &summary);

        if (_verbosity > 1)
            std::cout << summary.BriefReport() << "\n";
        else if (_verbosity >= 2)
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
            
            if (_verbosity > 1) {
                printf("The IDT matrix is ...\n");

                FORI(3) {
                    FORJ(3) {
                        printf("   %f", _idt[i][j]);
                    }
                printf("\n");
                }
            }
            
            return 1;
        }
        
        return 0;
    }
    
    //	=====================================================================
    //	Calculate IDT matrix by calling curveFit(...)
    //
    //	inputs:
    //         N/A
    //
    //	outputs: through curveFit(...)
    //      boolean: if succeed, _idt should be filled with values
    //               that minimize the distance between RGB and XYZ
    //               through updated B.

    int Idt::calIDT() {
        
        double BStart[6] = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
        vector < vector<double> > TI = calTI();
        
        return curveFit(calRGB(TI), calXYZ(TI), BStart);
    }
    
    //	=====================================================================
    //  Get camera sensitivity data that was loaded from the file
    //
    //	inputs:
    //         N/A
    //
    //	outputs:
    //      const Spst: camera sensitivity data that was loaded from the file
    
    const Spst Idt::getCameraSpst() const {
        return static_cast<const Spst>(_cameraSpst);
    }
    
    //	=====================================================================
    //  Get Illuminant data / light source that was loaded from the file
    //
    //	inputs:
    //         N/A
    //
    //	outputs:
    //      const illum: Illuminant data that was loaded from the file

    const illum Idt::getIlluminant() const {
        return static_cast<const illum>(_Illuminant);
    }
    
    //	=====================================================================
    //	Get Verbosity value for the length of IDT generation status message
    //
    //	inputs:
    //      N/A
    //
    //	outputs:
    //		int: _verbosity (const)
    
    const int Idt::getVerbosity() const {
        return static_cast<const int>(_verbosity);
    }
    
    //	=====================================================================
    //  Get camera sensitivity data that was loaded from the file
    //
    //	inputs:
    //         N/A
    //
    //	outputs:
    //         Spst: camera sensitivity data that was loaded from the file
    
    Spst Idt::getCameraSpst() {
        return _cameraSpst;
    }
    
    //	=====================================================================
    //  Get Illuminant data / light source that was loaded from the file
    //
    //	inputs:
    //         N/A
    //
    //	outputs:
    //         illum: Illuminant data that was loaded from the file
    
    illum Idt::getIlluminant() {
        return _Illuminant;
    }
    
    //	=====================================================================
    //	Get Verbosity value for the length of IDT generation status message
    //
    //	inputs:
    //      N/A
    //
    //	outputs:
    //		int: _verbosity
    
    int Idt::getVerbosity() {
        return _verbosity;
    }

    //	=====================================================================
    //  Get Idt matrix if CalIDT() succeeds
    //
    //	inputs:
    //         N/A
    //
    //	outputs:
    //      const vector< vector < double > >: _idt matrix (3 x 3)

    
    const vector< vector<double> > Idt::getIDT() const {
        return static_cast< const vector< vector < double > > > (_idt);
    }
    
    //	=====================================================================
    //  Get white balanced if calWB(...) succeeds
    //
    //	inputs:
    //         N/A
    //
    //	outputs:
    //      const vector< double >: _wb vector (1 x 3)
    
    const vector< double > Idt::getWB() const {
        return static_cast< const vector < double > > (_wb);
    }
}







