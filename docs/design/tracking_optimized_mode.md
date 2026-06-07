# Tracking Optimized Mode Design

`v0.6.0` stabilizes a bounded local `RUCKIG_TRACKING_OPTIMIZED` MVP. This does
not claim source-level oracle parity, formal global optimality, or Pro/cloud
numerical equivalence. The frozen Community baseline still has no local
tracking optimizer source to use as an oracle.

## Public C ABI Additions

The release adds intentional public C symbols on top of the stable `v0.5.0`
tracking ABI:

- `ruckig_tracking_set_max_optimized_candidates`
- `ruckig_tracking_get_max_optimized_candidates`
- `ruckig_tracking_get_last_calculation_status`
- `ruckig_tracking_get_last_candidate_count`
- `ruckig_tracking_update_with_lookahead`

The new diagnostic enum is:

- `RUCKIG_TRACKING_CALCULATION_NONE = 0`
- `RUCKIG_TRACKING_CALCULATION_FAST = 1`
- `RUCKIG_TRACKING_CALCULATION_OPTIMIZED = 2`
- `RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK = 3`
- `RUCKIG_TRACKING_CALCULATION_ERROR = 4`

The default Optimized candidate budget is `16`. The public setter accepts
`1..128`. The budget is deterministic and bounds the number of candidate
trajectory evaluations per online step. Invalid budget values return
`RUCKIG_ERROR_INVALID_INPUT`.

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
- Scores feasible candidates with a horizon tracking-error objective:
  position and velocity error are primary, acceleration and jerk effort are
  secondary, and the terminal lookahead sample receives the strongest weight.
- Rejects solver-failing or numerically invalid candidates. Penalties are only
  used to rank valid candidates, never to accept invalid output.
- Falls back to the Fast baseline when no Optimized candidate improves the
  objective. Fallback success returns the normal `RUCKIG_WORKING` or
  `RUCKIG_FINISHED` result; callers use
  `ruckig_tracking_get_last_calculation_status` to see the fallback.
- Returns existing solver errors when even the Fast baseline is infeasible.
  The public result enum is unchanged.

For offline sequence calculation, `last_calculation_status` is aggregate:
if any successful step used Fast fallback, the aggregate status is
`FAST_FALLBACK`; if all steps used an improved optimized candidate, the status
is `OPTIMIZED`.

## No-Allocation Policy

The tracking handle owns the internal solver, work input/output, candidate
scratch arrays, and best-candidate scratch arrays. Constructors prepare the
default Optimized workspace. The candidate-budget setter validates the budget
outside the calculation path. Prepared online and offline calculation paths must
not allocate.

`v0.6.0` does not implement timeout checkpoints. `interrupt_calculation_duration`
does not create hard or soft real-time interruption behavior for tracking.

## Evidence Strategy

Routine evidence is local and deterministic:

- C API lifecycle and validation tests.
- Online Optimized single-target and lookahead tests.
- Offline Optimized sliding-window sequence tests.
- Fallback and error diagnostic tests.
- Quality dominance against the Fast baseline for fixed ramp and
  constant-acceleration cases; sinus/half-sinus remain trend metrics until a
  later threshold decision.
- No-allocation tests for prepared Optimized online and offline paths.
- Python `cffi` prototype smoke.
- Rust alpha wrapper smoke and examples.
- ABI allowlist and exported-symbol checks.

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
