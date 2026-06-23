# Post-v0.16.0 Solver Hotspot Commentary

## Scope

This slice adds narrow intent comments to selected third-order position solver
helpers. It does not change public C ABI, public symbols, workflows, version
metadata, release state, upstream baseline, visualization assets, wrappers,
package recipes, formulas, candidate ordering, constants, or oracle behavior.

## Changes

- Documented the rest-to-rest velocity-limited UDDU timing family and the
  symmetric `tf/4` jerk-pulse bound.
- Documented that the general UDDU root helper refines a candidate once before
  delegating limit acceptance to the profile timing check.
- Documented that `time_none` keeps no-limit synchronization families together
  to preserve candidate-order auditability.
- Documented that the public step2 entry point dispatches timing families and
  relies on profile checks as the final acceptance gate.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_tests\|ruckig_c_solver_branch_coverage\|ruckig_c_roots_numeric_audit\|ruckig_c_header_c\|ruckig_c_header_cpp"` | Passed, 5/5 |
| `cmake --build --preset windows-clang-ninja-oracle` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja-oracle --output-on-failure -R "ruckig_c_oracle_tests\|ruckig_c_waypoint_section_oracle"` | Passed, 2/2 |
| Public header / ABI allowlist / workflow diff | Empty |
| `original/ruckig-main` and `docs/assets/visualization` diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |
