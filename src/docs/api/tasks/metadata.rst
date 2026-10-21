..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0

.. _task-metadata:

Solve from metadata
===================

Build a camera RGB to ACES matrix from DNG calibration you already have.
:cpp:class:`rta::core::MetadataSolver` and
:py:class:`rawtoaces.MetadataSolver` do not decode a file and do not read
the spectral database. There is no command-line equivalent.

Pass ``neutral_RGB``, ``baseline_exposure``, and two calibration sets.
Each set has an EXIF light-source tag and an XYZ-to-camera-RGB matrix.
The numbers below are sample calibration values, not a profile for a
particular camera. On success ``calculate_transform`` returns true and
``transform_matrix`` holds a 3×3 matrix. That matrix includes chromatic
adaptation. A false return clears ``transform_matrix`` and leaves the
reason in ``last_error_message``. Read the matrix only after a true
return.

.. tabs::

   .. group-tab:: C++

      Assign the two ``calibration`` elements in place. The solver copies
      the metadata it is given.

      .. code-block:: cpp

         #include <rawtoaces/rawtoaces_core.h>
         #include <iostream>

         int main()
         {
             rta::core::Metadata metadata;
             metadata.baseline_exposure = 2.4;
             metadata.neutral_RGB = {0.6290, 1.0, 0.7904};

             metadata.calibration[0].illuminant = 17;
             metadata.calibration[0].XYZ_to_RGB_matrix = {
                 1.31197, -0.49679, 0.01156,
                 -0.41723, 1.44237, 0.04528,
                 0.06723, 0.21710, 0.72651,
             };
             metadata.calibration[1].illuminant = 21;
             metadata.calibration[1].XYZ_to_RGB_matrix = {
                 1.00885, -0.27351, -0.08258,
                 -0.48997, 1.34441, 0.11174,
                 -0.06406, 0.32997, 0.53917,
             };

             rta::core::MetadataSolver solver(metadata);
             if (!solver.calculate_transform())
             {
                 std::cerr << solver.last_error_message << '\n';
                 return 1;
             }
             const auto &transform = solver.transform_matrix;
             return transform.empty() ? 1 : 0;
         }

   .. group-tab:: Python

      ``calibration`` must be a list of two
      :py:class:`rawtoaces.Metadata.Calibration` objects. Assign that list
      back to the property. The property returns a new list, so changing
      an item in the list you just read does not update the metadata.

      .. code-block:: python

         import rawtoaces

         metadata = rawtoaces.Metadata()
         metadata.baseline_exposure = 2.4
         metadata.neutral_RGB = [0.6290, 1.0, 0.7904]

         first = rawtoaces.Metadata.Calibration()
         first.illuminant = 17
         first.XYZ_to_RGB_matrix = [
             1.31197, -0.49679, 0.01156,
             -0.41723, 1.44237, 0.04528,
             0.06723, 0.21710, 0.72651,
         ]
         second = rawtoaces.Metadata.Calibration()
         second.illuminant = 21
         second.XYZ_to_RGB_matrix = [
             1.00885, -0.27351, -0.08258,
             -0.48997, 1.34441, 0.11174,
             -0.06406, 0.32997, 0.53917,
         ]
         metadata.calibration = [first, second]

         solver = rawtoaces.MetadataSolver(metadata)
         if not solver.calculate_transform():
             raise RuntimeError(solver.last_error_message)
         transform = solver.transform_matrix

      A ``neutral_RGB`` or matrix of the wrong length raises
      ``ValueError``.

The :doc:`Python core guide <../python/rawtoaces_core>`
describes the same workflow and the deprecated solver methods.
Signatures are in the :doc:`C++ Reference <../cpp/api_reference>` and
the :doc:`Python Reference <../python/api_reference>`.
