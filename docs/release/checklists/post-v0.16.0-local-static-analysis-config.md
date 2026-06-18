# Post-v0.16.0 Local Static Analysis Config Checklist

This checklist records the local static-analysis and formatting configuration
slice. It adds developer configuration only; it does not format existing
sources, change default CI, change public ABI, or change release state.

## Scope

- [x] Added `.clang-format` aligned with the existing two-space C style and
  disabled include sorting to avoid broad churn.
- [x] Added `.clang-tidy` with low-noise analyzer, bugprone, performance,
  portability, and selected readability checks.
- [x] Kept clang-format and clang-tidy as local/manual evidence.
- [x] Did not add a formatter, clang-tidy, static analyzer, or coverage gate
  to `.github/workflows/ci.yml`.
- [x] Did not run automatic formatting over existing sources.

## Usage

Manual developers may run targeted checks on edited files. Example:

```powershell
clang-format --dry-run --Werror src\ruckig_c\input.c
clang-tidy src\ruckig_c\input.c -- -std=c99 -Iinclude -Isrc
```

Any future default CI adoption must be handled by a separate CI policy slice
that accepts noise budget, platform availability, and maintenance ownership.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed; no work to do |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_header_c\|ruckig_c_header_cpp\|ruckig_c_public_diagnostics"` | Passed; 3/3 tests |
| Public header / ABI / workflow boundary diff | Empty |
| `original/ruckig-main` and visualization asset boundary diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |
