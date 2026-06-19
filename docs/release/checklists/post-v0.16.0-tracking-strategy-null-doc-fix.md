# Post-v0.16.0 Tracking Strategy NULL Documentation Fix

## Scope

This slice fixes a public header documentation mismatch only. It does not
change runtime behavior, public C ABI, public symbols, workflows, version
metadata, release state, upstream baseline, visualization assets, wrappers, or
package-manager recipes.

## Change

`ruckig_tracking_get_optimized_strategy(NULL)` already returns
`RUCKIG_TRACKING_OPTIMIZED_BALANCED`, matching the constructor default
optimized tracking strategy. The public header previously documented the NULL
fallback as Stable. The header now documents the NULL fallback as Balanced.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_header_c\|ruckig_c_header_cpp\|ruckig_c_tracking_public_diagnostics\|ruckig_c_public_diagnostics"` | Passed, 4/4 |
| ABI allowlist / workflow diff | Empty |
| `original/ruckig-main` and `docs/assets/visualization` diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |
