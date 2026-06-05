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
