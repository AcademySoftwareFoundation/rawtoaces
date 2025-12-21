..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0


.. _sec-pythonimageconverter:

ImageConverter
==============

.. py:class:: ImageConverter

  The ImageConverter class which corresponds to the C++ `rta::util::ImageConverter`.
  It allows high level raw file conversion.

  Usage examples:

  .. code-block:: python

    import rawtoaces
    converter = rawtoaces.ImageConverter()
    converter.process_image("/path/to/input/file.CR3")

  This will convert the input file "/path/to/input/file.CR3" using the default settings, 
  and create the output file "/path/to/input/file_aces.exr" in the same directory.

  .. code-block:: python

    import rawtoaces
    converter = rawtoaces.ImageConverter()
    converter.settings.WB_method == rawtoaces.ImageConverter.Settings.WBMethod.Illuminant
    converter.settings.illuminant = "3200K"
    converter.settings.output_dir = "/path/to/output"
    converter.process_image("/path/to/an/image/file.CR3")
    
  This will convert the input file "/path/to/input/file.CR3" white-balancing to the colour
  temperature of 3200K, and create the output file "/path/to/output/file_aces.exr" at the provided location.

  .. py:class:: Settings

    The Settings class which corresponds to the C++ `rta::util::ImageConverter::Settings`.

    .. py:class:: WBMethod

      The WBMethod enum which corresponds to the C++ `rta::util::ImageConverter::Settings::WBMethod` and contains the following values::
          
        Metadata Illuminant Box Custom
    
    .. py:attribute:: WBMethod WB_method

    .. py:class:: MatrixMethod

      The MatrixMethod enum which corresponds to the C++ `rta::util::ImageConverter::Settings::MatrixMethod` and contains the following values::
          
        Auto Spectral Metadata Adobe Custom

    .. py:attribute:: MatrixMethod matrix_method

    .. py:class:: CropMode

      The CropMode enum which corresponds to the C++ `rta::util::ImageConverter::Settings::CropMode` and contains the following values::
          
        Off Soft Hard

    .. py:attribute:: CropMode crop_mode

    .. py:attribute:: str illuminant

    .. py:attribute:: float headroom

    .. py:attribute:: str custom_camera_make

    .. py:attribute:: str custom_camera_model

    .. py:attribute:: bool auto_bright

    .. py:attribute:: bool adjust_maximum_threshold

    .. py:attribute:: float black_level

    .. py:attribute:: float saturation_level

    .. py:attribute:: bool half_size

    .. py:attribute:: int highlight_mode

    .. py:attribute:: int flip

    .. py:attribute:: float denoise_threshold

    .. py:attribute:: float scale
    
    .. py:attribute:: str demosaic_algorithm

    .. py:attribute:: list[str] database_directories

    .. py:attribute:: bool overwrite

    .. py:attribute:: bool create_dirs

    .. py:attribute:: str output_dir

    .. py:attribute:: bool use_timing

    .. py:attribute:: int verbosity

    .. py:attribute:: list[int] WB_box

    .. py:attribute:: list[int] crop_box

    .. py:attribute:: list[float] custom_WB

    .. py:attribute:: list[float] custom_matrix
    
    .. py:attribute:: list[float] chromatic_aberration
    
  .. py:attribute:: Settings settings

  .. py:method:: bool process_image(str)

  .. py:method:: list[str] get_supported_illuminants()

  .. py:method:: list[str] get_supported_cameras()

  .. py:method:: bool configure(str)

  .. py:method:: list[float] get_WB_multipliers()

  .. py:method:: list[float] get_IDT_matrix()

  .. py:method:: list[float] get_CAT_matrix()

.. py:function:: list[list[str]] collect_image_files(list[str])
