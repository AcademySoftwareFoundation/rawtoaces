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

- :py:class:`rawtoaces.Metadata`: container for DNG-style calibration/neutral metadata.
- :py:class:`rawtoaces.MetadataSolver`: solves CAT/IDT using :py:class:`rawtoaces.Metadata`.
- :py:class:`rawtoaces.SpectralData`: spectral JSON container/loader.
- :py:class:`rawtoaces.SpectralSolver`: solves WB/IDT using spectral datasets.


Metadata workflow
-----------------

Use :py:class:`rawtoaces.MetadataSolver` when you already have DNG metadata
values and do not need spectral database lookup.

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

Spectral workflow
-----------------

Use :py:class:`rawtoaces.SpectralSolver` for full spectral solving from camera
and illuminant spectral data.

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


Spectral Data
-------------

The :py:class:`rawtoaces.SpectralData` class is a minimal spectral data container exposed
for :py:class:`rawtoaces.SpectralSolver` workflows.
Corresponds to :cpp:class:`rta::core::SpectralData`.

File format and schema references:

- Format description:
  `rawtoaces-data README (JSON Schema for Spectral Datasets) <https://github.com/AcademySoftwareFoundation/rawtoaces-data/blob/main/README.md#json-schema-for-spectral-datasets>`_
- Current schema:
  `schema_1.0.0.json <https://github.com/AcademySoftwareFoundation/rawtoaces-data/blob/main/schema_1.0.0.json>`_
- Legacy schema (still found in many existing datasets):
  `schema_0.1.0.json <https://github.com/AcademySoftwareFoundation/rawtoaces-data/blob/main/schema_0.1.0.json>`_

Error handling notes
--------------------

- Most solver methods return ``True``/``False`` for operational success.
- On failure, ``last_error_message`` is populated for state-validation failures
  (for example, missing camera/illuminant before solving).
- Input-shape validation at the python binding layer raises ``ValueError``
  immediately for invalid arguments (empty required strings, wrong WB vector
  length, invalid metadata calibration array size).


Data requirements for ``calculate_transform``
----------------------------------------------

Before calling :py:meth:`SpectralSolver.calculate_transform`, ensure:

- ``camera`` has 3 channels (R, G, B)
- ``illuminant`` has 1 channel (power)
- ``observer`` has 3 channels (X, Y, Z)
- ``training_data`` is non-empty

If these conditions are not met, the method returns ``False`` and
``last_error_message`` describes the missing prerequisite.
