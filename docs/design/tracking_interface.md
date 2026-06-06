# Tracking Interface Design Preparation

This document records the accepted direction for tracking interface work after
`v0.4.2`. It is design preparation only. `0.4.2` must not add public tracking
symbols or implement tracking behavior.

## Source Inventory

The frozen Community baseline under `original/ruckig-main` does not include
`include/ruckig/trackig.hpp`. Tracking appears in the README and in Pro-only
examples:

- `original/ruckig-main/examples/14_tracking.cpp`
- `original/ruckig-main/examples/15_tracking_offline.cpp`
- `original/ruckig-main/examples/14_tracking.py`
- `original/ruckig-main/examples/15_tracking_offline.py`

Because the local tracking implementation source is not present in the frozen
Community baseline, future work cannot claim source-level oracle parity with
original tracking internals. Tracking is still a required full-original-parity
gap because it is part of the original product surface.

## Accepted Scope

- Implement tracking in `0.5.0-design`, not `0.4.2`.
- Add a local implementation. Do not add a cloud client or remote fallback.
- Add public C API only after a separate `0.5.0` API decision.
- Keep existing `v0.4.x` public functions, signatures, enum values, and result
  codes unchanged.
- Extend Python `cffi` and Rust alpha wrappers only after the C tracking ABI is
  accepted.

## C ABI Direction

The future C surface should map the original concepts without exposing C++
templates:

- `Trackig` becomes an opaque `ruckig_tracking_t` or similarly named handle.
- `TargetState` becomes an opaque or flat-array C target-state object that owns
  position, velocity, and acceleration arrays for a fixed DoF count.
- `TrackigMode` becomes a public enum with at least `Fast` and `Optimized`.
- Online tracking updates accept a target state, an existing input handle, and
  an output handle.
- Offline tracking accepts a finite sequence of target states and writes a
  sequence of output states or a trajectory-like result object.

The exact symbol names are intentionally not finalized in `0.4.2`. The
`0.5.0-design` API proposal must include the full symbol list, ownership
rules, result-code mapping, and ABI allowlist exceptions.

## Behavior Goals

Online tracking:

- The caller provides the current input state and constraints.
- The caller provides a target signal state with position, velocity, and
  acceleration for every DoF.
- The tracker predicts ahead from the target state and feeds a target-solver
  calculation that reduces lag relative to directly chasing the instantaneous
  target position.
- `output_pass_to_input` remains the normal loop handoff.

Offline tracking:

- The caller provides an ordered finite list of target states.
- The tracker returns a smooth output sequence with one output per accepted
  target sample unless a documented validation failure occurs.
- `look_ahead_cycles` may be used only after deterministic memory and
  validation rules are defined.

Mode policy:

- `Fast` should prioritize deterministic low overhead and simple prediction.
- `Optimized` should allow a higher-quality local search, but it must remain
  bounded and deterministic.
- `reactiveness` is constrained to `[0, 1]`; values outside the range should
  be rejected or clamped only if that policy is explicitly approved.

## Validation and Ownership

- Tracking requires finite target position, velocity, and acceleration arrays.
- DoF count must match the tracking handle and input/output handles.
- Kinematic constraints reuse the existing input validation rules.
- Large buffers must be allocated during construction or setter calls, not in
  the prepared online update path.
- Destroying a tracking handle must be idempotent for `NULL` and must not own
  caller-provided input or output handles.
- Python and Rust wrappers must enforce lifecycle errors before calling into C
  after close/drop.

## Evidence Strategy

Routine `0.5.0-design` evidence should be local and deterministic:

- Fixed examples for ramp, constant-acceleration, sinusoidal, and half-sinus
  target signals.
- Lag/error metrics against naive instantaneous target chasing.
- Constraint sampling for position, velocity, acceleration, and jerk.
- Validation tests for shape, finite values, DoF mismatch, and invalid modes.
- Online lifecycle tests for repeated update/pass-to-input loops.
- Offline sequence tests for output count, continuity, and deterministic
  results.

Optional Ruckig Pro/cloud black-box samples may be recorded as non-blocking
evidence if available. They must not become routine CI, release gates, or a
source-level equivalence claim.

## Non-Goals for 0.4.2

- No tracking public C API.
- No tracking implementation.
- No Python or Rust tracking wrapper.
- No Pro/cloud equivalence claim.
- No cloud or remote calculation.
