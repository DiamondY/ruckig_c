# Post-v0.16.0 Consumer Install And Docs Refresh Checklist

Status: local verification complete; ordinary remote push CI is observed after
the checklist commit is pushed.

This checklist records the
`post-v0.16.0-consumer-install-and-docs-refresh` slice. It upgrades installed
CMake and pkg-config consumer smoke sources to compile and run minimal
`v0.16.0` public diagnostics usage. It does not change the public C API,
public ABI, version metadata, release/tag state, workflow behavior, package
recipes, wrapper publication status, upstream baseline, or visualization
assets.

## Scope

- [x] Updated the installed CMake consumer smoke source to initialize
  `ruckig_diagnostics_t`, verify an invalid limit diagnostic, and run a valid
  `ruckig_calculate_with_diagnostics` path.
- [x] Updated the pkg-config consumer smoke source with the same minimal
  diagnostics usage.
- [x] Preserved the existing basic trajectory calculation smoke behavior.
- [x] Kept installed target/export behavior unchanged.
- [x] Kept package-manager recipes frozen.

## Public API / ABI Boundary

- [x] No exported C function is added.
- [x] No public C function signature is changed.
- [x] No enum numeric value or result-code numeric value is changed.
- [x] No public struct layout or public diagnostics layout is changed.
- [x] `include/ruckig_c/ruckig.h` is unchanged.
- [x] No ABI allowlist or public-symbol exception file is changed.
- [x] No version metadata, tag, GitHub Release, package-manager recipe,
  workflow, upstream baseline, or visualization asset is changed.
- [x] Python and Rust wrappers remain prototype-only.

## Local Gates

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Pass; no rebuild work |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_cmake_consumer\|ruckig_c_public_diagnostics\|ruckig_c_header_c\|ruckig_c_header_cpp"` | Pass, 4/4 |
| `cmake --build --preset windows-clang-ninja-shared` | Pass; no rebuild work |
| `ctest --test-dir out\build\windows-clang-ninja-shared --output-on-failure -R "ruckig_c_shared_install_consumer\|ruckig_c_cmake_consumer\|ruckig_c_public_diagnostics"` | Pass, 2/2 matched tests; `ruckig_c_shared_install_consumer` is not registered in this Windows shared build |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_pkg_config_consumer"` | Not configured in this Windows build; pkg-config consumer remains covered by pkg-config-capable configurations |

## Boundary Checks

| Check | Result |
| --- | --- |
| `git diff -- include/ruckig_c/ruckig.h docs/abi/public-symbols.txt docs/abi/public-symbol-exceptions.txt docs/abi/exceptions.md .github/workflows/ci.yml` | Clean |
| `git diff -- original/ruckig-main docs/assets/visualization` | Clean |
| `git diff --check` | Pass; Git reported expected CRLF normalization warnings only |

## Remote CI

Ordinary remote push CI is observed after pushing this checklist commit to
`main`. The run URL and conclusion should be recorded in the delivery summary.
