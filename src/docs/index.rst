rawtoaces Documentation
=======================

rawtoaces is a tool for converting RAW camera images to ACES (Academy Color Encoding System) format.
It provides both a command-line utility and a C++ library for integration into other applications.

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   getting_started
   user_guide
   api/index
   contributing

Features
--------

- Convert RAW images from various camera manufacturers to ACES EXR files
- Multiple white balance methods: metadata, illuminant, box averaging, or custom
- Multiple matrix methods: spectral (using camera sensitivity data), metadata, Adobe, or custom
- Flexible cropping options
- Batch processing support

Quick Start
-----------

Command Line
^^^^^^^^^^^^

.. code-block:: bash

   # Basic conversion using camera metadata
   rawtoaces input.dng

   # Convert with specific illuminant
   rawtoaces --wb-method illuminant --illuminant D55 input.cr2

   # Batch convert all files in a directory
   rawtoaces /path/to/raw/files/

Library Usage (C++)
^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

   #include <rawtoaces/image_converter.h>

   rta::util::ImageConverter converter;
   converter.settings.WB_method = rta::util::ImageConverter::Settings::WBMethod::Metadata;
   converter.settings.matrix_method = rta::util::ImageConverter::Settings::MatrixMethod::Auto;

   bool success = converter.process_image("input.dng");

Indices and tables
==================

* :ref:`genindex`
* :ref:`search`
