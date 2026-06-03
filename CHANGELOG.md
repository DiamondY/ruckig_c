# Changelog

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
