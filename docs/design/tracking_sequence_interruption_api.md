# Tracking Sequence Interruption API Draft

Status: `0.15.0-alpha.7` Fast and Optimized continuation behavior plus
C/Python/Rust prototype smoke coverage implemented locally.

This document evaluates whether `ruckig_tracking_calculate_sequence` can support
interruption without public API changes. The current answer is no: online
tracking can report interruption through `ruckig_output_t`, but sequence
calculation writes into `ruckig_tracking_output_sequence_t`, which has no
API-neutral interruption flag, partial-completion status, or continuation
carrier.

## Current Boundary

- `ruckig_tracking_update` and `ruckig_tracking_update_with_lookahead` can use
  the existing input interrupt field and `ruckig_output_was_calculation_interrupted`.
- `ruckig_tracking_calculate_sequence` still runs complete sequence solves and
  does not use interruption.
- `ruckig_tracking_diagnostics_t` records aggregate candidate diagnostics after
  a complete sequence calculation, but it does not identify a partial sequence
  boundary that callers can safely resume from.
- No public header, public ABI, enum value, result-code value, exported symbol,
  or public diagnostics field is changed by this draft.

## Why API-Neutral Sequence Interruption Is Not Clean

The online APIs already return one `ruckig_output_t` per call. That output has
an existing `was_calculation_interrupted` flag and a trajectory that can be
sampled by the caller on the next online cycle.

The sequence API is different:

- It fills a caller-provided `ruckig_tracking_output_sequence_t` with multiple
  step outputs.
- The current output sequence stores per-step result codes, state arrays,
  sections, and times, but no sequence-level interruption flag.
- Returning `RUCKIG_WORKING` with fewer steps filled would be ambiguous: it
  could mean partial success, a normal shorter sequence, or an interrupted
  sequence unless a new public status carrier explains it.
- Returning an error code for interruption would collide with existing
  execution and validation errors unless a new result code or status field is
  approved.
- Reusing `tracking->diagnostics` alone is not enough because diagnostics are
  a snapshot of candidate evaluation, not a stable public sequence-resume
  contract.
- A true continuation would need to preserve target-sequence index, tracking
  work input state, optimized candidate state, and diagnostics. None of those
  have a public carrier today.

## Candidate Public API Shapes

These were evaluated in `0.15.0-alpha.2`. `0.15.0-alpha.4` accepts the
explicit continuation carrier.

### Sequence-Level Interruption Flag

Add a public query such as
`ruckig_tracking_output_sequence_was_interrupted(const ruckig_tracking_output_sequence_t*)`.

Impact:

- Smallest caller model: callers can detect an interrupted sequence after the
  call.
- Requires a new exported symbol and a new field inside the opaque output
  sequence implementation.
- Still does not tell callers whether they may resume, retry, or consume a
  partial prefix unless paired with filled-count rules.

### Partial Sequence Status

Define explicit public semantics for `ruckig_tracking_output_sequence_get_count`
after an interrupted call, plus a status query such as
`ruckig_tracking_output_sequence_get_completion_status`.

Impact:

- Makes partial-prefix consumption explicit.
- Requires a public status enum or result-code expansion.
- Needs migration documentation because current callers can treat a successful
  sequence call as complete.

### Diagnostics Snapshot Extension

Extend `ruckig_tracking_diagnostics_t` with sequence interruption fields such
as interrupted step, completed step count, and budget-exhausted reason.

Impact:

- Keeps diagnostics and interruption attribution together.
- Changes a public struct layout, which is an ABI break for existing binaries
  unless deferred to a major ABI expansion strategy.
- Diagnostics alone still does not provide continuation state.

### Explicit Resume Or Continuation Carrier

Add a new opaque continuation handle for interrupted sequence calculations.

Impact:

- Cleanest true-resume story if sequence interruption needs deterministic
  continuation instead of retry-from-prefix behavior.
- Largest API surface: new create/destroy functions, continuation ownership
  rules, reset semantics, and likely new result/status queries.
- Adds exported symbols and a new lifecycle contract; this should not be hidden
  inside an API-neutral alpha slice.

`0.15.0-alpha.4` accepts this shape with an opaque
`ruckig_tracking_sequence_continuation_t` handle. The handle owns its target
sequence and input snapshots, exposes compact active/interrupted/complete and
count accessors, and keeps `ruckig_tracking_calculate_sequence` unchanged.
The first scaffold adds the public lifecycle and status API plus
`ruckig_tracking_calculate_sequence_interruptible` and
`ruckig_tracking_resume_sequence`. `0.15.0-alpha.5` implements Fast-mode
sequence continuation on that API. Fast interruption happens only at complete
sequence-step boundaries: interrupted calls expose a complete prefix in
`ruckig_tracking_output_sequence_t`, keep the continuation active, and resume
from handle-owned target/input snapshots without requiring the caller to resend
the target sequence or input.

`0.15.0-alpha.6` implements Optimized-mode continuation with a private
candidate-boundary state machine. The handle stores the current step's phase,
candidate index, best-so-far candidate target, input snapshot, and diagnostics
between calls. Interrupted calls still expose only the complete output prefix;
the in-progress step is completed privately after enough resume calls. The old
`ruckig_tracking_calculate_sequence` entry point still performs complete
sequence solves.

Post-alpha.7 hardening also stores the initiating tracking handle's
`delta_time` in the opaque continuation. Resume is valid on another tracking
handle only when both DoF count and `delta_time` match; this keeps output
timestamps and Optimized candidate horizons deterministic across continuation
calls.

`0.15.0-alpha.7` adds consumer smoke coverage for this API: a C example,
Python cffi prototype declarations/classes/tests, and Rust wrapper
declarations/classes/tests/examples. These wrappers remain prototype-only; no
wheel, crate publication, package-manager recipe, or stable wrapper API claim
is made.

## Risk Review

- ABI risk: any public query, public enum, public result code, public struct
  field, or continuation handle changes the 172-symbol baseline or public
  layout/numeric contract.
- Migration risk: partial sequence success changes caller assumptions about
  `ruckig_tracking_output_sequence_get_count` and per-step result arrays.
- Diagnostics risk: aggregate diagnostics may become misleading if they mix
  complete sequence semantics with partial interruption semantics.
- Testing risk: acceptance would need new C, Python prototype, Rust wrapper,
  allocation, duration-enabled, ABI/export, and documentation gates.
- Release risk: this is larger than a local evidence-only alpha and should be
  isolated from current post-release hardening.

## Current Go/No-Go

Go for the continuation public API scaffold in `0.15.0-alpha.4`, Fast sequence
continuation behavior in `0.15.0-alpha.5`, and Optimized sequence continuation
behavior in `0.15.0-alpha.6`.

Current no-go boundaries:

- Do not add enum or result-code numeric values.
- Do not change `ruckig_tracking_diagnostics_t` layout.
- Do not publish wrapper/package-manager claims after alpha.7 smoke coverage;
  the wrapper coverage is prototype evidence only.

Fast sequence continuation is implemented in `0.15.0-alpha.5`. Optimized
solver-internal sequence continuation is implemented in `0.15.0-alpha.6`.
Prototype wrapper smoke coverage is implemented in `0.15.0-alpha.7`.
