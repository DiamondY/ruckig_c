# Changelog

## 0.4.0 - 2026-06-06

`0.4.0` starts the full original-surface parity line after `v0.3.0`. This
release intentionally expands the public C ABI for intermediate waypoints,
per-section constraints, global position bounds, and multi-section trajectory
queries. The waypoint optimizer is local-only; no cloud or remote calculation
client is implemented.

Added:

- Added waypoint-aware constructors for `ruckig_t`, `ruckig_input_t`,
  `ruckig_output_t`, and `ruckig_trajectory_t`.
- Added public C ABI access to global max/min position bounds.
- Added intermediate waypoint set/get/clear APIs.
- Added per-section max/min velocity, max/min acceleration, max jerk,
  max/min position, and per-section minimum-duration APIs.
- Added interrupt-calculation-duration storage APIs for original API surface
  parity; alpha behavior records the value but does not yet guarantee soft
  interruption.
- Added multi-section trajectory metadata APIs for section count and
  intermediate durations.
- Added deterministic local `ruckig_filter_intermediate_positions`.
- Added an experimental local coupled waypoint optimizer. It searches shared
  internal waypoint velocity/acceleration candidates, evaluates each candidate
  through the existing target solver section evaluator, rejects constraint
  violations, explores a deterministic internal branch queue around the best
  candidates, and selects the lowest-duration feasible candidate.
- Added C examples for waypoint offline calculation, waypoint online updates,
  per-section minimum duration, per-section limits, intermediate-position
  filtering, and dynamic DoFs with waypoints.
- Added focused CTest entries for waypoint optimizer, per-section constraints,
  and waypoint quality alpha checks.
- Added a waypoint alpha performance benchmark mode for the local C optimizer
  corpus. It is C-only evidence because Ruckig Community `0.17.3` has no local
  global waypoint optimizer oracle.
- Added CI coverage for `0.4.0` ABI/export artifact paths, Linux
  waypoint alpha performance output, Python prototype smoke, and Rust alpha
  wrapper smoke.
- Extended the experimental Python `cffi` ABI-mode prototype to cover the
  `0.4.0` waypoint-aware C ABI surface.
- Added an experimental Rust alpha wrapper over `ruckig_c` with smoke tests and
  examples for position, offline calculation, velocity, waypoints, and
  per-section minimum duration.
- Strengthened Python and Rust prototype smoke coverage for per-section
  position constraints, interrupt-calculation-duration storage,
  first-time-at-position, intermediate-position readback/filtering, and output
  calculation-state accessors.

Changed:

- `CMakeLists.txt` and public version macros now point at `0.4.0`.
- ABI artifact output paths now use `artifacts/abi/0.4.0-design`.
- `docs/abi/public-symbols.txt` and
  `docs/abi/public-symbol-exceptions.txt` now record the approved `0.4.0`
  public API expansion.
- `bindings/python_prototype/` remains prototype-only; it is not installed,
  packaged, or treated as a stable Python binding API.
- `bindings/rust/` is prototype-only; it is not published as a crate and does
  not wrap original C++ Ruckig.
- No-waypoint target-solver behavior remains on the existing frozen C++ oracle
  path and must not regress.

Still deferred:

- Formal cloud/Pro numerical equivalence claims, hard real-time guarantees for
  waypoint optimization, released Python wheels, published Rust crate,
  package-manager recipes, cloud API support, and upstream baseline upgrades.

## 0.3.0 - 2026-06-05

`0.3.0` is a hardening release. It promotes the completed `0.3.0-design`
engineering work into a versioned release boundary without adding public C API,
changing solver scope, publishing bindings, adding package-manager recipes, or
updating the frozen upstream oracle baseline.

Changed:

- `main` now prepares the `0.3.0` hardening release after publishing `v0.2.5`
  as the final planned `0.2.x` stabilization release.
- ABI comparison baselines now roll forward to `docs/abi/v0.2.5/`; strict
  public ABI diff failure remains opt-in for local builds, while the dedicated
  Linux/Windows exported-symbol CI jobs upload warning/evidence artifacts.
- `0.3.0` release priority starts with ABI/export hygiene and existing
  installed-package consumer paths before binding release work.
- Added `docs/abi/public-symbols.txt` as the approved public C ABI symbol
  allowlist derived from `include/ruckig_c/ruckig.h`.
- Added a public symbol allowlist verification target that extracts
  `RUCKIG_C_API` declarations from the public header and checks the tracked
  allowlist.
- Non-Windows shared builds now hide implementation-internal symbols by
  default and export only declarations marked with `RUCKIG_C_API`.
- Linux shared builds additionally link with a public-symbol version script
  generated from `docs/abi/public-symbols.txt`.
- Added a public exported-symbol comparison target for warning/evidence-only
  strict ABI gate trial artifacts, with the dedicated ABI CI jobs currently
  uploading public diff evidence without yet failing the workflow on drift.
- Added `docs/abi/public-symbol-exceptions.txt` as the explicit approval file
  for intentional future public symbol additions; it is empty by default.
- Added fixed oracle cases for 50s and 100s exact-target first-time boundaries;
  the 100s case is retained with a documented case-specific first-time
  tolerance exception.
- Added and smoke-tested a Python `cffi` ABI-mode prototype workspace against a
  local shared `ruckig_c` build.
- Retained the experimental vcpkg overlay prototype as frozen reference
  evidence after verifying local `x64-windows` shared/default and
  `x64-windows-static` consumer paths.
- Downgraded package-manager recipes and new package-manager prototypes to
  long-term optional work; the existing vcpkg overlay is retained as frozen
  reference evidence outside the active roadmap.
- Added opt-in MSVC `cl` standalone static and DLL consumer CTest gates and
  verified both locally; they remain outside routine CI.
- Added CMake presets that keep routine local builds under `out/build/`, plus
  a dry-run-first local cleanup script for ignored build trees, caches, and
  temporary files.
- Added a Windows-specific `windows-clang-ninja` preset as the default local
  README build path, plus a matching `windows-clang-ninja-shared` preset for
  ABI/export and Python prototype smoke validation.
- Recorded a `0.3.0` hardening pass covering the Windows preset,
  shared DLL/import-library consumer, public symbol allowlist verification,
  public exported-symbol comparison, Python `cffi` prototype smoke, and current
  MinGW/MSVC `cl` toolchain availability.
- Clarified that MSVC `cl` standalone consumer smokes remain opt-in local gates
  rather than routine CI.
- Added MinGW static and DLL/import-library consumer smoke support, verified
  both locally with GCC 15.2.0, and added a dedicated MSYS2 MinGW64 routine CI
  consumer gate.
- Completed the Python prototype design decisions for low-level ABI shape,
  future high-level wrappers, result/error handling, explicit shared-library
  discovery, copy-in/copy-out arrays, and deferred wheel/package strategy.
- Added a `0.3.0` release decision document and release checklist to keep the
  hardening-release scope separate from future feature, packaging, and binding
  projects.

Still deferred:

- Intermediate waypoints, per-section constraints, cloud calculation, Python
  binding release work, Rust bindings, package-manager recipes and new
  package-manager prototypes, strict exported-symbol fail gates, and upstream
  baseline upgrades.

## 0.2.5 - 2026-06-05

`0.2.5` is planned as the final `0.2.x` stabilization release before
`0.3.0-design`.

Added:

- `v0.2.4` Linux and Windows exported-symbol baselines for `0.2.5`
  warning/evidence-only ABI comparison.
- `0.2.5` release checklist with strict ABI gate design fields, consumer
  matrix evidence, performance trend comparison, and targeted oracle regression
  gates.
- `0.3.0` readiness decision document covering the post-`v0.2.5` design entry
  criteria, binding/package-manager priorities, ABI gate status, and deferred
  feature boundaries.
- Targeted fixed C++ oracle regression cases for high-DoF discrete
  minimum-duration synchronization, disabled-DoF online updates, large-duration
  first-time boundaries, and mixed first/second/third-order
  synchronization edges.

Changed:

- `0.2.5` ABI work is still warning/evidence-only by default; strict ABI diff
  enforcement remains a design target until the documented prerequisites are
  satisfied.
- Expanded Windows consumer documentation for existing `clang`/`clang-cl`
  coverage, planned MSVC `cl` standalone static/DLL smoke gates, and MinGW
  feasibility status.
- Expanded package-manager feasibility notes with vcpkg first, Conan second,
  Homebrew third, and FetchContent/vendored subdirectory guidance only.
- Expanded Python binding feasibility notes for the future `cffi` ABI-mode
  prototype without adding binding implementation.

Still deferred:

- Intermediate waypoints, per-section constraints, cloud calculation, Python
  binding implementation, Rust bindings, package-manager recipes, strict ABI
  fail gates, and upstream baseline upgrades.

## 0.2.4 - 2026-06-04

Added:

- `v0.2.3` Linux and Windows exported-symbol baselines for `0.2.4`
  warning/evidence-only ABI comparison.
- Windows `clang-cl` shared C-only CI coverage so the DLL/import-library
  consumer smoke also runs under the MSVC frontend variant.

Changed:

- Windows static and DLL consumer smoke scripts now support both GNU-like
  `clang` and `clang-cl` frontend modes.
- `0.2.4` ABI comparison remains warning/evidence only; it is not a strict CI
  fail gate.

Fixed:

- Windows `clang-cl` manual static consumer smoke now uses the dynamic CRT mode
  expected by the CMake-built static library, avoiding mixed CRT link failures.

Still deferred:

- Intermediate waypoints, per-section constraints, cloud calculation, Python
  binding implementation, Rust bindings, and upstream baseline upgrades.

## 0.2.3 - 2026-06-04

Added:

- Non-strict exported-symbol baseline comparison against the `v0.2.2`
  baseline for Linux and Windows shared builds. The comparison is warning and
  evidence only; it is not yet a strict CI fail gate.
- `0.2.3` release checklist template with ABI baseline comparison, consumer
  matrix, numerical regression, performance trend, and release-random evidence
  fields.
- Package-manager feasibility notes for vcpkg, Conan, Homebrew, FetchContent,
  and vendored subdirectory use. No package-manager recipe is implemented.
- Additional fixed C++ oracle regression cases for higher-DoF per-DoF
  synchronization, disabled DoFs, discrete minimum-duration edge cases, tiny
  nonzero limits with large position magnitude, long online update loops, and
  repeated first-time-at-position boundary queries.

Changed:

- Expanded Python binding feasibility design to select `cffi` ABI mode as the
  default prototype path and document prototype acceptance criteria.
- Clarified that `0.2.3` maintenance keeps `original/ruckig-main` frozen and
  does not add public C API.

Still deferred:

- Intermediate waypoints, per-section constraints, cloud calculation, Python
  binding implementation, Rust bindings, and upstream baseline upgrades.

## 0.2.2 - 2026-06-04

Added:

- `0.2.2` release checklist template with ABI/exported-symbol, consumer
  automation, performance trend, and per-DoF random oracle evidence fields.
- Shared-build exported-symbol evidence target `ruckig_c_exported_symbols`,
  using `nm` on Unix-like systems and `llvm-readobj` or `dumpbin` on Windows.
- GitHub Actions Linux/Windows exported-symbol snapshot job that runs the
  shared-build helper and uploads review artifacts.
- Windows consumer smoke CTest scripts for manual static linking and DLL
  import-library consumption where the release-check toolchain supports them.
- Additional fixed C++ oracle regression cases for 4-6 DoF mixed scenarios,
  long high-frequency online update loops, very small `delta_time` with
  per-DoF mixed synchronization, segment-boundary query coverage, and
  multi-disabled mixed-order inputs.
- Python bindings feasibility design for `0.3.0-design`; this is design-only
  and does not add binding code.
- `0.3.0-design` priority evaluation documenting that `0.2.x` package,
  consumer, ABI, performance, and regression evidence should mature before
  bindings work; Python bindings should be evaluated before Rust bindings once
  prerequisites are met.

Changed:

- Expanded `0.2.x` packaging, ABI, performance, and roadmap documentation for
  repeatable `0.2.2` maintenance evidence collection.

Still deferred:

- Intermediate waypoints, per-section constraints, cloud calculation, Python
  bindings, Rust bindings, and upstream baseline upgrades.

## 0.2.1 - 2026-06-04

Added:

- Routine per-DoF random oracle smoke coverage through
  `ruckig_c_oracle_tests --random-per-dof 100 --seed 1`.
- `0.2.1` release checklist template for patch-release evidence collection.
- Packaging and consumer guidance for installed CMake, pkg-config, Windows
  manual static linking, DLL consumers, and shared install-tree verification.
- API/ABI compatibility policy documentation for `0.2.x` patch releases.

Changed:

- Clarified that `docs/historical/c_rewrite_execution_plan.md` is a historical execution
  plan, while current scope is defined by README, public header, roadmap,
  release checklists, and upstream baseline policy.
- Documented post-`v0.2.0` hardening on `main`, including the fixed oracle suite
  increasing from 48 release-time cases to 59 cases, controlled
  `--random-per-dof N --seed S` stress, per-DoF clear behavior regression,
  per-DoF update recalculation stability regression, and
  `examples/c/08_per_dof_online.c`.
- Added fixed oracle regression coverage for large-magnitude positions,
  tiny nonzero limits, large discrete minimum duration, mixed first/second/third
  order per-DoF inputs, explicit first-time-at-position boundaries, and
  disabled DoF per-DoF overrides under discrete duration.

Still deferred:

- Intermediate waypoints, per-section constraints, cloud calculation, Python
  bindings, Rust bindings, and upstream baseline upgrades.

## 0.2.0 - 2026-06-03

Added:

- Public per-DoF control-interface override setters and clearers:
  `ruckig_input_set_per_dof_control_interface` and
  `ruckig_input_clear_per_dof_control_interface`.
- Public per-DoF synchronization override setters and clearers:
  `ruckig_input_set_per_dof_synchronization` and
  `ruckig_input_clear_per_dof_synchronization`.
- Fixed C++ oracle cases for mixed position/velocity control overrides and
  mixed `Time`/`None` synchronization overrides.
- C API tests for invalid per-DoF setter inputs, clear behavior, update
  recalculation, and the no-allocation runtime contract with per-DoF settings
  enabled.
- Minimal per-DoF override C example wired into CMake and CTest.
- The `0.1.1` stability queue additions are included in this `0.2.0` mainline
  release.

Changed:

- Fixed oracle suite now contains 48 deterministic cases.
- The target calculator dispatch now uses effective per-DoF control and
  synchronization settings when per-DoF vectors are enabled.

Still deferred:

- Intermediate waypoints, per-section constraints, cloud calculation, Python
  bindings, Rust bindings, and upstream baseline upgrades.

## 0.1.1 - Unreleased

This section is retained only for a possible `v0.1` maintenance branch. These
stability changes are already present on `main` through the `0.2.0` mainline
release or later `0.2.x` hardening work.

Added:

- Fixed oracle regression cases for 3 DoF high-frequency online updates,
  near-limit velocity control, very small discrete `delta_time`, mixed
  disabled/active DoFs, discrete minimum duration, and directional lower-limit
  edge values.
- C API diagnostic tests for invalid numerical inputs, zero-limit error paths,
  finite/infinite solver selection semantics, and `Synchronization::None`
  behavior.
- Minimal offline and online C examples wired into CMake and CTest.
- API diagnostics documentation in `docs/current/api_diagnostics.md`.
- Patch-release performance recording procedure in `docs/release/evidence/performance_report.md`.
- Frozen upstream baseline policy in `docs/current/upstream_baseline_policy.md`.
- Per-DoF override design gate in `docs/design/per_dof_overrides.md`.

Changed:

- Fixed oracle suite now contains 44 deterministic cases.
- CMake example tests now include the minimal offline and online examples.

## 0.1.0 - 2026-06-03

Initial public release for the pure C99 rewrite of Ruckig Community `0.17.3`
local state-to-state trajectory generation.

Added:

- Public opaque C ABI in `include/ruckig_c/ruckig.h`.
- Offline `ruckig_calculate` and online `ruckig_update` APIs.
- Position and velocity control for first-, second-, and third-order supported
  local state-to-state trajectories.
- Multi-DoF synchronization modes `Time`, `TimeIfNecessary`, `Phase`, and
  `None`.
- Continuous and discrete duration handling.
- Directional min velocity/min acceleration limits, disabled DoFs, global
  minimum duration, trajectory sampling, position extrema, and first-time query.
- CMake static/shared builds, C examples, C unit tests, C++ oracle differential
  tests, allocation audit, and performance benchmark.
- CMake package and pkg-config install metadata for downstream consumers.

Deferred:

- Intermediate waypoints, per-section constraints, cloud calculation, Python
  bindings, Rust bindings, and per-DoF control/synchronization overrides.

Verification:

- Windows clang/clang-cl validation is recorded in `docs/release/evidence/verification_report.md`.
- Linux/macOS and sanitizer/memcheck gates are captured in CI and
  `docs/release/checklists/0.1.0.md`.
- Performance results against the frozen C++ oracle are recorded in
  `docs/release/evidence/performance_report.md`.

Known scope limitations:

- The C library does not implement intermediate waypoints, per-section
  constraints, cloud calculation, Python/Rust bindings, or per-DoF
  control/synchronization overrides in `0.1.0`.
- `original/ruckig-main` remains a frozen test oracle and is not linked into
  the C library.
