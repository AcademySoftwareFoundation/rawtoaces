..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0

Python API
==========

The Python bindings provide file conversion and metadata/spectral transform
solving. Start with the :doc:`image conversion guide <image_converter>` or the
:doc:`core workflows <rawtoaces_core>`, then use the
:doc:`Python Reference <api_reference>` for signatures and settings.

The main classes are:

- :py:class:`rawtoaces.ImageConverter` for image conversion.
- Core solver bindings for metadata-based and spectral workflows:
  :py:class:`rawtoaces.Metadata`, :py:class:`rawtoaces.MetadataSolver`,
  :py:class:`rawtoaces.SpectralData`, and :py:class:`rawtoaces.SpectralSolver`.

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
   * - ``apply_lens_correction``, ``apply_matrix``, ``apply_scale``, and
       ``apply_crop``
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

.. toctree::
   :maxdepth: 2

   image_converter
   rawtoaces_core
   api_index
