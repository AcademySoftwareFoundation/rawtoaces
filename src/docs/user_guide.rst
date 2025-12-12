User Guide
==========

Command Line Interface
----------------------

The ``rawtoaces`` command-line tool provides a comprehensive interface for converting
RAW images to ACES format.

Basic Syntax
^^^^^^^^^^^^

.. code-block:: bash

   rawtoaces [options] <input_files_or_directories>

White Balance Options
^^^^^^^^^^^^^^^^^^^^^

``--wb-method <method>``
   Specify the white balance method:

   - ``metadata`` (default): Use white balance from file metadata
   - ``illuminant``: White balance to a specific illuminant
   - ``box``: Calculate white balance from a region of the image
   - ``custom``: Use custom white balance multipliers

``--illuminant <name>``
   Specify the illuminant for white balancing. Can be:

   - A blackbody color temperature below 4000K (e.g., ``2800K``, ``3200K``)
   - A D-series illuminant (e.g., ``D50``, ``D55``, ``D65``)
   - Any illuminant name present in the data folder

``--wb-box <x,y,w,h>``
   Define a region for box white balance calculation.

``--custom-wb <r,g,b,g2>``
   Provide custom white balance multipliers.

Matrix Options
^^^^^^^^^^^^^^

``--mat-method <method>``
   Specify the color matrix method:

   - ``auto`` (default): Use spectral if available, otherwise metadata
   - ``spectral``: Use camera spectral sensitivity curves
   - ``metadata``: Use matrix from file metadata (DNG)
   - ``adobe``: Use Adobe color matrix from LibRaw
   - ``custom``: Use a custom 3x3 matrix

``--custom-matrix <m00,m01,...,m22>``
   Provide a custom 3x3 color transformation matrix.

Output Options
^^^^^^^^^^^^^^

``--output-dir <path>``
   Specify output directory for converted files.

``--overwrite``
   Allow overwriting existing output files.

``--create-dirs``
   Create output directories if they don't exist.

``--headroom <value>``
   Set the highlight headroom (default: 6.0 stops).

Cropping Options
^^^^^^^^^^^^^^^^

``--crop <mode>``
   Specify cropping mode:

   - ``off``: Write full sensor area
   - ``soft``: Full sensor area with crop marked as display window
   - ``hard`` (default): Write only the crop area

Camera Override Options
^^^^^^^^^^^^^^^^^^^^^^^

``--custom-camera-make <name>``
   Override camera manufacturer name.

``--custom-camera-model <name>``
   Override camera model name.

These options are useful when metadata is missing or incorrect.

Verbosity Options
^^^^^^^^^^^^^^^^^

``--verbose`` or ``-v``
   Enable verbose output.

``--timing``
   Show timing information for each processing step.

Examples
--------

Convert a single file with default settings:

.. code-block:: bash

   rawtoaces photo.dng

Convert using a specific illuminant:

.. code-block:: bash

   rawtoaces --wb-method illuminant --illuminant D55 photo.cr2

Batch convert a directory:

.. code-block:: bash

   rawtoaces --output-dir ./converted --overwrite /path/to/raw/files/

Use custom white balance:

.. code-block:: bash

   rawtoaces --wb-method custom --custom-wb 2.1,1.0,1.5,1.0 photo.nef
