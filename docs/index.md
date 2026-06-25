# Documentation Index

This is the entry point for repository documentation. Current scope is defined
by this README, the public header, and the documents under `docs/current/`.
Historical rewrite plans and release evidence are retained for traceability.

## Current Maintainer Entry Points

- `current/status.md` - short current-state entry point for maintainers:
  stable baseline, supported/frozen scope, local verification commands, and
  touched-file verification matrix.
- `current/roadmap.md` - shipped scope, maintenance queues, current stable
  release scope, the `0.16.0` release line, and future design boundaries.
- `current/api_compatibility.md` - patch-release API/ABI policy and
  exported-symbol review.
- `current/api_diagnostics.md` - C API validation and diagnostics behavior.
- `current/code_quality_audit.md` - post-`v0.15.0` code-quality and
  test-quality risk map, state-machine and solver branch coverage evidence,
  external-review follow-up hardening evidence, local quality gates, local
  static-analysis configuration policy, and random replay/materialization
  policy.
- `current/ci_quality_policy.md` - default CI quality coverage, local/manual
  evidence policy, and owner-gated triggers for future CI expansion.
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
- `current/upstream_delta_audit.md` - post-`v0.16.0` audit of the frozen
  upstream tree against current upstream tags and HEAD.
- `current/maintenance_watch.md` - post-`v0.16.0` event-driven maintenance
  watch list and explicit non-triggers.
- `design/0.18.0_upstream_baseline_provenance_readiness.md` - docs-only
  readiness record for upstream baseline provenance.
- `historical/README.md` - historical documentation maintenance policy and
  active-line boundaries.

## Release Evidence

- `release/checklists/` - historical release and maintenance-slice checklists
  by version. These files are evidence archives for the time they were
  produced, not the current-state entry point; start from
  `current/status.md` for today's baseline.
- `release/evidence/` - release-readiness, verification, performance,
  visualization, ABI, and local/manual evidence artifacts. These records are
  retained for traceability and should not be rewritten as current status.
- `v0.16.0` is the current stable release line. The current maintainer entry
  points are `current/status.md`, `current/roadmap.md`,
  `current/maintenance_watch.md`, and `current/ci_quality_policy.md`.
- Older release checklists, including the `v0.9.0` tracking
  quality/stability line, remain historically authoritative for the facts they
  recorded. Use them as evidence references, not as the default maintenance
  baseline.

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
- `design/0.17.0_wrapper_stabilization_readiness.md` - docs-only wrapper
  stabilization criteria for possible future Python/Rust package work.
- `design/0.17.0_wrapper_stabilization_decision.md` - docs-only decision to
  keep wrappers prototype-only until package/discovery ownership is accepted.
- `design/0.17.0_wrapper_package_policy.md` - docs-only package/discovery
  policy blockers for future wrapper stabilization.
- `design/0.17.0_wrapper_route_selection.md` - docs-only wrapper route
  selection; keeps wrappers prototype-only unless a future route-specific
  implementation is accepted.
- `design/0.18.0_upstream_baseline_provenance_readiness.md` - docs-only
  upstream baseline provenance readiness; keeps the baseline frozen without
  re-labeling it as a post-tag snapshot.
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
