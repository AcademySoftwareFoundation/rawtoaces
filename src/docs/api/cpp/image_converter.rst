..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0

ImageConverter
==============

The ``ImageConverter`` class is the main interface for converting RAW images to ACES format.
It provides a high-level API that handles the entire conversion pipeline.

.. contents:: Contents
   :local:
   :depth: 2

Overview
--------

The ``ImageConverter`` class encapsulates the complete RAW-to-ACES conversion workflow:

1. Configure white balance and color matrix methods
2. Load the RAW image
3. Apply color transformations
4. Apply exposure scaling and cropping
5. Save the ACES container output

Basic Usage
-----------

Pass the path to a supported RAW file as the first argument to this program.
It writes ``<input_stem>_aces.exr`` beside the input file.

.. code-block:: cpp

   #include <rawtoaces/image_converter.h>
   #include <iostream>

   int main(int argc, char **argv)
   {
       if (argc != 2)
       {
           std::cerr << "Usage: convert <INPUT_PATH>\n";
           return 1;
       }
       const std::string input_path = argv[1];
       rta::util::ImageConverter converter;

       converter.settings.WB_method = rta::util::ImageConverter::Settings::WBMethod::Metadata;
       converter.settings.matrix_method = rta::util::ImageConverter::Settings::MatrixMethod::Auto;

       if (!converter.process_image(input_path))
       {
           std::cerr << converter.last_error_message << '\n';
           return 1;
       }
       return 0;
   }

Output Scaling
--------------

After applying the colour transform, the converter multiplies pixel values by
``settings.headroom * settings.scale``. ``headroom`` is a linear factor with a
default of ``6.0``. Changing it from ``6.0`` to ``12.0`` doubles the output scale,
equivalent to adding one exposure stop. To apply an adjustment of ``s`` stops,
multiply ``settings.scale`` by ``std::pow(2.0, s)`` (from ``<cmath>``).
