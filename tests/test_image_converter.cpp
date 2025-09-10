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
#include <vector>
#include <sys/stat.h> // for mkfifo
#include <cstdlib>    // for set_env_var, unset_env_var

#include "../src/rawtoaces_util/rawtoaces_util_priv.h"

using namespace rta::util;

// Cross-platform environment variable helpers
#ifdef WIN32
void set_env_var( const char *name, const char *value )
{
    SetEnvironmentVariableA( name, value );
}
void unset_env_var( const char *name )
{
    SetEnvironmentVariableA( name, nullptr );
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
        test_dir = ( std::filesystem::temp_directory_path() / "rawtoaces_test" )
                       .string();
        std::filesystem::create_directories( test_dir );
    }

    ~TestDirectory() { std::filesystem::remove_all( test_dir ); }

    // Disable copy constructor and assignment operator
    TestDirectory( const TestDirectory & )            = delete;
    TestDirectory &operator=( const TestDirectory & ) = delete;

    const std::string &path() const { return test_dir; }

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

private:
    std::string test_dir;
};

/// Verifies that collect_image_files can traverse a directory, identify valid RAW image files,
/// filter out unwanted file types, and organize them into batches for processing
void test_collect_image_files_directory()
{
    TestDirectory test_dir;
    test_dir.create_test_files();

    std::vector<std::vector<std::string>> batches;
    bool result = collect_image_files( test_dir.path(), batches );

    OIIO_CHECK_EQUAL( result, true );
    OIIO_CHECK_EQUAL( batches.size(), 1 );
    OIIO_CHECK_EQUAL( batches[0].size(), 5 ); // Should have 5 valid files

    // Check that the correct files are included
    std::vector<std::string> expected_files = { test_dir.path() + "/test1.raw",
                                                test_dir.path() + "/test2.cr2",
                                                test_dir.path() + "/test3.nef",
                                                test_dir.path() + "/test4.dng",
                                                test_dir.path() +
                                                    "/symlink.raw" };

    for ( const auto &expected: expected_files )
    {
        bool found = false;
        for ( const auto &actual: batches[0] )
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
    TestDirectory test_dir;
    std::string   test_file = test_dir.path() + "/test.raw";
    std::ofstream( test_file ).close();

    std::vector<std::vector<std::string>> batches;
    bool result = collect_image_files( test_file, batches );

    OIIO_CHECK_EQUAL( result, true );
    OIIO_CHECK_EQUAL( batches.size(), 1 );
    OIIO_CHECK_EQUAL( batches[0].size(), 1 );
    OIIO_CHECK_EQUAL( batches[0][0], test_file );
}

/// Verifies that collect_image_files returns false and creates no batches when given
/// a path that doesn't exist, ensuring proper error handling for invalid input paths
void test_collect_image_files_nonexistent_path()
{
    std::vector<std::vector<std::string>> batches;
    bool result = collect_image_files( "nonexistent_path", batches );

    OIIO_CHECK_EQUAL( result, false );
    OIIO_CHECK_EQUAL( batches.size(), 0 );
}

/// Ensures that when given an empty directory, collect_image_files returns true but
/// creates an empty batch, maintaining consistent behavior for edge cases
void test_collect_image_files_empty_directory()
{
    TestDirectory test_dir;

    std::vector<std::vector<std::string>> batches;
    bool result = collect_image_files( test_dir.path(), batches );

    OIIO_CHECK_EQUAL( result, true );
    OIIO_CHECK_EQUAL( batches.size(), 1 );
    OIIO_CHECK_EQUAL(
        batches[0].size(), 0 ); // Empty directory should result in empty batch
}

/// Verifies that when a directory contains only files that should be filtered out
/// (like .DS_Store, .jpg, .exr), collect_image_files still returns true but creates an empty batch,
/// ensuring the filtering logic works correctly in real-world scenarios
void test_collect_image_files_directory_with_only_filtered_files()
{
    TestDirectory test_dir;
    test_dir.create_filtered_files_only();

    std::vector<std::vector<std::string>> batches;
    bool result = collect_image_files( test_dir.path(), batches );

    OIIO_CHECK_EQUAL( result, true );
    OIIO_CHECK_EQUAL( batches.size(), 1 );
    OIIO_CHECK_EQUAL( batches[0].size(), 0 ); // All files filtered out
}

/// Tests database_paths with no environment variables set (uses default paths)
void test_database_paths_default()
{
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
    std::cout << "  Setting RAWTOACES_DATA_PATH..." << std::endl;
    // Set RAWTOACES_DATA_PATH
#ifdef WIN32
    set_env_var( "RAWTOACES_DATA_PATH", "C:\\custom\\path1;C:\\custom\\path2" );
#else
    set_env_var( "RAWTOACES_DATA_PATH", "/custom/path1:/custom/path2" );
#endif
    unset_env_var( "AMPAS_DATA_PATH" );

    std::cout << "  Calling database_paths()..." << std::endl;
    std::vector<std::string> paths = database_paths();
    std::cout << "  database_paths() returned " << paths.size() << " paths"
              << std::endl;

    OIIO_CHECK_EQUAL( paths.size(), 2 );
#ifdef WIN32
    OIIO_CHECK_EQUAL( paths[0], "C:\\custom\\path1" );
    OIIO_CHECK_EQUAL( paths[1], "C:\\custom\\path2" );
#else
    OIIO_CHECK_EQUAL( paths[0], "/custom/path1" );
    OIIO_CHECK_EQUAL( paths[1], "/custom/path2" );
#endif

    std::cout << "  Cleaning up..." << std::endl;
    // Clean up
    unset_env_var( "RAWTOACES_DATA_PATH" );
    std::cout << "  Cleanup complete" << std::endl;
}

/// Tests database_paths with deprecated AMPAS_DATA_PATH environment variable
void test_database_paths_ampas_env()
{
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

/// Tests database_paths with Windows-style path separator
void test_database_paths_windows_separator()
{
    // Set RAWTOACES_DATA_PATH with Windows-style separator
    set_env_var( "RAWTOACES_DATA_PATH", "/path1;/path2;/path3" );

    std::vector<std::string> paths = database_paths();

// Should split by the appropriate separator for the platform
#if defined( WIN32 ) || defined( WIN64 )
    OIIO_CHECK_EQUAL( paths.size(), 3 );
    OIIO_CHECK_EQUAL( paths[0], "/path1" );
    OIIO_CHECK_EQUAL( paths[1], "/path2" );
    OIIO_CHECK_EQUAL( paths[2], "/path3" );
#else
    // On Unix, semicolon won't be split, so we get one path
    OIIO_CHECK_EQUAL( paths.size(), 1 );
    OIIO_CHECK_EQUAL( paths[0], "/path1;/path2;/path3" );
#endif

    // Clean up
    unset_env_var( "RAWTOACES_DATA_PATH" );
}

/// Tests fix_metadata with both Make and Model attributes
void test_fix_metadata_both_attributes()
{
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

/// Tests fix_metadata with float attributes
void test_fix_metadata_float_make()
{
    OIIO::ImageSpec spec;

    // Add original "Make" attribute as float (unusual but possible)
    spec["Make"] = 42.5f;

    // Call fix_metadata
    fix_metadata( spec );

    // Check that "cameraMake" was created with correct float value
    OIIO_CHECK_EQUAL( spec.get_float_attribute( "cameraMake" ), 42.5f );

    // Check that original "Make" was removed
    OIIO_CHECK_EQUAL( spec.find_attribute( "Make" ), nullptr );
}

/// Tests fix_metadata when destination already exists (should not overwrite or remove source)
void test_fix_metadata_destination_exists()
{
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
    OIIO::ImageSpec spec;

    // Don't add any "Make" or "Model" attributes

    // Call fix_metadata
    fix_metadata( spec );

    // Check that no attributes were created
    OIIO_CHECK_EQUAL( spec.find_attribute( "cameraMake" ), nullptr );
    OIIO_CHECK_EQUAL( spec.find_attribute( "cameraModel" ), nullptr );
}

/// Tests fix_metadata with non-string, non-float attributes (should be ignored)
void test_fix_metadata_unsupported_type()
{
    OIIO::ImageSpec spec;

    // Add integer attribute (this should be ignored by fix_metadata)
    spec["Make"] = 42; // Integer, not string or float

    // Call fix_metadata
    fix_metadata( spec );

    // Check that no destination was created (unsupported types are ignored)
    OIIO_CHECK_EQUAL( spec.find_attribute( "cameraMake" ), nullptr );

    // Check that original was removed
    OIIO_CHECK_EQUAL( spec.find_attribute( "Make" ), nullptr );
}

int main( int, char ** )
{
    std::cout << "Starting Test_ImageConverter tests..." << std::endl;

    try
    {
        std::cout << "Running test_collect_image_files_directory..."
                  << std::endl;
        test_collect_image_files_directory();
        std::cout << "✓ test_collect_image_files_directory passed" << std::endl;

        std::cout << "Running test_collect_image_files_single_file..."
                  << std::endl;
        test_collect_image_files_single_file();
        std::cout << "✓ test_collect_image_files_single_file passed"
                  << std::endl;

        std::cout << "Running test_collect_image_files_nonexistent_path..."
                  << std::endl;
        test_collect_image_files_nonexistent_path();
        std::cout << "✓ test_collect_image_files_nonexistent_path passed"
                  << std::endl;

        std::cout << "Running test_collect_image_files_empty_directory..."
                  << std::endl;
        test_collect_image_files_empty_directory();
        std::cout << "✓ test_collect_image_files_empty_directory passed"
                  << std::endl;

        std::cout
            << "Running test_collect_image_files_directory_with_only_filtered_files..."
            << std::endl;
        test_collect_image_files_directory_with_only_filtered_files();
        std::cout
            << "✓ test_collect_image_files_directory_with_only_filtered_files passed"
            << std::endl;

        std::cout << "Running test_database_paths_default..." << std::endl;
        test_database_paths_default();
        std::cout << "✓ test_database_paths_default passed" << std::endl;

        std::cout << "Running test_database_paths_rawtoaces_env..."
                  << std::endl;
        test_database_paths_rawtoaces_env();
        std::cout << "✓ test_database_paths_rawtoaces_env passed" << std::endl;

        std::cout << "Running test_database_paths_ampas_env..." << std::endl;
        test_database_paths_ampas_env();
        std::cout << "✓ test_database_paths_ampas_env passed" << std::endl;

        std::cout << "Running test_database_paths_both_env..." << std::endl;
        test_database_paths_both_env();
        std::cout << "✓ test_database_paths_both_env passed" << std::endl;

        std::cout << "Running test_database_paths_windows_separator..."
                  << std::endl;
        test_database_paths_windows_separator();
        std::cout << "✓ test_database_paths_windows_separator passed"
                  << std::endl;

        std::cout << "Running test_fix_metadata_both_attributes..."
                  << std::endl;
        test_fix_metadata_both_attributes();
        std::cout << "✓ test_fix_metadata_both_attributes passed" << std::endl;

        std::cout << "Running test_fix_metadata_float_make..." << std::endl;
        test_fix_metadata_float_make();
        std::cout << "✓ test_fix_metadata_float_make passed" << std::endl;

        std::cout << "Running test_fix_metadata_destination_exists..."
                  << std::endl;
        test_fix_metadata_destination_exists();
        std::cout << "✓ test_fix_metadata_destination_exists passed"
                  << std::endl;

        std::cout << "Running test_fix_metadata_source_missing..." << std::endl;
        test_fix_metadata_source_missing();
        std::cout << "✓ test_fix_metadata_source_missing passed" << std::endl;

        std::cout << "Running test_fix_metadata_unsupported_type..."
                  << std::endl;
        test_fix_metadata_unsupported_type();
        std::cout << "✓ test_fix_metadata_unsupported_type passed" << std::endl;

        std::cout << "All tests completed successfully!" << std::endl;
    }
    catch ( const std::exception &e )
    {
        std::cerr << "Exception caught in main: " << e.what() << std::endl;
        return 1;
    }
    catch ( ... )
    {
        std::cerr << "Unknown exception caught in main" << std::endl;
        return 1;
    }

    return unit_test_failures;
}
