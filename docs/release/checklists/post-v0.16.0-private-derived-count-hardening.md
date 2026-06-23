# Post-v0.16.0 Private Derived Count Hardening

## Scope

This slice adds defense-in-depth checked arithmetic to selected private
tracking, waypoint, and trajectory helper paths that derive buffer lengths from
counts and DoF values. It does not change public C ABI, public symbols,
workflow, version metadata, release state, upstream baseline, visualization
assets, wrappers, package recipes, solver candidate order, waypoint optimizer
phases, or oracle tolerances.

## Changes

- Added checked profile-count multiplication to no-waypoint trajectory copying.
- Hardened tracking sequence prefix copy, continuation capture, stored output,
  complete-sequence offsets, and Optimized continuation window offsets.
- Hardened Optimized tracking target finite-vector validation by checking
  `target_count * dofs` before using the derived value.
- Hardened waypoint engine candidate accept/refine/branch/step helpers and
  waypoint trajectory copying with checked derived counts.
- Added a `RUCKIG_C_TESTING`-only waypoint engine step hook for white-box
  corrupted-state coverage in `ruckig_c_tests`; the hook is not compiled into
  the `ruckig_c` library target.
- Extended `ruckig_c_constructor_boundaries` to cover tracking sequence
  overflow capture/prefix behavior and waypoint engine overflow early return.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_constructor_boundaries\|ruckig_c_tracking_sequence_continuation_api\|ruckig_c_tracking_sequence_fast_continuation\|ruckig_c_tracking_sequence_optimized_continuation\|ruckig_c_waypoint_resume_stress\|ruckig_c_tests\|ruckig_c_header_c\|ruckig_c_header_cpp"` | Passed, 8/8 |
| `cmake --build --preset windows-clang-ninja-shared` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja-shared --output-on-failure -R "ruckig_c_constructor_boundaries\|ruckig_c_tracking_sequence_continuation_api\|ruckig_c_header_c\|ruckig_c_header_cpp"` | Passed, 4/4 |
| Public header / ABI allowlist / workflow diff | Empty |
| `original/ruckig-main` and `docs/assets/visualization` diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |
