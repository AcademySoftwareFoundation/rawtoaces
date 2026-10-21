..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0

.. _task-errors:

Handle errors
=============

File conversion reports failure in the same three places in C++ and
Python: a false return, ``status``, and ``last_error_message``. The
output file is not written. Read ``status`` to tell a missing input
apart from an existing output, a missing output directory, a missing
spectral database, or a failure while configuring, reading, correcting
the lens, applying the matrix, scaling, cropping, or writing.

These examples use soft crop and leave ``overwrite`` false. A second run
on the same input fails because ``<stem>_aces.exr`` is already beside
the input. The command prints that reason and a hint to pass
``--overwrite``, then exits non-zero. It stops a batch at the first
failed file.

Solver calls use the same false return and ``last_error_message``. They
do not use ``ImageConverter.Status``. See
:doc:`solve from metadata <metadata>` and
:doc:`solve from spectra <spectra>`.

.. tabs::

   .. group-tab:: Shell

      .. code-block:: bash

         rawtoaces --crop-mode soft photo.dng

   .. group-tab:: C++

      .. code-block:: cpp

         #include <rawtoaces/image_converter.h>
         #include <iostream>
         #include <string>

         int main(int argc, char **argv)
         {
             if (argc != 2)
             {
                 std::cerr << "Usage: convert_checked <INPUT_PATH>\n";
                 return 1;
             }
             const std::string input_path = argv[1];
             rta::util::ImageConverter converter;
             converter.settings.crop_mode =
                 rta::util::ImageConverter::Settings::CropMode::Soft;
             converter.settings.overwrite = false;

             if (!converter.process_image(input_path))
             {
                 std::cerr << converter.last_error_message << '\n';
                 if (converter.status ==
                     rta::util::ImageConverter::Status::FileExists)
                 {
                     std::cerr << "Set overwrite to replace the existing file.\n";
                 }
                 return 1;
             }
             return 0;
         }

   .. group-tab:: Python

      .. code-block:: python

         import sys
         import rawtoaces

         if len(sys.argv) != 2:
             raise SystemExit("Usage: convert_checked.py <INPUT_PATH>")
         input_path = sys.argv[1]
         converter = rawtoaces.ImageConverter()
         converter.settings.crop_mode = (
             rawtoaces.ImageConverter.Settings.CropMode.Soft
         )
         converter.settings.overwrite = False
         if not converter.process_image(input_path):
             message = converter.last_error_message
             if converter.status == rawtoaces.ImageConverter.Status.FileExists:
                 message += " Set overwrite to replace the existing file."
             raise RuntimeError(message)

      A vector of the wrong length raises ``ValueError`` and does not
      change the setting. C++ ``custom_WB`` is a fixed 4-element array,
      so the same mistake is a compile error rather than a runtime status.

      .. code-block:: python

         import rawtoaces

         converter = rawtoaces.ImageConverter()
         try:
             converter.settings.custom_WB = [1.0, 1.0, 1.0]
         except ValueError as error:
             print(error)

      ``Metadata.calibration`` raises ``ValueError`` unless it is assigned
      a sequence of two calibration sets. See
      :doc:`solve from metadata <metadata>`.

See also :doc:`OpenImageIO buffers <buffer>` for a check after each
pipeline step.
