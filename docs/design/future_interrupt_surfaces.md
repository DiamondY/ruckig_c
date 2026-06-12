# Future Interrupt Surfaces Design Spec

This document records the `0.14.0-alpha.2` design-only quasi-spec that was used
to review interruption support outside waypoint `ruckig_update`.

The no-waypoint and online tracking update/lookahead subsets described here
were later implemented by `0.14.0-alpha.4` and `0.14.0-alpha.5` without public
C API, public ABI, exported-symbol, version metadata, tag, or release changes.
The remaining deferred surface is `ruckig_tracking_calculate_sequence`
interruption. `0.15.0-alpha.2` adds
`tracking_sequence_interruption_api.md` to document why that surface requires a
separate public diagnostics/API decision before implementation.

## Current Boundary

The active behavior remains:

- `interrupt_calculation_duration` affects waypoint `ruckig_update` when the
  input contains intermediate waypoints.
- Waypoint interruption uses the private waypoint optimizer engine and can
  true-resume across normal online `pass_to_input` cycles.
- No-waypoint `ruckig_update` uses complete-trajectory-boundary interruption
  without true-resume.
- Optimized online tracking update and lookahead update use
  complete-candidate-boundary interruption without waypoint or no-waypoint
  resume state.
- Public `ruckig_calculate` ignores the interrupt budget and runs complete
  solves.
- `ruckig_tracking_calculate_sequence` does not use interruption.
- `RUCKIG_C_ENABLE_CALCULATION_DURATION` only controls public reporting of
  `calculation_duration`; it does not control interruption availability.

See `tracking_sequence_interruption_api.md` for the current docs-only API
draft. It is not implemented and does not approve public ABI expansion.

## Shared Rules

The implemented alpha.4/alpha.5 surfaces and any future extension must keep
these rules unless a separate public API decision changes them:

- Reuse the existing `interrupt_calculation_duration` input field; do not add a
  public interruption setter, diagnostics handle, result code, enum value, or
  exported symbol.
- Check the budget only at complete candidate or complete trajectory
  boundaries. Do not interrupt target/profile/root solvers mid-evaluation.
- Never publish a partial trajectory or a half-evaluated tracking candidate.
- Keep online paths allocation-free after object/input/output construction.
- Keep waypoint true-resume state isolated from no-waypoint and tracking paths.
- Mark a cycle interrupted only when useful work was stopped early and a later
  cycle could retry or continue the broader calculation policy.
- Keep hard real-time, global optimality, Cloud/Pro numerical equivalence, and
  proprietary runtime claims out of public documentation.

## No-Waypoint Update Spec

The implemented no-waypoint policy is complete-trajectory-boundary
interruption, not true-resume.

- Scope is only `ruckig_update` with `input->waypoint_count == 0`.
- Public `ruckig_calculate` remains a complete solve and ignores the interrupt
  field.
- The implementation may calculate a new target trajectory into private
  scratch storage before replacing the output trajectory.
- Budget checks happen after a complete target trajectory attempt returns.
- If a valid incumbent trajectory exists and the budget is exhausted at that
  boundary, keep sampling the incumbent trajectory, set
  `new_calculation=false`, set `was_calculation_interrupted=true`, and leave no
  private no-waypoint resume state.
- If no valid incumbent trajectory exists, publish a complete successful first
  trajectory when available. If no complete trajectory can be produced, return
  the existing execution-time or calculation error style without publishing
  partial output.
- A later cycle may retry from the advanced current state, but there is no
  cross-cycle root/profile cursor and no no-waypoint true-resume guarantee.
- Changing only the interrupt duration does not need to preserve any private
  no-waypoint state, because there is no future no-waypoint resume state in
  this policy.
- Clearing the interrupt field disables the future no-waypoint budget check and
  must not affect waypoint resume semantics beyond the existing waypoint
  boundary rules.

Rejected for this policy:

- Pausing inside polynomial root solving or profile synchronization.
- Reusing the waypoint optimizer engine for no-waypoint target solving.
- Adding public diagnostics for the kept-incumbent case.
- Claiming hard deadline enforcement from a complete-trajectory-boundary check.

## Online Tracking Spec

The implemented tracking policy is online-only interruption for
`ruckig_tracking_update` and `ruckig_tracking_update_with_lookahead`.

- `ruckig_tracking_calculate_sequence` remains deferred. The current public
  sequence output has no API-neutral interruption carrier equivalent to
  `ruckig_output_t::was_calculation_interrupted`.
- Fast tracking mode has at most one prepared candidate. It may report no
  interruption when that single complete candidate is accepted, even if the
  elapsed time is measured after the candidate boundary.
- Optimized tracking mode may stop candidate-family enumeration at complete
  candidate boundaries.
- The fast candidate remains the minimum complete fallback candidate for an
  interrupted optimized online cycle.
- If optimized enumeration is stopped early, publish the best complete feasible
  candidate selected so far, set `was_calculation_interrupted=true`, and keep
  tracking diagnostics internally consistent with the candidates actually
  evaluated.
- If a valid incumbent output trajectory exists and no complete tracking
  candidate is publishable before the boundary decision, keep sampling the
  incumbent trajectory, set `new_calculation=false`, and set
  `was_calculation_interrupted=true`.
- No tracking interruption state is preserved across cycles in this future
  policy. A later online cycle may retry from the advanced current state and
  current target/lookahead input.
- Tracking interruption must not activate waypoint resume state or reuse
  waypoint optimizer cursors.

Rejected for this policy:

- Extending `ruckig_tracking_calculate_sequence` without a separate public
  diagnostics/API decision.
- Adding public tracking interruption diagnostics in the API-neutral phase.
- Pausing inside target/profile/root solving.
- Claiming equivalence with proprietary Pro or Cloud tracking behavior.

## Acceptance Gates

The alpha.4/alpha.5 implementation plans used local evidence at least as
strict as:

- Focused C selectors and CTest entries for no-waypoint interrupt boundary and
  online tracking interrupt boundary.
- Budget matrix coverage for unset, zero, tiny positive, large, changed, and
  cleared interrupt durations.
- Incumbent-preservation checks: no partial publish, no backward time/section
  movement, and no stale waypoint resume leakage.
- Allocation guards for interrupted online cycles.
- Duration-enabled regression coverage proving interruption availability does
  not depend on `RUCKIG_C_ENABLE_CALCULATION_DURATION`.
- ABI/export guards proving the public symbol count remains `172`, public
  additions remain `0`, removals remain `0`, and unapproved exports remain
  `0`.
- Documentation that keeps `tracking_calculate_sequence`, public diagnostics,
  runtime clock public hooks, package-manager work, Cloud/Pro claims, hard
  real-time guarantees, formal global optimality proof, and upstream baseline
  upgrades outside the implementation slice unless separately approved.
