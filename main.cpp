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
    // print out help menu
    if ( argc==1 ) usage( argv[0] );
    
    LibRaw RawProcessor;
    int c,ret,arg;
    char opm,opt,*cp,*sp,*path,*cameraSenPath, *illumType;
    int use_bigfile = 0, use_timing = 0, use_camera_path = 0, use_illum = 0, use_Mul = 0, use_mat = -1, use_wb = -1;
    float scale = 1.0;
    
    vector < vector < double > > idtm( 3, vector < double > ( 3, 1.0 ) );
    vector < double > wbv( 3, 1.0 );

    
#ifndef WIN32
    int msize = 0,use_mmap=0;
    void *iobuffer=0;
#endif

#ifdef OUT
#undef OUT
#endif

#define OUT RawProcessor.imgdata.params
    
    create_key();
    argv[argc] = (char*)"";
    
    for ( arg = 1; arg < argc; )
    {
          string key(argv[arg]);
            
          if ( key[0] != '-' ) {
              break;
          }
          
          arg++;
          opt = keys[key];
          if (( cp = strchr ( sp = (char*)"RpMgcnbksStqmHBC", opt )) != 0 ) {
            for (int i=0; i < "111411111142"[cp-sp]-'0'; i++) {
                if (!isdigit(argv[arg+i][0]))
                {
                    if ( opt == 'R' || opt == 'p')
                    {
                        fprintf (stderr,"Non-numeric argument to \"%s\"\n", key.c_str());
                        return 1;
                    }
                    fprintf (stderr,"Non-numeric argument to \"-%c\"\n", opt);
                    return 1;
                }
            }
          }
          else if ((cp = strchr ( sp = (char*)"T", opt )) != 0) {
            for (int i=0; i < "111411111142"[cp-sp]-'0'; i++) {
                if ( !isalnum(argv[arg+i][0] ) )
                {
                    fprintf (stderr,"Non-numeric and/or Non-compatible"
                                    " argument to \"-%c\"\n", opt);
                    return 1;
                }
             }
          }
        
         // The camera information can be found from libraw_types.h
          switch ( opt )
          {
              case 'I':  usage( argv[0] );  break;
              case 'V':  printf ( "%s\n", VERSION );  break;
              case 'v':  verbosity++;  break;
              case 'G':  OUT.green_matching = 1; break;
              case 'c':  OUT.adjust_maximum_thr   = (float)atof(argv[arg++]);  break;
              case 'n':  OUT.threshold   = (float)atof(argv[arg++]);  break;
              case 'b':  OUT.bright      = (float)atof(argv[arg++]);  break;
              case 'P':  OUT.bad_pixels  = argv[arg++];        break;
              case 'K':  OUT.dark_frame  = argv[arg++];        break;
              case 'C':
                  OUT.aber[0] = 1.0 / atof(argv[arg++]);
                  OUT.aber[2] = 1.0 / atof(argv[arg++]);
                  break;
              case 'k':  OUT.user_black  = atoi(argv[arg++]);  break;
              case 'S':  OUT.user_sat    = atoi(argv[arg++]);  break;
              case 't':  OUT.user_flip   = atoi(argv[arg++]);  break;
              case 'q':  OUT.user_qual   = atoi(argv[arg++]);  break;
              case 'm':  OUT.med_passes  = atoi(argv[arg++]);  break;
              case 'H':  OUT.highlight   = atoi(argv[arg++]);  break;
              case 'h':  OUT.half_size         = 1;
                  // no break:  "-h" implies "-f"
              case 'f':  
                  OUT.four_color_rgb    = 1;  
                  break;
              case 'B':  FORI(4) OUT.cropbox[i]  = atoi(argv[arg++]); break;
              case 'j':  OUT.use_fuji_rotate   = 0;  break;
              case 'W':  OUT.no_auto_bright    = 1;  break;
              case 'F':  use_bigfile           = 1;  break;
              case 'd':  use_timing            = 1;  break;
              case 'R':
                  use_wb = atoi(argv[arg++]);
                  if ( use_wb == 3 ) {
                      FORI(4) {
                          if (!isdigit(argv[arg][0]))
                          {
                              fprintf (stderr,"Non-numeric argument to \"%s\" 3\n", key.c_str());
                              return 1;
                          }
                          OUT.greybox[i] = (float)atof(argv[arg++]);
                      }
                  }
                  else if ( use_wb == 4 ) {
                      use_Mul = 1;
                      FORI(4) {
                          if (!isdigit(argv[arg][0]))
                          {
                              fprintf (stderr,"Non-numeric argument to \"%s\" 4\n", key.c_str());
                              return 1;
                          }
                          OUT.user_mul[i] = (float)atof(argv[arg++]);
                      }
                  }
                  break;
              case 'p':
                  use_mat = atoi(argv[arg++]);
                  break;
              case 'Q':
                  use_camera_path = 1;
                  use_wb = 0;
                  cameraSenPath = (char *)(argv[arg++]);
                  break;
              case 'T':
                  use_illum = 1;
                  illumType = (char *)(argv[arg++]);
                  break;
              case 'M':  scale = atof(argv[arg++]); break;
#ifndef WIN32
              case 'E':  use_mmap              = 1;  break;
#endif
              default:
                  fprintf (stderr,"Unknown option \"-%s\".\n", key.c_str());
                  return 1;
              }
       }
    
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

  if( use_camera_path ) {
      string cameraSenPathS( cameraSenPath );
      if ( cameraSenPathS.find("_380_780") == std::string::npos ) {
          fprintf( stderr,"Cannot locate camera sensitivity data in the file.\n" );
          exit(1);
      }
  }
    
  if( verbosity>1 )
      RawProcessor.set_progress_handler( my_progress_callback,
                                         ( void * )"Sample data passed" );
#ifdef LIBRAW_USE_OPENMP
  if( verbosity )
          printf ( "Using %d threads\n", omp_get_max_threads() );
#endif
    
    for ( ; arg < argc; arg++ )
    {
        char outfn[1024];
        if( verbosity )
            printf( "Processing file %s\n",argv[arg] );
        
        timerstart_timeval();
            
#ifndef WIN32
            if(use_mmap)
            {
                int file = open( argv[arg],O_RDONLY );
                
                if( file<0 )
                {
                    fprintf( stderr,"Error: Cannot open %s: %s\n", argv[arg], strerror(errno) );
                    continue;
                }
                
                if( fstat( file,&st ) )
                {
                    fprintf( stderr,"Error: Cannot stat %s: %s\n", argv[arg], strerror(errno) );
                    close( file );
                    continue;
                }
                
                int pgsz = getpagesize();
                msize = ( ( st.st_size+pgsz-1 ) / pgsz ) * pgsz;
                iobuffer = mmap( NULL, msize, PROT_READ, MAP_PRIVATE, file, 0 );
                if( !iobuffer )
                {
                    fprintf ( stderr, "Error: Cannot mmap %s: %s\n", argv[arg], strerror(errno) );
                    close( file );
                    continue;
                }
                
                close( file );
                if( (ret = RawProcessor.open_buffer( iobuffer,st.st_size ) != LIBRAW_SUCCESS ) )
                {
                    fprintf ( stderr, "Error: Cannot open_buffer %s: %s\n",
                              argv[arg],
                              libraw_strerror(ret) );
                    continue; // no recycle b/c open file will recycle itself
                }

            }
            else
#endif
            {
               if( use_bigfile )
                   ret = RawProcessor.open_file( argv[arg],1 );
               else
                   ret = RawProcessor.open_file( argv[arg] );
                        
               if( ret  != LIBRAW_SUCCESS)
               {
                   fprintf( stderr,"Error: Cannot open %s: %s\n", argv[arg], libraw_strerror(ret) );
                   continue;
               }
            }

            if ( use_timing )
                timerprint( "LibRaw::open_file()", argv[arg] );

            timerstart_timeval();
            if (( ret = RawProcessor.unpack() ) != LIBRAW_SUCCESS )
            {
                fprintf( stderr, "Error: Cannot unpack %s: %s\n", argv[arg], libraw_strerror(ret) );
                continue;
            }
            
            if ( use_timing )
                timerprint( "LibRaw::unpack()", argv[arg] );
        
            if ( use_illum && illumType ) {
                if( use_wb || use_mat)
                    fprintf( stderr,"Warning: --wb-method and --mat-method will be "
                                    "changed to 0 to support --adopt-white \n" );
                use_wb = 0;
                use_mat = 0;
                
                illumType = lowerCase ( illumType );
            }
        
        // --mat-method condition. Default of libraw is to use adobe_coeffs
            if ( use_mat == 0 ) {
                if ( use_wb ) {
                    fprintf( stderr,"Error: --wb-method should be 0 in this case. Stopping now\n" );
                    exit(1);
                }
                OUT.use_camera_matrix = 0;
                bool gotIDT = prepareIDT( cameraSenPath,
                                          illumType,
                                          P1,
                                          C,
                                          idtm,
                                          wbv );
                    
                if ( !gotIDT && C.profile )
                    use_wb = 1;
                else if ( !gotIDT && !C.profile )
                    use_wb = 2;
                else if ( gotIDT ) {
                    OUT.output_color = 0;
                    use_Mul = 1;
                    
                    if ( OUT.half_size == 1 )
                        OUT.four_color_rgb = 0;
                    
                    FORI(3) OUT.user_mul[i] = wbv[i];
                }
            }
            else if ( use_mat == 1 && C.profile )
                OUT.use_camera_matrix = 3;
            else if ( use_mat == 1 && !C.profile ) {
                fprintf( stderr,"Warning: Cannot find color profile from the RAW, "
                                "will use the default process from libraw\n" );
                OUT.use_camera_matrix = 1;
            }
            else if ( use_mat > 0 && !use_wb ) {
                fprintf( stderr,"Error: --wb-method should not be 0 in this case. Stopping now\n" );
                exit(1);
            }
        
            OUT.output_color      = 5;
            OUT.highlight         = 0;
            OUT.gamm[0]           = 1.0;
            OUT.gamm[1]           = 1.0;
            OUT.no_auto_bright    = 1;

            // --wb-method condition
            if ( use_wb == 0 ) {
                if ( use_mat ) {
                    fprintf( stderr,"Error: --mat-method should be 0 in this case. Stopping now\n" );
                    exit(1);
                }
                OUT.use_camera_wb = 0;
                OUT.use_auto_wb = 0;
            }
            else if ( use_wb == 1 && C.profile ) {
                OUT.use_camera_wb = 1;
                OUT.use_auto_wb = 0;
            }
            else if ( use_wb == 2 ||
                     (use_wb == 1 && !C.profile) ) {
                OUT.use_camera_wb = 0;
                OUT.use_auto_wb = 1;
            }
        
            // For --wb-method 4
            if ( use_Mul && !isnan ( OUT.user_mul[0] ) ){
                OUT.use_camera_wb = 0;
                OUT.use_auto_wb = 0;
                
                double sc = dmax;
                FORI( P1.colors ){
                    if ( OUT.user_mul[i] <= sc )
                        sc = OUT.user_mul[i];
                }
                
                if ( sc != 1.0 ) {
                    fprintf ( stderr, "Warning: The smallest channel multiplier should be 1.0.\n" );
                }
            }
        
            timerstart_timeval();
            if ( LIBRAW_SUCCESS != ( ret = RawProcessor.dcraw_process() ) )
                {
                    fprintf ( stderr,"Error: Cannot do postpocessing on %s: %s\n",
                              argv[arg],libraw_strerror(ret) );
                    if ( LIBRAW_FATAL_ERROR( ret ) )
                        continue; 
                }
        
            if ( use_timing )
                timerprint ( "LibRaw::dcraw_process()", argv[arg] );
            
            if (( cp = strrchr ( argv[arg], '.' ))) *cp = 0;
            snprintf( outfn, sizeof(outfn),
                      "%s%s",
                      argv[arg], "_aces.exr" );
            
            if ( verbosity >= 2 ) // verbosity set by repeat -v switches
                printf ("Converting to aces RGB\n" );
            else if ( verbosity )
                printf ("Writing file %s\n", outfn );
            
            if ( !P1.dng_version ) {
                libraw_processed_image_t *post_image = RawProcessor.dcraw_make_mem_image(&ret);
                if ( use_timing )
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
                            scale );
            }
//            else {
////              DNG files always use the embedded color profile
//                OUT.use_camera_wb = 0;
//                OUT.use_auto_wb = 0;
//                OUT.use_camera_matrix = 1;
//
//                libraw_processed_image_t *post_image = RawProcessor.dcraw_make_mem_image(&ret);
//                if(use_timing)
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
            if ( use_mmap && iobuffer )
            {
                munmap ( iobuffer, msize );
                iobuffer = 0;
            }
#endif
            RawProcessor.recycle();
        }
    return 0;
}
