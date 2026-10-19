# Container Build Caching & Incremental Builds

How builds in *this repo* stay fast; `AGENTS.md` links here. The general
findings live in
[`ANTfrastructure / windows-container-build-performance.md`](../third_party/ANTfrastructure/docs/windows-container-build-performance.md)
and apply to any project built in that image: the reusable-container pattern
and its safety rails, why sccache cannot cache a C++23 modules build, why a
named volume cannot be a CMake build directory, the Windows path-limit trap
that silently truncates tar transfers, the Dev Drive bind-mount restriction
and the `docker exec` entrypoint bypass.

## Measured here (2026-07-19, `clangcl-debug`)

| Scenario | Build step | Wall clock |
| --- | --- | --- |
| Fresh container (`-FreshContainer`, host tree seeded) | 327.9 s | 364 s |
| Incremental, a few files changed | ~14 s | ~50 s |
| **No source changes** | **9.6 s** | **44 s** |
| Previously: fresh container every build | — | 352–484 s |

The wall clock exceeds the build step because sources stream in and artifacts
stream back out on every run; that transport, not compilation, is now the
dominant term. The incrementally built binaries passed 21/21 commit tests and
18/18 Pester tests at the time.

**The tar-pipe is still the right trade, and the script's default.** A bind
mount removes the transport but measured **slower** (2026-07-19): 32.7 s
ninja / 159 s wall for the same no-change build, because the build tree then
sits on the Dev Drive and every file operation crosses the `bindFlt` filter.
`-UseBindMount` opts in; measure before switching. Full measurements and when
the trade inverts:
[ANTfrastructure](../third_party/ANTfrastructure/docs/windows-container-build-performance.md).

## What is actually cached

**Build state, not compiler output.** One reused container,
`bb-build-persistent`, keeps ninja's dependency graph, the C++23 module BMIs
and the object files where they are. sccache is wired to a persistent volume
but contributes nothing here (0.00 % hit rate on a modules build; the
measurement is in ANTfrastructure).

## Repo-specific wiring

- **`KATAGLYPHIS_KEEP_BUILD_ROOT=1`** is passed into the container; without it
  `Invoke-CmakeConfigureAndBuild` wipes the build root before configuring
  (`Remove-BuildRootSafe`). Nothing sets it on the host, so host builds are
  unaffected.
- **The `cargo/` subtree is excluded in both directions.** The Rust bridge
  (corrosion/cxx) generates cxxbridge paths past the Windows path limit, and
  one such failure aborts the *whole* tar transfer; the exclusion turned a
  partial transfer into a working one. Cargo artifacts rebuild cheaply.
- **Outbound copies only what the host runs**: executables, debug info,
  `compile_commands.json`, logs. Selection is by *exclusion* because `tar`
  does not expand globs: a glob-based version reported success while copying
  nothing, and looked correct only because stale artifacts were already on
  the host.

## Delivery verification

A green build is not proof that anything was produced or delivered; both
halves have failed silently here (a build cut off before linking looked
successful, and the outbound transfer once copied nothing).
`Build-Windows-Container.ps1` compares the executables in the container with
those that reached the host and **fails the build** if the container produced
none or any did not arrive. Do not "fix" that check. It tests existence, not
timestamps, on purpose: a no-change build does not relink, so the executables
are legitimately older than the run.

## Gotchas

- The script takes `-Configurations` (comma-separated), **not** `-Preset`. The
  wrong parameter silently builds the default set, all three of
  `clangcl-debug,clangcl-profile,clangcl-release`, which reads as a slow build
  rather than an error.
- Host `cmake` (3.29) cannot read this repo's `CMakePresets.json`
  (`version: 10`); only the container's newer CMake can.
- **A file deleted on the host no longer keeps building in the container.**
  It did (reproduced 2026-07-19: tar extracts over the old tree and never
  removes a file). Since ANTfrastructure `637d4a91` (2026-08-02)
  `Invoke-ContainerBuild` prunes a reused container before the sources stream
  in: every top-level directory under `C:\ws` except `build*` and the
  `KeepDirs` (`logs`, `sccache-local`) is deleted. Top-level files are only
  overwritten, so a file deleted at the repo root still needs
  `-FreshContainer`, as does any C++23 module-interface change (the
  fresh-container rule in [`gpu-golden-testing.md`](gpu-golden-testing.md)).
- Reset an incremental build that behaves strangely:
  `docker rm -f bb-build-persistent`, or
  `Remove-Item -Recurse -Force build-clangcl-debug` for a cold but clean host
  tree.
