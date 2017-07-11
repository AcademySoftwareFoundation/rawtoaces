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

#include "src/usage.h"

int main(int argc, char *argv[])
{
    if ( argc == 1 ) usage( argv[0] );
    
    option opts;
    struct stat st;
    
    LibRawAces RawProcessor;
    AcesRender & Render = AcesRender::getInstance();
    
#ifndef WIN32
    void *iobuffer=0;
#endif

#ifdef OUT
#undef OUT
#endif

#define OUT RawProcessor.imgdata.params
#define P1 RawProcessor.imgdata.idata
#define C RawProcessor.imgdata.color

#ifndef WIN32
  putenv ((char*)"TZ=UTC");
#else
  _putenv ((char*)"TZ=UTC");
#endif

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
    
    const char ** cl = RawProcessor.cameraList();
    while (*(cl+1) != NULL)
        opts.cameraListLR.push_back(string(*cl++));

// Fetch conditions and conduct some pre-processing
    int arg = configureSetting (argc, argv, opts, OUT);
    Render.setOptions(opts);
    
// gather a list of illuminants supported - for testing purpose
    if (opts.get_illums) {
        Render.gatherSupportedIllums();
        vector < string > ilist = Render.getSupportedIllums();
        printf("\nThe following illuminants are available:\n\n");
        FORI(ilist.size()) printf("%s \n", ilist[i].c_str());
    }
    
// gather a list of cameras supported - for testing purpose
    if (opts.get_cameras) {
        Render.gatherSupportedCameras();
        vector < string > clist = Render.getSupportedCameras();
        printf("\nThe following cameras' sensitivity data is available:\n\n");
        FORI(clist.size()) printf("%s \n", clist[i].c_str());
    }
    
   if ( opts.verbosity > 2 )
       RawProcessor.set_progress_handler ( my_progress_callback,
                                          ( void * )"Sample data passed" );

	// Start rawtoaces
	if ( opts.verbosity )
		printf( "\nStarting rawtoaces ...\n");
    
#ifdef LIBRAW_USE_OPENMP
   if( opts.verbosity )
       printf ( "Using %d threads\n", omp_get_max_threads() );
#endif
    
    // Gather all the raw images from arg list
    vector < string > RAWs;
    for ( ; arg < argc; arg++ ) {
        if( stat( argv[arg], &st) != 0 ) {
            fprintf ( stderr, "Error: The directory or file may not exist - \"%s\"...",
                              argv[arg]);
            continue;
        }
        
        if ( st.st_mode & S_IFDIR ) {
            vector <string> files = openDir ( static_cast< string >( argv[arg] ) );
            for ( vector <string>::iterator file = files.begin( ); file != files.end( ); ++file ) {
                if ( stat( file->c_str(), &st) == 0 &&
                    ( st.st_mode & S_IFREG ) )
                    RAWs.push_back ( *file );
            }
        }
        else if ( st.st_mode & S_IFREG ) {
            RAWs.push_back ( argv[arg] );
        }
    }
    
// Load in illuminant data now
    int read = 0;
    if (!opts.illumType)
        read = Render.fetchIlluminant( );
    else
        read = Render.fetchIlluminant( opts.illumType );
    
    if( !read ) {
        fprintf( stderr, "\nError: No matching light source. "
                         "Please find available options by "
                         "\"rawtoaces --valid-illum\".\n");
        exit (-1);
    }
    
// Process RAW files ...
    FORI ( RAWs.size() )
    {
        const char * raw = (RAWs[i]).c_str();
        
        if ( opts.verbosity )
        	printf ( "Processing %s ...\n", raw );
        
        char outfn[1024];
        Render.setOptions(opts);
        
        timerstart_timeval();
            
#ifndef WIN32
            if ( opts.use_mmap )
            {
                int file = open ( raw, O_RDONLY );
                
                if( file < 0 )
                {
                    fprintf ( stderr, "\nError: Cannot open %s: %s\n\n",
                              raw, strerror(errno) );
                    continue;
                }
                
                if( fstat ( file, &st ) )
                {
                    fprintf ( stderr, "\nError: Cannot stat %s: %s\n\n",
                              raw, strerror(errno) );
                    close( file );

                    continue;
                }
                
                int pgsz = getpagesize();
                opts.msize = (( st.st_size+pgsz-1 ) / pgsz ) * pgsz;
                iobuffer = mmap ( NULL, size_t(opts.msize), PROT_READ, MAP_PRIVATE, file, 0 );
                if( !iobuffer )
                {
                    fprintf ( stderr, "\nError: Cannot mmap %s: %s\n\n",
                                      raw, strerror(errno) );
                    close( file );

                    continue;
                }
                
                close( file );
                if (( opts.ret = RawProcessor.open_buffer( iobuffer,st.st_size ) != LIBRAW_SUCCESS ))
                {
                    fprintf ( stderr, "\nError: Cannot open_buffer %s: %s\n\n",
                                      raw,
                                      libraw_strerror(opts.ret) );
                    continue;
                }
            }
            else
#endif
            {
               if ( opts.use_bigfile )
                   opts.ret = RawProcessor.open_file ( raw, 1 );
               else
                   opts.ret = RawProcessor.open_file ( raw );

                
               if ( opts.ret  != LIBRAW_SUCCESS )
               {
                   fprintf ( stderr, "\nError: Cannot open %s: %s\n\n",
                                     raw, libraw_strerror (opts.ret) );
                   
                   continue;
               }
            }

            if ( opts.use_timing )
                timerprint ( "LibRaw::open_file()", raw );

            timerstart_timeval();
            if (( opts.ret = RawProcessor.unpack() ) != LIBRAW_SUCCESS )
            {
                fprintf ( stderr, "\nError: Cannot unpack %s: %s\n\n",
                                  raw, libraw_strerror (opts.ret) );
                continue;
            }
        
            if ( opts.use_timing )
                timerprint ( "LibRaw::unpack()", raw );
        
            Render.setOptions(opts);
        
        	if (opts.verbosity > 1)
            	printf ( "The camera has been identified as a %s %s ...\n", P1.make, P1.model );
            
        
//          Set parameters for White Balance method is 
            switch ( opts.wb_method ) {
                // 0
                case wbMethod0 : {
                    opts.use_mul = 1;
                    vector < double > mulV (C.cam_mul, C.cam_mul+3);
                    
                    FORI(3) OUT.user_mul[i] = static_cast<float>(mulV[i]);
                    
                    if (opts.verbosity > 1 ) {
                    	printf ( "White Balance method is 0 - ");
                    	printf ( "Using white balance factors from "
                    			 "file metadata ...\n" );
                    }
                    break;
                }
                // 1
                case wbMethod1 : {
                    int gotWB = Render.prepareWB ( P1 );
                    
                    if ( gotWB ) {
                        opts.use_mul = 1;
                        vector < double > wbv = Render.getWB();
                        FORI(3) OUT.user_mul[i] = static_cast<float>(wbv[i]);
                    }
                    
                	if (opts.verbosity > 1) {
                    	printf ( "White Balance calculation method is 1 - ");
                    	printf ( "Using white balance factors calculated from "
                    			 "spec sens and user specified illuminant ...\n" );
                    }
                    
                    break;
                }
                // 2
                case wbMethod2 : {
                    OUT.use_auto_wb = 1;
                    
                    if (opts.verbosity > 1) {
                    	printf ( "White Balance calculation method is 2 - ");
                    	printf ( "Using white balance factors calculate by "
                             	"averaging the entire image ...\n" );
                    }
                    
                    break;
                }
                // 3
                case wbMethod3 : {
                	if (opts.verbosity > 1) {
                		printf ( "White Balance calculation method is 3 - ");
                    	printf ( "Using white balance factors calculated by "
                             "averaging a grey box ...\n" );
                    }
                    
                    break;
                }
                // 4
                case wbMethod4 : {
                    opts.use_mul = 1;
                    double sc = dmax;
                    
                    FORI(P1.colors){
                        if ( OUT.user_mul[i] <= sc )
                            sc = OUT.user_mul[i];
                    }
                    
                    if ( sc != 1.0 ) {
                        fprintf ( stderr, "\nWarning: The smallest channel  "
                                          "multiplier should be 1.0.\n\n" );
                    }
                    
                    if (opts.verbosity > 1) {
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
        
//          Set parameters for --mat-method
            switch ( opts.mat_method ) {
                // 0
                case matMethod0 : {
                    OUT.output_color = 0;
                    
                    if (opts.verbosity > 1) {
                    	printf ( "IDT matrix calculation method is 0 - ");
                    	printf ( "Calculating IDT matrix using camera spec sens ...\n" );
                    }
                    
                    break;
                }
                // 1
                case matMethod1 :
                    OUT.use_camera_matrix = 0;
                    
                    if (opts.verbosity > 1) {
                    	printf ( "IDT matrix calculation method is 1 - ");
                    	printf ( "Calculating IDT matrix using file metadata ...\n" );
                    }
                    
                    break;
                // 2
                case matMethod2 :
                    OUT.use_camera_matrix = 3;
                    
                    if (opts.verbosity > 1) {
                    	printf ( "IDT matrix calculation method is 2 - ");
                    	printf ( "Calculating IDT matrix using adobe coeffs included in libraw ...\n" );
                    }
                    
                    break;
                default:
                    fprintf ( stderr, "IDT matrix calculation method is must be 0, 1, 2 \n" );
                    exit(-1);
                    break;
            }
        
            Render.setOptions(opts);

//          Set four_color_rgb to 0 when half_size is set to 1
            if ( OUT.half_size == 1 ) OUT.four_color_rgb = 0;

//          Start the dcraw_process()
            timerstart_timeval();
            if ( LIBRAW_SUCCESS != ( opts.ret = RawProcessor.dcraw_process() ) ) {
                
                fprintf ( stderr, "Error: Cannot do postpocessing on %s: %s\n\n",
                          raw,libraw_strerror(opts.ret) );
                if ( LIBRAW_FATAL_ERROR( opts.ret ) )
                    continue;
            }
        
//          Use the final wb factors to otbain IDT matrix
            // 0
            if ( opts.mat_method == matMethod0 )
                Render.prepareIDT ( P1, C.pre_mul );

            if ( opts.use_timing )
                timerprint ( "LibRaw::dcraw_process()", raw );
        
            Render.setOptions(opts);
        
            char * cp;
            if (( cp = strrchr ( (char *)((RAWs[i]).c_str()), '.' ))) *cp = 0;

            snprintf( outfn,
                      sizeof(outfn),
                      "%s%s",
                      (char *)((RAWs[i]).c_str()),
                      "_aces.exr" );
        
            if ( !P1.dng_version ) {
                libraw_processed_image_t * post_image = RawProcessor.dcraw_make_mem_image ( &opts.ret );
                
                Render.setPixels (post_image);
                if ( opts.use_timing )
                    timerprint( "LibRaw::dcraw_make_mem_image()", raw);
                
                float * aces = 0;
                if ( !OUT.output_color )
                    aces = Render.renderNonDNG_IDT();
                else
                    aces = Render.renderNonDNG();
                
                if ( opts.verbosity > 1 ) {
                    if (opts.mat_method) {
                        vector <vector < double > > camXYZ(3, vector< double >(3, 1.0));
                        FORIJ(3,3) camXYZ[i][j] = C.cam_xyz[i][j];
                        vector <vector < double > > camcat = mulVector(camXYZ, Render.getCATMatrix());
                        
                        printf("The IDT matrix is ...\n");
                        FORI(3) printf("   %f, %f, %f\n", camcat[i][0], camcat[i][1], camcat[i][2]);
                    }
                    
                    // printing white balance coefficients
                    printf ("The final white balance coefficients are ...\n");
                    printf ("   %f   %f   %f\n", C.pre_mul[0], C.pre_mul[1], C.pre_mul[2]);
                    
            		printf ( "Writing ACES file to %s ...\n", outfn );
                }
                
                if ( opts.highlight > 0 ) {
                    float ratio = ( *(std::max_element(C.pre_mul, C.pre_mul+3)) /
                                    *(std::min_element(C.pre_mul, C.pre_mul+3)) );
                    Render.acesWrite ( outfn, aces, ratio );
                }
                else
                    Render.acesWrite ( outfn, aces );
            }
        
#ifndef WIN32
            if ( opts.use_mmap && iobuffer )
            {
                munmap ( iobuffer, size_t(opts.msize) );
                iobuffer = 0;
            }
#endif
//            RawProcessor.recycle();
    }

    RawProcessor.recycle();
    
    if (opts.verbosity)
    	printf ("Finished\n\n");

    return 0;
}
