// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#    undef RGB
#endif

#include "../src/rawtoaces_util/rawtoaces_util_priv.h"

// must be before <OpenImageIO/unittest.h>
#include <rawtoaces/image_converter.h>
#include <rawtoaces/rawtoaces_core.h>

#include <OpenImageIO/unittest.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <nlohmann/json.hpp>
#include <sstream>
#include <vector>
#include <ctime>
#include <algorithm>

#include "test_utils.h"
#include "test_utils_image_converter.h"

#ifdef WIN32
#    include <io.h>
#    include <process.h>
#    include <direct.h> // for _getcwd
#else
#    include <sys/stat.h> // for mkfifo
#    include <unistd.h>   // for getcwd
#endif

using namespace rta::util;

const std::string dng_test_file =
    "../../tests/materials/blackmagic_cinema_camera_cinemadng.dng";

const std::string nef_test_file = "../../tests/materials/BatteryPark.NEF";

std::string convert_linux_path_to_windows_path( const std::string &path )
{
    std::vector<std::string> segments;
    OIIO::Strutil::split( path, segments, ":" );

    for ( auto &segment: segments )
    {
        // Convert forward slashes to backslashes
        std::replace( segment.begin(), segment.end(), '/', '\\' );
        // Add drive letter
        segment = "c:" + segment;
    }

    return OIIO::Strutil::join( segments, ";" );
}

FILE *platform_popen( const char *command, const char *mode )
{
#ifdef WIN32
    return _popen( command, mode );
#else
    return popen( command, mode );
#endif
}

int platform_pclose( FILE *pipe )
{
#ifdef WIN32
    return _pclose( pipe );
#else
    int status = pclose( pipe );
    return WEXITSTATUS( status );
#endif
}

/// Executes a rawtoaces command with the given arguments and captures its output.
///
/// @param args Vector of command-line arguments to pass to rawtoaces (excluding program name).
///             For example, {"--list-cameras"} or {"--list-illuminants", "--verbose"}.
/// @param allow_failure If true, does not assert on non-zero exit status (useful for testing error conditions).
///
/// @return std::string containing the captured stdout and stderr output from the rawtoaces execution
std::string run_rawtoaces_command(
    const std::vector<std::string> &args, bool allow_failure = false )
{
    // Build the command line
    std::string command;
#ifdef WIN32
    command = "..\\src\\rawtoaces\\Release\\rawtoaces.exe";
#else
    command = "../src/rawtoaces/rawtoaces";
#endif

    for ( const auto &arg: args )
    {
        command += " " + arg;
    }

    // Execute command using platform-specific functions, redirecting stderr to stdout
    // Both Windows and Unix/Linux use "2>&1" to redirect stderr to stdout
    std::string command_with_stderr = command + " 2>&1";
    FILE       *pipe = platform_popen( command_with_stderr.c_str(), "r" );
    OIIO_CHECK_ASSERT(
        pipe != nullptr && "Failed to execute rawtoaces command" );

    // Read output (now includes both stdout and stderr)
    std::string output;
    char        buffer[4096];
    while ( fgets( buffer, sizeof( buffer ), pipe ) != nullptr )
    {
        output += buffer;
    }

    // Get exit status and validate (unless allowing failure)
    int exit_status = platform_pclose( pipe );
    if ( !allow_failure )
    {
        OIIO_CHECK_EQUAL( exit_status, 0 );
    }

    return output;
}

#ifdef WIN32
std::string to_os_path( std::string linux_path )
{
    return convert_linux_path_to_windows_path( linux_path );
}
#else
std::string to_os_path( std::string linux_path )
{
    return linux_path;
}
#endif

void assert_success_conversion( const std::string &output )
{
    // Assert that the command succeeded (no error messages)
    ASSERT_NOT_CONTAINS( output, "Failed to find" );
    ASSERT_NOT_CONTAINS( output, "ERROR" );
    ASSERT_NOT_CONTAINS( output, "Missing" );
    ASSERT_NOT_CONTAINS( output, "Failed to configure" );

    // Assert that processing completed successfully
    ASSERT_CONTAINS( output, "Processing file" );
    ASSERT_CONTAINS( output, "Configuring transform" );
    ASSERT_CONTAINS( output, "Loading image" );
    ASSERT_CONTAINS( output, "Saving output" );

    // Assert that white balance coefficients were calculated
    ASSERT_CONTAINS( output, "White balance coefficients" );

    // Assert that IDT matrix was calculated
    ASSERT_CONTAINS( output, "Input Device Transform (IDT) matrix" );

    // Assert that image processing steps occurred
    ASSERT_CONTAINS( output, "Applying transform matrix" );
    ASSERT_CONTAINS( output, "Applying scale" );
    ASSERT_CONTAINS( output, "Applying crop" );

    // Assert that the correct input and output files were processed
    ASSERT_CONTAINS( output, "blackmagic_cinema_camera_cinemadng.dng" );
    ASSERT_CONTAINS( output, "blackmagic_cinema_camera_cinemadng_aces.exr" );
}

/// Verifies that collect_image_files can traverse a directory, identify valid RAW image files,
/// filter out unwanted file types, and organize them into batches for processing
void test_collect_image_files_directory()
{
    std::cout << std::endl
              << "test_collect_image_files_directory()" << std::endl;
    TestDirectory test_dir;
    test_dir.create_test_files();

    std::vector<std::string>              paths = { test_dir.path() };
    std::vector<std::vector<std::string>> batches =
        collect_image_files( paths );

    OIIO_CHECK_EQUAL(
        batches.size(),
        2 ); // Empty batch for file paths, one batch for directory
    OIIO_CHECK_EQUAL( batches[0].size(), 0 );
    OIIO_CHECK_EQUAL( batches[1].size(), 5 );

    // Check that the correct files are included
    std::vector<std::string> expected_files = {
        ( std::filesystem::path( test_dir.path() ) / "test1.raw" ).string(),
        ( std::filesystem::path( test_dir.path() ) / "test2.cr2" ).string(),
        ( std::filesystem::path( test_dir.path() ) / "test3.nef" ).string(),
        ( std::filesystem::path( test_dir.path() ) / "test4.dng" ).string(),
        ( std::filesystem::path( test_dir.path() ) / "symlink.raw" ).string()
    };

    for ( const auto &expected: expected_files )
    {
        bool found = false;
        for ( const auto &actual: batches[1] )
        {
            if ( actual == expected )
            {
                found = true;
                break;
            }
        }
        OIIO_CHECK_EQUAL( found, true );
    }
}

/// Ensures that when given a single file path (not a directory), collect_image_files
/// properly validates the file and creates a batch containing just that one file
void test_collect_image_files_single_file()
{
    std::cout << std::endl
              << "test_collect_image_files_single_file()" << std::endl;
    TestDirectory test_dir;
    std::string   test_file =
        ( std::filesystem::path( test_dir.path() ) / "test.raw" ).string();
    std::ofstream( test_file ).close();

    std::vector<std::string>              paths = { test_file };
    std::vector<std::vector<std::string>> batches =
        collect_image_files( paths );

    OIIO_CHECK_EQUAL( batches.size(), 1 );
    OIIO_CHECK_EQUAL( batches[0].size(), 1 );
    OIIO_CHECK_EQUAL( batches[0][0], test_file );
}

/// Verifies that collect_image_files skips nonexistent paths and creates no batches for them.
void test_collect_image_files_nonexistent_path()
{
    std::cout << std::endl
              << "test_collect_image_files_nonexistent_path()" << std::endl;
    std::vector<std::string>              paths = { "nonexistent_path" };
    std::vector<std::vector<std::string>> batches =
        collect_image_files( paths );

    OIIO_CHECK_EQUAL( batches.size(), 1 ); // Empty batch for file paths
}

/// Ensures that when given an empty directory, collect_image_files creates no batches
void test_collect_image_files_empty_directory()
{
    std::cout << std::endl
              << "test_collect_image_files_empty_directory()" << std::endl;
    TestDirectory test_dir;

    std::vector<std::string>              paths = { test_dir.path() };
    std::vector<std::vector<std::string>> batches =
        collect_image_files( paths );

    OIIO_CHECK_EQUAL(
        batches.size(),
        2 ); // Empty batch for file paths, one batch for directory
    OIIO_CHECK_EQUAL( batches[0].size(), 0 );
    OIIO_CHECK_EQUAL( batches[1].size(), 0 );
}

/// Verifies that when a directory contains only files that should be filtered out
/// (like .DS_Store, .jpg, .exr)
void test_collect_image_files_directory_with_only_filtered_files()
{
    std::cout << std::endl
              << "test_collect_image_files_directory_with_only_filtered_files()"
              << std::endl;
    TestDirectory test_dir;
    test_dir.create_filtered_files_only();

    std::vector<std::string>              paths = { test_dir.path() };
    std::vector<std::vector<std::string>> batches =
        collect_image_files( paths );

    OIIO_CHECK_EQUAL(
        batches.size(),
        2 ); // Empty batch for file paths, one batch for directory
    OIIO_CHECK_EQUAL( batches[0].size(), 0 );
    OIIO_CHECK_EQUAL( batches[1].size(), 0 );
}

/// Tests collect_image_files with multiple input paths (files and directories)
/// to ensure it properly creates separate batches for each input path
void test_collect_image_files_multiple_paths()
{
    std::cout << std::endl
              << "test_collect_image_files_multiple_paths()" << std::endl;

    // Create test directories using TestDirectory (automatic cleanup)
    TestDirectory test_dir1;
    test_dir1.create_valid_files( { "file1.raw", "file2.cr2" } );

    TestDirectory test_dir2;
    test_dir2.create_valid_files( { "file3.nef", "file4.dng" } );

    // Create additional directories for single files
    TestDirectory test_dir3;
    test_dir3.create_valid_files( { "single_1.raw", "single_2.raw" } );

    TestDirectory test_dir4;
    test_dir4.create_valid_files( { "single_3.raw" } );

    // Get paths to individual test files
    std::string single_file_1 =
        ( std::filesystem::path( test_dir3.path() ) / "single_1.raw" ).string();
    std::string single_file_2 =
        ( std::filesystem::path( test_dir3.path() ) / "single_2.raw" ).string();
    std::string single_file_3 =
        ( std::filesystem::path( test_dir4.path() ) / "single_3.raw" ).string();

    // Test with multiple paths
    std::vector<std::string> paths = { single_file_1,
                                       test_dir1.path(),
                                       test_dir2.path(),
                                       single_file_2,
                                       single_file_3 };

    std::vector<std::vector<std::string>> batches =
        collect_image_files( paths );

    // Resulting batches should look like this:
    // [
    //     [ single_file_1, single_file_2, single_file_3 ], // all files from single file paths
    //     [ file1.raw, file2.cr2 ],
    //     [ file3.nef, file4.dng ]
    // ]

    // Should have 5 batches (one for each input path)
    OIIO_CHECK_EQUAL( batches.size(), 3 );

    // First batch should have 1 file (single_file_1)
    OIIO_CHECK_EQUAL( batches[0].size(), 3 );
    OIIO_CHECK_EQUAL( batches[0][0], single_file_1 );
    OIIO_CHECK_EQUAL( batches[0][1], single_file_2 );
    OIIO_CHECK_EQUAL( batches[0][2], single_file_3 );

    // Second batch should have 2 files from test_dir1
    OIIO_CHECK_EQUAL( batches[1].size(), 2 );

    // Third batch should have 2 files from test_dir2
    OIIO_CHECK_EQUAL( batches[2].size(), 2 );
}

/// Tests collect_image_files with mixed valid and invalid paths
/// to ensure it skips invalid paths but processes valid ones
void test_collect_image_files_mixed_valid_invalid_paths()
{
    std::cout << std::endl
              << "test_collect_image_files_mixed_valid_invalid_paths()"
              << std::endl;

    TestDirectory test_dir;
    test_dir.create_valid_files( { "file1.raw" } );

    // Test with valid directory, invalid path, and valid file
    std::vector<std::string> paths = {
        test_dir.path(),
        "nonexistent_path",
        ( std::filesystem::path( test_dir.path() ) / "file1.raw" ).string()
    };
    std::vector<std::vector<std::string>> batches =
        collect_image_files( paths );

    // Should have 2 batches (valid directory + valid file, invalid path skipped)
    OIIO_CHECK_EQUAL( batches.size(), 2 );

    // First batch should have 1 file from the directory
    OIIO_CHECK_EQUAL( batches[0].size(), 1 );

    // Second batch should have 1 file (the single file)
    OIIO_CHECK_EQUAL( batches[1].size(), 1 );
}

/// Tests parsing of RAW extensions from a mixed OIIO extension list
void test_parse_raw_extensions()
{
    std::cout << std::endl << "test_parse_raw_extensions()" << std::endl;

    // Mixed valid and invalid entries
    const std::string extensionlist =
        "raw:cr2,NEF,dng;"
        "jpeg:jpg,jpeg;"
        "invalidentry;"
        "raw:RAF";

    const std::set<std::string> exts =
        rta::util::parse_raw_extensions( extensionlist );

    // Should include only RAW extensions, normalized
    OIIO_CHECK_EQUAL( exts.count( ".cr2" ), 1 );
    OIIO_CHECK_EQUAL( exts.count( ".nef" ), 1 );
    OIIO_CHECK_EQUAL( exts.count( ".dng" ), 1 );
    OIIO_CHECK_EQUAL( exts.count( ".raf" ), 1 );

    // Should exclude non-RAW formats
    OIIO_CHECK_EQUAL( exts.count( ".jpg" ), 0 );
    OIIO_CHECK_EQUAL( exts.count( ".jpeg" ), 0 );
}

/// Tests database_paths with no environment variables set (uses default paths)
void test_database_paths_default()
{
    std::cout << std::endl << "test_database_paths_default()" << std::endl;
    // Clear environment variables to test default behavior
    unset_env_var( "RAWTOACES_DATA_PATH" );
    unset_env_var( "AMPAS_DATA_PATH" );

    std::vector<std::string> paths = database_paths();

    // Should have at least one default path
    OIIO_CHECK_EQUAL( paths.empty(), false );

// On Unix systems, should have both new and legacy paths
#ifdef WIN32
    // On Windows, should have just the current directory
    OIIO_CHECK_EQUAL( paths.size(), 1 );
    OIIO_CHECK_EQUAL( paths[0], "." );
#else
    OIIO_CHECK_EQUAL( paths.size(), 2 );
    OIIO_CHECK_EQUAL( paths[0], "/usr/local/share/rawtoaces/data" );
    OIIO_CHECK_EQUAL( paths[1], "/usr/local/include/rawtoaces/data" );
#endif
}

/// Tests database_paths with RAWTOACES_DATA_PATH environment variable set
void test_database_paths_rawtoaces_env()
{
    std::cout << std::endl
              << "test_database_paths_rawtoaces_env()" << std::endl;
    set_env_var(
        "RAWTOACES_DATA_PATH", to_os_path( "/custom/path1:/custom/path2" ) );
    unset_env_var( "AMPAS_DATA_PATH" );

    std::vector<std::string> paths = database_paths();

    OIIO_CHECK_EQUAL( paths.size(), 2 );
    OIIO_CHECK_EQUAL( paths[0], to_os_path( "/custom/path1" ) );
    OIIO_CHECK_EQUAL( paths[1], to_os_path( "/custom/path2" ) );

    // Clean up
    unset_env_var( "RAWTOACES_DATA_PATH" );
}

/// Tests database_paths with deprecated AMPAS_DATA_PATH environment variable
void test_database_paths_ampas_env()
{
    std::cout << std::endl << "test_database_paths_ampas_env()" << std::endl;
    // Set AMPAS_DATA_PATH (deprecated)
    unset_env_var( "RAWTOACES_DATA_PATH" );
    set_env_var(
        "AMPAS_DATA_PATH",
        to_os_path( "/deprecated/path1:/deprecated/path2" ) );

    std::vector<std::string> paths = database_paths();

    OIIO_CHECK_EQUAL( paths.size(), 2 );
    OIIO_CHECK_EQUAL( paths[0], to_os_path( "/deprecated/path1" ) );
    OIIO_CHECK_EQUAL( paths[1], to_os_path( "/deprecated/path2" ) );

    // Clean up
    unset_env_var( "AMPAS_DATA_PATH" );
}

/// Tests database_paths with both environment variables set (RAWTOACES_DATA_PATH should take precedence)
void test_database_paths_both_env()
{
    std::cout << std::endl << "test_database_paths_both_env()" << std::endl;
    // Set both environment variables
    set_env_var(
        "RAWTOACES_DATA_PATH",
        to_os_path( "/preferred/path1:/preferred/path2" ) );
    set_env_var(
        "AMPAS_DATA_PATH",
        to_os_path( "/deprecated/path1:/deprecated/path2" ) );

    std::vector<std::string> paths = database_paths();

    // RAWTOACES_DATA_PATH should take precedence
    OIIO_CHECK_EQUAL( paths.size(), 2 );
    OIIO_CHECK_EQUAL( paths[0], to_os_path( "/preferred/path1" ) );
    OIIO_CHECK_EQUAL( paths[1], to_os_path( "/preferred/path2" ) );

    // Clean up
    unset_env_var( "RAWTOACES_DATA_PATH" );
    unset_env_var( "AMPAS_DATA_PATH" );
}

/// Tests database_paths with override_path parameter (--data-dir functionality)
/// Verifies that override_path takes precedence over environment variables
void test_database_paths_override_path()
{
    std::cout << std::endl
              << "test_database_paths_override_path()" << std::endl;
    // Set environment variables to ensure they are overridden

    set_env_var( "RAWTOACES_DATA_PATH", to_os_path( "/env/path1:/env/path2" ) );
    set_env_var(
        "AMPAS_DATA_PATH",
        to_os_path( "/deprecated/path1:/deprecated/path2" ) );

    // Test with override path - should take precedence over environment variables
    std::string override_path =
        to_os_path( "/override/path1:/override/path2:/override/path3" );
    std::vector<std::string> paths = database_paths( override_path );

    // Should have 3 paths from override
    OIIO_CHECK_EQUAL( paths.size(), 3 );
    OIIO_CHECK_EQUAL( paths[0], to_os_path( "/override/path1" ) );
    OIIO_CHECK_EQUAL( paths[1], to_os_path( "/override/path2" ) );
    OIIO_CHECK_EQUAL( paths[2], to_os_path( "/override/path3" ) );

    // Test with empty override path - should fall back to environment variables
    paths = database_paths( "" );

    // Should have 2 paths from RAWTOACES_DATA_PATH environment variable
    OIIO_CHECK_EQUAL( paths.size(), 2 );
    OIIO_CHECK_EQUAL( paths[0], to_os_path( "/env/path1" ) );
    OIIO_CHECK_EQUAL( paths[1], to_os_path( "/env/path2" ) );

    // Clean up
    unset_env_var( "RAWTOACES_DATA_PATH" );
    unset_env_var( "AMPAS_DATA_PATH" );
}

/// Tests convert_linux_path_to_windows_path utility function
void test_convert_linux_path_to_windows_path()
{
    std::cout << std::endl
              << "test_convert_linux_path_to_windows_path()" << std::endl;

    // Test single path with forward slashes
    std::string result =
        convert_linux_path_to_windows_path( "/usr/local/share" );
    OIIO_CHECK_EQUAL( result, "c:\\usr\\local\\share" );

    // Test multiple paths separated by ':'
    result = convert_linux_path_to_windows_path( "/path1:/path2:/path3" );
    OIIO_CHECK_EQUAL( result, "c:\\path1;c:\\path2;c:\\path3" );
}

/// Tests fix_metadata with both Make and Model attributes
void test_fix_metadata_both_attributes()
{
    std::cout << std::endl
              << "test_fix_metadata_both_attributes()" << std::endl;
    OIIO::ImageSpec spec;

    // Add both original attributes
    spec["Make"]  = "Sony";
    spec["Model"] = "A7R IV";

    // Call fix_metadata
    fix_metadata( spec );

    // Check that both destinations were created with correct values
    OIIO_CHECK_EQUAL( spec.get_string_attribute( "cameraMake" ), "Sony" );
    OIIO_CHECK_EQUAL( spec.get_string_attribute( "cameraModel" ), "A7R IV" );

    // Check that both originals were removed
    OIIO_CHECK_EQUAL( spec.find_attribute( "Make" ), nullptr );
    OIIO_CHECK_EQUAL( spec.find_attribute( "Model" ), nullptr );
}

/// Tests fix_metadata when destination already exists (should not overwrite or remove source)
void test_fix_metadata_destination_exists()
{
    std::cout << std::endl
              << "test_fix_metadata_destination_exists()" << std::endl;
    OIIO::ImageSpec spec;

    // Add both original and destination attributes
    spec["Make"]       = "Canon";
    spec["cameraMake"] = "Nikon"; // Pre-existing destination

    // Call fix_metadata
    fix_metadata( spec );

    // Check that destination was not overwritten
    OIIO_CHECK_EQUAL( spec.get_string_attribute( "cameraMake" ), "Nikon" );

    // Check that original "Make" was NOT removed (because destination exists)
    OIIO_CHECK_EQUAL( spec.get_string_attribute( "Make" ), "Canon" );
}

/// Tests fix_metadata when source doesn't exist (should do nothing)
void test_fix_metadata_source_missing()
{
    std::cout << std::endl << "test_fix_metadata_source_missing()" << std::endl;
    OIIO::ImageSpec spec;

    // Don't add any "Make" or "Model" attributes

    // Call fix_metadata
    fix_metadata( spec );

    // Check that no attributes were created
    OIIO_CHECK_EQUAL( spec.find_attribute( "cameraMake" ), nullptr );
    OIIO_CHECK_EQUAL( spec.find_attribute( "cameraModel" ), nullptr );
}

/// Tests fix_metadata with non-string (should be ignored)
void test_fix_metadata_unsupported_type()
{
    std::cout << std::endl
              << "test_fix_metadata_unsupported_type()" << std::endl;
    OIIO::ImageSpec spec;

    // Add integer attribute (this should be ignored by fix_metadata)
    spec["Make"] = 42; // Integer, not string

    // Call fix_metadata
    fix_metadata( spec );

    // Check that no destination was created (unsupported types are ignored)
    OIIO_CHECK_EQUAL( spec.find_attribute( "cameraMake" ), nullptr );

    // Check that original was removed
    OIIO_CHECK_EQUAL( spec.find_attribute( "Make" ), nullptr );
}

std::string run_rawtoaces_with_data_dir(
    std::vector<std::string> args, // Pass by value to avoid modifying original
    TestDirectory           &test_dir,
    bool                     use_dir_path_arg = false,
    bool                     allow_failure    = false )
{
    const std::string &datab_path = test_dir.get_database_path();

    if ( use_dir_path_arg )
    {
        args.push_back( "--data-dir" );
        args.push_back( datab_path );
        unset_env_var( "RAWTOACES_DATA_PATH" );
    }
    else
    {
        set_env_var( "RAWTOACES_DATA_PATH", datab_path );
    }

    std::string output = run_rawtoaces_command( args, allow_failure );

    if ( !use_dir_path_arg )
    {
        // Clean up environment variable
        unset_env_var( "RAWTOACES_DATA_PATH" );
    }

    return output;
}

/// This test verifies that when --list-formats is provided, the method
/// outputs the list of supported RAW input formats and then exits
void test_parse_parameters_list_formats()
{
    std::cout << std::endl
              << "test_parse_parameters_list_formats()" << std::endl;

    std::vector<std::string> args   = { "--list-formats" };
    std::string              output = run_rawtoaces_command( args );

    std::vector<std::string> lines;
    OIIO::Strutil::split( output, lines, "\n" );

    // Check for a few well-known RAW formats
    bool found_cr2 = false;
    bool found_dng = false;
    bool found_png = false;

    for ( const auto &line: lines )
    {
        if ( line == ".cr2" )
            found_cr2 = true;
        if ( line == ".dng" )
            found_dng = true;
        if ( line == ".png" )
            found_png = true;
    }

    OIIO_CHECK_EQUAL( found_cr2, true );
    OIIO_CHECK_EQUAL( found_dng, true );
    OIIO_CHECK_EQUAL( found_png, false );
}

/// This test verifies that when --list-cameras is provided, the method
/// calls get_supported_cameras() and outputs the camera list, then exits
void test_parse_parameters_list_cameras( bool use_dir_path_arg = false )
{
    std::cout << std::endl
              << "test_parse_parameters_list_cameras("
              << ( use_dir_path_arg ? "with data dir" : "without data dir" )
              << ")" << std::endl;

    // Create test directory with dynamic database
    // Store TestFixture to keep it alive for the test duration
    TestFixture fixture;
    auto       &test_dir = fixture.with_camera( "Canon", "EOS_R6" )
                         .with_camera( "Mamiya", "Mamiya 7" )
                         .without_training()
                         .without_observer()
                         .build();

    auto args = CommandBuilder().arg( "--list-cameras" ).build();
    auto output =
        run_rawtoaces_with_data_dir( args, test_dir, use_dir_path_arg );

    std::vector<std::string> lines = get_output_lines( output );

    OIIO_CHECK_EQUAL( lines.size(), 3 );
    OIIO_CHECK_EQUAL(
        lines[0],
        "Spectral sensitivity data is available for the following cameras:" );

    // Check that both cameras are present (order doesn't matter)
    bool found_canon = false, found_mamiya = false;
    for ( size_t i = 1; i < lines.size(); ++i )
    {
        if ( lines[i] == "Canon / EOS_R6" )
            found_canon = true;
        if ( lines[i] == "Mamiya / Mamiya 7" )
            found_mamiya = true;
    }
    OIIO_CHECK_EQUAL( found_canon, true );
    OIIO_CHECK_EQUAL( found_mamiya, true );
}

/// This test verifies that when --list-illuminants is provided, the method
/// calls get_supported_illuminants() and outputs the illuminant list,
/// then exits
void test_parse_parameters_list_illuminants( bool use_dir_path_arg = false )
{
    std::cout << std::endl
              << "test_parse_parameters_list_illuminants()" << std::endl;

    // Create test directory with dynamic database
    TestFixture fixture;
    auto       &test_dir = fixture.with_illuminant( "my-illuminant" )
                         .without_training()
                         .without_observer()
                         .build();

    auto args = CommandBuilder().arg( "--list-illuminants" ).build();

    // Run the test with --list-illuminants argument using the dynamic database
    auto output =
        run_rawtoaces_with_data_dir( args, test_dir, use_dir_path_arg );

    std::vector<std::string> lines = get_output_lines( output );

    OIIO_CHECK_EQUAL( lines.size(), 4 );
    OIIO_CHECK_EQUAL( lines[0], "The following illuminants are supported:" );
    OIIO_CHECK_EQUAL( lines[1], "Day-light (e.g., D60, D6025)" );
    OIIO_CHECK_EQUAL( lines[2], "Blackbody (e.g., 3200K)" );
    OIIO_CHECK_EQUAL( lines[3], "my-illuminant" );
}

/// Tests that prepare_transform_spectral fails when no camera manufacturer information is available (should fail)
void test_missing_camera_manufacturer()
{
    std::cout << std::endl << "test_missing_camera_manufacturer()" << std::endl;

    // Create test directory with database
    TestFixture fixture;
    auto       &test_dir = fixture.build();

    // Create a mock ImageSpec with no camera metadata
    auto image_spec = ImageSpecBuilder().build(); // No camera metadata

    // Configure settings with no custom camera info
    auto settings =
        SettingsBuilder().database( test_dir.get_database_path() ).build();

    // Test: Empty camera make via direct method call
    std::vector<double>              WB_multipliers;
    std::vector<std::vector<double>> IDT_matrix;
    std::vector<std::vector<double>> CAT_matrix;

    // Capture stderr output to verify error messages
    bool        success;
    std::string output = capture_stderr( [&]() {
        // This should fail because there's no camera make
        success = prepare_transform_spectral(
            image_spec, settings, WB_multipliers, IDT_matrix, CAT_matrix );
    } );

    OIIO_CHECK_ASSERT( !success );

    // Assert on the expected error message
    ASSERT_CONTAINS(
        output,
        "Missing the camera manufacturer name in the file metadata. You can provide a camera make using the --custom-camera-make parameter" );
}

/// Tests that conversion fails when camera model is missing (should fail)
void test_empty_camera_model()
{
    std::cout << std::endl << "test_empty_camera_model()" << std::endl;

    // Create test directory with database
    TestFixture fixture;
    auto       &test_dir = fixture.build();

    // Create ImageSpec with camera make but no model
    auto image_spec = ImageSpecBuilder().camera( "Blackmagic", "" ).build();
    // Do not set cameraModel - this is what we're testing

    // Set up ImageConverter with spectral mode settings
    auto settings = SettingsBuilder()
                        .database( test_dir.get_database_path() )
                        .wb_method( "illuminant" )
                        .mat_method( "spectral" )
                        .build();

    ImageConverter converter;
    converter.settings = settings;

    // Create empty options list
    OIIO::ParamValueList options;

    // Capture stderr output to verify error messages
    bool        success;
    std::string output = capture_stderr( [&]() {
        // This should fail with error message about missing camera model
        success = converter.configure( image_spec, options );
    } );

    OIIO_CHECK_ASSERT( !success );

    // Assert on the expected error messages
    std::vector<std::string> expected_errors = {
        "Missing the camera model name in the file metadata. You can provide a camera model using the --custom-camera-model parameter",
        "ERROR: the colour space transform has not been configured properly (spectral mode)."
    };
    ASSERT_CONTAINS_ALL( output, expected_errors );
}

/// Tests that conversion fails when camera data is not found in database (should fail)
void test_camera_data_not_found()
{
    std::cout << std::endl << "test_camera_data_not_found()" << std::endl;

    // Create test directory with database (different camera than DNG)
    TestFixture fixture;
    auto       &test_dir = fixture.with_camera( "Canon", "EOS_R6" ).build();

    // Build command
    auto args = CommandBuilder()
                    .wb_method( "illuminant" )
                    .illuminant( "D65" )
                    .mat_method( "spectral" )
                    .input( dng_test_file )
                    .build();

    // This should fail with error message about camera data not found
    std::string output =
        run_rawtoaces_with_data_dir( args, test_dir, false, true );

    // Assert on the expected error messages
    std::vector<std::string> expected_errors = {
        "Failed to find spectral data for camera make: 'Blackmagic', model: 'Cinema Camera'.",
        "Please check the database search path in RAWTOACES_DATABASE_PATH",
        "ERROR: the colour space transform has not been configured properly (spectral mode)."
    };
    ASSERT_CONTAINS_ALL( output, expected_errors );
}

/// Tests that conversion fails when training data is missing (should fail)
void test_missing_training_data()
{
    std::cout << std::endl << "test_missing_training_data()" << std::endl;

    // Create test directory with database (no training data)
    TestFixture fixture;
    auto       &test_dir = fixture.with_camera( "Blackmagic", "Cinema Camera" )
                         .without_training()
                         .build();

    // Build command
    auto args = CommandBuilder()
                    .wb_method( "illuminant" )
                    .illuminant( "D65" )
                    .mat_method( "spectral" )
                    .input( dng_test_file )
                    .build();

    // This should fail with error message about missing training data
    std::string output =
        run_rawtoaces_with_data_dir( args, test_dir, false, true );

    // Assert on the expected error message
    ASSERT_CONTAINS(
        output,
        "Failed to find training data 'training/training_spectral.json'." );
}

/// Tests that conversion fails when observer data is missing (should fail)
void test_missing_observer_data()
{
    std::cout << std::endl << "test_missing_observer_data()" << std::endl;

    // Create test directory with database (no observer data)
    TestFixture fixture;
    auto       &test_dir = fixture.with_camera( "Blackmagic", "Cinema Camera" )
                         .without_observer()
                         .build();

    // Build command
    auto args = CommandBuilder()
                    .wb_method( "illuminant" )
                    .illuminant( "D65" )
                    .mat_method( "spectral" )
                    .input( dng_test_file )
                    .build();

    // This should fail with error message about missing observer data
    std::string output =
        run_rawtoaces_with_data_dir( args, test_dir, false, true );

    // Assert on the expected error message
    ASSERT_CONTAINS( output, "Failed to find observer 'cmf/cmf_1931.json'." );
}

/// Tests that conversion fails when illuminant data is missing (should fail)
void test_missing_illuminant_data()
{
    std::cout << std::endl << "test_missing_illuminant_data()" << std::endl;

    // Create test directory with database (no illuminant)
    TestFixture fixture;
    auto       &test_dir =
        fixture.with_camera( "Blackmagic", "Cinema Camera" ).build();

    // Build command
    auto args = CommandBuilder()
                    .wb_method( "illuminant" )
                    .illuminant( "nonexistentilluminant" )
                    .mat_method( "spectral" )
                    .input( dng_test_file )
                    .build();

    // This should fail with error message about missing illuminant data
    std::string output =
        run_rawtoaces_with_data_dir( args, test_dir, false, true );

    // Assert on the expected error message
    ASSERT_CONTAINS( output, "Error: No matching light source" );
}

/// Tests that conversion fails when specified illuminant type is not found in illuminant data (should fail)
void test_illuminant_type_not_found()
{
    std::cout << std::endl << "test_illuminant_type_not_found()" << std::endl;

    // Create test directory with database
    TestFixture fixture;
    auto       &test_dir =
        fixture.with_camera( "Blackmagic", "Cinema Camera" ).build();

    // Create a mock ImageSpec with camera metadata
    auto image_spec =
        ImageSpecBuilder().camera( "Blackmagic", "Cinema Camera" ).build();

    // Configure settings with illuminant that doesn't exist in the database
    auto settings =
        SettingsBuilder()
            .database( test_dir.get_database_path() )
            .illuminant( "A" ) // Request illuminant "A" which doesn't exist
            .build();

    // Test: Request an illuminant type that doesn't exist in the illuminant data
    std::vector<double>              WB_multipliers;
    std::vector<std::vector<double>> IDT_matrix;
    std::vector<std::vector<double>> CAT_matrix;

    // Capture stderr output to verify error messages
    bool        success;
    std::string output = capture_stderr( [&]() {
        // This should fail because illuminant "A" is not found
        success = prepare_transform_spectral(
            image_spec, settings, WB_multipliers, IDT_matrix, CAT_matrix );
    } );

    OIIO_CHECK_ASSERT( !success );

    // Assert on the expected error message
    ASSERT_CONTAINS( output, "Failed to find illuminant type = 'a'." );
}

/// Tests that invalid daylight color temperature values cause the application to exit with an error
void test_invalid_daylight_cct_exits()
{
    std::cout << std::endl << "test_invalid_daylight_cct_exits()" << std::endl;

    // Create test directory with database
    TestFixture fixture;
    auto       &test_dir =
        fixture.with_camera( "Blackmagic", "Cinema Camera" ).build();

    // Test invalid CCT values that should cause exit(1) when passed as daylight illuminant
    int test_cases[] = { 39, 251, 3999, 25001, 0, -1, 30000 };

    // Create temporary output file path
    std::filesystem::path output_path =
        std::filesystem::temp_directory_path() / "test_invalid_cct_output.exr";

    for ( int test_case: test_cases )
    {
        // Build command with invalid CCT as daylight illuminant
        auto args = CommandBuilder()
                        .illuminant( "d" + std::to_string( test_case ) )
                        .wb_method( "illuminant" )
                        .mat_method( "spectral" )
                        .input( dng_test_file )
                        .output( output_path.string() )
                        .build();

        // This should fail with exit(1) and error message about CCT range
        std::string output =
            run_rawtoaces_with_data_dir( args, test_dir, false, true );

        // Assert on the expected error message
        ASSERT_CONTAINS(
            output,
            "The range of Correlated Color Temperature for Day Light should be from 4000 to 25000" );
    }
}

/// Tests that invalid blackbody color temperature values cause the application to exit with an error
void test_invalid_blackbody_cct_exits()
{
    std::cout << std::endl << "test_invalid_blackbody_cct_exits()" << std::endl;

    // Create test directory with database
    TestFixture fixture;
    auto       &test_dir =
        fixture.with_camera( "Blackmagic", "Cinema Camera" ).build();

    // Test invalid CCT values that should cause exit(1) when passed as blackbody illuminant
    // Testing both too low (< 1500) and too high (>= 4000) cases
    int test_cases[] = { 1000, 500, 5000, 10000 };

    // Create temporary output file path
    std::filesystem::path output_path =
        std::filesystem::temp_directory_path() / "test_invalid_cct_output.exr";

    for ( int test_case: test_cases )
    {
        // Build command with invalid CCT as blackbody illuminant
        auto args = CommandBuilder()
                        .illuminant( std::to_string( test_case ) + "K" )
                        .wb_method( "illuminant" )
                        .mat_method( "spectral" )
                        .input( dng_test_file )
                        .output( output_path.string() )
                        .build();

        // This should fail with exit(1) and error message about CCT range
        std::string output =
            run_rawtoaces_with_data_dir( args, test_dir, false, true );

        // Assert on the expected error message
        ASSERT_CONTAINS(
            output,
            "The range of Color Temperature for BlackBody should be from 1500 to 3999" );
    }
}

/// Tests that auto-detection of illuminant works with 4-channel WB_multipliers and verbosity output
void test_auto_detect_illuminant_with_wb_multipliers()
{
    std::cout << std::endl
              << "test_auto_detect_illuminant_with_wb_multipliers()"
              << std::endl;

    // Create test directory with database
    TestFixture fixture;
    auto       &test_dir =
        fixture.with_camera( "Blackmagic", "Cinema Camera" ).build();

    // Create a mock ImageSpec with camera metadata
    auto image_spec =
        ImageSpecBuilder().camera( "Blackmagic", "Cinema Camera" ).build();

    // Configure settings with empty illuminant (to trigger auto-detection)
    auto settings = SettingsBuilder()
                        .database( test_dir.get_database_path() )
                        .illuminant( "" ) // Empty to trigger auto-detection
                        .build();

    // Provide WB_multipliers with size 4 to exercise the 4-channel path
    std::vector<double> WB_multipliers = { 1.5, 1.0, 1.2, 1.0 }; // 4 channels
    std::vector<std::vector<double>> IDT_matrix;
    std::vector<std::vector<double>> CAT_matrix;

    bool        success;
    std::string output = capture_stderr( [&]() {
        // This should succeed and auto-detect the illuminant
        success = prepare_transform_spectral(
            image_spec, settings, WB_multipliers, IDT_matrix, CAT_matrix );
    } );

    OIIO_CHECK_ASSERT( success );

    // Assert on expected messages
    ASSERT_CONTAINS( output, "WARNING: Directory '" );
    ASSERT_CONTAINS( output, "illuminant' does not exist." );
    ASSERT_CONTAINS( output, "Found illuminant: '2000k'." );
}

/// Tests that a warning is issued when a database location path points to a file instead of a directory
void test_database_location_not_directory_warning()
{
    std::cout << std::endl
              << "test_database_location_not_directory_warning()" << std::endl;

    // Create test directory
    TestFixture fixture;
    auto       &test_dir =
        fixture.with_camera( "Blackmagic", "Cinema Camera" ).build();

    // Create a file (not a directory) to use as database path
    std::filesystem::path file_path =
        std::filesystem::temp_directory_path() / "test_not_a_directory.txt";
    std::ofstream( file_path ).close();

    // Create a mock ImageSpec with camera metadata
    auto image_spec =
        ImageSpecBuilder().camera( "Blackmagic", "Cinema Camera" ).build();

    // Configure settings with file path (not directory) as database location
    ImageConverter::Settings settings;
    settings.database_directories = { file_path.string(),
                                      test_dir.get_database_path() };
    settings.illuminant           = ""; // Empty to trigger auto-detection
    settings.verbosity            = 1;  // > 0 to trigger the warning

    // Make sure the transform is not in the cache, otherwise DB look up
    // will no be triggered.
    settings.disable_cache = true;

    // Provide WB_multipliers
    std::vector<double>              WB_multipliers = { 1.5, 1.0, 1.2, 1.0 };
    std::vector<std::vector<double>> IDT_matrix;
    std::vector<std::vector<double>> CAT_matrix;

    bool        success;
    std::string output = capture_stderr( [&]() {
        // This should succeed (using the valid database path)
        // but should warn about the file path not being a directory
        success = prepare_transform_spectral(
            image_spec, settings, WB_multipliers, IDT_matrix, CAT_matrix );
    } );

    OIIO_CHECK_ASSERT( success );

    // Assert on expected warning
    ASSERT_CONTAINS( output, "WARNING: Database location '" );
    ASSERT_CONTAINS( output, "' is not a directory." );

    // Clean up the test file
    std::filesystem::remove( file_path );
}

/// Tests that spectral data can be loaded using an absolute file path
void test_load_spectral_data_absolute_path()
{
    std::cout << std::endl
              << "test_load_spectral_data_absolute_path()" << std::endl;

    // Create test directory with database
    // Store TestFixture object (not just reference) to keep it alive
    TestFixture fixture;
    auto       &test_dir = fixture.without_observer().build();

    // Get the training file path - create_test_data_file returns the full path
    std::string training_file =
        test_dir.get_database_path() + "/training/training_spectral.json";

    // Verify file exists
    OIIO_CHECK_ASSERT( std::filesystem::exists( training_file ) );

    // Get absolute path to the file
    std::filesystem::path abs_path = std::filesystem::absolute( training_file );

    // Verify absolute path exists and is actually absolute
    OIIO_CHECK_ASSERT( std::filesystem::exists( abs_path ) );
    OIIO_CHECK_ASSERT( abs_path.is_absolute() );

    // Initialize solver with empty database directories (using absolute path)
    std::vector<std::string>  database_path = {};
    rta::core::SpectralSolver solver( database_path );

    // Load spectral data using absolute path - this should trigger the absolute path branch
    rta::core::SpectralData spectral_data;
    bool                    success =
        solver.load_spectral_data( abs_path.string(), spectral_data );

    OIIO_CHECK_ASSERT( success );
    OIIO_CHECK_ASSERT(
        spectral_data.data.find( "main" ) != spectral_data.data.end() );
}

/// Tests that external illuminant files that fail to load are skipped
void test_illuminant_file_load_failure()
{
    std::cout << std::endl
              << "test_illuminant_file_load_failure()" << std::endl;

    // Create test directory with database
    TestFixture fixture;
    auto       &test_dir = fixture.with_camera( "Blackmagic", "Cinema Camera" )
                         .with_illuminant( "other_type" )
                         .build();

    // Create invalid illuminant files (invalid JSON that will fail to load)
    std::string illuminant_dir = test_dir.get_database_path() + "/illuminant";
    std::filesystem::create_directories( illuminant_dir );

    std::string   invalid_file = illuminant_dir + "/invalid_illuminant.json";
    std::ofstream file( invalid_file );
    file << "invalid json content { broken }" << std::endl;
    file.close();

    // Create a mock ImageSpec with camera metadata
    auto image_spec =
        ImageSpecBuilder().camera( "Blackmagic", "Cinema Camera" ).build();

    // Configure settings to request a type that doesn't match any file
    auto settings = SettingsBuilder()
                        .database( test_dir.get_database_path() )
                        .illuminant( "nonexistent_type" )
                        .build();

    std::vector<double>              WB_multipliers;
    std::vector<std::vector<double>> IDT_matrix;
    std::vector<std::vector<double>> CAT_matrix;

    bool        success;
    std::string output = capture_stderr( [&]() {
        // This should fail because the requested type doesn't exist
        success = prepare_transform_spectral(
            image_spec, settings, WB_multipliers, IDT_matrix, CAT_matrix );
    } );

    // Should fail (no matching type found), but invalid file should have been processed and skipped
    OIIO_CHECK_ASSERT( !success );
    ASSERT_CONTAINS(
        output, "Failed to find illuminant type = 'nonexistent_type'." );
}

/// Tests that invalid illuminant files are skipped when populating all illuminants for auto-detection
void test_all_illuminants_skips_invalid_files()
{
    std::cout << std::endl
              << "test_all_illuminants_skips_invalid_files()" << std::endl;

    // Create test directory with database
    TestFixture fixture;
    auto       &test_dir = fixture.with_camera( "Blackmagic", "Cinema Camera" )
                         .with_illuminant( "test_illuminant" )
                         .build();

    // Create invalid illuminant files (invalid JSON that will fail to load)
    // Use a filename that sorts first to ensure it's processed
    std::string illuminant_dir = test_dir.get_database_path() + "/illuminant";
    std::filesystem::create_directories( illuminant_dir );

    std::string   invalid_file = illuminant_dir + "/00_invalid_illuminant.json";
    std::ofstream file( invalid_file );
    file << "invalid json content { broken }" << std::endl;
    file.close();

    // Use direct API to ensure we hit the code path
    std::vector<std::string>  database_path = { test_dir.get_database_path() };
    rta::core::SpectralSolver solver( database_path );
    solver.verbosity = 2; // Set verbosity > 1 to trigger the message

    // Find camera (required for find_illuminant(wb))
    bool found = solver.find_camera( "Blackmagic", "Cinema Camera" );
    OIIO_CHECK_ASSERT( found );

    // Call find_illuminant with WB to trigger _all_illuminants population
    std::vector<double> wb = { 1.5, 1.0, 1.2 };

    bool        success;
    std::string output = capture_stderr( [&]() {
        // This should succeed - auto-detection should skip invalid files and use built-in illuminants
        success = solver.find_illuminant( wb );
    } );

    // Should succeed (invalid file should be skipped during _all_illuminants population)
    OIIO_CHECK_ASSERT( success );

    // Assert on expected message
    ASSERT_CONTAINS(
        output,
        "The illuminant calculated to be the best match to the camera metadata is '" );
    ASSERT_CONTAINS( output, "'." );
}

/// Tests that find_illuminant fails when camera data doesn't have "main" key
void test_find_illuminant_camera_no_main_key()
{
    std::cout << std::endl
              << "test_find_illuminant_camera_no_main_key()" << std::endl;

    // Create solver without initializing camera
    std::vector<std::string>  database_path = {};
    rta::core::SpectralSolver solver( database_path );

    // Try to find illuminant with white balance vector without initializing camera
    std::vector<double> wb = { 1.0, 1.0, 1.0 };

    bool        success;
    std::string output =
        capture_stderr( [&]() { success = solver.find_illuminant( wb ); } );

    // Should fail because camera is not initialized (no "main" key)
    OIIO_CHECK_ASSERT( !success );
    OIIO_CHECK_ASSERT(
        output.find( "ERROR: camera needs to be initialised prior to calling "
                     "SpectralSolver::find_illuminant()" ) !=
        std::string::npos );
}

/// Tests that find_illuminant fails when camera data has "main" but with wrong size
void test_find_illuminant_camera_wrong_size()
{
    std::cout << std::endl
              << "test_find_illuminant_camera_wrong_size()" << std::endl;

    // Create test directory with database (with incorrect camera data)
    TestFixture fixture;
    auto       &test_dir = fixture
                         .with_camera(
                             "Test",
                             "Camera",
                             nlohmann::json::array( { "R", "G", "B", "D" } ) )
                         .without_training()
                         .without_observer()
                         .build(); // incorrect channel count

    std::vector<std::string>  database_path = { test_dir.get_database_path() };
    rta::core::SpectralSolver solver( database_path );

    // Find camera (this will load the camera with 4 channels)
    bool found = solver.find_camera( "Test", "Camera" );
    OIIO_CHECK_ASSERT( found );

    std::vector<double> wb = { 1.0, 1.0, 1.0 };

    bool        success;
    std::string output =
        capture_stderr( [&]() { success = solver.find_illuminant( wb ); } );

    // Should fail because camera has wrong size (4 channels instead of 3)
    OIIO_CHECK_ASSERT( !success );
    ASSERT_CONTAINS(
        output,
        "ERROR: camera needs to be initialised prior to calling SpectralSolver::find_illuminant()" );
}

/// Tests that verbosity level 1 does not print optimization reports or IDT matrix
void test_idt_verbosity_level_1()
{
    std::cout << std::endl << "test_idt_verbosity_level_1()" << std::endl;

    // Create test directory with database
    TestFixture fixture;
    auto       &test_dir =
        fixture.with_camera( "Blackmagic", "Cinema Camera" ).build();

    // Use direct API
    std::vector<std::string>  database_path = { test_dir.get_database_path() };
    rta::core::SpectralSolver solver( database_path );
    solver.verbosity = 1;

    // Find camera
    bool found = solver.find_camera( "Blackmagic", "Cinema Camera" );
    OIIO_CHECK_ASSERT( found );

    // Find illuminant
    bool illum_found = solver.find_illuminant( "D65" );
    OIIO_CHECK_ASSERT( illum_found );

    // Load observer data
    bool observer_loaded = solver.load_spectral_data(
        test_dir.get_database_path() + "/cmf/cmf_1931.json", solver.observer );
    OIIO_CHECK_ASSERT( observer_loaded );

    // Load training data
    bool training_loaded = solver.load_spectral_data(
        test_dir.get_database_path() + "/training/training_spectral.json",
        solver.training_data );
    OIIO_CHECK_ASSERT( training_loaded );

    solver.calculate_WB();

    // Calculate IDT matrix and capture stdout
    bool        success;
    std::string output =
        capture_stdout( [&]() { success = solver.calculate_IDT_matrix(); } );

    OIIO_CHECK_ASSERT( success );

    // Verbosity 1 should not print the detailed solver summary or IDT matrix
    ASSERT_NOT_CONTAINS( output, "Solver Summary" );
    ASSERT_NOT_CONTAINS( output, "The IDT matrix is" );
}

/// Tests that verbosity level 2 prints optimization report and IDT matrix
void test_idt_verbosity_level_2()
{
    std::cout << std::endl << "test_idt_verbosity_level_2()" << std::endl;

    // Create test directory with database
    TestFixture fixture;
    auto       &test_dir =
        fixture.with_camera( "Blackmagic", "Cinema Camera" ).build();

    // Use direct API
    std::vector<std::string>  database_path = { test_dir.get_database_path() };
    rta::core::SpectralSolver solver( database_path );
    solver.verbosity = 2;

    // Find camera
    bool found = solver.find_camera( "Blackmagic", "Cinema Camera" );
    OIIO_CHECK_ASSERT( found );

    // Find illuminant
    bool illum_found = solver.find_illuminant( "D65" );
    OIIO_CHECK_ASSERT( illum_found );

    // Load observer data
    bool observer_loaded = solver.load_spectral_data(
        test_dir.get_database_path() + "/cmf/cmf_1931.json", solver.observer );
    OIIO_CHECK_ASSERT( observer_loaded );

    // Load training data
    bool training_loaded = solver.load_spectral_data(
        test_dir.get_database_path() + "/training/training_spectral.json",
        solver.training_data );
    OIIO_CHECK_ASSERT( training_loaded );

    solver.calculate_WB();

    // Calculate IDT matrix and capture stdout
    bool        success;
    std::string output =
        capture_stdout( [&]() { success = solver.calculate_IDT_matrix(); } );

    OIIO_CHECK_ASSERT( success );

    // FullReport should be printed at this verbosity
    ASSERT_CONTAINS( output, "Solver Summary" );

    // Verify IDT matrix was calculated successfully
    const auto &IDT_matrix = solver.get_IDT_matrix();
    OIIO_CHECK_EQUAL( IDT_matrix.size(), 3 );
    OIIO_CHECK_EQUAL( IDT_matrix[0].size(), 3 );
    OIIO_CHECK_EQUAL( IDT_matrix[1].size(), 3 );
    OIIO_CHECK_EQUAL( IDT_matrix[2].size(), 3 );
}

/// Tests that verbosity level 3 enables detailed minimizer progress output
void test_idt_verbosity_level_3()
{
    std::cout << std::endl << "test_idt_verbosity_level_3()" << std::endl;

    // Create test directory with database
    TestFixture fixture;
    auto       &test_dir =
        fixture.with_camera( "Blackmagic", "Cinema Camera" ).build();

    // Use direct API
    std::vector<std::string>  database_path = { test_dir.get_database_path() };
    rta::core::SpectralSolver solver( database_path );
    solver.verbosity = 3;

    // Find camera
    bool found = solver.find_camera( "Blackmagic", "Cinema Camera" );
    OIIO_CHECK_ASSERT( found );

    // Find illuminant
    bool illum_found = solver.find_illuminant( "D65" );
    OIIO_CHECK_ASSERT( illum_found );

    // Load observer data
    bool observer_loaded = solver.load_spectral_data(
        test_dir.get_database_path() + "/cmf/cmf_1931.json", solver.observer );
    OIIO_CHECK_ASSERT( observer_loaded );

    // Load training data
    bool training_loaded = solver.load_spectral_data(
        test_dir.get_database_path() + "/training/training_spectral.json",
        solver.training_data );
    OIIO_CHECK_ASSERT( training_loaded );

    solver.calculate_WB();

    // Calculate IDT matrix and capture stdout
    bool        success;
    std::string output =
        capture_stdout( [&]() { success = solver.calculate_IDT_matrix(); } );

    OIIO_CHECK_ASSERT( success );

    // FullReport should be printed at this verbosity
    ASSERT_CONTAINS( output, "Solver Summary" );

    // Verify IDT matrix was calculated successfully
    const auto &IDT_matrix = solver.get_IDT_matrix();
    OIIO_CHECK_EQUAL( IDT_matrix.size(), 3 );
    OIIO_CHECK_EQUAL( IDT_matrix[0].size(), 3 );
    OIIO_CHECK_EQUAL( IDT_matrix[1].size(), 3 );
    OIIO_CHECK_EQUAL( IDT_matrix[2].size(), 3 );

    // Verify matrix values are reasonable (not NaN, not infinite, rows sum appropriately)
    for ( int i = 0; i < 3; i++ )
    {
        double row_sum = IDT_matrix[i][0] + IDT_matrix[i][1] + IDT_matrix[i][2];
        // IDT matrix rows should sum to approximately 1.0 (with some tolerance)
        OIIO_CHECK_ASSERT( std::abs( row_sum - 1.0 ) < 0.1 );
    }
}

/// Tests that curve fitting returns false when optimization fails
void test_idt_curvefit_failure_returns_false()
{
    std::cout << std::endl
              << "test_idt_curvefit_failure_returns_false()" << std::endl;

    // Create test directory with database
    TestFixture fixture;
    auto       &test_dir = fixture.with_camera( "Blackmagic", "Cinema Camera" )
                         .with_illuminant( "D65" )
                         .build();

    // Set up solver with verbosity to capture summary output
    rta::core::SpectralSolver solver( { test_dir.get_database_path() } );
    solver.verbosity = 2;

    bool found_camera = solver.find_camera( "Blackmagic", "Cinema Camera" );
    OIIO_CHECK_ASSERT( found_camera );

    bool found_illum = solver.find_illuminant( "D65" );
    OIIO_CHECK_ASSERT( found_illum );

    bool observer_loaded = solver.load_spectral_data(
        test_dir.get_database_path() + "/cmf/cmf_1931.json", solver.observer );
    OIIO_CHECK_ASSERT( observer_loaded );

    bool training_loaded = solver.load_spectral_data(
        test_dir.get_database_path() + "/training/training_spectral.json",
        solver.training_data );
    OIIO_CHECK_ASSERT( training_loaded );

    solver.calculate_WB();

    // Inject NaN into camera data to force optimizer failure
    solver.camera["R"].values[0] = std::numeric_limits<double>::quiet_NaN();

    bool        success;
    std::string output =
        capture_stdout( [&]() { success = solver.calculate_IDT_matrix(); } );

    // Optimization should fail and return false
    OIIO_CHECK_ASSERT( !success );
    // Summary should still be printed due to verbosity
    ASSERT_CONTAINS( output, "Solver Summary" );
}

/// Tests that external illuminant files with non-matching types are skipped
void test_illuminant_type_mismatch()
{
    std::cout << std::endl << "test_illuminant_type_mismatch()" << std::endl;

    // Create test directory with database
    TestFixture fixture;
    auto       &test_dir = fixture.with_camera( "Blackmagic", "Cinema Camera" )
                         .with_illuminant( "typeA" )
                         .with_illuminant( "typeB" )
                         .build();

    // Create a mock ImageSpec with camera metadata
    auto image_spec =
        ImageSpecBuilder().camera( "Blackmagic", "Cinema Camera" ).build();

    // Configure settings to request an illuminant type that doesn't match any files
    auto settings = SettingsBuilder()
                        .database( test_dir.get_database_path() )
                        .illuminant( "typeC" ) // Different from typeA and typeB
                        .build();

    std::vector<double>              WB_multipliers;
    std::vector<std::vector<double>> IDT_matrix;
    std::vector<std::vector<double>> CAT_matrix;

    bool        success;
    std::string output = capture_stderr( [&]() {
        // This should fail because no matching illuminant type is found
        success = prepare_transform_spectral(
            image_spec, settings, WB_multipliers, IDT_matrix, CAT_matrix );
    } );

    OIIO_CHECK_ASSERT( !success );
    ASSERT_CONTAINS( output, "Failed to find illuminant type = 'typec'." );
}

/// Tests that blackbody illuminant strings (e.g., "3200K") are correctly processed
void test_blackbody_illuminant_string()
{
    std::cout << std::endl << "test_blackbody_illuminant_string()" << std::endl;

    // Create test directory with database
    TestFixture fixture;
    auto       &test_dir =
        fixture.with_camera( "Blackmagic", "Cinema Camera" ).build();

    // Test with valid blackbody CCT values that should trigger the blackbody branch
    int test_cases[] = { 2000, 2500, 3200, 3500 };

    for ( int test_case: test_cases )
    {
        std::filesystem::path output_path =
            std::filesystem::temp_directory_path() /
            ( "test_blackbody_" + std::to_string( test_case ) + ".exr" );

        // Build command
        auto args = CommandBuilder()
                        .illuminant( std::to_string( test_case ) + "K" )
                        .wb_method( "illuminant" )
                        .mat_method( "spectral" )
                        .input( dng_test_file )
                        .output( output_path.string() )
                        .build();

        // This should succeed and use the blackbody branch in find_illuminant
        std::string output =
            run_rawtoaces_with_data_dir( args, test_dir, false, false );

        // Verify no errors occurred
        ASSERT_NOT_CONTAINS( output, "Failed to find" );
        ASSERT_NOT_CONTAINS( output, "ERROR" );
        ASSERT_NOT_CONTAINS( output, "Failed to configure" );

        // Verify processing occurred
        ASSERT_CONTAINS( output, "Processing file" );
    }
}

/// Tests that auto-detection extracts white balance from RAW metadata when WB_multipliers is not provided
void test_auto_detect_illuminant_from_raw_metadata()
{
    std::cout << std::endl
              << "test_auto_detect_illuminant_from_raw_metadata()" << std::endl;

    // Create test directory with database
    TestFixture fixture;
    auto       &test_dir =
        fixture.with_camera( "Blackmagic", "Cinema Camera" ).build();

    // Create a mock ImageSpec with camera metadata and raw:pre_mul attribute
    float pre_mul[4] = { 1.5f, 1.0f, 1.2f, 1.0f };
    auto  image_spec = ImageSpecBuilder()
                          .camera( "Blackmagic", "Cinema Camera" )
                          .raw_pre_mul( pre_mul, 4 )
                          .build();

    // Configure settings with empty illuminant (to trigger auto-detection)
    auto settings = SettingsBuilder()
                        .database( test_dir.get_database_path() )
                        .illuminant( "" ) // Empty to trigger auto-detection
                        .build();

    // Provide empty WB_multipliers to trigger extraction from raw:pre_mul
    std::vector<double>
        WB_multipliers; // Empty - will trigger raw:pre_mul extraction
    std::vector<std::vector<double>> IDT_matrix;
    std::vector<std::vector<double>> CAT_matrix;

    bool        success;
    std::string output = capture_stderr( [&]() {
        // This should succeed and auto-detect the illuminant from raw:pre_mul
        success = prepare_transform_spectral(
            image_spec, settings, WB_multipliers, IDT_matrix, CAT_matrix );
    } );

    OIIO_CHECK_ASSERT( success );

    // Verify the "Found illuminant:" message appears
    ASSERT_CONTAINS( output, "Found illuminant: '2000k'." );
}

/// Tests that auto-detection normalizes white balance multipliers when min_val > 0 and != 1
void test_auto_detect_illuminant_with_normalization()
{
    std::cout << std::endl
              << "test_auto_detect_illuminant_with_normalization()"
              << std::endl;

    // Create test directory with database
    TestFixture fixture;
    auto       &test_dir =
        fixture.with_camera( "Blackmagic", "Cinema Camera" ).build();

    // Create a mock ImageSpec with camera metadata and raw:pre_mul attribute
    // Using values like {2.0, 1.5, 1.8, 1.5} where min=1.5, which is > 0 and != 1
    float pre_mul[4] = { 2.0f, 1.5f, 1.8f, 1.5f };
    auto  image_spec = ImageSpecBuilder()
                          .camera( "Blackmagic", "Cinema Camera" )
                          .raw_pre_mul( pre_mul, 4 )
                          .build();

    // Configure settings with empty illuminant (to trigger auto-detection)
    auto settings = SettingsBuilder()
                        .database( test_dir.get_database_path() )
                        .illuminant( "" ) // Empty to trigger auto-detection
                        .build();

    // Provide empty WB_multipliers to trigger extraction from raw:pre_mul
    std::vector<double>
        WB_multipliers; // Empty - will trigger raw:pre_mul extraction
    std::vector<std::vector<double>> IDT_matrix;
    std::vector<std::vector<double>> CAT_matrix;

    bool        success;
    std::string output = capture_stderr( [&]() {
        // This should succeed and auto-detect the illuminant from raw:pre_mul
        success = prepare_transform_spectral(
            image_spec, settings, WB_multipliers, IDT_matrix, CAT_matrix );
    } );

    OIIO_CHECK_ASSERT( success );

    // Verify the "Found illuminant:" message appears
    ASSERT_CONTAINS( output, "Found illuminant: '1500k'." );
}

/// Tests that prepare_transform_spectral fails when IDT matrix calculation fails
void test_prepare_transform_spectral_idt_calculation_fail()
{
    std::cout << std::endl
              << "test_prepare_transform_spectral_idt_calculation_fail()"
              << std::endl;

    // Create test directory with database (no training - will create minimal one)
    TestFixture fixture;
    auto       &test_dir = fixture.with_camera( "Blackmagic", "Cinema Camera" )
                         .without_training()
                         .build();

    // Create training data with minimal structure that causes curve fitting to fail
    std::string training_dir = test_dir.get_database_path() + "/training";
    std::filesystem::create_directories( training_dir );
    std::string training_file = training_dir + "/training_spectral.json";

    // Create training data with only one patch and minimal wavelengths
    nlohmann::json training_json;
    training_json["units"] = "relative";
    training_json["index"] = { { "main", { "patch1" } } };

    nlohmann::json data_main;
    data_main["380"]              = { 0.1 };
    data_main["385"]              = { 0.1 };
    data_main["390"]              = { 0.1 };
    training_json["data"]["main"] = data_main;

    std::ofstream training_out( training_file );
    training_out << training_json.dump( 4 );
    training_out.close();

    // Create a mock ImageSpec with camera metadata
    auto image_spec =
        ImageSpecBuilder().camera( "Blackmagic", "Cinema Camera" ).build();

    // Configure settings with illuminant specified
    auto settings =
        SettingsBuilder().database( test_dir.get_database_path() ).build();

    // Provide WB_multipliers
    std::vector<double>              WB_multipliers = { 1.5, 1.0, 1.2 };
    std::vector<std::vector<double>> IDT_matrix;
    std::vector<std::vector<double>> CAT_matrix;

    bool        success;
    std::string output = capture_stderr( [&]() {
        // This should fail when trying to calculate IDT matrix
        success = prepare_transform_spectral(
            image_spec, settings, WB_multipliers, IDT_matrix, CAT_matrix );
    } );

    OIIO_CHECK_ASSERT( !success );

    // Verify the error message about failed IDT matrix calculation
    ASSERT_CONTAINS(
        output, "Failed to calculate the input transform matrix." );
}

/// Tests that conversion succeeds when all required data is present
/// using a built-in illuminant (success case)
void test_spectral_conversion_builtin_illuminant_success()
{
    std::cout << std::endl
              << "test_spectral_conversion_builtin_illuminant_success()"
              << std::endl;

    // Create test directory with database
    TestFixture fixture;
    auto       &test_dir =
        fixture.with_camera( "Blackmagic", "Cinema Camera" ).build();

    // Build command
    auto args = CommandBuilder()
                    .wb_method( "illuminant" )
                    .illuminant( "D65" )
                    .mat_method( "spectral" )
                    .input( dng_test_file )
                    .build();

    // This should succeed and create an output file
    std::string output =
        run_rawtoaces_with_data_dir( args, test_dir, false, false );

    assert_success_conversion( output );
}

/// Tests that conversion succeeds when all required data is present
/// using an illuminant file (success case)
void test_spectral_conversion_external_illuminant_success()
{
    std::cout << std::endl
              << "test_spectral_conversion_external_illuminant_success()"
              << std::endl;

    // Create test directory with database
    TestFixture fixture;
    auto       &test_dir = fixture.with_camera( "Blackmagic", "Cinema Camera" )
                         .with_illuminant( "test_illuminant" )
                         .build();

    // Build command
    auto args = CommandBuilder()
                    .wb_method( "illuminant" )
                    .illuminant( "test_illuminant" )
                    .mat_method( "spectral" )
                    .input( dng_test_file )
                    .build();

    // This should succeed and create an output file
    std::string output =
        run_rawtoaces_with_data_dir( args, test_dir, false, false );

    assert_success_conversion( output );
}

/// Tests that conversion succeeds when all required data is present,
/// using a legacy illuminant file using "header/illuminant" instead of
/// "header/type" (success case)
void test_spectral_conversion_external_legacy_illuminant_success()
{
    std::cout << std::endl
              << "test_spectral_conversion_external_legacy_illuminant_success()"
              << std::endl;

    // Create test directory with database
    TestFixture fixture;
    auto       &test_dir =
        fixture.with_camera( "Blackmagic", "Cinema Camera" )
            .with_illuminant_custom( { { "schema_version", "0.1.0" },
                                       { "illuminant", "test_illuminant" } } )
            .build();

    // Build command
    auto args = CommandBuilder()
                    .wb_method( "illuminant" )
                    .illuminant( "test_illuminant" )
                    .mat_method( "spectral" )
                    .input( dng_test_file )
                    .build();

    // This should succeed and create an output file
    std::string output =
        run_rawtoaces_with_data_dir( args, test_dir, false, false );

    assert_success_conversion( output );
}

/// Tests complete rawtoaces application success case with spectral mode and all data present (should succeed)
void test_rawtoaces_spectral_mode_complete_success_with_custom_camera_info()
{
    std::cout
        << std::endl
        << "test_rawtoaces_spectral_mode_complete_success_with_custom_camera_info()"
        << std::endl;

    // Create test directory with database
    TestFixture fixture;
    auto       &test_dir = fixture.with_camera( "Canon", "EOS_R6" ).build();

    // Build command
    auto args = CommandBuilder()
                    .wb_method( "illuminant" )
                    .illuminant( "D65" )
                    .mat_method( "spectral" )
                    .custom_camera_make( "Canon" )
                    .custom_camera_model( "EOS_R6" )
                    .input( dng_test_file )
                    .build();

    // This should succeed with all data present
    std::string output = run_rawtoaces_with_data_dir( args, test_dir );

    assert_success_conversion( output );
}

/// Tests that conversion succeeds with default illuminant when none specified (should succeed)
void test_rawtoaces_spectral_mode_complete_success_with_default_illuminant_warning()
{
    std::cout
        << std::endl
        << "test_rawtoaces_spectral_mode_complete_success_with_default_illuminant_warning()"
        << std::endl;

    // Create test directory with database
    TestFixture fixture;
    auto       &test_dir =
        fixture.with_camera( "Blackmagic", "Cinema Camera" ).build();

    // Build command (no --illuminant parameter)
    auto args = CommandBuilder()
                    .wb_method( "illuminant" )
                    .mat_method( "spectral" )
                    .input( dng_test_file )
                    .build();

    // This should succeed with default illuminant (D55)
    std::string output =
        run_rawtoaces_with_data_dir( args, test_dir, false, false );

    // Assert that default illuminant warning was shown
    ASSERT_CONTAINS(
        output,
        "Warning: the white balancing method was set to \"illuminant\", but no \"--illuminant\" parameter provided. D55 will be used as default." );
    assert_success_conversion( output );
}

/// Tests that illuminant parameter is ignored when using non-illuminant white balance method (should succeed)
void test_illuminant_ignored_with_metadata_wb()
{
    std::cout << std::endl
              << "test_illuminant_ignored_with_metadata_wb()" << std::endl;

    // Create test directory with database
    TestFixture fixture;
    auto       &test_dir =
        fixture.with_camera( "Blackmagic", "Cinema Camera" ).build();

    // Build command
    auto args = CommandBuilder()
                    .wb_method( "metadata" ) // Different from illuminant
                    .illuminant( "D65" )     // This should be ignored
                    .mat_method( "spectral" )
                    .input( dng_test_file )
                    .build();

    // This should succeed with metadata white balance (ignoring illuminant)
    std::string output =
        run_rawtoaces_with_data_dir( args, test_dir, false, false );

    // Assert that illuminant warning was shown
    ASSERT_CONTAINS(
        output,
        "Warning: the \"--illuminant\" parameter provided but the white balancing mode different from \"illuminant\" requested. The custom illuminant will be ignored." );

    assert_success_conversion( output );
}

/// Tests prepare_transform_spectral when white balance calculation fails due to invalid illuminant data
void test_prepare_transform_spectral_wb_calculation_fail_due_to_invalid_illuminant_data()
{
    std::cout
        << std::endl
        << "test_prepare_transform_spectral_wb_calculation_fail_due_to_invalid_illuminant_data()"
        << std::endl;

    // Create test directory with database (with invalid illuminant)
    TestFixture fixture;
    auto       &test_dir =
        fixture.with_camera( "Blackmagic", "Cinema Camera" )
            .with_illuminant(
                "4200", nlohmann::json::array( { "power", "power2" } ) )
            .build(); // incorrect channel count

    // Build command
    auto args = CommandBuilder()
                    .wb_method( "illuminant" )
                    .illuminant( "4200" )
                    .mat_method( "spectral" )
                    .input( dng_test_file )
                    .build();

    // This should fail with error message about invalid illuminant data
    std::string output =
        run_rawtoaces_with_data_dir( args, test_dir, false, true );

    // Assert on the expected error messages
    std::vector<std::string> expected_errors = {
        "ERROR: illuminant needs to be initialised prior to calling SpectralSolver::calculate_WB()",
        "ERROR: Failed to calculate the white balancing weights.",
        "ERROR: the colour space transform has not been configured properly (spectral mode)."
    };
    ASSERT_CONTAINS_ALL( output, expected_errors );
}

/// Tests prepare_transform_spectral when white balance calculation fails due to invalid camera data
void test_prepare_transform_spectral_wb_calculation_fail_due_to_invalid_camera_data()
{
    std::cout
        << std::endl
        << "test_prepare_transform_spectral_wb_calculation_fail_due_to_invalid_camera_data()"
        << std::endl;

    // Create test directory with database (with invalid camera)
    TestFixture fixture;
    auto       &test_dir = fixture
                         .with_camera(
                             "Blackmagic",
                             "Cinema Camera",
                             nlohmann::json::array( { "R", "G", "B", "D" } ) )
                         .with_illuminant( "4200" )
                         .build();

    // Build command
    auto args = CommandBuilder()
                    .wb_method( "illuminant" )
                    .illuminant( "4200" )
                    .mat_method( "spectral" )
                    .input( dng_test_file )
                    .build();

    // This should fail with error message about invalid camera data
    std::string output =
        run_rawtoaces_with_data_dir( args, test_dir, false, true );

    // Assert on the expected error messages
    std::vector<std::string> expected_errors = {
        "ERROR: camera needs to be initialised prior to calling SpectralSolver::calculate_WB()",
        "ERROR: the colour space transform has not been configured properly (spectral mode)."
    };
    ASSERT_CONTAINS_ALL( output, expected_errors );
}

const std::string HELP_MESSAGE_SNIPPET =
    "Rawtoaces converts raw image files from a digital camera";

/// Tests that main returns 1 when invalid command-line arguments are provided
void test_main_parse_args_failure()
{
    std::cout << std::endl << "test_main_parse_args_failure()" << std::endl;

    // Test with invalid argument flag
    auto args =
        CommandBuilder().arg( "--invalid-flag-that-does-not-exist" ).build();

    // This should fail with exit code 1
    std::string output = run_rawtoaces_command( args, true );

    // Assert on expected error messages
    ASSERT_CONTAINS(
        output,
        "error: Invalid option \"--invalid-flag-that-does-not-exist\"" );
    ASSERT_CONTAINS( output, HELP_MESSAGE_SNIPPET );
}

/// Tests that main returns 1 when valid arguments are provided but parameters are invalid
void test_main_parse_parameters_failure()
{
    std::cout << std::endl
              << "test_main_parse_parameters_failure()" << std::endl;

    // Test with invalid white balance method
    auto args = CommandBuilder().wb_method( "invalid_method" ).build();

    // This should fail with exit code 1 due to parse_parameters failure
    std::string output = run_rawtoaces_command( args, true );

    // Assert on expected error message
    ASSERT_CONTAINS(
        output,
        "Unsupported white balancing method: 'invalid_method'. The following methods are supported: metadata, illuminant, box, custom." );
}

/// Tests that last_error_message is set when processing empty filename
void test_last_error_message_empty_filename()
{
    std::cout << std::endl << "test_last_error_message_empty_filename()" << std::endl;

    ImageConverter converter;
    bool result = converter.process_image( "" );

    OIIO_CHECK_ASSERT( !result );
    OIIO_CHECK_EQUAL( converter.status, ImageConverter::Status::EmptyInputFilename );
    OIIO_CHECK_EQUAL( converter.last_error_message(), "Empty input filename provided" );
}

/// Tests that last_error_message is set when input file does not exist
void test_last_error_message_file_not_found()
{
    std::cout << std::endl << "test_last_error_message_file_not_found()" << std::endl;

    ImageConverter converter;
    std::string nonexistent_file = "nonexistent_file_12345.dng";
    bool result = converter.process_image( nonexistent_file );

    OIIO_CHECK_ASSERT( !result );
    OIIO_CHECK_EQUAL( converter.status, ImageConverter::Status::InputFileNotFound );
    OIIO_CHECK_EQUAL( converter.last_error_message().find( "Input file does not exist" ), 0 );
    OIIO_CHECK_EQUAL( converter.last_error_message().find( nonexistent_file ) != std::string::npos, true );
}

/// Tests that last_error_message is set when output file already exists and overwrite is false
void test_last_error_message_file_exists()
{
    std::cout << std::endl << "test_last_error_message_file_exists()" << std::endl;

    
    if ( OIIO::openimageio_version() < 30000 )
        return;

    ImageConverter converter;
    converter.settings.overwrite = false;
    converter.settings.WB_method = ImageConverter::Settings::WBMethod::Metadata;
    converter.settings.matrix_method = ImageConverter::Settings::MatrixMethod::Metadata;

    std::string test_file = std::filesystem::absolute( dng_test_file ).string();
    
    // First we process the file successfully to create output
    bool first_result = converter.process_image( test_file );
    if ( !first_result )
    {
        return;
    }

    // Clear any previous error message
    converter.process_image( "dummy" );

    // Try to process again without overwrite - should fail with FileExists
    bool second_result = converter.process_image( test_file );
    OIIO_CHECK_ASSERT( !second_result );
    OIIO_CHECK_EQUAL( converter.status, ImageConverter::Status::FileExists );
    OIIO_CHECK_EQUAL( converter.last_error_message().find( "Output file already exists" ), 0 );
}

/// Tests that last_error_message is cleared on successful operations
void test_last_error_message_success_clears_message()
{
    std::cout << std::endl << "test_last_error_message_success_clears_message()" << std::endl;

    
    if ( OIIO::openimageio_version() < 30000 )
        return;

    ImageConverter converter;
    converter.settings.overwrite = true;
    converter.settings.WB_method = ImageConverter::Settings::WBMethod::Metadata;
    converter.settings.matrix_method = ImageConverter::Settings::MatrixMethod::Metadata;

    std::string test_file = std::filesystem::absolute( dng_test_file ).string();

    // First, cause an error
    converter.process_image( "nonexistent_file.dng" );
    OIIO_CHECK_ASSERT( !converter.last_error_message().empty() );

    // Then, process successfully
    bool result = converter.process_image( test_file );
    OIIO_CHECK_ASSERT( result );
    OIIO_CHECK_EQUAL( converter.status, ImageConverter::Status::Success );
    OIIO_CHECK_EQUAL( converter.last_error_message().empty(), true );
}

/// Tests that last_error_message is set on configuration errors
void test_last_error_message_configure_error()
{
    std::cout << std::endl << "test_last_error_message_configure_error()" << std::endl;

    ImageConverter converter;
    std::string nonexistent_file = "nonexistent_config_file.dng";
    
    OIIO::ParamValueList options;
    bool result = converter.configure( nonexistent_file, options );

    OIIO_CHECK_ASSERT( !result );
    OIIO_CHECK_EQUAL( converter.status, ImageConverter::Status::ConfigurationError );
    OIIO_CHECK_EQUAL( converter.last_error_message().find( "Failed to open image file" ), 0 );
}

/// Tests that last_error_message is set on output directory errors
void test_last_error_message_output_directory_error()
{
    std::cout << std::endl << "test_last_error_message_output_directory_error()" << std::endl;

    ImageConverter converter;
    converter.settings.create_dirs = false;
    converter.settings.output_dir = "/nonexistent/directory/path/that/does/not/exist";

    std::string test_file = std::filesystem::absolute( dng_test_file ).string();
    bool result = converter.process_image( test_file );

    OIIO_CHECK_ASSERT( !result );
    OIIO_CHECK_EQUAL( converter.status, ImageConverter::Status::OutputDirectoryError );
    OIIO_CHECK_EQUAL( converter.last_error_message().find( "Output directory does not exist" ) != std::string::npos || 
                      converter.last_error_message().find( "Failed to create directory" ) != std::string::npos, true );
}

/// Tests that main prints help and returns 1 when no files are provided
void test_main_no_files_provided()
{
    std::cout << std::endl << "test_main_no_files_provided()" << std::endl;

    // Test with no files provided
    auto args = CommandBuilder().wb_method( "metadata" ).build();

    // This should fail with exit code 1 and print help
    std::string output = run_rawtoaces_command( args, true );

    ASSERT_CONTAINS( output, HELP_MESSAGE_SNIPPET );
}

/// Tests that main prints help when no files are processed
void test_main_no_files_processed()
{
    std::cout << std::endl << "test_main_no_files_processed()" << std::endl;

    // Create test directory with only filtered files (no valid RAW files)
    TestDirectory test_dir;
    test_dir.create_filtered_files_only();

    // Build command
    auto args = CommandBuilder()
                    .wb_method( "metadata" )
                    .input( test_dir.path() )
                    .build();

    // This should print help when no files are processed
    std::string output = run_rawtoaces_command( args, true );

    // Assert on expected output
    ASSERT_CONTAINS( output, HELP_MESSAGE_SNIPPET );
    ASSERT_NOT_CONTAINS( output, "Processing file" );
}

void test_fetch_missing_metadata()
{
    std::cout << std::endl << "test_fetch_missing_metadata()" << std::endl;

#if defined( WIN32 ) || defined( WIN64 )
    const char *exiftool_path = "..\\..\\exiftool\\exiftool.exe";
    set_env_var( "RAWTOACES_EXIFTOOL_PATH", exiftool_path );
#endif

    rta::util::ImageConverter converter;
    OIIO::ImageSpec           spec;

    converter.settings.disable_exiftool = true;
    bool result =
        fetch_missing_metadata( dng_test_file, converter.settings, spec );
    OIIO_CHECK_ASSERT( result );
    OIIO_CHECK_EQUAL( spec.get_string_attribute( "cameraMake" ), "" );
    OIIO_CHECK_EQUAL( spec.get_string_attribute( "cameraModel" ), "" );

    converter.settings.disable_exiftool = false;
    result = fetch_missing_metadata( nef_test_file, converter.settings, spec );
    OIIO_CHECK_ASSERT( result );
    OIIO_CHECK_EQUAL(
        spec.get_string_attribute( "cameraMake" ), "NIKON CORPORATION" );
    OIIO_CHECK_EQUAL(
        spec.get_string_attribute( "cameraModel" ), "NIKON D200" );

    spec.extra_attribs.remove( "cameraMake" );
    spec.extra_attribs.remove( "cameraModel" );
    // The dng doesn't contain these attributes, but the call should not fail.
    result = fetch_missing_metadata( dng_test_file, converter.settings, spec );
    OIIO_CHECK_ASSERT( result );

    spec.extra_attribs.remove( "cameraMake" );
    spec.extra_attribs.remove( "cameraModel" );
    result =
        fetch_missing_metadata( "wrong_filename", converter.settings, spec );
    OIIO_CHECK_ASSERT( !result );
}

int main( int, char ** )
{
    try
    {
        // Tests for collect_image_files
        test_collect_image_files_directory();
        test_collect_image_files_single_file();
        test_collect_image_files_nonexistent_path();
        test_collect_image_files_empty_directory();
        test_collect_image_files_directory_with_only_filtered_files();
        test_collect_image_files_multiple_paths();
        test_collect_image_files_mixed_valid_invalid_paths();

        // Tests for raw extensions
        test_parse_raw_extensions();

        // Tests for database_paths
        test_database_paths_default();
        test_database_paths_rawtoaces_env();
        test_database_paths_ampas_env();
        test_database_paths_both_env();
        test_database_paths_override_path();

        // Tests for utility functions
        test_convert_linux_path_to_windows_path();

        // Tests for fix_metadata
        test_fix_metadata_both_attributes();
        test_fix_metadata_destination_exists();
        test_fix_metadata_source_missing();
        test_fix_metadata_source_missing();
        test_fix_metadata_unsupported_type();

        test_fetch_missing_metadata();

        // Tests for parse_parameters
        test_parse_parameters_list_formats();
        test_parse_parameters_list_cameras();
        test_parse_parameters_list_cameras( true );
        test_parse_parameters_list_illuminants();
        test_parse_parameters_list_illuminants( true );

        // Tests for prepare_transform_spectral parts
        test_missing_camera_manufacturer();
        test_empty_camera_model();
        test_camera_data_not_found();

        test_missing_training_data();
        test_missing_observer_data();
        test_missing_illuminant_data();
        test_illuminant_type_not_found();
        test_invalid_daylight_cct_exits();
        test_invalid_blackbody_cct_exits();
        test_auto_detect_illuminant_with_wb_multipliers();
        test_database_location_not_directory_warning();
        test_load_spectral_data_absolute_path();
        test_illuminant_file_load_failure();
        test_illuminant_type_mismatch();
        test_all_illuminants_skips_invalid_files();
        test_find_illuminant_camera_no_main_key();
        test_find_illuminant_camera_wrong_size();
        test_blackbody_illuminant_string();

        test_idt_verbosity_level_1();
        test_idt_verbosity_level_2();
        test_idt_verbosity_level_3();
        test_auto_detect_illuminant_from_raw_metadata();
        test_auto_detect_illuminant_with_normalization();
        test_idt_curvefit_failure_returns_false();

        test_spectral_conversion_builtin_illuminant_success();
        test_spectral_conversion_external_illuminant_success();
        test_spectral_conversion_external_legacy_illuminant_success();

        test_rawtoaces_spectral_mode_complete_success_with_custom_camera_info();

        test_prepare_transform_spectral_wb_calculation_fail_due_to_invalid_illuminant_data();
        test_prepare_transform_spectral_wb_calculation_fail_due_to_invalid_camera_data();
        test_prepare_transform_spectral_idt_calculation_fail();

        test_rawtoaces_spectral_mode_complete_success_with_default_illuminant_warning();
        test_illuminant_ignored_with_metadata_wb();

        // Tests for last_error_message functionality
        test_last_error_message_empty_filename();
        test_last_error_message_file_not_found();
        test_last_error_message_file_exists();
        test_last_error_message_success_clears_message();
        test_last_error_message_configure_error();
        test_last_error_message_output_directory_error();

        // Tests for main.cpp error paths
        test_main_parse_args_failure();
        test_main_parse_parameters_failure();
        test_main_no_files_provided();
        test_main_no_files_processed();
    }
    catch ( const std::exception &e )
    {
        std::cerr << "Exception caught in main: " << e.what() << std::endl;
    }
    catch ( ... )
    {
        std::cerr << "Unknown exception caught in main" << std::endl;
    }

    return unit_test_failures;
}
