# Interrupt Calculation Duration Design Notes

`interrupt_calculation_duration` is part of the public C API surface for
original API shape parity. In the current `0.11.0-design` line, it implements
V1 soft interruption for local waypoint `ruckig_update` recalculation only.

This is not a hard real-time guarantee. The budget is checked at safe waypoint
candidate boundaries, so the actual elapsed time can exceed the configured
microsecond value by the duration of the current complete candidate evaluation.

## Current V1 Behavior

- Unset or cleared `interrupt_calculation_duration` disables soft
  interruption.
- `interrupt_calculation_duration = 0.0` enables interruption at the first safe
  candidate boundary after at least one complete candidate evaluation.
- Negative values and NaN remain invalid input through the existing setter.
- The feature only affects `ruckig_update` when the input contains intermediate
  waypoints and a new trajectory calculation is required.
- Public `ruckig_calculate`, no-waypoint target solving, and tracking remain
  unchanged by the field.
- Internal budget timing is always available when the field is set. It does not
  depend on `RUCKIG_C_ENABLE_CALCULATION_DURATION`.
- `ruckig_output_get_calculation_duration` keeps its existing compile-time
  behavior: it reports measured duration only when
  `RUCKIG_C_ENABLE_CALCULATION_DURATION` is enabled.

## Platform Clock Abstraction

Soft interruption reads time through the internal
`ruckig_platform_monotonic_time_us()` helper. This is not a public ABI surface
and does not add exported symbols.

Default providers are selected at compile time:

- Windows uses `QueryPerformanceCounter`.
- POSIX targets use `clock_gettime(CLOCK_MONOTONIC)` when available.
- Other hosted targets fall back to `clock()`.

Embedded, RTOS, or bare-metal ports can provide a project-specific monotonic
microsecond provider without changing the public C API:

```c
#define RUCKIG_C_PLATFORM_CLOCK_HEADER "my_ruckig_clock.h"
#define RUCKIG_C_CUSTOM_MONOTONIC_TIME_US my_ruckig_clock_us
```

`RUCKIG_C_PLATFORM_CLOCK_HEADER` is optional and is included from the internal
clock helper before the provider is used. `RUCKIG_C_CUSTOM_MONOTONIC_TIME_US`
must name a no-argument function or macro returning an integer-compatible
microsecond count.

For MCU ports, the provider should return an expanded 64-bit monotonic
microsecond counter. Do not return a raw 16-bit or 32-bit hardware timer value
that can wrap during a calculation; handle timer extension or wrap accounting
in the platform layer. The soft-interruption budget remains soft: even with a
hardware timer provider, checks still happen only at complete waypoint
candidate boundaries.

## Checkpoint Semantics

- The optimizer checks the budget only after a complete waypoint candidate has
  been evaluated across all sections.
- A partially evaluated section or candidate is never accepted.
- The target/profile/root solvers are not interrupted.
- Candidate generation and evaluation order remain deterministic for the same
  input and platform.
- The first implementation does not reuse previous best candidates across
  update cycles.

## Result Semantics

- If the budget expires and at least one complete feasible waypoint candidate
  exists, `ruckig_update` writes the best complete candidate found so far,
  returns `RUCKIG_WORKING`, sets `output.new_calculation = true`, and sets
  `output.was_calculation_interrupted = true`.
- If the budget expires before any complete feasible candidate exists,
  `ruckig_update` returns `RUCKIG_ERROR_EXECUTION_TIME_CALCULATION`, does not
  mark a new calculation, and sets `output.was_calculation_interrupted = true`.
- Normal sampling cycles that do not create a new trajectory reset
  `output.was_calculation_interrupted` to false.
- If the field is unset or cleared, `output.was_calculation_interrupted`
  remains false for waypoint `ruckig_update` calculations.

## Evidence Requirements

Routine evidence for this feature should include:

- Waypoint update with no budget and cleared budget.
- Waypoint update with zero budget and an early feasible candidate.
- Waypoint update with zero budget and no feasible candidate.
- Continued online sampling after an interrupted waypoint update.
- No-waypoint `ruckig_update` and public `ruckig_calculate` unchanged when the
  field is set.
- No-allocation checks for the prepared update path.
- A duration-enabled build proving soft interruption does not depend on
  `RUCKIG_C_ENABLE_CALCULATION_DURATION`.

## Historical 0.4.x Behavior

In `0.4.x`, including `0.4.2`, `interrupt_calculation_duration` was a stored
input field only. Those release notes remain historical records and should not
be rewritten to claim V1 soft interruption.

## Deferred Work

The following remain separate design items:

- Cross-cycle previous-best reuse or true continuation of interrupted waypoint
  optimization.
- Finer-grained section or target-solver checkpoints.
- Public waypoint optimizer diagnostics.
- Hard real-time claims, proprietary Pro equivalence claims, or cloud/remote
  fallback behavior.
