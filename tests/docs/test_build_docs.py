# Copyright Contributors to the rawtoaces Project.
# SPDX-License-Identifier: Apache-2.0

"""Regression checks for documentation failures that ordinary HTML builds miss."""

from __future__ import annotations

import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
import xml.etree.ElementTree as ET

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "build_scripts"))
from build_docs import DEVELOPMENT_VERSION, check_html, documentation_version, run_doxygen


REPO_ROOT = Path(__file__).resolve().parents[2]


def reference_html() -> str:
    methods = {
        "rawtoaces.ImageConverter.configure": [
            "configure(input_filename: str)", "configure(image_spec: ImageSpec, options: ParamValueList)",
        ],
        "rawtoaces.SpectralSolver.find_illuminant": [
            "find_illuminant(type: str)", "find_illuminant(wb_multipliers: Sequence[float])",
        ],
        "rawtoaces.TransformSolver.calculate_transform": ["calculate_transform()"],
        "rawtoaces.TransformSolver.transform_matrix": ["transform_matrix"],
        "rawtoaces.TransformSolver.last_error_message": ["last_error_message"],
        "rawtoaces.MetadataSolver.calculate_transform": ["calculate_transform()"],
        "rawtoaces.SpectralSolver.calculate_transform": ["calculate_transform()"],
    }
    blocks = []
    for anchor, signatures in methods.items():
        blocks.append('<dl class="py method">')
        for index, signature in enumerate(signatures):
            identifier = f' id="{anchor}"' if index == 0 else ""
            blocks.append(f'<dt class="sig sig-object py"{identifier}>{signature}</dt>')
        blocks.append("</dl>")
    return "\n".join(blocks)


class OutputChecks(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.output = Path(self.temporary.name)
        self.reference = self.output / "api/python/api_reference.html"
        self.reference.parent.mkdir(parents=True)
        self.reference.write_text(reference_html(), encoding="utf-8")

    def test_complete_reference_passes(self):
        check_html(self.output)

    def test_missing_inherited_anchor_fails(self):
        self.reference.write_text(reference_html().replace("rawtoaces.SpectralSolver.calculate_transform", "missing"))
        with self.assertRaisesRegex(ValueError, "canonical anchor rawtoaces.SpectralSolver.calculate_transform"):
            check_html(self.output)

    def test_duplicate_canonical_anchor_fails(self):
        with self.reference.open("a") as file:
            file.write('<span id="rawtoaces.ImageConverter.configure"></span>')
        with self.assertRaisesRegex(ValueError, "canonical anchor rawtoaces.ImageConverter.configure"):
            check_html(self.output)

    def test_unrelated_signature_cannot_replace_missing_overload(self):
        self.reference.write_text(reference_html().replace(
            "configure(image_spec: ImageSpec, options: ParamValueList)", "configure(**kwds)",
        ) + '<dl><dt class="sig">configure(image_spec, options)</dt></dl>')
        with self.assertRaisesRegex(ValueError, "missing overload of rawtoaces.ImageConverter.configure"):
            check_html(self.output)

    def test_extra_overload_fails(self):
        self.reference.write_text(reference_html().replace(
            "configure(input_filename: str)</dt>",
            "configure(input_filename: str)</dt><dt class=\"sig\">configure(other)</dt>",
        ))
        with self.assertRaisesRegex(ValueError, "expected two overload signatures"):
            check_html(self.output)

    def test_typing_helper_fails_even_with_valid_signatures(self):
        with self.reference.open("a") as file:
            file.write("<p>Helper for @overload to raise when called.</p>")
        with self.assertRaisesRegex(ValueError, "typing overload helper"):
            check_html(self.output)

    def test_python_blocks_are_parsed_including_capitalized_language(self):
        snippet = '<div class="highlight-Python notranslate"><div class="highlight"><pre>{}</pre></div></div>'
        path = self.output / "lens_correction.html"
        path.write_text(snippet.format("flags = (1 |\n    2)\n"))
        check_html(self.output)
        path.write_text(snippet.format("flags =\n    1 | 2\n"))
        with self.assertRaisesRegex(ValueError, "lens_correction.html:Python block 1"):
            check_html(self.output)


class CheckoutVersion(unittest.TestCase):
    def test_hosted_checkout_identity(self):
        cases = [
            ({"RAWTOACES_DOCS_VERSION": "v2.2.2"}, "v2.2.2"),
            ({"READTHEDOCS": "True", "READTHEDOCS_VERSION_TYPE": "tag",
              "READTHEDOCS_VERSION": "stable", "READTHEDOCS_GIT_IDENTIFIER": "v2.2.2"}, "v2.2.2"),
            ({"READTHEDOCS": "True", "READTHEDOCS_VERSION_TYPE": "branch"}, DEVELOPMENT_VERSION),
            ({"READTHEDOCS": "True", "READTHEDOCS_VERSION_TYPE": "external",
              "READTHEDOCS_GIT_IDENTIFIER": "342"}, DEVELOPMENT_VERSION),
            ({"GITHUB_REF_TYPE": "tag", "GITHUB_REF_NAME": "v2.2.2"}, "v2.2.2"),
            ({"GITHUB_EVENT_NAME": "pull_request"}, DEVELOPMENT_VERSION),
        ]
        for env, expected in cases:
            with self.subTest(env=env), patch.dict(os.environ, env, clear=True):
                self.assertEqual(documentation_version(REPO_ROOT), expected)


class DoxygenDiagnostics(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        docs = self.root / "src/docs"
        docs.mkdir(parents=True)
        (docs / "Doxyfile").write_text("PROJECT_NAME = test\n")

    def warning(self, method="apply_matrix", parameter="roi", filename=None):
        header = filename or self.root / "include/rawtoaces/image_converter.h"
        return (
            f"{header.resolve()}:123: warning: The following parameter of "
            f"rta::util::ImageConverter::{method}(OIIO::ImageBuf &dst, "
            f"const OIIO::ImageBuf &src, OIIO::ROI roi={{}}) is not documented:\n"
            f"  parameter '{parameter}'\n"
        )

    def run_diagnostics(self, warnings, *, stderr="", returncode=0, create_log=True, strict=True):
        executable = self.root / "diagnostic-doxygen"
        executable.write_text(
            f"#!{sys.executable}\n"
            "import ast, pathlib, re, sys\n"
            "configuration = sys.stdin.read()\n"
            "log_value = re.search(r'^WARN_LOGFILE = (.+)$', configuration, re.MULTILINE).group(1)\n"
            "log = pathlib.Path(ast.literal_eval(log_value))\n"
            f"if {create_log!r}: log.write_text({warnings!r}, encoding='utf-8')\n"
            f"sys.stderr.write({stderr!r})\n"
            f"raise SystemExit({returncode})\n"
        )
        executable.chmod(0o755)
        run_doxygen(
            self.root, self.root / "doxygen", DEVELOPMENT_VERSION,
            executable=str(executable), strict=strict,
        )

    def test_only_known_roi_warnings_are_permitted(self):
        self.run_diagnostics("".join(self.warning(method) for method in (
            "apply_matrix", "apply_scale", "apply_crop",
        )))

    def test_unrelated_parameter_method_class_and_file_fail(self):
        cases = [
            self.warning(parameter="src"),
            self.warning(method="apply_lens_correction"),
            self.warning().replace("ImageConverter", "SpectralSolver"),
            self.warning(filename=self.root / "include/rawtoaces/other.h"),
            self.warning().replace("OIIO::ROI roi={}", "OIIO::ROI roi={}, bool extra=false"),
        ]
        for diagnostic in cases:
            with self.subTest(diagnostic=diagnostic), self.assertRaisesRegex(ValueError, "Unexpected Doxygen diagnostics"):
                self.run_diagnostics(diagnostic)

    def test_allowed_warning_does_not_hide_other_diagnostics(self):
        with self.assertRaisesRegex(ValueError, "unknown command"):
            self.run_diagnostics(self.warning() + "warning: unknown command @typo\n")

    def test_missing_warning_log_fails(self):
        with self.assertRaises(FileNotFoundError):
            self.run_diagnostics("", create_log=False)

    def test_stderr_without_log_diagnostic_fails(self):
        with self.assertRaisesRegex(ValueError, "Unexpected Doxygen stderr"):
            self.run_diagnostics("", stderr="warning: ignoring unsupported configuration option\n")

    def test_allowed_warning_does_not_excuse_nonzero_exit(self):
        for strict in (True, False):
            with self.subTest(strict=strict), self.assertRaises(subprocess.CalledProcessError) as error:
                self.run_diagnostics(self.warning(), returncode=17, strict=strict)
            self.assertEqual(error.exception.returncode, 17)

    def test_historical_warning_policy_keeps_unrelated_warning_nonfatal(self):
        self.run_diagnostics(self.warning(parameter="src"), strict=False)


class BuildFailures(unittest.TestCase):
    def test_doxygen_nonzero_exit_fails(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            docs = root / "src/docs"
            docs.mkdir(parents=True)
            (docs / "Doxyfile").write_text("PROJECT_NAME = test\n")
            executable = root / "failing-doxygen"
            executable.write_text(f"#!{sys.executable}\nraise SystemExit(17)\n")
            executable.chmod(0o755)
            for strict in (True, False):
                with self.subTest(strict=strict), self.assertRaises(subprocess.CalledProcessError) as error:
                    run_doxygen(root, root / "xml", "main (development)", strict=strict, executable=str(executable))
                self.assertEqual(error.exception.returncode, 17)

    def test_historical_label_reaches_both_tools_without_current_warning_policy(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            docs = root / "src/docs"
            docs.mkdir(parents=True)
            (root / "include").mkdir()
            (root / "include/example.h").write_text("/** Example class. */\nclass Example {};\n")
            (docs / "Doxyfile").write_text(
                "PROJECT_NAME=test\nPROJECT_NUMBER=2.0.0\nINPUT=../include\nGENERATE_XML=YES\n"
                "GENERATE_HTML=NO\nGENERATE_LATEX=NO\nQUIET=YES\n"
            )
            (docs / "conf.py").write_text("project='test'\nversion='2.0'\nrelease='2.0.0'\n")
            (docs / "index.rst").write_text(
                "Historical documentation\n========================\n\n|version| / |release|\n\n"
                ".. intentionally-invalid-directive::\n"
            )
            script = (
                "from pathlib import Path; import sys; "
                f"sys.path.insert(0, {str(REPO_ROOT / 'build_scripts')!r}); "
                "from build_docs import build_docs; "
                f"root = Path({str(root)!r}); "
                "build_docs(root, root / 'html', 'v2.2.2', strict=False, doxygen_output=root / 'doxygen')"
            )
            result = subprocess.run([sys.executable, "-c", script], text=True, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("build succeeded, 1 warning", result.stdout + result.stderr)
            self.assertIn("v2.2.2 / v2.2.2", (root / "html/index.html").read_text())
            configuration = ET.parse(root / "doxygen/xml/Doxyfile.xml")
            self.assertEqual(configuration.find(".//option[@id='PROJECT_NUMBER']/value").text, "v2.2.2")

    def test_broken_rst_in_proposed_checkout_fails(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            docs = root / "src/docs"
            docs.mkdir(parents=True)
            (docs / "conf.py").write_text("project = 'Proposed checkout'\n")
            (docs / "index.rst").write_text("Proposal\n========\n\n.. intentionally-invalid-directive::\n")
            result = subprocess.run([
                sys.executable, str(REPO_ROOT / "build_scripts/build_docs.py"),
                "--repo-root", str(root), "--output-dir", str(root / "html"),
                "--version", "main (development)", "--skip-doxygen",
            ], text=True, capture_output=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('Unknown directive type "intentionally-invalid-directive"', result.stdout + result.stderr)
            self.assertIn("build finished with problems", result.stdout + result.stderr)


if __name__ == "__main__":
    unittest.main()
