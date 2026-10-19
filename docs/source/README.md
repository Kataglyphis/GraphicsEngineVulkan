# Project Overview

BeschleunigerBallett is a renderer and graphics-engine playground for experimenting with Vulkan, a companion Rust WebGPU renderer, modern CMake, testing, packaging, and optional Rust integration. This documentation focuses on the build and maintenance workflow around the engine, not only on the rendering features themselves.

## What is in Scope?

- Vulkan renderer (C++23 modules) with forward + deferred rasterization, ray tracing, path tracing, PBR, cascaded shadow maps (consumed by both lighting paths), skybox, volumetric clouds, OBJ and glTF 2.0/GLB loading, and mip mapping
- VMA-backed memory, fence-synced uploads with a persistent staging buffer, a persisted `VkPipelineCache`, and fail-fast Vulkan error handling (exceptions are disabled project-wide)
- Companion Rust WebGPU renderer (`third_party/OxidANT/crates/webgpu_renderer`): glTF 2.0/GLB, PBR + IBL, CSM, SSAO, bloom, skinning, animations, LOD — runs natively and in the browser; shares Slang shader sources with the C++ Vulkan renderer (see `docs/shader-sharing.md`)
- Tooling around CMake presets, CI, code coverage, benchmarking, fuzzing (targets for OBJ and glTF parsing, texture decoding, the shader-file reader and the scene config), packaging, Sphinx, Doxygen, and Graphviz
- Linux and Windows as the primary development targets

## Repository Map

| Path | Purpose |
| --- | --- |
| `Src/` | Engine and renderer source code |
| `Resources/` | Shaders, textures, test models, and `assets.lock`, which pins the scenes (see *Scenes* in [Getting Started](getting_started.md)) |
| `cmake/` | This project's CMake policy on top of ANTfrastructure's modules |
| `scripts/linux/` | Linux build, test, coverage, analysis, and docs helpers |
| `scripts/windows/` | Windows build, run and validation helpers |
| `scripts/agentic-loop/` | Planner/executor loop config, runners and prompt overlays |
| `Test/` | Tests |
| `docs/` | Engineering docs: roadmaps, audits, pipeline notes; see the Docs table in `AGENTS.md` for the full inventory |
| `docs/source/` | Hand-written Sphinx pages |
| `Documents/` | Generated PDF and other reference artifacts |
| `third_party/` | Third-party dependencies and submodules |

## Supported Environments

- C++23 and C17 toolchains
- CMake 4.1 or newer
- Vulkan SDK 1.4 compatible environment for Vulkan builds
- Optional Rust toolchain for the experimental Rust-enabled build path

## Roadmap

- Keep the renderer and tooling foundations healthy on Linux and Windows
- Expand test, fuzz, and performance coverage
- Continue documenting build, packaging, and API workflows
- Keep generated reference material easy to reproduce locally

All open work is consolidated in [`BACKLOG.md`](https://github.com/Kataglyphis/BeschleunigerBallett/blob/develop/BACKLOG.md):
sized commitments first, then unsized ideas and recurring chores (perf tests, periodic
profile/TSan/sync-validation runs).

## Where to Go Next

- Read [Getting Started](getting_started.md) for clone, configure, build, and run instructions
- Read [Documentation Workflow](documentation_workflow.md) for Sphinx, Doxygen, and Graphviz details
- Read [Literature](literature.md) for the tutorials and papers behind the renderer
- Open **Graphviz Include Graphs** (`graphviz_files.rst`) for the stable entry page for generated diagrams
- Open **API Reference** (`api/library_root.rst`) for the C++ API landing page

The last two pages are WRITTEN BY THE DOCS BUILD - `graphviz_generator.py`, and
exhale or the placeholder `conf.py` writes when no Doxygen XML is present - so
they exist in a built tree, not in the checkout. `index.rst` links them there,
which is why they are named here rather than linked.

If Doxygen XML has already been generated, the API page is automatically replaced with the generated Exhale output.