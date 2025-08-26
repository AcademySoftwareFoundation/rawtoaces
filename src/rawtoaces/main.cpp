// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include <rawtoaces/acesrender.h>

#include <set>

int main( int argc, char *argv[] )
{
#ifndef WIN32
    putenv( (char *)"TZ=UTC" );
#else
    _putenv( (char *)"TZ=UTC" );
#endif

    if ( argc == 1 )
        rta::util::ImageConverter::usage( argv[0] );

    rta::util::ImageConverter converter;

    int arg = converter.configure_settings( argc, argv );

    // Create a separate batch for each input directory.
    // Reserve the first batch for the individual input files.
    std::vector<std::vector<std::string>> batches( 1 );

    // Gather all the raw images from arg list
    for ( ; arg < argc; arg++ )
    {
        std::string path = argv[arg];

        if ( !rta::util::collect_image_files( path, batches ) )
        {
            std::cerr << "File or directory not found: " << path << std::endl;
            return 1;
        }
    }

    // Process RAW files ...
    bool empty  = true;
    bool result = true;
    for ( auto const &batch: batches )
    {
        for ( auto const &input_filename: batch )
        {
            empty  = false;
            result = converter.process_image( input_filename );
            if ( !result )
                break;
        }
        if ( !result )
            break;
    }

    if ( empty )
        rta::util::ImageConverter::usage( argv[0] );

    return result ? 0 : 1;
}
