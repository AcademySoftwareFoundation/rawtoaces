..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0

Lens Correction
===============

Rawtoaces can correct the following lens phenomena when correction is requested:

- Chromatic Aberration
- Geometric Distortion
- Vignetting

Lens correction is disabled by default. Select the correction types using
``--lens-correction`` or ``settings.lens_correction_types`` to enable it.

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

Pass the RAW path as ``<INPUT_PATH>``. Output is ``<input_stem>_aces.exr`` beside
the input. The API examples set soft crop — full sensor, crop stored as the
display window — to match the CLI. C++ and Python default to hard crop.

Request correction of geometric distortion and vignetting:

.. tabs::
  .. tab:: Shell
    .. code-block:: bash

      rawtoaces --crop-mode soft --lens-correction dv <INPUT_PATH>

  .. tab:: C++
    .. code-block:: C++

      #include <rawtoaces/image_converter.h>
      #include <iostream>

      int main(int argc, char **argv)
      {
          if (argc != 2)
          {
              std::cerr << "Usage: lens_correction <INPUT_PATH>\n";
              return 1;
          }
          const std::string input_path = argv[1];
          rta::util::ImageConverter converter;

          converter.settings.crop_mode =
              rta::util::ImageConverter::Settings::CropMode::Soft;
          converter.settings.lens_correction_types =
              rta::util::ImageConverter::Settings::LensCorrectionType::Distortion |
              rta::util::ImageConverter::Settings::LensCorrectionType::Vignetting;

          if (!converter.process_image(input_path))
          {
              std::cerr << converter.last_error_message << '\n';
              return 1;
          }
          return 0;
      }

  .. tab:: Python
    .. code-block:: Python

      import sys
      import rawtoaces

      if len(sys.argv) != 2:
          raise SystemExit("Usage: lens_correction.py <INPUT_PATH>")
      input_path = sys.argv[1]
      converter = rawtoaces.ImageConverter()

      converter.settings.crop_mode = rawtoaces.ImageConverter.Settings.CropMode.Soft
      converter.settings.lens_correction_types = (
          rawtoaces.ImageConverter.Settings.LensCorrectionType.Distortion |
          rawtoaces.ImageConverter.Settings.LensCorrectionType.Vignetting
      )
      if not converter.process_image(input_path):
          raise RuntimeError(converter.last_error_message)

Without ``--require-lens-correction`` (or ``require_lens_correction = True``),
a missing profile or other failure warns and conversion continues uncorrected.
A successful conversion does not mean the correction was applied.

Request every supported correction, make it mandatory, and override the camera
and lens information used for correction. Match the overrides to the image, and
use a Lensfun profile calibrated for every requested correction:

.. tabs::
  .. tab:: Shell
    .. code-block:: bash

      rawtoaces                                    \
      --crop-mode soft                            \
      --lens-correction a                          \
      --require-lens-correction                    \
      --custom-camera-make "Canon"                 \
      --custom-camera-model "EOS R5"               \
      --custom-lens-make "Canon"                   \
      --custom-lens-model "RF 15-35mm F2.8L IS USM" \
      --custom-aperture 4.0                        \
      --custom-focal-length 35.0                   \
      --custom-focus-distance 240.0                \
      <INPUT_PATH>

  .. tab:: C++
    .. code-block:: C++

      #include <rawtoaces/image_converter.h>
      #include <iostream>

      int main(int argc, char **argv)
      {
          if (argc != 2)
          {
              std::cerr << "Usage: lens_correction <INPUT_PATH>\n";
              return 1;
          }
          const std::string input_path = argv[1];
          rta::util::ImageConverter converter;

          converter.settings.crop_mode =
              rta::util::ImageConverter::Settings::CropMode::Soft;
          converter.settings.lens_correction_types =
              rta::util::ImageConverter::Settings::LensCorrectionType::Aberration |
              rta::util::ImageConverter::Settings::LensCorrectionType::Distortion |
              rta::util::ImageConverter::Settings::LensCorrectionType::Vignetting;
          converter.settings.require_lens_correction = true;
          converter.settings.custom_camera_make = "Canon";
          converter.settings.custom_camera_model = "EOS R5";
          converter.settings.custom_lens_make = "Canon";
          converter.settings.custom_lens_model = "RF 15-35mm F2.8L IS USM";
          converter.settings.custom_aperture = 4.0f;
          converter.settings.custom_focal_length = 35.0f;
          converter.settings.custom_focus_distance = 240.0f;

          if (!converter.process_image(input_path))
          {
              std::cerr << converter.last_error_message << '\n';
              return 1;
          }
          return 0;
      }

  .. tab:: Python
    .. code-block:: Python

      import sys
      import rawtoaces

      if len(sys.argv) != 2:
          raise SystemExit("Usage: lens_correction.py <INPUT_PATH>")
      input_path = sys.argv[1]
      converter = rawtoaces.ImageConverter()

      converter.settings.crop_mode = rawtoaces.ImageConverter.Settings.CropMode.Soft
      converter.settings.lens_correction_types = (
          rawtoaces.ImageConverter.Settings.LensCorrectionType.Aberration |
          rawtoaces.ImageConverter.Settings.LensCorrectionType.Distortion |
          rawtoaces.ImageConverter.Settings.LensCorrectionType.Vignetting
      )
      converter.settings.require_lens_correction = True
      converter.settings.custom_camera_make = "Canon"
      converter.settings.custom_camera_model = "EOS R5"
      converter.settings.custom_lens_make = "Canon"
      converter.settings.custom_lens_model = "RF 15-35mm F2.8L IS USM"
      converter.settings.custom_aperture = 4.0
      converter.settings.custom_focal_length = 35.0
      converter.settings.custom_focus_distance = 240.0
      
      if not converter.process_image(input_path):
          raise RuntimeError(converter.last_error_message)

.. note::
  Overriding the camera make and model affects both lens correction and
  spectral colour space conversion mode.
