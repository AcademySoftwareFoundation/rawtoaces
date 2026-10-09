# Copyright Contributors to the rawtoaces Project.
# SPDX-License-Identifier: Apache-2.0

import ast
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "build_scripts"))
from generate_python_docs import generated_reference, normalize_source


class GeneratedReference(unittest.TestCase):
    def test_forward_enum_aliases_import_and_keep_their_documentation(self):
        source = '''"""Binding module documentation."""
import enum
class Converter:
    class Settings:
        Custom: Converter.Settings.MatrixMethod = Converter.Settings.MatrixMethod.Custom
        """Documented exported alias."""
        @property
        def matrix_method(self) -> Converter.Settings.MatrixMethod:
            """Binding property documentation."""
        class MatrixMethod(enum.Enum):
            Custom = 1
'''
        normalized = normalize_source(source)
        namespace = {}
        exec(compile(normalized, "generated.py", "exec"), namespace)
        settings = namespace["Converter"].Settings
        self.assertIs(settings.Custom, settings.MatrixMethod.Custom)
        self.assertEqual(namespace["__doc__"], "Binding module documentation.")
        self.assertEqual(settings.matrix_method.fget.__doc__, "Binding property documentation.")
        tree = ast.parse(normalized)
        cls = next(node for node in tree.body if isinstance(node, ast.ClassDef))
        settings_node = cls.body[0]
        alias_index = next(i for i, node in enumerate(settings_node.body) if isinstance(node, ast.AnnAssign))
        self.assertEqual(settings_node.body[alias_index + 1].value.value, "Documented exported alias.")

    def test_overloads_remain_generated_declarations_with_original_docstrings(self):
        source = '''from typing import overload
class Solver:
    @overload
    def find(self, name: str) -> bool:
        """Load from the supplied name."""
    @overload
    def find(self, values: list[float]) -> bool:
        """Match the supplied values."""
'''
        normalized = normalize_source(source)
        cls = next(node for node in ast.parse(normalized).body if isinstance(node, ast.ClassDef))
        self.assertEqual(len(cls.body), 2)
        self.assertTrue(all(ast.unparse(node.decorator_list[0]) == "overload" for node in cls.body))
        self.assertEqual([ast.get_docstring(node) for node in cls.body],
                         ["Load from the supplied name.", "Match the supplied values."])

    def test_missing_build_directory_cannot_use_an_installed_extension(self):
        with tempfile.TemporaryDirectory() as temporary:
            with self.assertRaises(FileNotFoundError):
                generated_reference(Path(temporary))


if __name__ == "__main__":
    unittest.main()
