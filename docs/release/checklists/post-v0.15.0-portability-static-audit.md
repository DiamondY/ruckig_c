# Post-v0.15.0 Portability And Static Audit Checklist

Status: local evidence complete; ordinary remote push CI is observed after the
checklist commit is pushed.

This checklist records the `post-v0.15.0-portability-static-audit` slice after
the residual branch coverage work. The slice adds portability, private-linkage,
platform-clock, and ABI/export evidence only. It does not start
`0.16.0-design`, change version metadata, expand the 184-symbol public C ABI,
create a tag, publish wrappers, change workflow behavior, edit ABI allowlists,
update the upstream baseline, or touch visualization assets.

## Scope

- [x] Added no production code.
- [x] Added no default CI test burden.
- [x] Added no mandatory clang-tidy, cppcheck, formatter, or style-churn gate.
- [x] Rechecked normal and shared CTest after the private tracking split and
  helper refactors.
- [x] Rechecked Windows exported-symbol evidence against the stable
  `v0.15.0` 184-symbol public C ABI allowlist.
- [x] Ran platform-clock compile probes for the default/private compile path
  and custom monotonic clock hook.

## Public API / ABI Boundary

- [x] No exported C function is added.
- [x] No public function signature is changed.
- [x] No enum numeric value or result-code numeric value is changed.
- [x] No public struct layout or public diagnostics layout is changed.
- [x] `include/ruckig_c/ruckig.h` is unchanged.
- [x] No ABI allowlist or public-symbol exception file is changed.
- [x] No version metadata, tag, GitHub Release, package-manager recipe,
  workflow, upstream baseline, or visualization asset is changed.
- [x] Python and Rust wrappers remain prototype-only.

## Platform Compile Probes

The raw source-file probe for `src\ruckig_c\waypoint.c` must be compiled in a
library/static context on Windows. Without `RUCKIG_C_BUILDING_LIBRARY` or
`RUCKIG_C_STATIC_DEFINE`, `RUCKIG_C_API` expands to `__declspec(dllimport)`,
which is correct for consumers but invalid on exported function definitions.

Corrected probes:

```powershell
zig cc -std=c99 -Wall -Wextra -DRUCKIG_C_STATIC_DEFINE -Iinclude -Isrc -c src\ruckig_c\waypoint.c -o $env:TEMP\ruckig_waypoint_posix_probe.o
zig cc -std=c99 -Wall -Wextra -DRUCKIG_C_STATIC_DEFINE -Iinclude -Isrc -Itest\c -c test\c\platform_clock_custom_compile.c -o $env:TEMP\ruckig_custom_clock_probe.o
```

Both probes passed. The custom-clock probe uses the existing
`test/c/platform_clock_custom_compile.c` source, which defines
`RUCKIG_C_CUSTOM_MONOTONIC_TIME_US` and includes the custom provider header.

## Local Gates

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Pass |
| `cmake --build --preset windows-clang-ninja-shared` | Pass |
| `cmake --build --preset windows-clang-ninja-oracle` | Pass |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure` | Pass, 67/67 |
| `ctest --test-dir out\build\windows-clang-ninja-shared --output-on-failure` | Pass, 67/67 |
| `cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_verify_public_symbols` | Pass |
| `cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_compare_public_exported_symbols` | Pass |
| `zig cc -std=c99 -Wall -Wextra -DRUCKIG_C_STATIC_DEFINE -Iinclude -Isrc -c src\ruckig_c\waypoint.c -o $env:TEMP\ruckig_waypoint_posix_probe.o` | Pass |
| `zig cc -std=c99 -Wall -Wextra -DRUCKIG_C_STATIC_DEFINE -Iinclude -Isrc -Itest\c -c test\c\platform_clock_custom_compile.c -o $env:TEMP\ruckig_custom_clock_probe.o` | Pass |

## Static Audit Policy

This slice records local/manual static-audit guidance rather than adding a new
required push-CI static analyzer. Future clang-tidy or cppcheck use should stay
explicitly scoped to private linkage, include hygiene, unused declarations,
visibility, and platform guards unless a separate CI-cost decision is accepted.

No formatter rewrite or broad C89-to-C99 declaration-style churn is part of
this slice.

## Boundary Checks

| Check | Result |
| --- | --- |
| `git diff -- include/ruckig_c/ruckig.h docs/abi/public-symbols.txt docs/abi/public-symbol-exceptions.txt docs/abi/exceptions.md .github/workflows/ci.yml` | Clean |
| `git diff -- original/ruckig-main docs/assets/visualization` | Clean |
| `git diff --check` | Pass; Git reported expected CRLF normalization warnings only |

## Remote CI

Ordinary remote push CI is observed after pushing this checklist commit to
`main`. The run URL and conclusion should be recorded in the delivery summary.
