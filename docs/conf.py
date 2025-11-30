# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import subprocess
import os

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'RAWtoACES'
copyright = '2024, Contributors to the rawtoaces Project'
author = 'Contributors to the rawtoaces Project'

# The version info for the project
version = '2.0'
release = '2.0.0'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'breathe',
    'sphinx.ext.autodoc',
    'sphinx.ext.intersphinx',
    'sphinx.ext.viewcode',
    'sphinx_rtd_theme',
    'myst_parser',
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

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']

html_theme_options = {
    'logo_only': False,
    'display_version': True,
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
if read_the_docs_build:
    # Run Doxygen when building on Read the Docs
    subprocess.call('cd .. && doxygen docs/Doxyfile', shell=True)
    breathe_projects = {'rawtoaces': '../doxygen/xml'}
else:
    # Local build - assume CMake has run Doxygen
    breathe_projects = {'rawtoaces': '_build/doxygen/xml'}

breathe_default_project = 'rawtoaces'
breathe_default_members = ('members', 'undoc-members')

# -- Intersphinx configuration -----------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/extensions/intersphinx.html

intersphinx_mapping = {
    'python': ('https://docs.python.org/3', None),
}

# -- MyST Parser configuration -----------------------------------------------
# https://myst-parser.readthedocs.io/en/latest/

myst_enable_extensions = [
    'colon_fence',
    'deflist',
]
