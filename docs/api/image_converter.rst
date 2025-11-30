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
4. Apply scaling and cropping
5. Save the ACES EXR output

Basic Usage
-----------

.. code-block:: cpp

   #include <rawtoaces/image_converter.h>

   rta::util::ImageConverter converter;

   // Configure settings
   converter.settings.WB_method = rta::util::ImageConverter::Settings::WBMethod::Metadata;
   converter.settings.matrix_method = rta::util::ImageConverter::Settings::MatrixMethod::Auto;
   converter.settings.overwrite = true;

   // Process an image
   bool success = converter.process_image("input.dng");

Class Reference
---------------

ImageConverter Class
^^^^^^^^^^^^^^^^^^^^

.. doxygenclass:: rta::util::ImageConverter
   :members:
   :protected-members:
   :undoc-members:

Settings Structure
^^^^^^^^^^^^^^^^^^

.. doxygenstruct:: rta::util::ImageConverter::Settings
   :members:
   :undoc-members:

Enumerations
^^^^^^^^^^^^

.. doxygenenum:: rta::util::ImageConverter::Settings::WBMethod

.. doxygenenum:: rta::util::ImageConverter::Settings::MatrixMethod

.. doxygenenum:: rta::util::ImageConverter::Settings::CropMode

Utility Functions
-----------------

.. doxygenfunction:: rta::util::collect_image_files
