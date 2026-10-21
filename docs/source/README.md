# Project Overview

BeschleunigerBallett is a renderer and graphics-engine playground for Vulkan, a companion Rust WebGPU renderer, modern CMake, testing, packaging and Rust integration. These pages cover the build and maintenance workflow around the engine, not only its rendering features.

## What is in Scope?

- Vulkan renderer (C++23 modules) with forward + deferred rasterization, ray tracing, path tracing, PBR, cascaded shadow maps (consumed by both lighting paths), skybox, volumetric clouds, OBJ and glTF 2.0/GLB loading, and mip mapping
- VMA-backed memory, fence-synced uploads with a persistent staging buffer, a persisted `VkPipelineCache`, and fail-fast Vulkan error handling (exceptions are disabled project-wide)
- Companion Rust WebGPU renderer (`third_party/OxidANT/crates/webgpu_renderer`): glTF 2.0/GLB, PBR + IBL, CSM, SSAO, bloom, skinning, animations, LOD — runs natively and in the browser; shares Slang shader sources with the C++ Vulkan renderer (see [`shader-sharing.md`](shader-sharing.md))
- Tooling: CMake presets, CI, code coverage, benchmarking, fuzzing (targets for OBJ and glTF parsing, texture decoding, the shader-file reader and the scene config), packaging, Sphinx, Doxygen, Graphviz
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
| `docs/source/` | This site: the guides, roadmaps, audits and pipeline notes; the Docs table in `AGENTS.md` lists them |
| `docs/packaging/` | The Linux desktop entry and AppStream file the packages install |
| `Documents/` | Generated PDF and other reference artifacts |
| `third_party/` | Third-party dependencies and submodules |

## Supported Environments

- C++23 and C17 toolchains
- CMake 4.1 or newer
- Vulkan SDK 1.4 compatible environment for Vulkan builds
- A Rust toolchain: every build links OxidANT's cxx bridge through Corrosion

## Roadmap

Keep the renderer and tooling healthy on Linux and Windows; expand test, fuzz and performance
coverage; keep documenting the build, packaging and API workflows; keep generated reference
material easy to reproduce locally.

All open work is in [`BACKLOG.md`](https://github.com/Kataglyphis/BeschleunigerBallett/blob/develop/BACKLOG.md):
sized commitments first, then unsized ideas and recurring chores (perf tests, periodic
profile/TSan/sync-validation runs).

## Where to Go Next

- [Getting Started](getting_started.md): clone, configure, build, run
- [Documentation Workflow](documentation_workflow.md): Sphinx, Doxygen, Graphviz
- [Literature](literature.md): the tutorials and papers behind the renderer
- **Graphviz Include Graphs** (`graphviz_files.rst`): the stable entry page for generated diagrams
- **API Reference** (`api/library_root.rst`): the C++ API landing page; once Doxygen XML has been generated, the generated Exhale output replaces it

The last two pages are **written by the docs build** (`graphviz_generator.py`, and exhale or
the placeholder `conf.py` writes when no Doxygen XML is present), so they exist in a built
tree, not in the checkout. `index.rst` links them there; this page only names them.
