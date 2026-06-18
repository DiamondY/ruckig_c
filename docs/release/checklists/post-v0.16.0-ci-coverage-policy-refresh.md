# Post-v0.16.0 CI Coverage Policy Refresh Checklist

This checklist records the CI quality policy refresh. It is docs-only and does
not modify workflow, public ABI, release state, upstream baseline, or package
publication status.

## Scope

- [x] Added `docs/current/ci_quality_policy.md`.
- [x] Recorded current routine CI coverage across OS, sanitizer, Valgrind,
  ABI/export, performance, consumer, wrapper, and visualization evidence.
- [x] Recorded non-default items: MSVC full matrix, Windows ASan/UBSan, macOS
  sanitizer/oracle/performance, coverage upload, clang-tidy/format, cppcheck,
  CodeQL, and heavy random.
- [x] Kept `.github/workflows/ci.yml` unchanged.
- [x] Kept future CI expansion event-driven and owner-gated.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed; no work to do |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_header_c\|ruckig_c_header_cpp\|ruckig_c_public_diagnostics"` | Passed; 3/3 tests |
| Public header / ABI / workflow boundary diff | Empty |
| `original/ruckig-main` and visualization asset boundary diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |
