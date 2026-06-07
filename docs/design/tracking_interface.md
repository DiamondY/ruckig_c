# Tracking Interface API and Fast-Mode Semantics

This document records the accepted `v0.5.0` tracking interface scope. It
supersedes the `v0.4.2` design-preparation note and the `0.5.0-design` alpha
evidence notes. `v0.5.0` stabilizes local Fast-mode online/offline tracking.

## Source Inventory

The frozen Community baseline under `original/ruckig-main` does not include
`include/ruckig/trackig.hpp`. Tracking appears only in README text and Pro-only
examples:

- `original/ruckig-main/examples/14_tracking.cpp`
- `original/ruckig-main/examples/15_tracking_offline.cpp`
- `original/ruckig-main/examples/14_tracking.py`
- `original/ruckig-main/examples/15_tracking_offline.py`

Because local tracking source is not present in the frozen Community baseline,
`ruckig_c` cannot claim source-level tracking oracle parity. Routine evidence
is local and deterministic; optional Pro/cloud black-box samples may be
recorded manually but are never CI gates or release blockers.

## Accepted C ABI

The tracking API adds intentional public C symbols in `v0.5.0` while keeping
all `v0.4.2` functions, signatures, enum values, and result-code values
unchanged. The approved symbols are listed in `docs/abi/public-symbols.txt`,
and the intentional additions are recorded in
`docs/abi/public-symbol-exceptions.txt`.

Public concepts:

- `ruckig_tracking_t`: opaque tracking handle with DoF count, `delta_time`,
  mode, `reactiveness`, and `look_ahead_cycles`.
- `ruckig_target_state_t`: opaque per-sample target state with position,
  velocity, and acceleration arrays.
- `ruckig_target_state_sequence_t`: preallocated offline input sequence with
  flat `capacity * dofs` position, velocity, and acceleration arrays.
- `ruckig_tracking_output_sequence_t`: preallocated offline output sequence
  with flat position, velocity, acceleration, jerk, time, section, and result
  arrays.
- `ruckig_tracking_mode_t`: `RUCKIG_TRACKING_FAST = 0` and
  `RUCKIG_TRACKING_OPTIMIZED = 1`.

Approved symbol groups:

- Tracking handle:
  `ruckig_tracking_create`, `ruckig_tracking_destroy`,
  `ruckig_tracking_get_dof_count`, `ruckig_tracking_get_delta_time`,
  `ruckig_tracking_set_mode`, `ruckig_tracking_get_mode`,
  `ruckig_tracking_set_reactiveness`, `ruckig_tracking_get_reactiveness`,
  `ruckig_tracking_set_look_ahead_cycles`, and
  `ruckig_tracking_get_look_ahead_cycles`.
- Target state:
  `ruckig_target_state_create`, `ruckig_target_state_destroy`,
  `ruckig_target_state_get_dof_count`, mutable position/velocity/acceleration
  data accessors, and matching const accessors.
- Target state sequence:
  `ruckig_target_state_sequence_create`,
  `ruckig_target_state_sequence_destroy`,
  `ruckig_target_state_sequence_get_dof_count`,
  `ruckig_target_state_sequence_get_capacity`,
  `ruckig_target_state_sequence_get_count`,
  `ruckig_target_state_sequence_set_count`,
  `ruckig_target_state_sequence_clear`, mutable flat position/velocity/
  acceleration data accessors, and matching const accessors.
- Tracking output sequence:
  `ruckig_tracking_output_sequence_create`,
  `ruckig_tracking_output_sequence_destroy`,
  `ruckig_tracking_output_sequence_get_dof_count`,
  `ruckig_tracking_output_sequence_get_capacity`,
  `ruckig_tracking_output_sequence_get_count`,
  `ruckig_tracking_output_sequence_clear`, and const accessors for flat new
  position, velocity, acceleration, jerk, time, section, and result arrays.
- Calculation:
  `ruckig_tracking_update` and `ruckig_tracking_calculate_sequence`.

Online tracking uses:

```c
ruckig_tracking_update(tracking, target_state, input, output)
```

Offline tracking uses:

```c
ruckig_tracking_calculate_sequence(tracking, target_sequence, input, output_sequence)
```

The offline API returns an output state sequence, not a `ruckig_trajectory_t`.
For successful offline calculation, the output sequence count equals the
target sequence count. Each step records the sampled state after one tracking
cycle, `time = (step + 1) * delta_time`, the output section, and the step
result code. If an error occurs after some steps, the output sequence count is
the number of fully written successful steps.

## Fast-Mode Behavior

`Fast` mode is implemented locally with constant-acceleration lookahead:

```text
h = look_ahead_cycles * delta_time * reactiveness
predicted_position = p + v*h + 0.5*a*h*h
predicted_velocity = v + a*h
predicted_acceleration = a
```

Defaults:

- `mode = RUCKIG_TRACKING_FAST`
- `reactiveness = 1.0`
- `look_ahead_cycles = 1`

`reactiveness = 0` tracks the instantaneous target state. `reactiveness = 1`
uses the full configured lookahead horizon. `Optimized` mode is declared for
API shape parity but returns `RUCKIG_ERROR_UNSUPPORTED` throughout `0.5.x`;
implementation is deferred to `0.6.0-design`.

The local implementation reuses the existing target solver. It copies the
caller input into internal tracking workspace, writes the predicted target
state there, and calls the existing update path. The caller input is not
modified by `ruckig_tracking_update`; callers continue to use
`ruckig_output_pass_to_input` for online loop handoff.

## Validation and Ownership

- Tracking requires position control input. Velocity-control tracking and
  per-DoF control-interface overrides are rejected in `v0.5.0`.
- Target position, velocity, and acceleration values must be finite.
- DoF counts must match the tracking handle and all input/output handles.
- `reactiveness` must be finite and in `[0, 1]`.
- `look_ahead_cycles` must be at least `1`.
- Target sequence count must be greater than zero and no larger than output
  sequence capacity.
- All large buffers are allocated by constructors; online update and offline
  preallocated sequence calculation must not allocate.
- Destroying `NULL` handles is safe. Tracking handles do not own
  caller-provided input or output handles.
- No public diagnostic getters are added in `v0.5.0`.
- `interrupt_calculation_duration` is unrelated to tracking timeout behavior
  in `v0.5.0` and does not create hard or soft real-time guarantees.

## Evidence Strategy

Routine evidence is local and deterministic:

- C tests cover tracking API lifecycle, validation, online Fast update,
  deterministic fixed-corpus coverage, offline sequence calculation, tuned
  ramp and constant-acceleration quality gates, trend-only sinus metrics, and
  no-allocation behavior.
- C examples cover online ramp, online constant-acceleration, and offline
  sequence tracking.
- Python `cffi` prototype smoke covers target-state handles, target sequences,
  online Fast tracking, offline sequence tracking, multi-DoF tracking,
  invalid tracking parameters, `Optimized` unsupported behavior, and lifecycle
  errors.
- Rust alpha wrapper smoke covers online Fast tracking, offline sequence
  tracking, multi-DoF tracking, invalid tracking parameters, `Optimized`
  unsupported behavior, and examples.
- Existing no-waypoint oracle and waypoint optimizer gates remain in scope to
  protect prior solver behavior.

Quality interpretation:

- Ramp and constant-acceleration evidence uses selected Fast configurations
  that must not be worse than naive instantaneous target chasing under the
  recorded lag metric.
- Sinus and mixed target signals are recorded as trend metrics only in
  `v0.5.0`. They are not hard gates until the metric and acceptance threshold
  are separately approved.
- The default `look_ahead_cycles = 1` is retained for API behavior, but release
  quality evidence must state the selected Fast configuration being measured.

Optional Pro/cloud black-box evidence may be recorded manually for comparison
notes, but it must not become routine CI, a release gate, or a claim of formal
Pro/cloud numerical equivalence.

## Deferred Work

- Bounded local `Optimized` tracking implementation, now tracked for
  `0.6.0-design` rather than `0.5.x`.
- Soft interruption checkpoints and timeout fallback semantics.
- Formal Python package/wheels and Rust crate publication.
- Package-manager recipes.
- Cloud/remote calculation and formal Pro/cloud equivalence claims.
- Upstream baseline upgrade.
