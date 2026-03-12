// Copyright Contributors to the rawtoaces project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/rawtoaces

#include "exiftool.h"

#include <iomanip>
#include <iostream>
#include <filesystem>

#include <OpenImageIO/imagebuf.h>

namespace rta
{
namespace util
{
namespace exiftool
{

std::string find_exiftool()
{
    const char *env = getenv( "RAWTOACES_EXIFTOOL_PATH" );
    if ( env != nullptr )
    {
        return env;
    }

#if defined( WIN32 ) || defined( WIN64 )
    const std::string delimiter = ";";
    const std::string name      = "exiftool.exe";
#else
    const std::string delimiter = ":";
    const std::string name      = "exiftool";
#endif

    env = getenv( "PATH" );
    if ( env )
    {
        std::string temp = env;

        std::vector<std::string> paths;
        size_t                   pos = 0;
        std::string              token;
        while ( ( pos = temp.find( delimiter ) ) != std::string::npos )
        {
            token = temp.substr( 0, pos );
            paths.push_back( token );
            temp.erase( 0, pos + delimiter.length() );
        }
        paths.push_back( temp );

        for ( const auto &path: paths )
        {
            std::filesystem::path p( path );
            p /= name;

            if ( std::filesystem::exists( p ) )
                return p.string();
        }
    }

    return "";
}

bool execute( const std::string &command, std::stringstream &stream )
{
    constexpr size_t buf_size = 255;
    char             buffer[buf_size];

    errno = 0;

#if defined( WIN32 ) || defined( WIN64 )
    FILE *file = _popen( command.c_str(), "r" );
#else
    // clang-format off
    FILE *file = popen( command.c_str(), "r" );
    // clang-format on
#endif

    bool success = ( errno == 0 );

    // Struggling to detect errors consistently across all platforms. In some
    // cases getting errno != 0 even when the command has been executed
    // successfully. As a workaround, I'm treating the empty output as error.
    bool empty = true;
    if ( success )
    {
        while ( fgets( buffer, buf_size, file ) != NULL )
        {
            stream << buffer;
            empty = false;
        }
    }

#if defined( WIN32 ) || defined( WIN64 )
    _pclose( file );
#else
    pclose( file );
#endif

    stream.flush();
    return success && !empty;
}

bool perform_exiftool_call(
    const std::string                  &exiftool_path,
    const std::string                  &image_path,
    const std::vector<std::string>     &keys,
    bool                                no_formatting,
    std::map<std::string, std::string> &parsed_data,
    std::string                        &error_message )
{
    std::string command = exiftool_path + " -S";

    if ( no_formatting )
    {
        command += " -n";
    }

    // Adding the file name to the requested attributes, so the result of a
    // successful call is never empty.
    command += " -FileName";

    // Mapping from OIIO attribute names to ExifTool attribute names.
    const std::map<std::string, std::string> oiio_to_exiftool = {
        { "cameraMake", "Make" },     { "cameraModel", "Model" },
        { "lensModel", "LensModel" }, { "lensID", "LensID" },
        { "aperture", "FNumber" },    { "focalLength", "FocalLength" }
    };

    for ( auto key: keys )
    {
        if ( key == "focus" )
        {
            command += " -FocusDistance";
            command += " -FocusDistanceUpper";
            command += " -FocusDistanceLower";
        }
        else
        {
            if ( oiio_to_exiftool.count( key ) )
            {
                command += " -" + oiio_to_exiftool.at( key );
            }
            else
            {
                error_message = "Exiftool: unknown key '" + key + "'.";
                return false;
            }
        }
    }

    command += " " + image_path;

    std::stringstream stream;
    if ( !execute( command, stream ) )
    {
        error_message =
            "Failed to execute exiftool. Please make sure that its location is "
            "available in PATH. Alternatively you can provide the path to the "
            "exiftool binary via the RAWTOACES_EXIFTOOL_PATH environment "
            "variable.";
        return false;
    }

    std::map<std::string, std::string> result;
    std::vector<std::string>           lines;
    std::string                        line;
    while ( std::getline( stream, line ) )
    {
        auto pos = line.find( ": " );
        if ( pos != line.npos )
        {
            std::string key   = line.substr( 0, pos );
            std::string value = line.substr( pos + 2 );
            parsed_data[key]  = value;
        }
    }

    return true;
}

bool fetch_metadata(
    OIIO::ImageSpec                &spec,
    const std::string              &path,
    const std::vector<std::string> &keys,
    std::string                    &error_message )
{
    std::string exiftool_path = find_exiftool();

    if ( exiftool_path.empty() )
    {
        error_message =
            "Exiftool not found, please make sure that its location is "
            "available in PATH. Alternatively you can provide the path to the "
            "exiftool binary via the RAWTOACES_EXIFTOOL_PATH environment "
            "variable.";
        return false;
    }

    std::map<std::string, std::string> parsed_data;
    if ( !perform_exiftool_call(
             exiftool_path, path, keys, true, parsed_data, error_message ) )
    {
        return false;
    }

    bool requested_lens_model =
        std::find( keys.begin(), keys.end(), "lensModel" ) != keys.end();
    if ( requested_lens_model )
    {
        bool found_lens_model =
            parsed_data.find( "LensModel" ) != parsed_data.end();
        if ( !found_lens_model )
        {
            // If the lens model name was requested but not returned by
            // exiftool, perform another call to fetch the LensID, but enable
            // the output formatting this time, so the vendor-specific lens
            // identifier gets converted into a human readable name.
            if ( !perform_exiftool_call(
                     exiftool_path,
                     path,
                     { "lensID" },
                     false,
                     parsed_data,
                     error_message ) )
            {
                return false;
            }
        }
    }

    // Mapping from ExifTool attribute names to OIIO attribute names,
    // the bool flag specifies whether the value must be converted
    // from string to float.
    const std::map<std::string, std::pair<std::string, bool>>
        exiftool_to_oiio = { { "Make", { "cameraMake", false } },
                             { "Model", { "cameraModel", false } },
                             { "LensModel", { "lensModel", false } },
                             { "LensID", { "lensModel", false } },
                             { "FNumber", { "aperture", true } },
                             { "FocalLength", { "focalLength", true } } };

    bool  do_focus_distance    = false;
    float focus_distance       = 0.0f;
    float focus_distance_upper = 0.0f;
    float focus_distance_lower = 0.0f;

    for ( auto [exiftool_key, value]: parsed_data )
    {
        if ( exiftool_key == "FocusDistance" )
        {
            do_focus_distance = true;
            focus_distance    = std::stof( value );
        }
        else if ( exiftool_key == "FocusDistanceUpper" )
        {
            do_focus_distance    = true;
            focus_distance_upper = std::stof( value );
        }
        else if ( exiftool_key == "FocusDistanceLower" )
        {
            do_focus_distance    = true;
            focus_distance_lower = std::stof( value );
        }
        else if ( exiftool_key != "FileName" )
        {
            assert( exiftool_to_oiio.count( exiftool_key ) );

            auto              &map_entry = exiftool_to_oiio.at( exiftool_key );
            const std::string &oiio_key  = std::get<0>( map_entry );
            bool               to_float  = std::get<1>( map_entry );

            if ( to_float )
            {
                spec[oiio_key] = std::stof( value );
            }
            else
            {
                spec[oiio_key] = value;
            }
        }
    }

    if ( do_focus_distance )
    {
        if ( focus_distance == 0.0f )
        {
            if ( focus_distance_upper > 0 )
            {
                if ( focus_distance_lower > 0 )
                {
                    focus_distance = 2.0f / ( 1.0f / focus_distance_lower +
                                              1.0f / focus_distance_upper );
                }
                else
                {
                    focus_distance = focus_distance_upper;
                }
            }
            else
            {
                if ( focus_distance_lower > 0 )
                {
                    focus_distance = focus_distance_lower;
                }
            }
        }

        spec["focus"] = focus_distance;
    }

    return true;
}

} // namespace exiftool
} // namespace util
} // namespace rta
