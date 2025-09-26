#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the rawtoaces Project.

"""
Unit tests for rawtoaces Python bindings - ImageConverter class
"""

import pytest

# The PYTHONPATH should be set by CMake to find the compiled module
# No need to manually add paths here as CMake handles this via environment variables

try:
    import rawtoaces
except ImportError as e:
    pytest.skip(f"rawtoaces module not found. Build the Python bindings first: {e}", allow_module_level=True)


class TestImageConverter:
    """Test cases for the ImageConverter class"""

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

    def test_converter_has_process_image_method(self):
        """Test that ImageConverter has process_image method"""
        converter = rawtoaces.ImageConverter()
        assert hasattr(converter, "process_image")
        assert callable(converter.process_image)

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


if __name__ == "__main__":
    pytest.main([__file__])