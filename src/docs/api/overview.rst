..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0

.. _api-overview:

API Overview
============

Use the :doc:`user guide <../user_guide>` to convert files from the command
line. This page chooses a programming API: convert a whole RAW file, run the
OpenImageIO buffer steps yourself, or solve a transform from metadata or
spectral data.

The table names the entry point. The sections below say what you pass in, what
comes back, which data the call needs, how failure is reported, and where the
worked example and signatures live. C++ and Python cover the same tasks where
both languages have the API. Helpers that exist only in C++ are listed at the
end.

.. list-table::
   :header-rows: 1
   :widths: 32 34 34

   * - Task
     - Main API
     - Where it is available
   * - Convert a RAW file to an ACES EXR
     - ``ImageConverter`` and ``Settings``, through ``process_image``
     - C++ and every Python build
   * - Run the pipeline on OpenImageIO buffers
     - ``configure``, ``load_image``, ``process_buffer``, ``save_image``, and the ``apply_*`` steps
     - C++. Python when the extension is built with OpenImageIO 3.2 or later
   * - Solve from extracted camera metadata
     - ``Metadata`` and ``MetadataSolver``
     - C++ and Python
   * - Solve from measured spectral data
     - ``SpectralData`` and ``SpectralSolver``
     - C++ and Python
   * - Generate an illuminant spectrum directly, or adapt D65 XYZ
     - ``calculate_daylight_SPD``, ``calculate_blackbody_SPD``, and ``XYZD65Solver``
     - C++. Python can load the built-in named illuminants through ``SpectralSolver.find_illuminant``

:doc:`Task examples <tasks/index>` repeats each of these tasks with the same
settings in Shell, C++, and Python.

.. _overview-oiio-notice:

OpenImageIO 3.2
---------------

Calling the buffer methods from Python requires an extension built against
OpenImageIO 3.2 or later, with ``ImageBuf``, ``ParamValueList``, and
``ImageSpec`` registered for nanobind. Importing that extension also imports
the ``OpenImageIO`` Python module. An earlier OpenImageIO build still provides
``process_image`` and ``configure(input_filename)``, and omits the buffer
methods. After a successful import,
``hasattr(rawtoaces.ImageConverter, "load_image")`` reports whether they are
present. The :ref:`capability table <python-oiio-availability>` lists each
method.

The C++ methods are members of :cpp:class:`rta::util::ImageConverter` in
``rawtoaces_util``, which links OpenImageIO.

Convert a RAW file
------------------

Pass a filesystem path to :cpp:func:`rta::util::ImageConverter::process_image`
or :py:meth:`rawtoaces.ImageConverter.process_image`. Set
``ImageConverter::Settings`` first when the defaults are not the conversion you
want. ``process_image`` configures the transform from the file, decodes it,
applies lens correction (off unless requested), the colour matrix, exposure
scaling, and crop, then writes an ACES EXR.

The output name is ``<stem>_aces.exr``. It is written beside the input, or
under ``output_dir`` when that is set. ``create_dirs`` defaults to false, so
the output directory must already exist unless you enable it. ``overwrite``
defaults to false. Pixel values are multiplied by ``headroom * scale``;
``headroom`` is a linear factor and defaults to ``6.0``. The API crop default
is hard: only the crop region is written. The command-line crop default is
described in the :doc:`user guide <../user_guide>`.

White balance defaults to metadata from the file. DNG is the format that
carries the calibration this method uses. A named illuminant, or the spectral matrix method, needs camera
sensitivity data. ``Auto`` selects the spectral matrix when that data is
available and otherwise uses metadata. An Adobe matrix comes from LibRaw
through OpenImageIO. A custom matrix is a complete camera RGB to ACES AP0
matrix and is applied as given. ``database_directories`` selects the spectral
data directory; when it is empty the search uses ``RAWTOACES_DATA_PATH``, then
the legacy ``AMPAS_DATA_PATH``, then the platform default. Setup is in
:doc:`Getting Started <../getting_started>`. Lens correction is opt-in and uses
the Lensfun database; see :doc:`Lens Correction <../lens_correction>`.

A failed call returns false. Read ``status`` and ``last_error_message``.
``status`` distinguishes a missing input, an existing output, a missing
spectral database, and a failure in configuration, reading, lens correction,
the matrix, scale, crop, or writing.

:doc:`Convert one image <tasks/convert_one>` shows this call in Shell, C++,
and Python. :doc:`Convert a batch <tasks/batch>`,
:doc:`white balance and matrix <tasks/white_balance>`,
:doc:`crop and scale <tasks/crop_and_scale>`, and
:doc:`errors <tasks/errors>` continue from it. The
:doc:`C++ <cpp/image_converter>` and :doc:`Python <python/image_converter>`
guides expand on the same class.
Signatures:
:doc:`C++ Reference <cpp/api_reference>` and
:doc:`Python Reference <python/api_reference>`.
:cpp:func:`rta::util::collect_image_files` and
:py:func:`rawtoaces.collect_image_files` group file and directory arguments
into batches. :cpp:func:`rta::util::ImageConverter::make_output_path`,
``init_parser``, and ``parse_parameters`` are C++ only; Python sets
``settings`` directly.

OpenImageIO buffers
-------------------

Use this layer when the surrounding application already owns the image or
needs to run load, colour, scale, crop, and save separately.

In C++, and in the Python overload that accepts options, pass the current
``settings`` plus either a RAW path or an ``OIIO::ImageSpec`` /
``OpenImageIO.ImageSpec`` into ``configure``. The file overload fills an
OpenImageIO parameter list with decoding hints. The ``ImageSpec`` overload uses
metadata you already have. Then pass that parameter
list to ``load_image`` to fill an ``ImageBuf``. ``process_buffer`` runs lens
correction, the matrix, scale, and crop. The same four steps are
``apply_lens_correction``, ``apply_matrix``, ``apply_scale``, and ``apply_crop``.
``save_image`` writes an ACES container; the default stored sample type is
half-float. ``get_WB_multipliers`` and ``get_transform_matrix`` are valid after
a successful ``configure``.

Python file conversion can call ``configure(input_filename)`` in every build.
The overloads that take ``ParamValueList`` or ``ImageSpec``, and
``load_image``, ``save_image``, ``process_buffer``, and the ``apply_*``
methods, require OpenImageIO 3.2 or later as described in
:ref:`overview-oiio-notice`.

Each step returns false and sets ``status`` and ``last_error_message`` when it
fails. Call ``configure`` again after changing settings that affect the
transform.

:doc:`OpenImageIO buffers <tasks/buffer>` shows the sequence in C++ and
Python. The C++ signatures are on :cpp:class:`rta::util::ImageConverter` in the
:doc:`C++ Reference <cpp/api_reference>`. The Python signatures are on
:py:class:`rawtoaces.ImageConverter` in the
:doc:`Python Reference <python/api_reference>`.

Solve from camera metadata
--------------------------

Use :cpp:class:`rta::core::MetadataSolver` or
:py:class:`rawtoaces.MetadataSolver` when you already have DNG calibration
metadata and want the transform without decoding a file.

Pass a :cpp:struct:`rta::core::Metadata` / :py:class:`rawtoaces.Metadata`
value: ``neutral_RGB``, ``baseline_exposure``, and two calibration sets. Each
calibration set carries an illuminant tag and an XYZ-to-camera-RGB matrix.
``calculate_transform`` returns a boolean and, on success, writes a 3×3 matrix
to ``transform_matrix``. That matrix includes chromatic adaptation and takes
camera RGB to ACES. The solver copies the metadata it is given; it does not
read a spectral database.

A false return leaves the explanation in ``last_error_message``. Python raises
``ValueError`` when a required vector has the wrong length.

:doc:`Solve from metadata <tasks/metadata>` shows the same calibration in
C++ and Python. The :doc:`Python core guide <python/rawtoaces_core>`
explains the data model. Signatures are in the
:doc:`C++ Reference <cpp/api_reference>` and the
:doc:`Python Reference <python/api_reference>`.

Solve from spectral data
------------------------

Use :cpp:class:`rta::core::SpectralSolver` or
:py:class:`rawtoaces.SpectralSolver` when the transform should come from
measured spectra.

Pass search directories to the constructor, or assign ``camera``,
``illuminant``, ``observer``, and ``training_data`` yourself. ``find_camera``
and ``find_illuminant`` load those two from the search path. Built-in
illuminant names such as ``d55`` and ``3200k`` are generated; any other name
must exist in the data directory. Before ``calculate_transform``, the camera
needs three channels, the illuminant one, the observer three, and the training
set must be non-empty. Calculate white balance with ``calculate_WB``, or with
the three-multiplier form of ``find_illuminant``. The name form only loads the
illuminant.

On success, ``get_WB_multipliers`` holds the RGB multipliers and
``transform_matrix`` holds the 3×3 transform. A missing camera, illuminant,
observer, or training set makes ``calculate_transform`` return false and sets
``last_error_message``. Leaving white balance unset keeps the multipliers at
``[1.0, 1.0, 1.0]`` and is not reported as an error. Python raises
``ValueError`` for an invalid argument shape.

The dataset layout and schema are documented with the
`rawtoaces-data repository <https://github.com/AcademySoftwareFoundation/rawtoaces-data>`_.
:doc:`Solve from spectra <tasks/spectra>` shows the same database lookup
in C++ and Python. The :doc:`Python core guide <python/rawtoaces_core>`
explains the containers. C++ types are introduced in
:doc:`rawtoaces Core <cpp/rawtoaces_core>` and
:doc:`Spectral Data <cpp/spectral_data>`, with signatures in the
:doc:`C++ Reference <cpp/api_reference>` and the
:doc:`Python Reference <python/api_reference>`.

C++-only helpers
----------------

These C++ entry points have no matching Python function. Python reaches the
built-in illuminants through ``SpectralSolver.find_illuminant`` instead of the
two spectrum generators.

- :cpp:func:`rta::core::calculate_daylight_SPD` fills a
  :cpp:struct:`rta::core::Spectrum` for daylight from 4000 K to 25000 K, or
  from the short form 40–250 (55 selects D55).
- :cpp:func:`rta::core::calculate_blackbody_SPD` fills a spectrum for
  1500–3999 K.
- :cpp:class:`rta::core::XYZD65Solver` writes the fixed D65 chromatic-adaptation
  and XYZ-to-ACES matrix. ``calculate_transform`` succeeds whenever it is
  called.
- ``SpectralSolver::collect_aliases`` reads camera make and model aliases from
  the spectral database.

Signatures are in the :doc:`C++ Reference <cpp/api_reference>`.
