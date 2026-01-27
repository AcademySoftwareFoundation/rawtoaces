// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#endif

#include "../src/rawtoaces_util/exiftool.h"

#include "test_utils.h"
#include <OpenImageIO/unittest.h>

const std::string test_file = "../../tests/materials/BatteryPark.NEF";

std::string check(
    bool                            should_succeed = true,
    const std::vector<std::string> &keys           = {
        "cameraMake", "cameraModel", "lensModel", "aperture", "focalLength" } )
{
    bool            success;
    OIIO::ImageSpec spec;
    std::string     output = capture_stderr( [&]() {
        success = rta::util::exiftool::fetch_metadata( spec, test_file, keys );
    } );

    OIIO_CHECK_EQUAL( success, should_succeed );

    if ( should_succeed )
    {
        OIIO_CHECK_EQUAL(
            spec.get_string_attribute( "cameraMake" ), "NIKON CORPORATION" );
        OIIO_CHECK_EQUAL(
            spec.get_string_attribute( "cameraModel" ), "NIKON D200" );
        OIIO_CHECK_EQUAL(
            spec.get_string_attribute( "lensModel" ),
            "AF Zoom-Nikkor 28-70mm f/3.5-4.5D" );
        OIIO_CHECK_EQUAL( spec.get_float_attribute( "aperture" ), 8.0f );
        OIIO_CHECK_EQUAL( spec.get_float_attribute( "focalLength" ), 28.0f );
    }
    return output;
}

void testExiftool_tool_not_found()
{
    unset_env_var( "RAWTOACES_EXIFTOOL_PATH" );
    unset_env_var( "PATH" );

    std::string output = check( false );
    ASSERT_CONTAINS( output, "Exiftool not found" );
}

void testExiftool_bad_env()
{
    set_env_var( "RAWTOACES_EXIFTOOL_PATH", "bad_path" );
    unset_env_var( "PATH" );

    std::string output = check( false );
    ASSERT_CONTAINS( output, "Failed to execute exiftool" );
}

void testExiftool_tool_in_env()
{
#if defined( WIN32 ) || defined( WIN64 )
    const char *exiftool_path = "..\\..\\exiftool\\exiftool.exe";
#elif defined( __APPLE__ )
    const char *exiftool_path = "/opt/homebrew/bin/exiftool";
#else
    const char *exiftool_path = "/usr/bin/exiftool";
#endif
    set_env_var( "RAWTOACES_EXIFTOOL_PATH", exiftool_path );
    unset_env_var( "PATH" );

    std::string output = check( true );
}

void testExiftool_tool_in_path()
{
#if defined( WIN32 ) || defined( WIN64 )
    const char *exiftool_path = "some_path;..\\..\\exiftool";
#elif defined( __APPLE__ )
    const char *exiftool_path = "some_path:/opt/homebrew/bin";
#else
    const char *exiftool_path = "some_path:/usr/bin";
#endif

    unset_env_var( "RAWTOACES_EXIFTOOL_PATH" );
    set_env_var( "PATH", exiftool_path );

    std::string output = check( true );
}

void testExiftool_bad_key()
{
#if defined( WIN32 ) || defined( WIN64 )
    const char *exiftool_path = "..\\..\\exiftool\\exiftool.exe";
#elif defined( __APPLE__ )
    const char *exiftool_path = "/opt/homebrew/bin/exiftool";
#else
    const char *exiftool_path = "/usr/bin/exiftool";
#endif
    set_env_var( "RAWTOACES_EXIFTOOL_PATH", exiftool_path );
    unset_env_var( "PATH" );

    std::string output = check( false, { "bad_key" } );
    ASSERT_CONTAINS( output, "Exiftool: unknown key " );
}

int main( int, char ** )
{
    testExiftool_tool_not_found();
    testExiftool_bad_env();
    testExiftool_tool_in_env();
    testExiftool_tool_in_path();
    testExiftool_bad_key();

    return unit_test_failures;
}
