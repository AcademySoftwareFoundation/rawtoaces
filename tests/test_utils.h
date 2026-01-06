// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <vector>
#include <ctime>
#include <nlohmann/json.hpp>

// Include headers for complete types needed in class members
#include <OpenImageIO/typedesc.h> // For TypeDesc
#include <OpenImageIO/imageio.h>  // For ImageSpec
#include <rawtoaces/image_converter.h>

/// RAII (Resource Acquisition Is Initialization)
/// helper class for test directory management
class TestDirectory
{
public:
    TestDirectory();
    ~TestDirectory();

    // Disable copy constructor and assignment operator
    TestDirectory( const TestDirectory & )            = delete;
    TestDirectory &operator=( const TestDirectory & ) = delete;

    const std::string &path() const;
    const std::string &get_database_path() const;

    void create_test_files();
    void create_filtered_files_only();
    void create_valid_files( const std::vector<std::string> &filenames );

    /// Creates a test data file (camera or illuminant) with the specified header data
    /// @param type The type of test data to create (e.g. camera or illuminant)
    /// @param header_data JSON object containing the header data to include
    /// @param is_incorrect_data Whether to create incorrect data (for testing error cases)
    /// @return The full path to the created file
    std::string create_test_data_file(
        const std::string    &type,
        const nlohmann::json &header_data = { { "schema_version", "1.0.0" } },
        const bool            is_incorrect_data = false );

private:
    std::string test_dir;
    std::string database_dir;
};

/// Wrapper function that captures stderr output from a callable action
/// @param action A callable (function, lambda, etc.) that may write to stderr
/// @return The captured stderr output as a string
std::string capture_stderr( std::function<void()> action );

/// Wrapper function that captures stdout output from a callable action
/// @param action A callable (function, lambda, etc.) that may write to stdout
/// @return The captured stdout output as a string
std::string capture_stdout( std::function<void()> action );

// ============================================================================
// DSL Builders for Test Setup
// ============================================================================

/// Builder for creating test fixtures with database files
/// Usage:
///   TestFixture fixture;
///   auto &test_dir = fixture.with_camera("X", "Y").build(); // training and observer included by default
///   auto &test_dir = fixture.with_camera("X", "Y").without_training().build(); // exclude training
/// Note: The TestFixture object must be kept alive for the test duration to avoid dangling references.
class TestFixture
{
public:
    TestFixture();
    ~TestFixture();

    // Disable copy
    TestFixture( const TestFixture & )            = delete;
    TestFixture &operator=( const TestFixture & ) = delete;

    /// Add a camera data file
    TestFixture &with_camera(
        const std::string &make,
        const std::string &model,
        bool               is_incorrect = false );

    /// Remove training data file (training is included by default)
    TestFixture &without_training();

    /// Remove observer data file (observer is included by default)
    TestFixture &without_observer();

    /// Add illuminant data file
    TestFixture &
    with_illuminant( const std::string &type, bool is_incorrect = false );

    /// Add illuminant data file with custom header data
    TestFixture &with_illuminant_custom(
        const nlohmann::json &header_data, bool is_incorrect = false );

    /// Build and return the TestDirectory
    TestDirectory &build();

private:
    TestDirectory *test_dir_;
    bool           include_training_ = true; /// Default to including training
    bool           include_observer_ = true; /// Default to including observer
};

/// Builder for creating ImageSpec objects
/// Usage: auto spec = ImageSpecBuilder().size(100, 100).channels(3).camera("X", "Y").build();
class ImageSpecBuilder
{
public:
    ImageSpecBuilder();

    /// Set image dimensions
    ImageSpecBuilder &size( int width, int height );

    /// Set number of channels
    ImageSpecBuilder &channels( int n );

    /// Set pixel format
    ImageSpecBuilder &format( const OIIO::TypeDesc &fmt );

    /// Set camera metadata
    ImageSpecBuilder &
    camera( const std::string &make, const std::string &model );

    /// Add raw:pre_mul attribute
    ImageSpecBuilder &raw_pre_mul( const float *values, size_t count );

    /// Build the ImageSpec
    OIIO::ImageSpec build();

private:
    OIIO::ImageSpec spec_;
};

/// Builder for creating ImageConverter::Settings
/// Usage: auto settings = SettingsBuilder().database(path).illuminant("D65").verbosity(1).build();
class SettingsBuilder
{
public:
    SettingsBuilder();

    /// Set database directory path
    SettingsBuilder &database( const std::string &path );

    /// Set illuminant
    SettingsBuilder &illuminant( const std::string &illum );

    /// Set verbosity level
    SettingsBuilder &verbosity( int level );

    /// Set white balance method
    SettingsBuilder &wb_method( const std::string &method );

    /// Set matrix method
    SettingsBuilder &mat_method( const std::string &method );

    /// Set overwrite flag
    SettingsBuilder &overwrite( bool value = true );

    /// Build the Settings
    rta::util::ImageConverter::Settings build();

private:
    rta::util::ImageConverter::Settings settings_;
};

/// Builder for creating command-line argument vectors
/// Usage: auto args = CommandBuilder().wb_method("illuminant").illuminant("D65").input(file).build();
class CommandBuilder
{
public:
    CommandBuilder();

    /// Add white balance method argument
    CommandBuilder &wb_method( const std::string &method );

    /// Add illuminant argument
    CommandBuilder &illuminant( const std::string &illum );

    /// Add matrix method argument
    CommandBuilder &mat_method( const std::string &method );

    /// Remove verbose flag (verbose is included by default)
    CommandBuilder &without_verbose();

    /// Remove overwrite flag (overwrite is included by default)
    CommandBuilder &without_overwrite();

    /// Add input file
    CommandBuilder &input( const std::string &file );

    /// Add output file
    CommandBuilder &output( const std::string &file );

    /// Add custom camera make
    CommandBuilder &custom_camera_make( const std::string &make );

    /// Add custom camera model
    CommandBuilder &custom_camera_model( const std::string &model );

    /// Add data directory
    CommandBuilder &data_dir( const std::string &path );

    /// Add arbitrary flag/argument
    CommandBuilder &arg( const std::string &arg );

    /// Build the argument vector
    std::vector<std::string> build();

private:
    std::vector<std::string> args_;
    bool include_verbose_   = true; /// Default to including verbose
    bool include_overwrite_ = true; /// Default to including overwrite
};

// ============================================================================
// Assertion Helpers
// ============================================================================

/// Assert that output contains the specified text
/// @param output The output string to check
/// @param text The text that must be present
void assert_contains( const std::string &output, const std::string &text );

/// Assert that output does not contain the specified text
/// @param output The output string to check
/// @param text The text that must not be present
void assert_not_contains( const std::string &output, const std::string &text );

/// Assert that output contains all specified texts
/// @param output The output string to check
/// @param texts Vector of texts that must all be present
void assert_contains_all(
    const std::string &output, const std::vector<std::string> &texts );

/// Filter out empty and whitespace-only lines from a vector of strings
/// @param lines Vector of strings to filter (modified in-place)
void filter_empty_lines( std::vector<std::string> &lines );

/// Split output string into lines, optionally filtering empty lines
/// @param output The output string to split
/// @param filter_empty If true, filter out empty and whitespace-only lines
/// @return Vector of lines
std::vector<std::string>
get_output_lines( const std::string &output, bool filter_empty = true );

/// Macro wrappers for convenience
#define ASSERT_CONTAINS( output, text ) assert_contains( output, text )
#define ASSERT_NOT_CONTAINS( output, text ) assert_not_contains( output, text )
#define ASSERT_CONTAINS_ALL( output, texts )                                   \
    assert_contains_all( output, texts )
