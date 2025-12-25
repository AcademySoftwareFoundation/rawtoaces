..
  Copyright Contributors to the rawtoaces Project.
  SPDX-License-Identifier: CC-BY-4.0


.. _sec-pythonimageconverter:

ImageConverter
==============

.. py:class:: ImageConverter

  The ImageConverter class which corresponds to the C++ class :cpp:class:`rta::util::ImageConverter`.
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

    The structure containing all parameters needed to configure image conversion.  
    This class corresponds to the C++  :cpp:class:`rta::util::ImageConverter::Settings`.

    .. py:class:: WBMethod

      The enumerator containing all supported white-balancing methods::
          
        Metadata Illuminant Box Custom

      See :cpp:enum:`rta::util::ImageConverter::Settings::WBMethod` for more information on each supported method.
    
    .. py:attribute:: WBMethod WB_method

      The selected white-balancing method to use for conversion.

    .. py:class:: MatrixMethod

      The enumerator containing all supported colour transform matrix calculation methods::
          
        Auto Spectral Metadata Adobe Custom

      See :cpp:enum:`rta::util::ImageConverter::Settings::MatrixMethod` for more information on each supported method.

    .. py:attribute:: MatrixMethod matrix_method

      The selected colour transform matrix calculation method to use for conversion.

    .. py:class:: CropMode

      The enumerator containing all supported crop modes::
          
        Off Soft Hard

      See :cpp:enum:`rta::util::ImageConverter::Settings::CropMode` for more information on each supported mode.

    .. py:attribute:: CropMode crop_mode

      The selected cropping mode to use for conversion.

    .. py:attribute:: str illuminant

      An illuminant to use for white balancing and/or colour matrix calculation.
      See :cpp:member:`rta::util::ImageConverter::Settings::illuminant` for more information.

    .. py:attribute:: float headroom

      Highlight headroom factor.

    .. py:attribute:: str custom_camera_make

      Camera manufacturer name to be used for spectral sensitivity curves lookup.

    .. py:attribute:: str custom_camera_model

      Camera model name to be used for spectral sensitivity curves lookup.

    .. py:attribute:: bool auto_bright

      Enable automatic exposure adjustment.

    .. py:attribute:: bool adjust_maximum_threshold

      Automatically lower the linearity threshold provided in the metadata by this scaling factor.

    .. py:attribute:: float black_level

      If non-negative, override the black level specified in the file metadata.

    .. py:attribute:: float saturation_level

      If non-negative, override the saturation level specified in the file metadata.

    .. py:attribute:: bool half_size

      Decode the image at half size resolution.

    .. py:attribute:: int highlight_mode

      Highlight recovery mode, as supported by OpenImageIO/Libraw.
      0 = clip, 1 = unclip, 2 = blend, 3..9 = rebuild.

    .. py:attribute:: int flip

      If not 0, override the orientation specified in the metadata.
      1..8 correspond to EXIF orientation codes
      (3 = 180 deg, 6 = 90 deg CCW, 8 = 90 deg CW.)

    .. py:attribute:: float denoise_threshold

      Wavelet denoising threshold.

    .. py:attribute:: float scale

      Additional scaling factor to apply to the pixel values.
    
    .. py:attribute:: str demosaic_algorithm

      Demosaicing algorithm. Supported options::
        
       linear VNG PPG AHD DCB AHD-Mod AFD VCD Mixed LMMSE AMaZE DHT AAHD AHD'

    .. py:attribute:: list[str] database_directories

      Directory containing rawtoaces spectral sensitivity and illuminant
      data files. Overrides the default search path and the
      RAWTOACES_DATA_PATH environment variable.

    .. py:attribute:: bool overwrite

      Allows overwriting existing files.

    .. py:attribute:: bool create_dirs

      Create output directories if they don't exist.

    .. py:attribute:: str output_dir

      The directory to write the output files to.

    .. py:attribute:: bool use_timing

      Log the execution time of each step of image processing.

    .. py:attribute:: int verbosity

      Verbosity level.

    .. py:attribute:: list[int] WB_box

      Box to use for white balancing when `WB_method` == `WBMethod::Box`,
      (default = (0,0,0,0) - full image).

    .. py:attribute:: list[int] crop_box

      Apply custom crop. If not specified (all values are zeroes),
      the default crop is applied, which should match the crop of the
      in-camera JPEG.

    .. py:attribute:: list[float] custom_WB

      Custom white balance multipliers to be used when 
      `WB_method` == `WBMethod::Custom`.

    .. py:attribute:: list[float] custom_matrix

      Custom camera RGB to XYZ matrix to be used when 
      `matrix_method` == `MatrixMethod::Custom`.
    
    .. py:attribute:: list[float] chromatic_aberration

      Red and blue scale factors for chromatic aberration correction.
    
  .. py:attribute:: Settings settings

    The conversion settings.

  .. py:method:: bool process_image(str)

    A convenience single-call method to process an image.
    See :cpp:func:`rta::util::ImageConverter::process_image` for more information.

  .. py:method:: list[str] get_supported_illuminants()

    Collects all illuminants supported by this version.

  .. py:method:: list[str] get_supported_cameras()

    Collects all camera models for which spectral sensitivity data is available in the database.

  .. py:method:: bool configure(str)

    Configures the converter using the requested white balance and colour matrix method, and the metadata of the file provided in `input_file`.
    See :cpp:func:`rta::util::ImageConverter::configure` for more information.

  .. py:method:: list[float] get_WB_multipliers()

    Get the solved white balance multipliers of the currently processed image.
    See :cpp:func:`rta::util::ImageConverter::get_WB_multipliers` for more information.

  .. py:method:: list[float] get_IDT_matrix()

    Get the solved input transform matrix of the currently processed image.
    See :cpp:func:`rta::util::ImageConverter::get_IDT_matrix` for more information.

  .. py:method:: list[float] get_CAT_matrix()

    Get the solved chromatic adaptation transform matrix of the currently processed image.
    See :cpp:func:`rta::util::ImageConverter::get_CAT_matrix` for more information.

.. py:function:: list[list[str]] collect_image_files(list[str])

  Collect all files from given `paths` into batches.
  See :cpp:func:`rta::util::collect_image_files` for more information.
