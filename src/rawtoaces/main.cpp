// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include <rawtoaces/image_converter.h>

#include <set>

int main( int argc, const char *argv[] )
{
#ifndef WIN32
    putenv( (char *)"TZ=UTC" );
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
                
                // CLI specific error messages
                switch ( converter.status )
                {
                    case rta::util::ImageConverter::Status::FileExists:
                        std::cerr << "  Error: Output file already exists. "
                                  << "Use --overwrite to allow overwriting existing files."
                                  << std::endl;
                        break;
                    case rta::util::ImageConverter::Status::InputFileNotFound:
                        std::cerr << "  Error: Input file does not exist."
                                  << std::endl;
                        break;
                    case rta::util::ImageConverter::Status::EmptyInputFilename:
                        std::cerr << "  Error: Empty input filename provided."
                                  << std::endl;
                        break;
                    case rta::util::ImageConverter::Status::OutputDirectoryError:
                        std::cerr << "  Error: Output directory error. "
                                  << "Use --create-dirs to create missing directories."
                                  << std::endl;
                        break;
                    case rta::util::ImageConverter::Status::ConfigurationError:
                        std::cerr << "  Error: Failed to configure image conversion."
                                  << std::endl;
                        break;
                    case rta::util::ImageConverter::Status::ReadError:
                        std::cerr << "  Error: Failed to read the input file."
                                  << std::endl;
                        break;
                    case rta::util::ImageConverter::Status::WriteError:
                        std::cerr << "  Error: Failed to write the output file."
                                  << std::endl;
                        break;
                    default:
                        break;
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
