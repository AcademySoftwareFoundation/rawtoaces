// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#endif

#include <filesystem>
#include <rawtoaces/image_converter.h>

#include "../assert.h"

void test_AcesRender()
{
    const char *argv[] = {
        "dummy_binary_path",
    // getting "Subprocess aborted" in unit tests
    // when this is enabled on Mac, the tests fail on linux if
    // skipped completely.
#if __linux__
        "--wb-method",
        "metadata",
#endif
        "--mat-method",
        "metadata",
        "dummy_image_path"
    };

    const int argc = sizeof( argv ) / sizeof( argv[0] );

    std::string image_path =
        "/tests/materials/blackmagic_cinema_camera_cinemadng.dng";

    std::filesystem::path pathToRaw =
        std::filesystem::absolute( "../.." + image_path );

    // Depending if it run via tests/config_tests/CMakeLists.txt or
    // as part of coverage via tests/CMakeLists.txt
    if ( !std::filesystem::exists( pathToRaw ) )
    {
        pathToRaw = std::filesystem::absolute( "../../../.." + image_path );
    }

    RTA_CHECK_ASSERT( std::filesystem::exists( pathToRaw ) );

    rta::util::ImageConverter converter;

    OIIO::ArgParse arg_parser;
    arg_parser.arg( "filename" ).action( OIIO::ArgParse::append() ).hidden();
    converter.init_parser( arg_parser );

    RTA_CHECK_ASSERT( arg_parser.parse_args( argc, argv ) >= 0 );
    RTA_CHECK_ASSERT( converter.parse_parameters( arg_parser ) );

    auto files = arg_parser["filename"].as_vec<std::string>();
    RTA_CHECK_EQUAL( files.size(), size_t( 1 ) );

    RTA_CHECK_ASSERT(
        converter.settings.WB_method ==
        rta::util::ImageConverter::Settings::WBMethod::Metadata );
    RTA_CHECK_ASSERT(
        converter.settings.matrix_method ==
        rta::util::ImageConverter::Settings::MatrixMethod::Metadata );
}

int main( int, char ** )
{
    test_AcesRender();

    return unit_test_failures;
}
