// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "py_core.h"
#include <nanobind/operators.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <rawtoaces/rawtoaces_core.h>
#include <cmath>
#include <functional>
#include <stdexcept>
#include "../misc/pragma.h"

using namespace rta::core;
using namespace nanobind::literals;

namespace
{

// Arithmetic asserts on mismatched curves. Raise ValueError for Python.
void require_same_sampling( const Spectrum &lhs, const Spectrum &rhs )
{
    if ( !( lhs.shape == rhs.shape ) || lhs.values.size() != rhs.values.size() )
    {
        throw std::invalid_argument(
            "Spectrum operands must have the same shape and sample count." );
    }
}

// A positive step makes the C++ constructor cast the sample count to
// size_t. Reject shapes where that cast would be undefined.
void require_valid_shape( const Spectrum::Shape &shape )
{
    if ( !( shape.step > 0 ) )
    {
        return;
    }

    if ( !std::isfinite( shape.first ) || !std::isfinite( shape.last ) ||
         !std::isfinite( shape.step ) || shape.last < shape.first )
    {
        throw std::invalid_argument(
            "Spectrum shape with a positive step must use finite "
            "wavelengths and last >= first." );
    }
}

} // namespace

void bind_spectrum( nanobind::module_ &m )
{
    nanobind::class_<Spectrum> spectrum( m, "Spectrum", R"""(
        A regularly sampled spectral curve.

        Corresponds to :cpp:struct:`rta::core::Spectrum`. Per-element
        arithmetic requires both curves to share a shape and sample count.
        )""" );

    nanobind::class_<Spectrum::Shape> shape( spectrum, "Shape", R"""(
        Regular wavelength sampling, in nanometers.

        ``first`` and ``last`` are the endpoint wavelengths and ``step`` is
        the spacing. Shapes compare equal when all three values match.
        )""" );
    shape.def(
        nanobind::init<float, float, float>(),
        "first"_a = 0.f,
        "last"_a  = 0.f,
        "step"_a  = 0.f,
        R"""(
        Create a sampling description.

        :param first: Wavelength of the first sample, in nanometers.
        :param last: Wavelength of the last sample, in nanometers.
        :param step: Spacing between samples, in nanometers. A zero step
            describes an empty curve.
        )""" );
    shape.def( nanobind::init<const Spectrum::Shape &>(), "other"_a, R"""(
        Copy constructor.
        )""" );
    shape.def_rw( "first", &Spectrum::Shape::first, R"""(
        Wavelength of the first sample, in nanometers.
        )""" );
    shape.def_rw( "last", &Spectrum::Shape::last, R"""(
        Wavelength of the last sample, in nanometers.
        )""" );
    shape.def_rw( "step", &Spectrum::Shape::step, R"""(
        Spacing between samples, in nanometers.
        )""" );
    // NOLINTBEGIN(misc-redundant-expression)
    shape.def( nanobind::self == nanobind::self, R"""(
        Return whether ``first``, ``last``, and ``step`` all match.
        )""" );
    // NOLINTEND(misc-redundant-expression)
    // Equality is by value, and a shape is mutable, so it cannot be hashed.
    shape.attr( "__hash__" ) = nanobind::none();

    spectrum.def(
        "__init__",
        []( Spectrum *self, double value, const Spectrum::Shape &shape ) {
            require_valid_shape( shape );
            new ( self ) Spectrum( value, shape );
        },
        "value"_a = 0.0,
        "shape"_a = Spectrum::ReferenceShape,
        R"""(
        Create a curve and fill every sample with ``value``.

        :param value: Initial value of every sample.
        :param shape: Sampling of the new curve. Defaults to
            ``Spectrum.ReferenceShape``. Pass ``Spectrum.EmptyShape`` to
            allocate no samples.
        :raises ValueError: If ``step`` is positive and the wavelengths are
            non-finite or ``last`` is less than ``first``.
        )""" );
    spectrum.def( nanobind::init<const Spectrum &>(), "other"_a, R"""(
        Copy constructor.
        )""" );
    spectrum.def_prop_ro_static(
        "ReferenceShape",
        []( nanobind::handle ) { return Spectrum::ReferenceShape; },
        R"""(
        Built-in sampling used by rawtoaces, from 380 nm through 780 nm in
        5 nm steps. Each access returns a new copy.
        )""" );
    spectrum.def_prop_ro_static(
        "EmptyShape",
        []( nanobind::handle ) { return Spectrum::EmptyShape; },
        R"""(
        Empty sampling (zero step) used to construct a curve with no samples.
        Each access returns a new copy.
        )""" );
    spectrum.def_rw( "shape", &Spectrum::shape, R"""(
        Wavelength sampling of this curve, in nanometers. Assigning a shape
        records that sampling and leaves ``values`` unchanged, so the sample
        count can disagree with the shape. ``reshape()`` resamples onto
        ``ReferenceShape`` from the current samples and does not check that
        the sample count matches the shape.
        )""" );
    spectrum.def_rw( "values", &Spectrum::values, R"""(
        Spectral samples. Reading returns a new list, so assigning to an
        index of that list leaves the curve unchanged. Assign a sequence to
        replace the samples.
        )""" );
    spectrum.def(
        "reshape",
        []( Spectrum &curve ) {
            if ( curve.values.empty() )
            {
                throw std::invalid_argument(
                    "Cannot reshape a spectrum with no samples." );
            }
            curve.reshape();
        },
        R"""(
        Resample this curve onto ``ReferenceShape`` with linear interpolation.

        Wavelengths outside the current range are filled from the nearest
        endpoint sample. A curve whose shape already equals
        ``ReferenceShape`` is left unchanged, including when its sample count
        does not match that shape.

        :raises ValueError: If the curve has no samples.
        )""" );
    spectrum.def( "integrate", &Spectrum::integrate, R"""(
        Sum every sample.

        :return: The sum of ``values``, or ``0`` when the curve is empty.
        )""" );
    spectrum.def( "max", &Spectrum::max, R"""(
        Return the largest sample, or ``0`` when ``values`` is empty.
        )""" );

    auto bind_binary = [&]( const char *name, auto op, const char *doc ) {
        spectrum.def(
            name,
            [op]( const Spectrum &lhs, const Spectrum &rhs ) {
                require_same_sampling( lhs, rhs );
                return op( lhs, rhs );
            },
            nanobind::is_operator(),
            doc );
    };
    auto bind_inplace = [&]( const char *name, auto op, const char *doc ) {
        spectrum.def(
            name,
            [op]( Spectrum &lhs, const Spectrum &rhs ) -> Spectrum & {
                require_same_sampling( lhs, rhs );
                op( lhs, rhs );
                return lhs;
            },
            nanobind::is_operator(),
            nanobind::rv_policy::reference,
            doc );
    };

    bind_binary( "__add__", std::plus<>(), R"""(
        Per-element addition.

        :param other: Curve with the same shape and sample count.
        :return: A new curve.
        :raises ValueError: If the operands differ in shape or sample count.
        )""" );
    bind_binary( "__sub__", std::minus<>(), R"""(
        Per-element subtraction.

        :param other: Curve with the same shape and sample count.
        :return: A new curve.
        :raises ValueError: If the operands differ in shape or sample count.
        )""" );
    bind_binary( "__mul__", std::multiplies<>(), R"""(
        Per-element multiplication.

        :param other: Curve with the same shape and sample count.
        :return: A new curve.
        :raises ValueError: If the operands differ in shape or sample count.
        )""" );
    bind_binary( "__truediv__", std::divides<>(), R"""(
        Per-element division.

        :param other: Curve with the same shape and sample count.
        :return: A new curve.
        :raises ValueError: If the operands differ in shape or sample count.
        )""" );
    bind_inplace(
        "__iadd__",
        []( Spectrum &lhs, const Spectrum &rhs ) { lhs += rhs; },
        R"""(
        Per-element addition in place.

        :param other: Curve with the same shape and sample count.
        :return: This curve.
        :raises ValueError: If the operands differ in shape or sample count.
        )""" );
    bind_inplace(
        "__isub__",
        []( Spectrum &lhs, const Spectrum &rhs ) { lhs -= rhs; },
        R"""(
        Per-element subtraction in place.

        :param other: Curve with the same shape and sample count.
        :return: This curve.
        :raises ValueError: If the operands differ in shape or sample count.
        )""" );
    bind_inplace(
        "__imul__",
        []( Spectrum &lhs, const Spectrum &rhs ) { lhs *= rhs; },
        R"""(
        Per-element multiplication in place.

        :param other: Curve with the same shape and sample count.
        :return: This curve.
        :raises ValueError: If the operands differ in shape or sample count.
        )""" );
    bind_inplace(
        "__itruediv__",
        []( Spectrum &lhs, const Spectrum &rhs ) { lhs /= rhs; },
        R"""(
        Per-element division in place.

        :param other: Curve with the same shape and sample count.
        :return: This curve.
        :raises ValueError: If the operands differ in shape or sample count.
        )""" );
}

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
        
        :param reshape: if set to ``True``, the data will be reshaped to
            :py:attr:`rawtoaces.Spectrum.ReferenceShape`
        :type reshape: bool
        
        :return: ``True`` if loaded successfully.
        )""" );
    spectral_data.def(
        "__getitem__",
        []( SpectralData &spectral_data_value, const std::string &name ) {
            return spectral_data_value[name];
        },
        "name"_a,
        R"""(
        Return a copy of the named channel in the ``main`` data set.

        :param name: Channel name in the ``main`` set.
        :return: A copy of that channel's spectrum.
        :raises ValueError: If the ``main`` set or the channel is absent.
        )""" );
    spectral_data.def(
        "__setitem__",
        []( SpectralData      &spectral_data_value,
            const std::string &name,
            const Spectrum    &curve ) { spectral_data_value[name] = curve; },
        "name"_a,
        "curve"_a,
        R"""(
        Replace the named channel in the ``main`` data set.

        :param name: Channel name in the ``main`` set.
        :param curve: Spectrum to store. The channel must already exist.
        :raises ValueError: If the ``main`` set or the channel is absent.
        )""" );
    spectral_data.def(
        "__contains__",
        []( const SpectralData &spectral_data_value, const std::string &name ) {
            const auto set = spectral_data_value.data.find( "main" );
            if ( set == spectral_data_value.data.end() )
            {
                return false;
            }

            for ( const auto &channel: set->second )
            {
                if ( channel.first == name )
                    return true;
            }
            return false;
        },
        "name"_a,
        R"""(
        Return whether ``name`` is a channel in the ``main`` data set.

        :param name: Channel name to look up.
        :return: ``True`` when that channel exists. An empty container and a
            missing ``main`` set return ``False``.
        )""" );
    spectral_data.attr( "__iter__" ) = nanobind::none();
    spectral_data.def(
        "get",
        []( SpectralData      &spectral_data_value,
            const std::string &set_name,
            const std::string &channel_name ) {
            return spectral_data_value.get( set_name, channel_name );
        },
        "set_name"_a,
        "channel_name"_a,
        R"""(
        Return a copy of one channel from a named data set.

        :param set_name: Spectral set name, usually ``"main"``.
        :param channel_name: Channel name within that set.
        :return: A copy of the matching spectrum.
        :raises ValueError: If the set or channel is absent.
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
    bind_spectrum( m );
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

        Call this method on :py:class:`rawtoaces.MetadataSolver` or
        :py:class:`rawtoaces.SpectralSolver`; the base class cannot be
        constructed directly in Python. The derived solver's inputs must be
        configured before solving. For spectral solving, calculate white
        balance first with :py:meth:`rawtoaces.SpectralSolver.calculate_WB`
        or the multiplier form of
        :py:meth:`rawtoaces.SpectralSolver.find_illuminant`.

        :return: ``True`` if calculated successfully, ``False`` otherwise.
            Inspect ``last_error_message`` on failure. Read
            ``transform_matrix`` only after a successful solve.
        )""" );
    transform_solver.def_rw(
        "transform_matrix", &TransformSolver::transform_matrix, R"""(
        A 3×3 colour transform matrix populated by `calculate_transform()`
        (initially the identity matrix).
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

        The inherited ``calculate_transform``, ``transform_matrix``,
        ``last_error_message``, and ``verbosity`` members are available on this
        class.
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
           Will be removed in v3. Use ``calculate_transform()``.
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
           Will be removed in v3. Use ``calculate_transform()``.
        )""" );
    ENABLE_WARNINGS

    nanobind::class_<SpectralSolver, TransformSolver> spectral_solver(
        m, "SpectralSolver", R"""(
        Solve an input transform using spectral sensitivity curves of a camera.

        The inherited ``calculate_transform``, ``transform_matrix``,
        ``last_error_message``, and ``verbosity`` members are available on this
        class.
        )""" );
    spectral_solver.def(
        nanobind::init<const std::vector<std::string> &>(),
        "search_directories"_a = std::vector<std::string>{},
        R"""(
        Initialize SpectralSolver with database search path.
        Sets white balance multipliers to neutral (1, 1, 1),
        ``transform_matrix`` to identity, and ``verbosity`` to zero. Takes the
        database search path as an optional parameter for finding spectral data
        files.
        
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
        Load an illuminant by its non-empty standard or database name.

        Generates daylight spectra such as ``"d55"`` and blackbody spectra such as
        ``"3200k"``; other names are searched in the configured spectral database
        paths. Camera data is not required for this form. This overload loads the
        illuminant but does not calculate white balance; call ``calculate_WB()``
        after loading the camera.

        :param type: Non-empty standard or database illuminant name.
        :return: ``True`` on success, ``False`` on failure. A failed database lookup
            may leave ``last_error_message`` empty or unchanged.
        :raises ValueError: If ``type`` is empty.
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
        Choose the illuminant best matching three white-balance multipliers.

        Requires camera data with three channels (R, G, B) to be loaded first.
        Searches generated daylight/blackbody spectra and database illuminants.
        On success this overload also calculates white-balance multipliers, available
        from ``get_WB_multipliers()``.

        :param wb_multipliers: Exactly three white-balance multipliers, [R, G, B].
        :return: ``True`` on success, ``False`` on failure. State-validation failures
            populate ``last_error_message``; a failed database lookup may leave it
            empty or unchanged.
        :raises ValueError: If ``wb_multipliers`` does not contain exactly three values.
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
            loaded

        .. deprecated:: 2.2.0
           Will be removed in v3. Use ``calculate_transform()``.
        )""" );

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
            completed successfully

        .. deprecated:: 2.2.0
           Will be removed in v3. Read ``transform_matrix`` after a successful
           ``calculate_transform()`` call.
        )""" );
    ENABLE_WARNINGS
}
