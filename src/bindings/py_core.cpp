// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "py_core.h"
#include <nanobind/stl/vector.h>
#include <rawtoaces/rawtoaces_core.h>
#include <stdexcept>

using namespace rta::core;

void bind_metadata( nanobind::module_ &m )
{
    nanobind::class_<Metadata> metadata( m, "Metadata" );

    nanobind::class_<Metadata::Calibration> calibration(
        metadata, "Calibration" );

    calibration.def( nanobind::init<>() );
    calibration.def_rw( "illuminant", &Metadata::Calibration::illuminant );
    calibration.def_rw(
        "camera_calibration_matrix",
        &Metadata::Calibration::camera_calibration_matrix );
    calibration.def_rw(
        "XYZ_to_RGB_matrix", &Metadata::Calibration::XYZ_to_RGB_matrix );

    metadata.def( nanobind::init<>() );
    metadata.def_rw( "neutral_RGB", &Metadata::neutral_RGB );
    metadata.def_rw( "baseline_exposure", &Metadata::baseline_exposure );
    metadata.def_prop_rw(
        "calibration",
        []( const Metadata &metadata_value ) {
            std::vector<Metadata::Calibration> result( 2 );
            result[0] = metadata_value.calibration[0];
            result[1] = metadata_value.calibration[1];
            return result;
        },
        []( Metadata                                 &metadata_value,
            const std::vector<Metadata::Calibration> &calibration_value ) {
            if ( calibration_value.size() != 2 )
            {
                throw std::invalid_argument(
                    "The calibration array must contain 2 elements." );
            }

            metadata_value.calibration[0] = calibration_value[0];
            metadata_value.calibration[1] = calibration_value[1];
        } );
}

void core_bindings( nanobind::module_ &m )
{
    bind_metadata( m );

    nanobind::class_<MetadataSolver> metadata_solver( m, "MetadataSolver" );
    metadata_solver.def( nanobind::init<const Metadata &>() );
    metadata_solver.def(
        "calculate_CAT_matrix", &MetadataSolver::calculate_CAT_matrix );
    metadata_solver.def(
        "calculate_IDT_matrix", &MetadataSolver::calculate_IDT_matrix );
}
