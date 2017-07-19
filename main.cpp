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
    
    struct Option opts;
    struct stat st;
    libraw_output_params_t OUT;
    
    AcesRender & Render = AcesRender::getInstance();
    
#ifndef WIN32
  putenv ((char*)"TZ=UTC");
#else
  _putenv ((char*)"TZ=UTC");
#endif

// Fetch conditions and conduct some pre-processing
    int arg = configureSetting (argc, argv, opts, OUT);
    Render.setSettings(opts, OUT);
    
// print a list of cameras supported by LibRaw
    if (opts.get_libraw_cameras) {
        Render.printLibRawCameras();
    }
    
// gather a list of illuminants supported
    if (opts.get_illums) {
        Render.gatherSupportedIllums();
        vector < string > ilist = Render.getSupportedIllums();
        printf("\nThe following illuminants are available:\n\n");
        FORI(ilist.size()) printf("%s \n", ilist[i].c_str());
    }
    
// gather a list of cameras supported
    if (opts.get_cameras) {
        Render.gatherSupportedCameras();
        vector < string > clist = Render.getSupportedCameras();
        printf("\nThe following cameras' sensitivity data is available:\n\n");
        FORI(clist.size()) printf("%s \n", clist[i].c_str());
    }
    
//   if ( opts.verbosity > 2 )
//       RawProcessor.set_progress_handler ( my_progress_callback,
//                                           ( void * )"Sample data passed" );
    
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
        timerstart_timeval();
        
        Render.preprocessRaw (raw);

//// dcraw_processing
////          Start the dcraw_process()
            timerstart_timeval();
//            if ( LIBRAW_SUCCESS != ( opts.ret = RawProcessor.dcraw_process() ) ) {
//                
//                fprintf ( stderr, "Error: Cannot do postpocessing on %s: %s\n\n",
//                          raw,libraw_strerror(opts.ret) );
//                if ( LIBRAW_FATAL_ERROR( opts.ret ) )
//                    continue;
//            }
        
//          Use the final wb factors to otbain IDT matrix
            // 0
//            if ( opts.mat_method == matMethod0 )
//                Render.prepareIDT ( P1, C.pre_mul );
        
        Render.postprocessRaw ();

        if ( opts.use_timing )
            timerprint ( "AcesRender::postprocessRaw()", raw );
        
        Render.renderACES ();
        if ( opts.use_timing )
            timerprint( "AcesRender::renderACES()", raw);
        
        Render.outputACES (raw);
        if ( opts.use_timing )
            timerprint( "AcesRender::outputACES()", raw);

    }

    return 0;
}
