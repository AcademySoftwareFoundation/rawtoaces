// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

// must be before <OpenImageIO/unittest.h>
#include <rawtoaces/image_converter.h>

#include <filesystem>
#include <OpenImageIO/unittest.h>

// This file contains some usage examples of the util library.
// It has only very little unit test functionality to keep the code clean.

/// Test the image converter using command line parameters for intialisation.
void test_ImageConverter_arguments()
{
    // This test fails on CI runners having an old version of OIIO.
    if ( OIIO::openimageio_version() < 30000 )
        return;

    const char *image_path =
        "../../tests/materials/blackmagic_cinema_camera_cinemadng.dng";
    std::string absolute_image_path =
        std::filesystem::absolute( image_path ).string();

    // Input parameters.
    std::filesystem::path temp_output_dir =
        std::filesystem::temp_directory_path() /
        "rawtoaces_usage_example_output";
    std::cout << "temp_output_dir: " << temp_output_dir << std::endl;
    std::filesystem::create_directories( temp_output_dir );
    std::string temp_output_dir_str = temp_output_dir.string();
    const char *argv[]              = { "DUMMY PROGRAM PATH", "--wb-method",
                                        "metadata",           "--mat-method",
                                        "metadata",           "--overwrite",
                                        "--output-dir",       temp_output_dir_str.c_str() };

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
}

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
    converter.settings.WB_method =
        rta::util::ImageConverter::Settings::WBMethod::Metadata;
    converter.settings.matrix_method =
        rta::util::ImageConverter::Settings::MatrixMethod::Metadata;
    converter.settings.overwrite = true;

    // Process an image.
    bool result = converter.process_image( absolute_image_path );

    // Check the result.
    OIIO_CHECK_ASSERT( result );
}

int main( int, char ** )
{
    // Run on Linux CI and macOS runners
#if defined( __LINUX__ ) || defined( __APPLE__ )
    test_ImageConverter_arguments();
    test_ImageConverter_settings();
#endif

    return unit_test_failures;
}
