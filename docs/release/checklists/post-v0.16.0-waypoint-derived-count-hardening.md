# Post-v0.16.0 Waypoint Derived Count Hardening

## Scope

This slice adds defense-in-depth checked arithmetic for public waypoint derived
counts. It does not change public C ABI, public symbols, workflows, version
metadata, release state, upstream baseline, visualization assets, wrappers, or
package-manager recipes.

## Changes

- `ruckig_input_set_intermediate_positions` and
  `ruckig_input_get_intermediate_positions` now check `waypoint_count * dofs`
  before using the derived count.
- Public per-section vector helpers now check `waypoint_count + 1` and
  `section_count * dofs` before copying.
- `ruckig_input_copy_state`, input equality, waypoint planning identity, and
  `ruckig_filter_intermediate_positions` now reject overflowing derived counts
  before using copy, compare, or filter lengths.
- `ruckig_c_constructor_boundaries` now covers artificial/corrupted overflow
  states for public waypoint derived count paths.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed; no work to do after final docs update |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_constructor_boundaries\|ruckig_c_tests\|ruckig_c_public_diagnostics\|ruckig_c_header_c\|ruckig_c_header_cpp"` | Passed, 5/5 |
| `cmake --build --preset windows-clang-ninja-shared` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja-shared --output-on-failure -R "ruckig_c_constructor_boundaries\|ruckig_c_public_diagnostics\|ruckig_c_header_c\|ruckig_c_header_cpp"` | Passed, 4/4 |
| Public header / ABI allowlist / workflow diff | Empty |
| `original/ruckig-main` and `docs/assets/visualization` diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |
