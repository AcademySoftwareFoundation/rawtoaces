..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0


.. _sec-pythonimageconverter:

ImageConverter
==============

The ImageConverter class allows high level raw file conversion.

Usage examples
--------------

.. code-block:: python

  import rawtoaces

  input_path = "/path/to/input/file.CR3"
  converter = rawtoaces.ImageConverter()
  if not converter.process_image(input_path):
      raise RuntimeError(converter.last_error_message)

This will convert the input file ``/path/to/input/file.CR3`` using the default
settings, and create the output file ``/path/to/input/file_aces.exr`` in the same
directory.

Using a named illuminant for white balance requires camera sensitivities, colour
matching functions, and training data in the spectral database. Set
``RAWTOACES_DATA_PATH`` when that database is not on the default search path.
This example uses spectral conversion and the generated 3200K blackbody:

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

This will convert the input file ``/path/to/input/file.CR3`` white-balancing to
the colour temperature of 3200K, and create the output file
``/path/to/output/file_aces.exr`` at the provided location.

Output scaling
--------------

Pixel values are multiplied by ``settings.headroom * settings.scale``.
``headroom`` is a linear factor (default ``6.0``). Doubling headroom is
equivalent to adding one exposure stop. An adjustment of ``s`` stops
corresponds to multiplying the factor by ``2 ** s``.

.. _python-oiio-availability:

OpenImageIO interoperability
----------------------------

The extension's compile-time OpenImageIO version determines whether the
lower-level image APIs are present. The reference deliberately describes a
superset; a method shown there may be absent from your installed build.

.. list-table:: Python capabilities
   :header-rows: 1
   :widths: 55 20 25

   * - Operation
     - Built with OIIO before 3.2
     - Built with OIIO 3.2+ and compatible Python bindings
   * - ``process_image(input_filename)``, ``configure(input_filename)``,
       settings, and supported-data queries
     - Available
     - Available
   * - ``MetadataSolver``, ``SpectralSolver``, and ``SpectralData``
     - Available
     - Available
   * - ``configure(image_spec, options)``
     - Absent
     - Available
   * - ``load_image`` and ``save_image``
     - Absent
     - Available
   * - ``apply_lens_correction``, ``apply_matrix``, ``apply_scale``,
       ``apply_crop``, and ``process_buffer``
     - Absent
     - Available

For a build compiled with OpenImageIO 3.2 or later, importing ``rawtoaces``
imports the ``OpenImageIO`` Python module and checks that ``ImageBuf``,
``ParamValueList``, and ``ImageSpec`` are registered with nanobind. A matching
version number alone is insufficient: the OpenImageIO Python module and the
extension must have compatible type registrations and native dependencies.
These import requirements also apply when you only use file conversion or
core solvers in that build.

After importing the extension successfully, you can check whether buffer
methods are exposed:

.. code-block:: python

   import rawtoaces

   has_oiio_buffers = hasattr(rawtoaces.ImageConverter, "load_image")
