..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0


.. _sec-pythonimageconverter:

ImageConverter
==============

The ImageConverter class allows high level raw file conversion.

Usage examples:

.. code-block:: python

  import rawtoaces
  converter = rawtoaces.ImageConverter()
  converter.process_image("/path/to/input/file.CR3")

This will convert the input file "/path/to/input/file.CR3" using the default
settings, and create the output file "/path/to/input/file_aces.exr" in the same
directory.

.. code-block:: python

  import rawtoaces
  converter = rawtoaces.ImageConverter()
  converter.settings.WB_method == rawtoaces.ImageConverter.Settings.WBMethod.Illuminant
  converter.settings.illuminant = "3200K"
  converter.settings.output_dir = "/path/to/output"
  converter.process_image("/path/to/an/image/file.CR3")
    
This will convert the input file "/path/to/input/file.CR3" white-balancing to
the colour temperature of 3200K, and create the output file
"/path/to/output/file_aces.exr" at the provided location.
