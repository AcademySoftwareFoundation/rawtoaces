// Copyright Contributors to the rawtoaces project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/rawtoaces

#ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#endif

#include <filesystem>
#include <OpenImageIO/unittest.h>

#include <rawtoaces/image_converter.h>

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

    std::filesystem::path pathToRaw = std::filesystem::absolute(
        "../../tests/materials/blackmagic_cinema_camera_cinemadng.dng" );
    OIIO_CHECK_ASSERT( std::filesystem::exists( pathToRaw ) );

    rta::util::ImageConverter converter;

    OIIO::ArgParse arg_parser;
    arg_parser.arg( "filename" ).action( OIIO::ArgParse::append() ).hidden();
    converter.init_parser( arg_parser );

    OIIO_CHECK_ASSERT( arg_parser.parse_args( argc, argv ) >= 0 );
    OIIO_CHECK_ASSERT( converter.parse_parameters( arg_parser ) );

    auto files = arg_parser["filename"].as_vec<std::string>();
    OIIO_CHECK_EQUAL( files.size(), 1 );

    OIIO_CHECK_ASSERT(
        converter.settings.wbMethod ==
        rta::util::ImageConverter::Settings::WBMethod::Metadata );
    OIIO_CHECK_ASSERT(
        converter.settings.matrixMethod ==
        rta::util::ImageConverter::Settings::MatrixMethod::Metadata );

    // Disable for now. Needs better checks if the installed OIIO version
    // is compatible.
    if ( false ) //(OIIO::openimageio_version() > 20500 )
    {
        OIIO::ParamValueList hints;
        bool result = converter.configure( pathToRaw.string(), hints );
        OIIO_CHECK_EQUAL( result, true );

        auto idt = converter.get_IDT_matrix();

        double matrix[3][3] = { { 1.0536466144, 0.0039044182, 0.0049084502 },
                                { -0.4899562165, 1.3614787986, 0.1020844728 },
                                { -0.0024498461, 0.0060497128, 1.0139159537 } };

        for ( size_t i = 0; i < 3; i++ )
            for ( size_t j = 0; j < 3; j++ )
                OIIO_CHECK_EQUAL_THRESH( idt[i][j], matrix[i][j], 1e-5 );
    }
};

int main( int, char ** )
{
    test_AcesRender();

    return unit_test_failures;
}
