# ABI Exception Policy

This policy governs public C ABI changes after the `v0.2.5` pre-`0.3.0`
baseline.

## Default Rule

No public C ABI change is accepted by default. The public ABI is the set of
`RUCKIG_C_API` declarations in `include/ruckig_c/ruckig.h` and the approved
symbol allowlist in `docs/abi/public-symbols.txt`.

## Allowed Without A Version Decision

- Version macro updates.
- Comments and documentation-only edits.
- Release evidence text and ABI artifact summaries.
- Internal implementation changes that do not alter public symbols, enum
  values, result-code values, or documented behavior.

## Forbidden Without Separate Approval

- Removing a public function.
- Changing a public function signature.
- Changing enum numeric values.
- Changing result-code numeric values.
- Adding a public function or exported symbol.
- Reclassifying implementation-internal symbols as public API.

## Required Approval For Public API Additions

Any public API addition requires:

- A dedicated design document.
- A version decision.
- `CHANGELOG.md` coverage.
- `docs/current/api_compatibility.md` coverage.
- An update to `docs/abi/public-symbols.txt`.
- An explicit `allow-add ruckig_symbol_name` entry in
  `docs/abi/public-symbol-exceptions.txt` before strict public ABI comparison
  can pass.
- Oracle-backed tests for any behavior change.

`docs/abi/public-symbol-exceptions.txt` is empty by default. Do not use it for
symbol removals or implementation-internal exports.

## Historical Linux Internal Exports

The `v0.2.5` Linux exported-symbol baseline includes implementation-internal
`ruckig_*` symbols because strict symbol visibility was not yet enforced. Those
symbols were never public API. Cleaning them up in `0.3.0` is export
hygiene, not a public API removal.

## 0.4.0 Public API Expansion

`0.4.0` is the first planned public C ABI expansion after the `0.3.0`
hardening release. The approved additions expose original-surface parity for
waypoint-aware construction, global position bounds, intermediate waypoints,
per-section constraints, intermediate duration queries, multi-section
trajectory metadata, and local waypoint filtering.

These additions are approved by `docs/design/0.4.0_original_parity.md`,
recorded in `CHANGELOG.md`, tracked in `docs/abi/public-symbols.txt`, and
listed as `allow-add` entries in `docs/abi/public-symbol-exceptions.txt`.
Existing `v0.3.0` public symbols, signatures, enum values, and result-code
numeric values remain frozen.

## 0.5.0 Tracking API Expansion

`0.5.0` stabilizes the public C tracking ABI for local Fast-mode online and
offline tracking. The approved additions expose tracking handles, target-state
handles, target-state sequences, tracking output sequences, mode/reactiveness
configuration, online update, and offline sequence calculation.

These additions are approved by `docs/design/tracking_interface.md`, recorded
in `CHANGELOG.md`, tracked in `docs/abi/public-symbols.txt`, and listed as
`allow-add` entries in `docs/abi/public-symbol-exceptions.txt`.

## 0.6.0 Optimized Tracking API Expansion

`0.6.0` stabilizes bounded local Optimized tracking controls and diagnostics:

- `ruckig_tracking_set_max_optimized_candidates`
- `ruckig_tracking_get_max_optimized_candidates`
- `ruckig_tracking_get_last_calculation_status`
- `ruckig_tracking_get_last_candidate_count`
- `ruckig_tracking_update_with_lookahead`

These additions are approved by `docs/design/tracking_optimized_mode.md`,
recorded in `CHANGELOG.md`, tracked in `docs/abi/public-symbols.txt`, and
listed as `allow-add` entries in `docs/abi/public-symbol-exceptions.txt`.
Existing `v0.5.0` public symbols, signatures, enum values, and result-code
numeric values remain frozen.

## 0.7.0-alpha Optimized Tracking Strategy Expansion

`0.7.0-alpha` adds high-level Optimized tracking strategy controls without
exposing internal objective weights or candidate-family masks:

- `ruckig_tracking_set_optimized_strategy`
- `ruckig_tracking_get_optimized_strategy`

The approved strategy enum values are `STABLE = 0`, `BALANCED = 1`, and
`AGGRESSIVE = 2`; `BALANCED` is the constructor default. These additions are
approved by `docs/design/tracking_optimized_mode.md`, recorded in
`CHANGELOG.md`, tracked in `docs/abi/public-symbols.txt`, and listed as
`allow-add` entries in `docs/abi/public-symbol-exceptions.txt`. Existing
`v0.6.0` public symbols, signatures, enum values, and result-code numeric
values remain frozen.

## 0.7.0-alpha.2 Tracking Diagnostics Snapshot Expansion

`0.7.0-alpha.2` adds a single diagnostics snapshot getter for the existing
tracking handle:

- `ruckig_tracking_get_last_diagnostics`

The getter copies the last Fast or Optimized tracking calculation summary into
the public `ruckig_tracking_diagnostics_t` struct. It exposes aggregate score
fields and named candidate-family counters, but does not expose raw optimizer
weights, candidate-family masks, or tuning knobs. This addition is approved by
`docs/design/tracking_optimized_mode.md`, recorded in `CHANGELOG.md`, tracked
in `docs/abi/public-symbols.txt`, and listed as an `allow-add` entry in
`docs/abi/public-symbol-exceptions.txt`. Existing `v0.6.0` and
`0.7.0-alpha` public symbols, signatures, enum values, and result-code numeric
values remain frozen.

## 0.15.0-alpha.4 Tracking Sequence Continuation API Expansion

`0.15.0-alpha.4` starts an explicit public C ABI expansion for interruptible
tracking sequence calculation with an opaque continuation handle. It adds
twelve public symbols:

- `ruckig_tracking_sequence_continuation_create`
- `ruckig_tracking_sequence_continuation_destroy`
- `ruckig_tracking_sequence_continuation_reset`
- `ruckig_tracking_sequence_continuation_get_dof_count`
- `ruckig_tracking_sequence_continuation_get_capacity`
- `ruckig_tracking_sequence_continuation_is_active`
- `ruckig_tracking_sequence_continuation_was_interrupted`
- `ruckig_tracking_sequence_continuation_is_complete`
- `ruckig_tracking_sequence_continuation_get_completed_count`
- `ruckig_tracking_sequence_continuation_get_target_count`
- `ruckig_tracking_calculate_sequence_interruptible`
- `ruckig_tracking_resume_sequence`

The addition is approved by `docs/design/tracking_sequence_interruption_api.md`,
tracked in `docs/abi/public-symbols.txt`, and listed in
`docs/abi/public-symbol-exceptions.txt`. It does not add result-code or enum
numeric values and does not change `ruckig_tracking_diagnostics_t` layout.
Alpha.4 provides the public handle scaffold; Fast and Optimized behavior
implementation is split into later alpha slices.

## 0.16.0 Public Diagnostics API Expansion

`0.16.0` stabilizes the opt-in public diagnostics API reviewed on the
`0.16.0` public diagnostics design line. It adds six public symbols:

- `ruckig_diagnostics_init`
- `ruckig_validate_input_with_diagnostics`
- `ruckig_calculate_with_diagnostics`
- `ruckig_update_with_diagnostics`
- `ruckig_tracking_get_last_public_diagnostics`
- `ruckig_tracking_sequence_continuation_get_last_diagnostics`

The addition is approved by `docs/design/0.16.0_public_diagnostics.md`,
tracked in `docs/abi/public-symbols.txt`, and listed in
`docs/abi/public-symbol-exceptions.txt`. It does not renumber existing
`ruckig_result_t` values and does not change existing public struct layouts.
The new `ruckig_diagnostics_t` record is caller-owned, opt-in, initialized by
`ruckig_diagnostics_init`, and protected by a `struct_size` prefix contract.
Diagnostics expose stable coarse public failure classes only; solver profile
branches, waypoint branch queues, tracking candidate ordering, score details,
optimizer phases, and random seed/sample tooling state remain private.
