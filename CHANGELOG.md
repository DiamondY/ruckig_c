# Changelog

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
