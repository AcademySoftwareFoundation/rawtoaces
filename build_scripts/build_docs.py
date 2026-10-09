#!/usr/bin/env python3
# Copyright Contributors to the rawtoaces Project.
# SPDX-License-Identifier: Apache-2.0

"""Build and validate the checked-out documentation, independently of CMake.

Documentation identifies its checkout: release tags keep their tag name and
other checkouts are labelled ``main (development)``. This deliberately does not
infer a release from CMake or the changelog. RAWTOACES_DOCS_VERSION is an explicit
override for packaged source builds. Historical publication calls build_docs
with strict=False; current sources always use strict validation.
"""

from __future__ import annotations

import argparse
import ast
from collections import Counter
from html.parser import HTMLParser
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile


DEVELOPMENT_VERSION = "main (development)"


def documentation_version(repo_root: Path) -> str:
    explicit = os.environ.get("RAWTOACES_DOCS_VERSION")
    if explicit:
        return explicit
    if os.environ.get("READTHEDOCS") == "True":
        if os.environ.get("READTHEDOCS_VERSION_TYPE") == "tag":
            return os.environ["READTHEDOCS_GIT_IDENTIFIER"]
        return DEVELOPMENT_VERSION
    if os.environ.get("GITHUB_REF_TYPE") == "tag":
        return os.environ["GITHUB_REF_NAME"]
    if os.environ.get("GITHUB_REF_TYPE") == "branch" or os.environ.get("GITHUB_EVENT_NAME") == "pull_request":
        return DEVELOPMENT_VERSION
    # A detached release checkout should also work outside CI.
    result = subprocess.run(
        ["git", "describe", "--tags", "--exact-match", "HEAD"],
        cwd=repo_root, text=True, capture_output=True,
    )
    return result.stdout.strip() if result.returncode == 0 else DEVELOPMENT_VERSION


def doxygen_quote(value: str) -> str:
    if "\n" in value or "\r" in value:
        raise ValueError("Doxygen configuration values cannot contain newlines")
    return '"' + value.replace("\\", "\\\\").replace('"', '\\"') + '"'


def check_doxygen_warnings(repo_root: Path, warnings: str) -> None:
    """Permit only three undocumented ROI inputs slated for deprecation.

    Keep parameter warnings enabled. Suppressing that whole warning category
    would hide missing documentation on current APIs as well.
    """
    header = re.escape(str((repo_root / "include/rawtoaces/image_converter.h").resolve()))
    ignored_roi = re.compile(
        rf"^{header}:\d+: warning: The following parameter of "
        r"rta::util::ImageConverter::apply_(?:matrix|scale|crop)\("
        r"OIIO::ImageBuf\s*&dst,\s*const OIIO::ImageBuf\s*&src,\s*"
        r"OIIO::ROI roi\s*=\s*\{\}\) is not documented:\n"
        r"[ \t]+parameter 'roi'\n?",
        re.MULTILINE,
    )
    unexpected = ignored_roi.sub("", warnings).strip()
    if unexpected:
        raise ValueError("Unexpected Doxygen diagnostics:\n" + unexpected)


def run_doxygen(
    repo_root: Path, output_dir: Path, label: str, *, strict: bool = True,
    executable: str = "doxygen", configuration_path: Path | None = None,
) -> None:
    # Append overrides so historical Doxyfiles receive the same checkout label.
    configuration = (configuration_path or repo_root / "src/docs/Doxyfile").read_text(encoding="utf-8")
    configuration += (
        f"\nPROJECT_NUMBER = {doxygen_quote(label)}\n"
        f"OUTPUT_DIRECTORY = {doxygen_quote(str(output_dir))}\n"
        "WARN_AS_ERROR = NO\n"
    )
    if strict:
        configuration += (
            "WARNINGS = YES\nWARN_IF_UNDOCUMENTED = YES\n"
            "WARN_IF_DOC_ERROR = YES\nWARN_NO_PARAMDOC = YES\n"
        )
    with tempfile.TemporaryDirectory(prefix="rawtoaces-doxygen-warnings-") as temporary:
        warning_log = Path(temporary) / "warnings.log"
        configuration += (
            f"WARN_LOGFILE = {doxygen_quote(str(warning_log))}\n"
            'WARN_FORMAT = "$file:$line: $text"\n'
        )
        result = subprocess.run(
            [executable, "-"], input=configuration, text=True,
            cwd=repo_root / "src", stderr=subprocess.PIPE,
        )
        if result.stderr:
            print(result.stderr, file=sys.stderr, end="")
        # A permitted warning never excuses a failed Doxygen process.
        result.check_returncode()
        warnings = warning_log.read_text(encoding="utf-8")
        if strict:
            check_doxygen_warnings(repo_root, warnings)
            if result.stderr.strip():
                raise ValueError("Unexpected Doxygen stderr:\n" + result.stderr.strip())
        elif warnings:
            print(warnings, file=sys.stderr, end="")


class DocumentationHTML(HTMLParser):
    """Read signatures, anchors and Python blocks without an HTML dependency."""

    def __init__(self) -> None:
        super().__init__(convert_charrefs=True)
        self.ids: Counter[str] = Counter()
        self.methods: dict[str, list[str]] = {}
        self.text: list[str] = []
        self.python_blocks: list[str] = []
        self._descriptions: list[dict] = []
        self._signature: list[str] | None = None
        self._signature_description: dict | None = None
        self._divs: list[bool] = []
        self._code: list[str] | None = None

    def handle_starttag(self, tag: str, attrs: list[tuple[str, str | None]]) -> None:
        attributes = dict(attrs)
        anchor = attributes.get("id")
        if anchor:
            self.ids[anchor] += 1
        classes = (attributes.get("class") or "").lower().split()
        if tag == "dl":
            self._descriptions.append({"anchor": None, "signatures": []})
        if tag == "dt" and "sig" in classes and self._descriptions:
            self._signature = []
            self._signature_description = self._descriptions[-1]
            if anchor:
                self._signature_description["anchor"] = anchor
        if tag == "div":
            self._divs.append("highlight-python" in classes or "highlight-python3" in classes)
        if tag == "pre" and any(self._divs):
            self._code = []

    def handle_endtag(self, tag: str) -> None:
        if tag == "dt" and self._signature is not None:
            self._signature_description["signatures"].append("".join(self._signature))
            self._signature = None
            self._signature_description = None
        if tag == "dl" and self._descriptions:
            description = self._descriptions.pop()
            if description["anchor"]:
                self.methods[description["anchor"]] = description["signatures"]
        if tag == "pre" and self._code is not None:
            self.python_blocks.append("".join(self._code))
            self._code = None
        if tag == "div" and self._divs:
            self._divs.pop()

    def handle_data(self, data: str) -> None:
        self.text.append(data)
        if self._signature is not None:
            self._signature.append(data)
        if self._code is not None:
            self._code.append(data)


def check_html(output_dir: Path) -> None:
    reference_path = output_dir / "api/python/api_reference.html"
    reference = DocumentationHTML()
    reference.feed(reference_path.read_text(encoding="utf-8"))
    required_anchors = [
        "rawtoaces.ImageConverter.configure",
        "rawtoaces.SpectralSolver.find_illuminant",
        "rawtoaces.TransformSolver.calculate_transform",
        "rawtoaces.TransformSolver.transform_matrix",
        "rawtoaces.TransformSolver.last_error_message",
        "rawtoaces.MetadataSolver.calculate_transform",
        "rawtoaces.SpectralSolver.calculate_transform",
    ]
    errors = []
    for anchor in required_anchors:
        if reference.ids[anchor] != 1:
            errors.append(f"{reference_path}: expected one canonical anchor {anchor}")

    expected_overloads = {
        "rawtoaces.ImageConverter.configure": [
            r"configure\s*\(\s*input_filename\b",
            r"configure\s*\(\s*image_spec\b[^)]*\boptions\b",
        ],
        "rawtoaces.SpectralSolver.find_illuminant": [
            r"find_illuminant\s*\(\s*type\b",
            r"find_illuminant\s*\(\s*wb_multipliers\b",
        ],
    }
    for anchor, patterns in expected_overloads.items():
        signatures = reference.methods.get(anchor, [])
        if len(signatures) != 2:
            errors.append(f"{reference_path}: expected two overload signatures for {anchor}")
        for pattern in patterns:
            if not any(re.search(pattern, signature) for signature in signatures):
                errors.append(f"{reference_path}: missing overload of {anchor}: {pattern}")
        if any("**kwds" in signature for signature in signatures):
            errors.append(f"{reference_path}: placeholder signature for {anchor}")

    for path in sorted(output_dir.rglob("*.html")):
        page = reference if path == reference_path else DocumentationHTML()
        if page is not reference:
            page.feed(path.read_text(encoding="utf-8"))
        text = "".join(page.text)
        if "Helper for @overload" in text or "typing._overload_dummy" in text:
            errors.append(f"{path}: typing overload helper leaked into the reference")
        for index, snippet in enumerate(page.python_blocks, 1):
            try:
                ast.parse(snippet, filename=f"{path}:Python block {index}")
            except SyntaxError as error:
                errors.append(f"{error.filename}, line {error.lineno}: {error.msg}")
    if errors:
        raise ValueError("Documentation output validation failed:\n" + "\n".join(errors))


def build_docs(
    repo_root: Path, output_dir: Path, label: str | None = None, *,
    strict: bool = True, doxygen_output: Path | None = None,
    skip_doxygen: bool = False, sphinx_build: str | None = None,
    doxygen_executable: str = "doxygen", doxygen_configuration: Path | None = None,
) -> None:
    label = label or documentation_version(repo_root)
    with tempfile.TemporaryDirectory(prefix="rawtoaces-doxygen-") as temporary:
        xml_root = doxygen_output or Path(temporary)
        if not skip_doxygen:
            run_doxygen(
                repo_root, xml_root, label, strict=strict,
                executable=doxygen_executable, configuration_path=doxygen_configuration,
            )
        env = os.environ.copy()
        # Older conf.py files otherwise run a second, unversioned Doxygen build.
        env.pop("READTHEDOCS", None)
        env["RAWTOACES_DOCS_VERSION"] = label
        env["RAWTOACES_DOCS_STRICT"] = "1" if strict else "0"
        env["RAWTOACES_DOCS_DOXYGEN_XML"] = str(xml_root / "xml")
        command = [sphinx_build] if sphinx_build else [sys.executable, "-m", "sphinx"]
        command.extend(["-b", "html", "-E"])
        if strict:
            command.extend(["-W", "--keep-going", "-n"])
        command.extend([
            "-D", f"breathe_projects.rawtoaces={xml_root / 'xml'}",
            "-D", f"version={label}", "-D", f"release={label}",
            "-D", f"nitpicky={int(strict)}",
            str(repo_root / "src/docs"), str(output_dir),
        ])
        subprocess.run(command, cwd=repo_root / "src/docs", env=env, check=True)
        # Historical conf.py lacks our build-finished hook; assertions apply
        # only to current sources, which must expose the current API contract.
        if strict:
            check_html(output_dir)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo-root", default=".")
    parser.add_argument("--output-dir", default="site-preview")
    parser.add_argument("--version", help="Explicit checkout label for both Sphinx and Doxygen.")
    parser.add_argument("--print-version", action="store_true")
    parser.add_argument("--check-html", type=Path, help="Validate existing generated HTML only.")
    parser.add_argument("--doxygen-output", type=Path)
    parser.add_argument("--doxygen-only", action="store_true", help="Generate and validate Doxygen XML only.")
    parser.add_argument("--doxygen-executable", default="doxygen")
    parser.add_argument("--doxygen-config", type=Path, help="Use the Doxyfile configured by CMake.")
    parser.add_argument("--skip-doxygen", action="store_true", help="Use XML already built by CMake.")
    parser.add_argument("--sphinx-build", help="Sphinx executable used by CMake.")
    args = parser.parse_args()
    if args.check_html:
        check_html(args.check_html.resolve())
        return 0
    repo_root = Path(args.repo_root).resolve()
    label = args.version or documentation_version(repo_root)
    if args.print_version:
        print(label)
        return 0
    if args.doxygen_only:
        if args.doxygen_output is None:
            parser.error("--doxygen-only requires --doxygen-output")
        run_doxygen(
            repo_root, args.doxygen_output.resolve(), label,
            executable=args.doxygen_executable,
            configuration_path=args.doxygen_config.resolve() if args.doxygen_config else None,
        )
        return 0
    build_docs(
        repo_root, Path(args.output_dir).resolve(), label,
        doxygen_output=args.doxygen_output.resolve() if args.doxygen_output else None,
        skip_doxygen=args.skip_doxygen, sphinx_build=args.sphinx_build,
        doxygen_executable=args.doxygen_executable,
        doxygen_configuration=args.doxygen_config.resolve() if args.doxygen_config else None,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
