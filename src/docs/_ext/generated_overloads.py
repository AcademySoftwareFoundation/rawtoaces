# Copyright Contributors to the rawtoaces Project.
# SPDX-License-Identifier: Apache-2.0

"""Render overload signatures and docstrings from the generated Python source.

The generated module keeps nanobind's ``@overload`` declarations. Importing
those declarations exposes ``typing._overload_dummy`` instead of their source
docstrings, so autodoc must read the declarations without executing them.
"""

import ast
import copy
import sys
import tokenize
from functools import lru_cache
from pathlib import Path

from sphinx.ext.autodoc import MethodDocumenter
from sphinx.util.docstrings import prepare_docstring


@lru_cache(maxsize=8)
def _source_tree(path, modified_ns):
    with tokenize.open(path) as source:
        return ast.parse(source.read(), filename=path)


def _decorator_name(decorator):
    if isinstance(decorator, ast.Name):
        return decorator.id
    if isinstance(decorator, ast.Attribute):
        return decorator.attr
    return None


def _overloads(module_name, object_path):
    if module_name != 'rawtoaces' or not object_path:
        return []
    module = sys.modules.get(module_name)
    source_path = getattr(module, '__file__', None)
    if not source_path or Path(source_path).suffix != '.py':
        return []
    source_path = Path(source_path)
    tree = _source_tree(str(source_path), source_path.stat().st_mtime_ns)
    body = tree.body
    for part in object_path[:-1]:
        owner = next(
            (node for node in body if isinstance(node, ast.ClassDef) and node.name == part),
            None,
        )
        if owner is None:
            return []
        body = owner.body
    return [
        node
        for node in body
        if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef))
        and node.name == object_path[-1]
        and any(_decorator_name(decorator) == 'overload' for decorator in node.decorator_list)
    ]


def _signature(declaration):
    arguments = copy.deepcopy(declaration.args)
    decorators = {_decorator_name(item) for item in declaration.decorator_list}
    positional = arguments.posonlyargs or arguments.args
    if 'staticmethod' not in decorators and positional and positional[0].arg in {'self', 'cls'}:
        positional.pop(0)
        # Defaults belong to the trailing positional arguments.
        count = len(arguments.posonlyargs) + len(arguments.args)
        arguments.defaults = arguments.defaults[-count:] if count else []
    signature = f'({ast.unparse(arguments)})'
    if declaration.returns is not None:
        signature += f' -> {ast.unparse(declaration.returns)}'
    return signature


class GeneratedOverloadDocumenter(MethodDocumenter):
    """Keep overload documentation attached to one canonical method target."""

    priority = MethodDocumenter.priority + 1

    def format_signature(self, **kwargs):
        declarations = _overloads(self.modname, self.objpath)
        if declarations:
            return '\n'.join(_signature(declaration) for declaration in declarations)
        return super().format_signature(**kwargs)

    def get_doc(self):
        declarations = _overloads(self.modname, self.objpath)
        if not declarations:
            return super().get_doc()
        tab_width = self.directive.state.document.settings.tab_width
        docstrings = []
        for declaration in declarations:
            # Each overload owns its parameter fields and source documentation.
            # A rubric associates that prose with the corresponding signature.
            lines = [
                f'.. rubric:: ``{declaration.name}{_signature(declaration)}``',
                '',
            ]
            docstring = ast.get_docstring(declaration)
            if docstring:
                lines.extend(prepare_docstring(docstring, tabsize=tab_width))
            docstrings.append(lines)
        return docstrings


def setup(app):
    app.setup_extension('sphinx.ext.autodoc')
    app.add_autodocumenter(GeneratedOverloadDocumenter, override=True)
    return {'version': '1.0', 'parallel_read_safe': True, 'parallel_write_safe': True}
