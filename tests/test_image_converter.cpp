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
#include <sstream>
#include <vector>
#include <ctime>

#include "test_utils.h"

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
    /// @param is_incorrect_data Whether to create incorrect data (for testing error cases)
    /// @return The full path to the created file
    std::string create_test_data_file(
        const std::string    &type,
        const nlohmann::json &header_data = { { "schema_version", "1.0.0" } },
        const bool            is_incorrect_data = false )
    {
        // Create target directory dynamically based on type
        std::string target_dir = database_dir + "/" + type;
        std::filesystem::create_directories( target_dir );

        // Use expected filename for specific types, random for others
        std::string filename;
        if ( type == "training" )
        {
            filename = "training_spectral.json";
        }
        else if ( type == "cmf" )
        {
            filename = "cmf_1931.json";
        }
        else
        {
            static int file_counter = 0;
            filename = "test_" + type + "_" + std::to_string( ++file_counter ) +
                       ".json";
        }
        std::string file_path = target_dir + "/" + filename;

        // Create minimal JSON structure with only what's actually used
        nlohmann::json json_data;

        // Header - only include what's actually used for camera matching
        nlohmann::json header = header_data;
        json_data["header"]   = header;

        // Spectral data - only include what's actually used
        nlohmann::json spectral_data;
        spectral_data["units"] = "relative";
        spectral_data["index"] = { { "main", { "R", "G", "B" } } };

        // Add spectral data based on type
        if ( type == "camera" )
        {
            // Camera data needs RGB channels
            if ( is_incorrect_data )
            {
                spectral_data["index"] = { { "main", { "R", "G", "B", "D" } } };
            }
            else
            {
                spectral_data["index"] = { { "main", { "R", "G", "B" } } };
            }
            nlohmann::json data_main;
            for ( int wavelength = 380; wavelength <= 780; wavelength += 5 )
            {
                // Simple test values - production code just needs the structure
                double r_val = 0.1 + ( wavelength - 380 ) * 0.001;
                double g_val = 0.2 + ( wavelength - 380 ) * 0.001;
                double b_val = 0.3 + ( wavelength - 380 ) * 0.001;
                if ( is_incorrect_data )
                {
                    data_main[std::to_string( wavelength )] = {
                        r_val, g_val, b_val, 1
                    };
                }
                else
                {
                    data_main[std::to_string( wavelength )] = { r_val,
                                                                g_val,
                                                                b_val };
                }
            }
            spectral_data["data"]["main"] = data_main;
        }
        else if ( type == "training" )
        {
            // Training data needs multiple patches
            spectral_data["index"] = { { "main",
                                         { "patch1", "patch2", "patch3" } } };
            nlohmann::json data_main;
            for ( int wavelength = 380; wavelength <= 780; wavelength += 5 )
            {
                // Simple test training patch values
                double patch1_val = 0.1 + ( wavelength - 380 ) * 0.001;
                double patch2_val = 0.2 + ( wavelength - 380 ) * 0.001;
                double patch3_val = 0.3 + ( wavelength - 380 ) * 0.001;

                data_main[std::to_string( wavelength )] = { patch1_val,
                                                            patch2_val,
                                                            patch3_val };
            }
            spectral_data["data"]["main"] = data_main;
        }
        else if ( type == "cmf" )
        {
            // Observer (CMF) data needs XYZ channels
            spectral_data["index"] = { { "main", { "X", "Y", "Z" } } };
            nlohmann::json data_main;
            for ( int wavelength = 380; wavelength <= 780; wavelength += 5 )
            {
                // Simple test CMF values
                double x_val = 0.1 + ( wavelength - 380 ) * 0.001;
                double y_val = 0.2 + ( wavelength - 380 ) * 0.001;
                double z_val = 0.3 + ( wavelength - 380 ) * 0.001;

                data_main[std::to_string( wavelength )] = { x_val,
                                                            y_val,
                                                            z_val };
            }
            spectral_data["data"]["main"] = data_main;
        }
        else if ( type == "illuminant" )
        {
            // Illuminant data needs 1 channel (power spectrum)
            if ( is_incorrect_data )
            {
                spectral_data["index"] = { { "main", { "power", "power2" } } };
            }
            else
            {
                spectral_data["index"] = { { "main", { "power" } } };
            };

            nlohmann::json data_main;
            for ( int wavelength = 380; wavelength <= 780; wavelength += 5 )
            {
                // Simple test illuminant power values
                double power_val = 1.0 + ( wavelength - 380 ) * 0.01;
                if ( is_incorrect_data )
                {
                    data_main[std::to_string( wavelength )] = { power_val,
                                                                power_val };
                }
                else
                {
                    data_main[std::to_string( wavelength )] = { power_val };
                }
            }

            spectral_data["data"]["main"] = data_main;
        }
        else
        {
            // For other types, use empty data object
            spectral_data["data"] = nlohmann::json::object();
        }

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
    bool                      use_dir_path_arg = false,
    bool                      allow_failure    = false )
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

    std::string output = run_rawtoaces_command( args, allow_failure );

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
/// calls get_supported_cameras() and outputs the camera list, then exits
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
        "camera", { { "manufacturer", "Canon" }, { "model", "EOS_R6" } } );
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
    TestDirectory test_dir;

    // Create test illuminant data file
    test_dir.create_test_data_file(
        "illuminant", { { "type", "my-illuminant" } } );

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

/// Tests that prepare_transform_spectral fails when no camera manufacturer information is available (should fail)
void test_missing_camera_manufacturer()
{
    std::cout << std::endl << "test_missing_camera_manufacturer()" << std::endl;

    // Create test directory with database
    TestDirectory test_dir;

    // Create training data (so training data loading succeeds)
    test_dir.create_test_data_file( "training" );

    // Create observer data (so observer data loading succeeds)
    test_dir.create_test_data_file( "cmf" );

    // Create a mock ImageSpec with no camera metadata
    OIIO::ImageSpec image_spec;
    image_spec.width     = 100;
    image_spec.height    = 100;
    image_spec.nchannels = 3;
    image_spec.format    = OIIO::TypeDesc::UINT8;

    // Don't set any camera make or model attributes

    // Configure settings with no custom camera info
    ImageConverter::Settings settings;
    settings.database_directories = { test_dir.get_database_path() };
    settings.illuminant           = "D65";
    settings.verbosity            = 1;

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

    // Should fail
    OIIO_CHECK_ASSERT( !success );

    // Assert on the expected error message
    OIIO_CHECK_ASSERT(
        output.find(
            "Missing the camera manufacturer name in the file metadata. You can provide a camera make using the --custom-camera-make parameter" ) !=
        std::string::npos );
}

/// Tests that conversion fails when camera model is missing (should fail)
void test_empty_camera_model()
{
    std::cout << std::endl << "test_empty_camera_model()" << std::endl;

    // Create test directory with database
    TestDirectory test_dir;

    // Create training data (so training data loading succeeds)
    test_dir.create_test_data_file( "training" );

    // Create observer data (so observer data loading succeeds)
    test_dir.create_test_data_file( "cmf" );

    // Create ImageSpec with camera make but no model
    OIIO::ImageSpec image_spec;
    image_spec["cameraMake"] = "Blackmagic";
    // Do not set cameraModel - this is what we're testing

    // Set up ImageConverter with spectral mode settings
    ImageConverter converter;
    converter.settings.WB_method =
        ImageConverter::Settings::WBMethod::Illuminant;
    converter.settings.matrix_method =
        ImageConverter::Settings::MatrixMethod::Spectral;
    converter.settings.illuminant = "D65";
    converter.settings.verbosity  = 1;
    converter.settings.database_directories.push_back(
        test_dir.get_database_path() );

    // Create empty options list
    OIIO::ParamValueList options;

    // Capture stderr output to verify error messages
    bool        success;
    std::string output = capture_stderr( [&]() {
        // This should fail with error message about missing camera model
        success = converter.configure( image_spec, options );
    } );

    // Should fail
    OIIO_CHECK_ASSERT( !success );

    // Assert on the expected error message - focus on the main camera identifier error
    OIIO_CHECK_ASSERT(
        output.find(
            "Missing the camera model name in the file metadata. You can provide a camera model using the --custom-camera-model parameter" ) !=
        std::string::npos );
    OIIO_CHECK_ASSERT(
        output.find(
            "ERROR: the colour space transform has not been configured properly (spectral mode)." ) !=
        std::string::npos );
}

/// Tests that conversion fails when camera data is not found in database (should fail)
void test_camera_data_not_found()
{
    std::cout << std::endl << "test_camera_data_not_found()" << std::endl;

    // Create test directory with database
    TestDirectory test_dir;

    // Create a proper camera data file using the helper method
    test_dir.create_test_data_file(
        "camera", { { "manufacturer", "Canon" }, { "model", "EOS_R6" } } );

    // Create training data (so training data loading succeeds)
    test_dir.create_test_data_file( "training" );

    // Create observer data (so observer data loading succeeds)
    test_dir.create_test_data_file( "cmf" );

    // Test: Camera data not found via main entrance (no custom camera info provided, DNG has metadata but no data in database)
    std::vector<std::string> args = { "--wb-method",  "illuminant",
                                      "--illuminant", "D65",
                                      "--mat-method", "spectral",
                                      "--verbose",    "--overwrite",
                                      dng_test_file };

    // This should fail with error message about camera data not found
    std::string output = run_rawtoaces_with_data_dir(
        args, test_dir.get_database_path(), false, true );

    // Assert on the expected error message - focus on the main camera data lookup failure
    OIIO_CHECK_ASSERT(
        output.find(
            "Failed to find spectral data for camera make: 'Blackmagic', model: 'Cinema Camera'." ) !=
        std::string::npos );
    OIIO_CHECK_ASSERT(
        output.find(
            "Please check the database search path in RAWTOACES_DATABASE_PATH" ) !=
        std::string::npos );
    OIIO_CHECK_ASSERT(
        output.find(
            "ERROR: the colour space transform has not been configured properly (spectral mode)." ) !=
        std::string::npos );
}

/// Tests that conversion fails when training data is missing (should fail)
void test_missing_training_data()
{
    std::cout << std::endl << "test_missing_training_data()" << std::endl;

    // Create test directory with database
    TestDirectory test_dir;

    // Create camera data (so camera lookup succeeds)
    test_dir.create_test_data_file(
        "camera",
        { { "manufacturer", "Blackmagic" }, { "model", "Cinema Camera" } } );

    // Create observer data (so observer data loading succeeds)
    test_dir.create_test_data_file( "cmf" );

    // Test: Missing training data via main entrance
    std::vector<std::string> args = { "--wb-method",  "illuminant",
                                      "--illuminant", "D65",
                                      "--mat-method", "spectral",
                                      "--verbose",    "--overwrite",
                                      dng_test_file };

    // This should fail with error message about missing training data
    std::string output = run_rawtoaces_with_data_dir(
        args, test_dir.get_database_path(), false, true );

    // Assert on the expected error message
    OIIO_CHECK_ASSERT(
        output.find(
            "Failed to find training data 'training/training_spectral.json'." ) !=
        std::string::npos );
}

/// Tests that conversion fails when observer data is missing (should fail)
void test_missing_observer_data()
{
    std::cout << std::endl << "test_missing_observer_data()" << std::endl;

    // Create test directory with database
    TestDirectory test_dir;

    // Create camera data (so camera lookup succeeds)
    test_dir.create_test_data_file(
        "camera",
        { { "manufacturer", "Blackmagic" }, { "model", "Cinema Camera" } } );

    // Create training data (so training data loading succeeds)
    test_dir.create_test_data_file( "training" );

    // Test: Missing observer data via main entrance
    std::vector<std::string> args = { "--wb-method",  "illuminant",
                                      "--illuminant", "D65",
                                      "--mat-method", "spectral",
                                      "--verbose",    "--overwrite",
                                      dng_test_file };

    // This should fail with error message about missing observer data
    std::string output = run_rawtoaces_with_data_dir(
        args, test_dir.get_database_path(), false, true );

    // Assert on the expected error message
    OIIO_CHECK_ASSERT(
        output.find( "Failed to find observer 'cmf/cmf_1931.json'." ) !=
        std::string::npos );
}

/// Tests that conversion fails when illuminant data is missing (should fail)
void test_missing_illuminant_data()
{
    std::cout << std::endl << "test_missing_illuminant_data()" << std::endl;

    // Create test directory with database
    TestDirectory test_dir;

    // Create camera data (so camera lookup succeeds)
    test_dir.create_test_data_file(
        "camera",
        { { "manufacturer", "Blackmagic" }, { "model", "Cinema Camera" } } );

    // Create training data (so training data loading succeeds)
    test_dir.create_test_data_file( "training" );

    // Create observer data (so observer data loading succeeds)
    test_dir.create_test_data_file( "cmf" );

    // Test: Missing illuminant data via main entrance (using non-existent illuminant)
    std::vector<std::string> args = { "--wb-method",  "illuminant",
                                      "--illuminant", "nonexistentilluminant",
                                      "--mat-method", "spectral",
                                      "--verbose",    "--overwrite",
                                      dng_test_file };

    // This should fail with error message about missing illuminant data
    std::string output = run_rawtoaces_with_data_dir(
        args, test_dir.get_database_path(), false, true );

    // Assert on the expected error message
    OIIO_CHECK_ASSERT(
        output.find( "Error: No matching light source" ) != std::string::npos );
}

/// Tests that conversion fails when specified illuminant type is not found in illuminant data (should fail)
void test_illuminant_type_not_found()
{
    std::cout << std::endl << "test_illuminant_type_not_found()" << std::endl;

    // Create test directory with database
    TestDirectory test_dir;

    // Create camera data (so camera lookup succeeds)
    test_dir.create_test_data_file(
        "camera",
        { { "manufacturer", "Blackmagic" }, { "model", "Cinema Camera" } } );

    // Create training data (so training data loading succeeds)
    test_dir.create_test_data_file( "training" );

    // Create observer data (so observer data loading succeeds)
    test_dir.create_test_data_file( "cmf" );

    // Create a mock ImageSpec with camera metadata
    OIIO::ImageSpec image_spec;
    image_spec.width          = 100;
    image_spec.height         = 100;
    image_spec.nchannels      = 3;
    image_spec.format         = OIIO::TypeDesc::UINT8;
    image_spec["cameraMake"]  = "Blackmagic";
    image_spec["cameraModel"] = "Cinema Camera";

    // Configure settings with illuminant that doesn't exist in the database
    ImageConverter::Settings settings;
    settings.database_directories = { test_dir.get_database_path() };
    settings.illuminant = "A"; // Request illuminant "A" which doesn't exist
    settings.verbosity  = 1;

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

    // Should fail
    OIIO_CHECK_ASSERT( !success );

    // Assert on the expected error message
    OIIO_CHECK_ASSERT(
        output.find( "Failed to find illuminant type = 'a'." ) !=
        std::string::npos );
}

/// Tests that auto-detection of illuminant works with 4-channel WB_multipliers and verbosity output
void test_auto_detect_illuminant_with_wb_multipliers()
{
    std::cout << std::endl
              << "test_auto_detect_illuminant_with_wb_multipliers()"
              << std::endl;

    // Create test directory with database
    TestDirectory test_dir;

    // Create camera data (so camera lookup succeeds)
    test_dir.create_test_data_file(
        "camera",
        { { "manufacturer", "Blackmagic" }, { "model", "Cinema Camera" } } );

    // Create training data (so training data loading succeeds)
    test_dir.create_test_data_file( "training" );

    // Create observer data (so observer data loading succeeds)
    test_dir.create_test_data_file( "cmf" );

    // Create a mock ImageSpec with camera metadata
    OIIO::ImageSpec image_spec;
    image_spec.width          = 100;
    image_spec.height         = 100;
    image_spec.nchannels      = 3;
    image_spec.format         = OIIO::TypeDesc::UINT8;
    image_spec["cameraMake"]  = "Blackmagic";
    image_spec["cameraModel"] = "Cinema Camera";

    // Configure settings with empty illuminant (to trigger auto-detection)
    // and verbosity > 0 (to trigger the "Found illuminant:" message)
    ImageConverter::Settings settings;
    settings.database_directories = { test_dir.get_database_path() };
    settings.illuminant           = ""; // Empty to trigger auto-detection
    settings.verbosity            = 1;  // > 0 to trigger the output message

    // Provide WB_multipliers with size 4 to exercise the 4-channel path
    std::vector<double> WB_multipliers = { 1.5, 1.0, 1.2, 1.0 }; // 4 channels
    std::vector<std::vector<double>> IDT_matrix;
    std::vector<std::vector<double>> CAT_matrix;

    bool        success;
    std::string output = capture_stderr( [&]() {
        // This should succeed and auto-detect the illuminant
        // This will exercise the 4-channel WB_multipliers path (when WB_multipliers.size() == 4)
        // and the verbosity output path (when verbosity > 0 and illuminant is found)
        success = prepare_transform_spectral(
            image_spec, settings, WB_multipliers, IDT_matrix, CAT_matrix );
    } );

    // Should succeed
    OIIO_CHECK_ASSERT( success );

    // With the current mocked input (WB_multipliers = {1.5, 1.0, 1.2, 1.0}),
    OIIO_CHECK_ASSERT(
        output.find( "Found illuminant: '2000k'." ) != std::string::npos );
}

/// Tests that auto-detection extracts white balance from RAW metadata when WB_multipliers is not provided
void test_auto_detect_illuminant_from_raw_metadata()
{
    std::cout << std::endl
              << "test_auto_detect_illuminant_from_raw_metadata()" << std::endl;

    // Create test directory with database
    TestDirectory test_dir;

    // Create camera data (so camera lookup succeeds)
    test_dir.create_test_data_file(
        "camera",
        { { "manufacturer", "Blackmagic" }, { "model", "Cinema Camera" } } );

    // Create training data (so training data loading succeeds)
    test_dir.create_test_data_file( "training" );

    // Create observer data (so observer data loading succeeds)
    test_dir.create_test_data_file( "cmf" );

    // Use direct library method to control WB_multipliers and force the else path
    // The exec method populates WB_multipliers from metadata, so it takes the if branch.
    // To test the else branch (extract from raw:pre_mul), we need WB_multipliers.size() != 4.

    // Create a mock ImageSpec with camera metadata and raw:pre_mul attribute
    OIIO::ImageSpec image_spec;
    image_spec.width          = 100;
    image_spec.height         = 100;
    image_spec.nchannels      = 3;
    image_spec.format         = OIIO::TypeDesc::UINT8;
    image_spec["cameraMake"]  = "Blackmagic";
    image_spec["cameraModel"] = "Cinema Camera";

    // Add raw:pre_mul attribute to simulate RAW metadata extraction path
    float pre_mul[4] = { 1.5f, 1.0f, 1.2f, 1.0f };
    image_spec.attribute(
        "raw:pre_mul", OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 4 ), pre_mul );

    // Configure settings with empty illuminant (to trigger auto-detection)
    // and verbosity > 0 (to trigger the "Found illuminant:" message)
    ImageConverter::Settings settings;
    settings.database_directories = { test_dir.get_database_path() };
    settings.illuminant           = ""; // Empty to trigger auto-detection
    settings.verbosity            = 1;  // > 0 to trigger the output message

    // Provide empty WB_multipliers to trigger extraction from raw:pre_mul
    // This exercises the path where WB_multipliers.size() != 4
    std::vector<double>
        WB_multipliers; // Empty - will trigger raw:pre_mul extraction
    std::vector<std::vector<double>> IDT_matrix;
    std::vector<std::vector<double>> CAT_matrix;

    bool        success;
    std::string output = capture_stderr( [&]() {
        // This should succeed and auto-detect the illuminant from raw:pre_mul
        // This exercises the extraction path when WB_multipliers is not provided
        success = prepare_transform_spectral(
            image_spec, settings, WB_multipliers, IDT_matrix, CAT_matrix );
    } );

    // Should succeed
    OIIO_CHECK_ASSERT( success );

    // Verify the "Found illuminant:" message appears
    OIIO_CHECK_ASSERT(
        output.find( "Found illuminant: '2000k'." ) != std::string::npos );
}

/// Tests that auto-detection normalizes white balance multipliers when min_val > 0 and != 1
void test_auto_detect_illuminant_with_normalization()
{
    std::cout << std::endl
              << "test_auto_detect_illuminant_with_normalization()"
              << std::endl;

    // Create test directory with database
    TestDirectory test_dir;

    // Create camera data (so camera lookup succeeds)
    test_dir.create_test_data_file(
        "camera",
        { { "manufacturer", "Blackmagic" }, { "model", "Cinema Camera" } } );

    // Create training data (so training data loading succeeds)
    test_dir.create_test_data_file( "training" );

    // Create observer data (so observer data loading succeeds)
    test_dir.create_test_data_file( "cmf" );

    // Create a mock ImageSpec with camera metadata and raw:pre_mul attribute
    OIIO::ImageSpec image_spec;
    image_spec.width          = 100;
    image_spec.height         = 100;
    image_spec.nchannels      = 3;
    image_spec.format         = OIIO::TypeDesc::UINT8;
    image_spec["cameraMake"]  = "Blackmagic";
    image_spec["cameraModel"] = "Cinema Camera";

    // Add raw:pre_mul attribute with values where min_val > 0 and != 1
    // Using values like {2.0, 1.5, 1.8, 1.5} where min=1.5, which is > 0 and != 1
    // This will trigger the normalization path
    float pre_mul[4] = { 2.0f, 1.5f, 1.8f, 1.5f };
    image_spec.attribute(
        "raw:pre_mul", OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 4 ), pre_mul );

    // Configure settings with empty illuminant (to trigger auto-detection)
    // and verbosity > 0 (to trigger the "Found illuminant:" message)
    ImageConverter::Settings settings;
    settings.database_directories = { test_dir.get_database_path() };
    settings.illuminant           = ""; // Empty to trigger auto-detection
    settings.verbosity            = 1;  // > 0 to trigger the output message

    // Provide empty WB_multipliers to trigger extraction from raw:pre_mul
    // This exercises the path where WB_multipliers.size() != 4
    std::vector<double>
        WB_multipliers; // Empty - will trigger raw:pre_mul extraction
    std::vector<std::vector<double>> IDT_matrix;
    std::vector<std::vector<double>> CAT_matrix;

    bool        success;
    std::string output = capture_stderr( [&]() {
        // This should succeed and auto-detect the illuminant from raw:pre_mul
        // The normalization path will be exercised when min_val > 0 and != 1
        success = prepare_transform_spectral(
            image_spec, settings, WB_multipliers, IDT_matrix, CAT_matrix );
    } );

    // Should succeed
    OIIO_CHECK_ASSERT( success );

    // Verify the "Found illuminant:" message appears
    OIIO_CHECK_ASSERT(
        output.find( "Found illuminant: '1500k'." ) != std::string::npos );
}

/// Tests that prepare_transform_spectral fails when IDT matrix calculation fails
void test_prepare_transform_spectral_idt_calculation_fail()
{
    std::cout << std::endl
              << "test_prepare_transform_spectral_idt_calculation_fail()"
              << std::endl;

    // Create test directory with database
    TestDirectory test_dir;

    // Create camera data (so camera lookup succeeds)
    test_dir.create_test_data_file(
        "camera",
        { { "manufacturer", "Blackmagic" }, { "model", "Cinema Camera" } } );

    // Create observer data (so observer data loading succeeds)
    test_dir.create_test_data_file( "cmf" );

    // Create training data with minimal structure that causes curve fitting to fail
    // We need to create a file that loads but causes optimization to fail
    std::string training_dir = test_dir.get_database_path() + "/training";
    std::filesystem::create_directories( training_dir );
    std::string training_file = training_dir + "/training_spectral.json";

    // Create training data with only one patch and minimal wavelengths
    // This should pass initial validation but cause curve fitting to fail
    nlohmann::json training_json;
    training_json["units"] = "relative";
    training_json["index"] = { { "main", { "patch1" } } };

    nlohmann::json data_main;
    // Add only a few wavelengths - insufficient for proper curve fitting
    data_main["380"]              = { 0.1 };
    data_main["385"]              = { 0.1 };
    data_main["390"]              = { 0.1 };
    training_json["data"]["main"] = data_main;

    std::ofstream training_out( training_file );
    training_out << training_json.dump( 4 );
    training_out.close();

    // Create a mock ImageSpec with camera metadata
    OIIO::ImageSpec image_spec;
    image_spec.width          = 100;
    image_spec.height         = 100;
    image_spec.nchannels      = 3;
    image_spec.format         = OIIO::TypeDesc::UINT8;
    image_spec["cameraMake"]  = "Blackmagic";
    image_spec["cameraModel"] = "Cinema Camera";

    // Configure settings with illuminant specified
    ImageConverter::Settings settings;
    settings.database_directories = { test_dir.get_database_path() };
    settings.illuminant           = "D65";
    settings.verbosity            = 1;

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

    // Should fail
    OIIO_CHECK_ASSERT( !success );

    // Verify the error message about failed IDT matrix calculation
    OIIO_CHECK_ASSERT(
        output.find( "Failed to calculate the input transform matrix." ) !=
        std::string::npos );
}

void assert_success_conversion( const std::string &output )
{
    // Assert that the command succeeded (no error messages)
    OIIO_CHECK_ASSERT( output.find( "Failed to find" ) == std::string::npos );
    OIIO_CHECK_ASSERT( output.find( "ERROR" ) == std::string::npos );
    OIIO_CHECK_ASSERT( output.find( "Missing" ) == std::string::npos );
    OIIO_CHECK_ASSERT(
        output.find( "Failed to configure" ) == std::string::npos );

    // Assert that processing completed successfully
    OIIO_CHECK_ASSERT( output.find( "Processing file" ) != std::string::npos );
    OIIO_CHECK_ASSERT(
        output.find( "Configuring transform" ) != std::string::npos );
    OIIO_CHECK_ASSERT( output.find( "Loading image" ) != std::string::npos );
    OIIO_CHECK_ASSERT( output.find( "Saving output" ) != std::string::npos );

    // Assert that white balance coefficients were calculated
    OIIO_CHECK_ASSERT(
        output.find( "White balance coefficients" ) != std::string::npos );

    // Assert that IDT matrix was calculated
    OIIO_CHECK_ASSERT(
        output.find( "Input Device Transform (IDT) matrix" ) !=
        std::string::npos );

    // Assert that image processing steps occurred
    OIIO_CHECK_ASSERT(
        output.find( "Applying transform matrix" ) != std::string::npos );
    OIIO_CHECK_ASSERT( output.find( "Applying scale" ) != std::string::npos );
    OIIO_CHECK_ASSERT( output.find( "Applying crop" ) != std::string::npos );

    // Assert that the correct input and output files were processed
    OIIO_CHECK_ASSERT(
        output.find( "blackmagic_cinema_camera_cinemadng.dng" ) !=
        std::string::npos );
    OIIO_CHECK_ASSERT(
        output.find( "blackmagic_cinema_camera_cinemadng_aces.exr" ) !=
        std::string::npos );
}

/// Tests that conversion succeeds when all required data is present
/// using a built-in illuminant (success case)
void test_spectral_conversion_builtin_illuminant_success()
{
    std::cout << std::endl
              << "test_spectral_conversion_builtin_illuminant_success()"
              << std::endl;

    // Create test directory with database
    TestDirectory test_dir;

    // Create camera data (so camera lookup succeeds)
    test_dir.create_test_data_file(
        "camera",
        { { "manufacturer", "Blackmagic" }, { "model", "Cinema Camera" } } );

    // Create training data (so training data loading succeeds)
    test_dir.create_test_data_file( "training" );

    // Create observer data (so observer data loading succeeds)
    test_dir.create_test_data_file( "cmf" );

    // Test: Successful conversion via main entrance
    std::vector<std::string> args = { "--wb-method",  "illuminant",
                                      "--illuminant", "D65",
                                      "--mat-method", "spectral",
                                      "--verbose",    "--overwrite",
                                      dng_test_file };

    // This should succeed and create an output file
    std::string output = run_rawtoaces_with_data_dir(
        args, test_dir.get_database_path(), false, false );

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
    TestDirectory test_dir;

    // Create camera data (so camera lookup succeeds)
    test_dir.create_test_data_file(
        "camera",
        { { "manufacturer", "Blackmagic" }, { "model", "Cinema Camera" } } );

    // Create training data (so training data loading succeeds)
    test_dir.create_test_data_file( "training" );

    // Create observer data (so observer data loading succeeds)
    test_dir.create_test_data_file( "cmf" );

    //     Create illuminant data (so illuminant data loading succeeds)
    test_dir.create_test_data_file(
        "illuminant", { { "type", "test_illuminant" } } );

    // Test: Successful conversion via main entrance
    std::vector<std::string> args = { "--wb-method",  "illuminant",
                                      "--illuminant", "test_illuminant",
                                      "--mat-method", "spectral",
                                      "--verbose",    "--overwrite",
                                      dng_test_file };

    // This should succeed and create an output file
    std::string output = run_rawtoaces_with_data_dir(
        args, test_dir.get_database_path(), false, false );

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
    TestDirectory test_dir;

    // Create camera data (so camera lookup succeeds)
    test_dir.create_test_data_file(
        "camera",
        { { "manufacturer", "Blackmagic" }, { "model", "Cinema Camera" } } );

    // Create training data (so training data loading succeeds)
    test_dir.create_test_data_file( "training" );

    // Create observer data (so observer data loading succeeds)
    test_dir.create_test_data_file( "cmf" );

    // Create illuminant data (so illuminant data loading succeeds)
    test_dir.create_test_data_file(
        "illuminant",
        { { "schema_version", "0.1.0" },
          { "illuminant", "test_illuminant" } } );

    // Test: Successful conversion via main entrance
    std::vector<std::string> args = { "--wb-method",  "illuminant",
                                      "--illuminant", "test_illuminant",
                                      "--mat-method", "spectral",
                                      "--verbose",    "--overwrite",
                                      dng_test_file };

    // This should succeed and create an output file
    std::string output = run_rawtoaces_with_data_dir(
        args, test_dir.get_database_path(), false, false );

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
    TestDirectory test_dir;

    // Create proper camera data file
    test_dir.create_test_data_file(
        "camera", { { "manufacturer", "Canon" }, { "model", "EOS_R6" } } );

    // Create training data
    test_dir.create_test_data_file( "training" );

    // Create observer data
    test_dir.create_test_data_file( "cmf", { { "type", "observer" } } );

    // Test: Complete success case via main entrance
    std::vector<std::string> args = { "--wb-method",
                                      "illuminant",
                                      "--illuminant",
                                      "D65",
                                      "--mat-method",
                                      "spectral",
                                      "--custom-camera-make",
                                      "Canon",
                                      "--custom-camera-model",
                                      "EOS_R6",
                                      "--verbose",
                                      "--overwrite",
                                      dng_test_file };

    // This should succeed with all data present
    std::string output =
        run_rawtoaces_with_data_dir( args, test_dir.get_database_path() );

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
    TestDirectory test_dir;

    // Create camera data (so camera lookup succeeds)
    test_dir.create_test_data_file(
        "camera",
        { { "manufacturer", "Blackmagic" }, { "model", "Cinema Camera" } } );

    // Create training data (so training data loading succeeds)
    test_dir.create_test_data_file( "training" );

    // Create observer data (so observer data loading succeeds)
    test_dir.create_test_data_file( "cmf" );

    // Test: Default illuminant via main entrance (no --illuminant parameter)
    std::vector<std::string> args = { "--wb-method",  "illuminant",
                                      "--mat-method", "spectral",
                                      "--verbose",    "--overwrite",
                                      dng_test_file };

    // This should succeed with default illuminant (D55)
    std::string output = run_rawtoaces_with_data_dir(
        args, test_dir.get_database_path(), false, false );

    // Assert that default illuminant warning was shown
    OIIO_CHECK_ASSERT(
        output.find(
            "Warning: the white balancing method was set to \"illuminant\", but no \"--illuminant\" parameter provided. D55 will be used as default." ) !=
        std::string::npos );
    assert_success_conversion( output );
}

/// Tests that illuminant parameter is ignored when using non-illuminant white balance method (should succeed)
void test_illuminant_ignored_with_metadata_wb()
{
    std::cout << std::endl
              << "test_illuminant_ignored_with_metadata_wb()" << std::endl;

    // Create test directory with database
    TestDirectory test_dir;

    // Create camera data (so camera lookup succeeds)
    test_dir.create_test_data_file(
        "camera",
        { { "manufacturer", "Blackmagic" }, { "model", "Cinema Camera" } } );

    // Create training data (so training data loading succeeds)
    test_dir.create_test_data_file( "training" );

    // Create observer data (so observer data loading succeeds)
    test_dir.create_test_data_file( "cmf", { { "type", "observer" } } );

    // Test: Illuminant ignored when using metadata white balance method
    std::vector<std::string> args = {
        "--wb-method",  "metadata", // Different from illuminant
        "--illuminant", "D65",      // This should be ignored
        "--mat-method", "spectral", "--verbose", "--overwrite", dng_test_file
    };

    // This should succeed with metadata white balance (ignoring illuminant)
    std::string output = run_rawtoaces_with_data_dir(
        args, test_dir.get_database_path(), false, false );

    // Assert that illuminant warning was shown
    OIIO_CHECK_ASSERT(
        output.find(
            "Warning: the \"--illuminant\" parameter provided but the white balancing mode different from \"illuminant\" requested. The custom illuminant will be ignored." ) !=
        std::string::npos );

    assert_success_conversion( output );
}

/// Tests prepare_transform_spectral when white balance calculation fails due to invalid illuminant data
void test_prepare_transform_spectral_wb_calculation_fail_due_to_invalid_illuminant_data()
{
    std::cout
        << std::endl
        << "test_prepare_transform_spectral_wb_calculation_fail_due_to_invalid_illuminant_data()"
        << std::endl;

    // Create test directory with database
    TestDirectory test_dir;

    // Create proper camera data file
    test_dir.create_test_data_file(
        "camera",
        { { "manufacturer", "Blackmagic" }, { "model", "Cinema Camera" } } );

    // Create training data
    test_dir.create_test_data_file( "training" );

    // Create observer data
    test_dir.create_test_data_file( "cmf", { { "type", "observer" } } );

    // Create illuminant data with invalid structure (should cause WB calculation to fail)
    test_dir.create_test_data_file(
        "illuminant", { { "type", "4200" } }, true );

    // Test: WB calculation fails due to invalid illuminant data
    std::vector<std::string> args = { "--wb-method",  "illuminant",
                                      "--illuminant", "4200",
                                      "--mat-method", "spectral",
                                      "--verbose",    "--overwrite",
                                      dng_test_file };

    // This should fail with error message about invalid illuminant data
    std::string output = run_rawtoaces_with_data_dir(
        args, test_dir.get_database_path(), false, true );

    // Assert on the expected error message
    OIIO_CHECK_ASSERT(
        output.find(
            "ERROR: illuminant needs to be initialised prior to calling SpectralSolver::calculate_WB()" ) !=
        std::string::npos );
    OIIO_CHECK_ASSERT(
        output.find(
            "ERROR: Failed to calculate the white balancing weights." ) !=
        std::string::npos );
    OIIO_CHECK_ASSERT(
        output.find(
            "ERROR: the colour space transform has not been configured properly (spectral mode)." ) !=
        std::string::npos );
}

/// Tests prepare_transform_spectral when white balance calculation fails due to invalid camera data
void test_prepare_transform_spectral_wb_calculation_fail_due_to_invalid_camera_data()
{
    std::cout
        << std::endl
        << "test_prepare_transform_spectral_wb_calculation_fail_due_to_invalid_camera_data()"
        << std::endl;

    // Create test directory with database
    TestDirectory test_dir;

    // Create proper camera data file
    test_dir.create_test_data_file(
        "camera",
        { { "manufacturer", "Blackmagic" }, { "model", "Cinema Camera" } },
        true );

    // Create training data
    test_dir.create_test_data_file( "training" );

    // Create observer data
    test_dir.create_test_data_file( "cmf", { { "type", "observer" } } );

    // Create illuminant data with invalid structure (should cause WB calculation to fail)
    test_dir.create_test_data_file( "illuminant", { { "type", "4200" } } );

    // Test: WB calculation fails due to invalid illuminant data
    std::vector<std::string> args = { "--wb-method",  "illuminant",
                                      "--illuminant", "4200",
                                      "--mat-method", "spectral",
                                      "--verbose",    "--overwrite",
                                      dng_test_file };

    // This should fail with error message about invalid illuminant data
    std::string output = run_rawtoaces_with_data_dir(
        args, test_dir.get_database_path(), false, true );

    std::cout << "output: $$$<" << output << ">$$$" << std::endl;

    // Assert on the expected error message
    OIIO_CHECK_ASSERT(
        output.find(
            "ERROR: camera needs to be initialised prior to calling SpectralSolver::calculate_WB()" ) !=
        std::string::npos );
    OIIO_CHECK_ASSERT(
        output.find(
            "ERROR: the colour space transform has not been configured properly (spectral mode)." ) !=
        std::string::npos );
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

        // Tests for prepare_transform_spectral parts
        test_missing_camera_manufacturer();
        test_empty_camera_model();
        test_camera_data_not_found();

        test_missing_training_data();
        test_missing_observer_data();
        test_missing_illuminant_data();
        test_illuminant_type_not_found();
        test_auto_detect_illuminant_with_wb_multipliers();
        test_auto_detect_illuminant_from_raw_metadata();
        test_auto_detect_illuminant_with_normalization();

        test_spectral_conversion_builtin_illuminant_success();
        test_spectral_conversion_external_illuminant_success();
        test_spectral_conversion_external_legacy_illuminant_success();

        test_rawtoaces_spectral_mode_complete_success_with_custom_camera_info();

        test_prepare_transform_spectral_wb_calculation_fail_due_to_invalid_illuminant_data();
        test_prepare_transform_spectral_wb_calculation_fail_due_to_invalid_camera_data();
        test_prepare_transform_spectral_idt_calculation_fail();

        test_rawtoaces_spectral_mode_complete_success_with_default_illuminant_warning();
        test_illuminant_ignored_with_metadata_wb();
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
