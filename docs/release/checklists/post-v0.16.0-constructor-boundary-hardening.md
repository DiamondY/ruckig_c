# Post-v0.16.0 Constructor Boundary Hardening Checklist

This checklist records the constructor boundary hardening slice. It tightens
private allocation-size validation for public constructors without changing the
public C ABI, version metadata, release state, upstream baseline, wrapper
publication status, or default CI policy.

## Scope

- [x] Added shared checked-size helpers in `src/ruckig_c/internal.h`.
- [x] Routed waypoint-section and waypoint-value capacity calculations through
  checked arithmetic before allocation.
- [x] Hardened input, trajectory, output, OTG, and tracking sequence
  constructors against impossible waypoint/count capacities.
- [x] Added `--constructor-boundaries` and CTest
  `ruckig_c_constructor_boundaries`.
- [x] Covered null output pointers, zero DoF, `SIZE_MAX` waypoint counts,
  multiplication overflow shapes, and tracking sequence constructor overflow.

## Boundaries

- [x] No public C header change.
- [x] No public symbol allowlist or exception change.
- [x] No workflow, version metadata, tag, or GitHub Release change.
- [x] No update to `original/ruckig-main`.
- [x] No visualization asset change.
- [x] No solver, waypoint, tracking evaluator, wrapper, package-manager, or
  default heavy-CI scope change.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_constructor_boundaries\|ruckig_c_header_c\|ruckig_c_header_cpp\|ruckig_c_public_diagnostics"` | Passed; 4/4 tests |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_tests\|ruckig_c_allocation_audit"` | Passed; 2/2 tests |
| Public header / ABI / workflow boundary diff | Empty |
| `original/ruckig-main` and visualization asset boundary diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |
