#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the rawtoaces Project.

"""
Unit tests for rawtoaces Python bindings - ImageConverter class
"""

import pytest
import array

# The PYTHONPATH should be set by CMake to find the compiled module
# No need to manually add paths here as CMake handles this via environment variables

try:
    import rawtoaces
except ImportError as e:
    pytest.skip(f"rawtoaces module not found. Build the Python bindings first: {e}", allow_module_level=True)


class TestImageConverter:
    """Test cases for the ImageConverter class"""

    def test_converter_has_helper_functions(self):
        """Test that ImageConverter has helper functions"""
        assert hasattr(rawtoaces, "collect_image_files")
        assert callable(rawtoaces.collect_image_files)

    def test_collect_image_files(self):
        """Test that collect_image_files returns some images"""
        import os
        dir_path = os.path.join('.', 'tests', 'materials')
        dir_path = os.path.abspath(dir_path)
        # print(path)
        batches = rawtoaces.collect_image_files([dir_path])
        assert len(batches) == 2
        assert len(batches[0]) == 0
        files = batches[1]
        files.sort()
        assert len(files) == 2
        assert files[0] == os.path.join(dir_path, 'BatteryPark.NEF')
        assert files[1] == os.path.join(dir_path, 'blackmagic_cinema_camera_cinemadng.dng')
        
    def test_converter_creation(self):
        """Test that ImageConverter can be instantiated"""
        converter = rawtoaces.ImageConverter()
        assert converter is not None
        assert isinstance(converter, rawtoaces.ImageConverter)

    def test_converter_has_settings(self):
        """Test that ImageConverter has a settings attribute"""
        converter = rawtoaces.ImageConverter()
        assert hasattr(converter, "settings")
        assert converter.settings is not None

    def test_converter_has_methods(self):
        """Test that ImageConverter has all methods"""
        converter = rawtoaces.ImageConverter()
        
        assert hasattr(converter, "process_image")
        assert callable(converter.process_image)
        
        assert hasattr(converter, "configure")
        assert callable(converter.configure)
                
        assert hasattr(converter, "get_WB_multipliers")
        assert callable(converter.get_WB_multipliers)
        
        assert hasattr(converter, "get_IDT_matrix")
        assert callable(converter.get_IDT_matrix)
                
        assert hasattr(converter, "get_CAT_matrix")
        assert callable(converter.get_CAT_matrix)
                        
        assert hasattr(converter, "get_supported_illuminants")
        assert callable(converter.get_supported_illuminants)

        assert hasattr(converter, "get_supported_cameras")
        assert callable(converter.get_supported_cameras)
        

    def test_process_image_with_invalid_path(self):
        """Test process_image with non-existent file returns False"""
        import os
        converter = rawtoaces.ImageConverter()

        # Use a simple invalid filename (no path separators to avoid OS-specific issues)
        invalid_path = "nonexistent_file.txt"

        # Test with invalid path
        try:
            result = converter.process_image(invalid_path)
            assert result is False
        except Exception:
            # If an exception is thrown, that's also acceptable behavior for invalid input
            pass

        # Test with empty path
        try:
            result = converter.process_image("")
            assert result is False
        except Exception:
            # If an exception is thrown, that's also acceptable behavior for invalid input
            pass

    def test_configure_metadata(self):
        """Test configure() calculates in the WB multipliers and colour transform matrices"""

        import os
        path = os.path.join('.', 'tests', 'materials', 'blackmagic_cinema_camera_cinemadng.dng')
        converter = rawtoaces.ImageConverter()
        converter.settings.overwrite = True
        assert converter.configure(path)
        wb = converter.get_WB_multipliers()
        assert len(wb) == 4
        assert abs(wb[0] - 1.58982515335083) < 0.0001
        assert abs(wb[1] - 1) < 0.0001
        assert abs(wb[2] - 1.2651821374893188) < 0.0001
        assert abs(wb[3] - 0) < 0.0001

        idt = converter.get_IDT_matrix()
        cat = converter.get_CAT_matrix()

        # Older versions of OIIO fail to fetch the DNG metadata, so rawtoaces
        # picks the non-DNG path. Ideally we should restrict the supported
        # versions of OIIO, but since that issue is unrelated to the python
        # bindings, we'll just check that either of the two paths was
        # successful.
        if len(idt) == 3 and len(cat) == 0:
            assert len(idt[0]) == 3
            assert abs(idt[0][0] - 1.0536466144250152) < 0.0001
            assert abs(idt[0][1] - 0.00390441818863832) < 0.0001
            assert abs(idt[0][2] - 0.004908450238340354) < 0.0001
            assert len(idt[1]) == 3
            assert abs(idt[1][0] - -0.48995621645381615) < 0.0001
            assert abs(idt[1][1] - 1.3614787985962031) < 0.0001
            assert abs(idt[1][2] - 0.10208447284831194) < 0.0001
            assert len(idt[2]) == 3
            assert abs(idt[2][0] - -0.0024498461419844484) < 0.0001
            assert abs(idt[2][1] - 0.006049712791275535) < 0.0001
            assert abs(idt[2][2] - 1.013915953697747) < 0.0001
        elif len(idt) == 0 and len(cat) == 3:
            assert len(cat[0]) == 3
            assert abs(cat[0][0] - 1.0097583639200136) < 0.0001
            assert abs(cat[0][1] - 0.0050178093846550455) < 0.0001
            assert abs(cat[0][2] - -0.01505838909238814) < 0.0001
            assert len(cat[1]) == 3
            assert abs(cat[1][0] - 0.0036602813378778347) < 0.0001
            assert abs(cat[1][1] - 1.0030138169214682) < 0.0001
            assert abs(cat[1][2] - -0.005980232945639982) < 0.0001
            assert len(cat[2]) == 3
            assert abs(cat[2][0] - -0.00029980928869024906) < 0.0001
            assert abs(cat[2][1] - -0.0010516909063249997) < 0.0001
            assert abs(cat[2][2] - 0.9282027962747658) < 0.0001
        else:
            pytest.fail("Either IDT or CAT matrix must not be empty")

    def test_process_DNG(self):
        """Test process_image() returns True for a valid DNG file"""
        import os
        path = os.path.join('.', 'tests', 'materials', 'blackmagic_cinema_camera_cinemadng.dng')
        converter = rawtoaces.ImageConverter()
        converter.settings.overwrite = True
        assert converter.process_image(path)

    def test_converter_get_WB_multipliers(self):
        """Test uninitialised ImageConverter returns empty WB multipliers"""
        import os
        converter = rawtoaces.ImageConverter()
        try:
            wb = converter.get_WB_multipliers()
            assert len(wb) == 0
        except Exception as e:
            pytest.fail(f"Unexpected exception: {e}")

    def test_converter_get_IDT_matrix(self):
        """Test uninitialised ImageConverter returns empty IDT matrix"""
        import os
        converter = rawtoaces.ImageConverter()
        try:
            idt = converter.get_IDT_matrix()
            assert len(idt) == 0
        except Exception as e:
            pytest.fail(f"Unexpected exception: {e}")

    def test_converter_get_CAT_matrix(self):
        """Test uninitialised ImageConverter returns empty CAT matrix"""
        import os
        converter = rawtoaces.ImageConverter()
        try:
            cat = converter.get_CAT_matrix()
            assert len(cat) == 0
        except Exception as e:
            pytest.fail(f"Unexpected exception: {e}")

    def test_converter_get_supported_illuminants(self):
        """Test uninitialised ImageConverter returns built-in illuminants"""
        import os
        converter = rawtoaces.ImageConverter()
        try:
            illuminants = converter.get_supported_illuminants()
            assert len(illuminants) == 2
            assert illuminants[0] == 'Day-light (e.g., D60, D6025)'
            assert illuminants[1] == 'Blackbody (e.g., 3200K)'
        except Exception as e:
            pytest.fail(f"Unexpected exception: {e}")
        
    def test_converter_get_supported_cameras(self):
        """Test uninitialised ImageConverter returns empty camera list"""
        import os
        converter = rawtoaces.ImageConverter()
        try:
            cameras = converter.get_supported_cameras()
            assert len(cameras) == 0
        except Exception as e:
            pytest.fail(f"Unexpected exception: {e}")

class TestSettings:
    """Test cases for the ImageConverter.Settings class"""

    def test_settings_creation(self):
        """Test that Settings can be instantiated"""
        converter = rawtoaces.ImageConverter()
        settings = converter.settings
        assert settings is not None
        assert isinstance(settings, rawtoaces.ImageConverter.Settings)

    def test_settings_direct_creation(self):
        """Test that Settings can be created directly"""
        settings = rawtoaces.ImageConverter.Settings()
        assert settings is not None
        assert isinstance(settings, rawtoaces.ImageConverter.Settings)
    
        
    def test_settings_attributes(self):
        """Test that Settings has all attributes"""
        converter = rawtoaces.ImageConverter()
        
        converter.settings.WB_method = rawtoaces.ImageConverter.Settings.WBMethod.Metadata
        assert converter.settings.WB_method == rawtoaces.ImageConverter.Settings.WBMethod.Metadata
        
        converter.settings.WB_method = rawtoaces.ImageConverter.Settings.WBMethod.Illuminant
        assert converter.settings.WB_method == rawtoaces.ImageConverter.Settings.WBMethod.Illuminant
        
        converter.settings.WB_method = rawtoaces.ImageConverter.Settings.WBMethod.Box
        assert converter.settings.WB_method == rawtoaces.ImageConverter.Settings.WBMethod.Box
        
        converter.settings.WB_method = rawtoaces.ImageConverter.Settings.WBMethod.Custom
        assert converter.settings.WB_method == rawtoaces.ImageConverter.Settings.WBMethod.Custom
        
        converter.settings.matrix_method = rawtoaces.ImageConverter.Settings.MatrixMethod.Auto
        assert converter.settings.matrix_method == rawtoaces.ImageConverter.Settings.MatrixMethod.Auto
        
        converter.settings.matrix_method = rawtoaces.ImageConverter.Settings.MatrixMethod.Spectral
        assert converter.settings.matrix_method == rawtoaces.ImageConverter.Settings.MatrixMethod.Spectral
        
        converter.settings.matrix_method = rawtoaces.ImageConverter.Settings.MatrixMethod.Metadata
        assert converter.settings.matrix_method == rawtoaces.ImageConverter.Settings.MatrixMethod.Metadata
        
        converter.settings.matrix_method = rawtoaces.ImageConverter.Settings.MatrixMethod.Adobe
        assert converter.settings.matrix_method == rawtoaces.ImageConverter.Settings.MatrixMethod.Adobe
        
        converter.settings.matrix_method = rawtoaces.ImageConverter.Settings.MatrixMethod.Custom
        assert converter.settings.matrix_method == rawtoaces.ImageConverter.Settings.MatrixMethod.Custom
        
        converter.settings.crop_mode = rawtoaces.ImageConverter.Settings.CropMode.Off
        assert converter.settings.crop_mode == rawtoaces.ImageConverter.Settings.CropMode.Off
        
        converter.settings.crop_mode = rawtoaces.ImageConverter.Settings.CropMode.Soft
        assert converter.settings.crop_mode == rawtoaces.ImageConverter.Settings.CropMode.Soft
        
        converter.settings.crop_mode = rawtoaces.ImageConverter.Settings.CropMode.Hard
        assert converter.settings.crop_mode == rawtoaces.ImageConverter.Settings.CropMode.Hard
        
        converter.settings.illuminant = "illuminant"
        assert converter.settings.illuminant == "illuminant"
                
        converter.settings.headroom = 1.5
        assert converter.settings.headroom == 1.5
                
        converter.settings.custom_camera_make = "custom_camera_make"
        assert converter.settings.custom_camera_make == "custom_camera_make"
        
        converter.settings.custom_camera_model = "custom_camera_model"
        assert converter.settings.custom_camera_model == "custom_camera_model"
                
        converter.settings.auto_bright = True
        assert converter.settings.auto_bright == True
                        
        converter.settings.adjust_maximum_threshold = True
        assert converter.settings.adjust_maximum_threshold == True
                        
        converter.settings.black_level = 123
        assert converter.settings.black_level == 123
                        
        converter.settings.saturation_level = 1234
        assert converter.settings.saturation_level == 1234
                        
        converter.settings.half_size = True
        assert converter.settings.half_size == True
                        
        converter.settings.highlight_mode = 2
        assert converter.settings.highlight_mode == 2
                        
        converter.settings.flip = 3
        assert converter.settings.flip == 3
                        
        converter.settings.denoise_threshold = 1.23
        assert abs(converter.settings.denoise_threshold - 1.23) < 0.001
                                
        converter.settings.scale = 2.34
        assert abs(converter.settings.scale - 2.34) < 0.001
                                        
        converter.settings.demosaic_algorithm = "AHD"
        assert converter.settings.demosaic_algorithm == "AHD"
                                        
        converter.settings.database_directories = ["dir1", "dir2"]
        assert converter.settings.database_directories == ["dir1", "dir2"]
                                        
        converter.settings.overwrite = True
        assert converter.settings.overwrite == True
                                        
        converter.settings.create_dirs = True
        assert converter.settings.create_dirs == True
                                        
        converter.settings.output_dir = "output_dir"
        assert converter.settings.output_dir == "output_dir"
                                        
        converter.settings.use_timing = True
        assert converter.settings.use_timing == True
                                                
        converter.settings.verbosity = 3
        assert converter.settings.verbosity == 3
                                                
        converter.settings.WB_box = [1, 2, 3, 4]
        assert converter.settings.WB_box == [1, 2, 3, 4]
                                                        
        converter.settings.crop_box = [5, 6, 7, 8]
        assert converter.settings.crop_box == [5, 6, 7, 8]
                                                        
        custom_WB = [2.0, 1.0, 1.5, 1.0]
        converter.settings.custom_WB = custom_WB
        for i in range(4):
            assert abs(converter.settings.custom_WB[i] - custom_WB[i]) < 0.001
        
        custom_matrix = [
            [1.0, 0.1, 0.2],
            [0.3, 1.1, 0.4],
            [0.5, 0.6, 1.2]
        ]
        converter.settings.custom_matrix = custom_matrix
        for i in range(3):
            for j in range(3):
                assert abs(converter.settings.custom_matrix[i][j] - custom_matrix[i][j]) < 0.001
        
        converter.settings.chromatic_aberration = [1.2, 2.3]
        assert abs(converter.settings.chromatic_aberration[0] - 1.2) < 0.001
        assert abs(converter.settings.chromatic_aberration[1] - 2.3) < 0.001
        
    def test_settings_invalid_size(self):
        """Test that Settings has all attributes"""
        converter = rawtoaces.ImageConverter()
        
        with pytest.raises(ValueError) as err:
            converter.settings.WB_box = [1, 2, 3]
            assert str(err.value) == "The array must contain 4 values."

        with pytest.raises(ValueError) as err:
            converter.settings.custom_WB = [1, 2, 3]
            assert str(err.value) == "The array must contain 4 values."
        
        with pytest.raises(ValueError) as err:
            converter.settings.crop_box = [1, 2, 3]
            assert str(err.value) == "The array must contain 4 values."
                    
        with pytest.raises(ValueError) as err:
            converter.settings.chromatic_aberration = [1, 2, 3]
            assert str(err.value) == "The array must contain 2 values."
  
        with pytest.raises(ValueError) as err:
            converter.settings.custom_matrix = [[1.0, 2.0, 3.0], [1.0, 2.0, 3.0]]
            assert str(err.value) == "The matrix must contain 3 rows."
              
        with pytest.raises(ValueError) as err:
            converter.settings.custom_matrix = [[1.0, 2.0, 3.0], [1.0, 2.0, 3.0], [1.0, 2.0]]
            assert str(err.value) == "Each row of the matrix must contain 3 elements."

if __name__ == "__main__":
    pytest.main([__file__])
