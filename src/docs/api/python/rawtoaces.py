from collections.abc import Sequence
import enum
from typing import overload


class SpectralData:
    """
    A data-class for storing spectral data, based on the file format used in
    [rawtoaces-data](https://github.com/AcademySoftwareFoundation/rawtoaces-data).
    """

    def __init__(self) -> None:
        """Default constructor"""

    @property
    def manufacturer(self) -> str:
        """Manufacturer of the device being tested."""

    @manufacturer.setter
    def manufacturer(self, arg: str, /) -> None: ...

    @property
    def model(self) -> str:
        """Model of the device being tested."""

    @model.setter
    def model(self, arg: str, /) -> None: ...

    @property
    def type(self) -> str:
        """Type of the spectral dataset."""

    @type.setter
    def type(self, arg: str, /) -> None: ...

    @property
    def units(self) -> str:
        """Unit or quantity of measurement for the spectral dataset."""

    @units.setter
    def units(self, arg: str, /) -> None: ...

    def load(self, path: str, reshape: bool = True) -> bool:
        """
        Load spectral data from a given path

        :param path: path a path to the file to load data from
        :type path: str

        :param reshape: if set to ``True``, the data will be reshaped to the
            reference shape
        :type reshape: bool

        :return: ``True`` if loaded successfully.
        """

class Metadata:
    """DNG metadata required to calculate an input transform."""

    def __init__(self) -> None: ...

    class Calibration:
        """
        A calibration data set structure. Contains calibration matrices for
        a given light source.
        """

        def __init__(self) -> None: ...

        @property
        def illuminant(self) -> int:
            """EXIF light source tag."""

        @illuminant.setter
        def illuminant(self, arg: int, /) -> None: ...

        @property
        def camera_calibration_matrix(self) -> list[float]:
            """Camera calibration matrix."""

        @camera_calibration_matrix.setter
        def camera_calibration_matrix(self, arg: Sequence[float], /) -> None: ...

        @property
        def XYZ_to_RGB_matrix(self) -> list[float]:
            """XYZ to camera RGB colour transform matrix."""

        @XYZ_to_RGB_matrix.setter
        def XYZ_to_RGB_matrix(self, arg: Sequence[float], /) -> None: ...

    @property
    def neutral_RGB(self) -> list[float]:
        """
        Neutral RGB values. The colour channels in the camera RGB colour space
        representing a neutral colour.
        """

    @neutral_RGB.setter
    def neutral_RGB(self, arg: Sequence[float], /) -> None: ...

    @property
    def baseline_exposure(self) -> float:
        """Exposure calibration multiplier."""

    @baseline_exposure.setter
    def baseline_exposure(self, arg: float, /) -> None: ...

    @property
    def calibration(self) -> list[Metadata.Calibration]:
        """Calibration datasets. Currently two sets are supported."""

    @calibration.setter
    def calibration(self, arg: Sequence[Metadata.Calibration], /) -> None: ...

class TransformSolver:
    """Base class for a colour space transform solver."""

    def calculate_transform(self) -> bool:
        """
        Calculate the transform matrix. The solved matrix can be accessed via
        ``transform_matrix``.

        :return: ``True`` if calculated successfully.
        """

    @property
    def transform_matrix(self) -> list[list[float]]:
        """
        A 3×3 colour transform matrix populated by `calculate_transform()`
        (starts as identity from the base `TransformSolver` constructor).
        """

    @transform_matrix.setter
    def transform_matrix(self, arg: Sequence[Sequence[float]], /) -> None: ...

    @property
    def last_error_message(self) -> str:
        """
        Error message from the most recent method call that returned ``False``.
        """

    @last_error_message.setter
    def last_error_message(self, arg: str, /) -> None: ...

    @property
    def verbosity(self) -> int:
        """Verbosity level."""

    @verbosity.setter
    def verbosity(self, arg: int, /) -> None: ...

class MetadataSolver(TransformSolver):
    """Solve an input transform using the metadata stored in DNG files."""

    def __init__(self, metadata: Metadata) -> None:
        """Default constructor."""

    def calculate_CAT_matrix(self) -> list[list[float]]:
        """
        Calculate the Color Adaptation Transform (CAT) matrix for color space
        conversion. This function computes the CAT matrix needed to transform
        colors from the camera's white point to the target ACES RGB white point.
        It first obtains the camera's XYZ transformation matrix and white point,
        then creates the target ACES RGB to XYZ matrix, and finally calculates
        the color adaptation transform between the two white points using the
        Bradford or CAT02 method.

        The CAT matrix is essential for maintaining color appearance when
        converting between different illuminant conditions, ensuring that colors
        look consistent across different lighting environments. Strictly
        speaking, this matrix is not required for image processing, as it is
        embedded in the IDT; see `calculate_transform()` or deprecated
        `calculate_IDT_matrix()`.

        :return: 3×3 Color Adaptation Transform matrix

        .. deprecated:: 2.2.0
           will be removed in v3. Use ``calculate_transform()``.
        """

    def calculate_IDT_matrix(self) -> list[list[float]]:
        """
        Calculate the Input Device Transform (IDT) matrix for DNG color space
        conversion. This function computes the final IDT matrix that transforms
        camera RGB values to ACES RGB color space. It combines the Color
        Adaptation Transform (CAT) matrix with the D65 ACES RGB to XYZ
        transformation matrix to create a complete camera-to-ACES transformation
        pipeline.

        :return: 3×3 Input Device Transform matrix for DNG to ACES conversion
        :note: CAT is computed internally; ``calculate_CAT_matrix()`` does not 
               need to be called first.

        .. deprecated:: 2.2.0
           will be removed in v3. Use ``calculate_transform()``.
        """

class SpectralSolver(TransformSolver):
    """
    Solve an input transform using spectral sensitivity curves of a camera.
    """

    def __init__(self, search_directories: Sequence[str] = []) -> None:
        """
        Initialize SpectralSolver with database search path.
        Sets white balance multipliers to neutral (1, 1, 1) and relies on the
        base `TransformSolver` constructor for an identity `transform_matrix` and
        zero verbosity. Takes the database search path as an optional parameter
        for finding spectral data files.

        :param search_directories: optional database search path for spectral
            data files
        """

    def collect_data_files(self, type: str) -> list[str]:
        """
        A helper method collecting spectral data files of a given type from the
        database. This function searches through the configured search
        directories to find all spectral data files matching the specified type
        (e.g., ``camera``, ``illuminant``). It searches for type subdirectories at
        the top level of each directory and returns JSON files matching the
        type.

        :param type: data type of the files to search for (e.g., ``camera``,
            ``illuminant``, ``cmf``)
        :type type: str
        :return: a collection of file paths found in the database
        """

    def find_camera(self, make: str, model: str) -> bool:
        """
        Load spectral sensitivity data for a camera by searching the database.
        This function searches through camera data files in the database to find
        a match for the specified camera manufacturer and model. It loads the
        spectral sensitivity data into the camera member variable.

        :param make: the camera make to search for
        :type make: str
        :param model: the camera model to search for
        :type model: str
        :return: ``True`` if loaded successfully, ``False`` otherwise
        """

    @overload
    def find_illuminant(self, type: str) -> bool:
        """
        Find spectral power distribution data of an illuminant of the given
        type. This function can handle both built-in illuminant types (e.g.,
        ``d55``, ``3200k``) and custom illuminants stored in the database. For
        built-in types, it generates the spectral data using standard formulas.

        :param type: illuminant type. Can be one of the built-in types, e.g. 
           ``d55``, ``3200k``, or a custom illuminant stored in the  database.
        :type type: str

        :return: ``True`` if loaded successfully, ``False`` otherwise
        """

    @overload
    def find_illuminant(self, wb_multipliers: Sequence[float]) -> bool:
        """
        Find the illuminant best matching the given white-balancing multipliers.
        This function analyzes all available illuminants and selects the one
        that best matches the white balance coefficients. It uses Sum of Squared
        Errors (SSE) to find the optimal match and automatically scales the
        white balance multipliers.

        :param wb_multipliers: white-balancing multipliers to match
        :type wb_multipliers: list[float]

        :return: ``True`` if loaded successfully, ``False`` otherwise
        """

    def calculate_WB(self) -> bool:
        """
        Calculate the white-balance multipliers for the given configuration.
        This function computes RGB white balance multipliers by integrating
        camera spectral sensitivity with illuminant power spectrum. The
        multipliers normalize the camera response to achieve proper white
        balance under the specified illuminant conditions.
        The `camera` and `illuminant` data have to be configured prior to this
        call.

        :return: ``True`` if calculated successfully, ``False`` otherwise
        :pre: camera and illuminant data must be properly loaded
        """

    def load_spectral_data(self, file_path: str, out_data: SpectralData) -> bool:
        """
        A helper method loading the spectral data for a file at the given path.
        This function loads spectral data from a file, handling both absolute
        and relative paths. For relative paths, it searches through all
        configured search directories.

        :param file_path: the path to the file to load. If the path is relative,
           all locations in the search path will be searched in.
        :type file_path: str
        :param out_data: the `SpectralData` object to be filled with the loaded
           data.
        :type out_data: rawtoaces.SpectralData
        :return: ``True`` if loaded successfully, ``False`` otherwise
        """

    def get_WB_multipliers(self) -> list[float]:
        """
        Get the white-balance multipliers calculated using ``find_illuminant()``
        or ``calculate_WB()``. This function returns a reference to the 
        3-element vector containing RGB white balance multipliers. These 
        multipliers scale the camera response to achieve proper white balance 
        under the specified illuminant conditions.

        :return: a 3-element white balance multiplier list [R, G, B]
        :pre: white balance calculation must have been performed successfully
        """

    @property
    def camera(self) -> SpectralData:
        """
        The camera spectral data. Can be either assigned directly, loaded
        in place via ``solver.camera.load()``, or found via
        ``solver.find_camera()``.
        """

    @camera.setter
    def camera(self, arg: SpectralData, /) -> None: ...

    @property
    def illuminant(self) -> SpectralData:
        """
        The illuminant spectral data. Can be either assigned directly, loaded
        in place via ``solver.illuminant.load()``, or found via
        ``solver.find_illuminant()``.
        """

    @illuminant.setter
    def illuminant(self, arg: SpectralData, /) -> None: ...

    @property
    def observer(self) -> SpectralData:
        """
        The observer spectral data. Can be either assigned directly, or loaded
        in place via ``solver.observer.load()``.
        """

    @observer.setter
    def observer(self, arg: SpectralData, /) -> None: ...

    @property
    def training_data(self) -> SpectralData:
        """
        The training set spectral data. Can be either assigned directly, or loaded
        in place via ``solver.training_data.load()``.
        """

    @training_data.setter
    def training_data(self, arg: SpectralData, /) -> None: ...

    def calculate_IDT_matrix(self) -> bool:
        """
        Calculate an input transform matrix using curve fitting optimization.
        This function computes the optimal IDT matrix by comparing camera RGB
        responses with target XYZ values across all training patches.
        The ``camera``, ``illuminant``, ``observer`` and ``training_data`` have
        to be configured prior to this call.

        :return: ``True`` if calculated successfully, ``False`` otherwise
        :pre: camera, illuminant, observer, and training_data must be properly
            loaded
        """

    def get_IDT_matrix(self) -> list[list[float]]:
        """
        Get the matrix from the last successful spectral solve (same data as
        ``transform_matrix``).
        This function returns a reference to the 3×3 IDT matrix that transforms
        camera RGB values to standardized color space. The matrix is computed by
        curve fitting optimization and represents the optimal color
        transformation for the camera under the specified illuminant conditions.

        :return: a 3×3 IDT transformation matrix
        :pre: ``calculate_transform()`` or deprecated ``calculate_IDT_matrix()``
            completed successfully
        """

def collect_image_files(path: Sequence[str]) -> list[list[str]]:
    """
    Collect all files from given `paths` into batches.
    For each path that is a directory, a batch is created in the returned 
    vector and filled with the file names. Invalid paths are skipped with
    an error message.

    First batch is reserved for all paths that are individual files. If no
    such paths are provided, the first batch will be empty.

    :param path: paths vector of paths to files or directories to process.
    :type path: list[str]

    :return: list of batches, where each batch contains files from one input 
        path.
    """

class ImageConverter:
    """
    The ImageConverter class is the main interface for converting RAW images
    to ACES format. It provides a high-level API that handles the entire
    conversion pipeline.
    """

    def __init__(self) -> None:
        """Default constructor."""

    @property
    def settings(self) -> ImageConverter.Settings:
        """The conversion settings."""

    @settings.setter
    def settings(self, arg: ImageConverter.Settings, /) -> None: ...

    @property
    def status(self) -> ImageConverter.Status:
        """
        This property holds the error code from the most recent method call 
        that returns a bool.
        """

    @property
    def last_error_message(self) -> str:
        """
        Error message from the most recent method call that returned ``False``.
        """

    @last_error_message.setter
    def last_error_message(self, arg: str, /) -> None: ...

    def process_image(self, input_filename: str) -> bool:
        """
        A convenience single-call method to process an image.

        :param input_filename: Full path to the file to be converted.
        :type input_filename: str

        :return: ``True`` if processed successfully.
        """

    def get_WB_multipliers(self) -> list[float]:
        """
        Get the solved white balance multipliers of the currently processed
        image. The multipliers become available after calling the ``configure``
        method.

        :return: a list containing 3 multiplier values.
        """

    def get_transform_matrix(self) -> list[list[float]]:
        """
        Get the solved colour transform matrix to be applied by 
        ``process_image``.
        The matrix becomes available after calling the ``configure`` method.

        :return: a list containing a 3x3 matrix.
        """

    def get_IDT_matrix(self) -> list[list[float]]:
        """
        Get the solved input transform matrix of the currently processed image.
        The matrix becomes available after calling the ``configure`` method.

        :return: a list containing a 3x3 matrix.

        .. deprecated:: 2.2.0
            will be removed in v3. Use ``get_transform_matrix()``.
        """

    def get_CAT_matrix(self) -> list[list[float]]:
        """
        Get the solved chromatic adaptation transform matrix of the currently
        processed image. The matrix becomes available after calling the 
        ``configure`` method.

        :return: a list containing a 3x3 matrix.

        .. deprecated:: 2.2.0
            will be removed in v3. Use ``get_transform_matrix()``.
        """

    def configure(self, input_filename: str) -> bool:
        """
        Configures the converter using the requested white balance and colour
        matrix method, and the metadata of the file provided in 
        ``input_filename``.

        This method loads the metadata from the given image file and
        initialises the options to give the OIIO raw image reader to
        decode the pixels.

        :param input_filename: A file name of the raw image file to read the 
            metadata from.
        :type input_filename: str
        :return: ``True`` if configured successfully.
        """

    def get_supported_formats(self) -> list[str]:
        """
        Collects all camera raw formats supported by this version.

        :return: List of all supported image formats.
        """

    def get_supported_illuminants(self) -> list[str]:
        """
        Collects all illuminants supported by this version.

        :return: List of all supported illuminants.
        """

    def get_supported_cameras(self) -> list[str]:
        """
        Collects all camera models for which spectral sensitivity data is
        available in the database.

        :return: List containing camera model names.
        """

    class Settings:
        """
        The structure containing all parameters needed to configure image
        conversion.
        """

        def __init__(self) -> None: ...
        
        class LensCorrectionType(enum.Flag):
            """The enumerator containing all supported lens correction types."""

            Aberration = 1
            """Chromatic aberration"""

            Distortion = 2
            """Geeometric distortion"""

            Vignetting = 4
            """Vignetting"""
            
        class MatrixMethod(enum.Enum):
            """
            The enumerator containing all supported colour transform matrix 
            calculation methods.
            """

            Auto = 0
            """
            Automatically choose the best available matrix method.

            - If spectral sensitivity data for the camera is available, use ``Spectral``.
            - Otherwise, fall back to ``Metadata``.
            """

            Spectral = 1
            """
            Use the camera spectral sensitivity curves to solve for the colour
            conversion matrix. In this mode the illuminant is either provided
            directly in ``illuminant`` if ``WB_method`` == 
            ``WBMethod::Illuminant``, or the best illuminant is derived from the
            white balancing multipliers.
            """

            Metadata = 2
            """
            Use the metadata provided in the image file. This mode is mostly
            usable with DNG files, as the information needed for conversion
            is mandatory in the DNG format.
            """

            Adobe = 3
            """Use the Adobe colour matrix for the camera supplied in LibRaw."""

            Custom = 4
            """
            Specify a custom matrix in `colourMatrix`. This mode is useful if
            the matrix is calculated by an external tool.
            """
            
        class WBMethod(enum.Enum):
            """The enumerator containing all supported white-balancing methods."""

            Metadata = 0
            """
            Use the metadata provided in the image file. This mode is mostly
            usable with DNG files, as the information needed for conversion
            is mandatory in the DNG format.
            """

            Illuminant = 1
            """
            White balance to a specified illuminant. See the ``illuminant``
            property for more information on the supported illuminants. This
            mode can only be used if spectral sensitivities are available for
            the camera.
            """

            Box = 2
            """
            Calculate white balance by averaging over a specified region of
            the image. See ``WB_box``. In this mode if an empty box is provided,
            white balancing is done by averaging over the whole image.
            """

            Custom = 3
            """
            Use custom white balancing multipliers. This mode is useful if
            the white balancing coefficients are calculated by an external tool.
            """

        @property
        def WB_method(self) -> ImageConverter.Settings.WBMethod:
            """The selected white-balancing method to use for conversion."""

        @WB_method.setter
        def WB_method(self, arg: ImageConverter.Settings.WBMethod, /) -> None: ...

        @property
        def matrix_method(self) -> ImageConverter.Settings.MatrixMethod:
            """
            The selected colour transform matrix calculation method to use for
            conversion.
            """

        @matrix_method.setter
        def matrix_method(self, arg: ImageConverter.Settings.MatrixMethod, /) -> None: ...

        @property
        def crop_mode(self) -> ImageConverter.Settings.CropMode:
            """The selected cropping mode to use for conversion."""

        @crop_mode.setter
        def crop_mode(self, arg: ImageConverter.Settings.CropMode, /) -> None: ...

        @property
        def illuminant(self) -> str:
            """
            An illuminant to use for white balancing and/or colour matrix
            calculation. Only used when ``WB_method`` == ``WBMethod::Illuminant``
            and ``matrix_method`` == ``MatrixMethod::Spectral``.

            An illuminant can be provided as a black body correlated colour
            temperature, like ``3200K``; or a D-series illuminant, like ``D56``;
            or any other illuminant, in such case it must be present in the data
            folder.
            """

        @illuminant.setter
        def illuminant(self, arg: str, /) -> None: ...

        @property
        def headroom(self) -> float:
            """Highlight headroom factor."""

        @headroom.setter
        def headroom(self, arg: float, /) -> None: ...

        @property
        def custom_camera_make(self) -> str:
            """
            Camera manufacturer name to be used for spectral sensitivity
            curves lookup.
            """

        @custom_camera_make.setter
        def custom_camera_make(self, arg: str, /) -> None: ...

        @property
        def custom_camera_model(self) -> str:
            """Camera model name to be used for spectral sensitivity curves lookup."""

        @custom_camera_model.setter
        def custom_camera_model(self, arg: str, /) -> None: ...

        @property
        def auto_bright(self) -> bool:
            """Enable automatic exposure adjustment."""

        @auto_bright.setter
        def auto_bright(self, arg: bool, /) -> None: ...

        @property
        def adjust_maximum_threshold(self) -> float:
            """
            Automatically lower the linearity threshold provided in the metadata by
            this scaling factor.
            """

        @adjust_maximum_threshold.setter
        def adjust_maximum_threshold(self, arg: float, /) -> None: ...

        @property
        def black_level(self) -> int:
            """If >= 0, override the black level specified in the file metadata."""

        @black_level.setter
        def black_level(self, arg: int, /) -> None: ...

        @property
        def saturation_level(self) -> int:
            """If >= 0, override the saturation level specified in the file metadata."""

        @saturation_level.setter
        def saturation_level(self, arg: int, /) -> None: ...

        @property
        def half_size(self) -> bool:
            """Decode the image at half size resolution."""

        @half_size.setter
        def half_size(self, arg: bool, /) -> None: ...

        @property
        def highlight_mode(self) -> int:
            """
            Highlight recovery mode, as supported by OpenImageIO/Libraw: 
            0 = clip, 1 = unclip, 2 = blend, 3..9 = rebuild.
            """

        @highlight_mode.setter
        def highlight_mode(self, arg: int, /) -> None: ...

        @property
        def flip(self) -> int:
            """
            If not -1, override the orientation specified in the metadata.
            1..8 correspond to EXIF orientation codes
            (0 = none, 3 = 180 deg, 6 = 90 deg CCW, 8 = 90 deg CW.)
            """

        @flip.setter
        def flip(self, arg: int, /) -> None: ...

        @property
        def denoise_threshold(self) -> float:
            """Wavelet denoising threshold."""

        @denoise_threshold.setter
        def denoise_threshold(self, arg: float, /) -> None: ...

        @property
        def scale(self) -> float:
            """Additional scaling factor to apply to the pixel values."""

        @scale.setter
        def scale(self, arg: float, /) -> None: ...

        @property
        def demosaic_algorithm(self) -> str:
            """
            Demosaicing algorithm. 
            Supported options: ``linear``, ``VNG``, ``PPG``, ``AHD``, ``DCB``,
            ``DHT``, ``AAHD``.
            """

        @demosaic_algorithm.setter
        def demosaic_algorithm(self, arg: str, /) -> None: ...

        @property
        def bad_pixels_path(self) -> str:
            """
            A path to the file containing a list of bad bixels in libraw format:
            a plain text where each line describes a single bad pixel using
            three numbers separated by whitespace for the column, row and UNIX
            timestamp.
            """

        @bad_pixels_path.setter
        def bad_pixels_path(self, arg: str, /) -> None: ...

        @property
        def database_directories(self) -> list[str]:
            """
            Directory containing rawtoaces spectral sensitivity and illuminant
            data files. Overrides the default search path and the
            ``RAWTOACES_DATA_PATH`` environment variable.
            """

        @database_directories.setter
        def database_directories(self, arg: Sequence[str], /) -> None: ...

        @property
        def overwrite(self) -> bool:
            """Allows overwriting existing files."""

        @overwrite.setter
        def overwrite(self, arg: bool, /) -> None: ...

        @property
        def create_dirs(self) -> bool:
            """Create output directories if they don't exist."""

        @create_dirs.setter
        def create_dirs(self, arg: bool, /) -> None: ...

        @property
        def compression(self) -> str:
            """
            "Output file compression type. Supported options: none, rle, "
                "zip, zips, piz, pxr24, b44, b44a, dwaa, dwab, htj2k256, htj2k32. "
                "(default: none)\"
            """

        @compression.setter
        def compression(self, arg: str, /) -> None: ...

        @property
        def output_dir(self) -> str:
            """The directory to write the output files to."""

        @output_dir.setter
        def output_dir(self, arg: str, /) -> None: ...

        @property
        def use_timing(self) -> bool:
            """Log the execution time of each step of image processing."""

        @use_timing.setter
        def use_timing(self, arg: bool, /) -> None: ...

        @property
        def disable_cache(self) -> bool:
            """Disable caching."""

        @disable_cache.setter
        def disable_cache(self, arg: bool, /) -> None: ...

        @property
        def disable_exiftool(self) -> bool:
            """Disable calling exiftool to fetch missing metadata."""

        @disable_exiftool.setter
        def disable_exiftool(self, arg: bool, /) -> None: ...

        @property
        def verbosity(self) -> int:
            """Verbosity level."""

        @verbosity.setter
        def verbosity(self, arg: int, /) -> None: ...

        @property
        def lens_correction_types(self) -> ImageConverter.Settings.LensCorrectionType:
            """The selected lens correction types."""

        @lens_correction_types.setter
        def lens_correction_types(self, arg: ImageConverter.Settings.LensCorrectionType, /) -> None: ...

        @property
        def require_lens_correction(self) -> bool:
            """
            If true, treat lens correction as mandatory. The conversion will
            fail if any of the correction types above is requested,
            but rawtoaces is unable to obtain a suitable lens model.
            """

        @require_lens_correction.setter
        def require_lens_correction(self, arg: bool, /) -> None: ...

        @property
        def custom_lens_make(self) -> str:
            """Lens manufacturer name to be used for lens correction data lookup."""

        @custom_lens_make.setter
        def custom_lens_make(self, arg: str, /) -> None: ...

        @property
        def custom_lens_model(self) -> str:
            """Lens model name to be used for lens correction data lookup."""

        @custom_lens_model.setter
        def custom_lens_model(self, arg: str, /) -> None: ...

        @property
        def custom_aperture(self) -> float:
            """Aperture (f-number) to be user for lens correction."""

        @custom_aperture.setter
        def custom_aperture(self, arg: float, /) -> None: ...

        @property
        def custom_focal_length(self) -> float:
            """Focal length to be user for lens correction."""

        @custom_focal_length.setter
        def custom_focal_length(self, arg: float, /) -> None: ...

        @property
        def custom_focus_distance(self) -> float:
            """Focus distance to be user for lens correction."""

        @custom_focus_distance.setter
        def custom_focus_distance(self, arg: float, /) -> None: ...

        @property
        def WB_box(self) -> list[int]:
            """
            Box to use for white balancing when ``WB_method`` == ``WBMethod::Box``.
            (default = (0,0,0,0) - full image)
            """

        @WB_box.setter
        def WB_box(self, arg: Sequence[int], /) -> None: ...

        @property
        def custom_WB(self) -> list[float]:
            """
            Custom white balance multipliers to be used when
            ``WB_method`` == ``WBMethod::Custom``.
            """

        @custom_WB.setter
        def custom_WB(self, arg: Sequence[float], /) -> None: ...

        @property
        def custom_matrix(self) -> list[list[float]]:
            """
            Custom camera RGB to XYZ matrix to be used when
            ``matrix_method`` == ``MatrixMethod::Custom``.
            """

        @custom_matrix.setter
        def custom_matrix(self, arg: Sequence[Sequence[float]], /) -> None: ...

        @property
        def crop_box(self) -> list[int]:
            """
            Apply custom crop. If not specified (all values are zeroes),
            the default crop is applied, which should match the crop of the
            in-camera JPEG.
            """

        @crop_box.setter
        def crop_box(self, arg: Sequence[int], /) -> None: ...

        @property
        def chromatic_aberration(self) -> list[float]:
            """Red and blue scale factors for chromatic aberration correction."""

        @chromatic_aberration.setter
        def chromatic_aberration(self, arg: Sequence[float], /) -> None: ...

        

        Metadata: MatrixMethod = MatrixMethod.Metadata

        Illuminant: WBMethod = WBMethod.Illuminant

        Box: WBMethod = WBMethod.Box

        Custom: MatrixMethod = MatrixMethod.Custom

        Auto: MatrixMethod = MatrixMethod.Auto

        Spectral: MatrixMethod = MatrixMethod.Spectral

        Adobe: MatrixMethod = MatrixMethod.Adobe

        class CropMode(enum.Enum):
            """The enumerator containing all supported cropping modes."""

            Off = 0
            """Write out full sensor area."""

            Soft = 1
            """
            Write out full sensor area, mark the crop area as the display 
            window.
            """

            Hard = 2
            """Write out only the crop area."""

        Off: CropMode = CropMode.Off

        Soft: CropMode = CropMode.Soft

        Hard: CropMode = CropMode.Hard

        

        Aberration: LensCorrectionType = LensCorrectionType.Aberration

        Distortion: LensCorrectionType = LensCorrectionType.Distortion

        Vignetting: LensCorrectionType = LensCorrectionType.Vignetting

    class Status(enum.Enum):
        """Status codes for operation results."""

        Success = 0
        """"Operation completed successfully."""

        DatabaseNotFound = 14
        """Failed to locate spectral measurements database."""

        FileExists = 1
        """Output file already exists and overwrite is not enabled."""

        InputFileNotFound = 2
        """Input file does not exist."""

        EmptyInputFilename = 3
        """Empty input filename provided."""

        FilesystemError = 4
        """Filesystem error occurred."""

        OutputDirectoryError = 5
        """Output directory does not exist and cannot be created."""

        InvalidPath = 6
        """Invalid path format."""

        ConfigurationError = 7
        """Failed to configure the image reader."""

        ReadError = 8
        """Failed to read the image file."""

        LensCorrectionError = 9
        """Failed to apply lens correction."""

        MatrixApplicationError = 10
        """Failed to apply colour space conversion."""

        ScaleApplicationError = 11
        """Failed to apply scale."""

        CropApplicationError = 12
        """Failed to apply crop."""

        WriteError = 13
        """Failed to save the output file."""

        UnknownError = 15
        """Unknown error."""

    Success: Status = Status.Success

    DatabaseNotFound: Status = Status.DatabaseNotFound

    FileExists: Status = Status.FileExists

    InputFileNotFound: Status = Status.InputFileNotFound

    EmptyInputFilename: Status = Status.EmptyInputFilename

    FilesystemError: Status = Status.FilesystemError

    OutputDirectoryError: Status = Status.OutputDirectoryError

    InvalidPath: Status = Status.InvalidPath

    ConfigurationError: Status = Status.ConfigurationError

    ReadError: Status = Status.ReadError

    LensCorrectionError: Status = Status.LensCorrectionError

    MatrixApplicationError: Status = Status.MatrixApplicationError

    ScaleApplicationError: Status = Status.ScaleApplicationError

    CropApplicationError: Status = Status.CropApplicationError

    WriteError: Status = Status.WriteError

    UnknownError: Status = Status.UnknownError
