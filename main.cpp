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
    
    LibRaw RawProcessor;
    vector < vector < double > > idtm( 3, vector < double > ( 3, 1.0 ) );
    vector < double > wbv( 3, 1.0 );

#ifndef WIN32
    void *iobuffer=0;
#endif

#ifdef OUT
#undef OUT
#endif

#define OUT RawProcessor.imgdata.params

#ifndef WIN32
  putenv ((char*)"TZ=UTC");
#else
  _putenv ((char*)"TZ=UTC");
#endif
    
#define P1 RawProcessor.imgdata.idata
#define S RawProcessor.imgdata.sizes
#define C RawProcessor.imgdata.color
#define R RawProcessor.imgdata.rawdata
#define T RawProcessor.imgdata.thumbnail
#define P2 RawProcessor.imgdata.other

  int arg = checkConditions(argc, argv, OUT, opts);

  if ( opts.use_camera_path ) {
      string cameraSenPathS( opts.cameraSenPath );
      if ( cameraSenPathS.find("_380_780") == std::string::npos ) {
          fprintf( stderr,"Cannot locate camera sensitivity data in the file.\n" );
          exit(1);
      }
  }
    
  if ( opts.verbosity > 2 )
      RawProcessor.set_progress_handler( my_progress_callback,
                                         (void * )"Sample data passed" );
#ifdef LIBRAW_USE_OPENMP
  if( opts.verbosity )
      printf ( "Using %d threads\n", omp_get_max_threads() );
#endif
    
    for ( ; arg < argc; arg++ )
    {
        char outfn[1024];
        if( opts.verbosity )
            printf( "Processing file %s\n",argv[arg] );
        
        timerstart_timeval();
            
#ifndef WIN32
            if ( opts.use_mmap )
            {
                int file = open( argv[arg],O_RDONLY );
                
                if( file<0 )
                {
                    fprintf( stderr, "Error: Cannot open %s: %s\n",
                             argv[arg], strerror(errno) );
                    continue;
                }
                
                if( fstat( file,&st ) )
                {
                    fprintf( stderr, "Error: Cannot stat %s: %s\n",
                             argv[arg], strerror(errno) );
                    close( file );
                    continue;
                }
                
                int pgsz = getpagesize();
                opts.msize = ( ( st.st_size+pgsz-1 ) / pgsz ) * pgsz;
                iobuffer = mmap( NULL, opts.msize, PROT_READ, MAP_PRIVATE, file, 0 );
                if( !iobuffer )
                {
                    fprintf ( stderr, "Error: Cannot mmap %s: %s\n",
                              argv[arg], strerror(errno) );
                    close( file );
                    continue;
                }
                
                close( file );
                if( (opts.ret = RawProcessor.open_buffer( iobuffer,st.st_size ) != LIBRAW_SUCCESS ) )
                {
                    fprintf ( stderr, "Error: Cannot open_buffer %s: %s\n",
                              argv[arg],
                              libraw_strerror(opts.ret) );
                    continue; // no recycle b/c open file will recycle itself
                }

            }
            else
#endif
            {
               if( opts.use_bigfile )
                   opts.ret = RawProcessor.open_file( argv[arg],1 );
               else
                   opts.ret = RawProcessor.open_file( argv[arg] );
                        
               if( opts.ret  != LIBRAW_SUCCESS)
               {
                   fprintf( stderr, "Error: Cannot open %s: %s\n",
                                    argv[arg], libraw_strerror(opts.ret) );
                   continue;
               }
            }

            if ( opts.use_timing )
                timerprint( "LibRaw::open_file()", argv[arg] );

            timerstart_timeval();
            if (( opts.ret = RawProcessor.unpack() ) != LIBRAW_SUCCESS )
            {
                fprintf( stderr, "Error: Cannot unpack %s: %s\n",
                                  argv[arg], libraw_strerror(opts.ret) );
                continue;
            }
            
            if ( opts.use_timing )
                timerprint( "LibRaw::unpack()", argv[arg] );
        
            if ( opts.use_illum ) {
                if( opts.use_wb || opts.use_mat)
                    fprintf( stderr, "Warning: \"--wb-method\" and \"--mat-method\" "
                                     "will be forced to change to 0 to support "
                                     "\"--adopt-white\" feature \n" );
                opts.use_wb = 0;
                opts.use_mat = 0;
                
                opts.illumType = lowerCase ( opts.illumType );
            }
        
        // --mat-method condition. Default of libraw is to use adobe_coeffs
            if ( !opts.use_mat ) {
                if ( opts.use_wb ) {
                    fprintf( stderr, "Error: --wb-method should be 0 "
                                     "in this case. Stopping now\n" );
                    exit(1);
                }
                OUT.use_camera_matrix = 0;
                
                bool gotIDT = prepareIDT( opts.cameraSenPath,
                                          opts.illumType,
                                          P1,
                                          C,
                                          idtm,
                                          wbv );
                    
                if ( !gotIDT && C.profile )
                    opts.use_wb = 1;
                else if ( !gotIDT && !C.profile )
                    opts.use_wb = 2;
                else if ( gotIDT ) {
                    OUT.output_color = 0;
                    opts.use_Mul = 1;
                    
                    if ( OUT.half_size == 1 )
                        OUT.four_color_rgb = 0;
                    
                    FORI(3) OUT.user_mul[i] = wbv[i];
                }
            }
            else if ( opts.use_mat == 1 && !C.profile ) {
                fprintf( stderr, "Warning: Cannot find color profile from the RAW, "
                                 "will use the default process from libraw\n" );
                OUT.use_camera_matrix = 1;
            }
            else if ( opts.use_mat == 1 && C.profile ) {
                OUT.use_camera_matrix = 3;
            }
            else if ( opts.use_mat > 0 && !opts.use_wb ) {
                fprintf( stderr, "Error: --wb-method should not be 0 when "
                                 "--mat-method is not set to 0. Stopping now\n" );
                exit(1);
            }
        
            OUT.output_color      = 5;
            OUT.highlight         = 0;
            OUT.gamm[0]           = 1.0;
            OUT.gamm[1]           = 1.0;
            OUT.no_auto_bright    = 1;

            // --wb-method condition
            if ( !opts.use_wb ) {
                if ( opts.use_mat ) {
                    fprintf( stderr, "Error: --mat-method should be 0 in "
                                     "this case. Stopping now\n" );
                    exit(1);
                }
                OUT.use_camera_wb = 0;
                OUT.use_auto_wb = 0;
            }
            else if ( opts.use_wb == 1 && C.profile ) {
                OUT.use_camera_wb = 1;
                OUT.use_auto_wb = 0;
            }
            else if ( opts.use_wb == 2 ||
                     (opts.use_wb == 1 && !C.profile) ) {
                OUT.use_camera_wb = 0;
                OUT.use_auto_wb = 1;
            }
        
            // For --wb-method 4
            if ( opts.use_Mul ){
                OUT.use_camera_wb = 0;
                OUT.use_auto_wb = 0;
                
                double sc = dmax;
                FORI( P1.colors ){
                    if ( OUT.user_mul[i] <= sc )
                        sc = OUT.user_mul[i];
                }
                
                if ( sc != 1.0 ) {
                    fprintf ( stderr, "Warning: The smallest channel multiplier "
                                      "should be 1.0.\n" );
                }
            }
        
            timerstart_timeval();
            if ( LIBRAW_SUCCESS != ( opts.ret = RawProcessor.dcraw_process() ) )
                {
                    fprintf ( stderr,"Error: Cannot do postpocessing on %s: %s\n",
                              argv[arg],libraw_strerror(opts.ret) );
                    if ( LIBRAW_FATAL_ERROR(opts.ret) )
                        continue; 
                }
        
            if ( opts.use_timing )
                timerprint ( "LibRaw::dcraw_process()", argv[arg] );
            
//            if (( cp = strrchr ( argv[arg], '.' ))) *cp = 0;
            snprintf( outfn, sizeof(outfn),
                      "%s%s",
                      argv[arg], "_aces.exr" );
            
            if ( opts.verbosity >= 2 ) // verbosity set by repeat -v switches
                printf ("Converting to aces RGB\n" );
            else if ( opts.verbosity )
                printf ("Writing file %s\n", outfn );
            
            if ( !P1.dng_version ) {
                libraw_processed_image_t *post_image = RawProcessor.dcraw_make_mem_image(&opts.ret);
                if ( opts.use_timing )
                    timerprint("LibRaw::dcraw_make_mem_image()",argv[arg]);
                
                float * aces = 0;
                if ( !OUT.output_color )
                    aces = prepareAcesData_NonDNG_IDT( post_image, idtm, wbv );
                else
                    aces = prepareAcesData_NonDNG( post_image );
                
                aces_write( outfn,
                            post_image->width,
                            post_image->height,
                            post_image->colors,
                            post_image->bits,
                            aces,
                            opts.scale );
            }
//            else {
////              DNG files always use the embedded color profile
        
//                OUT.use_camera_wb = 0;
//                OUT.use_auto_wb = 0;
//                OUT.use_camera_matrix = 1;
//
//                libraw_processed_image_t *post_image = RawProcessor.dcraw_make_mem_image(&opts.ret);
//                if(opts.use_timing)
//                    timerprint("LibRaw::dcraw_make_mem_image()",argv[arg]);
//                
//                float * aces = prepareAcesData_DNG(R, post_image);
//                aces_write (outfn,
//                            post_image->width,
//                            post_image->height,
//                            post_image->colors,
//                            post_image->bits,
//                            aces,
//                            scale );
//            }
            
#ifndef WIN32
            if ( opts.use_mmap && iobuffer )
            {
                munmap ( iobuffer, opts.msize );
                iobuffer = 0;
            }
#endif
            RawProcessor.recycle();
        }
    return 0;
}
