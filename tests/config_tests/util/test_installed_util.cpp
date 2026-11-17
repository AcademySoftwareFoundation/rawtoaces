// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#endif

#include <rawtoaces/image_converter.h>

// A simple test to confirm that the library has been linked successfully.
int main( int, char ** )
{
    const char *argv[] = {
        "dummy_binary_path",
        "--mat-method",
        "metadata",
    };

    const int argc = sizeof( argv ) / sizeof( argv[0] );

    rta::util::ImageConverter converter;

    OIIO::ArgParse arg_parser;
    converter.init_parser( arg_parser );

    if ( arg_parser.parse_args( argc, argv ) < 0 )
        return 1;

    if ( !converter.parse_parameters( arg_parser ) )
        return 2;

    return 0;
}
