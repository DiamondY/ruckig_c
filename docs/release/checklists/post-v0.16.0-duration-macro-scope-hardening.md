# Post-v0.16.0 Duration Macro Scope Hardening

## Scope

This slice keeps optional calculation-duration reporting as a library build
option without exporting `RUCKIG_C_ENABLE_CALCULATION_DURATION` as a downstream
CMake usage requirement. It does not change the public C ABI, public symbols,
workflow, version metadata, release state, upstream baseline, visualization
assets, wrappers, or package recipes.

## Changes

- Changed the `ruckig_c` target definition for
  `RUCKIG_C_ENABLE_CALCULATION_DURATION` from `PUBLIC` to `PRIVATE`.
- Kept `ruckig_c_tests` duration-enabled compile definition private.
- Extended the installed CMake consumer fixture to inspect
  `INTERFACE_COMPILE_DEFINITIONS` in addition to instrumentation options.
- Added a default metadata check that rejects installed targets exporting
  `RUCKIG_C_ENABLE_CALCULATION_DURATION`.
- Preserved static/shared `RUCKIG_C_STATIC_DEFINE` metadata checks.
- Documented that `RUCKIG_C_ENABLE_CALCULATION_DURATION` is a library build
  option, not an installed target public feature macro.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_cmake_consumer\|ruckig_c_cmake_consumer_versioned\|ruckig_c_header_c\|ruckig_c_header_cpp\|ruckig_c_public_diagnostics"` | Passed, 5/5 |
| `cmake --build --preset windows-clang-ninja-duration` | Not a current preset; used existing `out\build\windows-clang-ninja-duration` build directory instead. |
| `cmake --build out\build\windows-clang-ninja-duration` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja-duration --output-on-failure -R "ruckig_c_tests\|ruckig_c_platform_clock_custom"` | Passed, 2/2 |
| `cmake --build --preset windows-clang-ninja-shared` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja-shared --output-on-failure -R "ruckig_c_shared_install_consumer\|ruckig_c_header_c\|ruckig_c_header_cpp"` | Passed, 2/2; the Windows shared preset did not configure `ruckig_c_shared_install_consumer`, so the selector matched the header gates only. |
| Public header / ABI allowlist / workflow diff | Empty |
| `original/ruckig-main` and `docs/assets/visualization` diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |
