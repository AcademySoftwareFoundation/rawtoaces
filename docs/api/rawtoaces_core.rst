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
- Functions for calculating illuminant spectral power distributions
- Color science utilities and matrices

SpectralSolver Class
--------------------

.. doxygenclass:: rta::core::SpectralSolver
   :members:
   :protected-members:
   :undoc-members:

Utility Functions
-----------------

.. doxygenfunction:: rta::core::calculate_daylight_SPD

.. doxygenfunction:: rta::core::calculate_blackbody_SPD
