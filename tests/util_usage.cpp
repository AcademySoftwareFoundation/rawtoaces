// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include <filesystem>
#include <OpenImageIO/unittest.h>
#include <rawtoaces/image_converter.h>

// This file contains some usage examples of the util library.
// It has only very little unit test functionality to keep the code clean.

/// Test the image converter using command line parameters for intialisation.
void test_ImageConverter_arguments()
{
    // This test fails on CI runners having an old version of OIIO.
    if ( OIIO::openimageio_version() < 30000 )
        return;

    const char *image_path =
        "../../../tests/materials/blackmagic_cinema_camera_cinemadng.dng";
    std::string absolute_image_path =
        std::filesystem::absolute( image_path ).string();

    // Input parameters.
    const char *argv[] = { "DUMMY PROGRAM PATH",
                           "--wb-method",
                           "metadata",
                           "--mat-method",
                           "metadata" };

    const size_t argc = sizeof( argv ) / sizeof( argv[0] );

    // Parse the command line parameters and configure the converter.
    rta::util::ImageConverter converter;
    OIIO::ArgParse            arg_parser;
    converter.init_parser( arg_parser );
    arg_parser.parse_args( argc, argv );
    converter.parse_parameters( arg_parser );

    // Process an image.
    bool result = converter.process_image( absolute_image_path );

    // Check the result.
    OIIO_CHECK_ASSERT( result );
};

/// Test the image converter, initialising the settings struct directly.
void test_ImageConverter_settings()
{
    // This test fails on CI runners having an old version of OIIO.
    if ( OIIO::openimageio_version() < 30000 )
        return;

    const char *image_path =
        "../../tests/materials/blackmagic_cinema_camera_cinemadng.dng";
    std::string absolute_image_path =
        std::filesystem::absolute( image_path ).string();

    // Configure the converter.
    rta::util::ImageConverter converter;
    converter.settings.wbMethod =
        rta::util::ImageConverter::Settings::WBMethod::Metadata;
    converter.settings.matrixMethod =
        rta::util::ImageConverter::Settings::MatrixMethod::Metadata;

    // Process an image.
    bool result = converter.process_image( absolute_image_path );

    // Check the result.
    OIIO_CHECK_ASSERT( result );
};

int main( int, char ** )
{
    // Only run on the linux CI runners.
#ifdef __LINUX__
    test_ImageConverter_arguments();
    test_ImageConverter_settings();
#endif

    return unit_test_failures;
}
