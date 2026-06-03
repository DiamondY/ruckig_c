# Changelog

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

Added:

- Fixed oracle regression cases for 3 DoF high-frequency online updates,
  near-limit velocity control, very small discrete `delta_time`, mixed
  disabled/active DoFs, discrete minimum duration, and directional lower-limit
  edge values.
- C API diagnostic tests for invalid numerical inputs, zero-limit error paths,
  finite/infinite solver selection semantics, and `Synchronization::None`
  behavior.
- Minimal offline and online C examples wired into CMake and CTest.
- API diagnostics documentation in `docs/api_diagnostics.md`.
- Patch-release performance recording procedure in `docs/performance_report.md`.
- Frozen upstream baseline policy in `docs/upstream_baseline_policy.md`.
- Per-DoF override design gate in `docs/design_per_dof_overrides.md`.

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

- Windows clang/clang-cl validation is recorded in `docs/verification_report.md`.
- Linux/macOS and sanitizer/memcheck gates are captured in CI and
  `docs/release_checklist.md`.
- Performance results against the frozen C++ oracle are recorded in
  `docs/performance_report.md`.

Known scope limitations:

- The C library does not implement intermediate waypoints, per-section
  constraints, cloud calculation, Python/Rust bindings, or per-DoF
  control/synchronization overrides in `0.1.0`.
- `original/ruckig-main` remains a frozen test oracle and is not linked into
  the C library.
