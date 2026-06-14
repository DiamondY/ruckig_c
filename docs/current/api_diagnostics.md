# Ruckig C API Diagnostics

This document explains common public API failures and limit-selection behavior
for `ruckig_c`. It describes the current C API only; waypoints, per-section
constraints, cloud calculation, and language bindings remain outside the
supported scope.

## Successful Offline Calculation

`ruckig_calculate` returns `RUCKIG_WORKING` when an offline trajectory was
calculated successfully and can be queried. `RUCKIG_FINISHED` is used by the
online `ruckig_update` loop after the sampled output time has passed the
trajectory duration.

`ruckig_trajectory_at_time` requires a non-NULL `position` array with at least
the trajectory DoF count. `velocity`, `acceleration`, `jerk`, and `section` may
be `NULL` when the caller does not need those outputs.

## Invalid Input

`RUCKIG_ERROR_INVALID_INPUT` is returned for malformed handles, DoF mismatches,
or numerical inputs that cannot be validated. Common causes include:

- `NULL` handles passed to required API parameters.
- A `ruckig_t`, `ruckig_input_t`, `ruckig_output_t`, or `ruckig_trajectory_t`
  created for a different DoF count.
- NaN position, velocity, acceleration, or limit values.
- Negative `max_velocity`, `max_acceleration`, or `max_jerk`.
- Positive directional lower limits passed through `min_velocity` or
  `min_acceleration`.
- `RUCKIG_DURATION_DISCRETE` with a non-positive `delta_time`.
- Current or target state outside limits when the corresponding validation flag
  is enabled.
- Per-DoF control-interface or synchronization setters called with a `NULL`
  values pointer, a `count` that differs from the input DoF count, or an enum
  value outside the public range.

`ruckig_validate_input(otg, input, check_current, check_target)` only enforces
current-state and target-state limit checks when the matching flag is `true`.
`ruckig_calculate` validates target state limits.

## Per-DoF Overrides

Global setters define the default control-interface and synchronization mode.
When a per-DoF vector is enabled, each DoF uses its own value from that vector.
Use `ruckig_input_clear_per_dof_control_interface` or
`ruckig_input_clear_per_dof_synchronization` to restore global-only behavior.

Per-DoF storage is allocated during input creation. Setting or clearing per-DoF
vectors does not allocate during `ruckig_calculate` or `ruckig_update`.

## Zero Limits

`RUCKIG_ERROR_ZERO_LIMITS` is used when the selected solver path cannot produce
a trajectory because a required limit is exactly zero. Representative examples:

- First-order position with `max_velocity == 0.0`.
- Second-order position or velocity with `max_acceleration == 0.0`.
- Third-order position or velocity with `max_jerk == 0.0`.

When synchronization requires a non-limiting DoF to stretch to a shared
duration, a zero-limit conflict can also surface as a synchronization
calculation error, matching the C++ oracle behavior.

## Finite And Infinite Limits

The public API uses infinite acceleration or jerk limits to select lower-order
solvers, matching the frozen C++ oracle:

- Position control with `max_acceleration = INFINITY` and
  `max_jerk = INFINITY` selects a first-order position trajectory.
- Position control with finite acceleration and `max_jerk = INFINITY` selects a
  second-order position trajectory.
- Position control with finite jerk selects a third-order position trajectory.
- Velocity control with `max_jerk = INFINITY` selects a second-order velocity
  trajectory.
- Velocity control with finite jerk selects a third-order velocity trajectory.

Do not enable unsafe fast-math options that erase IEEE infinity semantics.

## Minimum Duration And Discrete Duration

`ruckig_input_set_minimum_duration` sets a global lower bound on the trajectory
duration. With continuous duration, the solver may use that exact value when it
is longer than the independent minimum duration. With discrete duration, the
selected synchronization duration is rounded up to a multiple of `delta_time`.

`minimum_duration` must be finite or infinite according to normal `double`
rules, non-NaN, and non-negative. Use `ruckig_input_clear_minimum_duration` to
remove it.

## Synchronization Modes

`RUCKIG_SYNCHRONIZATION_TIME` synchronizes enabled DoFs to a common duration.

`RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY` allows a DoF with zero target
velocity and zero target acceleration to keep its independent timing when time
synchronization is unnecessary.

`RUCKIG_SYNCHRONIZATION_PHASE` attempts phase synchronization and falls back to
time synchronization when phase constraints cannot be satisfied.

`RUCKIG_SYNCHRONIZATION_NONE` avoids common-duration synchronization for
continuous duration. With discrete duration, duration rounding can still affect
the trajectory timing because the duration must be compatible with the control
cycle.

Disabled DoFs keep their current state with constant acceleration behavior and
do not contribute an independent minimum duration.

## Future Public Diagnostics Design

The core public diagnostics API is now available on the `0.16.0` design line.
It is opt-in and leaves the legacy entry points unchanged:

- `ruckig_diagnostics_init` initializes a caller-owned diagnostics record.
  Passing `NULL` is a no-op.
- `ruckig_validate_input_with_diagnostics` explains validation failures with a
  stable coarse scope/code plus optional DoF, section, count, value, and limit
  fields.
- `ruckig_calculate_with_diagnostics` and
  `ruckig_update_with_diagnostics` preserve the legacy result code while
  filling diagnostics for invalid handles, DoF/capacity mismatches, validation
  failures, zero-limit errors, synchronization errors, trajectory-duration
  errors, unsupported combinations, observable interruption, and waypoint
  resume identity mismatch.
- Passing `NULL` diagnostics to a `_with_diagnostics` function is defined to
  behave like the corresponding legacy API.

Callers that pass a non-NULL diagnostics pointer must initialize it with
`ruckig_diagnostics_init`. A too-small `struct_size` is rejected with
`RUCKIG_ERROR_INVALID_INPUT` before the main operation runs. Valid records are
written only within the caller-declared stable prefix, leaving reserved fields
for future ABI-compatible expansion.

The post-`v0.15.0` readiness audit starts a docs-only
`0.16.0-design-public-diagnostics` line in
`docs/design/0.16.0_public_diagnostics.md`. The follow-up
`0.16.0-alpha.2` contract freeze locked the intended public diagnostics shape,
and `0.16.0-alpha.3` implements the core validate/calculate/update API:

- `ruckig_diagnostics_t` is an opt-in output record with `struct_size`,
  result, stable scope/code, location, count, value, limit, and reserved
  fields.
- The public ABI expands from the `v0.15.0` 184-symbol baseline to 188 symbols
  by adding `ruckig_diagnostics_init`,
  `ruckig_validate_input_with_diagnostics`,
  `ruckig_calculate_with_diagnostics`, and
  `ruckig_update_with_diagnostics`.
- `0.16.0-alpha.4` adds no public symbols. It extends
  `ruckig_update_with_diagnostics` diagnostics for no-waypoint interruption,
  waypoint interruption, waypoint resume identity mismatch, waypoint
  count/capacity mismatch, and cleared/stale waypoint resume state using the
  existing stable coarse codes.
- Tracking and continuation diagnostics use getter-style accessors instead of
  adding `_with_diagnostics` variants to every tracking operation; those
  getter symbols remain deferred to alpha.5.

Diagnostics remain stable and coarse-grained. They explain public failure
classes and locations, but do not expose solver profile branches, candidate
ordering, waypoint queue internals, or random seed/sample tooling state. Python
and Rust wrappers remain prototype-only until a separate stabilization design
is accepted.
