// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include <rawtoaces/acesrender.h>

#include <set>

int main( int argc, const char *argv[] )
{
#ifndef WIN32
    putenv( (char *)"TZ=UTC" );
#else
    _putenv( (char *)"TZ=UTC" );
#endif

    rta::util::ImageConverter converter;

    OIIO::ArgParse arg_parser;
    arg_parser.arg( "filename" ).action( OIIO::ArgParse::append() ).hidden();
    converter.init_parser( arg_parser );

    if ( arg_parser.parse_args( argc, argv ) < 0 )
    {
        return 1;
    }

    if ( !converter.parse_parameters( arg_parser ) )
    {
        return 1;
    }

    auto files = arg_parser["filename"].as_vec<std::string>();
    if ( files.empty() || ( files.size() == 1 && files[0] == "" ) )
    {
        arg_parser.print_help();
        return 1;
    }

    // Create a separate batch for each input directory.
    // Reserve the first batch for the individual input files.
    std::vector<std::vector<std::string>> batches( 1 );

    // Gather all the raw images from arg list
    for ( const auto &path: files )
    {
        if ( !rta::util::collect_image_files( path, batches ) )
        {
            std::cerr << "File or directory not found: " << path << std::endl;
            return 1;
        }
    }

    // Process raw files ...
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
        arg_parser.print_help();

    return result ? 0 : 1;
}
