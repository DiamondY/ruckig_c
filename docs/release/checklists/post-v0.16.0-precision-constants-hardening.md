# Post-v0.16.0 Precision Constants Hardening Checklist

This checklist records the private precision-constant hardening slice. It
centralizes selected implementation tolerances without changing numeric values,
oracle tolerances, public ABI, workflow, upstream baseline, or release state.

## Scope

- [x] Added private `src/ruckig_c/precision.h`.
- [x] Preserved existing private `RUCKIG_DBL_EPSILON` and `RUCKIG_TIME_EPS`
  macro names through `precision.h`.
- [x] Centralized selected profile, roots shrink-interval, block duration tie,
  and `position_third_step2.c` polynomial/refinement tolerances.
- [x] Did not change any numeric tolerance value.
- [x] Did not refactor solver formulas or adjust oracle tolerance policy.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_tests\|ruckig_c_roots_numeric_audit\|ruckig_c_solver_branch_coverage\|ruckig_c_public_diagnostics"` | Passed; 4/4 tests |
| `cmake --build --preset windows-clang-ninja-oracle` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja-oracle --output-on-failure -R "^ruckig_c_oracle_tests$"` | Passed; 1/1 fixed oracle test. The planned `ruckig_c_oracle_fixed` name is not registered in this CTest tree. |
| Public header / ABI / workflow boundary diff | Empty |
| `original/ruckig-main` and visualization asset boundary diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |
