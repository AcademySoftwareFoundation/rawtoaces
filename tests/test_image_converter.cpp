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

int main(int, char**)
{
    test_collect_image_files_directory();
    test_collect_image_files_single_file();
    test_collect_image_files_nonexistent_path();
    test_collect_image_files_empty_directory();
    test_collect_image_files_directory_with_only_filtered_files();
    
    return unit_test_failures;
}
