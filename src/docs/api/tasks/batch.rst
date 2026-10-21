..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0

.. _task-batch:

Convert a batch
===============

Convert loose files and the RAW files in a directory with one set of
settings. :cpp:func:`rta::util::collect_image_files` and
:py:func:`rawtoaces.collect_image_files` group the arguments. Individual
files share the first batch. Each directory becomes a later batch.
Directory collection is not recursive. A missing path is reported on
standard error and skipped. Files that are not a supported RAW extension
are skipped. ``.DS_Store`` is ignored.

These examples use soft crop, write every output under the absolute
directory ``/path/to/output``, and replace an existing ``<stem>_aces.exr``.
``create_dirs`` creates that directory and not its parents. Two inputs
with the same file stem share one output name; with ``overwrite`` set,
the later file replaces the earlier one.

The command and the library examples stop at the first conversion
failure. Files already written are left in place. The command exits
non-zero and prints the reason. If nothing was collected, the command
prints its help and exits zero. The library examples treat that empty
result as a failure so the caller can tell that no file was converted.

For ``photo.dng`` the output is ``/path/to/output/photo_aces.exr``.

.. tabs::

   .. group-tab:: Shell

      .. code-block:: bash

         rawtoaces --crop-mode soft \
             --output-dir /path/to/output --create-dirs --overwrite \
             photo.dng /path/to/raw/files

   .. group-tab:: C++

      .. code-block:: cpp

         #include <rawtoaces/image_converter.h>
         #include <iostream>
         #include <string>
         #include <vector>

         int main(int argc, char **argv)
         {
             if (argc < 2)
             {
                 std::cerr << "Usage: convert_batch <INPUT>...\n";
                 return 1;
             }
             const std::vector<std::string> paths(argv + 1, argv + argc);
             rta::util::ImageConverter converter;
             converter.settings.crop_mode =
                 rta::util::ImageConverter::Settings::CropMode::Soft;
             converter.settings.output_dir = "/path/to/output";
             converter.settings.create_dirs = true;
             converter.settings.overwrite = true;

             const auto batches = rta::util::collect_image_files(paths);
             int converted = 0;
             for (const auto &batch : batches)
             {
                 for (const auto &input_path : batch)
                 {
                     if (!converter.process_image(input_path))
                     {
                         std::cerr << input_path << ": "
                                   << converter.last_error_message << '\n';
                         return 1;
                     }
                     ++converted;
                 }
             }
             if (converted == 0)
             {
                 std::cerr << "No supported RAW files were found.\n";
                 return 1;
             }
             return 0;
         }

   .. group-tab:: Python

      .. code-block:: python

         import sys
         import rawtoaces

         if len(sys.argv) < 2:
             raise SystemExit("Usage: convert_batch.py <INPUT>...")
         converter = rawtoaces.ImageConverter()
         converter.settings.crop_mode = (
             rawtoaces.ImageConverter.Settings.CropMode.Soft
         )
         converter.settings.output_dir = "/path/to/output"
         converter.settings.create_dirs = True
         converter.settings.overwrite = True

         batches = rawtoaces.collect_image_files(sys.argv[1:])
         converted = 0
         for batch in batches:
             for input_path in batch:
                 if not converter.process_image(input_path):
                     raise RuntimeError(
                         f"{input_path}: {converter.last_error_message}"
                     )
                 converted += 1
         if converted == 0:
             raise SystemExit("No supported RAW files were found.")

The first batch is empty when every argument is a directory. Loop over
every batch, including that empty one. See also
:doc:`convert one image <convert_one>`.
