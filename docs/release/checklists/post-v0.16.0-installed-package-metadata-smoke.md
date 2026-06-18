# Post-v0.16.0 Installed Package Metadata Smoke

## Scope

This slice extends installed CMake consumer smoke coverage without changing the
public C ABI, public headers, workflows, version metadata, release state,
upstream baseline, visualization assets, wrappers, or package-manager recipes.

The goal is metadata regression coverage:

- Static installed `ruckig_c::ruckig_c` target consumers must receive
  `RUCKIG_C_STATIC_DEFINE` from the exported target.
- Shared installed target consumers must not receive `RUCKIG_C_STATIC_DEFINE`.
- Exported target usage requirements must not leak sanitizer or coverage flags
  to ordinary CMake consumers.

The existing white-box `ruckig_c_tests` structure remains unchanged.

## Changes

- Extended `test/consumer/cmake/main.c` with compile-time assertions for
  expected static/shared import metadata.
- Extended `test/consumer/cmake/CMakeLists.txt` with cache-controlled metadata
  expectations and usage-requirement checks for instrumentation flag leakage.
- Updated the installed CMake consumer scripts to pass the expected
  `RUCKIG_C_STATIC_DEFINE` state for static and shared builds.
- Updated current packaging, coverage, code-quality, roadmap, and index
  documentation.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_cmake_consumer\|ruckig_c_windows_manual_static_consumer\|ruckig_c_header_c\|ruckig_c_header_cpp"` | Passed, 5/5 |
| `cmake --build --preset windows-clang-ninja-shared` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja-shared --output-on-failure -R "ruckig_c_shared_install_consumer\|ruckig_c_windows_dll_consumer\|ruckig_c_header_c\|ruckig_c_header_cpp"` | Passed, 3/3 matching tests; `ruckig_c_shared_install_consumer` is not configured in this Windows shared preset |
| Public header / ABI allowlist / workflow diff | Empty |
| `original/ruckig-main` and `docs/assets/visualization` diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |

## Boundary

No heavy random, coverage upload, static-analysis CI gate, package-manager
recipe, wrapper publication, workflow edit, or solver behavior change is part
of this slice.
