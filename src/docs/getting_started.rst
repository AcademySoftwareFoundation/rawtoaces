..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0

Getting Started
===============

Installation
------------

Dependencies
^^^^^^^^^^^^

The default CMake configuration builds the CLI, C++ libraries, Python bindings,
and tests, with Eigen, Ceres, Lensfun, and the spectral database enabled.
Install development headers and libraries as well as command line tools:

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Dependency
     - Requirement
   * - CMake and compiler
     - CMake 3.12 or later and a C++17 compatible compiler.
   * - OpenImageIO
     - OpenImageIO 3.0 or later, including a RAW input plugin built with LibRaw
       support and an OpenEXR output plugin.
   * - nlohmann-json
     - Required JSON headers and CMake package configuration.
   * - Eigen3 and Ceres Solver
     - Enabled by default. Ceres may also require Eigen and other dependencies
       from the package it was built against.
   * - Lensfun and pkg-config
     - Lensfun 0.3.2 or later, discoverable through pkg-config. Required when
       ``RTA_ENABLE_LENSFUN=ON`` (the default).
   * - Python and nanobind
     - Python 3.8 or later, its development headers, and nanobind's CMake package.
       Required when ``RTA_BUILD_PYTHON_BINDINGS=ON`` (the default).
   * - pytest
     - Needed to run Python tests when both bindings and tests are enabled.

ExifTool is an optional runtime tool for fetching missing image metadata. Lens
correction also needs Lensfun's profile database; that database is separate from
the rawtoaces spectral database.

On macOS, the native dependencies can be obtained from Homebrew:

.. code-block:: bash

   brew install cmake eigen ceres-solver nlohmann-json openimageio lensfun pkgconf
   export CMAKE_PREFIX_PATH="$(brew --prefix):$(brew --prefix eigen)"

The source-build and conversion steps below were checked with these dependencies
already installed on macOS. Distribution packages on Linux and Windows differ
in their versions and optional OpenImageIO plugins. This guide does not claim
that an untested package-manager recipe supplies a compatible default build.
Install the dependencies listed above through your package manager or from
source, then use the same CMake build steps. The old CentOS 7 Word documents in
the repository describe historical installations.

If CMake cannot find a dependency, add its installation prefix to
``CMAKE_PREFIX_PATH`` or provide its package directory, such as
``-DEigen3_DIR=/path/to/eigen/share/eigen3/cmake``. For Lensfun, ensure
``pkg-config --modversion lensfun`` succeeds; set ``PKG_CONFIG_PATH`` to the
directory containing ``lensfun.pc`` for a custom installation. A Ceres package
must find the Eigen version it was compiled against.

From Source
^^^^^^^^^^^

This example builds the CLI and C++ libraries while disabling the Python
bindings and tests, so it does not require Python or nanobind:

.. code-block:: bash

   git clone https://github.com/AcademySoftwareFoundation/rawtoaces.git
   cd rawtoaces
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
     -DCMAKE_INSTALL_PREFIX="$PWD/install" \
     -DRTA_BUILD_PYTHON_BINDINGS=OFF -DRTA_BUILD_TESTS=OFF
   cmake --build build --config Release
   cmake --install build --config Release
   ./install/bin/rawtoaces --version
   ./install/bin/rawtoaces --list-formats

``--config Release`` selects the configuration for generators such as Visual
Studio and Xcode; ``CMAKE_BUILD_TYPE`` selects it for single-configuration
generators. Set the install prefix during configuration: this project resolves
some installation directories to absolute paths at that point.

The install layout under the configured prefix is:

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - Default location
     - Contents
   * - ``bin``
     - ``rawtoaces`` executable.
   * - ``lib``
     - ``rawtoaces_core`` and ``rawtoaces_util`` libraries, and the Python
       extension when enabled. Shared-library version suffixes follow the
       platform convention: for example, ``librawtoaces_core.2.2.0.dylib`` on
       macOS, with an unversioned ``librawtoaces_core.dylib`` link.
   * - ``include/rawtoaces``
     - Public C++ headers.
   * - ``lib/cmake/RAWTOACES``
     - CMake package configuration and exported library targets.
   * - ``share/rawtoaces/data``
     - Spectral data and its license on macOS and Unix when
       ``RTA_INSTALL_DATABASE=ON``.

Library and executable locations can be changed with ``INSTALL_LIB_DIR`` and
``INSTALL_BIN_DIR``. The Python extension installs into the configured library
directory, rather than automatically into Python's site-packages.

Build Options
^^^^^^^^^^^^^

.. list-table::
   :header-rows: 1
   :widths: 35 15 50

   * - Option
     - Default
     - Effect
   * - ``RTA_BUILD_PYTHON_BINDINGS``
     - ``ON``
     - Build the Python extension; requires Python development files and
       nanobind.
   * - ``RTA_BUILD_TESTS``
     - ``ON``
     - Build the C++ tests and register Python tests when bindings are enabled.
   * - ``RTA_INSTALL_DATABASE``
     - ``ON``
     - Fetch the pinned spectral data repository during configuration and
       install its data on macOS and Unix. Requires network access to fetch it.
   * - ``RTA_ENABLE_LENSFUN``
     - ``ON``
     - Enable lens corrections; requires Lensfun and pkg-config.
   * - ``RTA_ENABLE_EIGEN`` / ``RTA_ENABLE_CERES``
     - ``ON``
     - Use Eigen/Ceres for numerical operations. Disabling either selects
       experimental alternatives. Disabling Eigen alone does not remove a
       Ceres package's own Eigen dependency.
   * - ``ENABLE_SHARED``
     - ``ON``
     - Build shared C++ libraries.

To build the Python extension, select the interpreter before configuring. For
example, prepare a virtual environment and install its build and test packages:

.. code-block:: bash

   python3 -m venv .venv
   . .venv/bin/activate
   python -m pip install nanobind pytest
   cmake -S . -B build-python -DCMAKE_BUILD_TYPE=Release \
     -DCMAKE_INSTALL_PREFIX="$PWD/install-python" \
     -DPython_EXECUTABLE="$VIRTUAL_ENV/bin/python"
   cmake --build build-python --config Release
   cmake --install build-python --config Release

Use the platform's virtual-environment activation command on Windows.
``RTA_PYTHON_VERSION`` can additionally constrain CMake to an exact Python
version. The interpreter must have matching development headers. Use that same
interpreter to import the compiled extension. On macOS and Unix, for example:

.. code-block:: bash

   PYTHONPATH="$PWD/install-python/lib" python -c \
     "import rawtoaces; print(rawtoaces.ImageConverter)"

For builds against OpenImageIO 3.2 or later, ``rawtoaces`` also imports the
``OpenImageIO`` Python extension and checks its nanobind type registrations.
Install a compatible OpenImageIO Python extension for the selected interpreter,
with its native dependencies available to the runtime loader. Merely installing
nanobind or satisfying the OpenImageIO version number is not sufficient. See
:doc:`api/python/index` for method availability and setup details.

Environment Setup
-----------------

With ``RTA_INSTALL_DATABASE=ON``, CMake fetches the spectral data into
``build/_deps/rawtoaces_data-src/data`` and installs it to
``<CMAKE_INSTALL_PREFIX>/share/rawtoaces/data`` on macOS and Unix. The library
does not derive its runtime search path from a custom install prefix, so pass
that data directory explicitly when using the local installation above.

On Windows, the data is fetched but not installed automatically. Keep the
fetched data directory or obtain the
`rawtoaces-data repository <https://github.com/AcademySoftwareFoundation/rawtoaces-data>`_
and pass its ``data`` directory. That repository also describes dataset usage
and the schema.

The CLI selects its data search paths in this order:

1. ``--data-dir`` overrides the other locations.
2. ``RAWTOACES_DATA_PATH`` supplies the primary environment override.
3. ``AMPAS_DATA_PATH`` is a deprecated fallback when ``RAWTOACES_DATA_PATH`` is
   unset.
4. Platform defaults, including ``/usr/local/share/rawtoaces/data`` on macOS
   and Unix and ``/opt/homebrew/share/rawtoaces/data`` on macOS. Windows defaults
   to the current directory. The old ``/usr/local/include/rawtoaces/data`` path
   is retained only as a compatibility search location.

Separate multiple data directories with ``:`` on macOS and Unix, or ``;`` on
Windows. The path must identify the directory containing ``camera``,
``illuminant``, and the other spectral datasets, rather than the camera
subdirectory alone.

Basic Usage
-----------

Replace ``input.dng`` with an existing RAW file:

.. code-block:: bash

   ./install/bin/rawtoaces --data-dir "$PWD/install/share/rawtoaces/data" \
     --output-dir "$PWD/output" --create-dirs --crop-mode soft input.dng

This uses metadata white balance and the ``auto`` matrix method, which selects
spectral solving if camera sensitivities are available and otherwise selects
the metadata matrix method. It writes ``input_aces.exr`` into ``output``.
Existing output files are preserved unless ``--overwrite`` is supplied.

The CLI's crop default is ``soft``: it retains the sensor area and records the
crop as the EXR display window. C++ and Python ``ImageConverter.Settings``
default to hard cropping. Choose the mode explicitly when comparing interfaces.
For the full sensor area without a crop display window, use
``--crop-mode off``. To write only the crop area, use ``--crop-mode hard``.

To use the Adobe camera coefficients supplied by LibRaw:

.. code-block:: bash

   ./install/bin/rawtoaces --mat-method Adobe --crop-mode soft input.dng

Illuminant white balance and the spectral matrix method require measured camera
sensitivities in the spectral database. For example, with ``rawtoaces`` on the
command search path and a configured data path:

.. code-block:: bash

   rawtoaces --wb-method illuminant --illuminant D60 \
     --mat-method spectral --crop-mode soft input.dng

Use ``rawtoaces --help`` for the options supported by the installed build. For
batch conversion and other options, see :doc:`user_guide`.

Migrating from v1.x
--------------------

The v2 CLI uses named methods and long options. The tables below translate
option fragments; append an input path to a complete conversion command.
``X Y W H`` and multiplier/matrix names are placeholders for separate numeric
arguments, without commas or angle brackets.

White Balance and Matrix Methods
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. list-table::
   :header-rows: 1
   :widths: 30 45 25

   * - v1 option fragment
     - Current option fragment
     - Meaning
   * - ``--wb-method 0``
     - ``--wb-method metadata``
     - Metadata multipliers.
   * - ``--wb-method 1 D65``
     - ``--wb-method illuminant --illuminant D65``
     - Spectral illuminant white balance.
   * - ``--wb-method 2``
     - ``--wb-method box``
     - Whole-image neutral average.
   * - ``--wb-method 3 X Y W H``
     - ``--wb-method box --wb-box X Y W H``
     - Neutral average of a region.
   * - ``--wb-method 4 R G B G``
     - ``--wb-method custom --custom-wb R G B G``
     - Four custom multipliers.
   * - ``--mat-method 0``
     - ``--mat-method spectral``
     - Spectral transform.
   * - ``--mat-method 1``
     - ``--mat-method metadata``
     - Metadata matrices.
   * - ``--mat-method 2``
     - ``--mat-method Adobe``
     - LibRaw's Adobe coefficients; case-sensitive name.
   * - ``--mat-method 3 M00 M01 M02 M10 M11 M12 M20 M21 M22``
     - ``--mat-method custom --custom-mat M00 M01 M02 M10 M11 M12 M20 M21 M22``
     - Complete camera RGB to ACES AP0 transform, in row order.

Supply a complete camera RGB to ACES AP0 transform. Both v1 and v2 custom matrix
modes apply the supplied matrix directly to decoded camera RGB pixels before
headroom and scale, without adding chromatic adaptation or an XYZ to ACES
conversion. Older help and documentation incorrectly described this as camera
RGB to XYZ.

RAW and Diagnostic Options
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. list-table::
   :header-rows: 1
   :widths: 25 40 35

   * - v1 option fragment
     - Current option fragment
     - Migration note
   * - ``-c 0.8``
     - ``--adjust-maximum-threshold 0.8``
     - Adjust the maximum threshold.
   * - ``-C 1.2 1.1``
     - ``--chromatic-aberration 1.2 1.1``
     - Red and blue correction factors.
   * - ``-k 512``
     - ``--black-level 512``
     - Override the black level.
   * - ``-S 15000``
     - ``--saturation-level 15000``
     - Override the saturation level.
   * - ``-n 150``
     - ``--denoise-threshold 150``
     - Wavelet threshold. The current CLI parses this as an integer; do not
       assume a fractional v1 threshold is preserved.
   * - ``-H 1``
     - ``--highlight-mode 1``
     - Highlight recovery mode.
   * - ``-W``
     - Omit ``--auto-bright``.
     - Automatic brightening is already disabled by default. Supplying
       ``--auto-bright`` enables it and reverses the intent of ``-W``.
   * - ``-b 1.1``
     - ``--scale 1.1`` for final pixel scaling.
     - This is a different processing stage: v1 adjusted LibRaw's decoder
       brightness, while ``--scale`` multiplies transformed pixels together
       with the headroom factor. It is not a guarantee of identical pixels.
   * - ``--headroom 6``
     - ``--headroom 6``
     - Linear multiplier, not a number of exposure stops.
   * - ``-q 3``
     - ``--demosaic AHD``
     - Select AHD by name.
   * - ``-t 0``
     - ``--flip 0``
     - Disable rotation. Other orientations use the current EXIF codes; v1's
       numeric orientation codes must not be copied unchanged.
   * - ``-f``
     - No equivalent option.
     - v1's four-color interpolation flag was not an orientation flag.
   * - ``-h``
     - ``--half-size``
     - Decode at half-size resolution.
   * - ``-B X Y W H``
     - ``--crop-box X Y W H --crop-mode hard``
     - Explicitly request cropped pixels; the CLI default ``soft`` retains
       the sensor area instead.
   * - ``-P path``
     - ``--bad-pixels-path path``
     - LibRaw-format bad-pixel file.
   * - ``-d``
     - ``--use-timing``
     - Print processing timings.
   * - ``--valid-illums``
     - ``--list-illuminants``
     - List spectral illuminants.
   * - ``--valid-cameras``
     - ``--list-cameras``
     - List cameras with spectral data, not every LibRaw-supported camera.

Output files are no longer overwritten automatically. Use ``--overwrite`` only
when replacement is intended. ``--output-dir`` accepts an absolute directory or
a directory relative to each input's directory; ``--create-dirs`` creates
missing output directories.
