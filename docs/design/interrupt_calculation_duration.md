# Interrupt Calculation Duration Design Notes

`interrupt_calculation_duration` is part of the public C API surface for
original API shape parity. In `v0.11.0`, it implemented V1 soft interruption
for local waypoint `ruckig_update` new-trajectory recalculation. In
`0.12.0-alpha.1`, the same public field drives V2 true-resume for waypoint
`ruckig_update` only. In `0.12.0-alpha.2`, complete waypoint solving and
soft-interruption resume share the same private step-driven optimizer engine.
After `v0.12.0`, `0.13.0-alpha.1` adds stress and quality evidence for the
same V2 behavior. `0.13.0-alpha.2` then rewrites the private optimizer/resume
state into an internal waypoint engine and adds deterministic quality-baseline
evidence. `0.13.0-readiness` reruns the full local stable-review audit for
that post-release evidence line, and the published `v0.13.0` stable release
adopts that evidence without changing the public API, public ABI, or runtime
semantics described below. Current `main` is `0.14.0-design - Unreleased`;
no new interrupt semantics are accepted yet.

This is not a hard real-time guarantee. The budget is checked at safe waypoint
candidate boundaries, so the actual elapsed time can exceed the configured
microsecond value by the duration of the current complete candidate evaluation.

## Current V2 Behavior

- Unset or cleared `interrupt_calculation_duration` disables soft
  interruption and discards any private waypoint resume state.
- `interrupt_calculation_duration = 0.0` enables interruption at the first safe
  candidate boundary after at least one complete candidate evaluation.
- Negative values and NaN remain invalid input through the existing setter.
- The feature only affects `ruckig_update` when the input contains intermediate
  waypoints.
- Public `ruckig_calculate`, no-waypoint target solving, and tracking remain
  unchanged by the field.
- Public waypoint `ruckig_calculate` uses the same private step-driven
  waypoint optimizer engine, but always runs it to completion, ignores the
  interrupt field, and leaves no active private resume state.
- Internal budget timing is always available when the field is set. It does not
  depend on `RUCKIG_C_ENABLE_CALCULATION_DURATION`.
- `ruckig_output_get_calculation_duration` keeps its existing compile-time
  behavior: it reports measured duration only when
  `RUCKIG_C_ENABLE_CALCULATION_DURATION` is enabled.
- No public C ABI, public struct, public function signature, enum numeric
  value, or exported symbol is added for V2.

## True Resume

V2 keeps private waypoint optimizer state after an interrupted waypoint
`ruckig_update` calculation. The stored state is held in the internal waypoint
engine and includes the current search phase, complete-candidate cursor,
branch queue cursor, best waypoint velocity/acceleration candidate, baseline
duration, best duration, scratch trajectory, and diagnostics used by the C
tests.

The optimizer is still advanced only at complete-candidate boundaries:

- Baseline, finite-difference candidates, refine attempts, and branch-search
  candidates are each evaluated across all sections before they can be accepted.
- The target/profile/root solvers are not interrupted.
- No half-candidate or partially evaluated section is published.
- Background publish uses a transaction boundary: the engine first writes a
  scratch trajectory, verifies it is complete and valid, and only then copies
  it into the output trajectory when it improves over incumbent remaining
  duration.
- Complete waypoint solving and resumable waypoint solving share one private
  candidate engine. With no budget pressure, the engine runs to completion.
  Post-`v0.12.0` alpha.2 evidence uses a 128-case checked-in quality baseline
  to prevent complete-solve duration regressions if the private engine changes
  its internal candidate ordering or quality strategy.
- `0.13.0-readiness`, `v0.13.0` release-candidate, tag, and release evidence
  gates validate the same V2 behavior through focused resume stress, quality
  audit, allocation, duration-enabled, coverage, ABI/export, oracle,
  performance, wrapper, and visualization gates without changing the semantics
  above.

Ordinary online cycles can continue an active resume before sampling the old
trajectory when all of the following are true:

- The input still has intermediate waypoints and an interrupt budget.
- The target, intermediate waypoints, limits, per-section constraints,
  synchronization/control settings, enabled DoFs, and waypoint count match the
  stored private planning identity.
- The current state matches the previous `ruckig_update` output state, i.e. it
  is the normal `ruckig_output_pass_to_input` progression.

Changing only the interrupt duration value does not invalidate the private
resume state. Clearing the interrupt field disables soft interruption and drops
the private resume state. Changing the planning identity or providing a current
state that is not the normal online progression starts a fresh waypoint
calculation instead of reusing stale search state.

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
- The V2 implementation resumes the private search cursor across normal online
  cycles, but each candidate is still fully evaluated against the current input
  state before it can be accepted or published.
- The complete waypoint calculation path uses the same candidate steps without
  an interrupt context, so it cannot stop early due to
  `interrupt_calculation_duration`.

## Result Semantics

- If the budget expires and at least one complete feasible waypoint candidate
  exists, `ruckig_update` writes the best complete candidate found so far,
  returns `RUCKIG_WORKING`, sets `output.new_calculation = true`, and sets
  `output.was_calculation_interrupted = true`. If the optimizer is incomplete,
  the private resume state remains active for later online cycles.
- If the budget expires before any complete feasible candidate exists,
  `ruckig_update` returns `RUCKIG_ERROR_EXECUTION_TIME_CALCULATION`, does not
  mark a new calculation, and sets `output.was_calculation_interrupted = true`.
- Normal sampling cycles with active private resume may run more optimizer work
  before sampling. If the cycle's budget is exhausted, they set
  `output.was_calculation_interrupted = true`.
- Background resume publishes only a complete feasible trajectory whose
  duration is better than the old trajectory's remaining duration. Publishing
  follows normal new-calculation semantics: `output.new_calculation = true`,
  `output.time` and section state are reset, then the current cycle samples at
  `delta_time`.
- If a background resume cycle is interrupted without a publishable candidate,
  the old trajectory remains valid and is sampled normally. The result remains
  `RUCKIG_WORKING`, `output.new_calculation = false`, and
  `output.was_calculation_interrupted = true`.
- Normal sampling cycles that do not run interrupted optimizer work reset
  `output.was_calculation_interrupted` to false.
- If the field is unset or cleared, `output.was_calculation_interrupted`
  remains false for waypoint `ruckig_update` calculations.

## Evidence Requirements

Routine evidence for this feature should include:

- Waypoint update with no budget and cleared budget.
- Waypoint update with zero budget and an early feasible candidate.
- Background resume after `pass_to_input` with a later publishable candidate.
- Background resume interrupted without a publishable candidate, preserving the
  old trajectory.
- Interrupt clear, planning-identity change, non-`pass_to_input` state change,
  and `ruckig_reset` invalidating private resume state.
- Waypoint update with zero budget and no feasible candidate.
- No-waypoint `ruckig_update` and public `ruckig_calculate` unchanged when the
  field is set.
- Unified-engine complete waypoint solve parity with existing fixed waypoint
  regression durations, intermediate durations, samples, and section limits.
- Multi-DoF and multi-waypoint background resume with per-section constraints,
  fresh full-solve quality comparison, invalidation matrix coverage, and long
  online-loop stability.
- Post-`v0.12.0` stress evidence for budget matrices, background
  interrupted-without-publish cycles, background publish incumbent-improvement
  checks, fresh full-solve quality references, and allocation-guarded resume
  paths.
- No-allocation checks for the prepared update and background resume paths.
- A duration-enabled build proving soft interruption does not depend on
  `RUCKIG_C_ENABLE_CALCULATION_DURATION`.

## Historical 0.4.x Behavior

In `0.4.x`, including `0.4.2`, `interrupt_calculation_duration` was a stored
input field only. Those release notes remain historical records and should not
be rewritten to claim V1 soft interruption.

## Deferred Work

The following remain separate design items:

- Finer-grained section or target-solver checkpoints.
- Cross-cycle continuation for no-waypoint target solving or tracking.
- Public waypoint optimizer diagnostics.
- Hard real-time claims, proprietary Pro equivalence claims, or cloud/remote
  fallback behavior.
