# Post-v0.16.0 CI Static-Analysis Evidence Policy

## Scope

This slice is docs-only. It records local static-analysis evidence and keeps
default CI policy unchanged.

No production code, tests, public headers, ABI allowlists, workflows, version
metadata, release state, upstream baseline, visualization assets, wrappers, or
package-manager recipes are changed.

## Evidence

The local tooling polish series made `.clang-tidy` usable for targeted runs by
using a path-portable `HeaderFilterRegex`.

Local evidence from the same series:

```sh
clang-tidy test/c/linked_library_smoke.c --quiet -- -std=c99 -Iinclude
```

Result: exited successfully locally and emitted only the normal warning summary.

## Policy

- `.clang-tidy` remains local/manual targeted evidence.
- `.clang-format` remains configuration only; no repository-wide formatting
  churn is part of this slice.
- clang-tidy, cppcheck, CodeQL, formatter checks, and coverage upload are not
  added to default CI.
- MSVC full matrix, Windows ASan/UBSan, and macOS sanitizer/oracle/performance
  remain event-driven.
- Future CI expansion requires accepted owner, runtime budget, tool-noise
  policy, and failure triage ownership.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed; no work to do |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_header_c\|ruckig_c_header_cpp\|ruckig_c_public_diagnostics"` | Passed, 3/3 |
| Public header / ABI allowlist / workflow diff | Empty |
| `original/ruckig-main` and `docs/assets/visualization` diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |
