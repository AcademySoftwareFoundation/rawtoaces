..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0

.. _task-spectra:

Solve from spectra
==================

Solve white balance and a camera RGB to ACES matrix from measured
spectra. :cpp:class:`rta::core::SpectralSolver` and
:py:class:`rawtoaces.SpectralSolver` do not decode a RAW file. There is
no command-line equivalent. Pass the spectral database directories to
the constructor, or set ``RAWTOACES_DATA_PATH`` before a solver that
uses the default search.

Before ``calculate_transform``, the camera needs three channels, the
illuminant one, the observer three, and the training set must be
non-empty. ``find_camera`` and the name form of ``find_illuminant`` load
those two objects. ``d55`` and ``3200k`` are generated; any other name
must exist in the database. The name match is case-insensitive. The name
form does not calculate white balance. Call ``calculate_WB`` after it, or
use the three-multiplier form of ``find_illuminant`` when you already
have multipliers.

On success ``get_WB_multipliers`` returns the RGB multipliers and
``transform_matrix`` holds the 3×3 transform. A missing camera,
illuminant, observer, or training set makes ``calculate_transform``
return false and sets ``last_error_message``. Leaving white balance
unset keeps the multipliers at ``[1.0, 1.0, 1.0]`` and is not reported
as an error. Read both results only after the calls that fill them
return true.

The dataset layout is documented with the
`rawtoaces-data repository <https://github.com/AcademySoftwareFoundation/rawtoaces-data>`_.
The paths below are relative to a database directory from that layout.

.. tabs::

   .. group-tab:: C++

      .. code-block:: cpp

         #include <rawtoaces/rawtoaces_core.h>
         #include <iostream>
         #include <string>
         #include <vector>

         int main()
         {
             const std::vector<std::string> search_paths = {
                 "/path/to/rawtoaces-data/data"
             };
             rta::core::SpectralSolver solver(search_paths);

             if (!solver.find_camera("nikon", "d200"))
             {
                 std::cerr << solver.last_error_message << '\n';
                 return 1;
             }
             if (!solver.find_illuminant("d55"))
             {
                 std::cerr << solver.last_error_message << '\n';
                 return 1;
             }
             if (!solver.load_spectral_data(
                     "cmf/cmf_1931.json", solver.observer))
             {
                 std::cerr << solver.last_error_message << '\n';
                 return 1;
             }
             if (!solver.load_spectral_data(
                     "training/training_spectral.json", solver.training_data))
             {
                 std::cerr << solver.last_error_message << '\n';
                 return 1;
             }
             if (!solver.calculate_WB())
             {
                 std::cerr << solver.last_error_message << '\n';
                 return 1;
             }
             if (!solver.calculate_transform())
             {
                 std::cerr << solver.last_error_message << '\n';
                 return 1;
             }

             const auto &white_balance = solver.get_WB_multipliers();
             const auto &transform = solver.transform_matrix;
             return white_balance.empty() || transform.empty() ? 1 : 0;
         }

      C++ can also fill a spectrum with ``calculate_daylight_SPD`` or
      ``calculate_blackbody_SPD`` and adapt D65 XYZ with
      ``XYZD65Solver``.

   .. group-tab:: Python

      An empty illuminant name, or a multiplier list whose length is not
      three, raises ``ValueError``.

      .. code-block:: python

         import rawtoaces

         solver = rawtoaces.SpectralSolver([
             "/path/to/rawtoaces-data/data"
         ])
         if not solver.find_camera("nikon", "d200"):
             raise RuntimeError(solver.last_error_message or "camera not found")
         if not solver.find_illuminant("d55"):
             raise RuntimeError(
                 solver.last_error_message or "illuminant not found"
             )
         if not solver.load_spectral_data("cmf/cmf_1931.json", solver.observer):
             raise RuntimeError(
                 solver.last_error_message or "failed to load observer"
             )
         if not solver.load_spectral_data(
             "training/training_spectral.json", solver.training_data
         ):
             raise RuntimeError(
                 solver.last_error_message or "failed to load training data"
             )
         if not solver.calculate_WB():
             raise RuntimeError(solver.last_error_message)
         if not solver.calculate_transform():
             raise RuntimeError(solver.last_error_message)

         white_balance = solver.get_WB_multipliers()
         transform = solver.transform_matrix

      ``calculate_daylight_SPD``, ``calculate_blackbody_SPD``, and
      ``XYZD65Solver`` are C++ only. Python reaches the built-in
      illuminants through ``find_illuminant``, as this example does.

The :doc:`Python core guide <../python/rawtoaces_core>` explains the
containers. Signatures are in the
:doc:`C++ Reference <../cpp/api_reference>` and the
:doc:`Python Reference <../python/api_reference>`.
