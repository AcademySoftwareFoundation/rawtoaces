// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "test_utils.h"

#include <OpenImageIO/unittest.h>
#include <OpenImageIO/strutil.h>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <vector>
#include <ctime>
#include <nlohmann/json.hpp>

// Forward declare ImageConverter::Settings
#include <rawtoaces/image_converter.h>
#include <OpenImageIO/imageio.h>

/// RAII helper class to capture stderr output for testing
class StderrCapture
{
public:
    StderrCapture() : buffer(), old( std::cerr.rdbuf( buffer.rdbuf() ) ) {}

    ~StderrCapture() { std::cerr.rdbuf( old ); }

    /// Get the captured output as a string
    std::string str() const { return buffer.str(); }

private:
    std::stringstream buffer;
    std::streambuf   *old;
};

/// RAII helper class to capture stdout output for testing
class StdoutCapture
{
public:
    StdoutCapture() : buffer(), old( std::cout.rdbuf( buffer.rdbuf() ) ) {}

    ~StdoutCapture() { std::cout.rdbuf( old ); }

    /// Get the captured output as a string
    std::string str() const { return buffer.str(); }

private:
    std::stringstream buffer;
    std::streambuf   *old;
};

/// Wrapper function that captures stderr output from a callable action
std::string capture_stderr( std::function<void()> action )
{
    StderrCapture capture;
    action();
    return capture.str();
}

/// Wrapper function that captures stdout output from a callable action
std::string capture_stdout( std::function<void()> action )
{
    StdoutCapture capture;
    action();
    return capture.str();
}

// ============================================================================
// TestDirectory Implementation
// ============================================================================

TestDirectory::TestDirectory()
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

TestDirectory::~TestDirectory()
{
    std::filesystem::remove_all( test_dir );
}

const std::string &TestDirectory::path() const
{
    return test_dir;
}

const std::string &TestDirectory::get_database_path() const
{
    return database_dir;
}

void TestDirectory::create_test_files()
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
    std::filesystem::create_symlink( "test1.raw", test_dir + "/symlink.raw" );
#endif

    // Create a subdirectory (should be ignored)
    std::filesystem::create_directories( test_dir + "/subdir" );
    std::ofstream( test_dir + "/subdir/test8.raw" ).close();
}

void TestDirectory::create_filtered_files_only()
{
    // Create only files that should be filtered out
    std::ofstream( test_dir + "/.DS_Store" ).close();
    std::ofstream( test_dir + "/test.exr" ).close();
    std::ofstream( test_dir + "/test.jpg" ).close();
    std::ofstream( test_dir + "/test.jpeg" ).close();
}

void TestDirectory::create_valid_files(
    const std::vector<std::string> &filenames )
{
    for ( const auto &filename: filenames )
    {
        std::ofstream( test_dir + "/" + filename ).close();
    }
}

std::string TestDirectory::create_test_data_file(
    const std::string    &type,
    const nlohmann::json &header_data,
    const bool            is_incorrect_data )
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
        filename =
            "test_" + type + "_" + std::to_string( ++file_counter ) + ".json";
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

            data_main[std::to_string( wavelength )] = { x_val, y_val, z_val };
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

// ============================================================================
// TestFixture Implementation
// ============================================================================

TestFixture::TestFixture() : test_dir_( new TestDirectory() )
{
    /// Default to including training and observer (most common case)
    include_training_ = true;
    include_observer_ = true;
}

TestFixture::~TestFixture()
{
    delete test_dir_;
}

TestFixture &TestFixture::with_camera(
    const std::string &make, const std::string &model, bool is_incorrect )
{
    test_dir_->create_test_data_file(
        "camera",
        { { "manufacturer", make }, { "model", model } },
        is_incorrect );
    return *this;
}

TestFixture &TestFixture::without_training()
{
    /// Exclude training from defaults
    include_training_ = false;
    return *this;
}

TestFixture &TestFixture::without_observer()
{
    /// Exclude observer from defaults
    include_observer_ = false;
    return *this;
}

TestFixture &
TestFixture::with_illuminant( const std::string &type, bool is_incorrect )
{
    test_dir_->create_test_data_file(
        "illuminant", { { "type", type } }, is_incorrect );
    return *this;
}

TestFixture &TestFixture::with_illuminant_custom(
    const nlohmann::json &header_data, bool is_incorrect )
{
    test_dir_->create_test_data_file( "illuminant", header_data, is_incorrect );
    return *this;
}

TestDirectory &TestFixture::build()
{
    /// Create training and observer files if they should be included
    if ( include_training_ )
    {
        test_dir_->create_test_data_file( "training" );
    }
    if ( include_observer_ )
    {
        test_dir_->create_test_data_file( "cmf" );
    }
    return *test_dir_;
}

// ============================================================================
// ImageSpecBuilder Implementation
// ============================================================================

ImageSpecBuilder::ImageSpecBuilder() : spec_()
{
    /// Default to commonly used test values
    spec_.width     = 100;
    spec_.height    = 100;
    spec_.nchannels = 3;
    spec_.format    = OIIO::TypeDesc::UINT8;
}

ImageSpecBuilder &ImageSpecBuilder::size( int width, int height )
{
    spec_.width  = width;
    spec_.height = height;
    return *this;
}

ImageSpecBuilder &ImageSpecBuilder::channels( int n )
{
    spec_.nchannels = n;
    return *this;
}

ImageSpecBuilder &ImageSpecBuilder::format( const OIIO::TypeDesc &fmt )
{
    spec_.format = fmt;
    return *this;
}

ImageSpecBuilder &
ImageSpecBuilder::camera( const std::string &make, const std::string &model )
{
    spec_["cameraMake"]  = make;
    spec_["cameraModel"] = model;
    return *this;
}

ImageSpecBuilder &
ImageSpecBuilder::raw_pre_mul( const float *values, size_t count )
{
    spec_.attribute(
        "raw:pre_mul",
        OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, static_cast<int>( count ) ),
        values );
    return *this;
}

OIIO::ImageSpec ImageSpecBuilder::build()
{
    return spec_;
}

// ============================================================================
// SettingsBuilder Implementation
// ============================================================================

SettingsBuilder::SettingsBuilder() : settings_()
{
    /// Default to commonly used test values
    settings_.verbosity  = 1;
    settings_.illuminant = "D65";
}

SettingsBuilder &SettingsBuilder::database( const std::string &path )
{
    settings_.database_directories = { path };
    return *this;
}

SettingsBuilder &SettingsBuilder::illuminant( const std::string &illum )
{
    settings_.illuminant = illum;
    return *this;
}

SettingsBuilder &SettingsBuilder::verbosity( int level )
{
    settings_.verbosity = level;
    return *this;
}

SettingsBuilder &SettingsBuilder::wb_method( const std::string &method )
{
    if ( method == "metadata" )
    {
        settings_.WB_method =
            rta::util::ImageConverter::Settings::WBMethod::Metadata;
    }
    else if ( method == "illuminant" )
    {
        settings_.WB_method =
            rta::util::ImageConverter::Settings::WBMethod::Illuminant;
    }
    else if ( method == "box" )
    {
        settings_.WB_method =
            rta::util::ImageConverter::Settings::WBMethod::Box;
    }
    else if ( method == "custom" )
    {
        settings_.WB_method =
            rta::util::ImageConverter::Settings::WBMethod::Custom;
    }
    return *this;
}

SettingsBuilder &SettingsBuilder::mat_method( const std::string &method )
{
    if ( method == "auto" )
    {
        settings_.matrix_method =
            rta::util::ImageConverter::Settings::MatrixMethod::Auto;
    }
    else if ( method == "spectral" )
    {
        settings_.matrix_method =
            rta::util::ImageConverter::Settings::MatrixMethod::Spectral;
    }
    else if ( method == "custom" )
    {
        settings_.matrix_method =
            rta::util::ImageConverter::Settings::MatrixMethod::Custom;
    }
    return *this;
}

SettingsBuilder &SettingsBuilder::overwrite( bool value )
{
    settings_.overwrite = value;
    return *this;
}

rta::util::ImageConverter::Settings SettingsBuilder::build()
{
    return settings_;
}

// ============================================================================
// CommandBuilder Implementation
// ============================================================================

CommandBuilder::CommandBuilder() : args_()
{
    /// Default to commonly used test flags (flags only, not key-value pairs to avoid duplicates)
    include_verbose_   = true;
    include_overwrite_ = true;
}

CommandBuilder &CommandBuilder::wb_method( const std::string &method )
{
    args_.push_back( "--wb-method" );
    args_.push_back( method );
    return *this;
}

CommandBuilder &CommandBuilder::illuminant( const std::string &illum )
{
    args_.push_back( "--illuminant" );
    args_.push_back( illum );
    return *this;
}

CommandBuilder &CommandBuilder::mat_method( const std::string &method )
{
    args_.push_back( "--mat-method" );
    args_.push_back( method );
    return *this;
}

CommandBuilder &CommandBuilder::without_verbose()
{
    /// Exclude verbose from defaults
    include_verbose_ = false;
    return *this;
}

CommandBuilder &CommandBuilder::without_overwrite()
{
    /// Exclude overwrite from defaults
    include_overwrite_ = false;
    return *this;
}

CommandBuilder &CommandBuilder::input( const std::string &file )
{
    args_.push_back( file );
    return *this;
}

CommandBuilder &CommandBuilder::output( const std::string &file )
{
    args_.push_back( file );
    return *this;
}

CommandBuilder &CommandBuilder::custom_camera_make( const std::string &make )
{
    args_.push_back( "--custom-camera-make" );
    args_.push_back( make );
    return *this;
}

CommandBuilder &CommandBuilder::custom_camera_model( const std::string &model )
{
    args_.push_back( "--custom-camera-model" );
    args_.push_back( model );
    return *this;
}

CommandBuilder &CommandBuilder::data_dir( const std::string &path )
{
    args_.push_back( "--data-dir" );
    args_.push_back( path );
    return *this;
}

CommandBuilder &CommandBuilder::arg( const std::string &arg )
{
    args_.push_back( arg );
    return *this;
}

std::vector<std::string> CommandBuilder::build()
{
    /// Add verbose and overwrite flags if they should be included
    if ( include_verbose_ )
    {
        args_.push_back( "--verbose" );
    }
    if ( include_overwrite_ )
    {
        args_.push_back( "--overwrite" );
    }
    return args_;
}

// ============================================================================
// Assertion Helpers Implementation
// ============================================================================

void assert_contains( const std::string &output, const std::string &text )
{
    if ( output.find( text ) == std::string::npos )
    {
        std::string error_msg = "Expected output to contain: " + text;
        OIIO_CHECK_ASSERT( false && error_msg.c_str() );
    }
}

void assert_not_contains( const std::string &output, const std::string &text )
{
    if ( output.find( text ) != std::string::npos )
    {
        std::string error_msg = "Expected output to not contain: " + text;
        OIIO_CHECK_ASSERT( false && error_msg.c_str() );
    }
}

void assert_contains_all(
    const std::string &output, const std::vector<std::string> &texts )
{
    for ( const auto &text: texts )
    {
        assert_contains( output, text );
    }
}

void filter_empty_lines( std::vector<std::string> &lines )
{
    lines.erase(
        std::remove_if(
            lines.begin(),
            lines.end(),
            []( const std::string &s ) {
                return s.empty() ||
                       s.find_first_not_of( " \t\r\n" ) == std::string::npos;
            } ),
        lines.end() );
}

std::vector<std::string>
get_output_lines( const std::string &output, bool filter_empty )
{
    std::vector<std::string> lines;
    OIIO::Strutil::split( output, lines, "\n" );
    if ( filter_empty )
    {
        filter_empty_lines( lines );
    }
    return lines;
}
