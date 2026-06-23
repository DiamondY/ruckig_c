# Post-v0.16.0 Manual Analysis Evidence Policy

## Scope

This docs-only slice records how local/manual static-analysis evidence should
be used for touched high-risk implementation files. It does not modify default
CI workflow, public C ABI, public symbols, version metadata, release state,
upstream baseline, visualization assets, wrappers, package recipes, or source
formatting.

## Changes

- Added touched-file manual static-analysis guidance to
  `docs/current/ci_quality_policy.md`.
- Documented high-risk implementation file families where targeted
  `clang-tidy` evidence is preferred when locally available.
- Kept clang-tidy, cppcheck, CodeQL, formatter gates, and coverage upload out
  of default CI.
- Updated maintenance watch, code-quality audit, roadmap, and documentation
  index entries.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed; no work to do |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_header_c\|ruckig_c_header_cpp\|ruckig_c_public_diagnostics"` | Passed, 3/3 |
| `clang-tidy test/c/linked_library_smoke.c --quiet -- -std=c99 -Iinclude` | Passed; exited successfully with the normal warning summary (`1837 warnings generated`). |
| Public header / ABI allowlist / workflow diff | Empty |
| `original/ruckig-main` and `docs/assets/visualization` diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |
