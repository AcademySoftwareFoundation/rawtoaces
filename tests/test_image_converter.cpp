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

#include <OpenImageIO/unittest.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>
#include <ctime>

#ifdef WIN32
#    include <io.h>
#    include <process.h>
#    include <direct.h> // for _getcwd
#else
#    include <sys/stat.h> // for mkfifo
#    include <unistd.h>   // for getcwd
#endif

using namespace rta::util;

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
///
/// @return std::string containing the captured stdout output from the rawtoaces execution
std::string run_rawtoaces_command( const std::vector<std::string> &args )
{
    // Build the command line - try multiple possible paths for different environments
    std::string command;
#ifdef WIN32
    // Use the standard path from build directory
    command = "..\\src\\rawtoaces\\Release\\rawtoaces.exe";
#else
    command = "../src/rawtoaces/rawtoaces";
#endif

    for ( const auto &arg: args )
    {
        command += " " + arg;
    }

    // Execute command using platform-specific functions
    FILE *pipe = platform_popen( command.c_str(), "r" );
    OIIO_CHECK_ASSERT(
        pipe != nullptr && "Failed to execute rawtoaces command" );

    // Read output
    std::string output;
    char        buffer[4096];
    while ( fgets( buffer, sizeof( buffer ), pipe ) != nullptr )
    {
        output += buffer;
    }

    // Get exit status and validate
    int exit_status = platform_pclose( pipe );
    OIIO_CHECK_EQUAL( exit_status, 0 );

    return output;
}

// Cross-platform environment variable helpers
/*
Standard C Library vs POSIX
getenv() - Part of standard C library (C89/C99) - available everywhere
setenv()/unsetenv() - Part of POSIX standard - only on Unix-like systems
*/
#ifdef WIN32
void set_env_var( const std::string &name, const std::string &value )
{
    _putenv_s( name.c_str(), value.c_str() );
}

std::string to_os_path( std::string linux_path )
{
    return convert_linux_path_to_windows_path( linux_path );
}

void unset_env_var( const std::string &name )
{
    _putenv_s( name.c_str(), "" );
}
#else
std::string to_os_path( std::string linux_path )
{
    return linux_path;
}

void set_env_var( const std::string &name, const std::string &value )
{
    setenv( name.c_str(), value.c_str(), 1 );
}

void unset_env_var( const std::string &name )
{
    unsetenv( name.c_str() );
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
    std::vector<std::string> &args,
    const std::string        &datab_path,
    bool                      use_dir_path_arg = false )
{
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

    std::string       output      = run_rawtoaces_command( args );
    OIIO::string_view output_view = output;
    OIIO::Strutil::trim_whitespace( output_view );

    if ( !use_dir_path_arg )
    {
        // Clean up environment variable
        unset_env_var( "RAWTOACES_DATA_PATH" );
    }

    return output_view;
}

/// This test verifies that when --list-cameras is provided, the method
/// calls supported_cameras() and outputs the camera list, then exits
void test_parse_parameters_list_cameras( bool use_dir_path_arg = false )
{
    std::cout << std::endl
              << "test_parse_parameters_list_cameras("
              << ( use_dir_path_arg ? "with data dir" : "without data dir" )
              << ")" << std::endl;

    // Create test directory with dynamic database
    TestDirectory test_dir;

    // Create test camera data files
    test_dir.create_test_data_file(
        "camera", { { "manufacturer", "Canon" }, { "model", "EOS R6" } } );
    test_dir.create_test_data_file(
        "camera", { { "manufacturer", "Mamiya" }, { "model", "Mamiya 7" } } );

    std::vector<std::string> args   = { "--list-cameras" };
    auto                     output = run_rawtoaces_with_data_dir(
        args, test_dir.get_database_path(), use_dir_path_arg );

    std::vector<std::string> lines;
    OIIO::Strutil::split( output, lines, "\n" );

    OIIO_CHECK_EQUAL( lines.size(), 3 );
    OIIO_CHECK_EQUAL(
        lines[0],
        "Spectral sensitivity data is available for the following cameras:" );

    // Check that both cameras are present (order doesn't matter)
    bool found_canon = false, found_mamiya = false;
    for ( size_t i = 1; i < lines.size(); ++i )
    {
        if ( lines[i] == "Canon / EOS R6" )
            found_canon = true;
        if ( lines[i] == "Mamiya / Mamiya 7" )
            found_mamiya = true;
    }
    OIIO_CHECK_EQUAL( found_canon, true );
    OIIO_CHECK_EQUAL( found_mamiya, true );
}

/// This test verifies that when --list-illuminants is provided, the method
/// calls supported_illuminants() and outputs the illuminant list, then exits
void test_parse_parameters_list_illuminants( bool use_dir_path_arg = false )
{
    std::cout << std::endl
              << "test_parse_parameters_list_illuminants()" << std::endl;

    // Create test directory with dynamic database
    TestDirectory test_dir;

    // Create test illuminant data file
    test_dir.create_test_data_file(
        "illuminant", { { "illuminant", "my-illuminant" } } );

    std::vector<std::string> args = { "--list-illuminants" };

    // Run the test with --list-illuminants argument using the dynamic database
    auto output = run_rawtoaces_with_data_dir(
        args, test_dir.get_database_path(), use_dir_path_arg );

    std::vector<std::string> lines;
    OIIO::Strutil::split( output, lines, "\n" );

    OIIO_CHECK_EQUAL( lines.size(), 4 );
    OIIO_CHECK_EQUAL( lines[0], "The following illuminants are supported:" );
    OIIO_CHECK_EQUAL( lines[1], "Day-light (e.g., D60, D6025)" );
    OIIO_CHECK_EQUAL( lines[2], "Blackbody (e.g., 3200K)" );
    OIIO_CHECK_EQUAL( lines[3], "my-illuminant" );
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
        test_database_paths_override_path();

        // Tests for utility functions
        test_convert_linux_path_to_windows_path();

        // Tests for fix_metadata
        test_fix_metadata_both_attributes();
        test_fix_metadata_destination_exists();
        test_fix_metadata_source_missing();
        test_fix_metadata_source_missing();
        test_fix_metadata_unsupported_type();

        // Tests for parse_parameters
        test_parse_parameters_list_cameras();
        test_parse_parameters_list_cameras( true );
        test_parse_parameters_list_illuminants();
        test_parse_parameters_list_illuminants( true );
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
