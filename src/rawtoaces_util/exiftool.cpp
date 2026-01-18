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
    char             buffer1[buf_size];
    //    std::stringstream stream;

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
        while ( fgets( buffer1, buf_size, file ) != NULL )
        {
            stream << buffer1;
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

bool fetch_metadata(
    OIIO::ImageSpec                &spec,
    const std::string              &path,
    const std::vector<std::string> &keys )
{

    std::string exiftool_path = find_exiftool();

    if ( exiftool_path.empty() )
    {
        std::cerr
            << "Exiftool not found, please provide the path to "
            << "exiftool binary via the RAWTOACES_EXIFTOOL_PATH environment "
            << "variable." << std::endl;
        return false;
    }

    // Mapping from OIIO attribute names to ExifTool attribute names.
    const std::map<std::string, std::string> oiio_to_exiftool = {
        { "cameraMake", "Make" },
        { "cameraModel", "Model" },
        { "lensModel", "LensID" },
        { "aperture", "FNumber" },
        { "focalLength", "FocalLength" }
    };

    // Mapping from ExifTool attribute names to OIIO attribute names,
    // the bool flag specifies whether the value must be converted
    // from string to float.
    const std::map<std::string, std::pair<std::string, bool>>
        exiftool_to_oiio = { { "Make", { "cameraMake", false } },
                             { "Model", { "cameraModel", false } },
                             { "LensID", { "lensModel", false } },
                             { "LensModel", { "lensModel", false } },
                             { "FNumber", { "aperture", true } },
                             { "FocalLength", { "focalLength", true } } };

    std::string command = exiftool_path + " -S ";
    for ( auto key: keys )
    {
        if ( oiio_to_exiftool.count( key ) )
        {
            command += " -" + oiio_to_exiftool.at( key );
        }
        else
        {
            std::cerr << "Exiftool: unknown key " << key << std::endl;
            return false;
        }
    }

    command += " " + path;

    std::stringstream stream2;
    if ( !execute( command, stream2 ) )
    {
        std::cerr
            << "Failed to execute exiftool. Please provide the path to "
            << "exiftool binary via the RAWTOACES_EXIFTOOL_PATH environment "
            << "variable." << std::endl;
        return false;
    }

    std::map<std::string, std::string> result;
    std::vector<std::string>           lines;
    std::string                        line;
    while ( std::getline( stream2, line ) )
    {
        auto pos = line.find( ": " );
        if ( pos != line.npos )
        {
            std::string exiftool_key = line.substr( 0, pos );
            std::string value        = line.substr( pos + 2 );

            auto              &map      = exiftool_to_oiio.at( exiftool_key );
            const std::string &oiio_key = std::get<0>( map );
            bool               to_float = std::get<1>( map );

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

    return true;
}

} // namespace exiftool
} // namespace util
} // namespace rta
