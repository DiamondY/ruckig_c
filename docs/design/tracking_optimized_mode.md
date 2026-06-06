# Tracking Optimized Mode Design Entry

`RUCKIG_TRACKING_OPTIMIZED` is declared in the `0.5.0-design` public C ABI for
API shape parity, but the alpha implementation returns
`RUCKIG_ERROR_UNSUPPORTED`. This document reserves the implementation work for
`0.6.0-design`.

## Scope Boundary

`Optimized` tracking is not part of `0.5.x`.

- Do not alias `Optimized` to Fast.
- Do not silently fall back to Fast.
- Do not add hidden network, cloud, or Pro dependencies.
- Do not make routine CI depend on external licenses or network access.
- Do not change existing `v0.5.0` public tracking symbols unless a new API
  decision is approved.

## Design Topics for `0.6.0-design`

- Objective function for tracking error, smoothness, and solver effort.
- Bounded local search or predictive optimizer over future target samples.
- Deterministic iteration order and reproducible results.
- Online update behavior with limited lookahead.
- Offline sequence behavior and result sequence semantics.
- Relationship between `reactiveness`, `look_ahead_cycles`, and optimized
  horizon length.
- Reuse of the existing target solver and trajectory constraints.
- No-allocation policy for prepared online and offline paths.
- Failure policy when no feasible optimized candidate exists.
- Whether a best feasible fallback is allowed and how it is reported.
- Timeout and interruption semantics if combined with future
  `interrupt_calculation_duration` work.
- Python and Rust wrapper mapping after the C behavior is stable.

## Evidence Strategy

There is no frozen Community source-level oracle for tracking optimized mode.
Routine evidence must therefore be local and deterministic:

- Fixed target-signal corpus.
- Online tracking lag and constraint metrics.
- Offline sequence continuity and constraint metrics.
- Fast baseline comparison for accepted scenarios.
- Deterministic repeat tests.
- No-allocation tests for prepared paths.
- ASan/UBSan and Valgrind coverage in CI.

Optional Pro/cloud black-box samples may be recorded manually as comparison
notes, but they must remain non-blocking and must not be used to claim formal
numerical equivalence.

## Acceptance Bar

`Optimized` should only move from design to implementation after:

- The objective and failure policy are written down.
- The bounded search budget is explicit.
- The interruption relationship is either implemented with tests or explicitly
  excluded.
- The fixed corpus and metric thresholds are approved.
- The implementation exports no internal symbols.
