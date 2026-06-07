# Tracking Optimized Mode Design

`v0.6.0` stabilizes a bounded local `RUCKIG_TRACKING_OPTIMIZED` MVP. The
`0.7.0-alpha` line deepens that implementation with high-level strategy
presets and stronger deterministic evidence. This remains a local bounded
candidate evaluator. It does not claim source-level oracle parity, formal
global optimality, or Pro/cloud numerical equivalence. The frozen Community
baseline still has no local tracking optimizer source to use as an oracle.

## Public C ABI

`v0.6.0` added intentional public C symbols on top of the stable `v0.5.0`
tracking ABI:

- `ruckig_tracking_set_max_optimized_candidates`
- `ruckig_tracking_get_max_optimized_candidates`
- `ruckig_tracking_get_last_calculation_status`
- `ruckig_tracking_get_last_candidate_count`
- `ruckig_tracking_update_with_lookahead`

The diagnostic enum remains:

- `RUCKIG_TRACKING_CALCULATION_NONE = 0`
- `RUCKIG_TRACKING_CALCULATION_FAST = 1`
- `RUCKIG_TRACKING_CALCULATION_OPTIMIZED = 2`
- `RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK = 3`
- `RUCKIG_TRACKING_CALCULATION_ERROR = 4`

`0.7.0-alpha` adds the approved high-level strategy enum:

- `RUCKIG_TRACKING_OPTIMIZED_STABLE = 0`
- `RUCKIG_TRACKING_OPTIMIZED_BALANCED = 1`
- `RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE = 2`

and the corresponding public C controls:

- `ruckig_tracking_set_optimized_strategy`
- `ruckig_tracking_get_optimized_strategy`

The default strategy after `ruckig_tracking_create` is
`RUCKIG_TRACKING_OPTIMIZED_BALANCED`. Invalid strategy values return
`RUCKIG_ERROR_INVALID_INPUT`. Existing `v0.6.0` public symbols, function
signatures, enum numeric values, and result-code numeric values remain
unchanged. Public symbol count for the `0.7.0-alpha` evidence line is `171`.

The default Optimized candidate budget remains `16`. The public budget setter
accepts `1..128`. The budget is deterministic and bounds candidate trajectory
evaluations per online step. Invalid budget values return
`RUCKIG_ERROR_INVALID_INPUT`.

## Strategy Presets

The strategy surface intentionally exposes presets rather than raw weights,
candidate-family masks, or tuning knobs:

- `Stable` is the conservative compatibility preset. It keeps scoring closest
  to the `v0.6.0` MVP and avoids aggressive candidate prioritization. It is not
  a bit-for-bit output guarantee.
- `Balanced` is the default. It preserves the default cost budget of `16`
  candidates and prioritizes no-regression against the Fast baseline across the
  fixed local corpus.
- `Aggressive` uses the same public budget but gives stronger priority to
  horizon position/terminal tracking error and enables all bounded candidate
  families within the configured budget.

All presets preserve the same public fallback policy: if no valid Optimized
candidate improves the Fast baseline by `RUCKIG_TRACKING_SCORE_EPSILON`, the
calculation returns the Fast result and reports
`RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK`.

## Behavior

Fast mode remains the stable constant-acceleration lookahead implementation
from `v0.5.0`.

Optimized mode:

- Uses `ruckig_tracking_update_with_lookahead` for online calls with a target
  sequence. Sequence index `0` is the current target sample and later entries
  are future samples.
- Uses single-sample lookahead when callers use the existing
  `ruckig_tracking_update` API in Optimized mode.
- Uses a sliding window in `ruckig_tracking_calculate_sequence`: each offline
  step sees at most `look_ahead_cycles` samples from the current target index.
- Reuses the existing target solver for each candidate target state.
- Evaluates candidates in deterministic order and stops when
  `last_candidate_count == max_optimized_candidates`.
- Rejects solver-failing or numerically invalid candidates. Penalties are only
  used to rank valid candidates, never to accept invalid output.
- Falls back to the Fast baseline when no Optimized candidate improves the
  objective. Fallback success returns the normal `RUCKIG_WORKING` or
  `RUCKIG_FINISHED` result; callers use
  `ruckig_tracking_get_last_calculation_status` to see the fallback.
- Returns existing solver errors when even the Fast baseline is infeasible.
  The public result enum is unchanged.

The candidate families are bounded and generated in deterministic order:

- Fast baseline.
- Instantaneous window samples.
- Horizon-predicted samples.
- Terminal blends.
- Derivative-damped terminal candidates.
- Strategy-specific lead/lag horizon variants.

For offline sequence calculation, `last_calculation_status` is aggregate:
if any successful step used Fast fallback, the aggregate status is
`FAST_FALLBACK`; if all steps used an improved Optimized candidate, the status
is `OPTIMIZED`. `last_candidate_count` is the total evaluated candidate count
across all offline steps.

## Scoring

The score is a weighted horizon objective over feasible candidate outputs:

- Position, velocity, and acceleration error against the lookahead samples.
- Stronger terminal-sample weighting.
- Jerk effort as a secondary smoothness term or tie-break component.
- Deterministic tie-break: a candidate must improve by
  `RUCKIG_TRACKING_SCORE_EPSILON`; otherwise the earlier candidate wins.

`Aggressive` increases position and terminal weighting and lowers jerk-effort
priority relative to `Balanced`. `Stable` keeps conservative weights and
candidate-family selection. The implementation must not accept an invalid
candidate through a penalty score.

## No-Allocation Policy

The tracking handle owns the internal solver, work input/output, candidate
scratch arrays, and best-candidate scratch arrays. Constructors prepare the
default Optimized workspace. Candidate-budget and strategy setters run outside
the online/offline calculation path. Prepared online and offline calculation
paths must not allocate.

`0.7.0-alpha` does not implement timeout checkpoints. The
`interrupt_calculation_duration` field does not create hard or soft real-time
interruption behavior for tracking.

## Evidence Strategy

Routine evidence is local and deterministic:

- C API lifecycle and validation tests, including strategy setter/getter and
  invalid strategy values.
- Online Optimized single-target and lookahead tests across all three
  strategies.
- Offline Optimized sliding-window sequence tests across all three strategies.
- Fallback and error diagnostic tests.
- Quality gates: Balanced must be no worse than Fast baseline on fixed ramp,
  constant-acceleration, sinus, and half-sinus cases; Aggressive must improve
  over Balanced by at least `1%` on fixed sinus and half-sinus cases.
- Trend metrics print average, max, final error, improvement ratio, candidate
  count, fallback count, and strategy.
- Routine deterministic stress uses
  `ruckig_c_tests --tracking-random 10000 --seed 1`.
- Manual/release deterministic stress uses
  `ruckig_c_tests --tracking-random 100000 --seed 1`.
- No-allocation tests for prepared Optimized online and offline paths.
- Python `cffi` prototype smoke.
- Rust alpha wrapper smoke and examples.
- ABI allowlist and exported-symbol checks. Approved public symbol count is
  `171`.

Optional Pro/cloud black-box samples may be recorded manually as comparison
notes, but they remain non-blocking and cannot be used to claim formal
equivalence.

## Deferred

- Formal proof of global optimality.
- Formal Pro/cloud numerical equivalence.
- Cloud/remote calculation.
- Soft interruption checkpoints.
- Published Python wheels or Rust crate.
- Package-manager recipes.
- Upstream baseline upgrade.
