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

TestFile::TestFile( const std::string &dir, const std::string &filename )
{
    file_path = ( std::filesystem::path( dir ) / filename ).string();
}

TestFile::~TestFile()
{
    std::filesystem::remove( file_path );
}

const std::string &TestFile::path() const
{
    return file_path;
}

void TestFile::write( const std::string &contents ) const
{
    std::ofstream out( file_path );
    out << contents;
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
    const std::string                   &type,
    const nlohmann::json                &header_data,
    const std::optional<nlohmann::json> &index_main_override,
    const std::optional<nlohmann::json> &data_main_override )
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

    nlohmann::json index_main;
    if ( index_main_override.has_value() )
    {
        index_main = index_main_override.value();
    }
    else if ( type == "camera" )
    {
        index_main = { "R", "G", "B" };
    }
    else if ( type == "training" )
    {
        index_main = { "patch1", "patch2", "patch3" };
    }
    else if ( type == "cmf" )
    {
        index_main = { "X", "Y", "Z" };
    }
    else if ( type == "illuminant" )
    {
        index_main = { "power" };
    }
    else
    {
        index_main = nlohmann::json::array();
    }
    spectral_data["index"] = { { "main", index_main } };

    if ( data_main_override.has_value() )
    {
        spectral_data["data"]["main"] = data_main_override.value();
    }
    else if ( !index_main.empty() )
    {
        nlohmann::json data_main;
        const size_t   channels = index_main.size();
        for ( int wavelength = 380; wavelength <= 780; wavelength += 5 )
        {
            nlohmann::json values = nlohmann::json::array();
            if ( type == "illuminant" )
            {
                double power_val = 1.0 + ( wavelength - 380 ) * 0.01;
                for ( size_t c = 0; c < channels; c++ )
                    values.push_back( power_val );
            }
            else
            {
                double v1 = 0.1 + ( wavelength - 380 ) * 0.001;
                double v2 = 0.2 + ( wavelength - 380 ) * 0.001;
                double v3 = 0.3 + ( wavelength - 380 ) * 0.001;
                if ( channels > 0 )
                    values.push_back( v1 );
                if ( channels > 1 )
                    values.push_back( v2 );
                if ( channels > 2 )
                    values.push_back( v3 );
                for ( size_t c = 3; c < channels; c++ )
                    values.push_back( 1.0 );
            }

            data_main[std::to_string( wavelength )] = values;
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
    const std::string                   &make,
    const std::string                   &model,
    const std::optional<nlohmann::json> &index_main_override,
    const std::optional<nlohmann::json> &data_main_override )
{
    test_dir_->create_test_data_file(
        "camera",
        { { "manufacturer", make }, { "model", model } },
        index_main_override,
        data_main_override );
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

TestFixture &TestFixture::with_illuminant(
    const std::string                   &type,
    const std::optional<nlohmann::json> &index_main_override,
    const std::optional<nlohmann::json> &data_main_override )
{
    test_dir_->create_test_data_file(
        "illuminant",
        { { "type", type } },
        index_main_override,
        data_main_override );
    return *this;
}

TestFixture &TestFixture::with_illuminant_custom(
    const nlohmann::json                &header_data,
    const std::optional<nlohmann::json> &index_main_override,
    const std::optional<nlohmann::json> &data_main_override )
{
    test_dir_->create_test_data_file(
        "illuminant", header_data, index_main_override, data_main_override );
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

#ifdef WIN32
void set_env_var( const std::string &name, const std::string &value )
{
    _putenv_s( name.c_str(), value.c_str() );
}

void unset_env_var( const std::string &name )
{
    _putenv_s( name.c_str(), "" );
}
#else

void set_env_var( const std::string &name, const std::string &value )
{
    setenv( name.c_str(), value.c_str(), 1 );
}

void unset_env_var( const std::string &name )
{
    unsetenv( name.c_str() );
}
#endif
