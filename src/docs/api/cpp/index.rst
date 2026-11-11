..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0

C++ API
=======

This section contains the API documentation for the rawtoaces C++ libraries,
automatically generated from the source code using Doxygen and Breathe.

Start with the :doc:`API overview <../overview>` to choose file conversion, an
OpenImageIO buffer pipeline, or a metadata or spectral solve. The
:doc:`task examples <../tasks/index>` show each task in C++ and Python.
Go directly to
:doc:`C++ Reference <api_reference>` for class and function signatures. The
matching Python buffer methods require OpenImageIO 3.2 or later
(:ref:`overview-oiio-notice`).

.. toctree::
   :maxdepth: 3

   image_converter
   rawtoaces_core
   spectral_data
   api_reference

Namespaces
----------

rawtoaces organizes its code into the following namespaces:

``rta::util``
   The utility library containing the high-level :cpp:class:`rta::util::ImageConverter` class
   for easy RAW-to-ACES conversion.

``rta::core``
   The core library containing lower-level functionality for spectral calculations,
   color science, and data management.
