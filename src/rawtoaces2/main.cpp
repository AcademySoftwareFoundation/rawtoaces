// Copyright Contributors to the rawtoaces project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/rawtoaces

#include <rawtoaces/rawtoaces_util.h>

#include <filesystem>

using namespace rta;

int main( int argc, const char *argv[] )
{

    OIIO::ArgParse argParse;

    ImageConverter converter;
    converter.init_parser( argParse );

    if ( argParse.parse_args( argc, argv ) < 0 )
    {
        return 1;
    }

    if ( !converter.parse_params( argParse ) )
    {
        return 1;
    }

    auto                     files = argParse["filename"].as_vec<std::string>();
    std::vector<std::string> files_to_convert;

    for ( auto filename: files )
    {
        if ( !std::filesystem::exists( filename ) )
        {
            std::cerr << "File or directory not found: " << filename
                      << std::endl;
            return 1;
        }

        auto canonical_filename = std::filesystem::canonical( filename );

        if ( std::filesystem::is_directory( filename ) )
        {
            auto it = std::filesystem::directory_iterator( filename );

            for ( auto filename2: it )
            {
                if ( std::filesystem::is_regular_file( filename2 ) ||
                     std::filesystem::is_symlink( filename2 ) )
                {
                    files_to_convert.push_back( filename2.path().string() );
                }
            }
        }
        else if (
            std::filesystem::is_regular_file( filename ) ||
            std::filesystem::is_symlink( filename ) )
        {
            files_to_convert.push_back( filename );
        }
        else
        {
            std::cerr << "Not a file or directory: " << filename << std::endl;
            return 1;
        }
    }

    bool result = true;
    for ( auto const &input_filename: files_to_convert )
    {
        std::string output_filename = input_filename;
        size_t      pos             = input_filename.rfind( '.' );
        if ( pos != std::string::npos )
        {
            output_filename = input_filename.substr( 0, pos );
        }
        output_filename += "_oiio.exr";

        OIIO::ParamValueList options;

        if ( !converter.configure( input_filename, options ) )
        {
            std::cerr << "Failed to configure the reader for the file: "
                      << input_filename << std::endl;
            result = false;
            continue;
        }

        OIIO::ImageSpec imageSpec;
        imageSpec.extra_attribs = options;

        OIIO::ImageBuf buffer = OIIO::ImageBuf(
            input_filename, 0, 0, nullptr, &imageSpec, nullptr );

        if ( !buffer.read(
                 0, 0, 0, buffer.nchannels(), true, OIIO::TypeDesc::FLOAT ) )
        {
            std::cerr << "Failed to read for the file: " << input_filename
                      << std::endl;
            result = false;
            continue;
        }

        if ( !converter.apply_matrix( buffer, buffer ) )
        {
            std::cerr << "Failed to apply colour space conversion to the file: "
                      << input_filename << std::endl;
            result = false;
            continue;
        }

        if ( !converter.apply_scale( buffer, buffer ) )
        {
            std::cerr << "Failed to apply scale to the file: " << input_filename
                      << std::endl;
            result = false;
            continue;
        }

        if ( !converter.save( output_filename, buffer ) )
        {
            std::cerr << "Failed to save the file: " << output_filename
                      << std::endl;
            result = false;
            continue;
        }
    }

    return result ? 0 : 1;
}
