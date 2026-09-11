..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0

Python API
==========

The python bindings are currently work in progress.

At this stage we provide:

- The file-based methods of the :py:class:`ImageConverter` class.
- Core solver bindings for metadata-based and spectral workflows:
  :py:class:`Metadata`, :py:class:`MetadataSolver`,
  :py:class:`SpectralData`, and :py:class:`SpectralSolver`.

Lower-level methods that operate directly with OpenImageIO image buffers
are not currently available.

.. toctree::
   :maxdepth: 2

   image_converter
   rawtoaces_core
   api_index
