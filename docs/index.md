# Documentation Index

This is the entry point for repository documentation. Current scope is defined
by this README, the public header, and the documents under `docs/current/`.
Historical rewrite plans and release evidence are retained for traceability.

## Current Maintainer Entry Points

- `current/roadmap.md` - shipped scope, maintenance queues, and future design
  boundaries.
- `current/api_compatibility.md` - patch-release API/ABI policy and
  exported-symbol review.
- `current/api_diagnostics.md` - C API validation and diagnostics behavior.
- `current/packaging.md` - installed CMake, pkg-config, static, DLL, and shared
  install-tree consumer notes.
- `current/upstream_baseline_policy.md` - frozen upstream oracle baseline
  policy.

## Release Evidence

- `release/checklists/` - release checklists by version.
- `release/evidence/verification_report.md` - local and CI verification
  history.
- `release/evidence/performance_report.md` - performance procedure and release
  benchmark records.
- `release/notes/` - release notes source material.

## Design Documents

- `design/per_dof_overrides.md` - per-DoF control/synchronization override
  design.
- `design/0.3.0_priorities.md` - design-only `0.3.0` priority ordering.
- `design/python_bindings_feasibility.md` - Python binding feasibility, without
  implementation approval.
- `design/package_manager_feasibility.md` - package-manager feasibility, without
  recipes.

## Technical References

- `technical/profile_check_conversion.md` - profile check conversion notes.
- `technical/callback_conversion.md` - callback/lambda conversion notes.
- `technical/cpp_to_c_conversion_table.md` - C++ to C conversion lookup table.

## Historical

- `historical/c_rewrite_execution_plan.md` - historical `0.1.0` rewrite
  execution plan. It is not the current scope source of truth.

## ABI Baselines

- `abi/v0.2.2/` - exported-symbol baselines used by the `0.2.3` ABI
  comparison helper.
- `abi/v0.2.3/` - exported-symbol baselines used by the `0.2.4` ABI
  comparison helper.
- `abi/v0.2.4/` - exported-symbol baselines used by the `0.2.5` ABI
  comparison helper.
