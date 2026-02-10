// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "py_core.h"
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <rawtoaces/rawtoaces_core.h>
#include <stdexcept>

using namespace rta::core;
using namespace nanobind::literals;

void bind_spectral_data( nanobind::module_ &m )
{
    nanobind::class_<SpectralData> spectral_data( m, "SpectralData" );

    spectral_data.def( nanobind::init<>() );
    spectral_data.def_rw( "manufacturer", &SpectralData::manufacturer );
    spectral_data.def_rw( "model", &SpectralData::model );
    spectral_data.def_rw( "type", &SpectralData::type );
    spectral_data.def_rw( "units", &SpectralData::units );
    spectral_data.def(
        "load",
        []( SpectralData      &spectral_data_value,
            const std::string &path,
            bool               reshape ) {
            return spectral_data_value.load( path, reshape );
        },
        "path"_a,
        "reshape"_a = true );
}

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
    bind_spectral_data( m );
    bind_metadata( m );

    nanobind::class_<MetadataSolver> metadata_solver( m, "MetadataSolver" );
    metadata_solver.def( nanobind::init<const Metadata &>() );
    metadata_solver.def(
        "calculate_CAT_matrix", &MetadataSolver::calculate_CAT_matrix );
    metadata_solver.def(
        "calculate_IDT_matrix", &MetadataSolver::calculate_IDT_matrix );

    nanobind::class_<SpectralSolver> spectral_solver( m, "SpectralSolver" );
    spectral_solver.def(
        nanobind::init<const std::vector<std::string> &>(),
        "search_directories"_a = std::vector<std::string>{} );
    spectral_solver.def(
        "collect_data_files", &SpectralSolver::collect_data_files );
    spectral_solver.def( "find_camera", &SpectralSolver::find_camera );
    spectral_solver.def(
        "find_illuminant",
        static_cast<bool ( SpectralSolver::* )( const std::string & )>(
            &SpectralSolver::find_illuminant ) );
    spectral_solver.def(
        "find_illuminant",
        static_cast<bool ( SpectralSolver::* )( const std::vector<double> & )>(
            &SpectralSolver::find_illuminant ) );
    spectral_solver.def( "calculate_WB", &SpectralSolver::calculate_WB );
    spectral_solver.def(
        "calculate_IDT_matrix", &SpectralSolver::calculate_IDT_matrix );
    spectral_solver.def(
        "load_spectral_data", &SpectralSolver::load_spectral_data );
    spectral_solver.def(
        "get_WB_multipliers", &SpectralSolver::get_WB_multipliers );
    spectral_solver.def( "get_IDT_matrix", &SpectralSolver::get_IDT_matrix );
    spectral_solver.def_rw( "camera", &SpectralSolver::camera );
    spectral_solver.def_rw( "illuminant", &SpectralSolver::illuminant );
    spectral_solver.def_rw( "observer", &SpectralSolver::observer );
    spectral_solver.def_rw( "training_data", &SpectralSolver::training_data );
    spectral_solver.def_rw(
        "last_error_message", &SpectralSolver::last_error_message );
    spectral_solver.def_rw( "verbosity", &SpectralSolver::verbosity );
}
