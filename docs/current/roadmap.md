# Ruckig C Post-Release Roadmap

This roadmap keeps shipped release scope locked while preserving concrete
follow-up work for stability and future feature planning.

## 0.1.x Stability Queue

- Done for `0.1.1`: fixed regression cases for 3 DoF high-frequency online
  update loops, near-limit velocity control, very small `delta_time`, mixed
  disabled/active DoFs, discrete duration plus minimum duration, and
  directional min velocity/min acceleration edge values.
- Done for `0.1.1`: C API diagnostics tests and `docs/current/api_diagnostics.md`.
- Done for `0.1.1`: minimal offline and online examples wired into CMake and
  CTest.
- Done for `0.1.1`: patch-release performance recording procedure in
  `docs/release/evidence/performance_report.md`.
- Done for `0.1.1`: frozen upstream baseline policy in
  `docs/current/upstream_baseline_policy.md`.
- Remaining for later `0.1.x`: record final Windows and Linux benchmark
  results for each patch release, including average, p99, worst, and C/oracle
  average ratio.
- Remaining for later `0.1.x`: expand downstream consumer notes if new
  packaging or toolchain-specific issues appear.

## 0.2.0 Feature Planning

- Done for `0.2.0`: per-DoF control-interface and synchronization overrides
  implemented after the dedicated design document in
  `docs/design/per_dof_overrides.md`.
- Done for `0.2.0`: fixed oracle cases compare mixed per-DoF control and
  synchronization settings against the frozen C++ baseline.
- Defer waypoints and per-section constraints until a separate design addresses the Community cloud/pro behavior boundary.
- Defer Python and Rust bindings until the C ABI has stabilized through a
  `0.2.x` patch cycle.
- Treat any upstream Ruckig baseline update as a separate project with source inventory, tolerance review, oracle corpus updates, full random stress, and new performance baselines.

## 0.2.x Maintenance

- Done for `0.2.1`: documentation source-of-truth cleanup, routine per-DoF
  random oracle smoke, post-`v0.2.0` hardening in the changelog, consumer
  packaging guidance, and API/ABI compatibility documentation.
- Done for `0.2.2`: automated exported-symbol evidence through the
  shared-build `ruckig_c_exported_symbols` target and GitHub Actions
  Linux/Windows exported-symbol artifact jobs.
- Done for `0.2.2`: Windows static/DLL consumer smoke scripts where supported
  by the release-check toolchain and continued fixed oracle regression corpus
  expansion.
- Done for `0.2.2`: published `v0.2.2` with final tag, GitHub Release, push
  CI, manual release-random workflow, performance, consumer, and ABI evidence.
- Done for `0.2.3`: published `v0.2.3` with final tag, GitHub Release, push
  CI, manual release-random workflow, performance, consumer, and ABI evidence.
- Done for `0.2.4`: published `v0.2.4` with final tag, GitHub Release, push
  CI, manual release-random workflow, performance, consumer, and ABI evidence.
- Done for `0.2.5`: published `v0.2.5` as the final planned `0.2.x`
  stabilization release before `0.3.0-design`, with final tag, GitHub Release,
  push CI, manual release-random workflow, performance, consumer, and ABI
  evidence.
- `0.2.x` planned maintenance is complete.
- Reserve `0.2.6` only for emergency patch work after `v0.2.5`; it is not the
  default post-release route.
- Track `very large duration + exact target first-time-at-position` as a
  documented tolerance investigation after `v0.2.5`; it is not a `v0.2.5`
  release blocker. `0.3.0-design` now has fixed 50s and 100s oracle cases for
  the boundary shape. The 100s case is retained with a case-specific
  `2e-4` first-time tolerance because the long near-flat final segment differs
  from the frozen C++ oracle by about `1.64e-4s` while still matching trajectory
  sampling and found/not-found semantics.
- Before each `0.2.x` patch release, record Windows and Linux release
  benchmarks with average, p99, worst, and C/oracle average ratio.
- Keep per-DoF override hardening focused on oracle coverage, diagnostics, and
  examples without changing the public C API.
- Done after `0.2.0`: expanded fixed per-DoF oracle cases for Phase,
  TimeIfNecessary, discrete None/Time, disabled DoFs, and mixed-order/mixed
  control inputs.
- Done after `0.2.0`: added controlled per-DoF random oracle coverage through
  `ruckig_c_oracle_tests --random-per-dof N --seed S` as a development/manual
  gate without changing the existing `--random` behavior.
- Done after `0.2.0`: added C API regression coverage for per-DoF clear behavior
  and update recalculation stability, plus an online per-DoF C example.
- Keep waypoints, per-section constraints, cloud calculation, Python/Rust
  bindings, and upstream baseline upgrades as separate future projects.

## 0.3.0 Design Candidates

The default mainline stage is now `0.3.0-design`. The `0.3.0` line is
design-only until a separate proposal is accepted. It must not change public
API, solver dispatch, or the frozen oracle baseline during design evaluation.

- Current priority decision is recorded in
  `docs/design/0.3.0_priorities.md` and
  `docs/design/0.3.0_readiness.md`: maintain ABI/export hygiene before binding
  or package-manager prototypes depend on the shared-library surface; evaluate
  Python bindings before Rust bindings once prerequisites are met.
- The pre-`0.3.0` readiness decision is recorded in
  `docs/design/0.3.0_readiness.md`.
- Python binding feasibility is scoped in
  `docs/design/python_bindings_feasibility.md`. Current work is prototype-only;
  it does not approve a formal binding API, release package, or C ABI change.
- ABI/export hygiene is the first `0.3.0-design` implementation queue and its
  first pass is implemented on `main`. Linux historical implementation-internal
  exports from `v0.2.5` are not public API; `docs/abi/public-symbols.txt` is the
  approved public symbol allowlist.
- Evaluate Python or Rust bindings only after the C ABI has passed at least one
  `0.2.x` patch cycle, `docs/current/api_compatibility.md` is complete, and CMake,
  pkg-config, and shared/static consumer paths are stable.
- Evaluate an upstream Ruckig baseline upgrade only as a separate project with
  upstream diff review, source inventory update, tolerance review, oracle
  corpus update, full random stress, and new performance baselines.
- Keep waypoints, per-section constraints, and cloud calculation behind a
  separate design document that defines the Community cloud/pro behavior
  boundary, C API shape, and unsupported/partial behavior before any public API
  is implemented.
- First `0.3.0-design` priorities:
  1. Maintain ABI/export hygiene and Linux internal symbol cleanup evidence.
  2. Strict public ABI diff gate trial, opt-in locally and warning/evidence-only
     in the dedicated exported-symbol CI jobs.
  3. Windows consumer matrix hardening for MSVC `cl` and MinGW feasibility.
  4. Python `cffi` ABI-mode prototype design and prototype plan.
  5. vcpkg feasibility prototype plan.
  6. Upstream baseline upgrade evaluation as a separate project.
  7. Rust binding feasibility after Python feasibility results.
