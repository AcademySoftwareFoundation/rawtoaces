..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0

User Guide
==========

Command Line Interface
----------------------

The ``rawtoaces`` command-line tool provides a comprehensive interface for converting
RAW images to ACES format.

White balance, matrix, and crop mode names are case-sensitive. Pass each value of
a vector option as a separate space-separated argument.

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

``--wb-box <x y w h>``
   Define a region for box white balance calculation, using four separate integers
   for the origin and size. Use with ``--wb-method box``.

``--custom-wb <r g b g2>``
   Provide four separate custom white balance multipliers. Use with
   ``--wb-method custom``.

Matrix Options
^^^^^^^^^^^^^^

``--mat-method <method>``
   Specify the color matrix method:

   - ``auto`` (default): Use spectral if available, otherwise metadata
   - ``spectral``: Use camera spectral sensitivity curves
   - ``metadata``: Use matrix from file metadata (DNG)
   - ``Adobe``: Use Adobe color matrix from LibRaw
   - ``custom``: Use a full custom 3x3 camera RGB to ACES AP0 matrix

``--custom-mat <m00 m01 m02 m10 m11 m12 m20 m21 m22>``
   Provide a custom 3x3 camera RGB to ACES AP0 matrix as nine separate values in
   row order. Use with ``--mat-method custom``. The matrix is applied directly;
   supply the complete colour-space transform to ACES AP0 because no additional
   XYZ to ACES conversion is composed.

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

``--crop-mode <mode>``
   Specify cropping mode:

   - ``off``: Write full sensor area
   - ``soft`` (CLI default): Full sensor area with crop marked as display window
   - ``hard``: Write only the crop area

The C++ ``ImageConverter::Settings`` and Python ``ImageConverter.Settings`` APIs
default to hard cropping (``CropMode::Hard`` in C++, ``CropMode.Hard`` in Python).
The CLI defaults to ``soft``. Set the crop mode explicitly when matching results
across interfaces.

``--crop-box <x y w h>``
   Specify a custom crop using four separate integers for the origin and size.
   Without this option, the default crop should match the in-camera JPEG.

Camera Override Options
^^^^^^^^^^^^^^^^^^^^^^^

``--custom-camera-make <name>``
   Override camera manufacturer name.

``--custom-camera-model <name>``
   Override camera model name.

These options are useful when metadata is missing or incorrect.

.. _lens-correction-options:

Lens Correction Options
^^^^^^^^^^^^^^^^^^^^^^^

See :doc:`Lens Correction<lens_correction>` for more information on lens correction.

``--lens-correction <types>``
   Lens correction types to be applied to the images.
   Specify a string containing the following symbols in any order:
    
   - ``c``: chromatic abberation
   - ``d``: geometric distortion
   - ``v``: vignetting
   - ``a``: all of the above
    
``--require-lens-correction``
   Lens correction is treated as optional by default, i.e. the correction will
   only be applied if there is sufficient information about the camera/lens and
   the shot conditions (aperture, focal length, focus distance) is available
   for the image. Setting this flag will make lens correction mandatory, the
   conversion will fail if lens correction can't be applied.
    
``--custom-lens-make <name>``
   Override lens manufacturer name.
    
``--custom-lens-model <name>``
   Override lens model name.
   
``--custom-aperture <value>``
   Override aperture (F-number) value.
   
``--custom-focal-length <value>``
   Override focal length value (in millimetres).
      
``--custom-focus-distance <value>``
   Override focus distance value (in metres).

Verbosity Options
^^^^^^^^^^^^^^^^^

``--verbose`` or ``-v``
   Enable verbose output.

``--use-timing``
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

   rawtoaces --wb-method custom --custom-wb 2.1 1.0 1.5 1.0 photo.nef

Use a region for white balance:

.. code-block:: bash

   rawtoaces --wb-method box --wb-box 100 100 200 200 photo.nef

Use the Adobe matrix supplied by LibRaw:

.. code-block:: bash

   rawtoaces --mat-method Adobe photo.nef

Supply a camera RGB to ACES AP0 matrix. The identity matrix below only
illustrates the argument order; it does not calibrate camera RGB or convert it
to ACES AP0. Replace it with your camera's full colour-space transform:

.. code-block:: bash

   rawtoaces --mat-method custom --custom-mat 1 0 0 0 1 0 0 0 1 photo.nef

Write the full sensor area:

.. code-block:: bash

   rawtoaces --crop-mode off photo.dng

Write only a custom crop and show processing times:

.. code-block:: bash

   rawtoaces --crop-mode hard --crop-box 100 100 1000 800 --use-timing photo.dng
