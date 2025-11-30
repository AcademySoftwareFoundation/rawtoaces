Getting Started
===============

Installation
------------

From Source
^^^^^^^^^^^

RAWtoACES requires the following dependencies:

- CMake 3.12 or later
- C++17 compatible compiler
- OpenImageIO
- LibRaw
- Ceres Solver
- Eigen3

Build steps:

.. code-block:: bash

   git clone https://github.com/AcademySoftwareFoundation/rawtoaces.git
   cd rawtoaces
   mkdir build && cd build
   cmake ..
   make
   make install

Python Bindings
^^^^^^^^^^^^^^^

If you want to use the Python bindings, ensure Python development headers are installed
and enable the bindings during configuration:

.. code-block:: bash

   cmake -DRTA_BUILD_PYTHON_BINDINGS=ON ..

Environment Setup
-----------------

RAWtoACES needs access to camera spectral sensitivity data and other reference data.
The data is automatically fetched during the build process and installed to
``share/rawtoaces/data``.

You can override the data location using environment variables:

- ``RAWTOACES_DATA_PATH``: Primary path to search for data files
- ``AMPAS_DATA_PATH``: Alternative path (for compatibility)

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
