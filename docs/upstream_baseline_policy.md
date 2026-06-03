# Upstream Baseline Policy

`original/ruckig-main` is frozen as the Ruckig Community `0.17.3` oracle
baseline for `ruckig_c 0.1.x` and `0.2.x` until an explicit upstream baseline
upgrade project is approved and completed.

## Rules

- Ordinary maintenance commits must not modify files under `original/ruckig-main`.
- `0.1.x` stability work and `0.2.x` maintenance work must use the frozen
  baseline for all oracle comparisons.
- Bug fixes in the C rewrite must be validated against the frozen oracle unless
  a documented oracle/platform tolerance exception is required.
- Release closeout, per-DoF hardening, bindings design, `0.2.x` patch-release
  work, and ordinary bug-fix work must not update the upstream baseline.

## Upstream Upgrade Requirements

Any future Ruckig baseline upgrade must be a separate project. It must include:

- Source inventory review against the new upstream version.
- Public API and deferred-scope review.
- Numerical tolerance review.
- Fixed oracle corpus update.
- Full deterministic random stress rerun.
- Static/shared build verification.
- Sanitizer and Valgrind verification where available.
- New Windows and Linux performance baselines.

The upgrade must not be mixed with bug fixes, release hardening, bindings,
waypoints, or per-DoF override hardening.
