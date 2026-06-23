# Post-v0.16.0 Brake Epsilon Constant Polish

## Scope

This slice names the remaining private brake timing epsilon without changing
numeric behavior. It does not change public C ABI, public symbols, workflows,
version metadata, release state, upstream baseline, visualization assets,
wrappers, package recipes, solver formulas, branch conditions, or oracle
tolerances.

## Changes

- Added private `RUCKIG_C_BRAKE_TIME_EPS` in `src/ruckig_c/precision.h`.
- Updated `src/ruckig_c/brake.c` to keep the local `brake_eps` alias backed by
  `RUCKIG_C_BRAKE_TIME_EPS`.
- Updated white-box brake assertions to use `RUCKIG_C_BRAKE_TIME_EPS`.
- Preserved the exact `2.2e-14` value and all existing assertion tolerances.
- Kept clang-tidy/cppcheck/CodeQL CI adoption, large solver splitting,
  nullability annotations, and platform-matrix expansion owner/event-gated.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_tests\|ruckig_c_solver_branch_coverage\|ruckig_c_header_c\|ruckig_c_header_cpp"` | Passed, 4/4 |
| `cmake --build --preset windows-clang-ninja-oracle` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja-oracle --output-on-failure -R "ruckig_c_oracle\|ruckig_c_waypoint_section_oracle"` | Passed, 12/12; the selector matched fixed, waypoint, smoke, shrink, development random, and release random oracle tests. |
| Public header / ABI allowlist / workflow diff | Empty |
| `original/ruckig-main` and `docs/assets/visualization` diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |
