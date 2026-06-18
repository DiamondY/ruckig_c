# Ruckig C Rewrite

This repository contains a pure C99 rewrite of the Ruckig Community trajectory generator. The `ruckig_c` library does not link to or require a C++ runtime. The documentation entry point is `docs/index.md`; current implementation scope is defined by this README, the public header, `docs/current/roadmap.md`, and `docs/current/upstream_baseline_policy.md`. `docs/historical/c_rewrite_execution_plan.md` is retained as a historical execution plan. The original C++ implementation under `original/ruckig-main` is kept unchanged and is used only as a frozen oracle in tests, not as part of the C library runtime.

The rewrite targets Ruckig Community `0.17.3`. The upstream project in
`original/ruckig-main` is MIT licensed, and this repository keeps that license
at the root in `LICENSE`.

## Current Status

Implemented and covered by fixed C/oracle tests plus deterministic random oracle stress:

- C99 library target `ruckig_c`.
- Public opaque C ABI in `include/ruckig_c/ruckig.h`.
- Lifecycle, input/output/trajectory accessors, validation, optional min velocity/min acceleration/minimum duration setters.
- First-order position trajectories.
- Second-order position trajectories.
- Second-order velocity trajectories.
- Third-order velocity trajectories.
- Third-order position trajectories, including blocked intervals and brake pre-trajectory handling.
- Multi-DoF synchronization modes: `Time`, `TimeIfNecessary`, `Phase`, and `None`.
- Per-DoF control-interface and synchronization overrides.
- Continuous and discrete duration handling.
- Directional min velocity/min acceleration limits.
- Disabled DoF behavior.
- Offline `ruckig_calculate`, online `ruckig_update`, `ruckig_output_pass_to_input`.
- Trajectory duration, independent minimum durations, sampling, position extrema, and first-time-at-position helpers.
- `0.4.x` waypoint-aware C ABI, global position bounds, per-section
  constraints, intermediate duration queries, and local coupled waypoint
  optimizer.
- `v0.5.0` tracking C ABI and local Fast-mode online/offline tracking
  implementation.
- `v0.6.0` bounded local `Optimized` tracking MVP, including deterministic
  candidate search, online lookahead update, offline sliding-window sequence
  calculation, Fast fallback diagnostics, and Python/Rust prototype smoke
  coverage.
- `v0.7.0` Optimized tracking strategy and diagnostics stabilization, adding
  Stable/Balanced/Aggressive controls, public diagnostics snapshots, stricter
  deterministic quality gates, and 100k-seed tracking random stress evidence.
- `0.7.0-alpha.3` test coverage audit evidence, adding a local
  LLVM coverage runner and original Community test/example coverage matrix.
- `0.7.0-alpha.4` targeted solver branch coverage evidence, adding
  fixed oracle cases and a lightweight solver branch CTest gate for the lowest
  coverage target-solver files from the alpha.3 audit.
- `0.7.0-readiness` audit evidence, rerunning the full local
  release-readiness gate set against the current 172-symbol strategy preset
  and diagnostics ABI candidate before stable closeout.
- `v0.8.0` visualization/gallery evidence stabilization, adopting the local
  Matplotlib PNG gallery and verifier while keeping the `v0.7.0` 172-symbol
  public C ABI unchanged.
- `0.9.0-alpha` tracking quality baseline evidence, adding a deterministic
  `--tracking-random-audit` test-runner selector, routine audit CTest, fixed
  representative diagnostics cases, and local 10k/100k/1M fallback audit
  summaries without changing public C ABI.
- `0.9.0-alpha.2` tracking Optimized evaluator quality hardening evidence,
  tuning the bounded local evaluator, adding private candidate-family
  attribution, adding `--tracking-quality-hardening`, and passing hard 10k,
  100k, and 1M per-strategy quality thresholds without changing public C ABI.
- `0.9.0-alpha.3` tracking stability regression evidence, adding
  `--tracking-stability` and routine CTest coverage for representative tuned
  Optimized tracking behavior without further evaluator tuning or public C ABI
  changes.
- `v0.9.0` tracking quality/stability evidence stabilization, adopting the
  deterministic tracking audit, tuned evaluator hardening, and fixed stability
  regression evidence while keeping the `v0.8.0` 172-symbol public C ABI
  unchanged.
- `v0.10.0` Visualization v2 evidence stabilization, adopting the 30-PNG local
  gallery, local verifier, strict regeneration evidence, and manual-only CI
  artifact workflow while keeping the `v0.9.0` stable C ABI unchanged.
- `v0.11.0` waypoint soft-interruption and platform-clock evidence
  stabilization, adopting waypoint `ruckig_update` soft-interruption V1 and
  the internal platform clock abstraction while keeping the `v0.9.0` stable C
  ABI unchanged.
- `v0.12.0` waypoint soft-interruption true-resume stabilization, adopting
  background resume/publish semantics and the unified private waypoint
  optimizer engine while keeping the `v0.9.0` stable C ABI unchanged.
- `0.13.0-alpha.1` waypoint true-resume stress evidence, adding focused
  multi-DoF, multi-waypoint, per-section, budget-matrix, fresh-solve
  reference, long online-loop, and allocation-guard coverage without changing
  public C ABI.
- `0.13.0-alpha.2` waypoint true-resume engine hardening, rewriting the
  private optimizer/resume state into an internal waypoint engine and adding a
  128-case deterministic quality baseline audit without changing public C ABI.
- `0.13.0-readiness` local stable-review audit evidence, rerunning build,
  CTest, oracle, release-random, performance, ABI/export, platform-clock,
  visualization, wrapper, coverage, and boundary gates for the alpha.1/alpha.2
  waypoint true-resume evidence without changing public C ABI.
- `v0.13.0` stable release, adopting the post-`v0.12.0`
  waypoint true-resume stress coverage and private engine rewrite while
  keeping the 172-symbol public C ABI unchanged.
- `0.14.0-alpha.1` API-neutral interrupt boundary audit evidence, adding the
  focused boundary selector without changing public C ABI.
- `0.14.0-alpha.2` future interrupt surfaces design evidence, documenting
  no-waypoint and online tracking interruption boundaries without
  implementation changes.
- `0.14.0-alpha.3` interrupt implementation-readiness evidence, approving
  API-neutral no-waypoint and online tracking implementation slices while
  keeping public diagnostics and tracking sequence interruption deferred.
- `0.14.0-alpha.4` no-waypoint complete-trajectory-boundary interruption,
  using the existing interrupt field and output flag without adding public
  C ABI.
- `0.14.0-alpha.5` Optimized online tracking candidate-boundary interruption
  for tracking update and lookahead update, without public C ABI changes.
- `0.14.0-readiness` local stable-review audit evidence for alpha.1 through
  alpha.5, rerunning build, CTest, oracle, release-random, performance,
  ABI/export, platform-clock, visualization, wrapper, coverage, and boundary
  gates without version bump, tag, release, push, or manual workflow.
- `v0.14.0` stable release, adopting the waypoint,
  no-waypoint, and online tracking interrupt surface evidence while keeping
  the 172-symbol public C ABI unchanged.
- `0.15.0-alpha.1` post-release interrupt quality baseline evidence for the
  `v0.14.0` waypoint, no-waypoint, and Optimized online tracking interrupt
  surfaces.
- `0.15.0-alpha.2` tracking sequence interruption API draft evidence,
  documenting why sequence interruption needs an explicit public carrier.
- `0.15.0-alpha.3` consumer and wrapper interrupt smoke coverage for the
  `v0.14.0` no-waypoint and Optimized online tracking interrupt surfaces.
- `0.15.0-alpha.4` tracking sequence continuation public API scaffold,
  accepting a design-line public C ABI expansion from 172 to 184 symbols.
- `0.15.0-alpha.5` Fast tracking sequence continuation behavior for the
  alpha.4 interruptible/resume API.
- `0.15.0-alpha.6` Optimized tracking sequence continuation behavior with
  private candidate-boundary continuation state.
- `0.15.0-alpha.7` C/Python/Rust prototype smoke coverage for tracking
  sequence continuation, keeping wrappers prototype-only and adding no further
  public C ABI beyond the 184-symbol alpha.4 baseline.
- `0.15.0-alpha.8` tracking sequence continuation hardening, tightening the
  private `delta_time` resume contract, sharing Optimized candidate enumeration,
  and adding continuation matrix coverage without new exported C symbols.
- `0.15.0-readiness` local stable-review audit evidence for alpha.1 through
  alpha.8, rerunning build, CTest, oracle, release-random, performance,
  ABI/export, platform-clock, visualization, wrapper, coverage, and boundary
  gates without version bump, tag, release, or manual workflow.
- `v0.15.0` stable release, adopting the tracking sequence continuation public
  C ABI, Fast/Optimized continuation behavior, wrapper smoke evidence, and
  continuation hardening while promoting the public symbol baseline to 184
  symbols.
- `0.16.0-alpha.3` public diagnostics core API, adding
  `ruckig_diagnostics_init` and validate/calculate/update
  `_with_diagnostics` entry points while preserving legacy API behavior.
- `0.16.0-alpha.4` public diagnostics mapping for interruption and waypoint
  resume state without adding public symbols.
- `0.16.0-alpha.5` tracking public diagnostics getters for tracking handles
  and tracking sequence continuation state, exposing stable coarse diagnostics
  while keeping solver/profile/candidate/queue internals private.
- `v0.16.0` stable release, adopting the public diagnostics API and raising
  the public C ABI baseline to 190 symbols without publishing Python/Rust
  wrappers or package-manager recipes.
- `0.10.0-alpha` visualization v2 local gallery evidence, replacing the
  current `main` gallery with 30 project-owned `1400x900` PNGs and a strict
  local verifier while keeping the `v0.9.0` stable C ABI unchanged.
- `0.10.0-alpha.2` optional Visualization v2 CI artifact evidence, adding a
  manual-only `visualization_artifacts=true` workflow path that regenerates the
  gallery, verifies it, strict-regenerates it, and uploads the regenerated PNGs,
  manifest, and logs without changing default push/PR CI.
- `0.10.0-readiness` release readiness audit evidence, rerunning local build,
  visualization verifier, strict regeneration, routine CTest, performance,
  ABI/export, wrapper smoke, optional manual visualization artifact, and
  boundary gates before the Visualization v2 line entered `v0.10.0` stable
  closeout.
- C examples for position, offline position, online update, per-DoF overrides,
  velocity, stop, minimum duration, waypoints, per-section minimum duration,
  tracking Fast-mode scenarios, Optimized tracking scenarios, sequence
  continuation, and public diagnostics.

Release-readiness evidence is tracked under `docs/release/`; see
`docs/index.md` for the organized documentation map. `v0.16.0` is the current
stable release after completed `0.16.0-readiness`, release-candidate local
gates, tag CI, tag manual release-random, and GitHub Release publication.
`v0.16.0` promotes the public diagnostics API and moves the stable public C ABI
baseline to 190 symbols while preserving legacy API behavior and existing
result-code numeric values. `v0.15.0` remains the previous stable tracking
sequence continuation
release with a 184-symbol public C ABI baseline. `v0.14.0` keeps the `v0.9.0`
172-symbol public C ABI unchanged while stabilizing API-neutral interrupt
surfaces. Public
`ruckig_calculate` still ignores
`interrupt_calculation_duration` and runs complete solves. On the `0.14.0`
design line, no-waypoint `ruckig_update` now supports complete-trajectory
boundary interruption without true-resume, and Optimized online tracking
update/lookahead now supports best-so-far complete-candidate-boundary
interruption. On the `0.15.0` design line, the old
`ruckig_tracking_calculate_sequence` entry point remains complete-only, while
the new interruptible/resume sequence API uses a continuation handle and
exposes only complete sequence-step prefixes. Python and Rust wrappers remain
prototype smoke evidence only; no wheel, crate, package-manager recipe, or
stable wrapper API is claimed.

Minimal public diagnostics usage initializes a caller-owned record, calls the
diagnostics variant, and reads stable coarse fields:

```c
ruckig_diagnostics_t diagnostics;
ruckig_diagnostics_init(&diagnostics);
ruckig_result_t result = ruckig_calculate_with_diagnostics(
    otg, input, trajectory, &diagnostics);

printf("result=%d scope=%d code=%d dof=%zu value=%g limit=%g\n",
    (int)result,
    (int)diagnostics.scope,
    (int)diagnostics.code,
    diagnostics.dof,
    diagnostics.value,
    diagnostics.limit);
```

Passing `NULL` diagnostics to a `_with_diagnostics` API is defined to behave
like the corresponding legacy API. `examples/c/24_public_diagnostics.c`
contains a complete validation-failure and successful-calculate example.
`v0.10.0` adopts the current gallery as 30 `1400x900` Matplotlib `Agg` and
NumPy PNG assets under `docs/assets/visualization/`. The gallery covers local
C ABI equivalents of original examples `01-10` and `14-16`, plus tracking
diagnostics, waypoint diagnostics, trajectory anatomy, and cross-topic summary
plots; examples `11-13` remain excluded because they demonstrate C++
Eigen/custom-vector ergonomics rather than C ABI behavior.
`tools/visualization/verify_gallery.py` verifies the committed PNG/manifest
assets locally, including an optional strict regeneration check. The optional
manual CI artifact path can regenerate Visualization v2 PNGs, manifest, and
logs for review; it is not a default push/PR gate and does not replace
committed assets. The previous v1 gallery provenance remains available through
the `v0.9.0` tag rather than being duplicated on `main`. Stable closeout
evidence, including release-random and Visualization v2 artifact workflow
runs, is recorded in `docs/release/checklists/0.10.0.md`.
`v0.11.0` stable closeout evidence, including release-random workflow and tag
publication evidence, is recorded in `docs/release/checklists/0.11.0.md`.
The `0.9.0` tracking evidence line
starts with `docs/current/tracking_quality_audit.md` and a deterministic
`--tracking-random-audit` C test-runner selector so fallback-heavy Optimized
tracking behavior can be classified before evaluator tuning. `0.9.0-alpha.2`
adds `docs/current/tracking_quality_hardening.md`, private evaluator
attribution, and the `--tracking-quality-hardening` selector to record tuned
Optimized tracking quality thresholds while keeping the public C ABI unchanged.
`0.9.0-alpha.3` adds `docs/current/tracking_stability.md` and the
`--tracking-stability` selector to freeze representative alpha.2 tuned behavior
as regression evidence before readiness. `0.9.0-readiness` records full local
readiness evidence for the tracking quality/stability line, and `v0.9.0`
stabilizes that evidence without changing the 172-symbol public C ABI.
`0.9.1`, `0.10.1`, `0.11.1`, `0.12.1`, `0.13.1`, and `0.14.1` are reserved
for emergency patch work only.
`v0.4.0` added waypoint-aware C ABI entry points, per-section constraints,
global position bounds, and a local coupled waypoint optimizer; `v0.4.1`
deepened waypoint optimizer evidence; `v0.4.2` keeps that public C surface
unchanged while recording the original parity coverage matrix and the
`0.5.0-design` tracking/soft-interruption preparation work. `v0.5.0`
stabilizes the tracking public C API and local Fast-mode implementation.
Tracking public C API is not exposed in `v0.4.2`. `v0.3.0`
remains the last no-new-C-API hardening release, and `v0.2.5` remains the final
planned `0.2.x` stabilization baseline.

In `v0.11.0`, waypoint `ruckig_update` implements soft-interruption V1 through
the existing `interrupt_calculation_duration` field and
`was_calculation_interrupted` output state. Public `ruckig_calculate`,
no-waypoint target solving, and tracking remain unchanged by that field.
`v0.12.0` extends only waypoint `ruckig_update` with private true-resume after
an interrupted calculation. Later normal `pass_to_input` cycles can continue
the waypoint optimizer and publish a better complete remaining trajectory; no
public ABI, no-waypoint interruption, tracking interruption, or runtime clock
setter is added. The stable release keeps that public boundary and removes the
internal split between complete waypoint solving and the resumable optimizer
engine.

Current stable release scope intentionally excludes:

- Cloud and remote calculation runtime; local optimizer and tracking work only.
- Source-level Cloud/Pro parity. Upstream Cloud/Pro implementation source is
  not available, so accepted Cloud/Pro-described surfaces are evaluated by
  local interface/effect behavior instead of source copying.
- Formal Ruckig Pro/cloud global numerical equivalence claims.
- Hard real-time guarantees for waypoint optimization.
- Formal global optimality guarantees for `Optimized` tracking. The `v0.7.0`
  implementation is a bounded deterministic local evaluator with Fast fallback
  diagnostics, strategy presets, and diagnostics snapshots; these are local
  quality controls, not a global optimality claim.
- Finer-grained interruption expansion beyond complete candidate boundaries.
  No-waypoint true-resume, tracking sequence interruption, public interrupt
  diagnostics, hard real-time guarantees, and runtime platform-clock setters
  remain deferred.
- Python/Rust binding publication. The Python `cffi` prototype and Rust alpha
  wrapper remain prototype-only; package publication is frozen until a
  separate demand decision accepts it.
- Algorithm visualization as a default CI release gate. `v0.8.0` stabilizes the
  local `ruckig_c`-owned Matplotlib PNG gallery and verifier evidence, but no
  original images or PDFs are copied as primary project evidence and no
  plotting job is added to default CI.
- Package-manager recipes and new package-manager prototypes are frozen
  outside the active roadmap. Existing CMake install, pkg-config, static/DLL,
  and shared install-tree consumption paths remain the supported integration
  surface.

## Build

On Windows, the default local path uses the checked-in LLVM clang/Ninja preset:

```powershell
cmake --preset windows-clang-ninja
cmake --build --preset windows-clang-ninja
ctest --preset windows-clang-ninja
```

The Windows preset points at the currently verified local toolchain:

- C compiler: `D:/Program Files/LLVM/bin/clang.exe`
- C++ compiler: `D:/Program Files/LLVM/bin/clang++.exe`
- Ninja: `C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe`

On other platforms, or on Windows shells where the compiler is already
discoverable through the environment, the generic developer preset remains:

```sh
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

Local build trees should stay under `out/build/` by using the checked-in
CMake presets. This keeps ad hoc build directories from accumulating in the
repository root.

Useful presets:

- `windows-clang-ninja` is the verified Windows default. It builds the C
  library, C tests, and examples under `out/build/windows-clang-ninja` using
  LLVM clang and the Visual Studio bundled Ninja.
- `windows-clang-ninja-shared` builds the same Windows LLVM/Ninja target set as
  a shared library under `out/build/windows-clang-ninja-shared`; it is intended
  for ABI/export hygiene checks and Python prototype smoke tests.
- `windows-clang-ninja-coverage` builds the local LLVM coverage target set
  under `out/build/windows-clang-ninja-coverage`; it is intended for
  `tools/coverage/run_coverage.ps1` and is not a CI gate.
- `dev` builds the C library, C tests, and examples under `out/build/dev`.
  It is intentionally generic and requires the current shell or CMake generator
  selection to find a working C/C++ compiler.
- `release` builds the same routine targets under `out/build/release`.
- `shared` builds a shared library under `out/build/shared`.
- `oracle` enables the frozen C++ differential oracle tests under
  `out/build/oracle`.

Clean ignored local build artifacts with a dry run first:

```powershell
.\scripts\clean-local.ps1
```

Then apply the cleanup when the preview is correct:

```powershell
.\scripts\clean-local.ps1 -Apply
```

Use `-KeepReleaseBuilds` to preserve `build_release_check_ninja`,
`build_release_check_shared`, `out/build/release`, and `out/build/shared`.

Useful CMake options:

- `BUILD_RUCKIG_C=ON` builds the C library.
- `BUILD_RUCKIG_C_TESTS=ON` builds C unit tests and header compile tests.
- `BUILD_RUCKIG_C_EXAMPLES=ON` builds C examples.
- `BUILD_RUCKIG_C_ORACLE_TESTS=ON` builds the C++ differential oracle tests against `original/ruckig-main`.
- `BUILD_RUCKIG_C_PERFORMANCE_TESTS=ON` builds the C/C++ oracle performance benchmark.
- `RUCKIG_C_ENABLE_CALCULATION_DURATION=ON` records `ruckig_update` monotonic elapsed calculation duration in microseconds in `ruckig_output_get_calculation_duration`.
- `BUILD_SHARED_LIBS=ON` builds a shared library instead of a static library.

On Windows, the current verified CMake path uses LLVM clang/clang++ with the
Visual Studio bundled Ninja. The `windows-clang-ninja` preset is the default
local entry point for that path. The oracle harness must compile `.c` sources
with a C compiler and the original Ruckig sources with a C++ compiler. Current
verification includes static-library and DLL/import-library builds; see
`docs/release/evidence/verification_report.md`.

## Install and Consume

Install the library and public headers with CMake:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build --prefix /path/to/prefix
```

Downstream CMake consumers can use the installed package:

```cmake
find_package(ruckig_c CONFIG REQUIRED)
add_executable(app main.c)
target_link_libraries(app PRIVATE ruckig_c::ruckig_c)
```

On systems with `pkg-config`, downstream C consumers can use:

```sh
cc main.c $(pkg-config --cflags --libs ruckig_c) -o app
```

The installed package exports the target `ruckig_c::ruckig_c`. Static CMake
consumers receive `RUCKIG_C_STATIC_DEFINE` automatically from the target. When
manually linking a static Windows build without CMake, define
`RUCKIG_C_STATIC_DEFINE`; DLL consumers should not define it.

Additional consumer and packaging notes are collected in `docs/current/packaging.md`,
including installed CMake consumers, pkg-config consumers, Windows manual
static linking, DLL consumers, shared install-tree verification, and
post-`v0.16.0` consumer smoke coverage for public diagnostics.
Patch-release API/ABI review notes, including exported-symbol snapshot
generation, are collected in `docs/current/api_compatibility.md`.

## C API Shape

The public API exposes opaque handle types:

- `ruckig_t`
- `ruckig_input_t`
- `ruckig_output_t`
- `ruckig_trajectory_t`
- `ruckig_tracking_t`
- `ruckig_target_state_t`
- `ruckig_target_state_sequence_t`
- `ruckig_tracking_output_sequence_t`
- `ruckig_tracking_sequence_continuation_t`

Handles are created once for a fixed DoF count. Data vectors are accessed through preallocated arrays returned by accessors such as `ruckig_input_current_position_data` and `ruckig_input_max_velocity_data`.
User code should use only the `ruckig_*_t` typedef names; struct tag names are
opaque implementation details and are intentionally not part of the public API.

The public functions are annotated with `RUCKIG_C_API` for shared-library
exports. Define `RUCKIG_C_STATIC_DEFINE` when consuming a manually built static
library on Windows; the CMake target defines it automatically for static builds.

Required input state and limit arrays:

- Current state: `ruckig_input_current_position_data`, `ruckig_input_current_velocity_data`, `ruckig_input_current_acceleration_data`.
- Target state: `ruckig_input_target_position_data`, `ruckig_input_target_velocity_data`, `ruckig_input_target_acceleration_data`.
- Limits: `ruckig_input_max_velocity_data`, `ruckig_input_max_acceleration_data`, `ruckig_input_max_jerk_data`.
- DoF enable flags: `ruckig_input_enabled_data`.

Optional input fields:

- `ruckig_input_set_min_velocity` / `ruckig_input_clear_min_velocity`.
- `ruckig_input_set_min_acceleration` / `ruckig_input_clear_min_acceleration`.
- `ruckig_input_set_minimum_duration` / `ruckig_input_clear_minimum_duration`.
- Global mode setters: `ruckig_input_set_control_interface`, `ruckig_input_set_synchronization`, and `ruckig_input_set_duration_discretization`.

Per-DoF override setters:

- `ruckig_input_set_per_dof_control_interface` / `ruckig_input_clear_per_dof_control_interface`.
- `ruckig_input_set_per_dof_synchronization` / `ruckig_input_clear_per_dof_synchronization`.

When a per-DoF vector is enabled, its `count` must match the input DoF count.
Clearing a per-DoF vector restores the matching global setter behavior.

Numerical inputs must be finite except where the C++ baseline accepts infinite acceleration or jerk limits to select lower-order solvers. Velocity, acceleration, and jerk maxima must be non-negative, directional minima must be non-positive, and enabled DoFs must satisfy the same current/target state limit checks as the C++ oracle when validation requests those checks.

For detailed error-code and limit-selection notes, see
`docs/current/api_diagnostics.md`.

Basic offline usage:

```c
ruckig_t* otg = NULL;
ruckig_input_t* input = NULL;
ruckig_trajectory_t* trajectory = NULL;

ruckig_create(&otg, 1, 0.01);
ruckig_input_create(&input, 1);
ruckig_trajectory_create(&trajectory, 1);

ruckig_input_target_position_data(input)[0] = 2.0;
ruckig_input_max_velocity_data(input)[0] = 2.0;
ruckig_input_max_acceleration_data(input)[0] = 1.5;
ruckig_input_max_jerk_data(input)[0] = 1.0;

if (ruckig_calculate(otg, input, trajectory) == RUCKIG_WORKING) {
    double position[1], velocity[1], acceleration[1];
    ruckig_trajectory_at_time(trajectory, 0.5, position, velocity, acceleration, NULL, NULL);
}

ruckig_trajectory_destroy(trajectory);
ruckig_input_destroy(input);
ruckig_destroy(otg);
```

For offline `ruckig_calculate`, `RUCKIG_WORKING` is the successful result that
means the trajectory is valid and can be queried. `RUCKIG_FINISHED` is used by
the online update loop once the sampled time has passed the trajectory
duration.

`ruckig_trajectory_at_time` requires a non-NULL `position` output array with at
least the trajectory DoF count. `velocity`, `acceleration`, `jerk`, and
`section` may be `NULL` when the caller does not need those outputs.

Online usage calls `ruckig_update`, reads `ruckig_output_new_*_data`, then calls
`ruckig_output_pass_to_input` when the caller wants to feed the new state into
the next cycle. The caller should read the output arrays before mutating or
destroying the owning output handle.

Tracking usage creates a `ruckig_tracking_t` handle plus target
state or target sequence handles. `Fast` mode performs local
constant-acceleration lookahead and calls the existing update path. `v0.6.0`
also includes bounded local `Optimized` mode with candidate search, online
lookahead sequences, offline sliding-window sequence calculation, and fallback
diagnostics. `v0.7.0` stabilizes high-level Optimized strategy presets plus
`ruckig_tracking_get_last_diagnostics` snapshots covering score summary,
candidate-family counters, and aggregate step counts. Balanced is the default
strategy. See
`docs/design/tracking_interface.md` for the tracking ABI semantics and
`docs/design/tracking_optimized_mode.md` for the Optimized tracking design.
Interruptible tracking sequence continuation stores the initiating tracking
handle's `delta_time` in the opaque continuation. `ruckig_tracking_resume_sequence`
may resume on another tracking handle only when the DoF count and `delta_time`
match the captured continuation contract.

## Memory Model

Create functions allocate handles and all vectors owned by those handles. The intended release contract is that `ruckig_calculate`, `ruckig_update`, trajectory sampling, and root solvers do not allocate heap memory.

The current implementation includes an internal allocation counter hook used by C tests. It verifies representative `ruckig_calculate`, `ruckig_update`, and `ruckig_trajectory_at_time` paths do not call the library's allocation helpers after lifecycle setup. CTest also runs a source-level allocation audit that rejects direct `malloc`/`calloc`/`realloc`/`free` calls outside `src/ruckig_c/alloc.c`.

`ruckig_output_get_calculation_duration` returns `0.0` by default. Define `RUCKIG_C_ENABLE_CALCULATION_DURATION` or configure CMake with `-DRUCKIG_C_ENABLE_CALCULATION_DURATION=ON` to measure `ruckig_update` monotonic elapsed calculation duration in microseconds.

Ownership rules:

- Destroy functions accept `NULL`.
- `ruckig_output_t` owns its internal trajectory.
- A standalone `ruckig_trajectory_t` created by `ruckig_trajectory_create` is owned by the caller.
- `ruckig_tracking_t` owns only its internal workspace; it does not own caller
  input, output, target-state, or sequence handles.
- Accessor-returned arrays are borrowed pointers. They remain valid until the owning handle is destroyed, and output-owned trajectory pointers remain valid until the next successful `ruckig_update` with that output handle.

## Thread Safety

Independent handles can be used from independent threads. A single handle is not internally synchronized; callers must not mutate or use the same `ruckig_t`, input, output, or trajectory concurrently without external synchronization.

## Result Codes

The main non-error results are:

- `RUCKIG_WORKING`
- `RUCKIG_FINISHED`

`RUCKIG_RESULT_IS_OK(result)` is a header-only helper for callers that want to
treat both non-error results as successful control flow.

Common error results:

- `RUCKIG_ERROR_INVALID_INPUT`
- `RUCKIG_ERROR_ZERO_LIMITS`
- `RUCKIG_ERROR_EXECUTION_TIME_CALCULATION`
- `RUCKIG_ERROR_SYNCHRONIZATION_CALCULATION`
- `RUCKIG_ERROR_UNSUPPORTED`

`RUCKIG_ERROR_UNSUPPORTED` is reserved for public features outside the current
C API scope. Solver calculation failures return execution-time or
synchronization calculation errors instead of being silently ignored.

## Examples

The C examples are in `examples/c`:

- `00_minimal_offline.c`
- `01_position.c`
- `02_position_offline.c`
- `03_minimal_online.c`
- `04_per_dof_override.c`
- `05_velocity.c`
- `06_stop.c`
- `07_minimum_duration.c`
- `08_per_dof_online.c`
- `09_waypoints_offline.c`
- `10_per_section_minimum_duration.c`
- `11_waypoints_online.c`
- `12_per_section_limits.c`
- `13_filter_intermediate_positions.c`
- `14_dynamic_dofs_waypoints.c`
- `15_tracking_online_fast_ramp.c`
- `16_tracking_online_constant_acceleration.c`
- `17_tracking_offline_sequence.c`
- `18_tracking_online_optimized_lookahead.c`
- `19_tracking_online_optimized_sinus.c`
- `20_tracking_offline_optimized_sequence.c`
- `21_no_waypoint_interrupt_boundary.c`
- `22_tracking_interrupt_boundary.c`
- `23_tracking_sequence_continuation.c`
- `24_public_diagnostics.c`
- `25_tracking_public_diagnostics.c`

All examples are wired into CMake when `BUILD_RUCKIG_C_EXAMPLES=ON`.

## Oracle Tests

The oracle target compares the C implementation with the frozen original C++ implementation under `original/ruckig-main`. It covers fixed regressions and deterministic random stress cases, and prints profile details on mismatches. It is a test-only target; the `ruckig_c` library does not link the C++ oracle and must not depend on a C++ runtime.

The oracle executable also accepts a deterministic random smoke-test mode:

```sh
ruckig_c_oracle_tests --random 100 --seed 1
```

For per-DoF override hardening, the oracle executable also supports a controlled
routine smoke mode and a larger manual/development stress mode:

```sh
ruckig_c_oracle_tests --random-per-dof 100 --seed 1
```

```sh
ruckig_c_oracle_tests --random-per-dof 100000 --seed 1
```

The small per-DoF random smoke is suitable for routine CI when oracle tests are
enabled. The larger stress command is intended for manual development and patch
release gates.

The fixed oracle suite also covers per-DoF control-interface and
synchronization overrides. The random generator covers first/second/third-order
position and velocity cases, 1-3 DoF inputs, synchronization modes,
continuous/discrete duration, directional limits, disabled DoFs, and general
third-order position states. Development random coverage has been run with
`100,000` trajectories for seeds `1`, `2`, and `41`; the release stress command
has been run with `1,000,000` trajectories for seed `1`. See
`docs/release/evidence/verification_report.md`.

## Performance Benchmark

`BUILD_RUCKIG_C_PERFORMANCE_TESTS=ON` enables `ruckig_c_performance_benchmark`, which measures C `ruckig_calculate` against the frozen C++ oracle on the same deterministic generated corpus.

Example:

```sh
ruckig_c_performance_benchmark --samples 10000 --seed 1
```

The current development performance report is in
`docs/release/evidence/performance_report.md`. Windows clang ASan/UBSan CMake
tests pass when the LLVM sanitizer runtime directory is present in `PATH`.
Linux Clang ASan/UBSan, Valgrind, pkg-config consumer, and performance evidence
is recorded in the release checklists under `docs/release/checklists/`.

## Verification

Current CMake/Ninja and direct-clang verification evidence is recorded in `docs/release/evidence/verification_report.md`, including:

- C unit tests.
- C and C++ header compile tests.
- Static and shared CMake builds.
- Fixed oracle suite.
- `100,000` development random oracle runs.
- `1,000,000` release random oracle run.
- C examples.
- Internal allocation-counter checks and source-level allocation audit.
- Windows clang ASan/UBSan CMake tests.

GitHub Actions CI covers Windows, Linux, and macOS routine checks. The
`ruckig_c_oracle_random_release` test is intentionally excluded from routine
CI and is available as a manual release gate.

GitHub CLI release/workflow operation notes are recorded in
`docs/current/github_operations.md`. On Windows, a sandboxed `gh auth status`
can report an invalid token if it cannot read the user keyring; confirm from a
keyring-aware command environment before treating the result as a permissions
problem.

Local test coverage evidence for the `v0.9.0` stable release is recorded in
`docs/current/test_coverage_audit.md`. The release coverage runner writes raw
LLVM artifacts under `out/coverage/0.9.0/`; those raw artifacts are local
evidence only and are not committed.

Visualization gallery evidence is local-only. To verify committed gallery
assets without regenerating them:

```powershell
.\_local\visualization-venv\Scripts\python.exe tools\visualization\verify_gallery.py --output docs\assets\visualization
```

To additionally regenerate into an ignored `out/` directory and compare against
committed assets:

```powershell
$env:RUCKIG_C_SHARED_LIBRARY=(Resolve-Path out\build\windows-clang-ninja-shared\ruckig_c.dll).Path
.\_local\visualization-venv\Scripts\python.exe tools\visualization\verify_gallery.py --output docs\assets\visualization --strict-regenerate
```

`docs/release/checklists/0.9.0.md` records the stable release closeout for the
tracking quality/stability evidence line. Visualization gallery verification
remains a local evidence tool; it is not wired into default GitHub Actions,
CMake, or CTest.
