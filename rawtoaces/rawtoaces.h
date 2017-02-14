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

#ifndef NO_ACESCONTAINER
#include <aces/aces_Writer.h>
#endif

#ifndef NO_OPENEXR
#include <OpenEXR/half.h>
#endif

#include <libraw/libraw.h>
#include "../lib/rta.h"

using namespace rta;

bool readCameraSenPath( const char * cameraSenPath,
                        libraw_iparams_t P,
                        Idt * idt )
{
    bool readC = 0;
    
    if ( cameraSenPath )  {
        if ( stat( static_cast<const char *>( cameraSenPath ), &st ) )  {
            fprintf ( stderr,"The camera sensitivity file does not seem to exist.\n" );
            exit (  EXIT_FAILURE );
        }
        
        readC = idt->loadCameraSpst( cameraSenPath,
                                     static_cast<const char *> ( P.make ),
                                     static_cast<const char *> ( P.model ));
    }
    else  {
        if ( !stat(  FILEPATH, &st ) )  {
            vector<string> cFiles = openDir ( static_cast<string>(  FILEPATH )
                                              +"/camera" );
            
            for(  vector<string>::iterator file = cFiles.begin( ); file != cFiles.end( ); ++file ) {
                string fn( *file );
                readC = idt->loadCameraSpst( fn,
                                             static_cast<const char *>( P.make ),
                                             static_cast<const char *>( P.model ) );
                if ( readC ) return 1;
            }
        }
    }
    
    return readC;
}


bool readIlluminate( const char * illumType,
                     map< string, vector<double> >& illuCM,
                     Idt * idt )
{
    bool readI = 0;
    
    if( !stat( FILEPATH, &st ) ) {
        vector<string> iFiles = openDir( static_cast<string>( FILEPATH )
                                         +"illuminate" );
        
        for ( vector<string>::iterator file = iFiles.begin(); file != iFiles.end(); ++file ) {
            string fn( *file );
            string strType(illumType);
            
            if ( strType.compare("unknown") != 0 ) {
                if( fn.find( illumType ) == std::string::npos )  {
                    continue;
                }
                
                readI = idt->loadIlluminate( fn, static_cast<const char *>( illumType ) );
                if ( readI )
                {
                    illuCM[static_cast<string>( *file )] = idt->calCM();
                    return 1;
                }
            }
            else {
                readI = idt->loadIlluminate( fn, static_cast<const char *>( illumType ) );
                if ( readI )
                    illuCM[static_cast<string>( *file )] = idt->calCM();
            }
        }
    }
    
    return readI;
}


bool prepareIDT ( const char * cameraSenPath,
                  const char * illumType,
                  libraw_iparams_t P,
                  libraw_colordata_t C,
                  vector < vector < double > > &idtm,
                  vector < double > &wbv )
{
    Idt * idt = new Idt(  );
    bool read = readCameraSenPath( cameraSenPath, P, idt );
    
    if (!read ) {
        fprintf( stderr,"No matching cameras found. "
                        "Will go with the default method. \n");
        
        if (illumType) {
            fprintf( stderr,"The Illuminate should be accompanied "
                            "by a matching camera sensitivity data.\n" );
        }
        
        return 0;
    }

    if (!illumType)
        illumType = "unknown";
    
    map< string, vector<double> > illuCM;
    read = readIlluminate( illumType, illuCM, idt );
    
    if( !read ) {
        fprintf( stderr,"No matching light source. "
                        "Will use the default settings.\n" );
    }
    else
    {
        idt->loadTrainingData ( static_cast<string>( FILEPATH )
                                +"/training/training_spectral" );
        idt->loadCMF ( static_cast<string>( FILEPATH )
                       +"/cmf/cmf_193" );
        
        vector<double> pre_mulV( 3, 1.0 );
        FORI(3) pre_mulV[i] = ( double )( C.pre_mul[i] );
        idt->chooseIlluminate( illuCM, pre_mulV );
        
        idt->calWB(illumType);
        
        if ( idt->calIDT() )  {
            idtm = idt->getIDT();
            wbv = idt->getWB();
            
            return 1;
        }
    }
    
    return 0;
}

void apply_WB ( float * pixels,
                uint8_t bits,
                uint32_t total,
                vector<double> wb )
{
    double min_wb = * min_element ( wb.begin(), wb.end() );
    double target = 1.0;
    
    if ( bits == 8 ) {
        target /= INV_255;
    }
    else if ( bits == 16 ){
        target /= INV_65535;
    }
    
    if ( !pixels ) {
        fprintf ( stderr, "The pixel code value may not exist. \n" );
        exit (EXIT_FAILURE);
    }
    else {
        for ( uint32_t i = 0; i < total; i+=3 ){
            pixels[i]   = clip (wb[0] * pixels[i] / min_wb, target);
            pixels[i+1] = clip (wb[1] * pixels[i+1] / min_wb, target);
            pixels[i+2] = clip (wb[2] * pixels[i+2] / min_wb, target);
        }
    }
}

void apply_IDT ( float * pixels,
                 uint8_t channel,
                 uint32_t total,
                 vector < vector < double > > idt)
{
    assert(pixels);
    
    if ( channel == 4 ){
        idt.resize(4);
        FORI(4) idt[i].resize(4);
        
        idt[0][3] = 0.0;
        idt[1][3] = 0.0;
        idt[2][3] = 0.0;
        idt[3][0] = 0.0;
        idt[3][1] = 0.0;
        idt[3][2] = 0.0;
        idt[3][3] = 1.0;
    }
    
    if ( channel != 3 && channel != 4 ) {
        fprintf (stderr, "Currenly support 3 channels and 4 channels. \n");
        exit (EXIT_FAILURE);
    }
    
    pixels = mulVectorArray(pixels, total, channel, idt);
}

float * convert_to_aces_NonDNG_IDT ( libraw_processed_image_t *image,
                                     vector < vector < double > > idt,
                                     vector <double> wb )
{
    uchar * pixels = image->data;
    uint32_t total = image->width * image->height * image->colors;
    float * aces = new (std::nothrow) float[total];
    
    FORI(total) aces[i] = static_cast<float>(pixels[i]);
    
    apply_WB ( aces, image->bits, total, wb );
    apply_IDT ( aces, image->colors, total, idt );
    
    return aces;
 }

float * convert_to_aces_NonDNG ( libraw_processed_image_t *image )
{
    uchar * pixel = image->data;
    uint32_t total = image->width*image->height*image->colors;
    float * aces = new (std::nothrow) float[total];
    
    for(uint32_t i = 0; i < total; i++ ){
        aces[i] = static_cast<float>(pixel[i]);
    }
    
    vector < vector< double> > XYZ_acesrgb(image->colors,
                                           vector< double >(image->colors));
    if(image->colors == 3) {
        FORI(3)
            FORJ(3)
                XYZ_acesrgb[i][j] = XYZ_acesrgb_3[i][j];
        
        return mulVectorArray(aces, total, 3, XYZ_acesrgb);
    }
    else if(image->colors == 4){
        FORI(4)
            FORJ(4)
                XYZ_acesrgb[i][j] = XYZ_acesrgb_4[i][j];
        
        return mulVectorArray(aces, total, 4, XYZ_acesrgb);
    }
    else {
        fprintf (stderr, "Currenly support 3 channels and 4 channels. \n");
        exit (EXIT_FAILURE);
    }
    
    return aces;
}

float * convert_to_aces_DNG ( libraw_processed_image_t *image,
                              valarray<float> cameraToDisplayMtx )
{
    uchar * pixels = image->data;
    uint32_t total = image->width * image->height * image->colors;
    vector < vector< double> > CAT( image->colors,
                                    vector< double >(image->colors));

    float * aces = new (std::nothrow) float[total];
    FORI(total){
        aces[i] = static_cast<float>(pixels[i]);
    }
    
    FORI(3)
        FORJ(3)
            CAT[i][j] = static_cast<double>(cameraToDisplayMtx[i*3+j]);
   
    if(image->colors == 3) {
        return mulVectorArray(aces, total, 3, CAT);
    }
    else if(image->colors == 4){
        CAT[0][3]=0.0;
        CAT[1][3]=0.0;
        CAT[2][3]=0.0;
        CAT[3][3]=1.0;
        CAT[3][0]=0.0;
        CAT[3][1]=0.0;
        CAT[3][2]=0.0;
        
        return mulVectorArray(aces, total, 4, CAT);
    }
    else {
        fprintf(stderr, "Currenly support 3 channels and 4 channels. \n");
        exit(EXIT_FAILURE);
    }

    return aces;
}

float * prepareAcesData_NonDNG ( libraw_processed_image_t *image )
{
    return convert_to_aces_NonDNG ( image );
}

float * prepareAcesData_NonDNG_IDT ( libraw_processed_image_t *image,
                                     vector < vector < double > > idtm,
                                     vector < double > wbv)
{
    return convert_to_aces_NonDNG_IDT( image, idtm, wbv );
}

//float * prepareAcesData_DNG(libraw_rawdata_t R,
//                            libraw_processed_image_t *image
//                            )
//{
//    float (*dng_cm1)[3] = R.color.dng_color[0].colormatrix;
//    float (*dng_cc1)[4] = R.color.dng_color[0].calibration;
//    float (*dng_cm2)[3] = R.color.dng_color[1].colormatrix;
//    float (*dng_cc2)[4] = R.color.dng_color[1].calibration;
//        
//    calibrateIllum[0] = R.color.dng_color[0].illuminant;
//    calibrateIllum[1] = R.color.dng_color[1].illuminant;
//    
////    cout << "calibrateIllum[0]: " << calibrateIllum[0] << endl;
////    cout << "calibrateIllum[1]: " << calibrateIllum[1] << endl;
//    
//    valarray<float> XYZToACESMtx(1.0f, 9);
//    
//    for(int i=0; i<3; i++) {
//        neutralRGBDNG[i] = 1.0f/(R.color.cam_mul)[i];
//        for (int j=0; j<3; j++){
//            xyz2rgbMatrix1DNG[i*3+j] = (dng_cm1)[i][j];
//            xyz2rgbMatrix2DNG[i*3+j] = (dng_cm2)[i][j];
//            cameraCalibration1DNG[i*3+j] = (dng_cc1)[i][j];
//            cameraCalibration1DNG[i*3+j] = (dng_cc2)[i][j];
//            XYZToACESMtx[i*3+j] = XYZ_acesrgb_3[i][j];
//        }
//    }
//    
//    valarray<float> deviceWhiteV(deviceWhite, 3);
//    getCameraXYZMtxAndWhitePoint(R.color.baseline_exposure);
//    valarray<float> outputRGBtoXYZMtx(matrixRGBtoXYZ(chromaticitiesACES));
////    valarray<float> XYZToDisplayMtx(invertMatrix(outputRGBtoXYZMtx));
//    valarray<float> outputXYZWhitePoint(multiplyMatrix(outputRGBtoXYZMtx, deviceWhiteV));
//    valarray<float> chadMtx(matrixChromaticAdaptation(cameraXYZWhitePoint, outputXYZWhitePoint));
//    
////    valarray<float> cameraToDisplayMtx(multiplyMatrix(multiplyMatrix(XYZToDisplayMtx, chadMtx), cameraToXYZMtx));
////    valarray<float> cameraToDisplayMtx(multiplyMatrix(multiplyMatrix(XYZToACESMtx, chadMtx), cameraToXYZMtx));
//    valarray<float> cameraToDisplayMtx(multiplyMatrix(XYZToACESMtx, chadMtx));
//    
//    valarray<float> outRGBWhite(multiplyMatrix(cameraToDisplayMtx, multiplyMatrix(invertMatrix(cameraToXYZMtx), cameraXYZWhitePoint)));
//    outRGBWhite	= outRGBWhite/outRGBWhite.max();
//    
//    valarray<float> absdif = abs(outRGBWhite-deviceWhiteV);
//    if (absdif.max() >= 0.0001 ) {
//        printf("WARNING: The neutrals should come out white balanced.\n");
//    }
//
//    assert(cameraToDisplayMtx.sum()!= 0);
//    
////    float * pixels = convert_to_aces_DNG(R, P, cameraToDisplayMtx);
//    float * pixels = convert_to_aces_DNG(image, cameraToDisplayMtx);
//
//    return pixels;
//}

void aces_write( const char * name,
                 uint16_t width,
                 uint16_t height,
                 uint8_t  channels,
                 uint8_t  bits,
                 float *  pixels,
                 float    scale = 1.0 )
{
    halfBytes *in = new (std::nothrow) halfBytes[channels * width * height];
//    if (bits == 8) {
//        cout << "8" << endl;
//    }
//    else if (bits == 16){
//        cout << "16" << endl;
//    }
    
    FORI(channels*width*height){
        if (bits == 8) {
            pixels[i] = (double)pixels[i] * INV_255 * scale;
        }
        else if (bits == 16){
            pixels[i] = (double)pixels[i] * INV_65535 * scale;
        }
        
        half tmpV(pixels[i]/1.0f);
        in[i] = tmpV.bits();
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
            //			writeParams.hi.channels.resize(6);
            //			writeParams.hi.channels[0].name = "B";
            //			writeParams.hi.channels[1].name = "G";
            //			writeParams.hi.channels[2].name = "R";
            //			writeParams.hi.channels[3].name = "left.B";
            //			writeParams.hi.channels[4].name = "left.G";
            //			writeParams.hi.channels[5].name = "left.R";
            //			break;
        case 8:
            throw std::invalid_argument("Stereo RGB support not yet implemented");
            //			writeParams.hi.channels.resize(8);
            //			writeParams.hi.channels[0].name = "A";
            //			writeParams.hi.channels[1].name = "B";
            //			writeParams.hi.channels[2].name = "G";
            //			writeParams.hi.channels[3].name = "R";
            //			writeParams.hi.channels[4].name = "left.A";
            //			writeParams.hi.channels[5].name = "left.B";
            //			writeParams.hi.channels[6].name = "left.G";
            //			writeParams.hi.channels[7].name = "left.R";
            //			break;
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
    
    FORI( height ){
        halfBytes *rgbData = in + width * channels * i;
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
