..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0

.. _task-convert-one:

Convert one image
=================

Convert one RAW file to an ACES EXR with
:cpp:func:`rta::util::ImageConverter::process_image` or
:py:meth:`rawtoaces.ImageConverter.process_image`.

The examples set soft crop, so the full sensor is written and the crop is
stored as the display window. That matches the command-line default. The
C++ and Python APIs would otherwise write only the crop rectangle. White
balance stays on file metadata, the matrix method stays ``auto``,
``headroom`` stays ``6``, and ``scale`` stays ``1``. Pixel values are
multiplied by ``headroom * scale``.

The output is ``<stem>_aces.exr`` beside the input. For ``photo.dng`` that
is ``photo_aces.exr`` in the same directory. ``overwrite`` stays false, so
an existing output makes the call fail and the file is left unchanged.
``create_dirs`` is not involved.

A failed command exits non-zero and prints the reason. C++ and Python
return false and set ``status`` and ``last_error_message``. DNG is the
format that carries the calibration used by the metadata white balance.

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
                 std::cerr << "Usage: convert_one <INPUT_PATH>\n";
                 return 1;
             }
             const std::string input_path = argv[1];
             rta::util::ImageConverter converter;
             converter.settings.crop_mode =
                 rta::util::ImageConverter::Settings::CropMode::Soft;

             if (!converter.process_image(input_path))
             {
                 std::cerr << converter.last_error_message << '\n';
                 return 1;
             }
             return 0;
         }

      C++ types live in ``rta::util``.

   .. group-tab:: Python

      .. code-block:: python

         import sys
         import rawtoaces

         if len(sys.argv) != 2:
             raise SystemExit("Usage: convert_one.py <INPUT_PATH>")
         input_path = sys.argv[1]
         converter = rawtoaces.ImageConverter()
         converter.settings.crop_mode = (
             rawtoaces.ImageConverter.Settings.CropMode.Soft
         )
         if not converter.process_image(input_path):
             raise RuntimeError(converter.last_error_message)

      Python raises from ``last_error_message``. The binding itself
      returns ``False`` for a failed conversion. Passing a white-balance
      or matrix vector of the wrong length raises ``ValueError`` before
      the conversion runs.

See also :doc:`white balance and matrix <white_balance>`,
:doc:`batch conversion <batch>`, :doc:`errors <errors>`, and the
:doc:`API overview <../overview>`.
