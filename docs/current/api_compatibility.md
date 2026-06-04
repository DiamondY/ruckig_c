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

## 0.2.2 ABI Evidence Procedure

Shared builds expose a non-invasive helper target:

```powershell
cmake --build build_release_check_shared --target ruckig_c_exported_symbols
```

The target writes a text snapshot under the build tree, for example:

```text
build_release_check_shared/artifacts/abi/0.2.2/windows-exports.txt
build-shared/artifacts/abi/0.2.2/linux-exports.txt
```

On Linux it uses `nm -D --defined-only`. On macOS it uses `nm -gU`. On Windows
it prefers `llvm-readobj --coff-exports` and falls back to `dumpbin /EXPORTS`.
The helper only generates review artifacts; it does not rewrite tracked
documentation and it is not yet a strict ABI-diff fail gate.

GitHub Actions also runs the same shared-build helper for Linux and Windows in
the `Linux exported symbols` and `Windows exported symbols` jobs. Those jobs
upload the generated snapshots as CI artifacts so patch-release evidence can be
reviewed without relying only on a local workstation run.

For `0.2.x`, public header diffs are expected to be limited to version macros,
comments, or documentation-only changes. Any public symbol addition, removal,
signature change, enum numeric-value change, or result-code numeric-value
change requires a separate design and version decision before release.

## 0.2.3 Baseline Comparison

The `v0.2.2` release established the first tracked exported-symbol baseline:

```text
docs/abi/v0.2.2/linux-symbols.txt
docs/abi/v0.2.2/windows-symbols.txt
```

On shared builds, `ruckig_c_compare_exported_symbols` compares the current
generated export snapshot against the platform baseline:

```powershell
cmake --build build_release_check_shared --target ruckig_c_compare_exported_symbols
```

The comparison normalizes `ruckig_*` symbol names, writes a diff summary under
the build tree, for example
`build_release_check_shared/artifacts/abi/0.2.3/windows-export-diff.txt`, and
emits only a warning when differences are found. This is intentional for
`0.2.3`: the comparison is release evidence and review support, not a strict CI
fail gate. The diff summary should be reviewed before release and recorded in
the release checklist.

Upgrade the comparison to a strict fail gate only after at least one patch
release has used the warning/evidence mode successfully on Windows and Linux,
and after the project has a documented exception process for intentional public
ABI changes.

## 0.2.4 Baseline Comparison

The `v0.2.3` release rolls the exported-symbol baseline forward for the next
maintenance line:

```text
docs/abi/v0.2.3/linux-symbols.txt
docs/abi/v0.2.3/windows-symbols.txt
```

Shared builds now write comparison artifacts under an `0.2.4` build-tree path,
for example:

```text
build_release_check_shared/artifacts/abi/0.2.4/windows-exports.txt
build_release_check_shared/artifacts/abi/0.2.4/windows-export-diff.txt
```

The `0.2.4` comparison remains warning/evidence only. Differences must be
reviewed before release and recorded in the release checklist, but the helper
is not a strict CI fail gate.

## Public Header Diff

Review the public header against the previous release tag:

```powershell
git -c safe.directory=E:/Yww/DownLoad/source/ruckig_c diff v0.2.2 -- include/ruckig_c/ruckig.h
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
