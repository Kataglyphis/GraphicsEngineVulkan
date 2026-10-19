# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

Every push to `main` releases `v<VERSION.txt's version>` once every lane is green (`release.yml`),
with this file's `## [<version>]` section as the release notes, so before a merge raise
`VERSION.txt` and turn `## [Unreleased]` into that section. The notes of v1.4.2 and earlier are on
the GitHub releases themselves.

## [Unreleased]

### Fixed

- **The engine draws on a Raspberry Pi 5** (2026-10-18). Its two fullscreen vertex shaders (`post`,
  `deferred`'s lighting pass) read `SV_VertexID`, which Slang lowers to `gl_VertexIndex -
  gl_BaseVertex`; `gl_BaseVertex` needs draw parameters, and Mesa's v3dv aborted at pipeline creation
  (`Unknown intrinsic ... load_first_vertex`, exit 134). They read `SV_VulkanVertexID` now, the same
  index for their `draw(3, 1, 0, 0)`. The 1.5.2 arm64 package with the two recompiled shaders renders
  on himbeere2's desktop (Debian 13, v3dv).
- **The `.deb` puts the app in the desktop's menu, and ships no GLFW development files** (2026-10-18).
  The desktop entry, the AppStream file and the icons were installed to a relative `usr/share/`, which
  the `.deb`'s `/usr` prefix made `/usr/usr/share/...`, where no desktop looks; they go to `share/`
  now, so the `.deb` has them in `/usr/share/` and CPack's AppImage still finds the entry by name.
  GLFW's install rules put its headers, `libglfw3.a`, its CMake package and `glfw3.pc` into every
  package (`/usr/include/GLFW`, `/usr/lib` in the `.deb`, a clash with `libglfw3-dev`);
  `GLFW_INSTALL` is off, the engine links it statically. `scripts/linux/check-deb-layout.sh` reads
  every lane's `.deb` after packaging (it fails on the 1.5.2 one with all four faults), and its
  synthetic-deb suite runs with Linux x64.

### Changed
- **The README is a short overview** (2026-10-19, owner: every README in the family): header,
  badges, one paragraph, six highlights, the quick start (cloned over https) and one row per guide,
  after ANTfrastructure's README rule. What left it moved into the docs: the scene licence table to
  `docs/source/getting_started.md` § Scenes, the full feature list, the repository map and the
  roadmap to `docs/source/README.md`, the literature list to the new `docs/source/literature.md`,
  and the WSL2 pointer to the troubleshooting section. Contributing, contact and licence are the
  family page's (`docs/repository-family.md` in the hub).
- **The family's ant replaces the old logos** (2026-10-17, owner: one icon in every repository):
  `images/logo.png` in the README, and `Engine_logo.png`, `Engine_logo.bmp` and `faviconNew.ico`,
  which the installers and the Linux desktop entry show. `faviconNew.ico` now holds 16 to 256 px,
  where it had 16 px only, and `Engine_logo.png` is 256 px, the size of the `hicolor/256x256`
  folder it is installed into (it was 200 px).
- **`docs/source/wsl2_vulkan.rst` is in English** (2026-10-19), with every command and fact kept,
  and without the chat assistant's closing offer it ended with. `getting_started.md` clones over
  https, as the family's repositories and submodules do.
- **`docs/LICENSES-README.md` is in English** (2026-10-19). Every licence, pin, version, path and
  date is unchanged; the translation changes no fact.
- **The docs link the hub instead of restating it** (2026-10-19). `AGENTS.md` keeps this repo's
  half of the CI rule, Renovate, the container transports, the sanitizer modules, the branch sync,
  the history rewrite and the riscv64 lane, and links ANTfrastructure (pinned at a447ef3c) for the
  rest; `docs/container-build-caching.md` keeps its measurements. Three stale facts went with it:
  `--apply` rewrites `requirements.txt`, actions and CMake pins too, not only gitlinks;
  microsoft/STL#6294 is closed (#6298, not yet in the image's VC Tools); the agentic loop needs
  OpenCode v2. `/plan` and `/execute` include the hub's task prompts instead of a copy, and
  `doc-dupes-hub.allow` turns the hub's `doc-dupes --against-hub` gate on.
- **The site uses the family theme's `setup_theme()`** (2026-10-19) instead of loading DocumANTation's
  frozen `conf_base.py` by path: Markdown links and GitHub heading anchors resolve on the site,
  links to unpublished repository files go to GitHub, and the logo, favicon and title are the
  family's. The `custom.css` symlink went; `beschleunigerballett.css` keeps the dark hero. The API
  tree view's 176 dead anchors (Exhale links its labels with their case kept, docutils lowercases
  the ids) now point at the ids.
- **The fourteen hand-written guides are on the site** (2026-10-19). `docs/*.md` (path tracing,
  clouds, the shader pipeline, GPU golden testing, …) moved into `docs/source/` and its toctree,
  where Sphinx never read them before. Every link and code pointer to them follows, the
  `BuildIntegrity` checks that read their markers included; their links into the hub go to GitHub.

## [1.5.2] - 2026-10-17

### Added
- **Windows arm64 ships a setup `.exe` too** (2026-10-17), as x64 always did. NSIS has no arm64 stub,
  so it is the same x86 stub, which Windows on Arm runs emulated; the arm64 build used to drop it
  because the hub's arch gate refused that stub. The gate now reports an NSIS installer instead of
  failing it, and the `windows-11-arm` job installs it silently, checks that every installed binary
  is ARM64, starts `bin/GraphicsEngine.exe --version` from the install directory and uninstalls it again
  (`installed-run-command`). The hub moves to 6ea38c48. Owner rule of the same day: every release ships
  Windows arm64 installers next to the x64 ones.

### Changed

- 2026-10-17 — **The scenes are AssetANT release assets, not tracked files.** The fourteen scene
  directories under `Resources/Models` (518 of the 633 MiB HEAD tracked) left the tree for
  [AssetANT](https://github.com/Kataglyphis/AssetANT), one reproducible `<name>.tar.xz` per release.
  `Resources/assets.lock` pins each by SHA-256 and URL, and `cmake/FetchAssets.cmake` extracts
  `crytek-sponza` and `Dinosaurs` into `Resources/Models` at configure time through a per-user cache,
  so the paths the engine loads did not move. `-DKATAGLYPHIS_FETCH_ALL_ASSETS=ON` fetches all
  fourteen, and `cmake -P cmake/FetchAssets.cmake` fetches without a configure. The test fixtures
  (`GltfTest`, `ShadowTest`, `Plane`, `VikingRoom`, `plane.obj`, `suzanne.obj`) and
  `Resources/Textures` stay tracked, and the history keeps every scene: nothing was rewritten.

## [1.5.1] - 2026-10-17

### Fixed

- 2026-10-16 — **The Linux packages start outside the build image.** 1.5.0's `.deb`, tarball and
  AppImage need GCC 16.2's `libstdc++` (`GLIBCXX_3.4.36`, newer than any distro's) and carry
  none; the `.deb` claimed `libstdc++6 (>= 13.1)`. ANTfrastructure's
  `kataglyphis_install_linux_closure` now puts the compiler's runtime in `lib/graphicsengine/`
  together with the image's loader and glibc 2.43, and `bin/GraphicsEngine` is a launcher that
  starts the program through them (`LOADER`), so an older glibc (Debian 13, Ubuntu 24.04) is no
  obstacle: the arm64 `.deb`, tarball and AppImage start on a Debian 13 Raspberry Pi 5 (measured
  2026-10-17). The `.deb` takes only the Vulkan loader from the distro. GraphicsEngine finds its
  resources from the working directory, which the launcher leaves alone.
- 2026-10-16 — **A clean-host job proves it**: after each Linux lane, `ubuntu-26.04` and
  `ubuntu-26.04-arm` install the `.deb` with apt, check every installed ELF with `ldd`, and start
  `GraphicsEngine --version` from the `.deb`, the unpacked tarball and the AppImage.

## [1.5.0] - 2026-10-16

### Changed

- 2026-10-16 — **A push runs Linux x64 and the gates; Linux arm64, Linux riscv64, Windows x64 and
  Windows arm64 run on request** (`on-request.yml`, the ci token in a commit message, or each
  lane's Run workflow button), the family's rule in [ANTfrastructure's
  `branch-and-release-strategy.md`](https://github.com/Kataglyphis/ANTfrastructure/blob/develop/docs/branch-and-release-strategy.md).
- 2026-10-16 — **Every push to `main` releases `v<version>`, and only when all five lanes are
  green** (`release.yml`). It carries the `.deb`, AppImage, flatpak (named per arch) and tarball of
  both Linux arches and the MSI, MSIX and zip of both Windows arches plus x64's installer, gathered
  by `scripts/linux/collect-release-assets.sh`; its suite runs with Linux x64 on every push. The
  Linux packages upload as `linux-packages-<arch>`, since one release run holds both arches.
  `gh workflow run release.yml --ref develop -f dry-run=true` runs it all on `develop` and
  publishes nothing.
