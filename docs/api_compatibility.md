# API and ABI Compatibility

This document defines the `ruckig_c 0.2.x` patch-release compatibility policy.

## Policy

- Patch releases must not remove public functions from
  `include/ruckig_c/ruckig.h`.
- Patch releases must not change public enum numeric values or result-code
  numeric values.
- Patch releases should not add public API in `0.2.x` unless a separate design
  and version decision explicitly approves it.
- Opaque handle internals may change, but the public C ABI surface must be
  reviewed before every patch release.
- Public header changes must be recorded in `CHANGELOG.md`.

## Release Review

Before each patch release:

1. Review the diff for `include/ruckig_c/ruckig.h`.
2. Confirm C and C++ header compile tests pass.
3. Confirm static and shared builds pass.
4. Generate or inspect exported symbols for the shared library:
   - Linux: `nm -D` or an equivalent tool.
   - Windows: `dumpbin /EXPORTS`, `llvm-nm`, or an equivalent tool.
5. Record whether public symbols, enum values, or result-code values changed in
   the release checklist.

The initial `0.2.x` process records exported symbols as release evidence; it
does not require a strict automated ABI diff yet.
