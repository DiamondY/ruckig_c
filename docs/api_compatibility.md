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

## Public Header Diff

Review the public header against the previous release tag:

```powershell
git -c safe.directory=E:/Yww/DownLoad/source/ruckig_c diff v0.2.0 -- include/ruckig_c/ruckig.h
```

For `0.2.x` patch releases, expected changes are limited to version macros and
documentation comments unless a separate design and version decision explicitly
approves a public API change.

## Exported Symbol Commands

Linux shared-library builds can inspect exported public symbols with:

```sh
nm -D --defined-only build-shared/libruckig_c.so
```

Windows DLL builds can inspect exports with one of:

```powershell
dumpbin /EXPORTS build_release_check_shared\ruckig_c.dll
llvm-nm --defined-only build_release_check_shared\ruckig_c.dll
llvm-readobj --coff-exports build_release_check_shared\ruckig_c.dll
```

Record the command output summary in the release checklist. At minimum confirm
that lifecycle, input, output, trajectory, calculate, update, and reset APIs are
exported and that no unintended public symbols were added.
