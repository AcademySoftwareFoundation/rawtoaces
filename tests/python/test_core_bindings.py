#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the rawtoaces Project.

"""
Unit tests for rawtoaces Python bindings - core metadata solver bindings
"""

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


class TestMetadataBindings:
    def test_metadata_types_exist(self):
        assert hasattr(rawtoaces, "Metadata")
        assert hasattr(rawtoaces.Metadata, "Calibration")
        assert hasattr(rawtoaces, "MetadataSolver")

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

        with pytest.raises(ValueError):
            metadata.calibration = [calibration]


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
