..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0

.. _task-white-balance:

White balance and matrix
========================

``WB_method`` chooses how camera RGB is balanced.
``matrix_method`` chooses the camera RGB to ACES transform. The settings
below are the same operation in each language. Command-line method names
are case-sensitive. ``Adobe`` is the spelling of that matrix method.

Both examples set soft crop. An absolute ``output_dir`` of
``/path/to/output`` places ``<stem>_aces.exr`` in that directory.
``create_dirs`` creates the directory itself and does not create missing
parents, so ``/path/to`` must already exist. A failed call leaves
``status`` and ``last_error_message`` set and does not write the output.

Illuminant and spectral matrix
------------------------------

White balance to D55 and solve the matrix from camera spectral
sensitivities. This needs the spectral database: camera sensitivities,
an observer, and training spectra. Set ``RAWTOACES_DATA_PATH`` when that
database is not on the default search path. ``D55`` and ``3200K`` are
accepted in any case.

The output for ``photo.dng`` is ``/path/to/output/photo_aces.exr``.

.. tabs::

   .. group-tab:: Shell

      .. code-block:: bash
         :caption: Shell

         rawtoaces --crop-mode soft \
             --wb-method illuminant --illuminant D55 \
             --mat-method spectral \
             --output-dir /path/to/output --create-dirs \
             photo.dng

   .. group-tab:: C++

      .. code-block:: cpp
         :caption: C++

         #include <rawtoaces/image_converter.h>
         #include <iostream>
         #include <string>

         int main(int argc, char **argv)
         {
             if (argc != 2)
             {
                 std::cerr << "Usage: convert_illuminant <INPUT_PATH>\n";
                 return 1;
             }
             const std::string input_path = argv[1];
             rta::util::ImageConverter converter;

             converter.settings.crop_mode =
                 rta::util::ImageConverter::Settings::CropMode::Soft;
             converter.settings.WB_method =
                 rta::util::ImageConverter::Settings::WBMethod::Illuminant;
             converter.settings.matrix_method =
                 rta::util::ImageConverter::Settings::MatrixMethod::Spectral;
             converter.settings.illuminant = "D55";
             converter.settings.output_dir = "/path/to/output";
             converter.settings.create_dirs = true;

             if (!converter.process_image(input_path))
             {
                 std::cerr << converter.last_error_message << '\n';
                 return 1;
             }
             return 0;
         }

   .. group-tab:: Python

      .. code-block:: python
         :caption: Python

         import sys
         import rawtoaces

         if len(sys.argv) != 2:
             raise SystemExit("Usage: convert_illuminant.py <INPUT_PATH>")
         input_path = sys.argv[1]
         converter = rawtoaces.ImageConverter()
         converter.settings.crop_mode = (
             rawtoaces.ImageConverter.Settings.CropMode.Soft
         )
         converter.settings.WB_method = (
             rawtoaces.ImageConverter.Settings.WBMethod.Illuminant
         )
         converter.settings.matrix_method = (
             rawtoaces.ImageConverter.Settings.MatrixMethod.Spectral
         )
         converter.settings.illuminant = "D55"
         converter.settings.output_dir = "/path/to/output"
         converter.settings.create_dirs = True
         if not converter.process_image(input_path):
             raise RuntimeError(converter.last_error_message)

Custom multipliers
------------------

Use four white-balance multipliers from an external tool, and take the
colour matrix from DNG metadata. The fourth multiplier is the second
green. Metadata matrix conversion applies to DNG. The output is
``<stem>_aces.exr`` beside the input.

A custom matrix is a different method: ``--mat-method custom`` and
``custom_matrix``. It must already be the complete camera RGB to ACES AP0
transform. The converter applies it as given and does not add adaptation
or an XYZ to ACES step. The :doc:`user guide <../../user_guide>` shows
the nine-value argument order.

.. tabs::

   .. group-tab:: Shell

      .. code-block:: bash
         :caption: Shell

         rawtoaces --crop-mode soft \
             --wb-method custom --custom-wb 2.1 1.0 1.5 1.0 \
             --mat-method metadata \
             photo.dng

   .. group-tab:: C++

      ``custom_WB`` is a 4-element array.

      .. code-block:: cpp
         :caption: C++

         #include <rawtoaces/image_converter.h>
         #include <iostream>
         #include <string>

         int main(int argc, char **argv)
         {
             if (argc != 2)
             {
                 std::cerr << "Usage: convert_custom_wb <INPUT_PATH>\n";
                 return 1;
             }
             const std::string input_path = argv[1];
             rta::util::ImageConverter converter;

             converter.settings.crop_mode =
                 rta::util::ImageConverter::Settings::CropMode::Soft;
             converter.settings.WB_method =
                 rta::util::ImageConverter::Settings::WBMethod::Custom;
             converter.settings.custom_WB[0] = 2.1f;
             converter.settings.custom_WB[1] = 1.0f;
             converter.settings.custom_WB[2] = 1.5f;
             converter.settings.custom_WB[3] = 1.0f;
             converter.settings.matrix_method =
                 rta::util::ImageConverter::Settings::MatrixMethod::Metadata;

             if (!converter.process_image(input_path))
             {
                 std::cerr << converter.last_error_message << '\n';
                 return 1;
             }
             return 0;
         }

   .. group-tab:: Python

      Assigning ``custom_WB`` with a length other than 4 raises
      ``ValueError`` and does not start the conversion.

      .. code-block:: python
         :caption: Python

         import sys
         import rawtoaces

         if len(sys.argv) != 2:
             raise SystemExit("Usage: convert_custom_wb.py <INPUT_PATH>")
         input_path = sys.argv[1]
         converter = rawtoaces.ImageConverter()
         converter.settings.crop_mode = (
             rawtoaces.ImageConverter.Settings.CropMode.Soft
         )
         converter.settings.WB_method = (
             rawtoaces.ImageConverter.Settings.WBMethod.Custom
         )
         converter.settings.custom_WB = [2.1, 1.0, 1.5, 1.0]
         converter.settings.matrix_method = (
             rawtoaces.ImageConverter.Settings.MatrixMethod.Metadata
         )
         if not converter.process_image(input_path):
             raise RuntimeError(converter.last_error_message)

See also :doc:`convert one image <convert_one>` and
:doc:`solve from spectra <spectra>`.
