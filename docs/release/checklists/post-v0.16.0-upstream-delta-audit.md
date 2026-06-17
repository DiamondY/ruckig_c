# Post-v0.16.0 Upstream Delta Audit Checklist

This checklist records the upstream delta audit after `v0.16.0`. It does not
update `original/ruckig-main`, change oracle tolerances, modify tests, change
the public C ABI, or start an upstream baseline upgrade.

## Scope

- [x] Added `docs/current/upstream_delta_audit.md`.
- [x] Reviewed local `original/ruckig-main` as a frozen source tree.
- [x] Queried upstream tags and HEAD without modifying the local baseline.
- [x] Compared upstream `v0.17.3..HEAD` in a temporary clone.
- [x] Recorded material delta categories and follow-up recommendation.

## Findings

- [x] No upstream tag newer than `v0.17.3` was observed.
- [x] Upstream HEAD was `f48cf5fe8c48083b88b7ceda9a069dd5565d0d38`.
- [x] Temporary upstream clone described HEAD as `v0.17.3-7-gf48cf5fe8c48`.
- [x] Post-tag changes touched README/examples/plotting, Cloud client error
  handling, C++ position-limit and position-extrema API shape, upstream
  wrappers, and related upstream tests.
- [x] No post-tag delta was observed in analytical solver step or brake source
  files.
- [x] Local checked code files matched upstream HEAD for the material post-tag
  code changes when ignoring CRLF differences.

## Decision

- [x] Do not update `original/ruckig-main` in this slice.
- [x] Do not change oracle baseline, tolerances, public C ABI, or tests.
- [x] If baseline provenance needs regularization, open a separate
  `0.18.0-upstream-baseline-upgrade-readiness` slice.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed; no work to do |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_header_c\|ruckig_c_header_cpp\|ruckig_c_public_diagnostics"` | Passed; 3/3 tests |
| Public header / ABI / workflow boundary diff | Empty |
| `original/ruckig-main` and visualization asset boundary diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |
