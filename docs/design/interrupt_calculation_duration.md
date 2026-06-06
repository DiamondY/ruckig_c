# Interrupt Calculation Duration Design Notes

`interrupt_calculation_duration` is part of the public C API surface for
original API shape parity. In `0.4.x`, including `0.4.2`, it is a stored input
field only. The local waypoint optimizer does not yet implement interruption
checkpoints, best-feasible timeout fallback, or hard/soft real-time guarantees.

## Current 0.4.x Behavior

- The input can store and clear an interrupt calculation duration.
- The stored value is preserved through the public C accessors.
- No waypoint optimizer loop is interrupted solely because this value is set.
- `ruckig_output_was_calculation_interrupted` is not forced by the local
  optimizer budget in `0.4.x`.
- Release notes must not claim hard or soft real-time waypoint optimization.

## Future Semantics to Design

Before implementation, a separate design must decide these cases:

- Tiny budget: whether the optimizer may return a previous best feasible
  candidate, a newly found best feasible candidate, or an error.
- Zero or no budget: whether this disables interruption or requests immediate
  fallback.
- Best feasible candidate exists: how to set result code, output trajectory,
  and `was_calculation_interrupted`.
- No feasible candidate exists: whether to return invalid input, generic error,
  or a dedicated future diagnostic.
- Online recalculation: how to reuse previous best candidates without
  allocating or accepting stale constraints.
- Timeout during candidate generation: how to preserve deterministic search
  ordering and cleanup.
- Timeout during section evaluation: whether a partially evaluated complete
  waypoint candidate can ever be accepted. The default should be no.
- Determinism: repeated calls with the same input and budget should return the
  same accepted candidate on the same platform.
- Allocation: checkpoint and timeout paths must not allocate in the prepared
  online update path.

## Implementation Preconditions

Future implementation should not start until:

- The optimizer has explicit phase and candidate-loop checkpoints.
- The best-feasible candidate is stored in a reusable workspace.
- Tests cover tiny budget, no budget, feasible fallback, no-feasible fallback,
  online previous-best reuse, and lifecycle cleanup.
- Release notes can state the exact semantics without implying a hard real-time
  guarantee.

## 0.4.2 Acceptance

`0.4.2` is accepted when the storage-only behavior is documented clearly and
all release notes keep soft interruption as deferred work.
