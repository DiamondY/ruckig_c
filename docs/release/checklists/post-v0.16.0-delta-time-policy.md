# Post-v0.16.0 Delta-Time Policy Checklist

This checklist records the constructor `delta_time` policy hardening slice. It
rejects invalid control-cycle values in public OTG constructors while retaining
the existing zero-`delta_time` compatibility path for offline calculation.

## Scope

- [x] Rejected negative `delta_time` in `ruckig_create` and
  `ruckig_create_with_waypoints`.
- [x] Rejected NaN and infinite `delta_time` in both public OTG constructors.
- [x] Preserved `delta_time == 0.0` constructor compatibility.
- [x] Confirmed zero `delta_time` with `RUCKIG_DURATION_DISCRETE` remains a
  validation/calculation error rather than a constructor error.
- [x] Added constructor selector coverage for ordinary and waypoint OTG
  constructors.

## Boundaries

- [x] No public C header change.
- [x] No public symbol allowlist or exception change.
- [x] No workflow, version metadata, tag, or GitHub Release change.
- [x] No update to `original/ruckig-main`.
- [x] No visualization asset change.
- [x] No solver, wrapper, package-manager, or default heavy-CI scope change.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_constructor_boundaries\|ruckig_c_tests\|ruckig_c_public_diagnostics\|ruckig_c_header_c\|ruckig_c_header_cpp"` | Passed; 5/5 tests |
| Public header / ABI / workflow boundary diff | Empty |
| `original/ruckig-main` and visualization asset boundary diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |
