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

  input_path = "/path/to/input/file.CR3"
  converter = rawtoaces.ImageConverter()
  if not converter.process_image(input_path):
      raise RuntimeError(converter.last_error_message)

This will convert the input file "/path/to/input/file.CR3" using the default
settings, and create the output file "/path/to/input/file_aces.exr" in the same
directory.

To white-balance to a specified illuminant, the spectral database must contain
camera sensitivity measurements for the input camera, along with the colour
matching functions and training data needed to solve its transform. Set
``RAWTOACES_DATA_PATH`` to the database location if it is not installed in a
default search directory. The following example explicitly selects spectral
conversion and uses the generated 3200K blackbody illuminant:

.. code-block:: python

  import rawtoaces

  input_path = "/path/to/input/file.CR3"
  converter = rawtoaces.ImageConverter()
  converter.settings.WB_method = rawtoaces.ImageConverter.Settings.WBMethod.Illuminant
  converter.settings.matrix_method = rawtoaces.ImageConverter.Settings.MatrixMethod.Spectral
  converter.settings.illuminant = "3200K"
  converter.settings.output_dir = "/path/to/output"
  converter.settings.create_dirs = True
  if not converter.process_image(input_path):
      raise RuntimeError(converter.last_error_message)
    
This will convert the input file "/path/to/input/file.CR3" white-balancing to
the colour temperature of 3200K, and create the output file
"/path/to/output/file_aces.exr" at the provided location.

Output scaling
--------------

After applying the colour transform, the converter multiplies pixel values by
``settings.headroom * settings.scale``. ``headroom`` is a linear factor with a
default of ``6.0``. Changing it from ``6.0`` to ``12.0`` doubles the output scale,
equivalent to adding one exposure stop. To apply an adjustment of ``s`` stops,
multiply ``settings.scale`` by ``2 ** s``.
