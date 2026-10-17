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
  converter = rawtoaces.ImageConverter()
  converter.process_image("/path/to/input/file.CR3")

This will convert the input file ``/path/to/input/file.CR3`` using the default
settings, and create the output file ``/path/to/input/file_aces.exr`` in the same
directory.

.. code-block:: python

  import rawtoaces
  converter = rawtoaces.ImageConverter()
  converter.settings.WB_method == rawtoaces.ImageConverter.Settings.WBMethod.Illuminant
  converter.settings.illuminant = "3200K"
  converter.settings.output_dir = "/path/to/output"
  converter.process_image("/path/to/an/image/file.CR3")

This will convert the input file ``/path/to/input/file.CR3`` white-balancing to
the colour temperature of 3200K, and create the output file
``/path/to/output/file_aces.exr`` at the provided location.

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
