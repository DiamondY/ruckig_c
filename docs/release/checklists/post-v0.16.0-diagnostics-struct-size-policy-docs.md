# Post-v0.16.0 Diagnostics Struct-Size Policy Docs

## Scope

This slice documents and tests the diagnostics `struct_size` safety boundary.
It does not change public C ABI, public symbols, workflows, version metadata,
release state, upstream baseline, visualization assets, wrappers, or
package-manager recipes.

## Policy

Callers that pass a non-NULL `ruckig_diagnostics_t*` must initialize it with
`ruckig_diagnostics_init`. If `struct_size` is smaller than the stable prefix
required by the library, the API returns `RUCKIG_ERROR_INVALID_INPUT` before
the operation runs and does not write diagnostics fields. This preserves the
storage safety and forward-compatibility meaning of `struct_size`.

## Changes

- Updated public header diagnostics documentation.
- Updated `docs/current/api_diagnostics.md`.
- Extended `ruckig_c_public_diagnostics` coverage for calculate/update
  too-small `struct_size` paths with sentinel preservation checks.
- Updated current quality, coverage, roadmap, and index docs.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_public_diagnostics\|ruckig_c_header_c\|ruckig_c_header_cpp"` | Passed, 3/3 |
| ABI allowlist / workflow diff | Empty |
| `original/ruckig-main` and `docs/assets/visualization` diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |
