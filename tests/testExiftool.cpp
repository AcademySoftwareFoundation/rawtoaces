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
    OIIO::ImageSpec spec;
    std::string     output;
    bool            success =
        rta::util::exiftool::fetch_metadata( spec, test_file, keys, output );

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
    std::cout << "\n" << __FUNCTION__ << "\n";

    set_exiftool_path( false, false );

    std::string output = check( false );
    ASSERT_CONTAINS( output, "Exiftool not found" );
}

void testExiftool_bad_env()
{
    std::cout << "\n" << __FUNCTION__ << "\n";

    set_exiftool_path( false, false );
    set_env_var( "RAWTOACES_EXIFTOOL_PATH", "bad_path" );

    std::string output = check( false );
    ASSERT_CONTAINS( output, "Failed to execute exiftool" );
}

void testExiftool_tool_in_env()
{
    std::cout << "\n" << __FUNCTION__ << "\n";

    set_exiftool_path( true, false );

    std::string output = check( true );
}

void testExiftool_tool_in_path()
{
    std::cout << "\n" << __FUNCTION__ << "\n";

    set_exiftool_path( false, true );

    std::string output = check( true );
}

void testExiftool_bad_key()
{
    std::cout << "\n" << __FUNCTION__ << "\n";

    set_exiftool_path( false, true );

    std::string output = check( false, { "bad_key" } );
    ASSERT_CONTAINS( output, "Exiftool: unknown key " );
}

std::string make_test_file(
    bool write_camera_model,
    bool write_lens_model,
    bool write_aperture,
    bool write_focal_length,
    bool write_focus_distance,
    bool write_focus_distance_upper,
    bool write_focus_distance_lower )
{
    // Create temporary output file path
    std::filesystem::path output_path =
        std::filesystem::temp_directory_path() / "rta_exif_test.xmp";

    std::ofstream stream( output_path );
    stream << "<?xpacket begin='﻿'?>\n";
    stream << "<x:xmpmeta>\n";
    stream << "<rdf:RDF>\n";
    stream << " <rdf:Description>\n";

    if ( write_camera_model )
        stream << "   <exif:Model>lens_name</exif:Model>\n";
    if ( write_lens_model )
        stream << "   <exif:LensID>lens_name</exif:LensID>\n";
    if ( write_aperture )
        stream << "   <exif:FNumber>28/5</exif:FNumber>\n";
    if ( write_focal_length )
        stream << "   <exif:FocalLength>50/1</exif:FocalLength>\n";
    if ( write_focus_distance )
        stream << "   <exif:FocusDistance>100/1</exif:FocusDistance>\n";
    if ( write_focus_distance_upper )
        stream
            << "   <exif:FocusDistanceUpper>600/1</exif:FocusDistanceUpper>\n";
    if ( write_focus_distance_lower )
        stream
            << "   <exif:FocusDistanceLower>300/1</exif:FocusDistanceLower>\n";

    stream << " </rdf:Description>\n";
    stream << "</rdf:RDF>\n";
    stream << "</x:xmpmeta>\n";
    stream << "<?xpacket end='w'?>\n";
    stream.close();

    return output_path.string();
}

void test_focus_distance()
{
    std::cout << "\n" << __FUNCTION__ << "\n";

    set_exiftool_path( false, true );

    const struct test
    {
        bool  focus;
        bool  focus_upper;
        bool  focus_lower;
        float value;
    } tests[] = { { false, false, false, 0.0f },
                  { true, false, false, 100.0f },
                  { false, true, false, 600.0f },
                  { false, false, true, 300.0f },
                  { false, true, true, 400.0f } };

    for ( size_t i = 0; i < sizeof( tests ) / sizeof( test ); i++ )
    {
        std::string path = make_test_file(
            true,
            true,
            true,
            true,
            tests[i].focus,
            tests[i].focus_upper,
            tests[i].focus_lower );

        OIIO::ImageSpec spec;
        std::string     error_message;
        bool            success = rta::util::exiftool::fetch_metadata(
            spec, path, { "focus" }, error_message );

        OIIO_CHECK_ASSERT( success );
        OIIO_CHECK_EQUAL( spec.get_float_attribute( "focus" ), tests[i].value );
        OIIO_CHECK_EQUAL( error_message, "" );
    }
}

int main( int, char ** )
{
    testExiftool_tool_not_found();
    testExiftool_bad_env();
    testExiftool_tool_in_env();
    testExiftool_tool_in_path();
    testExiftool_bad_key();

    test_focus_distance();

    return unit_test_failures;
}
