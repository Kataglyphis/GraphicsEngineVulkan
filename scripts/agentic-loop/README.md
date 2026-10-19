# Agentic loop — the BeschleunigerBallett half

A planner/executor loop that drains `BACKLOG.md` autonomously: an expensive
planner writes tasks, a cheap executor implements them, and builds, tests and
quality gates run on a cadence.

**The loop's logic is not in this directory.** It lives in the ANTfrastructure
submodule: `windows/scripts/modules/WindowsAgenticLoop.Common.psm1` and
`linux/scripts/lib/agentic-loop.sh`. Here is the thin consumer half: one
config, two runners, two prompt overlays. Everything generic (queue discipline,
the blocked-task protocol, model tiering, retries, build-failure fixing) is
documented upstream and not repeated here: this file used to restate it, and
the restatement went stale.

## Upstream docs

| Topic | Doc |
| --- | --- |
| Module API, `Invoke-AgenticLoop` parameters, the config-key table (including `promptOverlays`), prerequisites, env overrides, usage examples | [`windows-agentic-loop.md`](../../third_party/ANTfrastructure/docs/windows-agentic-loop.md) |
| Build-matrix entry fields, sanitizer env vars, cycling order, full-matrix sweeps | [`agentic-loop-build-matrix.md`](../../third_party/ANTfrastructure/docs/agentic-loop-build-matrix.md) |
| What a consumer owns, what stays upstream, and the `- [ ]` / `- [b]` / `- [x]` backlog protocol | [`templates/README.md`](../../third_party/ANTfrastructure/shared/agentic-loop/templates/README.md) |

## What this directory owns

| File | Purpose |
| --- | --- |
| [`AgenticLoop.config.json`](AgenticLoop.config.json) | Engine choice, model IDs, cadences, timeouts, build matrix, build/test/quality commands |
| [`Invoke-AgenticLoop.ps1`](Invoke-AgenticLoop.ps1) | Windows runner — resolves the module, loads the config, calls `Invoke-AgenticLoop` |
| [`Run-AgenticLoop.sh`](Run-AgenticLoop.sh) | Linux runner — sources the library, maps flags onto the env vars it reads, calls `run_agentic_loop` |
| [`prompts/planner-overlay.md`](prompts/planner-overlay.md) | Project delta appended to ANTfrastructure's shared planner system prompt |
| [`prompts/executor-overlay.md`](prompts/executor-overlay.md) | Project delta appended to ANTfrastructure's shared executor system prompt |

Both runners are watched copies of the upstream templates
([`shared/agentic-loop/templates/`](../../third_party/ANTfrastructure/shared/agentic-loop/templates/README.md)):
`agentic-loop-sh` and `agentic-loop-ps1` in
[`.antfrastructure-shared.manifest`](../../.antfrastructure-shared.manifest), compared in body mode by
`sync-shared-config.sh --check` in the lint gates. Only the leading header differs, and the Bash
runner's `--help` range (`head -29 | tail -28`), which follows its header's length. Re-copy them
after an upstream change; never edit the body here. Hard-coding prompt text or a build-config list
in a runner let the Windows and Linux copies drift apart once already.

The two overlays are the **only** prompt text this repo owns. At start-up the
loop composes ANTfrastructure's shared role prompt plus the overlay into one text
for both engines: `claude` gets a temp file behind
`--append-system-prompt-file`; `opencode` takes no prompt file on its command
line, so it gets a **generated** `.opencode/agents/<role>.md`. `.opencode/agents/`
is gitignored and never edited by hand; why a tracked copy is the wrong cure for
prompt drift:
[adopting-in-a-new-project.md § 4](../../third_party/ANTfrastructure/docs/adopting-in-a-new-project.md#4-the-agentic-loop).

[`.opencode/commands/`](../../.opencode/commands) stays hand-written and tracked:
TUI commands, not role prompts. They give the OpenCode TUI `/plan`,
`/execute`, `/build <preset>`, `/test` and `/quality` for driving single loop
phases interactively; `/plan` and `/execute` include the hub's task prompts
(`shared/agentic-loop/prompts/`) rather than a copy. The loop refuses OpenCode
v1, which scoop and `opencode-ai` still install: install v2 and authenticate
under it as
[`windows-agentic-loop.md` § opencode v2](../../third_party/ANTfrastructure/docs/windows-agentic-loop.md#opencode-v2)
says.

## Running it

```pwsh
pwsh -File .\scripts\agentic-loop\Invoke-AgenticLoop.ps1
```

```bash
./scripts/agentic-loop/Run-AgenticLoop.sh
```

`-Engine`, `-DryRun`, `-MaxIterations`, `-SkipBuild`, `-SkipTests`,
`-SkipQuality`, `-PlannerOnly`, `-ExecutorOnly` (`--kebab-case` on the Bash
side) map one-to-one onto the module parameters in
[`windows-agentic-loop.md`](../../third_party/ANTfrastructure/docs/windows-agentic-loop.md).

## What this repo configures

The config restates several upstream defaults on purpose, to pin them against
upstream drift; the bullets say which values deviate. Key-by-key reference:
[`windows-agentic-loop.md`](../../third_party/ANTfrastructure/docs/windows-agentic-loop.md).

- `engine: "claude"` (upstream default: `opencode`): `plannerModel: claude-opus-5`,
  `plannerFallbackModel: claude-fable-5` for when Opus is overloaded,
  `executorModel: claude-sonnet-5`. The `opencode` engine stays configured as an
  alternative (GLM 5.2 planner, DeepSeek v4 Flash executor).
- `fullMatrixEveryNIterations: 5` (default 0 = no sweeps): the full-matrix
  sweep cadence. `buildEveryNTasks: 3`, `qualityEveryNTasks: 5` and
  `refactorEveryNIterations: 3` pin the defaults.
- `plannerTimeoutSeconds: 1800`, `executorTimeoutSeconds: 3600` (default 0 =
  no timeout): per-role wall-clock timeouts; the executor value also covers
  the build fixer. The generic `timeoutSeconds: 1200` is dead while both are
  set: `Get-AgentTimeoutForRole` falls through to it only for a role whose own
  timeout is 0.
- `agentRetryDelaySeconds: 30` (default 20). `agentRetries: 2`,
  `fixBuildFailures: true` and `maxConsecutiveBuildFailures: 3` pin the
  defaults.
- `plannerAllowedTools: "Read Glob Grep Edit(BACKLOG.md) Bash(git:*) PowerShell(git:*)"`:
  the planner reads the whole tree but writes only the backlog. The executor
  runs with `bypassPermissions` (the upstream default, restated as a trust
  statement: this is a trusted repo).
- `build.windowsTestCommand` / `build.linuxTestCommand` (no upstream default):
  the fallback test command for a matrix entry without its own `testCommand`.
  Both run the debug build's ctest; see the matrix bullet for the consequence.
- `logging.logDir: logs/agentic-loop` (pins the default): one timestamped log
  per run, with all agent, build, test and quality output.
- The `buildMatrix` maps this repo's presets to their build dirs and `ctest`
  commands: `clangcl-debug` (ASAN), `clangcl-profile`, `clangcl-release` on
  Windows; `linux-debug-asan-clang`, `linux-debug-tsan-clang`,
  `linux-profile-clang`, `linux-release-clang` on Linux. The release entries
  carry `testCommand: null`, which does **not** skip tests: the module falls
  back to `build.windowsTestCommand` / `build.linuxTestCommand`, both set here,
  so the release lane re-runs the debug build's ctest. `null` means "no tests"
  only when both fallback keys are unset; to make the release lanes build-only,
  null those two keys as well.

## Build and quality commands

Windows builds never run CMake on the host. They go through the Stevedore
container script
[`scripts/windows/Build-Windows-Container.ps1`](../windows/Build-Windows-Container.ps1),
whose configuration name maps to a preset via
[`Build-Windows.config.psd1`](../windows/Build-Windows.config.psd1). Stevedore
setup and service recovery:
[`windows-stevedore-and-docker.md`](../../third_party/ANTfrastructure/docs/windows-stevedore-and-docker.md).

Linux builds go through
[`scripts/linux/cmake-configure-build.sh`](../linux/cmake-configure-build.sh),
natively or in a Rancher Desktop container.

The Windows quality gate is
`Build-Windows.ps1 -Configurations clangcl-debug -SkipBuild -SkipTests -SkipPerfTests -SkipMsix`.
`-SkipBuild` clears the selected configurations, so it runs only the
report-only clang-format check: not clang-tidy, a step of the `clangcl-debug`
build, and not cmake-format, which runs only with `-ApplyFormat` (see
[`docs/source/code-quality.md`](../../docs/source/code-quality.md)). On Linux it is
[`scripts/linux/run-static-analysis-format.sh`](../linux/run-static-analysis-format.sh).

## The prompt overlays

The overlays are declared once, in the config's top-level `promptOverlays`
block (engine-agnostic: both engines get the same composition); composition
and delivery are upstream in
[`agentic-loop-build-matrix.md`](../../third_party/ANTfrastructure/docs/agentic-loop-build-matrix.md).
They carry only what is true of *this* renderer:

- [`prompts/planner-overlay.md`](prompts/planner-overlay.md): which `BACKLOG.md`
  heading a task belongs under, which preset a task names (`clangcl-debug`
  for iteration, `clangcl-profile` for benchmarks, `clangcl-release` for
  packaging), and the conventions a proposed task must never violate: no
  exceptions, `PipelineBuilder` for pipelines, VMA for buffer/image memory.
- [`prompts/executor-overlay.md`](prompts/executor-overlay.md): the exact build
  commands, why a container build needs a container-native `--build-dir`, why
  Windows tests run the delivered executable from the repo root instead of
  `ctest --test-dir` (the container build's CTest metadata carries container
  paths), and the stale-SPIR-V trap after touching a shader.
