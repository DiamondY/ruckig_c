# Tracking Stability Regression Evidence

This document records the `0.9.0-alpha.3` tracking stability regression
evidence on `0.9.0-design - Unreleased`. It freezes representative
`0.9.0-alpha.2` tuned Optimized tracking behavior before any later readiness
audit.

## Scope

- Public C API additions: `0`.
- Public symbol additions: `0`.
- Public diagnostics struct additions: `0`.
- Evaluator scoring or near-tie policy changes: `0`.
- Tags, GitHub Releases, manual release-random workflows: none.
- Frozen original source changes: none.

The new evidence surface is test-runner only:

```powershell
out\build\windows-clang-ninja\ruckig_c_tests.exe --tracking-stability
```

CTest runs the same fixed corpus through `ruckig_c_tracking_stability`.

## Stability Corpus

The fixed corpus uses deterministic tuples from the alpha.2 audit harness. It
covers all strategies, all tracking signals, all lookahead buckets,
disabled-DoF and enabled-only cases, default and `tight_valid` constraints,
optimized and Fast fallback statuses, budget exhaustion, Aggressive near-tie
selection, and every selected candidate family currently exposed by alpha.2:
`fast`, `instantaneous`, `horizon`, `terminal_blend`,
`derivative_damped`, and `lead_lag`.

Representative selected-family cases are hard-coded from
`--tracking-random-audit 100000 --seed 1`:

```text
sample 1: derivative_damped, Stable, 4 DoF, ramp, lookahead 1, tight_valid
sample 8: horizon, Balanced, 2 DoF, sinus, lookahead 10
sample 403: lead_lag, Aggressive, 2 DoF, half_sinus, lookahead 1, disabled DoF
sample 602: terminal_blend, Balanced, 2 DoF, half_sinus, lookahead 5, disabled DoF
sample 1400: Aggressive near-tie, instantaneous, 2 DoF, constant_acceleration, lookahead 5, disabled DoF
sample 1614: Fast fallback, Aggressive, 8 DoF, ramp, lookahead 10
```

## Assertions

Each case reuses the alpha.2 audit harness and then asserts its expected
behavior class:

- calculation succeeds and output constraints hold.
- public diagnostics mode, strategy, status, counters, scores, and reserved
  fields remain internally consistent.
- private attribution sums match public diagnostics candidate counts.
- exactly one selected candidate family is reported.
- fallback cases remain Fast fallback with selected family `fast`.
- optimized cases remain optimized.
- selected near-tie occurs only for Aggressive and has nonzero near-tie
  attribution.
- budget-exhausted representatives keep nonzero budget-exhaustion counters.

The fixed cases avoid full floating-point score snapshots. They assert coarse
diagnostics class, selected family, near-tie flag, and improvement sign only
where alpha.2 already required improvement.

## Interpretation

`0.9.0-alpha.3` is stability evidence, not readiness or stable closeout. It
does not raise a Pro/cloud equivalence claim and does not prove formal global
optimality for Optimized tracking. The intended next step, if the alpha.3
evidence and ordinary CI remain green, is a separate `0.9.0-readiness` audit.

That separate `0.9.0-readiness` audit now records full local stable-review
evidence for the alpha, alpha.2, and alpha.3 tracking work. It treats this
stability corpus as a candidate stable regression gate while keeping the public
C ABI unchanged at 172 symbols and leaving version bump, tag, GitHub Release,
and manual release-random workflow work to a later stable closeout decision.
