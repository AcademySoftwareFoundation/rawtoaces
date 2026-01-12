// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "py_util.h"
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/ndarray.h>
#include <rawtoaces/image_converter.h>

using namespace rta::util;

void util_bindings( nanobind::module_ &m )
{
    m.def( "collect_image_files", &collect_image_files );

    nanobind::class_<ImageConverter> image_converter( m, "ImageConverter" );

    image_converter.def( nanobind::init<>() );

    image_converter.def_rw( "settings", &ImageConverter::settings );
    image_converter.def_ro( "status", &ImageConverter::status );
    image_converter.def( "process_image", &ImageConverter::process_image );
    image_converter.def(
        "get_WB_multipliers", &ImageConverter::get_WB_multipliers );
    image_converter.def( "get_IDT_matrix", &ImageConverter::get_IDT_matrix );
    image_converter.def( "get_CAT_matrix", &ImageConverter::get_CAT_matrix );
    image_converter.def(
        "configure",
        []( ImageConverter &converter, const std::string &input_filename ) {
            OIIO::ParamValueList options;
            return converter.configure( input_filename, options );
        } );
    image_converter.def(
        "get_supported_formats", &ImageConverter::get_supported_formats );
    image_converter.def(
        "get_supported_illuminants",
        &ImageConverter::get_supported_illuminants );
    image_converter.def(
        "get_supported_cameras", &ImageConverter::get_supported_cameras );

    nanobind::class_<ImageConverter::Settings> settings(
        image_converter, "Settings" );

    settings.def( nanobind::init<>() );
    settings.def_rw( "WB_method", &ImageConverter::Settings::WB_method );
    settings.def_rw(
        "matrix_method", &ImageConverter::Settings::matrix_method );
    settings.def_rw( "crop_mode", &ImageConverter::Settings::crop_mode );
    settings.def_rw( "illuminant", &ImageConverter::Settings::illuminant );
    settings.def_rw( "headroom", &ImageConverter::Settings::headroom );
    settings.def_rw(
        "custom_camera_make", &ImageConverter::Settings::custom_camera_make );
    settings.def_rw(
        "custom_camera_model", &ImageConverter::Settings::custom_camera_model );
    settings.def_rw( "auto_bright", &ImageConverter::Settings::auto_bright );
    settings.def_rw(
        "adjust_maximum_threshold",
        &ImageConverter::Settings::adjust_maximum_threshold );
    settings.def_rw( "black_level", &ImageConverter::Settings::black_level );
    settings.def_rw(
        "saturation_level", &ImageConverter::Settings::saturation_level );
    settings.def_rw( "half_size", &ImageConverter::Settings::half_size );
    settings.def_rw(
        "highlight_mode", &ImageConverter::Settings::highlight_mode );
    settings.def_rw( "flip", &ImageConverter::Settings::flip );
    settings.def_rw(
        "denoise_threshold", &ImageConverter::Settings::denoise_threshold );
    settings.def_rw( "scale", &ImageConverter::Settings::scale );
    settings.def_rw(
        "demosaic_algorithm", &ImageConverter::Settings::demosaic_algorithm );
    settings.def_rw(
        "database_directories",
        &ImageConverter::Settings::database_directories );
    settings.def_rw( "overwrite", &ImageConverter::Settings::overwrite );
    settings.def_rw( "create_dirs", &ImageConverter::Settings::create_dirs );
    settings.def_rw( "output_dir", &ImageConverter::Settings::output_dir );
    settings.def_rw( "use_timing", &ImageConverter::Settings::use_timing );
    settings.def_rw( "verbosity", &ImageConverter::Settings::verbosity );

    settings.def_prop_rw(
        "WB_box",
        []( const ImageConverter::Settings &settings ) {
            std::vector<int> result( 4 );
            for ( size_t i = 0; i < 4; i++ )
                result[i] = settings.WB_box[i];
            return result;
        },
        []( ImageConverter::Settings &settings, const std::vector<int> &box ) {
            if ( box.size() == 4 )
            {
                settings.WB_box[0] = box[0];
                settings.WB_box[1] = box[1];
                settings.WB_box[2] = box[2];
                settings.WB_box[3] = box[3];
            }
            else
            {
                throw std::invalid_argument(
                    "The array must contain 4 elements." );
            }
        } );

    settings.def_prop_rw(
        "custom_WB",
        []( const ImageConverter::Settings &settings ) {
            std::vector<float> result( 4 );
            for ( size_t i = 0; i < 4; i++ )
                result[i] = settings.custom_WB[i];
            return result;
        },
        []( ImageConverter::Settings &settings,
            const std::vector<float> &custom_WB ) {
            if ( custom_WB.size() == 4 )
            {
                settings.custom_WB[0] = custom_WB[0];
                settings.custom_WB[1] = custom_WB[1];
                settings.custom_WB[2] = custom_WB[2];
                settings.custom_WB[3] = custom_WB[3];
            }
            else
            {
                throw std::invalid_argument(
                    "The array must contain 4 elements." );
            }
        } );

    settings.def_prop_rw(
        "custom_matrix",
        []( const ImageConverter::Settings &settings ) {
            std::vector<std::vector<float>> result(
                3, std::vector<float>( 3 ) );
            for ( size_t i = 0; i < 3; i++ )
                for ( size_t j = 0; j < 3; j++ )
                    result[i][j] = settings.custom_matrix[i][j];
            return result;
        },
        []( ImageConverter::Settings              &settings,
            const std::vector<std::vector<float>> &matrix ) {
            if ( matrix.size() == 3 )
            {
                for ( size_t i = 0; i < 3; i++ )
                {
                    if ( matrix[i].size() == 3 )
                    {
                        for ( size_t j = 0; j < 3; j++ )
                            settings.custom_matrix[i][j] = matrix[i][j];
                    }
                    else
                    {
                        throw std::invalid_argument(
                            "Each row of the matrix must contain 3 elements." );
                    }
                }
            }
            else
            {
                throw std::invalid_argument(
                    "The matrix must contain 3 rows." );
            }
        } );

    settings.def_prop_rw(
        "crop_box",
        []( const ImageConverter::Settings &settings ) {
            std::vector<int> result( 4 );
            for ( size_t i = 0; i < 4; i++ )
                result[i] = settings.crop_box[i];
            return result;
        },
        []( ImageConverter::Settings &settings, const std::vector<int> &box ) {
            if ( box.size() == 4 )
            {
                settings.crop_box[0] = box[0];
                settings.crop_box[1] = box[1];
                settings.crop_box[2] = box[2];
                settings.crop_box[3] = box[3];
            }
            else
            {
                throw std::invalid_argument(
                    "The array must contain 4 elements." );
            }
        } );

    settings.def_prop_rw(
        "chromatic_aberration",
        []( const ImageConverter::Settings &settings ) {
            std::vector<float> result( 2 );
            for ( size_t i = 0; i < 2; i++ )
                result[i] = settings.chromatic_aberration[i];
            return result;
        },
        []( ImageConverter::Settings &settings,
            const std::vector<float> &value ) {
            if ( value.size() == 2 )
            {
                settings.chromatic_aberration[0] = value[0];
                settings.chromatic_aberration[1] = value[1];
            }
            else
            {
                throw std::invalid_argument(
                    "The array must contain 2 elements." );
            }
        } );

    nanobind::enum_<ImageConverter::Settings::WBMethod>( settings, "WBMethod" )
        .value( "Metadata", ImageConverter::Settings::WBMethod::Metadata )
        .value( "Illuminant", ImageConverter::Settings::WBMethod::Illuminant )
        .value( "Box", ImageConverter::Settings::WBMethod::Box )
        .value( "Custom", ImageConverter::Settings::WBMethod::Custom )
        .export_values();

    nanobind::enum_<ImageConverter::Settings::MatrixMethod>(
        settings, "MatrixMethod" )
        .value( "Auto", ImageConverter::Settings::MatrixMethod::Auto )
        .value( "Spectral", ImageConverter::Settings::MatrixMethod::Spectral )
        .value( "Metadata", ImageConverter::Settings::MatrixMethod::Metadata )
        .value( "Adobe", ImageConverter::Settings::MatrixMethod::Adobe )
        .value( "Custom", ImageConverter::Settings::MatrixMethod::Custom )
        .export_values();

    nanobind::enum_<ImageConverter::Settings::CropMode>( settings, "CropMode" )
        .value( "Off", ImageConverter::Settings::CropMode::Off )
        .value( "Soft", ImageConverter::Settings::CropMode::Soft )
        .value( "Hard", ImageConverter::Settings::CropMode::Hard )
        .export_values();

    nanobind::enum_<ImageConverter::Status>( image_converter, "Status" )
        .value( "Success", ImageConverter::Status::Success )
        .value( "FileExists", ImageConverter::Status::FileExists )
        .value( "InputFileNotFound", ImageConverter::Status::InputFileNotFound )
        .value( "EmptyInputFilename", ImageConverter::Status::EmptyInputFilename )
        .value( "FilesystemError", ImageConverter::Status::FilesystemError )
        .value( "OutputDirectoryError", ImageConverter::Status::OutputDirectoryError )
        .value( "InvalidPath", ImageConverter::Status::InvalidPath )
        .value( "ConfigurationError", ImageConverter::Status::ConfigurationError )
        .value( "ReadError", ImageConverter::Status::ReadError )
        .value( "MatrixApplicationError", ImageConverter::Status::MatrixApplicationError )
        .value( "ScaleApplicationError", ImageConverter::Status::ScaleApplicationError )
        .value( "CropApplicationError", ImageConverter::Status::CropApplicationError )
        .value( "WriteError", ImageConverter::Status::WriteError )
        .value( "UnknownError", ImageConverter::Status::UnknownError )
        .export_values();
}
