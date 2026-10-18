// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include <rawtoaces/image_converter.h>

/// Amend the error message by adding info on the relevant command line parameter.
void update_error_message( std::string &error_message )
{
    static const std::map<std::string, std::string> message_map = {
        { "Missing the camera manufacturer name in the file metadata.",
          " You can provide a custom value using the --custom-camera-make "
          "parameter." },
        { "Missing the camera model name in the file metadata.",
          " You can provide a custom value using the --custom-camera-model "
          "parameter." },
        { "Missing the lens model name in the file metadata.",
          " You can provide a custom value using the --custom-lens-model "
          "parameter." },
        { "Missing the aperture value in the file metadata.",
          " You can provide a custom value using the --custom-aperture "
          "parameter." },
        { "Missing the focal length value in the file metadata.",
          " You can provide a custom value using the --custom-focal-length "
          "parameter." },
        { "Please provide the database path using the RAWTOACES_DATA_PATH "
          "environment variable",
          " or the --data-dir parameter" }
    };

    for ( auto [key, value]: message_map )
    {
        auto found = error_message.find( key );
        if ( found != std::string::npos )
        {
            error_message.insert( found + key.length(), value );
        }
    }
}

int main( int argc, const char *argv[] )
{
#ifndef WIN32
    setenv( "TZ", "UTC", 1 );
#else
    _putenv( (char *)"TZ=UTC" );
#endif

    rta::util::ImageConverter converter;

    OIIO::ArgParse arg_parser;
    arg_parser.arg( "filename" ).action( OIIO::ArgParse::append() ).hidden();
    converter.init_parser( arg_parser );

    arg_parser.parse_args( argc, argv );

    if ( !converter.parse_parameters( arg_parser ) )
    {
        std::string error_message = converter.last_error_message;
        update_error_message( error_message );
        std::cerr << "Error: " << error_message << std::endl;
        return 1;
    }

    auto files = arg_parser["filename"].as_vec<std::string>();
    if ( files.empty() || ( files.size() == 1 && files[0] == "" ) )
    {
        arg_parser.print_help();
        return 1;
    }

    // Gather all the raw images from arg list
    std::vector<std::vector<std::string>> batches =
        rta::util::collect_image_files( files ); // LCOV_EXCL_LINE

    // Process raw files
    bool empty  = true;
    bool result = true;

    size_t file_index  = 0;
    size_t total_files = 0;

    for ( auto const &batch: batches )
        total_files += batch.size();

    for ( auto const &batch: batches )
    {
        for ( auto const &input_filename: batch )
        {
            ++file_index;
            std::cout << "[" << file_index << "/" << total_files
                      << "] Processing file: " << input_filename << std::endl;

            empty  = false;
            result = converter.process_image( input_filename );
            if ( !result )
            {
                std::cerr << "Failed on file [" << file_index << "/"
                          << total_files << "]: " << input_filename
                          << std::endl;

                // Print library-level error message if available
                if ( !converter.last_error_message.empty() )
                {
                    std::string error_message = converter.last_error_message;
                    update_error_message( error_message );
                    std::cerr << "  Reason: " << error_message << std::endl;

                    if ( converter.status ==
                         rta::util::ImageConverter::Status::ConfigurationError )
                    {
                        if ( error_message.find(
                                 "Failed to find illuminant type" ) !=
                             std::string::npos )
                        {
                            // We can not recover from this error on subsequent
                            // images, so error out.
                            return 1;
                        }
                    }
                }

                // Add CLI-specific hints based on error status
                switch ( converter.status )
                {
                    case rta::util::ImageConverter::Status::FileExists:
                        std::cerr
                            << "  Hint: Use --overwrite to allow overwriting existing files."
                            << std::endl;
                        break;
                    case rta::util::ImageConverter::Status::
                        OutputDirectoryError:
                        std::cerr
                            << "  Hint: Use --create-dirs to create missing directories."
                            << std::endl;
                        break;
                    default: break;
                }
                break;
            }
        }
        if ( !result )
            break;
    }

    if ( empty )
        arg_parser.print_help();

    return result ? 0 : 1;
}
