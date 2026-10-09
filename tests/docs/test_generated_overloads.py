# Copyright Contributors to the rawtoaces Project.
# SPDX-License-Identifier: Apache-2.0

"""Exercise the reference renderer with unmodified generated overload stubs."""

import ast
import importlib.util
import subprocess
import sys
import tempfile
import textwrap
import unittest
from html.parser import HTMLParser
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
EXTENSIONS = ROOT / 'src' / 'docs' / '_ext'
SPEC = importlib.util.spec_from_file_location(
    'generated_overloads', EXTENSIONS / 'generated_overloads.py'
)
RENDERER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(RENDERER)


class RenderedPage(HTMLParser):
    def __init__(self):
        super().__init__()
        self.ids = []
        self.text = []

    def handle_starttag(self, tag, attributes):
        self.ids.extend(value for key, value in attributes if key == 'id')

    def handle_data(self, data):
        self.text.append(data)


class GeneratedOverloadTests(unittest.TestCase):
    def test_signature_preserves_argument_kinds_and_defaults(self):
        declaration = ast.parse(
            'def configure(self, filename: str, /, *, option: bool = False) -> bool: pass'
        ).body[0]
        self.assertEqual(
            RENDERER._signature(declaration),
            '(filename: str, /, *, option: bool=False) -> bool',
        )

    def test_static_method_keeps_first_argument(self):
        declaration = ast.parse(
            '@staticmethod\ndef configure(cls: str, /) -> bool: pass'
        ).body[0]
        self.assertEqual(RENDERER._signature(declaration), '(cls: str, /) -> bool')

    def test_fresh_generated_source_renders_each_overload_once(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / 'source'
            source.mkdir()
            (source / 'conf.py').write_text(
                textwrap.dedent(
                    f'''\
                    import sys
                    sys.path.insert(0, {str(source)!r})
                    sys.path.insert(0, {str(EXTENSIONS)!r})
                    extensions = ['sphinx.ext.autodoc', 'sphinx_autodoc_typehints',
                                  'generated_overloads']
                    project = 'Generated overload test'
                    nitpick_ignore = [('py:class', name) for name in
                                      ['str', 'bool', 'float', 'list',
                                       'collections.abc.Sequence']]
                    '''
                )
            )
            (source / 'index.rst').write_text(
                textwrap.dedent(
                    '''\
                    Generated reference
                    ===================

                    .. autoclass:: rawtoaces.ImageConverter
                       :members:

                    .. autoclass:: rawtoaces.SpectralSolver
                       :members:
                       :inherited-members:
                    '''
                )
            )
            stub = source / 'rawtoaces.py'
            for revision in ('first', 'regenerated'):
                stub.write_text(
                    textwrap.dedent(
                        f'''\
                        from collections.abc import Sequence
                        from typing import overload

                        class TransformSolver:
                            def calculate_transform(self) -> bool:
                                """Inherited transform documentation."""

                        class ImageConverter:
                            @overload
                            def configure(self, input_filename: str, /) -> bool:
                                """{revision} filename overload documentation.

                                :param input_filename: The generated filename field.
                                :return: Whether configuration succeeds.
                                """

                            @overload
                            def configure(self, image_spec: str, *, options: list[str]) -> bool:
                                """{revision} image specification overload documentation.

                                :param image_spec: The generated specification field.
                                :param options: The generated options field.
                                :return: Whether configuration succeeds.
                                """

                        class SpectralSolver(TransformSolver):
                            @overload
                            def find_illuminant(self, type: str) -> bool:
                                """{revision} named illuminant documentation."""

                            @overload
                            def find_illuminant(self, wb_multipliers: Sequence[float]) -> bool:
                                """{revision} multiplier illuminant documentation."""
                        '''
                    )
                )
                # Regeneration changes the source rather than adding callable
                # implementations for typing's dummy overload functions.
                original = stub.read_bytes()
                result = subprocess.run(
                    [
                        sys.executable, '-m', 'sphinx', '-b', 'html', '-E', '-n', '-W',
                        str(source), str(Path(directory) / revision),
                    ],
                    capture_output=True,
                    text=True,
                    check=False,
                )
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                self.assertEqual(stub.read_bytes(), original)
                page = RenderedPage()
                page.feed((Path(directory) / revision / 'index.html').read_text())
                for method in (
                    'rawtoaces.ImageConverter.configure',
                    'rawtoaces.SpectralSolver.find_illuminant',
                    'rawtoaces.SpectralSolver.calculate_transform',
                ):
                    self.assertEqual(page.ids.count(method), 1, method)
                rendered = ''.join(page.text)
                for documentation in (
                    f'{revision} filename overload documentation.',
                    f'{revision} image specification overload documentation.',
                    f'{revision} named illuminant documentation.',
                    f'{revision} multiplier illuminant documentation.',
                    'The generated filename field.',
                    'The generated options field.',
                    'Inherited transform documentation.',
                ):
                    self.assertIn(documentation, rendered)
                self.assertNotIn('typing._overload_dummy', rendered)
                self.assertNotIn('You should not call an overloaded function', rendered)
                self.assertNotIn('configure(self', rendered)
                self.assertIn('configure(input_filename: str, /) -> bool', rendered)
                self.assertIn('configure(image_spec: str, *, options: list[str]) -> bool', rendered)


if __name__ == '__main__':
    unittest.main()
