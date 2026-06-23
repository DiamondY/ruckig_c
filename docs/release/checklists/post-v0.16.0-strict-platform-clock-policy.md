# Post-v0.16.0 Strict Platform Clock Policy

## Scope

This slice removes the private CPU-time `clock()` fallback from platform timing.
It does not change public C ABI, public symbols, workflows, version metadata,
release state, upstream baseline, visualization assets, wrappers, or
package-manager recipes.

## Changes

- `ruckig_platform_monotonic_time_us` now uses only a custom monotonic provider,
  Windows QueryPerformanceCounter, or POSIX `CLOCK_MONOTONIC`.
- Unsupported platforms fail at compile time unless they provide
  `RUCKIG_C_PLATFORM_CLOCK_HEADER` and `RUCKIG_C_CUSTOM_MONOTONIC_TIME_US`.
- Runtime timestamp failures return `0u` instead of falling back to CPU time.
- Optional calculation-duration reporting returns `0.0` when the start or stop
  timestamp is unavailable.
- Positive soft-interruption budgets do not compute elapsed time from a zero
  timestamp; zero budgets remain immediate.
- CMake sets `_POSIX_C_SOURCE=200809L` privately on targets that compile the
  library sources so Linux builds expose `clock_gettime` and `CLOCK_MONOTONIC`
  regardless of include order.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_platform_clock_custom\|ruckig_c_header_c\|ruckig_c_header_cpp\|ruckig_c_public_diagnostics\|ruckig_c_constructor_boundaries"` | Passed, 5/5 |
| `cmake --build out\build\windows-clang-ninja-duration` | Passed; the historical duration build directory exists, while the named duration preset is not present in current `CMakePresets.json` |
| `ctest --test-dir out\build\windows-clang-ninja-duration --output-on-failure -R "ruckig_c_tests\|ruckig_c_platform_clock_custom"` | Passed, 2/2 |
| `cmake --build --preset windows-clang-ninja-shared` | Passed |
| Shared public-symbol verification | Passed; header allowlist and exported symbols match |
| `rg -n "clock\(" src\ruckig_c\platform_clock.h` | No matches |
| `zig cc -target x86_64-linux-gnu -std=c99 -Wall -Wextra -Wpedantic -D_POSIX_C_SOURCE=200809L -Iinclude -Isrc -c src\ruckig_c\ruckig.c` | Passed |
| Public ABI allowlist / workflow diff | Empty |
| `original/ruckig-main` and `docs/assets/visualization` diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |
