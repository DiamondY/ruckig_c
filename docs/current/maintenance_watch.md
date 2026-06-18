# Post-v0.16.0 Maintenance Watch

This document is the current watch list for maintenance work that should stay
event-driven after `v0.16.0`. It is not an active implementation roadmap.

## Active Stable Baseline

- Current stable release: `v0.16.0`.
- Public C ABI baseline: 190 symbols.
- Wrapper status: Python and Rust remain prototype-only.
- Upstream baseline status: `original/ruckig-main` remains the
  `0.17.3-line frozen baseline`.
- Package-manager recipes remain frozen.
- Constructor capacity overflow and invalid constructor `delta_time` handling
  are covered baseline risks after the post-`v0.16.0` quality hardening slices.

## Watch Triggers

| Item | Open only when |
| --- | --- |
| `0.18.0-upstream-baseline-upgrade-readiness` | A new upstream tag appears, a material solver/API/performance delta is found, an oracle mismatch appears, a user reports an upstream-fixed bug, or baseline provenance needs re-anchoring. |
| `post-v0.16.0-oracle-backed-solver-regression` | A reproducible oracle mismatch, public behavior regression, user report, or stable invariant justifies a fixed solver case. |
| `post-v0.16.0-tooling-generated-fixture-write` | Shrinker users explicitly need generated fixture auto-write rather than fixture-ready initializer output. |
| `0.16.1` | An emergency stable patch bug affects `v0.16.0`. |
| Package-manager recipes | External install demand exists and a maintenance owner accepts recipe support. |
| Wrapper stable publication | The wrapper route-selection work is superseded by an accepted Python-first, Rust-first, or dual-wrapper route with package/discovery ownership. |
| Static-analysis CI | A maintainer accepts the noise budget, platform availability, runtime cost, and triage ownership for clang-tidy, cppcheck, CodeQL, or formatter gates. |

## Explicit Non-Triggers

- Do not open coverage-percentage slices without oracle, public behavior, user
  report, or invariant evidence.
- Do not publish `0.16.1` for documentation polish, coverage numbers, examples,
  or developer tooling.
- Do not churn visualization assets without a separate visualization evidence
  slice.
- Do not commit to Cloud/Pro behavior that depends on unavailable upstream
  source.
- Do not update `original/ruckig-main` opportunistically during routine
  maintenance.
- Do not promote local `.clang-format` or `.clang-tidy` configuration into
  routine CI without a separate CI policy decision.

## Current Default

The default maintenance route is conservative:

- keep public C ABI and release state unchanged;
- keep wrappers prototype-only;
- keep package recipes frozen;
- keep upstream baseline frozen;
- require checked arithmetic for public constructor derived counts;
- add tests only when backed by public behavior, oracle evidence, a reproducible
  audit/shrinker sample, or a clear invariant.
