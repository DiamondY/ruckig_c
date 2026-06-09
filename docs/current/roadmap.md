# Ruckig C Post-Release Roadmap

This roadmap keeps shipped release scope locked while preserving concrete
follow-up work for stability and future feature planning.

## 0.11.0 Design Line

Current `main` is `0.11.0-design - Unreleased` after the published `v0.10.0`
Visualization v2 evidence release.

- First priority: soft interruption checkpoint design and evidence planning.
- Review `docs/design/interrupt_calculation_duration.md` before changing
  timeout/checkpoint behavior.
- Keep soft interruption semantics behind a dedicated compatibility review;
  do not change public C ABI, enum numeric values, or result-code numeric
  values by default.
- Keep `v0.10.0` as the current stable release.
- Keep the `v0.9.0` 172-symbol public C ABI baseline unless a separate public
  API decision is accepted.
- `0.10.1` remains reserved for emergency patch fixes only.
- Package-manager recipes, formal Python/Rust publication, cloud/remote
  calculation, Pro/cloud equivalence claims, formal global optimality proof,
  and upstream baseline upgrades remain deferred unless separately accepted.

## 0.10.0 Visualization V2 Evidence Release

Published as `v0.10.0`. This release keeps the `v0.9.0` 172-symbol public C
ABI unchanged while stabilizing the Visualization v2 gallery, local verifier,
strict regeneration evidence, and manual-only CI artifact workflow reviewed
during `0.10.0-readiness`.

- First priority: visualization v2, optional CI visualization artifacts, and
  richer local plots. This priority is complete for the stable release.
- `0.10.0-alpha` adds the first Visualization v2 evidence slice: a local-only
  30-PNG Matplotlib `Agg` gallery, `1400x900` PNG assets, deterministic
  manifest, and hybrid verifier with optional strict regeneration.
- `0.10.0-alpha.2` adds optional CI artifact evidence for Visualization v2:
  a manual-only `visualization_artifacts=true` workflow path that regenerates
  the gallery, verifies it, strict-regenerates it, and uploads the regenerated
  PNGs, manifest, and logs as review artifacts.
- `0.10.0-readiness` records the full local release-readiness audit for the
  Visualization v2 gallery, verifier, and manual-only artifact path. The
  ordinary push CI for the evidence commit succeeded, and the line entered
  stable closeout.
- `v0.10.0` stabilizes the committed 30-PNG gallery and manifest
  byte-for-byte. The manifest label remains `0.10.0-alpha visualization v2
  evidence` as asset provenance.
- Stable release evidence includes local verifier, strict regeneration,
  ordinary CI, manual release-random, manual Visualization v2 artifact
  workflows on the release candidate and tag, annotated tag publication, and
  GitHub Release publication.
- The alpha replaces the current `main` gallery assets; the previous v1
  provenance remains available through the `v0.9.0` tag.
- Keep `v0.10.0` as the current stable release.
- Keep the `v0.9.0` 172-symbol public C ABI baseline unless a separate public
  API decision is accepted.
- Do not promote plotting or verifier work into a default release/CI gate
  without a separate dependency and artifact policy decision; the alpha.2 CI
  artifact path remains manual-only. Stable closeout, not readiness, is where
  version bump, tag, GitHub Release, and manual release-random workflow steps
  occur. For this line, those steps are complete in `v0.10.0`.
- `0.10.1` remains reserved for emergency patch fixes only.
- Package-manager recipes, formal Python/Rust publication, soft interruption,
  cloud/remote calculation, formal Pro/cloud equivalence claims, formal global
  optimality proof, and upstream baseline upgrades remain deferred unless
  separately accepted.

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

## 0.3.0 Hardening Release

Published as `v0.3.0`. The accepted release decision is recorded in
`docs/design/0.3.0_release_decision.md`. The release publishes the completed
`0.3.0-design` engineering hardening work without changing public API, solver
dispatch, or the frozen oracle baseline. Final tag, GitHub Release, push CI,
manual release-random workflow, ABI/export artifacts, macOS bootstrap evidence,
performance, consumer, and Python prototype evidence are recorded in
`docs/release/checklists/0.3.0.md` and
`docs/release/evidence/verification_report.md`.

- The priority decision that shaped `0.3.0` is recorded in
  `docs/design/0.3.0_priorities.md` and
  `docs/design/0.3.0_readiness.md`: maintain ABI/export hygiene and existing
  installed-package consumer paths; evaluate Python bindings before Rust
  bindings once prerequisites are met.
- The pre-`0.3.0` readiness decision is recorded in
  `docs/design/0.3.0_readiness.md`.
- Python binding feasibility is scoped in
  `docs/design/python_bindings_feasibility.md`. Current work is prototype-only;
  it does not approve a formal binding API, release package, or C ABI change.
- ABI/export hygiene shipped as the first `0.3.0` release queue. Linux historical
  implementation-internal exports from `v0.2.5` are not public API;
  `docs/abi/public-symbols.txt` is the approved public symbol allowlist.
- Evaluate Python or Rust bindings only after the C ABI has passed at least one
  `0.2.x` patch cycle, `docs/current/api_compatibility.md` is complete, and CMake,
  pkg-config, and shared/static consumer paths are stable.
- Package-manager recipes and new package-manager prototypes are downgraded to
  long-term optional work. The active roadmap keeps only the existing installed
  CMake package, pkg-config, Windows static/DLL, and shared install-tree
  consumption paths. Existing package-manager prototype notes may be frozen
  unless a separate packaging project is explicitly accepted.
- Evaluate an upstream Ruckig baseline upgrade only as a separate project with
  upstream diff review, source inventory update, tolerance review, oracle
  corpus update, full random stress, and new performance baselines.
- Keep waypoints, per-section constraints, and cloud calculation behind a
  separate design document that defines the Community cloud/pro behavior
  boundary, C API shape, and unsupported/partial behavior before any public API
  is implemented.
- `0.3.0` shipped priorities and follow-up boundaries:
  1. Maintain ABI/export hygiene and Linux internal symbol cleanup evidence.
     The current build exposes public-symbol allowlist verification and
     public exported-symbol comparison targets for shared builds.
  2. Strict public ABI diff gate trial, opt-in locally and warning/evidence-only
     in the dedicated exported-symbol CI jobs. Do not promote it to routine
     strict failure until repeated artifact review is clean.
  3. Windows consumer matrix hardening for MSVC `cl` and MinGW feasibility.
     `clang` and `clang-cl` static/DLL paths remain the routine verified
     Windows paths. MSVC `cl` standalone static/DLL gates remain opt-in and
     local, not routine CI. MinGW static and DLL/import-library consumers now
     have local GCC evidence and a dedicated MSYS2 MinGW64 routine CI gate.
  4. Python `cffi` ABI-mode prototype design and prototype smoke, still
     experimental and outside routine CI until shared-library discovery is
     stable.
  5. Upstream baseline upgrade evaluation as a separate project.
  6. Rust binding feasibility after Python feasibility results.

Long-term optional, outside the active roadmap:

- Package-manager recipes for vcpkg, Conan, Homebrew, FetchContent, and
  vendored subdirectory use.
- New package-manager prototype work beyond the existing experimental notes.

## 0.4.x Original-Surface Parity

The planned `0.4.x` original-surface parity line is complete through published
`v0.4.2`. `v0.4.0` was the first planned public C ABI expansion after the
`v0.3.0` hardening release. `v0.4.1` was a deep waypoint optimizer
stabilization release. `v0.4.2` is the coverage and evidence closeout release
for the surface: it keeps the `v0.4.0` public C ABI unchanged while recording
original parity coverage, tracking design scope, and soft-interruption design
boundaries. The goal remains original-surface parity with a local waypoint
optimizer, not a cloud client.

- Added waypoint-aware C constructors and input/trajectory APIs for
  intermediate waypoints, global position bounds, per-section constraints,
  interrupt-duration storage, intermediate duration queries, and local
  waypoint filtering.
- Added a local coupled waypoint optimizer. It searches shared internal
  waypoint velocity/acceleration candidates, evaluates every complete candidate
  through the existing target solver section evaluator, rejects constraint
  violations, explores a deterministic internal branch queue around promising
  candidates, and selects the lowest-duration feasible result.
- The optimizer is local-only. It does not call Ruckig cloud, does not
  provide remote fallback, and does not yet claim Ruckig Pro/cloud global
  numerical equivalence.
- No-waypoint target-solver behavior remains frozen against the existing C++
  oracle path.
- Python `cffi` and Rust wrappers now have prototypes over the public C ABI.
  They remain unpublished and outside stable package scope.
- CI configuration now includes `0.4.x` ABI artifact paths, Linux
  waypoint performance output, Python prototype smoke, and Rust alpha wrapper
  smoke. Multiple alpha push CI runs have passed; the stable release closeout
  uses the same artifact path for final CI evidence.
- Package-manager recipes, cloud API support, and upstream baseline upgrades
  remain separate projects.

## 0.4.1 Deep Stabilization

`0.4.1` is the immediate follow-up stabilization release after `v0.4.0`.

- No new public C API is planned for `0.4.1`; the only public header change is
  the release version macros.
- Waypoint optimizer evidence is strengthened with deeper fixed C corpus,
  sampled invariant checks, per-section limit sampling, section-oracle
  coverage, and an expanded local waypoint benchmark corpus.
- Routine stress remains local and deterministic. CI and release gates do not
  depend on Ruckig cloud, Ruckig Pro, network access, or external licenses.
- `interrupt_calculation_duration` remains storage/API-surface parity in
  `0.4.1`; optimizer interruption checkpoints require a later design and are
  not claimed as hard or soft real-time behavior in this release.
- The Python `cffi` prototype receives smoke-test hardening only. Python wheel
  publication, Rust crate publication, package-manager recipes, new public C
  API expansion, and upstream baseline upgrades move to separate future design
  work such as `0.5.0-design`.

## 0.4.2 Original Parity Coverage Closeout

Published as `v0.4.2`. It is the final planned `0.4.x` evidence closeout
before `0.5.0-design` and a patch release, not a feature release.

- No new public C API is planned for `0.4.2`; the only public header change is
  the release version macros.
- `docs/current/original_parity_coverage.md` records the current engineering
  coverage estimates and remaining original parity gaps. The estimates are not
  formal line, branch, or proof coverage.
- Tracking is now a mandatory full-original-parity gap. `0.4.2` records the
  future API and evidence direction only; implementation and public C API
  additions move to `0.5.0-design`.
- `interrupt_calculation_duration` remains storage/API-surface parity in
  `0.4.2`; optimizer interruption checkpoints and timeout fallback semantics
  require future implementation work.
- Python `cffi` and Rust remain prototype/alpha evidence. Wheels, crate
  publication, package-manager recipes, cloud/remote calculation, formal
  Pro/cloud numerical equivalence claims, and upstream baseline upgrades remain
  separate projects.

## 0.5.0 Stable Tracking Release

Published as `v0.5.0`. This release stabilizes the public C tracking ABI and
the local Fast-mode online/offline tracking implementation. It is a Fast-only
tracking release; `Optimized` remains declared for API shape parity but returns
`RUCKIG_ERROR_UNSUPPORTED` throughout `0.5.x`.

- The public C tracking ABI is now part of the stable C surface:
  opaque tracking handles, target-state handles, target-state sequences,
  tracking output sequences, online update, and offline sequence calculation.
- Local `Fast` mode is implemented with deterministic constant-acceleration
  lookahead. `Optimized` mode is declared for API shape parity but returns
  `RUCKIG_ERROR_UNSUPPORTED` in `0.5.x`.
- Tracking evidence is local and deterministic: API lifecycle and validation
  tests, online/offline C tests, quality smoke, no-allocation smoke, C examples,
  Python `cffi` prototype smoke, and Rust alpha wrapper smoke.
- `0.5.0-alpha.2` hardens the local evidence without changing the public C API:
  C tracking now includes a deterministic fixed corpus, tuned ramp and
  constant-acceleration quality gates against naive instantaneous chasing,
  multi-DoF no-allocation coverage, and expanded Python/Rust smoke.
- A stable `v0.5.0` can be Fast-only if the evidence remains clean. The
  `Optimized` tracking implementation is deferred to `0.6.0-design`; it must
  not be aliased to Fast.
- ABI review for this line uses `docs/abi/public-symbols.txt`,
  `docs/abi/public-symbol-exceptions.txt`, and artifact paths under
  `artifacts/abi/0.5.0`.
- Soft-interruption implementation design continues separately and is not
  coupled to the first tracking implementation.
- Cloud/remote calculation, package-manager recipes, formal Pro/cloud
  numerical equivalence claims, stable Python wheels, Rust crate publication,
  and upstream baseline upgrades stay outside the `0.5.x` patch scope and
  require a separate `0.6.0-design` or later decision.

## 0.6.0 Optimized Tracking Release

Published as `v0.6.0`. This release stabilizes the bounded local
`Optimized` tracking MVP on top of the public C tracking ABI and local Fast
tracking release from `v0.5.0`.

- `Optimized` tracking is local-only and bounded. It uses deterministic
  candidate search, online lookahead update, offline sliding-window sequence
  calculation, and Fast fallback diagnostics.
- The release adds intentional public C symbols for Optimized tracking
  diagnostics and lookahead update. ABI artifact paths use
  `artifacts/abi/0.6.0`.
- `0.6.1` is reserved for emergency patches only.
- Soft-interruption implementation, formal Python/Rust publication,
  package-manager recipes, cloud/remote calculation, formal Pro/cloud
  equivalence claims, and upstream baseline upgrades remain separate projects
  unless explicitly accepted.

## 0.7.0 Strategy And Diagnostics Release

Published as `v0.7.0`. This release stabilizes the 172-symbol public C ABI
reviewed during `0.7.0-readiness`: the stable `v0.6.0` bounded local
Optimized tracking API plus high-level strategy preset controls and the public
diagnostics snapshot getter.

- `0.7.0-alpha.2` evidence adds high-level Optimized strategy presets
  (`Stable`, `Balanced`, `Aggressive`) with Balanced as the default plus the
  public `ruckig_tracking_get_last_diagnostics` snapshot API. It keeps the
  bounded target-solver-per-candidate evaluator, default candidate budget `16`,
  and Fast fallback semantics.
- The alpha.2 hard gates are local and deterministic: Balanced must not be
  worse than Fast on the fixed tracking corpus, Balanced must improve by at
  least `0.5%` on selected smooth lookahead cases, Aggressive must improve over
  Balanced by at least `2%` on fixed oscillatory cases, routine tracking random
  stress runs `--tracking-random 100000 --seed 1/2/41`, and manual stress uses
  `--tracking-random 1000000 --seed 1`.
- `0.7.0-alpha.3` evidence adds a local-only LLVM coverage audit and original
  Community test/example behavior mapping. The local broad routine coverage
  corpus records implementation line/function/branch coverage under
  `docs/current/test_coverage_audit.md`, but coverage is not a CI job and not
  a hard release gate.
- `0.7.0-alpha.4` evidence adds targeted solver branch coverage hardening for
  the five lowest files from the alpha.3 audit. It adds a lightweight
  `ruckig_c_solver_branch_coverage` CTest gate and fixed frozen-oracle cases,
  but still does not start stable `v0.7.0` closeout.
- `0.7.0-readiness` evidence reruns the full local release-readiness gate set
  against the current strategy preset and diagnostics API candidate. It treats
  the 172-symbol public C ABI as ready for stable review, records coverage,
  performance, ABI/export, wrapper, oracle, and 1M release-random readiness
  evidence.
- Stable closeout updates `CMakeLists.txt`, `include/ruckig_c/ruckig.h`
  version macros, `CHANGELOG.md`, release notes/checklists, and ABI artifact
  paths to `artifacts/abi/0.7.0`.
- `0.7.1` is reserved for emergency patches only.
- Algorithm visualization and trajectory gallery generation are deferred until
  after `v0.7.0`. The original Ruckig `doc/` images, example trajectory PDFs,
  and `examples/plotter.py` are useful references, but `v0.7.0` closeout must
  not add plotting dependencies, generated images/PDFs, CI jobs, or new release
  gates. A later visualization evidence project can generate `ruckig_c`-owned
  plots for no-waypoint trajectories, velocity/stop/minimum-duration cases,
  local waypoint sections, and Fast/Optimized tracking comparison.
- Soft interruption, formal Python/Rust publication, package-manager recipes,
  cloud/remote calculation, formal Pro/cloud equivalence claims, and upstream
  baseline upgrades remain deferred unless separately accepted.

## 0.8.0 Visualization Evidence Release

Published as `v0.8.0`. The release stabilizes the local algorithm
visualization/gallery evidence line without changing the `v0.7.0` 172-symbol
public C ABI.

- First priority: local algorithm visualization and trajectory gallery
  evidence.
- `0.8.0-alpha` added the first local visualization evidence slice:
  `tools/visualization/generate_gallery.py`,
  `docs/current/visualization.md`, generated PNG assets under
  `docs/assets/visualization/`, and a deterministic manifest.
- `0.8.0-alpha.2` replaces the Pillow-only gallery with a NumPy and
  Matplotlib `Agg` renderer and expands the committed PNG gallery to local C
  ABI equivalents of original examples `01-10` and `14-16`.
- `0.8.0-alpha.3` adds local visualization verifier evidence for the committed
  PNG/manifest assets, including optional strict regeneration into ignored
  `out/` artifacts. It is hardening evidence, not a stable closeout.
- `0.8.0-readiness` records focused local readiness evidence for the current
  visualization gallery and verifier.
- Stable closeout adopts the existing 13 PNG assets and
  `docs/assets/visualization/manifest.json` without relabeling or regenerating
  tracked gallery files.
- ABI artifact paths use `artifacts/abi/0.8.0`; public symbol count remains
  `172`, with public additions `0` and public removals `0`.
- Generate `ruckig_c`-owned plots from local C/Python prototype data rather
  than copying original Ruckig images as primary project evidence.
- Original examples `11-13` remain excluded because they demonstrate C++ Eigen
  and custom-vector ergonomics rather than behavior exposed through the C ABI.
- Visualization remains documentation/evidence work, not public C API work.
- Do not add visualization as a default CI or stable-release gate until a
  separate dependency and artifact policy is accepted.
- `0.8.1` remains reserved for emergency patches only.
- Formal Python/Rust publication, package-manager recipes, soft interruption,
  cloud/remote calculation, formal Pro/cloud equivalence claims, formal global
  optimality proof, and upstream baseline upgrades remain deferred unless
  separately accepted.

## 0.9.0 Tracking Quality And Stability Evidence Release

Published as `v0.9.0`. The release stabilizes tracking quality and stability
evidence without changing the `v0.8.0` 172-symbol public C ABI.

- First priority: tracking quality and stability hardening.
- Default stance: deepen fixed quality cases, deterministic stress, fallback
  diagnostics, performance evidence, and no-allocation coverage without
  expanding public C ABI unless a separate API decision is accepted.
- `0.9.0-alpha` adds the baseline evidence layer for that work: a deterministic
  `--tracking-random-audit` selector, lightweight routine audit CTest, fixed
  representative diagnostics cases, and local 10k/100k/1M audit summaries.
- `0.9.0-alpha.2` is that accepted follow-up: it tunes the bounded local
  Optimized evaluator, adds private candidate-family attribution, adds
  `--tracking-quality-hardening`, and hard-gates 10k, 100k seed `1/2/41`, and
  1M seed `1` per-strategy optimized-count and average-improvement thresholds.
- `0.9.0-alpha.3` freezes representative alpha.2 tuned behavior as regression
  evidence with `--tracking-stability` and routine CTest coverage. It does not
  retune evaluator scoring, candidate generation, strategy weights, near-tie
  policy, public diagnostics structs, public ABI, tags, or release scope.
- `0.9.0-readiness` records full local release-readiness evidence for the
  tracking quality/stability line. It reruns full local build, CTest, oracle,
  tracking 10k/100k/1M audit, performance, coverage, ABI/export, and wrapper
  gates without changing public C ABI.
- Stable closeout promotes versions and ABI artifact paths to `0.9.0`, creates
  release notes/checklist evidence, and keeps public additions and removals at
  `0`.
- `0.9.1` is reserved for emergency patch work only; it is not the default
  post-release route.
- `0.10.0-design - Unreleased` starts after `v0.9.0`, with visualization v2,
  optional CI artifacts, and richer plots as the first priority.
- Package-manager recipes, formal Python/Rust publication, soft interruption,
  cloud/remote calculation, formal Pro/cloud equivalence claims, formal global
  optimality proof, and upstream baseline upgrades remain deferred unless
  separately accepted.
