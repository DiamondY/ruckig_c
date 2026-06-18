# Post-v0.16.0 Calculation Duration Clock Policy Checklist

This checklist records the optional calculation-duration timing policy slice.
It changes enabled calculation-duration measurement from `clock()` to the
existing monotonic platform clock helper while preserving the default disabled
behavior.

## Scope

- [x] Reused `ruckig_platform_monotonic_time_us()` for
  `RUCKIG_C_ENABLE_CALCULATION_DURATION` reporting.
- [x] Preserved default builds returning `0.0` from
  `ruckig_output_get_calculation_duration`.
- [x] Preserved public API declarations, symbols, enum values, and struct
  layout.
- [x] Documented enabled builds as monotonic elapsed microsecond measurement.
- [x] Avoided brittle timing thresholds in tests.

## Boundary

- [x] No public C ABI change.
- [x] No ABI allowlist or exception change.
- [x] No workflow, version metadata, tag, or release change.
- [x] No upstream baseline or visualization asset change.

## Verification

| Command | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_tests\|ruckig_c_platform_clock_custom\|ruckig_c_header_c\|ruckig_c_header_cpp"` | Passed 4/4 |
| `cmake --build out\build\windows-clang-ninja-duration` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja-duration --output-on-failure -R "ruckig_c_tests\|ruckig_c_platform_clock_custom"` | Passed 2/2 |
| `git diff -- docs/abi/public-symbols.txt docs/abi/public-symbol-exceptions.txt docs/abi/exceptions.md .github/workflows/ci.yml` | Empty |
| `git diff -- original/ruckig-main docs/assets/visualization` | Empty |
| `git diff --check` | Passed; CRLF normalization warnings only |
