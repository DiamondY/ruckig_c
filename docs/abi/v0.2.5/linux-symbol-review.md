# v0.2.5 Linux Exported-Symbol Review

This review classifies the `v0.2.5` Linux shared-library exported symbols for
`0.3.0-design` ABI cleanup. It is evidence for drift review, not a public API
expansion.

## Summary

- Linux baseline symbols: `127`.
- Approved public C ABI symbols: `66`.
- Test/debug allocation helpers visible in the historical Linux baseline: `4`.
- Implementation-internal symbols visible in the historical Linux baseline:
  `57`.
- Unexpected symbols: `0`.

Only the `66` symbols listed in `docs/abi/public-symbols.txt` are public C API.
The other `61` historical Linux exports were accidental visibility leaks and
are not supported consumer entry points.

## Test/Debug Allocation Helpers

These symbols support allocation diagnostics and were visible on Linux at
`v0.2.5`, but they are not declared in the public header:

- `ruckig_allocation_count`
- `ruckig_allocation_counters_reset`
- `ruckig_allocation_forbidden_count`
- `ruckig_allocation_forbidden_set`

## Implementation-Internal Symbols

These symbols are implementation details and must not be used by consumers:

- `ruckig_block_calculate`
- `ruckig_block_get_profile`
- `ruckig_block_init`
- `ruckig_block_is_blocked`
- `ruckig_brake_finalize`
- `ruckig_brake_finalize_second_order`
- `ruckig_brake_get_position_trajectory`
- `ruckig_brake_get_second_order_position_trajectory`
- `ruckig_brake_get_second_order_velocity_trajectory`
- `ruckig_brake_get_velocity_trajectory`
- `ruckig_brake_profile_init`
- `ruckig_calloc`
- `ruckig_free`
- `ruckig_free_count`
- `ruckig_input_copy_state`
- `ruckig_input_equals`
- `ruckig_input_same_dofs`
- `ruckig_integrate`
- `ruckig_poly_derivative`
- `ruckig_poly_eval`
- `ruckig_poly_monic_derivative`
- `ruckig_position_first_step1_get_profile`
- `ruckig_position_first_step2_get_profile`
- `ruckig_position_second_step1_get_profile`
- `ruckig_position_second_step2_get_profile`
- `ruckig_position_third_step1_get_profile`
- `ruckig_position_third_step2_get_profile`
- `ruckig_pow2`
- `ruckig_profile_check`
- `ruckig_profile_check_for_first_order`
- `ruckig_profile_check_for_first_order_with_timing`
- `ruckig_profile_check_for_first_order_with_timing_guarded`
- `ruckig_profile_check_for_second_order`
- `ruckig_profile_check_for_second_order_velocity`
- `ruckig_profile_check_for_second_order_velocity_with_timing`
- `ruckig_profile_check_for_second_order_velocity_with_timing_guarded`
- `ruckig_profile_check_for_second_order_with_timing`
- `ruckig_profile_check_for_second_order_with_timing_guarded`
- `ruckig_profile_check_for_velocity`
- `ruckig_profile_check_for_velocity_with_timing`
- `ruckig_profile_check_for_velocity_with_timing_guarded`
- `ruckig_profile_check_with_timing`
- `ruckig_profile_check_with_timing_guarded`
- `ruckig_profile_copy_boundary`
- `ruckig_profile_get_first_state_at_position`
- `ruckig_profile_get_position_extrema`
- `ruckig_profile_init`
- `ruckig_profile_set_boundary`
- `ruckig_profile_set_boundary_for_velocity`
- `ruckig_shrink_interval`
- `ruckig_solve_cubic`
- `ruckig_solve_quart_monic`
- `ruckig_solve_resolvent`
- `ruckig_velocity_second_step1_get_profile`
- `ruckig_velocity_second_step2_get_profile`
- `ruckig_velocity_third_step1_get_profile`
- `ruckig_velocity_third_step2_get_profile`

## Cleanup Decision

`0.3.0-design` may hide these historical Linux internal exports by default.
That cleanup is not a public API removal because the symbols are absent from
`include/ruckig_c/ruckig.h` and `docs/abi/public-symbols.txt`.
