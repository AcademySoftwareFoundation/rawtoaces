// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include <rawtoaces/acesrender.h>
#include <rawtoaces/usage_timer.h>

int main( int argc, char *argv[] )
{
    if ( argc == 1 )
        usage( argv[0] );

    struct stat st;
    AcesRender &Render = AcesRender::getInstance();

#ifndef WIN32
    putenv( (char *)"TZ=UTC" );
#else
    _putenv( (char *)"TZ=UTC" );
#endif

    // Fetch conditions and conduct some pre-processing
    Render.initialize( pathsFinder() );
    int arg = Render.configureSettings( argc, argv );

    // Gather all the raw images from arg list
    std::vector<std::string> RAWs;
    for ( ; arg < argc; arg++ )
    {
        if ( stat( argv[arg], &st ) != 0 )
        {
            fprintf(
                stderr,
                "Error: The directory or file may not exist - \"%s\"...",
                argv[arg] );
            continue;
        }

        if ( st.st_mode & S_IFDIR )
        {
            std::vector<std::string> files =
                openDir( static_cast<std::string>( argv[arg] ) );
            for ( std::vector<std::string>::iterator file = files.begin();
                  file != files.end();
                  ++file )
            {
                if ( stat( file->c_str(), &st ) == 0 &&
                     ( st.st_mode & S_IFREG ) )
                    RAWs.push_back( *file );
            }
        }
        else if ( st.st_mode & S_IFREG )
        {
            RAWs.push_back( argv[arg] );
        }
    }

    // Load illuminant dataset(s)
    int    read = 0;
    Option opts = Render.getSettings();
    if ( !opts.illumType )
        read = Render.fetchIlluminant();
    else
        read = Render.fetchIlluminant( opts.illumType );

    if ( !read )
    {
        fprintf(
            stderr,
            "\nError: No matching light source. "
            "Please find available options by "
            "\"rawtoaces --valid-illum\".\n" );
        exit( -1 );
    }

    rta::util::UsageTimer usage_timer;
    usage_timer.enabled = opts.use_timing;

    // Process RAW files ...
    FORI( RAWs.size() )
    {
        std::string raw = ( RAWs[i] ).c_str();

        usage_timer.reset();
        Render.preprocessRaw( raw.c_str() );
        usage_timer.print( raw, "AcesRender::preprocessRaw()" );

        usage_timer.reset();
        Render.postprocessRaw();
        usage_timer.print( raw, "AcesRender::postprocessRaw()" );

        usage_timer.reset();

        std::string output;
        size_t      pos = raw.rfind( '.' );
        if ( pos != std::string::npos )
        {
            output = raw.substr( 0, pos );
        }
        output += "_aces.exr";

        Render.outputACES( output.c_str() );
        usage_timer.print( raw, "AcesRender::outputACES()" );
    }

    return 0;
}
