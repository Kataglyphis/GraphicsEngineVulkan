# Documentation Workflow

## Documentation Layers

1. hand-written pages in `docs/source/`
2. optional C++ API reference generated from Doxygen XML through Breathe and Exhale
3. optional Graphviz include diagrams rendered from SVG files under `docs/source/_static/`

The hand-written pages must build even when the generated artifacts are missing.

## Build the HTML Docs Locally

### Windows

```pwsh
.\.venv\scripts\python.exe -m pip install -r requirements.txt
.\.venv\scripts\python.exe -m sphinx -M html docs/source docs/build -E
```

### Linux

```bash
python -m venv .venv
source .venv/bin/activate
python -m pip install -r requirements.txt
python -m sphinx -M html docs/source docs/build -E
```

The site lands in `docs/build/html`.

## Optional C++ API Reference

`conf.py` enables the API reference when it finds a Doxygen XML directory containing `index.xml` in one of these places:

- the path from `KATAGLYPHIS_DOXYGEN_XML_DIR`
- `build/build/xml`
- `build/xml`
- `build-clangcl-debug/xml`
- `build-clangcl-release/xml`
- `build-clangcl-profile/xml`

Without one, `conf.py` writes a placeholder `api/library_root.rst` that says so, and the rest of the docs still builds.

## Graphviz Include Graphs

`scripts/linux/docs-build-web.sh` automates the Graphviz flow:

1. ensure the Python environment and dependencies
2. copy generated SVG files into `docs/source/_static/`
3. run `docs/source/graphviz_generator.py`
4. build the HTML docs

By hand, once the SVG files are in place:

```bash
cd docs/source
python graphviz_generator.py
```

With no SVGs the generated page is a placeholder; the docs build does not fail.

## Authoring Guidelines

- Keep the repository root README a short overview: what the project is, the quick start, and one row per guide ([the family's README rule](https://github.com/Kataglyphis/ANTfrastructure/blob/develop/docs/repository-family.md#the-readme-rule)). Everything else has a page here or in `docs/`.
- Keep Sphinx pages task-focused and stable.
- Prefer relative links inside the docs tree.
- Treat generated artifacts as optional unless a page is about generated reference material.
- A change to a build or packaging flow updates the affected Sphinx page in the same change, and the root README only when its quick start or highlights change.
