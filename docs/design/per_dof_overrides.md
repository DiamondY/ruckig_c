# Per-DoF Overrides Design

This document records the `0.2.0` design for per-DoF control-interface and
synchronization overrides. The design gate has been implemented in the public C
API and is verified against the frozen C++ oracle with fixed cases.

## Goals

- Match the frozen Ruckig Community `0.17.3` oracle semantics for
  `per_dof_control_interface` and `per_dof_synchronization`.
- Preserve existing global setters as defaults.
- Keep the public C ABI opaque.
- Preserve the no-allocation contract for `ruckig_calculate`, `ruckig_update`,
  trajectory sampling, and root solvers.

## Public API Shape

The C API additions are:

```c
ruckig_result_t ruckig_input_set_per_dof_control_interface(
    ruckig_input_t* input,
    const ruckig_control_interface_t* values,
    size_t count
);

void ruckig_input_clear_per_dof_control_interface(ruckig_input_t* input);

ruckig_result_t ruckig_input_set_per_dof_synchronization(
    ruckig_input_t* input,
    const ruckig_synchronization_t* values,
    size_t count
);

void ruckig_input_clear_per_dof_synchronization(ruckig_input_t* input);
```

No accessor is planned initially. Add accessors only if a concrete downstream
need appears.

## Semantics

- If a per-DoF vector is not enabled, every DoF uses the matching global input
  setting.
- If a per-DoF vector is enabled, each DoF uses its per-DoF value.
- `count` must equal the input DoF count.
- `values` must be non-NULL.
- Enum values must be inside the public enum range.
- Setter failures return `RUCKIG_ERROR_INVALID_INPUT` and leave the previous
  setting unchanged.
- Clear functions disable the optional vector and restore global-only behavior.
- Create functions preallocate per-DoF storage.
- Clear functions do not free memory.

## Oracle Mapping

The C++ oracle fields are:

- `InputParameter::per_dof_control_interface`
- `InputParameter::per_dof_synchronization`

The oracle target extends `CaseData` with optional per-DoF vectors and fills
both the C++ input fields and the C setters. Random per-DoF generation remains
disabled; fixed cases are used to keep the initial `0.2.0` surface focused.

Fixed oracle cases:

- Global position with one velocity-control DoF override.
- Global velocity with one position-control DoF override.
- Global `Time` synchronization with one `None` DoF override.
- Global `None` synchronization with one `Time` DoF override.
- Clear per-DoF settings and verify behavior returns to the equivalent
  global-only input in C API tests.

## Implementation Notes

- Optional vector flags and preallocated storage live in the internal input
  struct only; do not expose struct fields publicly.
- Per-DoF flags and values are included in input copy and equality checks.
- Solver dispatch uses effective per-DoF settings, matching
  `calculator_target.hpp` in the frozen C++ oracle.
- Keep formula-heavy step1/step2 solver files unchanged unless an oracle
  mismatch proves a local fix is required.

## Still Deferred

- Intermediate waypoints.
- Per-section constraints.
- Cloud calculation.
- Python bindings.
- Rust bindings.
- Upstream Ruckig baseline upgrades.
