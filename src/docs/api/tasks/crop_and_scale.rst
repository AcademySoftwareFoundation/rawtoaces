..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0

.. _task-crop-scale:

Crop and scale
==============

Write only the rectangle ``x=100``, ``y=100``, ``width=1000``,
``height=800``, and multiply pixel values by ``headroom * scale``.
These examples set ``headroom`` to ``12`` and ``scale`` to ``1``, so the
factor is 12. The default headroom is ``6``; doubling it adds one stop.

Hard crop drops pixels outside the rectangle. Soft crop would keep the
full sensor and record the rectangle as the display window. ``off``
writes the full sensor and ignores the crop box. The origin and size are
in the decoded image. A zero crop box means the default crop, which
should match the in-camera JPEG. This example sets a non-zero box, so
that default is not used.

The output is ``<stem>_aces.exr`` beside the input. A failed call sets
``status`` and ``last_error_message`` and does not write the file.

.. tabs::

   .. group-tab:: Shell

      .. code-block:: bash

         rawtoaces --crop-mode hard --crop-box 100 100 1000 800 \
             --headroom 12 --scale 1 \
             photo.dng

   .. group-tab:: C++

      .. code-block:: cpp

         #include <rawtoaces/image_converter.h>
         #include <iostream>
         #include <string>

         int main(int argc, char **argv)
         {
             if (argc != 2)
             {
                 std::cerr << "Usage: crop_and_scale <INPUT_PATH>\n";
                 return 1;
             }
             const std::string input_path = argv[1];
             rta::util::ImageConverter converter;

             converter.settings.crop_mode =
                 rta::util::ImageConverter::Settings::CropMode::Hard;
             converter.settings.crop_box[0] = 100;
             converter.settings.crop_box[1] = 100;
             converter.settings.crop_box[2] = 1000;
             converter.settings.crop_box[3] = 800;
             converter.settings.headroom = 12.0f;
             converter.settings.scale = 1.0f;

             if (!converter.process_image(input_path))
             {
                 std::cerr << converter.last_error_message << '\n';
                 return 1;
             }
             return 0;
         }

   .. group-tab:: Python

      ``crop_box`` must contain four integers. A list of another length
      raises ``ValueError``.

      .. code-block:: python

         import sys
         import rawtoaces

         if len(sys.argv) != 2:
             raise SystemExit("Usage: crop_and_scale.py <INPUT_PATH>")
         input_path = sys.argv[1]
         converter = rawtoaces.ImageConverter()
         converter.settings.crop_mode = (
             rawtoaces.ImageConverter.Settings.CropMode.Hard
         )
         converter.settings.crop_box = [100, 100, 1000, 800]
         converter.settings.headroom = 12.0
         converter.settings.scale = 1.0
         if not converter.process_image(input_path):
             raise RuntimeError(converter.last_error_message)

See also :doc:`convert one image <convert_one>` and the crop options in
the :doc:`user guide <../../user_guide>`.
