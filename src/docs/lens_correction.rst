..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0

Lens Correction
===============

Rawtoaces automatically corrects the following lens phenomena:

- Chromatic Aberration
- Geometric Distortion
- Vignetting

To apply lens correction, Rawtoaces requires two key pieces of information:

- Lens and Shot Information
- Lens Profile

**Lens and shot information** (including lens make and model, as well as aperture,
focal length, and distance values) is typically extracted automatically from the
raw image file's EXIF metadata. If any required information is missing,
rawtoaces utilizes `ExifTool <https://exiftool.org/>`_ to attempt to retrieve the
missing data.
Alternatively, you can provide the information manually using command-line
:ref:`lens-correction-options` or programmatically
through the properties of the :cpp:class:`rta::util::ImageConverter::Settings` object.

**Lens profiles** are managed by the `Lensfun <https://lensfun.github.io/>`_
project. If the Lensfun library cannot automatically locate the Lensfun
database, you may need to specify the path using the RAWTOACES_LENSFUNDB_PATH
environment variable.

Basic usage:
------------

Request correction of geometric distortion and vignetting:

.. tabs::
  .. tab:: Shell
    .. code-block:: bash

      rawtoaces --lens-correction dv <INPUT_PATH>

  .. tab:: C++
    .. code-block:: C++

      #include <rawtoaces/rawtoaces_util.h>
      
      rta::util::ImageConverter converter;
      
      converter.settings.lens_correction_types =
        rta::util::ImageConverter::Settings::LensCorrectionType::Distortion |
        rta::util::ImageConverter::Settings::LensCorrectionType::Vignetting;

  .. tab:: Python
    .. code-block:: Python

      import rawtoaces
      
      converter = rawtoaces.ImageConverter()
      
      converter.settings.lens_correction_types =
        rawtoaces.ImageConverter.Settings.LensCorrectionType.Distortion |
        rawtoaces.ImageConverter.Settings.LensCorrectionType.Vignetting

Request all supported types of lens correction, make the correction mandatory,
override all camera/lens info used for correction:

.. tabs::
  .. tab:: Shell
    .. code-block:: bash

      rawtoaces                                    \
      --lens-correction a                          \
      --require-lens-correction                    \
      --custom-camera-make "Canon"                 \
      --custom-camera-model "EOS R5"               \
      --custom-lens-make "Canon"                   \
      --custom-lens-model "RF 24-105mm F4L IS USM" \
      --custom-aperture 4.0                        \
      --custom-focal-length 35.0                   \
      --custom-focus-distance 240.0                \
      <INPUT_PATH>

  .. tab:: C++
    .. code-block:: C++

      #include <rawtoaces/rawtoaces_util.h>
      
      rta::util::ImageConverter converter;
      
      converter.settings.lens_correction_types =
        rta::util::ImageConverter::Settings::LensCorrectionType::Aberration |
        rta::util::ImageConverter::Settings::LensCorrectionType::Distortion |
        rta::util::ImageConverter::Settings::LensCorrectionType::Vignetting;
      converter.settings.require_lens_correction = true;
      converter.settings.custom_camera_make = "Canon";
      converter.settings.custom_camera_model = "EOS R5";
      converter.settings.custom_lens_make = "Canon";
      converter.settings.custom_lens_model = "RF 24-105mm F4L IS USM";
      converter.settings.custom_aperture = 4.0f;
      converter.settings.custom_focal_length = 35.0f;
      converter.settings.custom_focus_distance = 240.0f;
      
      converter.process_image(input_path);

  .. tab:: Python
    .. code-block:: Python

      import rawtoaces
      
      converter = rawtoaces.ImageConverter()
      
      converter.settings.lens_correction_types =
        rawtoaces.ImageConverter.Settings.LensCorrectionType.Aberration |
        rawtoaces.ImageConverter.Settings.LensCorrectionType.Distortion |
        rawtoaces.ImageConverter.Settings.LensCorrectionType.Vignetting
      converter.settings.require_lens_correction = True
      converter.settings.custom_camera_make = "Canon"
      converter.settings.custom_camera_model = "EOS R5"
      converter.settings.custom_lens_make = "Canon"
      converter.settings.custom_lens_model = "RF 24-105mm F4L IS USM"
      converter.settings.custom_aperture = 4.0
      converter.settings.custom_focal_length = 35.0
      converter.settings.custom_focus_distance = 240.0
      
      converter.process_image(input_path);

.. note::
  Overriding the camera make and model affects both lens correction and
  spectral colour space conversion mode.
