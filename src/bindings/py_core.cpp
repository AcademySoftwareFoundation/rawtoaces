// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "py_core.h"
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <rawtoaces/rawtoaces_core.h>
#include <stdexcept>
#include "../misc/pragma.h"

using namespace rta::core;
using namespace nanobind::literals;

void bind_spectral_data( nanobind::module_ &m )
{
    nanobind::class_<SpectralData> spectral_data( m, "SpectralData", R"""(
        A data-class for storing spectral data, based on the file format used in
        [rawtoaces-data](https://github.com/AcademySoftwareFoundation/rawtoaces-data).
        )""" );

    spectral_data.def( nanobind::init<>(), R"""(
        Default constructor
        )""" );
    spectral_data.def_rw( "manufacturer", &SpectralData::manufacturer, R"""(
        Manufacturer of the device being tested.
        )""" );
    spectral_data.def_rw( "model", &SpectralData::model, R"""(
        Model of the device being tested.
        )""" );
    spectral_data.def_rw( "type", &SpectralData::type, R"""(
        Type of the spectral dataset.
        )""" );
    spectral_data.def_rw( "units", &SpectralData::units, R"""(
        Unit or quantity of measurement for the spectral dataset.
        )""" );
    spectral_data.def(
        "load",
        []( SpectralData      &spectral_data_value,
            const std::string &path,
            bool               reshape ) {
            return spectral_data_value.load( path, reshape );
        },
        "path"_a,
        "reshape"_a = true,
        R"""(
        Load spectral data from a given path

        :param path: path a path to the file to load data from
        :type path: str
        
        :param reshape: if set to ``True``, the data will be reshaped to the
            reference shape
        :type reshape: bool
        
        :return: ``True`` if loaded successfully.
        )""" );
}

void bind_metadata( nanobind::module_ &m )
{
    nanobind::class_<Metadata> metadata( m, "Metadata", R"""(
        DNG metadata required to calculate an input transform.
        )""" );

    nanobind::class_<Metadata::Calibration> calibration(
        metadata, "Calibration", R"""(
        A calibration data set structure. Contains calibration matrices for
        a given light source.
        )""" );

    calibration.def( nanobind::init<>() );
    calibration.def_rw( "illuminant", &Metadata::Calibration::illuminant, R"""(
        EXIF light source tag.
        )""" );
    calibration.def_rw(
        "camera_calibration_matrix",
        &Metadata::Calibration::camera_calibration_matrix,
        R"""(
        Camera calibration matrix.
        )""" );
    calibration.def_rw(
        "XYZ_to_RGB_matrix", &Metadata::Calibration::XYZ_to_RGB_matrix, R"""(
        XYZ to camera RGB colour transform matrix.
        )""" );

    metadata.def( nanobind::init<>() );
    metadata.def_rw( "neutral_RGB", &Metadata::neutral_RGB, R"""(
        Neutral RGB values. The colour channels in the camera RGB colour space
        representing a neutral colour.
        )""" );
    metadata.def_rw( "baseline_exposure", &Metadata::baseline_exposure, R"""(
        Exposure calibration multiplier.
        )""" );
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
        },
        R"""(
        Calibration datasets. Currently two sets are supported.
        )""" );
}

void core_bindings( nanobind::module_ &m )
{
    bind_spectral_data( m );
    bind_metadata( m );

    nanobind::class_<TransformSolver> transform_solver(
        m,
        "TransformSolver",
        R"""(
        Base class for a colour space transform solver.
        )""" );
    transform_solver.def(
        "calculate_transform", &TransformSolver::calculate_transform, R"""(
        Calculate the transform matrix. The solved matrix can be accessed via
        ``transform_matrix``.
        
        :return: ``True`` if calculated successfully.
        )""" );
    transform_solver.def_rw(
        "transform_matrix", &TransformSolver::transform_matrix, R"""(
        A 3×3 colour transform matrix populated by `calculate_transform()`
        (starts as identity from the base `TransformSolver` constructor).
        )""" );
    transform_solver.def_rw(
        "last_error_message", &TransformSolver::last_error_message, R"""(
        Error message from the most recent method call that returned ``False``.
        )""" );
    transform_solver.def_rw( "verbosity", &TransformSolver::verbosity, R"""(
        Verbosity level.
        )""" );

    nanobind::class_<MetadataSolver, TransformSolver> metadata_solver(
        m, "MetadataSolver", R"""(
        Solve an input transform using the metadata stored in DNG files.
        )""" );
    metadata_solver.def(
        nanobind::init<const Metadata &>(),
        "metadata"_a,
        R"""(
        Default constructor.
        )""" );

    DISABLE_DEPRECATED_WARNINGS
    metadata_solver.def(
        "calculate_CAT_matrix",
        []( MetadataSolver &metadata_solver ) {
            PyErr_WarnEx(
                PyExc_DeprecationWarning,
                "This method will be removed in v3. "
                "Use `calculate_transform()`.",
                1 );
            return metadata_solver.calculate_CAT_matrix();
        },
        R"""(
        Calculate the Color Adaptation Transform (CAT) matrix for color space
        conversion. This function computes the CAT matrix needed to transform
        colors from the camera's white point to the target ACES RGB white point.
        It first obtains the camera's XYZ transformation matrix and white point,
        then creates the target ACES RGB to XYZ matrix, and finally calculates
        the color adaptation transform between the two white points using the
        Bradford or CAT02 method.
        
        The CAT matrix is essential for maintaining color appearance when
        converting between different illuminant conditions, ensuring that colors
        look consistent across different lighting environments. Strictly
        speaking, this matrix is not required for image processing, as it is
        embedded in the IDT; see `calculate_transform()` or deprecated
        `calculate_IDT_matrix()`.
        
        :return: 3×3 Color Adaptation Transform matrix
        
        .. deprecated:: 2.2.0
           will be removed in v3. Use ``calculate_transform()``.
        )""" );

    metadata_solver.def(
        "calculate_IDT_matrix",
        []( MetadataSolver &metadata_solver ) {
            PyErr_WarnEx(
                PyExc_DeprecationWarning,
                "This method will be removed in v3. "
                "Use `calculate_transform()`.",
                1 );
            return metadata_solver.calculate_IDT_matrix();
        },
        R"""(
        Calculate the Input Device Transform (IDT) matrix for DNG color space
        conversion. This function computes the final IDT matrix that transforms
        camera RGB values to ACES RGB color space. It combines the Color
        Adaptation Transform (CAT) matrix with the D65 ACES RGB to XYZ
        transformation matrix to create a complete camera-to-ACES transformation
        pipeline.
        
        :return: 3×3 Input Device Transform matrix for DNG to ACES conversion
        :note: CAT is computed internally; ``calculate_CAT_matrix()`` does not 
               need to be called first.
                
        .. deprecated:: 2.2.0
           will be removed in v3. Use ``calculate_transform()``.
        )""" );
    ENABLE_WARNINGS

    nanobind::class_<SpectralSolver, TransformSolver> spectral_solver(
        m, "SpectralSolver", R"""(
        Solve an input transform using spectral sensitivity curves of a camera.
        )""" );
    spectral_solver.def(
        nanobind::init<const std::vector<std::string> &>(),
        "search_directories"_a = std::vector<std::string>{},
        R"""(
        Initialize SpectralSolver with database search path.
        Sets white balance multipliers to neutral (1, 1, 1) and relies on the
        base `TransformSolver` constructor for an identity `transform_matrix` and
        zero verbosity. Takes the database search path as an optional parameter
        for finding spectral data files.
        
        :param search_directories: optional database search path for spectral
            data files
        )""" );
    spectral_solver.def(
        "collect_data_files",
        &SpectralSolver::collect_data_files,
        "type"_a,
        R"""(
        A helper method collecting spectral data files of a given type from the
        database. This function searches through the configured search
        directories to find all spectral data files matching the specified type
        (e.g., ``camera``, ``illuminant``). It searches for type subdirectories at
        the top level of each directory and returns JSON files matching the
        type.
        
        :param type: data type of the files to search for (e.g., ``camera``,
            ``illuminant``, ``cmf``)
        :type type: str
        :return: a collection of file paths found in the database
        )""" );
    spectral_solver.def(
        "find_camera",
        []( SpectralSolver    &solver,
            const std::string &make,
            const std::string &model ) {
            if ( make.empty() || model.empty() )
            {
                throw std::invalid_argument(
                    "Camera make and model must be non-empty." );
            }
            return solver.find_camera( make, model );
        },
        "make"_a,
        "model"_a,
        R"""(
        Load spectral sensitivity data for a camera by searching the database.
        This function searches through camera data files in the database to find
        a match for the specified camera manufacturer and model. It loads the
        spectral sensitivity data into the camera member variable.
        
        :param make: the camera make to search for
        :type make: str
        :param model: the camera model to search for
        :type model: str
        :return: ``True`` if loaded successfully, ``False`` otherwise
        )""" );
    spectral_solver.def(
        "find_illuminant",
        []( SpectralSolver &solver, const std::string &type ) {
            if ( type.empty() )
            {
                throw std::invalid_argument(
                    "Illuminant type must be non-empty." );
            }
            return solver.find_illuminant( type );
        },
        "type"_a,
        R"""(
        Find spectral power distribution data of an illuminant of the given
        type. This function can handle both built-in illuminant types (e.g.,
        ``d55``, ``3200k``) and custom illuminants stored in the database. For
        built-in types, it generates the spectral data using standard formulas.
        
        :param type: illuminant type. Can be one of the built-in types, e.g. 
           ``d55``, ``3200k``, or a custom illuminant stored in the  database.
        :type type: str
        
        :return: ``True`` if loaded successfully, ``False`` otherwise
        )""" );
    spectral_solver.def(
        "find_illuminant",
        []( SpectralSolver            &solver,
            const std::vector<double> &wb_multipliers ) {
            if ( wb_multipliers.size() != 3 )
            {
                throw std::invalid_argument(
                    "White-balance multipliers must contain 3 elements." );
            }
            return solver.find_illuminant( wb_multipliers );
        },
        "wb_multipliers"_a,
        R"""(
        Find the illuminant best matching the given white-balancing multipliers.
        This function analyzes all available illuminants and selects the one
        that best matches the white balance coefficients. It uses Sum of Squared
        Errors (SSE) to find the optimal match and automatically scales the
        white balance multipliers.
        
        :param wb_multipliers: white-balancing multipliers to match
        :type wb_multipliers: list[float]
        
        :return: ``True`` if loaded successfully, ``False`` otherwise
        )""" );
    spectral_solver.def( "calculate_WB", &SpectralSolver::calculate_WB, R"""(
        Calculate the white-balance multipliers for the given configuration.
        This function computes RGB white balance multipliers by integrating
        camera spectral sensitivity with illuminant power spectrum. The
        multipliers normalize the camera response to achieve proper white
        balance under the specified illuminant conditions.
        The `camera` and `illuminant` data have to be configured prior to this
        call.
        
        :return: ``True`` if calculated successfully, ``False`` otherwise
        :pre: camera and illuminant data must be properly loaded
        )""" );
    spectral_solver.def(
        "load_spectral_data",
        &SpectralSolver::load_spectral_data,
        "file_path"_a,
        "out_data"_a,
        R"""(
        A helper method loading the spectral data for a file at the given path.
        This function loads spectral data from a file, handling both absolute
        and relative paths. For relative paths, it searches through all
        configured search directories.
        
        :param file_path: the path to the file to load. If the path is relative,
           all locations in the search path will be searched in.
        :type file_path: str
        :param out_data: the `SpectralData` object to be filled with the loaded
           data.
        :type out_data: rawtoaces.SpectralData
        :return: ``True`` if loaded successfully, ``False`` otherwise
        )""" );
    spectral_solver.def(
        "get_WB_multipliers", &SpectralSolver::get_WB_multipliers, R"""(
        Get the white-balance multipliers calculated using ``find_illuminant()``
        or ``calculate_WB()``. This function returns a reference to the 
        3-element vector containing RGB white balance multipliers. These 
        multipliers scale the camera response to achieve proper white balance 
        under the specified illuminant conditions.
        
        :return: a 3-element white balance multiplier list [R, G, B]
        :pre: white balance calculation must have been performed successfully
        )""" );
    spectral_solver.def_rw( "camera", &SpectralSolver::camera, R"""(
        The camera spectral data. Can be either assigned directly, loaded
        in place via ``solver.camera.load()``, or found via
        ``solver.find_camera()``.
        )""" );
    spectral_solver.def_rw( "illuminant", &SpectralSolver::illuminant, R"""(
        The illuminant spectral data. Can be either assigned directly, loaded
        in place via ``solver.illuminant.load()``, or found via
        ``solver.find_illuminant()``.
        )""" );
    spectral_solver.def_rw( "observer", &SpectralSolver::observer, R"""(
        The observer spectral data. Can be either assigned directly, or loaded
        in place via ``solver.observer.load()``.
        )""" );
    spectral_solver.def_rw(
        "training_data",
        &SpectralSolver::training_data,
        R"""(
        The training set spectral data. Can be either assigned directly, or loaded
        in place via ``solver.training_data.load()``.
        )""" );

    DISABLE_DEPRECATED_WARNINGS
    spectral_solver.def(
        "calculate_IDT_matrix",
        []( SpectralSolver &spectral_solver ) {
            PyErr_WarnEx(
                PyExc_DeprecationWarning,
                "This method will be removed in v3. "
                "Use `calculate_transform()`.",
                1 );
            return spectral_solver.calculate_IDT_matrix();
        },
        R"""(
        Calculate an input transform matrix using curve fitting optimization.
        This function computes the optimal IDT matrix by comparing camera RGB
        responses with target XYZ values across all training patches.
        The ``camera``, ``illuminant``, ``observer`` and ``training_data`` have
        to be configured prior to this call.
        
        :return: ``True`` if calculated successfully, ``False`` otherwise
        :pre: camera, illuminant, observer, and training_data must be properly
            loaded)""" );

    spectral_solver.def(
        "get_IDT_matrix",
        []( SpectralSolver &spectral_solver ) {
            PyErr_WarnEx(
                PyExc_DeprecationWarning,
                "This method will be removed in v3. "
                "Use `transform_matrix`.",
                1 );
            return spectral_solver.get_IDT_matrix();
        },
        R"""(
        Get the matrix from the last successful spectral solve (same data as
        ``transform_matrix``).
        This function returns a reference to the 3×3 IDT matrix that transforms
        camera RGB values to standardized color space. The matrix is computed by
        curve fitting optimization and represents the optimal color
        transformation for the camera under the specified illuminant conditions.
        
        :return: a 3×3 IDT transformation matrix
        :pre: ``calculate_transform()`` or deprecated ``calculate_IDT_matrix()``
            completed successfully)""" );
    ENABLE_WARNINGS
}
