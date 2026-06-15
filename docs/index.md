# Documentation Index

This is the entry point for repository documentation. Current scope is defined
by this README, the public header, and the documents under `docs/current/`.
Historical rewrite plans and release evidence are retained for traceability.

## Current Maintainer Entry Points

- `current/roadmap.md` - shipped scope, maintenance queues, current stable
  release scope, the `0.16.0` release line, and future design boundaries.
- `current/api_compatibility.md` - patch-release API/ABI policy and
  exported-symbol review.
- `current/api_diagnostics.md` - C API validation and diagnostics behavior.
- `current/code_quality_audit.md` - post-`v0.15.0` code-quality and
  test-quality risk map, state-machine and solver branch coverage evidence,
  external-review follow-up hardening evidence, local quality gates, and
  random replay/materialization policy.
- `current/original_parity_coverage.md` - current coverage estimates and gaps
  against the frozen original reference and original product surface.
- `current/test_coverage_audit.md` - local LLVM coverage summary and original
  test/example behavior mapping.
- `current/tracking_quality_audit.md` - deterministic Optimized tracking
  fallback, diagnostics, and random-audit baseline evidence.
- `current/tracking_quality_hardening.md` - tuned Optimized tracking evaluator
  quality thresholds, private attribution, and alpha.2 hardening evidence.
- `current/tracking_stability.md` - fixed regression evidence for alpha.2
  tuned Optimized tracking behavior and private attribution invariants.
- `current/visualization.md` - locally generated algorithm visualization
  gallery evidence and generation policy.
- `current/packaging.md` - installed CMake, pkg-config, static, DLL, and shared
  install-tree consumer notes.
- `current/github_operations.md` - GitHub CLI authentication, keyring, workflow,
  release, and CI evidence operation notes.
- `current/upstream_baseline_policy.md` - frozen upstream oracle baseline
  policy.

## Release Evidence

- `release/checklists/` - release checklists by version.
  `release/checklists/0.4.0-alpha.md` retains the alpha evidence, and
  `release/checklists/0.4.2.md` records the published original-parity
  coverage/evidence closeout. `release/checklists/0.5.0.md` records the stable
  tracking Fast-mode release closeout; `release/checklists/0.5.0-alpha.md` and
  `release/checklists/0.5.0-alpha.2.md` retain the tracking alpha evidence.
  `release/checklists/0.6.0-alpha.md` retains the Optimized tracking alpha
  evidence, and `release/checklists/0.6.0.md` records the stable Optimized
  tracking release closeout. `release/checklists/0.7.0-alpha.md`,
  `release/checklists/0.7.0-alpha.2.md`,
  `release/checklists/0.7.0-alpha.3.md`, and
  `release/checklists/0.7.0-alpha.4.md` retain the tracking strategy,
  diagnostics hardening, coverage audit, and targeted solver branch coverage
  evidence on the `0.7.0-design` line.
  `release/checklists/0.7.0-readiness.md` records the full local stable-review
  evidence audit for the 172-symbol `v0.7.0` ABI candidate without creating a
  tag or GitHub Release.
  `release/checklists/0.8.0-alpha.md` records the first post-`v0.7.0`
  visualization/gallery evidence slice, and
  `release/checklists/0.8.0-alpha.2.md` records the Matplotlib gallery
  expansion. `release/checklists/0.8.0-alpha.3.md` records the local gallery
  verifier hardening evidence. `release/checklists/0.8.0-readiness.md` records
  the focused stable-review readiness audit for the current gallery/verifier
  evidence. `release/checklists/0.8.0.md` records the stable visualization
  evidence release closeout. `release/checklists/0.9.0-alpha.md` records the
  first `0.9.0-design` tracking quality baseline evidence, and
  `release/checklists/0.9.0-alpha.2.md` records the tuned evaluator hardening
  evidence. `release/checklists/0.9.0-alpha.3.md` records the tracking
  stability regression evidence. `release/checklists/0.9.0-readiness.md`
  records the full local stable-review readiness audit for the tracking
  quality/stability line without creating a tag or GitHub Release.
  `release/checklists/0.9.0.md` records the stable tracking quality/stability
  evidence release closeout. `release/checklists/0.10.0-alpha.md` records the
  first `0.10.0-design` Visualization v2 local gallery evidence slice, and
  `release/checklists/0.10.0-alpha.2.md` records the optional manual CI
  artifact evidence for regenerated Visualization v2 review assets.
  `release/checklists/0.10.0-readiness.md` records the full local
  stable-review readiness audit for the Visualization v2 gallery, verifier,
  and manual artifact path without creating a tag or GitHub Release.
  `release/checklists/0.10.0.md` records the stable Visualization v2 evidence
  release closeout. `release/checklists/0.11.0-readiness.md` records the full
  local stable-review readiness audit for waypoint soft interruption V1 and
  the internal platform clock abstraction without creating a tag or GitHub
  Release. `release/checklists/0.11.0.md` records the stable waypoint
  soft-interruption and platform-clock evidence release closeout.
  `release/checklists/0.12.0-alpha.1.md` records the first `0.12.0-design`
  waypoint soft-interruption true-resume evidence slice, including ordinary
  remote push CI evidence. `release/checklists/0.12.0-alpha.2.md` records the
  local true-resume unified-engine hardening evidence slice.
  `release/checklists/0.12.0-readiness.md` records the full local
  stable-review readiness audit for waypoint soft-interruption true-resume and
  the unified waypoint optimizer engine without creating a tag or GitHub
  Release. `release/checklists/0.12.0.md` records the stable waypoint
  soft-interruption true-resume release closeout.
  `release/checklists/0.13.0-alpha.1.md` records the first post-`v0.12.0`
  waypoint true-resume stress and quality evidence slice on the
  `0.13.0-design` line. `release/checklists/0.13.0-alpha.2.md` records the
  waypoint true-resume private engine rewrite and deterministic quality
  baseline evidence slice. `release/checklists/0.13.0-readiness.md` records
  the full local stable-review audit for those post-`v0.12.0` waypoint
  true-resume slices without creating a tag or GitHub Release.
  `release/checklists/0.13.0.md` records the stable waypoint true-resume
  stress and private engine rewrite release closeout.
  `release/checklists/0.14.0-alpha.1.md` records the first
  `0.14.0-design` API-neutral interrupt boundary audit without creating a tag
  or GitHub Release. `release/checklists/0.14.0-alpha.2.md` records the
  design-only future interrupt surfaces quasi-spec for no-waypoint and online
  tracking interruption. `release/checklists/0.14.0-alpha.3.md` records the
  implementation-readiness gap audit for conditional API-neutral no-waypoint
  and online tracking interruption slices. `release/checklists/0.14.0-alpha.4.md`
  records the local no-waypoint complete-trajectory-boundary interruption
  implementation evidence. `release/checklists/0.14.0-alpha.5.md` records the
  local online tracking best-so-far candidate-boundary interruption evidence.
  `release/checklists/0.14.0-readiness.md` records the full local
  stable-review readiness audit for alpha.1 through alpha.5 without creating a
  tag or GitHub Release. `release/checklists/0.14.0.md` records the stable
  API-neutral interrupt surface release closeout.
  `release/checklists/0.15.0-alpha.1.md` records the post-release interrupt
  quality baseline selector evidence, and
  `release/checklists/0.15.0-alpha.2.md` records the docs-only tracking
  sequence interruption API draft evidence.
  `release/checklists/0.15.0-alpha.3.md` records the consumer and wrapper
  interrupt smoke evidence. `release/checklists/0.15.0-alpha.4.md` records
  the tracking sequence continuation public API scaffold evidence.
  `release/checklists/0.15.0-alpha.5.md` records the Fast tracking sequence
  continuation implementation evidence. `release/checklists/0.15.0-alpha.6.md`
  records the Optimized tracking sequence continuation implementation evidence.
  `release/checklists/0.15.0-alpha.7.md` records the tracking sequence
  continuation C/Python/Rust prototype smoke evidence.
  `release/checklists/0.15.0-alpha.8.md` records the tracking sequence
  continuation hardening evidence, and
  `release/checklists/0.15.0-readiness.md` records the full local
  stable-review audit for alpha.1 through alpha.8 without creating a tag or
  GitHub Release. `release/checklists/0.15.0.md` records the stable tracking
  sequence continuation release closeout.
  `release/checklists/0.16.0-alpha.1.md` records the docs-only public
  diagnostics design start without implementing API or changing ABI, and
  `release/checklists/0.16.0-alpha.2.md` records the docs-only public
  diagnostics contract freeze before implementation.
  `release/checklists/0.16.0-alpha.3.md` records the first public diagnostics
  ABI expansion for `ruckig_diagnostics_init` and the
  validate/calculate/update `_with_diagnostics` entry points.
  `release/checklists/0.16.0-alpha.4.md` records state-machine diagnostics
  mapping for interruption and waypoint resume state without adding public
  symbols.
  `release/checklists/0.16.0-alpha.5.md` records the tracking public
  diagnostics getter ABI expansion for tracking and tracking sequence
  continuation state.
  `release/checklists/0.16.0-readiness.md` records readiness evidence for the
  190-symbol public diagnostics design line before stable release promotion.
  `release/checklists/0.16.0.md` records the stable public diagnostics release
  closeout.
  `release/checklists/post-v0.16.0-docs-and-examples-polish.md` records the
  post-release public diagnostics C example and current-docs polish slice.
  `release/checklists/post-v0.16.0-tooling-maintenance.md` records the
  tracking failure shrinker tooling slice.
  `release/checklists/post-v0.15.0-quality-audit.md` records the post-release
  code-quality and test-quality audit slice without changing the stable ABI or
  release state.
  `release/checklists/post-v0.15.0-state-machine-branch-coverage.md` records
  the targeted post-release state-machine branch coverage slice.
  `release/checklists/post-v0.15.0-solver-branch-coverage.md` records the
  targeted solver branch coverage and private calculate skeleton refactor
  slice. `release/checklists/post-v0.15.0-solver-adjacent-branch-coverage.md`
  records the follow-up solver-adjacent branch coverage slice for brake and
  lower-order step paths.
  `release/checklists/post-v0.15.0-random-repro-materialization.md` records
  the follow-up random replay/export materialization slice.
  `release/checklists/post-v0.15.0-review-followup-quality-hardening.md`
  records the external-review follow-up quality hardening slice for roots
  numeric audit, waypoint branch-queue saturation, private interrupt context,
  tracking split, profile context conversion, and low-risk cleanup.
  `release/checklists/post-v0.15.0-quality-evidence-refresh.md` records the
  evidence-only coverage and hotspot refresh after the review-followup
  hardening slice.
  `release/checklists/post-v0.15.0-random-shrinker-mvp.md` records the
  follow-up local shrink tooling slice for oracle random, oracle per-DoF
  random, and tracking random audit seed/sample reproductions.
  `release/checklists/post-v0.15.0-residual-branch-coverage.md` records the
  compact residual coverage slice for public `output.c` and `trajectory.c`
  boundary invariants.
  `release/checklists/post-v0.15.0-portability-static-audit.md` records the
  evidence-only portability and static-audit slice for ABI/export verification
  and platform-clock compile probes.
  `release/checklists/post-v0.15.0-quality-closeout.md` records the final
  post-release quality-series closeout and maintenance conclusion.
  `release/checklists/post-v0.15.0-next-design-readiness.md` records the
  docs-only readiness decision for starting public diagnostics design.
  `release/checklists/post-v0.15.0-failure-shrinker-prototype.md` records the
  local failure-preserving oracle shrinker prototype.
- `release/evidence/verification_report.md` - local and CI verification
  history.
- `release/evidence/performance_report.md` - performance procedure and release
  benchmark records.
- `release/notes/` - release notes source material, including the stable
  `0.8.0` visualization/gallery evidence release notes, the stable `0.9.0`
  tracking quality/stability evidence release notes, the stable `0.10.0`
  Visualization v2 evidence release notes, the stable `0.11.0` waypoint
  soft-interruption and platform-clock evidence release notes, the stable
  `0.12.0` waypoint soft-interruption true-resume release notes, the stable
  `0.13.0` waypoint true-resume stress/private-engine release notes, and the
  stable `0.14.0` API-neutral interrupt surface release notes, the stable
  `0.15.0` tracking sequence continuation release notes, and the stable
  `0.16.0` public diagnostics release notes.

`v0.16.0` is the current stable release after the public diagnostics readiness
audit, release-candidate evidence, annotated tag, tag CI, tag manual
release-random workflow, and GitHub Release publication. It promotes the
alpha.3 core diagnostics API, alpha.4 state/resume diagnostics mapping, and
alpha.5 tracking public diagnostics getters to the stable 190-symbol public C
ABI baseline. `v0.15.0` remains the
previous stable tracking sequence continuation release with a 184-symbol public
C ABI baseline.
The latest post-release residual coverage slice records branch coverage
`73.91%` at
`out/coverage/post-v0.15.0-residual-branch-coverage/coverage-summary.txt`
without changing the stable ABI or release state.
The latest portability/static audit records normal/shared CTest, Windows
ABI/export allowlist verification, and corrected platform-clock compile probes
without adding mandatory static-analysis CI or changing the stable ABI.
The post-`v0.15.0` quality series is now closed at branch coverage `73.91%`;
future coverage work is reserved for concrete regressions, oracle-backed cases,
public-behavior invariants, or separate design decisions.
The next design-readiness audit recommended public diagnostics as the first
`0.16.0` design topic while keeping wrappers, package recipes, and release
actions deferred until separately accepted.
The `0.16.0-alpha.1` public diagnostics design opened as docs-only work.
The `0.16.0-alpha.2` contract freeze locks diagnostics initialization,
`struct_size` compatibility, staged public API boundaries, and stable
coarse-grained diagnostic codes before any header or ABI allowlist change.
The `0.16.0-alpha.3` implementation adds the core opt-in public diagnostics
API and raises the design-line public symbol count from the `v0.15.0`
184-symbol baseline to 188 while keeping version/tag/release state unchanged.
The `0.16.0-alpha.4` implementation keeps that 188-symbol count and extends
`ruckig_update_with_diagnostics` with no-waypoint/waypoint interruption and
waypoint resume mismatch diagnostics.
The `0.16.0-alpha.5` implementation adds the two tracking public diagnostics
getter symbols and raises the expected design-line public symbol count to 190
without changing version/tag/release state.
The `0.16.0-readiness` evidence confirmed the public diagnostics line was ready
for stable-release promotion. The `v0.16.0` stable release bumps version
metadata and ABI artifact paths while keeping wrapper stabilization deferred.
The failure-oriented shrinker prototype adds local oracle random failure
shrinking for seed/sample debugging without writing generated fixtures or
changing library API.
The follow-up random shrinker MVP adds local deterministic shrink commands for
seed/sample materialization without changing random corpus semantics or writing
generated fixtures automatically.
`v0.14.0` stabilizes the API-neutral interrupt surfaces without expanding the
172-symbol public C ABI. `v0.13.0` stabilizes post-`v0.12.0` waypoint
true-resume stress coverage and the private waypoint optimizer engine rewrite.
`v0.12.0` stabilizes waypoint
`ruckig_update` soft-interruption true-resume and the unified private waypoint
optimizer engine. `v0.11.0` stabilizes
waypoint `ruckig_update` soft-interruption V1 and the internal platform clock
abstraction without expanding the public C ABI. `v0.10.0`
stabilizes the 30 local
`1400x900` Visualization v2 PNG assets, manifest, strict local verifier, and
manual-only CI artifact evidence without making visualization a default push/PR
gate. The previous v1 gallery provenance remains available through the
`v0.9.0` tag.

## Design Documents

- `design/per_dof_overrides.md` - per-DoF control/synchronization override
  design.
- `design/0.3.0_priorities.md` - design-only `0.3.0` priority ordering.
- `design/0.3.0_readiness.md` - pre-`0.3.0` readiness decisions after the
  final planned `0.2.x` stabilization release.
- `design/0.16.0_readiness.md` - post-`v0.15.0` readiness decision for the
  next design line, recommending public diagnostics first.
- `design/0.16.0_public_diagnostics.md` - docs-only public diagnostics design
  for possible opt-in `0.16.0` diagnostics APIs.
- `design/0.3.0_release_decision.md` - accepted hardening-release scope for
  `0.3.0`.
- `design/0.3.0_closeout_checklist.md` - engineering-hardening closeout
  checklist for `0.3.0-design`.
- `design/0.4.0_original_parity.md` - original-surface parity design for the
  local waypoint optimizer and public C ABI expansion.
- `design/tracking_interface.md` - accepted `v0.5.0` tracking API semantics,
  local Fast-mode behavior, and evidence strategy.
- `design/0.5.0_release_decision.md` - release-scope decision record for a
  possible Fast-only stable `v0.5.0`.
- `design/tracking_optimized_mode.md` - accepted `v0.6.0` design for bounded
  local `Optimized` tracking.
- `design/interrupt_calculation_duration.md` - current waypoint true-resume,
  no-waypoint complete-trajectory-boundary, and online tracking
  candidate-boundary interruption semantics plus historical `0.4.x`
  storage-only behavior.
- `design/future_interrupt_surfaces.md` - alpha.2 quasi-spec that later guided
  alpha.4 no-waypoint and alpha.5 online tracking interruption surfaces
  without public ABI changes; tracking sequence interruption remains deferred.
- `design/tracking_sequence_interruption_api.md` - `0.15.0` tracking sequence
  continuation design record, from alpha.2 public-carrier draft through the
  alpha.4 public API, alpha.5/alpha.6 behavior, and alpha.7 wrapper smoke
  evidence.
- `../bindings/rust/README.md` - experimental Rust alpha wrapper over the
  public C ABI.
- `../bindings/python_prototype/README.md` - experimental Python cffi
  ABI-mode prototype over the public C ABI.
- `design/python_bindings_feasibility.md` - Python binding feasibility, without
  implementation approval.
- `design/package_manager_feasibility.md` - frozen package-manager feasibility
  record, without recipes or active roadmap scope.
- `design/vcpkg_feasibility_prototype.md` - experimental vcpkg prototype
  reference, without adding a supported recipe.

## Technical References

- `technical/profile_check_conversion.md` - profile check conversion notes.
- `technical/callback_conversion.md` - callback/lambda conversion notes.
- `technical/cpp_to_c_conversion_table.md` - C++ to C conversion lookup table.

## Historical

- `historical/c_rewrite_execution_plan.md` - historical `0.1.0` rewrite
  execution plan. It is not the current scope source of truth.

## ABI Baselines

- `abi/public-symbols.txt` - approved public C ABI symbol allowlist generated
  from the public header.
- `abi/public-symbol-exceptions.txt` - approval file for intentional public
  symbol additions such as the `0.4.0` waypoint ABI, `0.5.0` tracking ABI, and
  `0.6.0` Optimized tracking API.
- `abi/exceptions.md` - public ABI exception policy.
- `abi/v0.2.2/` - exported-symbol baselines used by the `0.2.3` ABI
  comparison helper.
- `abi/v0.2.3/` - exported-symbol baselines used by the `0.2.4` ABI
  comparison helper.
- `abi/v0.2.4/` - exported-symbol baselines used by the `0.2.5` ABI
  comparison helper.
- `abi/v0.2.5/` - exported-symbol baselines used by the `0.3.0` ABI comparison
  helper and any future emergency `0.2.6`.
  `abi/v0.2.5/linux-symbol-review.md` classifies the historical Linux internal
  exports that are cleaned up during `0.3.0`.
