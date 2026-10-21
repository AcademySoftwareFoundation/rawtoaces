..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0

Python API
==========

The Python bindings provide file conversion and metadata/spectral transform
solving. Start with the :doc:`API overview <../overview>` to choose a layer.
The :doc:`task examples <../tasks/index>` show each task in C++ and Python.
Then use the :doc:`image conversion guide <image_converter>` or the
:doc:`core workflows <rawtoaces_core>`. The
:doc:`Python Reference <api_reference>` lists signatures and settings.

The main classes are:

- :py:class:`rawtoaces.ImageConverter` for file-based image conversion and
  (when supported by the build) lower-level OpenImageIO operations.
- Core solver bindings for metadata-based and spectral workflows:
  :py:class:`rawtoaces.Metadata`, :py:class:`rawtoaces.MetadataSolver`,
  :py:class:`rawtoaces.Spectrum`, :py:class:`rawtoaces.SpectralData`, and
  :py:class:`rawtoaces.SpectralSolver`.

Some lower-level :py:class:`rawtoaces.ImageConverter` methods require a
compatible OpenImageIO 3.2+ build; see
:ref:`python-oiio-availability`.

.. toctree::
   :maxdepth: 2

   image_converter
   rawtoaces_core
   api_reference
