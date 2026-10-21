..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0

.. _task-buffer:

OpenImageIO buffers
===================

Run the same conversion as :cpp:func:`rta::util::ImageConverter::process_image`
when the application owns the image buffer. There is no separate command-line
form. The command line calls this sequence internally.

The order is the one ``process_image`` uses: build the output path,
``configure``, ``load_image``, ``process_buffer``, then ``save_image``.
``process_buffer`` applies lens correction, the matrix, scale, and crop.
Soft crop is set so the result matches :doc:`convert one image <convert_one>`.
``save_image`` stores half-float samples unless another type is passed.
``get_WB_multipliers`` and ``get_transform_matrix`` are valid after a
successful ``configure``.

Each step returns false and sets ``status`` and ``last_error_message``.
The examples stop at the first failure. Changing settings that affect
the transform requires another ``configure``.

The output is ``<stem>_aces.exr`` beside the input. ``overwrite`` stays
false, so an existing file is refused before the image is read.

C++ provides this sequence in ``rawtoaces_util``. Python provides
``configure(input_filename)`` in every build. ``load_image``,
``process_buffer``, ``save_image``, and ``configure`` with an
OpenImageIO parameter list require OpenImageIO 3.2 or later, as described
in :ref:`overview-oiio-notice`. Python has no ``make_output_path``; the
Python example builds the same file name and applies the same overwrite
check.

.. tabs::

   .. group-tab:: C++

      .. code-block:: cpp
         :caption: C++

         #include <rawtoaces/image_converter.h>
         #include <OpenImageIO/imagebuf.h>
         #include <OpenImageIO/paramlist.h>
         #include <iostream>
         #include <string>

         int main(int argc, char **argv)
         {
             if (argc != 2)
             {
                 std::cerr << "Usage: convert_buffer <INPUT_PATH>\n";
                 return 1;
             }
             const std::string input_path = argv[1];
             rta::util::ImageConverter converter;
             converter.settings.crop_mode =
                 rta::util::ImageConverter::Settings::CropMode::Soft;
             converter.settings.overwrite = false;

             std::string output_path = input_path;
             if (!converter.make_output_path(output_path))
             {
                 std::cerr << converter.last_error_message << '\n';
                 return 1;
             }

             OIIO::ParamValueList hints;
             if (!converter.configure(input_path, hints))
             {
                 std::cerr << converter.last_error_message << '\n';
                 return 1;
             }

             OIIO::ImageBuf buffer;
             if (!converter.load_image(input_path, hints, buffer))
             {
                 std::cerr << converter.last_error_message << '\n';
                 return 1;
             }
             if (!converter.process_buffer(buffer, buffer))
             {
                 std::cerr << converter.last_error_message << '\n';
                 return 1;
             }
             if (!converter.save_image(output_path, buffer))
             {
                 std::cerr << converter.last_error_message << '\n';
                 return 1;
             }
             return 0;
         }

   .. group-tab:: Python

      On a build without the buffer methods this script stops before
      calling them. Use ``process_image`` on that build.

      .. code-block:: python
         :caption: Python

         import sys
         from pathlib import Path
         import rawtoaces

         if len(sys.argv) != 2:
             raise SystemExit("Usage: convert_buffer.py <INPUT_PATH>")
         if not hasattr(rawtoaces.ImageConverter, "load_image"):
             raise SystemExit(
                 "This build has no OpenImageIO buffer methods. "
                 "Use process_image, or import an extension built with "
                 "OpenImageIO 3.2 or later."
             )

         import OpenImageIO as oiio

         input_path = sys.argv[1]
         converter = rawtoaces.ImageConverter()
         converter.settings.crop_mode = (
             rawtoaces.ImageConverter.Settings.CropMode.Soft
         )
         converter.settings.overwrite = False

         source = Path(input_path)
         output_path = Path(str(source.with_suffix("")) + "_aces.exr")
         if output_path.exists() and not converter.settings.overwrite:
             raise RuntimeError(
                 f"Output file already exists: '{output_path}'."
             )

         hints = oiio.ParamValueList()
         if not converter.configure(input_path, hints):
             raise RuntimeError(converter.last_error_message)
         buffer = oiio.ImageBuf()
         if not converter.load_image(input_path, hints, buffer):
             raise RuntimeError(converter.last_error_message)
         if not converter.process_buffer(buffer, buffer):
             raise RuntimeError(converter.last_error_message)
         if not converter.save_image(str(output_path), buffer):
             raise RuntimeError(converter.last_error_message)

See also :doc:`handle errors <errors>` and the
:doc:`API overview <../overview>`.
