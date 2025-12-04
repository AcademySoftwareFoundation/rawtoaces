Getting Started
===============

Installation
------------

From Source
^^^^^^^^^^^

rawtoaces requires the following dependencies:

- CMake 3.12 or later
- C++17 compatible compiler
- OpenImageIO (with RAW support via LibRaw)
- Ceres Solver
- Eigen3
- nlohmann-json

Build steps:

.. code-block:: bash

   git clone https://github.com/AcademySoftwareFoundation/rawtoaces.git
   cd rawtoaces
   mkdir build && cd build
   cmake ..
   make
   make install

Environment Setup
-----------------

rawtoaces needs access to camera spectral sensitivity data and other reference data.
The data is automatically fetched during the build process and installed to
``<CMAKE_INSTALL_PREFIX>/share/rawtoaces/data`` on macOS and Linux.

.. note::

   On Windows, the data files are not installed automatically. You will need to
   manually specify the data location.

You can override the data location using:

- The ``--data-dir`` command line parameter
- ``RAWTOACES_DATA_PATH`` environment variable: Primary path to search for data files
- ``AMPAS_DATA_PATH`` environment variable: Alternative path (for compatibility)

Basic Usage
-----------

The simplest way to convert a RAW file:

.. code-block:: bash

   rawtoaces input.dng

This will:

1. Read the RAW file
2. Apply white balance using metadata from the file
3. Calculate the color transformation matrix (using spectral data if available)
4. Write an ACES-compliant EXR file

For more options, see the :doc:`user_guide`.
