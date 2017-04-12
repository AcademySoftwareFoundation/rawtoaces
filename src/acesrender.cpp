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

AcesRender::AcesRender(){
    _idt = new Idt();
    
    _idtm.resize(3);
    _wbv.resize(3);
    
    FORI(3) {
        _idtm[i].resize(3);
        _wbv[i] = 1.0;
        FORJ(3) _idtm[i][j] = neutral3[i][j];
    }
}

AcesRender::~AcesRender(){
    delete _idt;
    vector < vector<double> >().swap(_idtm);
    vector < double >().swap(_wbv);
}

//	=====================================================================
//	Set option list
//
//	inputs:
//      option     : a list of initial options specified by users
//
//	outputs:
//      N/A        : _opts will be filled

void AcesRender::setOptions (option opts) {
    _opts = opts;
}


//	=====================================================================
//	Set processed image buffer from libraw
//
//	inputs:
//      libraw_processed_image_t     : processed RAW through libraw
//
//	outputs:
//      N/A        : _image will point to the address of image

void AcesRender::setPixels (libraw_processed_image_t * image) {
    assert(image);
    _image = image;
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

int AcesRender::readCameraSenPath( const char * cameraSenPath,
                                   libraw_iparams_t P )
{
    int readC = 0;
    struct stat st;
    
    if ( cameraSenPath )  {
        if ( stat( static_cast <const char *> ( cameraSenPath ), &st ) )  {
            fprintf ( stderr, "The camera sensitivity file does "
                              "not seem to exist. Please use other"
                              "options for \"mat-method\".\n" );
            exit (1);
        }
        
        readC = _idt->loadCameraSpst( cameraSenPath,
                                      static_cast <const char *> (P.make),
                                      static_cast <const char *> (P.model),
                                      _opts.use_camera_path );
    }
    else  {
        FORI (_opts.cEnvPaths.size()) {
            string cPath = (_opts.cEnvPaths)[i];            
//          vector<string> cFiles = openDir ( static_cast <string> ( FILEPATH )
//                                                  +"/camera" );
            vector<string> cFiles = openDir ( cPath );

            for ( vector<string>::iterator file = cFiles.begin( ); file != cFiles.end( ); ++file ) {
                string fn( *file );
                
                readC = _idt->loadCameraSpst( fn,
                                              static_cast <const char *> (P.make),
                                              static_cast <const char *> (P.model),
                                              _opts.use_camera_path );
                if ( readC ) return 1;
            }
        }
    }
    
    return readC;
}


//	=====================================================================
//	Read light source data to calcuate white balance coefficients
//
//	inputs:
//      const char *  : type of light source ("unknown" if not specified)
//                      (in the environment variable of "AMPAS_ILLUMINANT_PATH"
//                       such as "/usr/local/include/rawtoaces/data/Illuminant")
//      map <string, vector <double>> : key is the path (string) to each
//                                      light source; value is calculated
//                                      white balance coefficients (vector)
//                                      for each light source
//
//	outputs:
//		int : "1" means loading/injecting light source data successfully;
//            white balance coefficients will also be calculated in the meantime.
//            "0" means error in reading/ injecting data

int AcesRender::readIlluminant( const char * illumType,
                                map < string, vector < double > > & illuCM )
{
    int readI = 0;
    struct stat st;
    
    FORI (_opts.iEnvPaths.size()) {
        string iPath = (_opts.iEnvPaths)[i];
        vector <string> iFiles = openDir( iPath );
//        printf("%s, ", iPath.c_str());

        for ( vector<string>::iterator file = iFiles.begin(); file != iFiles.end(); ++file ) {
            string fn( *file );
            string strType(illumType);
                
            const char * illumC = static_cast < const char * >( illumType );

            if ( strType.compare("unknown") != 0 ) {
                readI = _idt->loadIlluminant( fn, illumC );
                if ( readI )
                {
                    illuCM[fn] = _idt->calWB( _opts.highlight );
                    return 1;
                }
            }
            else {
                readI = _idt->loadIlluminant( fn, illumC );
                if ( readI ) illuCM[fn] = _idt->calWB( _opts.highlight );
            }
        }
    }
    
    return readI;
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

int AcesRender::prepareIDT ( libraw_iparams_t P, libraw_colordata_t C )
{
    const char * cameraSenPath = static_cast <const char *> (_opts.cameraSenPath);
    const char * illumType = static_cast <const char *> (_opts.illumType);

    int read = readCameraSenPath( cameraSenPath, P );
    
    if ( !read &&
         !_opts.use_camera_path ) {
        fprintf( stderr, "\nError: No matching cameras found. "
                         "Please use other options for "
                         "\"--mat-method\" and/or \"--wb-method\".\n");
        exit (1);
    }

    if ( !illumType ) illumType = "unknown";
    
    map < string, vector < double > > illuCM;
    read = readIlluminant( illumType, illuCM );
    
    if( !read ) {
        fprintf( stderr, "\nError: No matching light source. "
                         "Please use other options for "
                         "\"--mat-method\" or \"--wb-method\".\n");
        exit (1);
    }
    else
    {
        printf ( "\nThe matching camera is: %s %s\n", P.make, P.model );
        
        vector < double > mulV (C.cam_mul, C.cam_mul+3);
        if( !_opts.highlight )
            scaleVectorMin (mulV);
        else
            scaleVectorMax (mulV);
        
        // loading training data (190 patches)
        _idt->loadTrainingData ( static_cast < string > ( FILEPATH )
                                 +"training/training_spectral.json" );
        // loading color matching function
        _idt->loadCMF ( static_cast < string > ( FILEPATH )
                        +"cmf/cmf_1931.json" );
        _idt->chooseIlluminant ( illuCM, mulV, illumType );
        
        printf ( "\nCalculating IDT Matrix from Spectral Sensitivity ...\n\n" );
        
        _idt->setVerbosity(_opts.verbosity);
        if (_opts.use_mat == 0 &&
            !_opts.verbosity )
            _idt->setVerbosity(1);
        
        if ( _idt->calIDT() )  {
            _idtm = _idt->getIDT();
            _wbv = _idt->getWB();
            
            return 1;
        }
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

int AcesRender::prepareWB ( libraw_iparams_t P, libraw_colordata_t C )
{
    const char * cameraSenPath = static_cast <const char *> (_opts.cameraSenPath);
    const char * illumType = static_cast <const char *> (_opts.illumType);
    
    int read = readCameraSenPath ( cameraSenPath, P );
    
    if ( !read &&
         !_opts.use_camera_path ) {
        fprintf( stderr, "\nError: No matching cameras found. "
                         "Please use other options for "
                         "\"--wb-method\".\n");
        exit (1);
    }
    
    if ( !illumType ) illumType = "unknown";
    
    map < string, vector < double > > illuCM;
    read = readIlluminant( illumType, illuCM );
    
    if( !read ) {
        fprintf( stderr, "\nError: No matching light source. "
                "Please use other options for "
                "\"--mat-method\" or \"--wb-method\".\n");
        exit (1);
    }
    else
    {
        printf ( "\nThe camera is: %s %s\n", P.make, P.model );
        
        vector < double > mulV (C.cam_mul, C.cam_mul+3);
        if( !_opts.highlight )
            scaleVectorMin (mulV);
        else
            scaleVectorMax (mulV);
        
        // loading training data (190 patches)
        _idt->loadTrainingData ( static_cast < string > ( FILEPATH )
                                +"/training/training_spectral" );
        
        // loading color matching function
        _idt->loadCMF ( static_cast < string > ( FILEPATH )
                      +"/cmf/json/cmf_1931.json" );
        
        // choose the best light source based on
        // as-shot white balance coefficients
        _idt->chooseIlluminant( illuCM, mulV, illumType );
        
        printf ( "\nCalculating White Balance Coefficients "
                "from Spectral Sensitivity...\n" );
        printf ( "\nApplying Calculated White Balance "
                 "Coefficients...\n\n" );
        
        _wbv = _idt->getWB();
        
        return 1;
    }
    
    return 0;
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
//		N/A              : pixel values modified by mutiplying white
//                         balance coefficients

void AcesRender::applyWB ( float * pixels, int bits, uint32_t total ) const
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

void AcesRender::applyCAT ( float * pixels, int channel, uint32_t total ) const
{
    assert(pixels);
    
    if ( channel != 3 && channel != 4 ) {
        fprintf ( stderr, "\nError: Currenly support 3 channels "
                 "and 4 channels. \n" );
        exit (1);
    }
    
    // will use calCAT() inside rawtoaces
    vector < double > d50V (d50, d50 + 3);
    vector < double > d60V (d60, d60 + 3);
    
    pixels = mulVectorArray ( pixels,
                              total,
                              channel,
                              _idt->calCAT(d50V, d60V) );
}


//	=====================================================================
//  Convert DNG RAW to aces file
//
//	inputs:
//      vector < float > : camera to display matrix
//
//	outputs:
//		float * : an array of converted aces values

float * AcesRender::renderDNG ( vector < float > cameraToDisplayMtx ) const
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

float * AcesRender::renderNonDNG () const
{
    assert(_image);

    ushort * pixels = (ushort *) _image->data;
    uint32_t total = _image->width * _image->height * _image->colors;
    float * aces = new (std::nothrow) float[total];
    
    FORI(total) aces[i] = static_cast <float> (pixels[i]);
    
    if(_opts.use_mat == 2) applyCAT(aces, _image->colors, total);
        
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
    
    FORI(total) aces[i] = static_cast <float> (pixels[i]);
    
    printf ( "\nApplying IDT Matrix ...\n\n" );
    
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

void AcesRender::acesWrite ( const char * name, float *  aces ) const
{
    assert(_image);

    uint16_t width     = _image->width;
    uint16_t height    = _image->height;
    uint8_t  channels  = _image->colors;
    uint8_t  bits      = _image->bits;
    
    halfBytes *halfIn = new (std::nothrow) halfBytes[channels * width * height];
    
    FORI ( channels * width * height ){
        if ( bits == 8 )
            aces[i] = (double) aces[i] * INV_255 * (_opts.scale);
        else if ( bits == 16 )
            aces[i] = (double) aces[i] * INV_65535 * (_opts.scale);
        
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

