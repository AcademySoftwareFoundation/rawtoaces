#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the rawtoaces Project.

"""
Unit tests for rawtoaces Python bindings - core solver and spectrum bindings
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
        assert hasattr(rawtoaces, "Spectrum")
        assert hasattr(rawtoaces.Spectrum, "Shape")
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

def compare_mat(m1, m2):
    assert len(m1) == len(m2)
    for i in range(len(m1)):
        assert len(m1[i]) == len(m2[i])
        for j in range(len(m1[i])):
            assert abs(m1[i][j] - m2[i][j]) < 1e-5

def true_cat():
    return  [
        [ 0.99099652028664909, -0.0039432226891074323, 0.018921317065371118 ],
        [ -0.0039159159820485179, 0.99816564406286434, 0.0060505224917670325 ],
        [ 0.0040217656639693206, -0.0069148526777877001, 1.1040273790037682 ]
    ]
    
def true_idt():
    return  [
        [ 0.68832510354036558, 0.21027184302848373, 0.10504235016823139 ],
        [ -0.0074165224762133899, 1.0071009763365999, 0.0039548428394460544 ],
        [ -0.086152187513382994, -0.50370399487211537, 1.5934954790790503 ]
    ]
        
class TestMetadataSolverBindings:
    def test_metadata_solver_calculate_cat_matrix(self):
        metadata = _init_reference_metadata()
        solver = rawtoaces.MetadataSolver(metadata)
        cat = solver.calculate_CAT_matrix()
        compare_mat(cat, true_cat())

    def test_metadata_solver_calculate_idt_matrix(self):
        metadata = _init_reference_metadata()
        solver = rawtoaces.MetadataSolver(metadata)
        idt = solver.calculate_IDT_matrix()
        compare_mat(idt, true_idt())
        
    def test_metadata_solver_calculate_transform(self):
        metadata = _init_reference_metadata()
        solver = rawtoaces.MetadataSolver(metadata)
        success = solver.calculate_transform()
        idt = solver.transform_matrix
        compare_mat(idt, true_idt())

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
        assert solver.calculate_transform() is True

        wb = solver.get_WB_multipliers()
        idt = solver.get_IDT_matrix()

        assert len(wb) == 3
        assert len(idt) == 3
        for row in idt:
            assert len(row) == 3
            for value in row:
                assert math.isfinite(value)


def _reference_sample_count():
    shape = rawtoaces.Spectrum.ReferenceShape
    return int((shape.last - shape.first) / shape.step) + 1


class TestSpectrumBindings:
    def test_reference_and_empty_shapes_are_copies(self):
        reference = rawtoaces.Spectrum.ReferenceShape
        assert reference == rawtoaces.Spectrum.Shape(380, 780, 5)
        reference.first = 0
        assert rawtoaces.Spectrum.ReferenceShape.first == 380

        empty = rawtoaces.Spectrum.EmptyShape
        assert empty == rawtoaces.Spectrum.Shape(0, 0, 0)
        assert reference != empty
        assert (reference == 1) is False
        with pytest.raises(TypeError):
            hash(rawtoaces.Spectrum.Shape(380, 780, 5))

    def test_constructor_fills_samples_for_the_requested_shape(self):
        shape = rawtoaces.Spectrum.Shape(380, 390, 5)
        curve = rawtoaces.Spectrum(2.5, shape)

        assert curve.shape == shape
        assert curve.values == [2.5, 2.5, 2.5]
        curve.values[0] = 9.0
        assert curve.values == [2.5, 2.5, 2.5]
        assert len(rawtoaces.Spectrum().values) == _reference_sample_count() == 81

        empty = rawtoaces.Spectrum(1.0, rawtoaces.Spectrum.EmptyShape)
        assert empty.values == []
        assert empty.max() == 0
        assert empty.integrate() == 0

        with pytest.raises(ValueError) as exc_info:
            rawtoaces.Spectrum(0.0, rawtoaces.Spectrum.Shape(780, 380, 5))
        assert "last >= first" in str(exc_info.value)
        with pytest.raises(ValueError):
            rawtoaces.Spectrum(
                0.0, rawtoaces.Spectrum.Shape(float("nan"), 780, 5)
            )

    def test_shape_assignment_keeps_existing_samples(self):
        curve = rawtoaces.Spectrum(1.0, rawtoaces.Spectrum.Shape(380, 390, 5))
        curve.shape.last = 400
        assert curve.shape.last == 400

        curve.shape = rawtoaces.Spectrum.Shape(400, 410, 5)
        assert curve.shape.first == 400
        assert curve.values == [1.0, 1.0, 1.0]

    def test_copy_is_independent(self):
        original = rawtoaces.Spectrum(4.0, rawtoaces.Spectrum.Shape(380, 390, 5))
        copied = rawtoaces.Spectrum(original)
        copied.values = [0.0, 0.0, 0.0]

        assert original.values == [4.0, 4.0, 4.0]

    def test_arithmetic_is_per_element_and_checks_sampling(self):
        shape = rawtoaces.Spectrum.Shape(380, 390, 5)
        left = rawtoaces.Spectrum(0.0, shape)
        right = rawtoaces.Spectrum(0.0, shape)
        left.values = [2.0, 3.0, 4.0]
        right.values = [5.0, 1.0, 2.0]

        assert (left + right).values == [7.0, 4.0, 6.0]
        assert (left - right).values == [-3.0, 2.0, 2.0]
        assert (left * right).values == [10.0, 3.0, 8.0]
        assert (left / right).values == [0.4, 3.0, 2.0]
        assert left.values == [2.0, 3.0, 4.0]

        updated = left
        left += right
        assert updated is left
        assert left.values == [7.0, 4.0, 6.0]
        left -= right
        left *= right
        assert left.values == [10.0, 3.0, 8.0]
        left /= right
        assert left.values == [2.0, 3.0, 4.0]

        mismatched = rawtoaces.Spectrum(1.0, rawtoaces.Spectrum.Shape(380, 400, 10))
        with pytest.raises(ValueError) as exc_info:
            left + mismatched
        assert (
            str(exc_info.value)
            == "Spectrum operands must have the same shape and sample count."
        )

        shortened = rawtoaces.Spectrum(left)
        shortened.values = shortened.values[:-1]
        with pytest.raises(ValueError):
            left += shortened
        assert left.values == [2.0, 3.0, 4.0]

        with pytest.raises(TypeError) as exc_info:
            left + 1
        assert "incompatible function arguments" not in str(exc_info.value)

    def test_reshape_interpolates_onto_the_reference_shape(self):
        curve = rawtoaces.Spectrum(0.0, rawtoaces.Spectrum.Shape(380, 400, 10))
        curve.values = [10.0, 20.0, 30.0]
        curve.reshape()

        assert curve.shape == rawtoaces.Spectrum.ReferenceShape
        assert len(curve.values) == 81
        assert curve.values[0] == pytest.approx(10.0)
        assert curve.values[1] == pytest.approx(15.0)
        assert curve.values[2] == pytest.approx(20.0)
        assert curve.values[3] == pytest.approx(25.0)
        assert curve.values[4] == pytest.approx(30.0)
        assert curve.integrate() == pytest.approx(sum(curve.values))
        assert curve.max() == pytest.approx(max(curve.values))

        already_reference = rawtoaces.Spectrum(1.0)
        already_reference.reshape()
        assert already_reference.values == [1.0] * 81

        with pytest.raises(ValueError) as exc_info:
            rawtoaces.Spectrum(0.0, rawtoaces.Spectrum.EmptyShape).reshape()
        assert str(exc_info.value) == "Cannot reshape a spectrum with no samples."

    def test_spectral_data_channels_round_trip(self, tmp_path):
        _write_test_spectral_file(
            tmp_path,
            "illuminant",
            "flat.json",
            ["power"],
            {"type": "illuminant"},
        )
        data = rawtoaces.SpectralData()
        assert data.load(str(tmp_path / "illuminant" / "flat.json")) is True

        loaded = data["power"]
        assert isinstance(loaded, rawtoaces.Spectrum)
        assert loaded.shape == rawtoaces.Spectrum.ReferenceShape
        assert loaded.values[0] == pytest.approx(1.0)
        assert loaded.values[1] == pytest.approx(1.05)
        assert data.get("main", "power").values == loaded.values

        loaded.values = [0.0] * len(loaded.values)
        assert data["power"].values[0] == pytest.approx(1.0)

        replacement = rawtoaces.Spectrum(3.0)
        data["power"] = replacement
        assert data["power"].values == [3.0] * 81

        with pytest.raises(ValueError) as exc_info:
            data["missing"]
        assert "missing" in str(exc_info.value)
        with pytest.raises(ValueError) as exc_info:
            data.get("other", "power")
        assert "other" in str(exc_info.value)

        assert "power" in data
        assert "missing" not in data
        assert "power" not in rawtoaces.SpectralData()
        with pytest.raises(ValueError) as exc_info:
            rawtoaces.SpectralData()["power"]
        assert "main" in str(exc_info.value)
        with pytest.raises(TypeError):
            list(data)
