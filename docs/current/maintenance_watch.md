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
- Public waypoint derived count overflow is covered as a defense-in-depth
  baseline after the review follow-up hardening slice.
- Selected private tracking/waypoint/trajectory derived count paths are covered
  by local checked arithmetic and corrupted-state selector evidence.
- Platform timing uses strict monotonic sources only: custom provider, Windows
  QueryPerformanceCounter, or POSIX `CLOCK_MONOTONIC`. CPU-time `clock()`
  fallback is not part of the baseline.
- Allocation audit counters are documented as local, non-thread-safe
  instrumentation; no atomics or locks are planned unless a future threaded
  audit requirement is accepted.
- Local `.clang-tidy` and portable preset guidance are available for targeted
  developer use; they remain manual evidence, not default CI gates.
- Static-analysis evidence policy is documented; default CI still excludes
  clang-tidy, cppcheck, CodeQL, formatter gates, and coverage upload.
- `RUCKIG_C_ENABLE_CALCULATION_DURATION` is a library-build option only. The
  installed CMake target must not propagate it through public usage
  requirements.
- External code-quality review triage is documented; large-file complexity,
  static-analysis CI, platform matrix expansion, and solver documentation work
  remain scoped by concrete triggers rather than broad churn.
- Selected third-order position solver hotspot comments are now covered; future
  comments should stay focused on branch-family intent, formula provenance, or
  invariants.
- The remaining review-adopted brake timing epsilon is named in the private
  precision header; future precision-constant cleanup should stay semantic,
  oracle-backed, and low-risk.
- The post-`v0.16.0` quality tightening series is closed out; remaining quality
  expansion is event-driven.

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
| Thread-safe allocation audit counters | A concrete multi-threaded audit use case needs consistent aggregate allocation statistics across threads. |
| CI matrix expansion | A maintainer accepts owner, runtime, and triage cost for MSVC full matrix, Windows sanitizer, macOS sanitizer/oracle/performance, coverage upload, or new static-analysis jobs. |
| Solver white-box coverage push | A reproducible oracle mismatch, public behavior regression, user report, or stable invariant justifies cases; coverage percentage alone is insufficient. |
| Non-standard platform clock support | A concrete platform lacks Windows QPC/POSIX `CLOCK_MONOTONIC` and cannot use the compile-time custom monotonic provider hook. |
| Solver hotspot documentation | A reviewer or maintainer identifies a branch family, formula source, invariant, or candidate-order rationale that is hard to understand and can be clarified without changing formulas. |
| Solver/module splitting | A concrete ownership boundary, duplicated logic, testability blocker, or regression-prone hotspot justifies a small refactor; file length alone is insufficient. |
| Precision-constant cleanup | A remaining literal has a stable semantic name, the exact value can be preserved, and normal/oracle-backed evidence can prove no numeric behavior change. |
| Private derived-count cleanup | A touched private helper computes buffer lengths from counts or sections and can be hardened locally without changing candidate order, scoring, or public behavior. |

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
- Do not add cppcheck, CodeQL, formatter, or coverage-upload gates merely
  because local configuration exists.
- Do not add locks or atomics to allocation counters for hypothetical threaded
  audit use without a concrete consumer.
- Do not expand default CI for coverage upload, static analysis, sanitizer
  matrix, or platform-matrix polish without an accepted CI policy slice.
- Do not open solver white-box coverage work solely to improve percentages.
- Do not reintroduce `clock()` CPU-time fallback for duration or interruption
  timing.
- Do not split solver files or move formula coefficients into constants merely
  to reduce line count or literal-count metrics.
- Do not rename remaining numeric literals unless the semantic name is clear
  and the diff can preserve exact numeric behavior.

## Current Default

The default maintenance route is conservative:

- keep public C ABI and release state unchanged;
- keep wrappers prototype-only;
- keep package recipes frozen;
- keep upstream baseline frozen;
- require checked arithmetic for public constructor derived counts;
- require checked arithmetic for touched private derived counts before buffer
  copy, finite-vector validation, or trajectory profile copying when the
  product can be expressed locally;
- require monotonic timing sources for private platform timing;
- add tests only when backed by public behavior, oracle evidence, a reproducible
  audit/shrinker sample, or a clear invariant.
- add solver comments only when they explain stable intent, candidate families,
  formula provenance, or invariants that reviewers are likely to misread.
- add precision constants only when the name captures an implementation
  contract and does not change numeric behavior.
