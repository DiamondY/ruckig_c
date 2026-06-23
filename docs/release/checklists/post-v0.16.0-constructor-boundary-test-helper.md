# Post-v0.16.0 Constructor Boundary Test Helper

## Scope

This slice clarifies the existing constructor-boundary corrupted-state tests. It
does not change public C ABI, public symbols, workflows, version metadata,
release state, upstream baseline, visualization assets, wrappers, package
recipes, production code, or test assertions.

## Changes

- Added test-only helpers for artificial waypoint capacity/count overflow
  states in `test/c/test_api.c`.
- Replaced direct private-field mutation in `test_constructor_boundary_validation`
  with the named helpers.
- Kept `ruckig_c_constructor_boundaries` assertions and selector behavior
  unchanged.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_constructor_boundaries\|ruckig_c_tests\|ruckig_c_header_c\|ruckig_c_header_cpp"` | Passed, 4/4 |
| `cmake --build --preset windows-clang-ninja-shared` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja-shared --output-on-failure -R "ruckig_c_constructor_boundaries\|ruckig_c_header_c\|ruckig_c_header_cpp"` | Passed, 3/3 |
| Public header / ABI allowlist / workflow diff | Empty |
| `original/ruckig-main` and `docs/assets/visualization` diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |
