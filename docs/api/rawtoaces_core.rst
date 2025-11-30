RAWtoACES Core
==============

The core library provides low-level functionality for spectral calculations,
color science operations, and data management.

.. contents:: Contents
   :local:
   :depth: 2

Overview
--------

The core library (``rawtoaces_core``) contains:

- Spectral solver for calculating IDT matrices from camera spectral sensitivities
- Database management for camera data, illuminants, and training spectra
- Color science utilities

SpectralSolver Class
--------------------

.. doxygenclass:: rta::core::SpectralSolver
   :members:
   :protected-members:
   :undoc-members:

Database Classes
----------------

CameraDatabase
^^^^^^^^^^^^^^

.. doxygenclass:: rta::core::CameraDatabase
   :members:
   :undoc-members:

IlluminantDatabase
^^^^^^^^^^^^^^^^^^

.. doxygenclass:: rta::core::IlluminantDatabase
   :members:
   :undoc-members:

Utility Functions
-----------------

.. doxygenfunction:: rta::core::database_paths
