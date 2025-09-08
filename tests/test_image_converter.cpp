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
#include <sys/stat.h>  // for mkfifo
#include <cstdlib>     // for setenv, unsetenv

#include "../src/rawtoaces_util/rawtoaces_util_priv.h"

using namespace rta::util;

/// RAII (Resource Acquisition Is Initialization) 
/// helper class for test directory management
class TestDirectory
{
public:
    TestDirectory() 
    {
        test_dir = std::filesystem::temp_directory_path() / "rawtoaces_test";
        std::filesystem::create_directories(test_dir);
    }
    
    ~TestDirectory()
    {   
        std::filesystem::remove_all(test_dir);
    }
    
    // Disable copy constructor and assignment operator
    TestDirectory(const TestDirectory&) = delete;
    TestDirectory& operator=(const TestDirectory&) = delete;
    
    const std::string& path() const { return test_dir; }
    
    void create_test_files()
    {
        // Create valid image files (different extensions)
        std::ofstream(test_dir + "/test1.raw").close();
        std::ofstream(test_dir + "/test2.cr2").close();
        std::ofstream(test_dir + "/test3.nef").close();
        std::ofstream(test_dir + "/test4.dng").close();
        
        // Create files that should be filtered out
        std::ofstream(test_dir + "/.DS_Store").close();
        std::ofstream(test_dir + "/test5.exr").close();
        std::ofstream(test_dir + "/test6.jpg").close();
        std::ofstream(test_dir + "/test7.jpeg").close();
        std::ofstream(test_dir + "/test8.EXR").close();
        std::ofstream(test_dir + "/test9.JPG").close();
        std::ofstream(test_dir + "/test10.JPEG").close();
        
        // Create a symlink to the regular file
        std::filesystem::create_symlink("test1.raw", test_dir + "/symlink.raw");

        // Create a subdirectory (should be ignored)
        std::filesystem::create_directories(test_dir + "/subdir");
        std::ofstream(test_dir + "/subdir/test8.raw").close();
    }
    
    void create_filtered_files_only()
    {
        // Create only files that should be filtered out
        std::ofstream(test_dir + "/.DS_Store").close();
        std::ofstream(test_dir + "/test.exr").close();
        std::ofstream(test_dir + "/test.jpg").close();
        std::ofstream(test_dir + "/test.jpeg").close();
    }
    
    void create_valid_files(const std::vector<std::string>& filenames)
    {
        for (const auto& filename : filenames) {
            std::ofstream(test_dir + "/" + filename).close();
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
    bool result = collect_image_files(test_dir.path(), batches);
    
    OIIO_CHECK_EQUAL(result, true);
    OIIO_CHECK_EQUAL(batches.size(), 1);
    OIIO_CHECK_EQUAL(batches[0].size(), 5); // Should have 6 valid files
    
    // Check that the correct files are included
    std::vector<std::string> expected_files = {
        test_dir.path() + "/test1.raw",
        test_dir.path() + "/test2.cr2", 
        test_dir.path() + "/test3.nef",
        test_dir.path() + "/test4.dng",
        test_dir.path() + "/symlink.raw"
    };
    
    for (const auto& expected : expected_files) {
        bool found = false;
        for (const auto& actual : batches[0]) {
            if (actual == expected) {
                found = true;
                break;
            }
        }
        OIIO_CHECK_EQUAL(found, true);
    }
}

/// Ensures that when given a single file path (not a directory), collect_image_files
/// properly validates the file and creates a batch containing just that one file
void test_collect_image_files_single_file()
{
    TestDirectory test_dir;
    std::string test_file = test_dir.path() + "/test.raw";
    std::ofstream(test_file).close();
    
    std::vector<std::vector<std::string>> batches;
    bool result = collect_image_files(test_file, batches);
    
    OIIO_CHECK_EQUAL(result, true);
    OIIO_CHECK_EQUAL(batches.size(), 1);
    OIIO_CHECK_EQUAL(batches[0].size(), 1);
    OIIO_CHECK_EQUAL(batches[0][0], test_file);
}

/// Verifies that collect_image_files returns false and creates no batches when given
/// a path that doesn't exist, ensuring proper error handling for invalid input paths
void test_collect_image_files_nonexistent_path()
{
    std::vector<std::vector<std::string>> batches;
    bool result = collect_image_files("nonexistent_path", batches);
    
    OIIO_CHECK_EQUAL(result, false);
    OIIO_CHECK_EQUAL(batches.size(), 0);
}

/// Ensures that when given an empty directory, collect_image_files returns true but
/// creates an empty batch, maintaining consistent behavior for edge cases
void test_collect_image_files_empty_directory()
{
    TestDirectory test_dir;
    
    std::vector<std::vector<std::string>> batches;
    bool result = collect_image_files(test_dir.path(), batches);
    
    OIIO_CHECK_EQUAL(result, true);
    OIIO_CHECK_EQUAL(batches.size(), 1);
    OIIO_CHECK_EQUAL(batches[0].size(), 0); // Empty directory should result in empty batch
}

/// Verifies that when a directory contains only files that should be filtered out
/// (like .DS_Store, .jpg, .exr), collect_image_files still returns true but creates an empty batch,
/// ensuring the filtering logic works correctly in real-world scenarios
void test_collect_image_files_directory_with_only_filtered_files()
{
    TestDirectory test_dir;
    test_dir.create_filtered_files_only();
    
    std::vector<std::vector<std::string>> batches;
    bool result = collect_image_files(test_dir.path(), batches);
    
    OIIO_CHECK_EQUAL(result, true);
    OIIO_CHECK_EQUAL(batches.size(), 1);
    OIIO_CHECK_EQUAL(batches[0].size(), 0); // All files filtered out
}

/// Tests database_paths with no environment variables set (uses default paths)
void test_database_paths_default()
{
    // Clear environment variables to test default behavior
    unsetenv("RAWTOACES_DATA_PATH");
    unsetenv("AMPAS_DATA_PATH");
    
    std::vector<std::string> paths = database_paths();
    
    // Should have at least one default path
    OIIO_CHECK_EQUAL(paths.empty(), false);
    
    // On Unix systems, should have both new and legacy paths
    #if !defined(WIN32) && !defined(WIN64)
    OIIO_CHECK_EQUAL(paths.size(), 2);
    OIIO_CHECK_EQUAL(paths[0], "/usr/local/share/rawtoaces/data");
    OIIO_CHECK_EQUAL(paths[1], "/usr/local/include/rawtoaces/data");
    #else
    // On Windows, should have just the current directory
    OIIO_CHECK_EQUAL(paths.size(), 1);
    OIIO_CHECK_EQUAL(paths[0], ".");
    #endif
}

/// Tests database_paths with RAWTOACES_DATA_PATH environment variable set
void test_database_paths_rawtoaces_env()
{
    // Set RAWTOACES_DATA_PATH
    setenv("RAWTOACES_DATA_PATH", "/custom/path1:/custom/path2", 1);
    unsetenv("AMPAS_DATA_PATH");
    
    std::vector<std::string> paths = database_paths();
    
    OIIO_CHECK_EQUAL(paths.size(), 2);
    OIIO_CHECK_EQUAL(paths[0], "/custom/path1");
    OIIO_CHECK_EQUAL(paths[1], "/custom/path2");
    
    // Clean up
    unsetenv("RAWTOACES_DATA_PATH");
}

/// Tests database_paths with deprecated AMPAS_DATA_PATH environment variable
void test_database_paths_ampas_env()
{
    // Set AMPAS_DATA_PATH (deprecated)
    unsetenv("RAWTOACES_DATA_PATH");
    setenv("AMPAS_DATA_PATH", "/deprecated/path1:/deprecated/path2", 1);
    
    std::vector<std::string> paths = database_paths();
    
    OIIO_CHECK_EQUAL(paths.size(), 2);
    OIIO_CHECK_EQUAL(paths[0], "/deprecated/path1");
    OIIO_CHECK_EQUAL(paths[1], "/deprecated/path2");
    
    // Clean up
    unsetenv("AMPAS_DATA_PATH");
}

/// Tests database_paths with both environment variables set (RAWTOACES_DATA_PATH should take precedence)
void test_database_paths_both_env()
{
    // Set both environment variables
    setenv("RAWTOACES_DATA_PATH", "/preferred/path1:/preferred/path2", 1);
    setenv("AMPAS_DATA_PATH", "/deprecated/path1:/deprecated/path2", 1);
    
    std::vector<std::string> paths = database_paths();
    
    // RAWTOACES_DATA_PATH should take precedence
    OIIO_CHECK_EQUAL(paths.size(), 2);
    OIIO_CHECK_EQUAL(paths[0], "/preferred/path1");
    OIIO_CHECK_EQUAL(paths[1], "/preferred/path2");
    
    // Clean up
    unsetenv("RAWTOACES_DATA_PATH");
    unsetenv("AMPAS_DATA_PATH");
}

/// Tests database_paths with Windows-style path separator
void test_database_paths_windows_separator()
{
    // Set RAWTOACES_DATA_PATH with Windows-style separator
    setenv("RAWTOACES_DATA_PATH", "/path1;/path2;/path3", 1);
    
    std::vector<std::string> paths = database_paths();
    
    // Should split by the appropriate separator for the platform
    #if defined(WIN32) || defined(WIN64)
    OIIO_CHECK_EQUAL(paths.size(), 3);
    OIIO_CHECK_EQUAL(paths[0], "/path1");
    OIIO_CHECK_EQUAL(paths[1], "/path2");
    OIIO_CHECK_EQUAL(paths[2], "/path3");
    #else
    // On Unix, semicolon won't be split, so we get one path
    OIIO_CHECK_EQUAL(paths.size(), 1);
    OIIO_CHECK_EQUAL(paths[0], "/path1;/path2;/path3");
    #endif
    
    // Clean up
    unsetenv("RAWTOACES_DATA_PATH");
}

int main(int, char**)
{
    test_collect_image_files_directory();
    test_collect_image_files_single_file();
    test_collect_image_files_nonexistent_path();
    test_collect_image_files_empty_directory();
    test_collect_image_files_directory_with_only_filtered_files();
    
    test_database_paths_default();
    test_database_paths_rawtoaces_env();
    test_database_paths_ampas_env();
    test_database_paths_both_env();
    test_database_paths_windows_separator();
    
    return unit_test_failures;
}
