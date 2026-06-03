# Ruckig C Post-Release Roadmap

This roadmap keeps the `0.1.0` release scope locked while preserving concrete
follow-up work for stability and future feature planning.

## 0.1.x Stability Queue

- Add fixed regression cases for 3 DoF high-frequency online update loops.
- Add cases near velocity, acceleration, and jerk limits.
- Add very small `delta_time` coverage.
- Add mixed disabled/active DoF coverage.
- Add discrete duration plus minimum duration coverage.
- Add directional min velocity/min acceleration edge-value coverage.
- Expand README and examples with minimal offline, minimal online, static link, shared library, CMake install, and pkg-config consumer notes.
- Record Windows and Linux release benchmark results for every patch release, including average, p99, worst, and C/oracle average ratio.
- Add API diagnostics documentation for common invalid input causes, zero limits, finite/infinite limit semantics, `minimum_duration` with discrete duration, and `Synchronization::None` behavior.
- Keep `original/ruckig-main` frozen as the Ruckig Community `0.17.3` oracle baseline.

## 0.2.0 Feature Planning

- Evaluate per-DoF control-interface and synchronization overrides before waypoints or bindings.
- Produce a dedicated design document before adding any public `0.2.0` API.
- Define how per-DoF overrides are represented in C, how they inherit global defaults, how unsupported combinations fail, and how the oracle harness compares them.
- Defer waypoints and per-section constraints until a separate design addresses the Community cloud/pro behavior boundary.
- Defer Python and Rust bindings until the C ABI has stabilized through `0.1.x`.
- Treat any upstream Ruckig baseline update as a separate project with source inventory, tolerance review, oracle corpus updates, full random stress, and new performance baselines.
