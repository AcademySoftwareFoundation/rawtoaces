..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0


.. _sec-pythoncore:

Core
====

The core python API exposes low-level solving logic implemented in
:cpp:class:`rta::core`.

Use this API when you need direct control over the solver inputs and outputs,
for example:

- Running metadata-only IDT/CAT calculations from extracted DNG metadata.
- Building a fully spectral pipeline from camera sensitivities, illuminants,
  observer curves, and training data.
- Integrating rawtoaces matrix/white-balance solving into your own tools.


Data model summary
------------------

The core bindings expose four classes:

- :py:class:`Metadata`: container for DNG-style calibration/neutral metadata.
- :py:class:`MetadataSolver`: solves CAT/IDT using :py:class:`Metadata`.
- :py:class:`SpectralData`: spectral JSON container/loader.
- :py:class:`SpectralSolver`: solves WB/IDT using spectral datasets.


Metadata workflow
-----------------

Use :py:class:`MetadataSolver` when you already have DNG metadata values and do
not need spectral database lookup.

Minimal example:

.. code-block:: python

  import rawtoaces

  md = rawtoaces.Metadata()
  md.baseline_exposure = 2.4
  md.neutral_RGB = [0.6290, 1.0, 0.7904]

  c0 = rawtoaces.Metadata.Calibration()
  c0.illuminant = 17
  c0.XYZ_to_RGB_matrix = [
      1.31197, -0.49679, 0.01156,
      -0.41723, 1.44237, 0.04528,
      0.06723, 0.21710, 0.72651,
  ]

  c1 = rawtoaces.Metadata.Calibration()
  c1.illuminant = 21
  c1.XYZ_to_RGB_matrix = [
      1.00885, -0.27351, -0.08258,
      -0.48997, 1.34441, 0.11174,
      -0.06406, 0.32997, 0.53917,
  ]

  md.calibration = [c0, c1]

  solver = rawtoaces.MetadataSolver(md)
  cat = solver.calculate_CAT_matrix()   # 3x3 list[list[float]]
  idt = solver.calculate_IDT_matrix()   # 3x3 list[list[float]]


.. py:class:: Metadata

  Metadata container used by :py:class:`MetadataSolver`.
  Corresponds to :cpp:class:`rta::core::Metadata`.

  .. py:class:: Calibration

    Per-illuminant calibration data.
    Corresponds to :cpp:class:`rta::core::Metadata::Calibration`.

    .. py:attribute:: int illuminant

      EXIF illuminant tag (for example ``17`` tungsten, ``21`` D65).

    .. py:attribute:: list[float] camera_calibration_matrix

      Optional camera calibration matrix values.

    .. py:attribute:: list[float] XYZ_to_RGB_matrix

      Flattened 3x3 XYZ-to-camera matrix (9 values).

  .. py:attribute:: list[Calibration] calibration

    Exactly two calibration entries are required.

    .. note::

      Assigning any list length other than 2 raises ``ValueError``.

  .. py:attribute:: list[float] neutral_RGB

    Neutral RGB values from metadata.

  .. py:attribute:: float baseline_exposure

    Baseline exposure value from metadata.


.. py:class:: MetadataSolver

  Solver for DNG-style metadata transforms.
  Corresponds to :cpp:class:`rta::core::MetadataSolver`.

  .. py:method:: MetadataSolver(Metadata)

    Construct from :py:class:`Metadata`.

  .. py:method:: list[list[float]] calculate_CAT_matrix()

    Calculate a 3x3 chromatic adaptation transform matrix.

  .. py:method:: list[list[float]] calculate_IDT_matrix()

    Calculate a 3x3 input device transform matrix.


Spectral workflow
-----------------

Use :py:class:`SpectralSolver` for full spectral solving from camera and
illuminant spectral data.

Typical sequence:

1. Construct solver with spectral database paths.
2. Load/find camera data.
3. Load/find illuminant.
4. Load observer and training data.
5. Calculate WB and IDT.

Minimal example using database lookup + file loading:

.. code-block:: python

  import rawtoaces

  solver = rawtoaces.SpectralSolver([
      "/path/to/rawtoaces-data/data"
  ])

  # camera and illuminant
  ok = solver.find_camera("nikon", "d200")
  if not ok:
      raise RuntimeError("camera not found")

  ok = solver.find_illuminant("d55")
  if not ok:
      raise RuntimeError(solver.last_error_message)

  # required for IDT solve
  if not solver.load_spectral_data("cmf/cmf_1931.json", solver.observer):
      raise RuntimeError("failed to load observer")
  if not solver.load_spectral_data(
      "training/training_spectral.json", solver.training_data
  ):
      raise RuntimeError("failed to load training data")

  if not solver.calculate_WB():
      raise RuntimeError(solver.last_error_message)
  if not solver.calculate_IDT_matrix():
      raise RuntimeError(solver.last_error_message)

  wb = solver.get_WB_multipliers()  # [R, G, B]
  idt = solver.get_IDT_matrix()     # 3x3


Finding a standard illuminant best matching the given white-balancing multipliers:

.. code-block:: python

  # Requires camera to be initialized first
  if not solver.find_illuminant([1.8, 1.0, 1.4]):
      raise RuntimeError(solver.last_error_message)


.. py:class:: SpectralData

  Minimal spectral data container exposed for
  :py:class:`SpectralSolver` workflows.
  Corresponds to :cpp:class:`rta::core::SpectralData`.

  File format and schema references:

  - Format description:
    `rawtoaces-data README (JSON Schema for Spectral Datasets) <https://github.com/AcademySoftwareFoundation/rawtoaces-data/blob/main/README.md#json-schema-for-spectral-datasets>`_
  - Current schema:
    `schema_1.0.0.json <https://github.com/AcademySoftwareFoundation/rawtoaces-data/blob/main/schema_1.0.0.json>`_
  - Legacy schema (still found in many existing datasets):
    `schema_0.1.0.json <https://github.com/AcademySoftwareFoundation/rawtoaces-data/blob/main/schema_0.1.0.json>`_

  .. py:attribute:: str manufacturer
  .. py:attribute:: str model
  .. py:attribute:: str type
  .. py:attribute:: str units

  .. py:method:: bool load(str path, bool reshape=True)

    Load spectral data from a JSON file.

    :param path: Absolute or relative path to a spectral JSON file.
    :param reshape: If ``True`` (default), reshapes curves to reference sampling.
    :return: ``True`` on success, otherwise ``False``.


.. py:class:: SpectralSolver

  Solver for spectral camera sensitivities and illuminants.
  Corresponds to :cpp:class:`rta::core::SpectralSolver`.

  .. py:method:: SpectralSolver(list[str] search_directories=[])

    Construct with optional spectral database search directories.

  .. py:attribute:: SpectralData camera
  .. py:attribute:: SpectralData illuminant
  .. py:attribute:: SpectralData observer
  .. py:attribute:: SpectralData training_data

    Public spectral inputs used by the solver.

  .. py:attribute:: str last_error_message

    Last error message from a failed solver call.

  .. py:attribute:: int verbosity

    Solver verbosity level.

  .. py:method:: list[str] collect_data_files(str type)

    Collect available spectral data files under configured search paths.

  .. py:method:: bool load_spectral_data(str file_path, SpectralData out_data)

    Load data from a relative or absolute path into ``out_data``.
    Relative paths are resolved against ``search_directories``.

  .. py:method:: bool find_camera(str make, str model)

    Find and load camera spectral sensitivity data.

    :raises ValueError: if ``make`` or ``model`` is empty.

  .. py:method:: bool find_illuminant(str type)

    Find or generate an illuminant by name (for example ``d55`` or ``3200k``).

    :raises ValueError: if ``type`` is empty.

  .. py:method:: bool find_illuminant(list[float] wb_multipliers)

    Find the illuminant that best matches provided RGB WB multipliers.

    :raises ValueError: if list length is not exactly 3.

  .. py:method:: bool calculate_WB()

    Calculate WB multipliers from current camera and illuminant.

  .. py:method:: bool calculate_IDT_matrix()

    Calculate an IDT matrix using camera, illuminant, observer, and training data.

  .. py:method:: list[float] get_WB_multipliers()

    Return current RGB WB multipliers ``[R, G, B]``.

  .. py:method:: list[list[float]] get_IDT_matrix()

    Return current 3x3 IDT matrix.


Error handling notes
--------------------

- Most solver methods return ``True``/``False`` for operational success.
- On failure, ``last_error_message`` is populated for state-validation failures
  (for example, missing camera/illuminant before solving).
- Input-shape validation at the python binding layer raises ``ValueError``
  immediately for invalid arguments (empty required strings, wrong WB vector
  length, invalid metadata calibration array size).


Data requirements for ``calculate_IDT_matrix``
----------------------------------------------

Before calling :py:meth:`SpectralSolver.calculate_IDT_matrix`, ensure:

- ``camera`` has 3 channels (R, G, B)
- ``illuminant`` has 1 channel (power)
- ``observer`` has 3 channels (X, Y, Z)
- ``training_data`` is non-empty

If these conditions are not met, the method returns ``False`` and
``last_error_message`` describes the missing prerequisite.
