// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "test_utils_image_converter.h"

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

CommandBuilder &CommandBuilder::output_dir( const std::string &path )
{
    args_.push_back( "--output-dir" );
    args_.push_back( path );
    return *this;
}

CommandBuilder &CommandBuilder::without_create_dirs()
{
    /// Note: create-dirs is not included by default, so this is mainly for clarity
    /// If needed in future, we can track this state to exclude --create-dirs explicitly
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
