# Agent guidance

- Read `CONTRIBUTING.md` and `docs/AI_Policy.md` first.
- Add the project copyright and SPDX license header to new source files.
  Generated files must receive that header from their generator.
- Do not hand-edit `src/docs/api/python/rawtoaces.py`. Update binding docstrings,
  rebuild `rawtoaces_bindings`, and run
  `python build_scripts/generate_python_docs.py --module-dir build/src/bindings`.
- Verify generated Python documentation with the same command plus `--check`.
- After documentation changes, rebuild the complete local site with Sphinx
  `-E -a`; inspect navigation when changing headings or toctrees.
