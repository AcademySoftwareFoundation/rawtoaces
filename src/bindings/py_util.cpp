// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "py_util.h"
#include <nanobind/stl/string.h>
#include <rawtoaces/image_converter.h>

using namespace rta::util;

void util_bindings( nanobind::module_ &m )
{
    nanobind::class_<ImageConverter> image_converter( m, "ImageConverter" );

    image_converter.def( nanobind::init<>() );

    // TODO: add the rest of the ImageConverter methods and properties
    image_converter
        .def_rw(
            "settings",
            &ImageConverter::settings ) // The Settings class is described below
        .def( "process_image", &ImageConverter::process_image );

    nanobind::class_<ImageConverter::Settings> settings(
        image_converter, "Settings" );

    // TODO: add the rest of the Settings' properties
    settings.def( nanobind::init<>() )
        .def_rw(
            "WB_Method",
            &ImageConverter::Settings::
                WB_method ) // The WBMethod enum is described below
        .def_rw( "illuminant", &ImageConverter::Settings::illuminant );

    // TODO: add the rest of WBMethod values, add other enums following this example
    nanobind::enum_<ImageConverter::Settings::WBMethod>( settings, "WBMethod" )
        .value( "Metadata", ImageConverter::Settings::WBMethod::Metadata )
        .value( "Illuminant", ImageConverter::Settings::WBMethod::Illuminant )
        .export_values();
}
