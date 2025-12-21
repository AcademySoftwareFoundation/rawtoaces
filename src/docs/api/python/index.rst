..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0

Python bindings
=============

The python bindings are currently work in progress. 

At this stage we only provide the binding for the file-based methods of the ImageConverter class, 
which should be enough to configure the converter and process a raw image file. 
All conversion types and optional parameters, which are available in the command line tool, 
are also available in the bindings. 

The lower level methods, which operate with OpenImageIO image buffers, are not currently available.

.. toctree::
   :maxdepth: 2

   image_converter
