// Copyright Contributors to the rawtoaces project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/rawtoaces

#include <rawtoaces/rawtoaces_util.h>

#include <filesystem>

using namespace rta;

bool check_and_add(
    const std::filesystem::path &path, std::vector<std::string> &batch )
{
    if ( std::filesystem::is_regular_file( path ) ||
         std::filesystem::is_symlink( path ) )
    {
        auto e = path.extension();

        if ( e == ".exr" || e == ".EXR" )
            return false;
        if ( e == ".jpg" || e == ".JPG" )
            return false;
        if ( e == ".jpeg" || e == ".JPEG" )
            return false;

        std::string str = path.string();
        batch.push_back( str );
    }
    else
    {
        std::cerr << "Not a regular file: " << path << std::endl;
    }
    return true;
}

int main( int argc, const char *argv[] )
{
    OIIO::ArgParse argParse;
    argParse.arg( "filename" ).action( OIIO::ArgParse::append() ).hidden();

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

    // Create a separate batch for each input directory.
    // Reserve the first batch for the individual input files.
    std::vector<std::vector<std::string>> batches( 1 );

    auto files = argParse["filename"].as_vec<std::string>();
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
            std::vector<std::string> &curr_batch = batches.emplace_back();
            auto it = std::filesystem::directory_iterator( filename );

            for ( auto filename2: it )
            {
                if ( !check_and_add( filename2, curr_batch ) )
                    continue;
            }
        }
        else
        {
            if ( !check_and_add( filename, batches[0] ) )
                continue;
        }
    }

    bool result = true;
    for ( auto const &batch: batches )
    {
        for ( auto const &input_filename: batch )
        {
            std::string output_filename = input_filename;
            if ( !converter.make_output_path( output_filename ) )
                continue;

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
                     0,
                     0,
                     0,
                     buffer.nchannels(),
                     true,
                     OIIO::TypeDesc::FLOAT ) )
            {
                std::cerr << "Failed to read for the file: " << input_filename
                          << std::endl;
                result = false;
                continue;
            }

            if ( !converter.apply_matrix( buffer, buffer ) )
            {
                std::cerr
                    << "Failed to apply colour space conversion to the file: "
                    << input_filename << std::endl;
                result = false;
                continue;
            }

            if ( !converter.apply_scale( buffer, buffer ) )
            {
                std::cerr << "Failed to apply scale to the file: "
                          << input_filename << std::endl;
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
    }

    return result ? 0 : 1;
}
