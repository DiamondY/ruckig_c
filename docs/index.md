# Documentation Index

This is the entry point for repository documentation. Current scope is defined
by this README, the public header, and the documents under `docs/current/`.
Historical rewrite plans and release evidence are retained for traceability.

## Current Maintainer Entry Points

- `current/roadmap.md` - shipped scope, maintenance queues, current stable
  release scope, the `v0.14.0` release closeout line, and future design
  boundaries.
- `current/api_compatibility.md` - patch-release API/ABI policy and
  exported-symbol review.
- `current/api_diagnostics.md` - C API validation and diagnostics behavior.
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
  stable `0.14.0` API-neutral interrupt surface release notes.

`v0.13.0` is the current stable release before `v0.14.0` publication. Current
`main` is in `v0.14.0` stable release closeout after completed local and
remote `0.14.0-readiness` evidence. Alpha.4 added no-waypoint interruption and
alpha.5 added online tracking interruption without expanding the public C ABI.
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
- `../bindings/rust/README.md` - experimental Rust alpha wrapper over the
  public C ABI.
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
