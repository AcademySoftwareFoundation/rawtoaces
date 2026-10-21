# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the rawtoaces project.

# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import os
import sys
from pathlib import Path

docs_dir = Path(__file__).resolve().parent
repo_root = docs_dir.parents[1]
sys.path.insert(0, str(repo_root / 'build_scripts'))
from build_docs import check_html, documentation_version, run_doxygen

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'rawtoaces'
copyright = '2024, Contributors to the rawtoaces Project'
author = 'Contributors to the rawtoaces Project'

# Label the documentation checkout, independently of library release metadata.
version = release = documentation_version(repo_root)
strict_docs = os.environ.get('RAWTOACES_DOCS_STRICT', '1') == '1'
nitpicky = strict_docs

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'breathe',
    'sphinx.ext.autodoc',
    'sphinx.ext.autosummary',
    'sphinx.ext.napoleon',
    'sphinx.ext.intersphinx',
    'sphinx.ext.viewcode',
    'sphinx_autodoc_typehints',
    'sphinx_rtd_theme',
    'sphinx_multiversion',
    'myst_parser',
    'sphinx_tabs.tabs',
    'enum_tools.autoenum',
    'generated_overloads',
]

autodoc_mock_imports = [
    "OpenImageIO",
    "OpenImageIO.OpenImageIO",
]

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

# The suffix(es) of source filenames.
source_suffix = {
    '.rst': 'restructuredtext',
    '.md': 'markdown',
}

# The master toctree document.
master_doc = 'index'

# Autodoc needs to import the rawtoaces module to read the docstrings.
# Adding the path to the module stub to the search path.
sys.path.insert(0, str(docs_dir / 'api' / 'python'))
sys.path.insert(0, str(docs_dir / '_ext'))

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']
html_css_files = ['css/tables.css', 'css/tabs.css']

# Keep one language example visible. The extension otherwise lets a second
# click on the selected tab hide every panel.
sphinx_tabs_disable_tab_closing = True

html_theme_options = {
    'logo_only': False,
    'prev_next_buttons_location': 'bottom',
    'style_external_links': False,
    'collapse_navigation': False,
    'sticky_navigation': True,
    'navigation_depth': 4,
    'includehidden': True,
    'titles_only': False,
}

# -- Breathe configuration ---------------------------------------------------
# https://breathe.readthedocs.io/en/latest/

# Check if we're building on Read the Docs
read_the_docs_build = os.environ.get('READTHEDOCS', None) == 'True'

# Path to Doxygen XML output
# When building locally with CMake, this will be in the build directory
# When building on RTD, we run Doxygen from conf.py
provided_xml = os.environ.get('RAWTOACES_DOCS_DOXYGEN_XML')
if provided_xml:
    breathe_projects = {'rawtoaces': provided_xml}
elif read_the_docs_build:
    # Apply the same warning check used by the direct documentation build.
    doxygen_output = repo_root / 'src' / 'doxygen'
    run_doxygen(repo_root, doxygen_output, release, strict=strict_docs)
    breathe_projects = {'rawtoaces': str(doxygen_output / 'xml')}
else:
    # Local build - assume CMake has run Doxygen
    breathe_projects = {'rawtoaces': '_build/doxygen/xml'}

breathe_default_project = 'rawtoaces'
breathe_default_members = ('members', 'undoc-members')

# -- Intersphinx configuration -----------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/extensions/intersphinx.html

intersphinx_mapping = {
    'python': ('https://docs.python.org/3', None),
    'openimageio': ('https://openimageio.readthedocs.io/en/stable', None),
}

# These namespace/helper names and qualified Python classes are absent from
# OpenImageIO's inventory. Keep exceptions exact so new missing APIs still fail.
nitpick_ignore = [
    ('cpp:identifier', 'OIIO'),
    ('cpp:identifier', 'OIIO::ImageBuf'),
    ('cpp:identifier', 'OIIO::ParamValueList'),
    ('cpp:identifier', 'OIIO::ArgParse'),
    ('py:class', 'OpenImageIO.ImageBuf'),
    ('py:class', 'OpenImageIO.ImageSpec'),
    ('py:class', 'OpenImageIO.ParamValueList'),
    ('py:class', 'OpenImageIO.TypeDesc'),
]

# -- MyST Parser configuration -----------------------------------------------
# https://myst-parser.readthedocs.io/en/latest/

myst_enable_extensions = [
    'colon_fence',
    'deflist',
]

# Match GitHub heading fragments in the included contribution guide.
myst_heading_anchors = 3


def validate_generated_html(app, exception):
    if exception is None and strict_docs and app.builder.name == 'html':
        from sphinx.errors import ExtensionError
        try:
            check_html(Path(app.outdir))
        except (ValueError, OSError) as error:
            raise ExtensionError(str(error)) from error


def setup(app):
    app.connect('build-finished', validate_generated_html)
