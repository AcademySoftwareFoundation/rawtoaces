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

#include "acesrender.h"

AcesRender::AcesRender() {
    _idt = new Idt();
    _image = new libraw_processed_image_t();
    _rawProcessor = new LibRawAces();

    _idtm.resize(3);
    _wbv.resize(3);
    _catm.resize(3);
    
    FORI(3) {
        _idtm[i].resize(3);
        _catm[i].resize(3);
        
        _wbv[i] = 1.0;
        FORJ(3) _idtm[i][j] = neutral3[i][j];
        FORJ(3) _catm[i][j] = neutral3[i][j];
    }
}


AcesRender::~AcesRender() {
    if (_pathToRaw) {
       delete _pathToRaw;
        _pathToRaw = nullptr;
    }

    if (_idt) {
        delete _idt;
        _idt = nullptr;
    }
    
    if (_image) {
       delete _image;
        _image = nullptr;
    }
    
    if (_rawProcessor) {
        delete _rawProcessor;
        _rawProcessor = nullptr;
    }
    
    vector < vector < double > >().swap(_idtm);
    vector < vector < double > >().swap(_catm);
    vector < double >().swap(_wbv);
    vector < string >().swap(_illuminants);
    vector < string >().swap(_cameras);
}

//	=====================================================================
//	Initialize the only instance of the "AcesRender" class
//
//	inputs:
//      N/A
//
//	outputs:
//      static AcesRender &  : the referece to the only instance
//      of "AcesRender" class

AcesRender & AcesRender::getInstance(){
    mutex mtx;
    mtx.lock();
    static AcesRender acesrender;
    mtx.unlock();
    
    return acesrender;
}

//	=====================================================================
//	Operator = overloading in "AcesRender" class
//
//	inputs:
//      const AcesRender & : acesrender
//
//	outputs:
//      const AcesRender & : current instance filled with data from
//      acesrender

const AcesRender & AcesRender::operator=( const AcesRender& acesrender ) {
    if ( this != &acesrender ) {
        clearVM(_idtm);
        clearVM(_catm);
        clearVM(_wbv);
        clearVM(_illuminants);
        clearVM(_cameras);
        
        if (_idt) delete _idt;
        _idt = (Idt *) malloc(sizeof(Idt));
        memcpy(_idt, acesrender._idt, sizeof(Idt));
        
        if (_rawProcessor) delete _rawProcessor;
        _rawProcessor = (LibRawAces *) malloc(sizeof(_rawProcessor));
        memcpy((void*)_rawProcessor, (void*)acesrender._rawProcessor, sizeof(LibRawAces));
        
        if (_image) delete _image;
        _image = (libraw_processed_image_t *) malloc(sizeof(libraw_processed_image_t));
        memcpy(_image, acesrender._image, sizeof(libraw_processed_image_t));

        _idtm = acesrender._idtm;
        _catm = acesrender._catm;
        _wbv = acesrender._wbv;
        _illuminants = acesrender._illuminants;
        _cameras = acesrender._cameras;
        _opts = acesrender._opts;
    }
    
    return *this;
}

//	=====================================================================
//	Set option list
//
//	inputs:
//      option     : a list of initial options specified by users
//
//	outputs:
//      N/A        : _opts will be filled

void AcesRender::setSettings ( Option opts, libraw_output_params_t params ) {
    _opts = opts;
//    *(&(_rawProcessor->imgdata.params)) = OUT;
//    memcpy(&(_rawProcessor->imgdata.params), &OUT, sizeof(libraw_output_params_t));
    
#ifdef OUT
#undef OUT
#endif
    
#define OUT _rawProcessor->imgdata.params
    
    OUT.green_matching = params.green_matching;
    OUT.adjust_maximum_thr   = params.adjust_maximum_thr;
    OUT.threshold   = params.threshold;
//    OUT.bright      = params.bright;
    OUT.bad_pixels  = params.bad_pixels;
    OUT.dark_frame  = params.dark_frame;
    OUT.user_black  = params.user_black ;
    OUT.user_sat    = params.user_sat;
    OUT.user_flip   = params.user_flip;
    OUT.user_qual   = params.user_qual;
    OUT.med_passes  = params.med_passes;
    OUT.half_size   = params.half_size;
    OUT.four_color_rgb      = params.four_color_rgb;
    OUT.use_fuji_rotate     = params.use_fuji_rotate;
    OUT.no_auto_bright      = params.no_auto_bright;
    OUT.highlight   = params.highlight;
//    OUT.aber[0] = params.aber[0];
//    OUT.aber[0] = params.aber[2];

    OUT.cropbox[0]  = params.cropbox[0];
    OUT.cropbox[1]  = params.cropbox[1];
    OUT.cropbox[2]  = params.cropbox[2];
//    OUT.cropbox[3]  = params.cropbox[3];

    FORI(4) {
//        OUT.cropbox[i]  = params.cropbox[i];
        OUT.greybox[i] = params.greybox[i];
        OUT.user_mul[i] = params.user_mul[i];
    }
}


//	=====================================================================
//	Set processed image buffer from libraw
//
//	inputs:
//      libraw_processed_image_t     : processed RAW through libraw
//
//	outputs:
//      N/A        : _image will point to the address of image

void AcesRender::setPixels ( libraw_processed_image_t * image ) {
    assert(image);
    _image = image;

////    Strange because memcpying on libraw_processed_image_t
//    will generate an error
//    if (_image) delete _image;
//    _image = (libraw_processed_image_t *) malloc(sizeof(libraw_processed_image_t));
//    memcpy(_image, image, sizeof(*image));
}


//	=====================================================================
//	Gather supported Illuminants by reading from JSON files
//
//	inputs:
//      N/A
//
//	outputs:
//      N/A        : _illuminants be filled

void AcesRender::gatherSupportedIllums ( ) {
    
    if (_illuminants.size() != 0)
        _illuminants.clear();
    
    _illuminants.push_back( "Day-light (e.g., D60, D6025)" );
    _illuminants.push_back( "Blackbody (e.g., 3200K)" );
    
    std::unordered_map < string, int > record;
    
    FORI (_opts.EnvPaths.size()) {
        vector<string> iFiles = openDir ( static_cast< string >( (_opts.EnvPaths)[i] )
                                          +"/illuminant" );
        for ( vector<string>::iterator file = iFiles.begin(); file != iFiles.end(); ++file ) {
            string path( *file );
            try
            {
                ptree pt;
                read_json (path, pt);
                string tmp = pt.get<string>( "header.illuminant" );
                
                if ( record.find(tmp) != record.end() )
                    continue;
                else {
                    _illuminants.push_back (tmp);
                    record[tmp] = 1;
                }
            }
            catch( std::exception const & e )
            {
                std::cerr << e.what() << std::endl;
            }
        }
    }
}


//	=====================================================================
//	Gather supported cameras by reading from JSON files
//
//	inputs:
//      N/A
//
//	outputs:
//      N/A        : _cameras be filled

void AcesRender::gatherSupportedCameras ( ) {
    
    if (_cameras.size() != 0)
        _cameras.clear();
    
    std::unordered_map < string, int > record;
    printf("%s\n", _opts.EnvPaths[0].c_str());
    
    FORI (_opts.EnvPaths.size()) {
        vector<string> iFiles = openDir ( static_cast< string >( (_opts.EnvPaths)[i] )
                                          +"/camera" );
        for ( vector<string>::iterator file = iFiles.begin(); file != iFiles.end(); ++file ) {
            string path( *file );
            try
            {
                ptree pt;
                read_json (path, pt);
                string tmp = pt.get<string>( "header.manufacturer" );
                tmp += ( " / " + pt.get<string>( "header.model" ) );
                
                if ( record.find(tmp) != record.end() )
                    continue;
                else {
                    _cameras.push_back (tmp);
                    record[tmp] = 1;
                }
            }
            catch( std::exception const & e )
            {
                std::cerr << e.what() << std::endl;
            }
        }
    }
}

//	=====================================================================
//  Open the RAW file from the path to the file
//
//	inputs:
//      const char *       : path to the raw file
//
//	outputs:
//		int                : "1" means raw file successfully opened;
//                           "0" means error when opening the file

int AcesRender::openRawPath ( const char * pathToRaw ) {
    assert ( pathToRaw != nullptr );
    
#ifndef WIN32
//    void *iobuffer=0;
    struct stat st;
    
    if ( _opts.use_mmap )
    {
        int file = open ( pathToRaw, O_RDONLY );
        
        if( file < 0 )
        {
            fprintf ( stderr, "\nError: Cannot open %s: %s\n\n",
                              pathToRaw, strerror(errno) );
            _opts.ret = 0;
            
            return 0;
        }
        
        if( fstat ( file, &st ) )
        {
            fprintf ( stderr, "\nError: Cannot stat %s: %s\n\n",
                              pathToRaw, strerror(errno) );
            close( file );
            _opts.ret = 0;
            
            return 0;
        }
        
        int pgsz = getpagesize();
        _opts.msize = (( st.st_size+pgsz-1 ) / pgsz ) * pgsz;
        _opts.iobuffer = mmap ( NULL, size_t(_opts.msize), PROT_READ, MAP_PRIVATE, file, 0 );
        if( !_opts.iobuffer )
        {
            fprintf ( stderr, "\nError: Cannot mmap %s: %s\n\n",
                              pathToRaw, strerror(errno) );
            close( file );
            _opts.ret = 0;
            
            return 0;
        }
        
        close( file );
        if (( _opts.ret = _rawProcessor->open_buffer( _opts.iobuffer,st.st_size ) != LIBRAW_SUCCESS ))
        {
            fprintf ( stderr, "\nError: Cannot open_buffer %s: %s\n\n",
                              pathToRaw, libraw_strerror(_opts.ret) );
        }
    }
    else
#endif
    {
        if ( _opts.use_bigfile )
            _opts.ret = _rawProcessor->open_file ( pathToRaw, 1 );
        else
            _opts.ret = _rawProcessor->open_file ( pathToRaw );
    }
    
    unpack ( pathToRaw );
    
    return _opts.ret;
}

//	=====================================================================
//  Unpack the RAW file based on the path to the file (after openRawPath)
//
//	inputs:
//      const char *       : path to the raw file
//
//	outputs:
//		int                : "1" means raw file successfully unpacked;
//                           "0" means error when unpacking the file

int AcesRender::unpack ( const char * pathToRaw ) {
    assert ( _opts.ret == LIBRAW_SUCCESS && pathToRaw != nullptr );
    
    if (( _opts.ret = _rawProcessor->unpack() ) != LIBRAW_SUCCESS )
    {
        fprintf ( stderr, "\nError: Cannot unpack %s: %s\n\n",
                           pathToRaw, libraw_strerror (_opts.ret) );
    }
    
    return _opts.ret;
}

//	=====================================================================
//	Read camera spectral sensitivity data from path
//
//	inputs:
//      const char *     : camera spectral sensitivity path
//                         (either in the environment variable of
//                          "AMPAS_CAMERA_SENSITIVITIES_PATH"
//                          such as  "/usr/local/include/rawtoaces/data/camera"
//                          or specified by users)
//      libraw_iparams_t : main parameters read from RAW
//
//	outputs:
//		int              : "1" means loading/injecting camera spectral
//                         sensitivity data successfully;
//                         "0" means error in reading/injecting data

int AcesRender::fetchCameraSenPath( libraw_iparams_t P )
{
    int readC = 0;
    
    FORI (_opts.EnvPaths.size()) {
        vector<string> cFiles = openDir ( static_cast< string >( (_opts.EnvPaths)[i] )
                                          +"/camera" );
        for ( vector<string>::iterator file = cFiles.begin( ); file != cFiles.end( ); ++file ) {
            string fn( *file );
            
            if ( fn.find(".json") == std::string::npos )
                continue;
            readC = _idt->loadCameraSpst( fn, P.make, P.model );
            if ( readC ) return 1;
        }
    }
    
    return readC;
}

//	=====================================================================
//	Fetch light source data to calcuate white balance coefficients
//
//	inputs:
//      const char *  : type of light source ("unknown" if not specified)
//                      (in the environment variable of "AMPAS_ILLUMINANT_PATH"
//                       such as "/usr/local/include/rawtoaces/data/Illuminant")
//
//	outputs:
//		int : "1" means loading/injecting light source datasets successfully,
//            "0" means error / no illumiant data has been loaded


// To be updated
int AcesRender::fetchIlluminant ( const char * illumType )
{
    vector <string> paths;
    
    FORI ( _opts.EnvPaths.size() ) {
        vector <string> iFiles = openDir ( (_opts.EnvPaths)[i] + "/illuminant" );
        for ( vector<string>::iterator file = iFiles.begin(); file != iFiles.end(); ++file ) {
            string fn( *file );
            if ( fn.find(".json") == std::string::npos )
                continue;
            paths.push_back(fn);
        }
    }
    
    return _idt->loadIlluminant( paths, (string)illumType );
}

//	=====================================================================
//  Calculate IDT matrix from camera spectral sensitivity data and the
//  selected or specified light source data. THe best White balance
//  coefficients will be generated in the process of obtaining IDT.
//
//	inputs:
//      libraw_iparams_t              : main parameters read from RAW
//      libraw_colordata_t            : color information from RAW
//
//	outputs:
//		int                           : "1" means IDT matrix generated;
//                                      "0" means error in calculation.

int AcesRender::prepareIDT ( libraw_iparams_t P, float * M )
{
// _rawProcessor->imgdata.idata
    int read = fetchCameraSenPath( P );
    
    if ( !read ) {
        fprintf( stderr, "\nError: No matching cameras found. "
                         "Please use other options for "
                         "\"--mat-method\" and/or \"--wb-method\".\n");
        exit (-1);
    }
    
    // loading training data (190 patches)
    _idt->loadTrainingData ( static_cast < string > ( FILEPATH )
                             +"training/training_spectral.json" );
    // loading color matching function
    _idt->loadCMF ( static_cast < string > ( FILEPATH )
                    +"cmf/cmf_1931.json" );
    
    _idt->setVerbosity(_opts.verbosity);
    if ( _opts.illumType )
        _idt->chooseIllumType( _opts.illumType, _opts.highlight );
    else {
        vector < double > mulV (M, M+3);
        _idt->chooseIllumSrc ( mulV, _opts.highlight );
    }
    
    if (_opts.verbosity > 1)
    	printf ( "Regressing IDT matrix coefficients ...\n" );
    
    if ( _idt->calIDT() )  {
        _idtm = _idt->getIDT();
        _wbv = _idt->getWB();
        
        return 1;
    }
    
    return 0;
}


//	=====================================================================
//  Calculate just white balance coefficients from camera spectral
//  sensitivity data and the best or specified light source data.
//  IDT matrix will not be calculated here, as curve-fitting may
//  take more time
//
//	inputs:
//      libraw_iparams_t   : main parameters read from RAW
//      libraw_colordata_t : color information from RAW
//
//	outputs:
//		int                : "1" means white balance coefficients generated;
//                           "0" means error during calculation

int AcesRender::prepareWB ( libraw_iparams_t P )
{
    int read = fetchCameraSenPath ( P );

    if ( !read ) {
        fprintf( stderr, "\nError: No matching cameras found. "
                         "Please use other options for "
                         "\"--wb-method\".\n");
        exit (-1);
    }

    assert(_opts.illumType);
    read = fetchIlluminant( _opts.illumType );

    if( !read ) {
        fprintf( stderr, "\nError: No matching light source. "
                         "Please find available options by "
                         "\"rawtoaces --valid-illum\".\n");
        exit (-1);
    }
    else
    {
        // loading training data (190 patches)
        _idt->loadTrainingData ( static_cast < string > ( FILEPATH )
                                +"/training/training_spectral.json" );

        // loading color matching function
        _idt->loadCMF ( static_cast < string > ( FILEPATH )
                      +"/cmf/cmf_1931.json" );

        // choose the best light source based on
        // as-shot white balance coefficients
       _idt->chooseIllumType( _opts.illumType, _opts.highlight );

       if (_opts.verbosity > 1) {
           printf ( "Calculating White Balance Coefficients "
                    "from Spectral Sensitivity ...\n" );
           printf ( "Applying Calculated White Balance "
                    "Coefficients ...\n" );
       }

       _wbv = _idt->getWB();

       return 1;
    }

    return 0;
}

//  =====================================================================
//  Conduct dcraw process on the RAW
//
//  inputs:
//      const char *       : path to the raw file
//
//  outputs:
//      int                : "1" means raw file successfully pre-processed;
//                           "0" means error when pre-processing the file

int AcesRender::dcraw ( ) {
    assert ( _opts.ret ==  LIBRAW_SUCCESS );

    if ( LIBRAW_SUCCESS != ( _opts.ret = _rawProcessor->dcraw_process() ) ) {      
        fprintf ( stderr, "Error: Cannot do postpocessing: %s\n\n",
                           libraw_strerror(_opts.ret) );
        _opts.ret = errno;

        if ( LIBRAW_FATAL_ERROR( _opts.ret ) )
            exit(1);
    }

    return _opts.ret;
}


//  =====================================================================
//  Preprocess the RAW file based on the path to the file
//
//  inputs:
//      const char *       : path to the raw file
//
//  outputs:
//      int                : "1" means raw file successfully pre-processed;
//                           "0" means error when pre-processing the file

int AcesRender::preprocessRaw ( const char * pathToRaw ) {
    assert ( pathToRaw != nullptr );
    
    size_t len = strlen(pathToRaw);
    _pathToRaw = (char *) malloc(len+1);
    memset(_pathToRaw, 0x0, len);
    memcpy(_pathToRaw, pathToRaw, len);
    _pathToRaw[len] = '\0';

   // if ( _opts.verbosity > 2 )
   //     _rawProcessor->set_progress_handler ( my_progress_callback,
   //                                         ( void * )"Sample data passed" );
    // Start rawtoaces
    if ( _opts.verbosity ) {
        printf( "\nStarting rawtoaces ...\n");
        printf ( "Processing %s ...\n", pathToRaw );
    }
    
#ifdef LIBRAW_USE_OPENMP
   if( _opts.verbosity )
       printf ( "Using %d threads\n", omp_get_max_threads() );
#endif
    
    if ( openRawPath ( pathToRaw ) )
        unpack ( pathToRaw );
    
    return _opts.ret;
}

//  =====================================================================
//  Postprocess the RAW file 
//
//  inputs:
//      N/A
//
//  outputs:
//      int                : "1" means raw file successfully pre-processed;
//                           "0" means error when pre-processing the file

int AcesRender::postprocessRaw ( ) {
    assert ( _opts.ret == LIBRAW_SUCCESS );
    
#ifdef OUT
#undef OUT
#endif

#define OUT _rawProcessor->imgdata.params
#define P  _rawProcessor->imgdata.idata
#define C   _rawProcessor->imgdata.color
    
//  General set-up
    OUT.output_color      = 5;
    OUT.output_bps        = 16;
    OUT.highlight         = 0;
    OUT.use_camera_matrix = 0;
    OUT.gamm[0]           = 1.0;
    OUT.gamm[1]           = 1.0;
    OUT.no_auto_bright    = 1;
    OUT.use_camera_wb     = 0;
    OUT.use_auto_wb       = 0;

    if (_opts.verbosity > 1)
        printf ( "The camera has been identified as a %s %s ...\n", P.make, P.model );
    
    if ( P.dng_version ) {
        fprintf(stderr, "\nCurrently does not support DNG files.\n");
        exit(1);
    }
    
    switch ( _opts.wb_method ) {
    // 0
        case wbMethod0 : {
            _opts.use_mul = 1;
            vector < double > mulV (C.cam_mul, C.cam_mul+3);
            
            FORI(3) OUT.user_mul[i] = static_cast<float>(mulV[i]);
            
            if ( _opts.verbosity > 1 ) {
                printf ( "White Balance method is 0 - ");
                printf ( "Using white balance factors from "
                         "file metadata ...\n" );
            }
            break;
        }
        // 1
        case wbMethod1 : {
            if ( prepareWB ( _rawProcessor->imgdata.idata ) ) {
                _opts.use_mul = 1;
                vector < double > wbv = getWB();
                FORI(3) OUT.user_mul[i] = static_cast<float>(wbv[i]);
            }
            else {
                fprintf ( stderr, "\nError: Cannot obtain a set of White "
                                  "Balance Coefficient Factors \n" );
                _opts.ret = errno;
                return 0;
            }
            
            if ( _opts.verbosity > 1 ) {
                printf ( "White Balance calculation method is 1 - ");
                printf ( "Using white balance factors calculated from "
                         "spec sens and user specified illuminant ...\n" );
            }
            
            break;
        }
        // 2
        case wbMethod2 : {
            OUT.use_auto_wb = 1;
            
            if ( _opts.verbosity > 1 ) {
                printf ( "White Balance calculation method is 2 - ");
                printf ( "Using white balance factors calculate by "
                        "averaging the entire image ...\n" );
            }
            
            break;
        }
        // 3
        case wbMethod3 : {
            if ( _opts.verbosity > 1 ) {
                printf ( "White Balance calculation method is 3 - ");
                printf ( "Using white balance factors calculated by "
                     "averaging a grey box ...\n" );
            }
            
            break;
        }
        // 4
        case wbMethod4 : {
            _opts.use_mul = 1;
            double sc = dmax;
            
            FORI ( P.colors ){
                if ( OUT.user_mul[i] <= sc )
                    sc = OUT.user_mul[i];
            }
            
            if ( sc != 1.0 ) {
                fprintf ( stderr, "\nWarning: The smallest channel  "
                                  "multiplier should be 1.0.\n\n" );
            }
            
            if ( _opts.verbosity > 1 ) {
                printf ( "White Balance calculation method is 4 - ");
                printf ( "Using user-supplied white balance factors ...\n" );
            }
            
            break;
        }
        default: {
            fprintf ( stderr, "White Balance method is must be 0, 1, 2, 3, "
                              "or 4 \n" );
            exit(-1);
            break;
        }
    }

//  Set parameters for --mat-method
    switch ( _opts.mat_method ) {
        // 0
        case matMethod0 : {
            OUT.output_color = 0;
            
            if ( _opts.verbosity > 1 ) {
                printf ( "IDT matrix calculation method is 0 - ");
                printf ( "Calculating IDT matrix using camera spec sens ...\n" );
            }
            
            break;
        }
        // 1
        case matMethod1 :
            OUT.use_camera_matrix = 0;
            
            if ( _opts.verbosity > 1 ) {
                printf ( "IDT matrix calculation method is 1 - ");
                printf ( "Calculating IDT matrix using file metadata ...\n" );
            }
            
            break;
        // 2
        case matMethod2 :
            OUT.use_camera_matrix = 3;
            
            if ( _opts.verbosity > 1 ) {
                printf ( "IDT matrix calculation method is 2 - ");
                printf ( "Calculating IDT matrix using adobe coeffs included in libraw ...\n" );
            }
            
            break;
        default:
            fprintf ( stderr, "IDT matrix calculation method is must be 0, 1, 2 \n" );
            exit(-1);
            break;
    }

// Set four_color_rgb to 0 when half_size is set to 1
    if ( OUT.half_size == 1 )
         OUT.four_color_rgb = 0;

    _opts.ret = dcraw();
    if ( _opts.mat_method == matMethod0 )
        if ( !prepareIDT ( P, C.pre_mul ) )
            _opts.ret = errno;

    libraw_processed_image_t * image = _rawProcessor->dcraw_make_mem_image ( &(_opts.ret) );
    setPixels (image);
    
    return _opts.ret;
}


float * AcesRender::renderACES ( ) {
    if ( !_rawProcessor->imgdata.params.output_color )
        return renderNonDNG_IDT();
    else
        return renderNonDNG();
}


void AcesRender::outputACES ( ) {
#ifdef C
#undef C
#endif

#define C   _rawProcessor->imgdata.color

    assert(_pathToRaw != nullptr);

    char * cp;
    if (( cp = strrchr ( _pathToRaw, '.' ))) *cp = 0;
    
    char outfn[1024];
    
    snprintf( outfn,
             sizeof(outfn),
             "%s%s",
             _pathToRaw,
             "_aces.exr" );
    
    if ( _opts.verbosity > 1 ) {
        if (_opts.mat_method) {
            vector <vector < double > > camXYZ(3, vector< double >(3, 1.0));
            FORIJ(3,3) camXYZ[i][j] = C.cam_xyz[i][j];
            vector <vector < double > > camcat = mulVector(camXYZ, _catm);
            
            printf("The IDT matrix is ...\n");
            FORI(3) printf("   %f, %f, %f\n", camcat[i][0], camcat[i][1], camcat[i][2]);
        }
        
        // printing white balance coefficients
        printf ("The final white balance coefficients are ...\n");
        printf ("   %f   %f   %f\n", C.pre_mul[0], C.pre_mul[1], C.pre_mul[2]);
        
        printf ( "Writing ACES file to %s ...\n", outfn );
    }
    
    float * aces = renderACES();
    
    if ( _opts.highlight > 0 ) {
        float ratio = ( *(std::max_element ( C.pre_mul, C.pre_mul+3)) /
                        *(std::min_element ( C.pre_mul, C.pre_mul+3)) );
        acesWrite ( outfn, aces, ratio );
    }
    else
        acesWrite ( outfn, aces );
    
#ifndef WIN32
    if ( _opts.use_mmap && _opts.iobuffer )
    {
        munmap ( _opts.iobuffer, size_t(_opts.msize) );
        _opts.iobuffer = 0;
    }
#endif
    
    _rawProcessor->recycle();

    if (_opts.verbosity)
        printf ("Finished\n\n");
}


//	=====================================================================
//  Apply white balance values to each pixel
//  ( We actually do not need it here because white-balancing
//  happens before demosaicing )
//
//	inputs:
//      float *          : pixels (R/G/B)
//      uint8_t          : number of channels
//      uint32_t         : the size of pixels
//
//	outputs:
//		N/A              : pixel values modified by multiplying
//                         white balance coefficients

void AcesRender::applyWB ( float * pixels, int bits, uint32_t total )
{
    double min_wb = * min_element ( _wbv.begin(), _wbv.end() );
    double target = 1.0;
    
    if ( bits == 8 )
        target /= INV_255;
    else if ( bits == 16 )
        target /= INV_65535;
    
    if ( !pixels ) {
        fprintf ( stderr, "\nThe pixels cannot be found. \n" );
        exit (1);
    }
    else {
        for ( uint32_t i = 0; i < total; i+=3 ){
            pixels[i]   = clip (_wbv[0] * pixels[i] / min_wb, target);
            pixels[i+1] = clip (_wbv[1] * pixels[i+1] / min_wb, target);
            pixels[i+2] = clip (_wbv[2] * pixels[i+2] / min_wb, target);
        }
    }
}


//	=====================================================================
//  Apply IDT matrix to each pixel
//
//	inputs:
//      float *   : pixels (R/G/B)
//      uint8_t   : number of channels
//      uint32_t  : the size of pixels
//      vector < vector <double> >: 3 x 3 IDT matrix
//
//	outputs:
//		N/A       : pixel values modified by mutiplying IDT matrix

void AcesRender::applyIDT ( float * pixels, int channel, uint32_t total )
{
    assert(pixels);
    
    if ( channel == 4 ){
        _idtm.resize(4);
        FORI(4) _idtm[i].resize(4);
        
        _idtm[0][3] = 0.0;
        _idtm[1][3] = 0.0;
        _idtm[2][3] = 0.0;
        _idtm[3][0] = 0.0;
        _idtm[3][1] = 0.0;
        _idtm[3][2] = 0.0;
        _idtm[3][3] = 1.0;
    }
    
    if ( channel != 3 && channel != 4 ) {
        fprintf ( stderr, "\nError: Currenly support 3 channels "
                          "and 4 channels. \n" );
        exit (1);
    }
    
    pixels = mulVectorArray ( pixels,
                              total,
                              channel,
                              _idtm );
}


//	=====================================================================
//  Apply CAT matrix (e.g., D50 to D60) to each pixel
//  It will be used if using adobe coeffs from "libraw"
//
//	inputs:
//      float *   : pixels (R/G/B)
//      uint8_t   : number of channels
//      uint32_t  : the size of pixels
//
//	outputs:
//		N/A       : pixel values modified by mutiplying CAT matrix

void AcesRender::applyCAT ( float * pixels, int channel, uint32_t total )
{
    assert(pixels);
    
    if ( channel != 3 && channel != 4 ) {
        fprintf ( stderr, "\nError: Currenly support 3 channels "
                 "and 4 channels. \n" );
        exit (1);
    }
    
    // will use calCAT() inside rawtoaces
    vector < double > dIV (d50, d50 + 3);
    vector < double > dOV (d60, d60 + 3);
    
    _catm = _idt->calCAT(dIV, dOV);

    pixels = mulVectorArray ( pixels,
                              total,
                              channel,
                              _catm );
}


//	=====================================================================
//  Convert DNG RAW to aces file
//
//	inputs:
//      vector < float > : camera to display matrix
//
//	outputs:
//		float * : an array of converted aces values

float * AcesRender::renderDNG ( vector < float > cameraToDisplayMtx )
{
    assert(_image);
    
    ushort * pixels = (ushort *) _image->data;
    uint32_t total = _image->width * _image->height * _image->colors;
    vector < vector< double> > CMT( _image->colors,
                                    vector< double >(_image->colors) );

    float * aces = new  (std::nothrow) float[total];
    FORI (total)
        aces[i] = static_cast < float > (pixels[i]);
    
    FORIJ(3, 3)
        CMT[i][j] = static_cast < double > (cameraToDisplayMtx[i*3+j]);
   
    if(_image->colors == 3) {
        aces = mulVectorArray( aces,
                               total,
                               3,
                               CMT);
    }
    else if(_image->colors == 4){
        CMT[0][3]=0.0;
        CMT[1][3]=0.0;
        CMT[2][3]=0.0;
        CMT[3][3]=1.0;
        CMT[3][0]=0.0;
        CMT[3][1]=0.0;
        CMT[3][2]=0.0;
        
        aces =  mulVectorArray( aces,
                                total,
                                4,
                                CMT );
    }
    else {
        fprintf(stderr, "\nError: Currenly support 3 channels "
                        "and 4 channels. \n");
        exit(1);
    }

    return aces;
}


//	=====================================================================
//  Convert Non-DNG RAW to aces file (no IDT involved)
//
//	inputs:  N/A
//
//	outputs:
//		float * : an array of converted aces code values

float * AcesRender::renderNonDNG ()
{
    assert(_image);

    ushort * pixels = (ushort *) _image->data;
    uint32_t total = _image->width * _image->height * _image->colors;
    float * aces = new (std::nothrow) float[total];
    
    FORI(total) aces[i] = static_cast <float> (pixels[i]);
    
    if(_opts.mat_method > 0) {
        applyCAT(aces, _image->colors, total);
    }
    
    vector < vector< double> > XYZ_acesrgb( _image->colors,
                                            vector < double > (_image->colors));
    if (_image->colors == 3) {
        FORIJ(3, 3) XYZ_acesrgb[i][j] = XYZ_acesrgb_3[i][j];
        aces = mulVectorArray(aces, total, 3, XYZ_acesrgb);
    }
    else if (_image->colors == 4){
        FORIJ(4, 4) XYZ_acesrgb[i][j] = XYZ_acesrgb_4[i][j];
        aces = mulVectorArray(aces, total, 4, XYZ_acesrgb);
    }
    else {
        fprintf ( stderr, "\nError: Currenly support 3 channels "
                          "and 4 channels. \n" );
        exit (1);
    }
    
    return aces;
}

//	=====================================================================
//  Convert Non-DNG RAW to aces file through IDT
//
//	inputs:  N/A
//
//	outputs:
//		float * : an array of aces values for each pixel

float * AcesRender::renderNonDNG_IDT ()
{
    assert(_image);
    ushort * pixels = (ushort *) _image->data;
    uint32_t total = _image->width * _image->height * _image->colors;
    float * aces = new (std::nothrow) float[total];
    
    FORI(total) {
        aces[i] = static_cast <float> (pixels[i]);
    }

    if (_opts.verbosity > 1)
    	printf ( "Applying IDT Matrix ...\n" );
    
    applyIDT ( aces, _image->colors, total );
    
    return aces;
};


//	=====================================================================
//  Write processed image file to an aces-compliant openexr file
//
//	inputs:
//      const char *               : the name of output file
//      float *                    : an array of converted aces values
//
//	outputs:
//		N/A                        : an aces file should be generated in
//                                   the same folder

void AcesRender::acesWrite ( const char * name, float *  aces, float ratio ) const
{
    assert(aces);

    uint16_t width     = _image->width;
    uint16_t height    = _image->height;
    uint8_t  channels  = _image->colors;
    uint8_t  bits      = _image->bits;
    
    halfBytes *halfIn = new (std::nothrow) halfBytes[channels * width * height];
        
    FORI ( channels * width * height ){
        if ( bits == 8 )
            aces[i] = (double) aces[i] * INV_255 * (_opts.scale) * ratio;
        else if ( bits == 16 )
            aces[i] = (double) aces[i] * INV_65535 * (_opts.scale) * ratio;
        
        half tmpV ( aces[i] );
        halfIn[i] = tmpV.bits();
    }
    
    std::vector<std::string> filenames;
    filenames.push_back(name);
    
    aces_Writer x;
    
    MetaWriteClip writeParams;
    
    writeParams.duration				= 1;
    writeParams.outputFilenames			= filenames;
    
    writeParams.outputRows				= height;
    writeParams.outputCols				= width;
    
    writeParams.hi = x.getDefaultHeaderInfo();
    writeParams.hi.originalImageFlag	= 1;
    writeParams.hi.software				= "rawtoaces v0.1";
    
    writeParams.hi.channels.clear();
    
    switch (channels)
    {
        case 3:
            writeParams.hi.channels.resize(3);
            writeParams.hi.channels[0].name = "B";
            writeParams.hi.channels[1].name = "G";
            writeParams.hi.channels[2].name = "R";
            break;
        case 4:
            writeParams.hi.channels.resize(4);
            writeParams.hi.channels[0].name = "A";
            writeParams.hi.channels[1].name = "B";
            writeParams.hi.channels[2].name = "G";
            writeParams.hi.channels[3].name = "R";
            break;
        case 6:
            throw std::invalid_argument("Stereo RGB support not yet implemented");
        case 8:
            throw std::invalid_argument("Stereo RGB support not yet implemented");
        default:
            throw std::invalid_argument("Only RGB, RGBA or"
                                        "stereo RGB[A] file supported");
            break;
    }
    
    DynamicMetadata dynamicMeta;
    dynamicMeta.imageIndex = 0;
    dynamicMeta.imageCounter = 0;
    
    x.configure ( writeParams );
    x.newImageObject ( dynamicMeta );
    
    FORI ( height ){
        halfBytes * rgbData = halfIn + width * channels * i;
        x.storeHalfRow (rgbData, i);
    }
    
#if 0
    std::cout << "saving aces file" << std::endl;
    std::cout << "size " << width << "x" << height << "x"
              << channels << std::endl;
    std::cout << "size " << writeParams.outputCols << "x"
              << writeParams.outputRows << std::endl;
    std::cout << "duration " << writeParams.duration << std::endl;
    std::cout << writeParams.hi;
    std::cout << "\ndynamic meta" << std::endl;
    std::cout << "imageIndex " << dynamicMeta.imageIndex << std::endl;
    std::cout << "imageCounter " << dynamicMeta.imageCounter << std::endl;
    std::cout << "timeCode " << dynamicMeta.timeCode << std::endl;
    std::cout << "keyCode " << dynamicMeta.keyCode << std::endl;
    std::cout << "capDate " << dynamicMeta.capDate << std::endl;
    std::cout << "uuid " << dynamicMeta.uuid << std::endl;
#endif
    
    x.saveImageObject ( );
}


//	=====================================================================
//	Get a list of Supported Illuminants
//
//	inputs:
//      N/A
//
//	outputs:
//      vector < vector < string > > : _illuminant values
//	=====================================================================

const vector < string > AcesRender::getSupportedIllums ( ) const {
    return _illuminants;
}


//	=====================================================================
//	Get a list of Supported Cameras
//
//	inputs:
//      N/A
//
//	outputs:
//      vector < string > : _cameras values

const vector < string > AcesRender::getSupportedCameras ( ) const {
    return _cameras;
}


//	=====================================================================
//	Print a list of Cameras Supported by LibRaw
//
//	inputs:
//      N/A
//
//	outputs:
//      N/A

void AcesRender::printLibRawCameras () {
    const char ** cl = _rawProcessor->cameraList();
    while (*(cl+1) != NULL)
        printf ("%s\n", *cl++);
}

//	=====================================================================
//	Get IDT matrix
//
//	inputs:
//      N/A
//
//	outputs:
//      vector < vector < double > > : _idtm (3x3) values

const vector < vector < double > > AcesRender::getIDTMatrix ( ) const {
    return _idtm;
}

//	=====================================================================
//	Get CAT matrix
//
//	inputs:
//      N/A
//
//	outputs:
//      vector < vector < double > > : _catm (3x3) values

const vector < vector < double > > AcesRender::getCATMatrix ( ) const {
    return _catm;
}

//	=====================================================================
//	Get White Balance Coefficients
//
//	inputs:
//      N/A
//
//	outputs:
//      vector < double >  : _wbv (1x3) values

const vector < double > AcesRender::getWB ( ) const {
    return _wbv;
}

//	=====================================================================
//	Get Image Buffer
//
//	inputs:
//      N/A
//
//	outputs:
//      libraw_processed_image_t : _image

const libraw_processed_image_t * AcesRender::getImageBuffer() const {
    assert(_image);
    
    return _image;
}

