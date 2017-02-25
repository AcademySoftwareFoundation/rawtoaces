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

#include "rawtoaces.h"

//void usage(const char *prog)
//{
//    printf ( "rawtoaces\n" );
//    printf ( "Usage:  %s [FILE]...\n", prog );
//    printf ( "OR Usage:  %s [OPTION]... [FILE]...\n", prog );
//    printf ( "-c float-num       Set adjust maximum threshold (default 0.75)\n"
//             "-v        Verbose: print progress messages (repeated -v will add verbosity)\n"
//             "-a        Average the whole image for white balance\n"\
//             "-A <x y w h> Average a grey box for white balance\n"
//             "-r <r g b g> Set custom white balance\n"
//             "-C <r b>  Correct chromatic aberration\n"
//             "-P <file> Fix the dead pixels listed in this file\n"
//             "-K <file> Subtract dark frame (16-bit raw PGM)\n"
//             "-k <num>  Set the darkness level\n"
//             "-S <num>  Set the saturation level\n"
//             "-n <num>  Set threshold for wavelet denoising\n"
//             "-H [0-9]  Highlight mode (0=clip, 1=unclip, 2=blend, 3+=rebuild)\n"
//             "-t [0-7]  Flip image (0=none, 3=180, 5=90CCW, 6=90CW)\n"
//             "-j        Don't stretch or rotate raw pixels\n"
//             "-W        Don't automatically brighten the image\n"
//             "-b <num>  Adjust brightness (default = 1.0)\n"
//             "-q [0-3]  Set the interpolation quality\n"
//             "-h        Half-size color image (twice as fast as \"-q 0\")\n"
//             "-f        Interpolate RGGB as four colors\n"
//             "-m <num>  Apply a 3x3 median filter to R-G and B-G\n"
//             "-s [0..N-1] Select one raw image from input file\n"
//             "-g pow ts Set gamma curve to gamma pow and toe slope ts (default = 2.222 4.5)\n"
//             "-G        Use green_matching() filter\n"
//             "-B <x y w h> use cropbox\n"
//             "-F        Use FILE I/O instead of streambuf API\n"
//             "-d        Detailed timing report\n"
//             "-r        User supplied channel multipliers, at least one of them should be 1.0\n"
//             "-D        Using the coeff matrix from Adobe\n"
//             "-Q        Specify the path to camera sensitivity data\n"
//             "-M        Set the value for scaling\n"
//             "-T        Set the desired color temperature (e.g., D60)\n"
//             "-z        Use camera metadata method and/or libraw method\n"
//    #ifndef WIN32
//             "-E        Use mmap()-ed buffer instead of plain FILE I/O\n"
//    #endif
//            );
//}

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
            "  --help                Show this screen\n"
            "  --verison             Show version\n"
            "  --wb-method [0-4]     White balance factor calculation method\n"
            "                          0=Calculate white balance from camera spec sens\n"
            "                          1=Use file metadata for white balance\n"
            "                          2=Average the whole image for white balance\n"
            "                          3=Average a grey box for white balance <x y w h>\n"
            "                          4=Use custom white balance  <r g b g>\n"
            "                          (default = 0)\n"
            "  --mat-method [0-2]    IDT matrix calculation method\n"
            "                          0=Calculate matrix from camera spec sens\n"
            "                          1=Use file metadata color matrix\n"
            "                          2=Use adobe coeffs\n"
            // Future feature ? "      3=Use custom matrix <m1r m1g m1b m2r m2g m2b m3r m3g m3b>\n"
            "                          (default = 0)\n"
            "  --ss-path <path>      Specify the path to camera sensitivity data\n"
            "  --exp-comp float      Set exposure compensation factor (default = 1.0)\n"
            "  --adopt-white         Set the desired color temperature (e.g., D60)\n"
            "\n"
            "Raw conversion options:\n"
            "  -c float              Set adjust maximum threshold (default 0.75)\n"
            "  -C <r b>              Correct chromatic aberration\n"
            "  -P <file>             Fix the dead pixels listed in this file\n"
            "  -K <file>             Subtract dark frame (16-bit raw PGM)\n"
            "  -k <num>              Set the darkness level\n"
            "  -n <num>              Set threshold for wavelet denoising\n"
            "  -H [0-9]              Highlight mode (0=clip, 1=unclip, 2=blend, 3+=rebuild)\n"
            "  -t [0-7]              Flip image (0=none, 3=180, 5=90CCW, 6=90CW)\n"
            "  -j                    Don't stretch or rotate raw pixels\n"
            "  -W                    Don't automatically brighten the image\n"
            "  -b <num>              Adjust brightness (default = 1.0)\n"
            "  -q [0-3]              Set the interpolation quality\n"
            "  -h                    Half-size color image (twice as fast as \"-q 0\")\n"
            "  -f                    Interpolate RGGB as four colors\n"
            "  -m <num>              Apply a 3x3 median filter to R-G and B-G\n"
            "  -s [0..N-1]           Select one raw image from input file\n"
            "  -G                    Use green_matching() filter\n"
            "  -B <x y w h>          Use cropbox\n"
            "\n"
            "Benchmarking options:\n"
            "  -v                    Verbose: print progress messages (repeated -v will add verbosity)\n"
            "  -F                    Use FILE I/O instead of streambuf API\n"
            "  -d                    Detailed timing report\n"
#ifndef WIN32
            "  -E                    Use mmap()-ed buffer instead of plain FILE I/O\n"
#endif
            );
    exit(1);
}

int my_progress_callback ( void *d,
                           enum LibRaw_progress p,
                           int iteration, int expected )
{
    char * passed  = ( char* ) (d ? d:"default string");
    
    if ( verbosity > 2 )
    {
        printf ("CB: %s  pass %d of %d (data passed=%s)\n",
                 libraw_strprogress(p),
                 iteration,
                 expected,
                 passed );
    }
    else if ( iteration == 0 )
        printf ( "start_timevaling %s (expecting %d iterations)\n",
                 libraw_strprogress(p),
                 expected );
    else if (iteration == expected-1)
        printf ( "%s finished\n",
                 libraw_strprogress(p) );
    
    ///    if(++cnt>10) return 1;
    
    return 0;
}

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
    float msec = ( end_timeval.tv_sec - start_timeval.tv_sec)*1000.0f + (end_timeval.tv_usec - start_timeval.tv_usec )/1000.0f;
    
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
}

#endif

