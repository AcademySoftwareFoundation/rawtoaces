#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the rawtoaces Project.

"""
Unit tests for rawtoaces Python bindings - core metadata solver bindings
"""

import json
import math
import pytest

try:
    import rawtoaces
except ImportError as e:
    pytest.skip(
        f"rawtoaces module not found. Build the Python bindings first: {e}",
        allow_module_level=True,
    )


def _init_reference_metadata():
    metadata = rawtoaces.Metadata()
    metadata.baseline_exposure = 2.4
    metadata.neutral_RGB = [0.6289999865031245, 1.0, 0.790400030452882]

    calibration_0 = rawtoaces.Metadata.Calibration()
    calibration_0.illuminant = 17
    calibration_0.XYZ_to_RGB_matrix = [
        1.3119699954986572,
        -0.49678999185562134,
        0.011559999547898769,
        -0.41723001003265381,
        1.4423700571060181,
        0.045279998332262039,
        0.067230001091957092,
        0.21709999442100525,
        0.72650998830795288,
    ]

    calibration_1 = rawtoaces.Metadata.Calibration()
    calibration_1.illuminant = 21
    calibration_1.XYZ_to_RGB_matrix = [
        1.0088499784469604,
        -0.27351000905036926,
        -0.082580000162124634,
        -0.48996999859809875,
        1.3444099426269531,
        0.11174000054597855,
        -0.064060002565383911,
        0.32997000217437744,
        0.5391700267791748,
    ]

    metadata.calibration = [calibration_0, calibration_1]
    return metadata


def _write_test_spectral_file(
    base_dir,
    data_type,
    filename,
    channels,
    header_data=None,
):
    target_dir = base_dir / data_type
    target_dir.mkdir(parents=True, exist_ok=True)

    header = header_data if header_data is not None else {}
    data_main = {}
    channel_count = len(channels)
    for wavelength in range(380, 781, 5):
        if data_type == "illuminant":
            power_val = 1.0 + (wavelength - 380) * 0.01
            values = [power_val for _ in range(channel_count)]
        else:
            v1 = 0.1 + (wavelength - 380) * 0.001
            v2 = 0.2 + (wavelength - 380) * 0.001
            v3 = 0.3 + (wavelength - 380) * 0.001
            values = []
            if channel_count > 0:
                values.append(v1)
            if channel_count > 1:
                values.append(v2)
            if channel_count > 2:
                values.append(v3)
            for _ in range(3, channel_count):
                values.append(1.0)
        data_main[str(wavelength)] = values

    payload = {
        "header": header,
        "spectral_data": {
            "units": "relative",
            "index": {"main": channels},
            "data": {"main": data_main},
        },
    }
    file_path = target_dir / filename
    file_path.write_text(json.dumps(payload), encoding="utf-8")
    return file_path


class TestMetadataBindings:
    def test_metadata_types_exist(self):
        assert hasattr(rawtoaces, "SpectralData")
        assert hasattr(rawtoaces, "Metadata")
        assert hasattr(rawtoaces.Metadata, "Calibration")
        assert hasattr(rawtoaces, "MetadataSolver")
        assert hasattr(rawtoaces, "SpectralSolver")

    def test_metadata_calibration_round_trip(self):
        metadata = _init_reference_metadata()
        calibration = metadata.calibration

        assert len(calibration) == 2
        assert calibration[0].illuminant == 17
        assert calibration[1].illuminant == 21
        assert len(calibration[0].XYZ_to_RGB_matrix) == 9
        assert len(calibration[1].XYZ_to_RGB_matrix) == 9

    def test_metadata_calibration_requires_two_entries(self):
        metadata = rawtoaces.Metadata()
        calibration = rawtoaces.Metadata.Calibration()

        with pytest.raises(ValueError) as exc_info:
            metadata.calibration = [calibration]
        assert str(exc_info.value) == "The calibration array must contain 2 elements."


class TestMetadataSolverBindings:
    def test_metadata_solver_calculate_cat_matrix(self):
        metadata = _init_reference_metadata()
        solver = rawtoaces.MetadataSolver(metadata)
        cat = solver.calculate_CAT_matrix()

        assert len(cat) == 3
        assert len(cat[0]) == 3
        assert abs(cat[0][0] - 0.9907763427) < 1e-5
        assert abs(cat[0][1] - -0.0022862289) < 1e-5
        assert abs(cat[0][2] - 0.0209908807) < 1e-5
        assert abs(cat[1][0] - -0.0017882434) < 1e-5
        assert abs(cat[1][1] - 0.9941341374) < 1e-5
        assert abs(cat[1][2] - 0.0083008330) < 1e-5
        assert abs(cat[2][0] - 0.0003777587) < 1e-5
        assert abs(cat[2][1] - 0.0015609315) < 1e-5
        assert abs(cat[2][2] - 1.1063201101) < 1e-5

    def test_metadata_solver_calculate_idt_matrix(self):
        metadata = _init_reference_metadata()
        solver = rawtoaces.MetadataSolver(metadata)
        idt = solver.calculate_IDT_matrix()

        assert len(idt) == 3
        assert len(idt[0]) == 3
        assert abs(idt[0][0] - 1.0536466144) < 1e-5
        assert abs(idt[0][1] - 0.0039044182) < 1e-5
        assert abs(idt[0][2] - 0.0049084502) < 1e-5
        assert abs(idt[1][0] - -0.4899562165) < 1e-5
        assert abs(idt[1][1] - 1.3614787986) < 1e-5
        assert abs(idt[1][2] - 0.1020844728) < 1e-5
        assert abs(idt[2][0] - -0.0024498461) < 1e-5
        assert abs(idt[2][1] - 0.0060497128) < 1e-5
        assert abs(idt[2][2] - 1.0139159537) < 1e-5


class TestSpectralSolverBindings:
    def test_spectral_solver_default_constructor(self):
        solver = rawtoaces.SpectralSolver()
        assert solver is not None
        assert solver.verbosity == 0
        assert solver.last_error_message == ""

        wb = solver.get_WB_multipliers()
        assert len(wb) == 3
        assert abs(wb[0] - 1.0) < 1e-12
        assert abs(wb[1] - 1.0) < 1e-12
        assert abs(wb[2] - 1.0) < 1e-12

        idt = solver.get_IDT_matrix()
        assert len(idt) == 3
        assert len(idt[0]) == 3
        assert len(idt[1]) == 3
        assert len(idt[2]) == 3

    def test_spectral_solver_collect_data_files(self, tmp_path):
        camera_dir = tmp_path / "camera"
        camera_dir.mkdir()
        (camera_dir / "test_camera.json").write_text("{}", encoding="utf-8")
        (camera_dir / "ignored.txt").write_text("x", encoding="utf-8")

        solver = rawtoaces.SpectralSolver([str(tmp_path)])
        files = solver.collect_data_files("camera")

        assert len(files) == 1
        assert files[0].endswith("test_camera.json")

    def test_spectral_solver_load_spectral_data_not_found(self):
        solver = rawtoaces.SpectralSolver()
        observer = rawtoaces.SpectralData()

        assert solver.load_spectral_data("cmf/missing.json", observer) is False

    def test_spectral_solver_find_camera_without_database_match(self, tmp_path):
        solver = rawtoaces.SpectralSolver([str(tmp_path)])
        assert solver.find_camera("nikon", "d200") is False

    def test_spectral_solver_find_camera_rejects_empty_make_or_model(self):
        solver = rawtoaces.SpectralSolver()

        with pytest.raises(ValueError) as exc_info:
            solver.find_camera("", "d200")
        assert str(exc_info.value) == "Camera make and model must be non-empty."

        with pytest.raises(ValueError) as exc_info:
            solver.find_camera("nikon", "")
        assert str(exc_info.value) == "Camera make and model must be non-empty."

    def test_spectral_solver_find_illuminant_string_builtin(self):
        solver = rawtoaces.SpectralSolver()

        assert solver.find_illuminant("d55") is True
        assert solver.find_illuminant("3200k") is True

    def test_spectral_solver_find_illuminant_string_rejects_empty(self):
        solver = rawtoaces.SpectralSolver()

        with pytest.raises(ValueError) as exc_info:
            solver.find_illuminant("")
        assert str(exc_info.value) == "Illuminant type must be non-empty."

    def test_spectral_solver_find_illuminant_wb_requires_camera(self):
        solver = rawtoaces.SpectralSolver()

        assert solver.find_illuminant([1.8, 1.0, 1.4]) is False
        assert (
            "Camera needs to be initialised prior to calling "
            "SpectralSolver::find_illuminant()."
        ) in solver.last_error_message

    def test_spectral_solver_find_illuminant_wb_rejects_non_rgb_vector(self):
        solver = rawtoaces.SpectralSolver()

        with pytest.raises(ValueError) as exc_info:
            solver.find_illuminant([1.0, 1.0])
        assert (
            str(exc_info.value)
            == "White-balance multipliers must contain 3 elements."
        )

    def test_spectral_solver_calculate_wb_requires_camera(self):
        solver = rawtoaces.SpectralSolver()
        solver.find_illuminant("d55")

        assert solver.calculate_WB() is False
        assert (
            "Camera needs to be initialised prior to calling "
            "SpectralSolver::calculate_WB()."
        ) in solver.last_error_message

    def test_spectral_solver_member_objects_are_writable(self):
        solver = rawtoaces.SpectralSolver()
        observer = rawtoaces.SpectralData()
        observer.type = "observer_custom"
        observer.units = "relative"

        solver.observer = observer
        assert isinstance(solver.observer, rawtoaces.SpectralData)
        assert solver.observer.type == "observer_custom"
        assert solver.observer.units == "relative"

    def test_spectral_solver_full_configuration_and_idt(self, tmp_path):
        _write_test_spectral_file(
            tmp_path,
            "camera",
            "test_camera.json",
            ["R", "G", "B"],
            {"manufacturer": "nikon", "model": "d200"},
        )
        _write_test_spectral_file(
            tmp_path,
            "cmf",
            "cmf_1931.json",
            ["X", "Y", "Z"],
            {"type": "cmf"},
        )
        _write_test_spectral_file(
            tmp_path,
            "training",
            "training_spectral.json",
            ["patch1", "patch2", "patch3"],
            {"type": "training"},
        )

        solver = rawtoaces.SpectralSolver([str(tmp_path)])
        assert solver.find_camera("nikon", "d200") is True
        assert solver.find_illuminant("d55") is True
        assert solver.load_spectral_data("cmf/cmf_1931.json", solver.observer) is True
        assert (
            solver.load_spectral_data(
                "training/training_spectral.json", solver.training_data
            )
            is True
        )
        assert solver.calculate_WB() is True
        assert solver.calculate_IDT_matrix() is True

        wb = solver.get_WB_multipliers()
        idt = solver.get_IDT_matrix()

        assert len(wb) == 3
        assert len(idt) == 3
        for row in idt:
            assert len(row) == 3
            for value in row:
                assert math.isfinite(value)
