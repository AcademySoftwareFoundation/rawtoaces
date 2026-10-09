#!/usr/bin/env python3
# Copyright Contributors to the rawtoaces Project.
# SPDX-License-Identifier: Apache-2.0

"""Regenerate the Python reference from the freshly built binding module.

Run with the interpreter used to build the extension. Docstrings and signatures
come exclusively from nanobind.stubgen. Mechanical normalization makes forward
annotations and enum aliases importable and uses OpenImageIO's public names.
"""

from __future__ import annotations

import argparse
import ast
from importlib.machinery import EXTENSION_SUFFIXES
from pathlib import Path
import subprocess
import sys
import tempfile


HEADER = (
    "# Generated from compiled bindings by nanobind.stubgen.\n"
    "# Edit src/bindings/*.cpp docstrings, rebuild and regenerate; do not edit prose here.\n"
    "# Forward annotations, enum aliases and public OpenImageIO names are normalized.\n"
    "\n"
)


def normalize_source(source: str) -> str:
    source = source.replace("OpenImageIO.OpenImageIO.", "OpenImageIO.")
    source = source.replace("import OpenImageIO.OpenImageIO", "import OpenImageIO")
    source = "\n".join(line.rstrip() for line in source.splitlines()) + "\n"
    tree = ast.parse(source)
    lines = source.splitlines(keepends=True)
    offsets = [0]
    for line in lines:
        offsets.append(offsets[-1] + len(line))
    replacements = []

    def visit_classes(nodes, scope):
        for node in nodes:
            if isinstance(node, ast.ClassDef):
                visit_classes(node.body, (*scope, node.name))
            elif isinstance(node, ast.AnnAssign) and isinstance(node.value, ast.Attribute):
                # stubgen can qualify an exported enum alias through its class
                # before that class exists. Within that class, use its local enum.
                value = ast.unparse(node.value)
                prefix = ".".join(scope) + "."
                if scope and value.startswith(prefix):
                    begin = offsets[node.value.lineno - 1] + node.value.col_offset
                    end = offsets[node.value.end_lineno - 1] + node.value.end_col_offset
                    replacements.append((begin, end, value[len(prefix):]))

    visit_classes(tree.body, ())
    for begin, end, replacement in sorted(replacements, reverse=True):
        source = source[:begin] + replacement + source[end:]
    # A generated alias can also precede its nested enum definition. Move such
    # exported aliases (and any attached docstrings) after the class members.
    def class_paths(nodes, scope=()):
        for node in nodes:
            if isinstance(node, ast.ClassDef):
                path = (*scope, node.name)
                yield from class_paths(node.body, path)
                yield path

    for path in list(class_paths(ast.parse(source).body)):
        nodes = ast.parse(source).body
        for name in path:
            cls = next(node for node in nodes if isinstance(node, ast.ClassDef) and node.name == name)
            nodes = cls.body
        nested = {node.name: node for node in nodes if isinstance(node, ast.ClassDef)}
        aliases = []
        for index, node in enumerate(nodes):
            if not isinstance(node, ast.AnnAssign) or not isinstance(node.value, ast.Attribute):
                continue
            value = node.value
            while isinstance(value, ast.Attribute):
                value = value.value
            if isinstance(value, ast.Name) and value.id in nested:
                end = node.end_lineno
                if index + 1 < len(nodes):
                    following = nodes[index + 1]
                    if isinstance(following, ast.Expr) and isinstance(following.value, ast.Constant) and isinstance(following.value.value, str):
                        end = following.end_lineno
                aliases.append((node, end, nested[value.id]))
        if any(node.lineno < enum.end_lineno for node, _, enum in aliases):
            lines = source.splitlines(keepends=True)
            moved = []
            removed = set()
            for node, end, _ in aliases:
                moved.extend(lines[node.lineno - 1:end])
                moved.append("\n")
                removed.update(range(node.lineno - 1, end))
            body = [line for index, line in enumerate(lines[cls.lineno - 1:cls.end_lineno], cls.lineno - 1)
                    if index not in removed]
            source = "".join(lines[:cls.lineno - 1] + body + ["\n"] + moved + lines[cls.end_lineno:])
    # Type annotations may also refer to their enclosing class before it exists.
    # Keep the generated module docstring in its required first position.
    tree = ast.parse(source)
    first = tree.body[0] if tree.body else None
    if isinstance(first, ast.Expr) and isinstance(first.value, ast.Constant) and isinstance(first.value.value, str):
        lines = source.splitlines(keepends=True)
        position = sum(map(len, lines[:first.end_lineno]))
        source = source[:position] + "\nfrom __future__ import annotations\n" + source[position:]
    else:
        source = "from __future__ import annotations\n\n" + source
    return HEADER + source


def generated_reference(module_dir: Path) -> str:
    module_dir = module_dir.resolve()
    if not any((module_dir / ("rawtoaces" + suffix)).is_file()
               for suffix in EXTENSION_SUFFIXES):
        raise FileNotFoundError(f"No rawtoaces extension for this interpreter in {module_dir}")
    with tempfile.TemporaryDirectory(prefix="rawtoaces-python-docs-") as temporary:
        stub = Path(temporary) / "rawtoaces.pyi"
        subprocess.run(
            [sys.executable, "-m", "nanobind.stubgen", "-q", "-i",
             str(module_dir), "-m", "rawtoaces", "-o", str(stub)],
            check=True,
        )
        source = stub.read_text(encoding="utf-8")
    result = normalize_source(source)
    ast.parse(result)
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--module-dir", type=Path, required=True,
                        help="Directory containing the freshly built extension.")
    parser.add_argument("--output", type=Path,
                        default=Path(__file__).resolve().parents[1] /
                        "src/docs/api/python/rawtoaces.py")
    parser.add_argument("--check", action="store_true",
                        help="Fail if the checked-in reference differs from regeneration.")
    args = parser.parse_args()
    source = generated_reference(args.module_dir)
    if args.check:
        if args.output.read_text(encoding="utf-8") != source:
            parser.exit(1, f"{args.output} differs from the built binding; regenerate it.\n")
        print(f"Generated reference matches {args.output}")
    else:
        args.output.write_text(source, encoding="utf-8")
        print(f"Regenerated {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
