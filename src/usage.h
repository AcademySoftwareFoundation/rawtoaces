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

#ifndef _USAGE_h__
#define _USAGE_h__

#include "acesrender.h"

//	=====================================================================
//  Print usage / help message
//
//	inputs:
//      const char * : name of the program (i.e., rawtoaces)
//
//	outputs:
//		N/A

void usage(const char *prog)
{
    printf ( "%s - convert RAW digital camera files to ACES\n", prog);
    printf ( "\n");
    printf ( "Usage:\n");
    printf ( "  %s file ...\n", prog );
    printf ( "  %s [options] file\n", prog );
    printf ( "  %s --help\n", prog );
    printf ( "  %s --version\n", prog );
    printf ( "\n");
    printf ( "IDT options:\n"
            "  --help                  Show this screen\n"
            "  --version               Show version\n"
            "  --wb-method [0-4]       White balance factor calculation method\n"
            "                            0=white balance using file metadata \n"
            "                            1=white balance using user specified illuminant [str] \n"
            "                            2=Average the whole image for white balance\n"
            "                            3=Average a grey box for white balance <x y w h>\n"
            "                            4=Use custom white balance  <r g b g>\n"
            "                            (default = 0)\n"
            "  --mat-method [0-2]      IDT matrix calculation method\n"
            "                            0=Calculate matrix from camera spec sens\n"
            "                            1=Use file metadata color matrix\n"
            "                            2=Use adobe coeffs included in libraw\n"
            // Future feature ? "        3=Use custom matrix <m1r m1g m1b m2r m2g m2b m3r m3g m3b>\n"
            "                            (default = 0)\n"
            "  --ss-path <path>        Specify the path to camera sensitivity data\n"
            "                            (default = /usr/local/include/rawtoaces/data/camera)\n"
            "  --exp-comp float        Set exposure compensation factor (default = 1.0)\n"
            "\n"
            "Raw conversion options:\n"
            "  -c float                Set adjust maximum threshold (default = 0.75)\n"
            "  -C <r b>                Correct chromatic aberration\n"
            "  -P <file>               Fix the dead pixels listed in this file\n"
            "  -K <file>               Subtract dark frame (16-bit raw PGM)\n"
            "  -k <num>                Set the darkness level\n"
            "  -S <num>                Set the saturation level\n"
            "  -n <num>                Set threshold for wavelet denoising\n"
            "  -H [0-9]                Highlight mode (0=clip, 1=unclip, 2=blend, 3+=rebuild) (default = 0)\n"
            "  -t [0-7]                Flip image (0=none, 3=180, 5=90CCW, 6=90CW)\n"
            "  -j                      Don't stretch or rotate raw pixels\n"
            "  -W                      Don't automatically brighten the image\n"
            "  -b <num>                Adjust brightness (default = 1.0)\n"
            "  -q [0-3]                Set the interpolation quality\n"
            "  -h                      Half-size color image (twice as fast as \"-q 0\")\n"
            "  -f                      Interpolate RGGB as four colors\n"
            "  -m <num>                Apply a 3x3 median filter to R-G and B-G\n"
            "  -s [0..N-1]             Select one raw image from input file\n"
            "  -G                      Use green_matching() filter\n"
            "  -B <x y w h>            Use cropbox\n"
            "\n"
            "Benchmarking options:\n"
            "  -v                      Verbose: print progress messages (repeated -v will add verbosity)\n"
            "  -F                      Use FILE I/O instead of streambuf API\n"
            "  -d                      Detailed timing report\n"
#ifndef WIN32
            "  -E                      Use mmap()-ed buffer instead of plain FILE I/O\n"
#endif
            );
    exit(-1);
};


//	=====================================================================
//  Prepare the matching between string flags and single character flag
//
//	inputs:
//      N/A
//
//	outputs:
//		N/A : keys should be prepared and loaded

void create_key()
{
    keys["--help"] = 'I';
    keys["--version"] = 'V';
    keys["--wb-method"] = 'R';
    keys["--mat-method"] = 'p';
    keys["--ss-path"] = 'Q';
    keys["--exp-comp"] = 'M';
//    keys["--adopt-white"] = 'T';
    keys["--valid-illum"] = 'z';
    keys["-c"] = 'c';
    keys["-C"] = 'C';
    keys["-P"] = 'P';
    keys["-K"] = 'K';
    keys["-k"] = 'k';
    keys["-S"] = 'S';
    keys["-n"] = 'n';
    keys["-H"] = 'H';
    keys["-t"] = 't';
    keys["-j"] = 'j';
    keys["-W"] = 'W';
    keys["-b"] = 'b';
    keys["-q"] = 'q';
    keys["-h"] = 'h';
    keys["-f"] = 'f';
    keys["-m"] = 'm';
    keys["-s"] = 's';
    keys["-G"] = 'G';
    keys["-B"] = 'B';
    keys["-v"] = 'v';
    keys["-F"] = 'F';
    keys["-d"] = 'd';
    keys["-E"] = 'E';
};

//	=====================================================================
//  Initialize Option structure
//
//	inputs:
//      N/A
//
//	outputs:
//		N/A : opts should be initialized

void initialize (option &opts)
{
    opts.use_bigfile     = 0;
    opts.use_timing      = 0;
    opts.use_camera_path = 0;
    opts.use_illum       = 0;
    opts.use_Mul         = 0;
    opts.verbosity       = 0;
    opts.use_mat         = 0;
    opts.use_wb          = 0;
    opts.highlight       = 0;
    opts.scale           = 1.0;
    
    struct stat st;
    dataPath dp = pathsFinder ();

    FORI ( dp.paths.size() ) {
        if ( !stat( (dp.paths)[i].c_str(), &st ) )
            opts.EnvPaths.push_back((dp.paths)[i]);
    }
    
#ifndef WIN32
    opts.msize = 0;
    opts.use_mmap=0;
#endif
    
};

//	=====================================================================
//  Fetch flags specified by users and configure preliminary settings
//
//	inputs:
//      int    : number of supplied arguments
//      char * : an array of supplied arguments
//      libraw_output_params_t : libraw parameters to manage
//                               dcraw processing
//	outputs:
//		ibt    : current position in the argument array

int configureSetting ( int argc,
                       char * argv[],
                       option &opts,
                       libraw_output_params_t &OUT )
{
    create_key();
    initialize(opts);
    
    char *cp, *sp;
    int arg;
    
    vector < string > vls (lightS, lightS + sizeof(lightS) / sizeof(char *));
    argv[argc] = (char *)"";
    
    for ( arg = 1; arg < argc; )
    {
        string key(argv[arg]);
        
        if ( key[0] != '-' ) {
            break;
        }
        
        arg++;
        char opt = keys[key];
        
        if (!opt) {
            fprintf (stderr,"\nNon-recognizable flag - \"%s\"\n", key.c_str());
            exit(-1);
        }
        
        // HMgcnbksStqmBC
        if (( cp = strchr ( sp = (char*)"HMGcnbksStqmBC", opt )) != 0 ) {
            for (int i=0; i < "111411111142"[cp-sp]-'0'; i++) {
                if (!isdigit(argv[arg+i][0]))
                {
                    fprintf ( stderr, "\nError: Non-numeric argument to "
                                      "\"%s\"\n", key.c_str() );
                    exit(-1);
                }
            }
        }
        else if ((cp = strchr ( sp = ( char * )"T", opt )) != 0) {
            for (int i=0; i < "111411111142"[cp-sp]-'0'; i++) {
                if ( !isalnum(argv[arg+i][0] ) )
                {
                    fprintf (stderr,"\nNon-numeric and/or Non-compatible argument to "
                                    "\"%s\"\n", key.c_str());
                    exit(-1);
                }
            }
        }
        
        switch ( opt )
        {
            case 'I':  usage( argv[0] );  break;
            case 'V':  printf ( "%s\n", VERSION );  break;
            case 'v':  opts.verbosity++;  break;
            case 'G':  OUT.green_matching = 1; break;
            case 'c':  OUT.adjust_maximum_thr   = (float)atof(argv[arg++]);  break;
            case 'n':  OUT.threshold   = (float)atof(argv[arg++]);  break;
            case 'b':  OUT.bright      = (float)atof(argv[arg++]);  break;
            case 'P':  OUT.bad_pixels  = argv[arg++];        break;
            case 'K':  OUT.dark_frame  = argv[arg++];        break;
            case 'C': {
                OUT.aber[0] = 1.0 / atof(argv[arg++]);
                OUT.aber[2] = 1.0 / atof(argv[arg++]);
                break;
            }
            case 'k':  OUT.user_black  = atoi(argv[arg++]);  break;
            case 'S':  OUT.user_sat    = atoi(argv[arg++]);  break;
            case 't':  OUT.user_flip   = atoi(argv[arg++]);  break;
            case 'q':  OUT.user_qual   = atoi(argv[arg++]);  break;
            case 'm':  OUT.med_passes  = atoi(argv[arg++]);  break;
            case 'H':  OUT.highlight   = atoi(argv[arg++]);  break;
            case 'h':  OUT.half_size         = 1;
                // no break:  "-h" implies "-f"
            case 'f':  OUT.four_color_rgb      = 1;            break;
            case 'B':  FORI(4) OUT.cropbox[i]  = atoi(argv[arg++]); break;
            case 'j':  OUT.use_fuji_rotate     = 0;  break;
            case 'W':  OUT.no_auto_bright      = 1;  break;
            case 'F':  opts.use_bigfile        = 1;  break;
            case 'd':  opts.use_timing         = 1;  break;
            case 'z':  printVS(vls);                 break;
            case 'p': {
                opts.use_mat = atoi(argv[arg++]);
                if ( opts.use_mat > 2
                     || opts.use_mat < -1) {
                    fprintf (stderr, "\nError: Invalid argument to "
                                     "\"%s\" \n", key.c_str());
                    exit(-1);
                }
                break;
            }
            case 'Q': {
                opts.use_camera_path = 1;
                opts.cameraSenPath = (char *)(argv[arg++]);
                break;
            }
//            case 'T': {
//                opts.use_illum = 1;
//                opts.illumType = (char *)(argv[arg++]);
//                break;
//            }
            case 'M':  opts.scale = atof(argv[arg++]); break;
            case 'R': {
                std::string flag = std::string(argv[arg]);
                FORI ( flag.size() ) {
                    if ( !isdigit (flag[i]) ) {
                        fprintf (stderr,"\nNon-recognizable argument to "
                                        "\"--wb-method\".\n");
                        exit(-1);
                    }
                }
                
                opts.use_wb = atoi(argv[arg++]);
                
                if ( opts.use_wb == 1 ) {
                    if ( isalnum(argv[arg][0]) )
                    {
                        opts.use_illum = 1;
                        opts.illumType = (char *)(argv[arg++]);
                    }
                    else {
                        fprintf( stderr,"\nError: Please specify a "
                                 "desirable illuminant to proceed.\n" );
                        exit(-1);
                    }
                }
                if ( opts.use_wb == 3 ) {
                    FORI(4) {
                        if ( !isdigit(argv[arg][0]) )
                        {
                            fprintf (stderr, "\nError: Non-numeric argument to "
                                             "\"%s %i\" \n",
                                             key.c_str(),
                                             opts.use_wb);
                            exit(-1);
                        }
                        OUT.greybox[i] = (float)atof(argv[arg++]);
                    }
                }
                else if ( opts.use_wb == 4 ) {
                    opts.use_Mul = 1;
                    FORI(4) {
                        if ( !isdigit(argv[arg][0]) )
                        {
                            fprintf (stderr, "\nError: Non-numeric argument to "
                                             "\"%s %i\" \n",
                                             key.c_str(),
                                             opts.use_wb);
                            exit(-1);
                        }
                        OUT.user_mul[i] = (float)atof(argv[arg++]);
                    }
                }
                else if ( opts.use_wb > 4
                         || opts.use_wb < -1) {
                    fprintf (stderr, "\nError: Invalid argument to "
                                     "\"%s\" \n",
                                     key.c_str());
                    exit(-1);
                }
                break;
            }
#ifndef WIN32
            case 'E':  opts.use_mmap = 1;  break;
#endif
            default:
                fprintf ( stderr, "\nError: Unknown option \"%s\".\n", key.c_str() );
                exit(-1);
        }
    }
    
    if ( opts.use_camera_path ) {
        string cameraSenPathS( opts.cameraSenPath );
        if ( cameraSenPathS.find("_380_780") == std::string::npos ) {
            fprintf( stderr,"\nError: Cannot locate camera "
                     "sensitivity data in the file.\n" );
            exit(-1);
        }
    }
    
    if ( opts.use_illum ) {
        lowerCase ( opts.illumType );
        bool illumCmp = 0;
        string strIllm (opts.illumType);
        FORI ( vls.size() ) {
            if ( strIllm.compare(vls[i]) == 0 ) {
                illumCmp = 1;
                break;
            }
        }
        
        if ( !illumCmp ) {
            fprintf ( stderr, "\nError: Unknown light source - %s.\n"
                      "Please use \"--valid-illum\" to see a "
                      "list of available light sources.\n",
                      strIllm.c_str());
            exit(-1);
        }
    }
    
    if ( opts.use_wb == 1 &&
        opts.use_illum != 1 ) {
        fprintf( stderr,"\nError: Please specify a "
                 "desirable illuminant to proceed.\n" );
        exit(-1);
    }
    
    if ( opts.use_camera_path
        && (opts.use_wb || opts.use_mat)) {
        fprintf ( stderr, "\n\"Warning: --wb-method\" and/or \"--mat-method\" "
                          "will be forced be 0 when the external "
                          "camera spectral data is used.\n\n");
        
        opts.use_wb = 0;
        opts.use_mat = 0;
    }
    
    return arg;
};

int my_progress_callback ( void *d,
                           enum LibRaw_progress p,
                           int iteration,
                           int expected
                          )
{
    char * passed  = ( char* ) (d ? d:"default string");
    
    printf ("CB: %s  pass %d of %d (data passed=%s)\n",
            libraw_strprogress(p),
            iteration,
            expected,
            passed );
    
    if ( iteration == 0 )
        printf ( "start_timevaling %s (expecting %d iterations)\n",
                 libraw_strprogress(p),
                 expected );
    else if (iteration == expected-1)
        printf ( "%s finished\n",
                 libraw_strprogress(p) );
    
    ///    if(++cnt>10) return 1;
    
    return 0;
};

// timer
#ifndef WIN32
static struct timeval start_timeval,end_timeval;
void timerstart_timeval (void)
{
    gettimeofday ( &start_timeval, NULL );
}

void timerprint ( const char *msg, const char *filename )
{
    gettimeofday ( &end_timeval,NULL );
    float msec = ( end_timeval.tv_sec - start_timeval.tv_sec)*1000.0f
                   + (end_timeval.tv_usec - start_timeval.tv_usec ) / 1000.0f;
    
    printf( "Timing: %s/%s: %6.3f msec\n",
            filename,
            msg,
            msec );
}
#else
LARGE_INTEGER start_timeval;
void timerstart_timeval ( void )
{
    QueryPerformanceCounter ( &start_timeval );
}

void timerprint (const char * msg, const char * filename)
{
    LARGE_INTEGER unit,end_timeval;
    
    QueryPerformanceCounter ( &end_timeval );
    QueryPerformanceFrequency ( &unit );
    
    float msec = (float) ( end_timeval.QuadPart - start_timeval.QuadPart );
    msec /= (float) unit.QuadPart / 1000.0f;
    printf ( "Timing: %s/%s: %6.3f msec\n",
             filename,msg,msec );
};
#endif

#endif

