// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#    undef RGB
#endif

#include <OpenImageIO/unittest.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>
#include <sys/stat.h> // for mkfifo
#include <ctime>

#include "../src/rawtoaces_util/rawtoaces_util_priv.h"
#include <rawtoaces/image_converter.h>

using namespace rta::util;

// Cross-platform environment variable helpers
/*
Standard C Library vs POSIX
getenv() - Part of standard C library (C89/C99) - available everywhere
setenv()/unsetenv() - Part of POSIX standard - only on Unix-like systems
*/
#ifdef WIN32
void set_env_var( const char *name, const char *value )
{
    _putenv_s( name, value );
}
void unset_env_var( const char *name )
{
    _putenv_s( name, "" );
}
#else
void set_env_var( const char *name, const char *value )
{
    setenv( name, value, 1 );
}
void unset_env_var( const char *name )
{
    unsetenv( name );
}
#endif

/// RAII (Resource Acquisition Is Initialization)
/// helper class for test directory management
class TestDirectory
{
public:
    TestDirectory()
    {
        // Create unique directory names using timestamp and random number
        static int counter = 0;
        test_dir           = ( std::filesystem::temp_directory_path() /
                     ( "rawtoaces_test_" + std::to_string( ++counter ) + "_" +
                       std::to_string( std::time( nullptr ) ) ) )
                       .string();
        std::filesystem::create_directories( test_dir );

        // Create database directory for test data
        database_dir = test_dir + "/test-database";
        std::filesystem::create_directories( database_dir );
    }

    ~TestDirectory() { std::filesystem::remove_all( test_dir ); }

    // Disable copy constructor and assignment operator
    TestDirectory( const TestDirectory & )            = delete;
    TestDirectory &operator=( const TestDirectory & ) = delete;

    const std::string &path() const { return test_dir; }
    const std::string &get_database_path() const { return database_dir; }

    void create_test_files()
    {
        // Create valid image files (different extensions)
        std::ofstream( test_dir + "/test1.raw" ).close();
        std::ofstream( test_dir + "/test2.cr2" ).close();
        std::ofstream( test_dir + "/test3.nef" ).close();
        std::ofstream( test_dir + "/test4.dng" ).close();

        // Create files that should be filtered out
        std::ofstream( test_dir + "/.DS_Store" ).close();
        std::ofstream( test_dir + "/test5.exr" ).close();
        std::ofstream( test_dir + "/test6.jpg" ).close();
        std::ofstream( test_dir + "/test7.jpeg" ).close();
        std::ofstream( test_dir + "/test8.EXR" ).close();
        std::ofstream( test_dir + "/test9.JPG" ).close();
        std::ofstream( test_dir + "/test10.JPEG" ).close();

#ifdef WIN32
        // On Windows, symlink creation requires elevated privileges
        // Just create a regular file instead for testing purposes
        std::ofstream( test_dir + "/symlink.raw" ).close();
#else
        // Create a symlink to the regular file
        std::filesystem::create_symlink(
            "test1.raw", test_dir + "/symlink.raw" );
#endif

        // Create a subdirectory (should be ignored)
        std::filesystem::create_directories( test_dir + "/subdir" );
        std::ofstream( test_dir + "/subdir/test8.raw" ).close();
    }

    void create_filtered_files_only()
    {
        // Create only files that should be filtered out
        std::ofstream( test_dir + "/.DS_Store" ).close();
        std::ofstream( test_dir + "/test.exr" ).close();
        std::ofstream( test_dir + "/test.jpg" ).close();
        std::ofstream( test_dir + "/test.jpeg" ).close();
    }

    void create_valid_files( const std::vector<std::string> &filenames )
    {
        for ( const auto &filename: filenames )
        {
            std::ofstream( test_dir + "/" + filename ).close();
        }
    }

    /// Creates a test data file (camera or illuminant) with the specified header data
    /// @param type The type of test data to create (e.g. camera or illuminant)
    /// @param header_data JSON object containing the header data to include
    /// @return The full path to the created file
    std::string create_test_data_file(
        const std::string &type, const nlohmann::json &header_data )
    {
        // Generate random filename
        static int  file_counter = 0;
        std::string filename = "test_data_" + std::to_string( ++file_counter ) +
                               "_" + std::to_string( std::time( nullptr ) ) +
                               ".json";

        // Create target directory dynamically based on type
        std::string target_dir = database_dir + "/" + type;
        std::filesystem::create_directories( target_dir );
        std::string file_path = target_dir + "/" + filename;

        // Create JSON object using nlohmann/json
        nlohmann::json json_data;

        // Start with default header and merge user data
        nlohmann::json header = header_data;

        // Build spectral_data object
        nlohmann::json spectral_data = { { "units", "relative" },
                                         { "index",
                                           { { "main", { "R", "G", "B" } } } },
                                         { "data", nlohmann::json::object() } };

        // Assemble final JSON
        json_data["header"]        = header;
        json_data["spectral_data"] = spectral_data;

        // Write to file with pretty formatting
        std::ofstream file( file_path );
        file << json_data.dump( 4 ) << std::endl;
        file.close();

        return file_path;
    }

private:
    std::string test_dir;
    std::string database_dir;
};

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
#if !defined( WIN32 ) && !defined( WIN64 )
    OIIO_CHECK_EQUAL( paths.size(), 2 );
    OIIO_CHECK_EQUAL( paths[0], "/usr/local/share/rawtoaces/data" );
    OIIO_CHECK_EQUAL( paths[1], "/usr/local/include/rawtoaces/data" );
#else
    // On Windows, should have just the current directory
    OIIO_CHECK_EQUAL( paths.size(), 1 );
    OIIO_CHECK_EQUAL( paths[0], "." );
#endif
}

/// Tests database_paths with RAWTOACES_DATA_PATH environment variable set
void test_database_paths_rawtoaces_env()
{
    std::cout << std::endl
              << "test_database_paths_rawtoaces_env()" << std::endl;
    // Set RAWTOACES_DATA_PATH
#ifdef WIN32
    set_env_var( "RAWTOACES_DATA_PATH", "C:\\custom\\path1;C:\\custom\\path2" );
#else
    set_env_var( "RAWTOACES_DATA_PATH", "/custom/path1:/custom/path2" );
#endif
    unset_env_var( "AMPAS_DATA_PATH" );

    std::vector<std::string> paths = database_paths();

    OIIO_CHECK_EQUAL( paths.size(), 2 );
#ifdef WIN32
    OIIO_CHECK_EQUAL( paths[0], "C:\\custom\\path1" );
    OIIO_CHECK_EQUAL( paths[1], "C:\\custom\\path2" );
#else
    OIIO_CHECK_EQUAL( paths[0], "/custom/path1" );
    OIIO_CHECK_EQUAL( paths[1], "/custom/path2" );
#endif

    // Clean up
    unset_env_var( "RAWTOACES_DATA_PATH" );
}

/// Tests database_paths with deprecated AMPAS_DATA_PATH environment variable
void test_database_paths_ampas_env()
{
    std::cout << std::endl << "test_database_paths_ampas_env()" << std::endl;
    // Set AMPAS_DATA_PATH (deprecated)
    unset_env_var( "RAWTOACES_DATA_PATH" );
#ifdef WIN32
    set_env_var(
        "AMPAS_DATA_PATH", "C:\\deprecated\\path1;C:\\deprecated\\path2" );
#else
    set_env_var( "AMPAS_DATA_PATH", "/deprecated/path1:/deprecated/path2" );
#endif

    std::vector<std::string> paths = database_paths();

    OIIO_CHECK_EQUAL( paths.size(), 2 );
#ifdef WIN32
    OIIO_CHECK_EQUAL( paths[0], "C:\\deprecated\\path1" );
    OIIO_CHECK_EQUAL( paths[1], "C:\\deprecated\\path2" );
#else
    OIIO_CHECK_EQUAL( paths[0], "/deprecated/path1" );
    OIIO_CHECK_EQUAL( paths[1], "/deprecated/path2" );
#endif

    // Clean up
    unset_env_var( "AMPAS_DATA_PATH" );
}

/// Tests database_paths with both environment variables set (RAWTOACES_DATA_PATH should take precedence)
void test_database_paths_both_env()
{
    std::cout << std::endl << "test_database_paths_both_env()" << std::endl;
    // Set both environment variables
#ifdef WIN32
    set_env_var(
        "RAWTOACES_DATA_PATH", "C:\\preferred\\path1;C:\\preferred\\path2" );
    set_env_var(
        "AMPAS_DATA_PATH", "C:\\deprecated\\path1;C:\\deprecated\\path2" );
#else
    set_env_var( "RAWTOACES_DATA_PATH", "/preferred/path1:/preferred/path2" );
    set_env_var( "AMPAS_DATA_PATH", "/deprecated/path1:/deprecated/path2" );
#endif

    std::vector<std::string> paths = database_paths();

    // RAWTOACES_DATA_PATH should take precedence
    OIIO_CHECK_EQUAL( paths.size(), 2 );
#ifdef WIN32
    OIIO_CHECK_EQUAL( paths[0], "C:\\preferred\\path1" );
    OIIO_CHECK_EQUAL( paths[1], "C:\\preferred\\path2" );
#else
    OIIO_CHECK_EQUAL( paths[0], "/preferred/path1" );
    OIIO_CHECK_EQUAL( paths[1], "/preferred/path2" );
#endif

    // Clean up
    unset_env_var( "RAWTOACES_DATA_PATH" );
    unset_env_var( "AMPAS_DATA_PATH" );
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

/// Helper function to set up test environment and capture output for parse_parameters tests
struct ParseParametersTestResult
{
    bool        success;
    std::string output;
};

/// Executes a parse_parameters test with the given command-line arguments and captures its output.
///
/// This helper function encapsulates all the common setup required for testing ImageConverter::parse_parameters,
/// including environment configuration, argument parsing, stdout capture, and cleanup.
///
/// @param args Vector of command-line arguments to pass to parse_parameters (excluding program name).
///             For example, {"--list-cameras"} or {"--list-illuminants", "--verbose"}.
/// @param database_path Path to the test database directory (optional, uses default if not provided)
///
/// @return ParseParametersTestResult containing:
///         - success: true if parse_parameters executed successfully, false if argument parsing failed
///         - output:  captured stdout output from the parse_parameters execution
ParseParametersTestResult run_parse_parameters_test(
    const std::vector<std::string> &args,
    const std::string              &database_path = "" )
{
    // Set up test data path to use the provided database path or default
    set_env_var( "RAWTOACES_DATA_PATH", database_path.c_str() );

    // Create ImageConverter instance
    rta::util::ImageConverter converter;

    // Create argument parser and initialize it
    OIIO::ArgParse arg_parser;
    converter.init_parser( arg_parser );

    // Convert args to char* array for parsing
    std::vector<const char *> argv;
    argv.push_back( "rawtoaces" ); // Program name
    for ( const auto &arg: args )
    {
        argv.push_back( arg.c_str() );
    }

    // Parse the arguments
    int parse_result =
        arg_parser.parse_args( static_cast<int>( argv.size() ), argv.data() );
    if ( parse_result != 0 )
    {
        unset_env_var( "RAWTOACES_DATA_PATH" );
        return { false, "" };
    }

    // Capture stdout to verify the output
    std::ostringstream captured_output;
    std::streambuf    *original_cout = std::cout.rdbuf();
    std::cout.rdbuf( captured_output.rdbuf() );

    // Call parse_parameters
    bool result = converter.parse_parameters( arg_parser );

    // Restore original cout
    std::cout.rdbuf( original_cout );

    // Clean up environment variable
    unset_env_var( "RAWTOACES_DATA_PATH" );

    return { result, captured_output.str() };
}

/// This test verifies that when --list-cameras is provided, the method
/// calls supported_cameras() and outputs the camera list, then exits
void test_parse_parameters_list_cameras()
{
    std::cout << std::endl
              << "test_parse_parameters_list_cameras()" << std::endl;

    // Create test directory with dynamic database
    TestDirectory test_dir;

    // Create test camera data files
    test_dir.create_test_data_file(
        "camera", { { "manufacturer", "Canon" }, { "model", "EOS R6" } } );
    test_dir.create_test_data_file(
        "camera", { { "manufacturer", "Mamiya" }, { "model", "Mamiya 7" } } );

    // Run the test with --list-cameras argument using the dynamic database
    auto result = run_parse_parameters_test(
        { "--list-cameras" }, test_dir.get_database_path() );

    // The method should return true (though it calls exit in real usage)
    OIIO_CHECK_EQUAL( result.success, true );

    // Verify the output contains expected camera list information
    OIIO_CHECK_EQUAL(
        result.output.find(
            "Spectral sensitivity data is available for the following cameras:" ) !=
            std::string::npos,
        true );

    // Verify that actual camera names from test data are present
    // The format is "manufacturer / model" as defined in supported_cameras()
    OIIO_CHECK_EQUAL(
        result.output.find( "Canon / EOS R6" ) != std::string::npos, true );
    OIIO_CHECK_EQUAL(
        result.output.find( "Mamiya / Mamiya 7" ) != std::string::npos, true );

    // Count occurrences of " / " to verify we have 2 camera entries
    size_t camera_count = 0;
    size_t pos          = 0;
    while ( ( pos = result.output.find( " / ", pos ) ) != std::string::npos )
    {
        camera_count++;
        pos += 3; // Move past " / "
    }
    OIIO_CHECK_EQUAL( camera_count, 2 );
}

/// This test verifies that when --list-illuminants is provided, the method
/// calls supported_illuminants() and outputs the illuminant list, then exits
void test_parse_parameters_list_illuminants()
{
    std::cout << std::endl
              << "test_parse_parameters_list_illuminants()" << std::endl;

    // Create test directory with dynamic database
    TestDirectory test_dir;

    // Create test illuminant data file
    test_dir.create_test_data_file(
        "illuminant", { { "illuminant", "my-illuminant" } } );

    // Run the test with --list-illuminants argument using the dynamic database
    auto result = run_parse_parameters_test(
        { "--list-illuminants" }, test_dir.get_database_path() );

    // The method should return true (though it calls exit in real usage)
    OIIO_CHECK_EQUAL( result.success, true );

    // Verify the output contains expected illuminant list information
    OIIO_CHECK_EQUAL(
        result.output.find( "The following illuminants are supported:" ) !=
            std::string::npos,
        true );

    // Verify that actual illuminant names from test data are present
    // The hardcoded illuminant types should be present
    OIIO_CHECK_EQUAL(
        result.output.find( "Day-light (e.g., D60, D6025)" ) !=
            std::string::npos,
        true );
    OIIO_CHECK_EQUAL(
        result.output.find( "Blackbody (e.g., 3200K)" ) != std::string::npos,
        true );

    // Verify that the specific illuminant from our test data is present
    OIIO_CHECK_EQUAL(
        result.output.find( "my-illuminant" ) != std::string::npos, true );

    // Verify we have exactly 3 illuminants total (2 hardcoded + 1 from test data)
    // Count newlines in the illuminant list section to verify count
    size_t illuminant_count = 0;
    size_t start_pos =
        result.output.find( "The following illuminants are supported:" );
    if ( start_pos != std::string::npos )
    {
        size_t end_pos = result.output.find(
            "\n\n", start_pos ); // Look for double newline (end of list)
        if ( end_pos == std::string::npos )
        {
            end_pos = result.output.length(); // If no double newline, go to end
        }

        std::string illuminant_section =
            result.output.substr( start_pos, end_pos - start_pos );
        size_t pos = 0;
        while ( ( pos = illuminant_section.find( "\n", pos ) ) !=
                std::string::npos )
        {
            illuminant_count++;
            pos += 1;
        }
        // Subtract 1 for the header line
        illuminant_count = ( illuminant_count > 0 ) ? illuminant_count - 1 : 0;
    }
    OIIO_CHECK_EQUAL( illuminant_count, 3 );
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

        // Tests for database_paths
        test_database_paths_default();
        test_database_paths_rawtoaces_env();
        test_database_paths_ampas_env();
        test_database_paths_both_env();

        // Tests for fix_metadata
        test_fix_metadata_both_attributes();
        test_fix_metadata_destination_exists();
        test_fix_metadata_source_missing();
        test_fix_metadata_source_missing();
        test_fix_metadata_unsupported_type();

        // Tests for parse_parameters
        test_parse_parameters_list_cameras();
        test_parse_parameters_list_illuminants();
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
