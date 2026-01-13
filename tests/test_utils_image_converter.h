// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include <OpenImageIO/typedesc.h>
#include <OpenImageIO/imageio.h>

#include <rawtoaces/image_converter.h>

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
