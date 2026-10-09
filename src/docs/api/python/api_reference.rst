..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0


Python Reference
================

This reference describes the superset of Python bindings supported by the
current source. Methods accepting OpenImageIO objects require an extension
built with compatible OpenImageIO 3.2+ bindings. Check the
:ref:`capability table <python-oiio-availability>` before using those methods.

.. automodule:: rawtoaces

.. py:currentmodule:: rawtoaces

Classes
-------


.. autoclass:: rawtoaces.ImageConverter
    :members:

.. autoclass:: rawtoaces.SpectralData
    :members:

.. autoclass:: rawtoaces.TransformSolver
    :members:

.. autoclass:: rawtoaces.SpectralSolver
    :members:
    :inherited-members:
    :show-inheritance:

.. autoclass:: rawtoaces.Metadata
    :members:

.. autoclass:: rawtoaces.MetadataSolver
    :members:
    :inherited-members:
    :show-inheritance:

Utility Functions
-----------------

.. autofunction:: rawtoaces.collect_image_files
