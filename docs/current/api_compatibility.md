# API and ABI Compatibility

This document defines the `ruckig_c 0.2.x` patch-release compatibility policy.
For the `0.3.0` hardening release, it also defines the public ABI guardrails
that are built on the final planned `0.2.x` baseline.

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
- The approved public symbol allowlist is `docs/abi/public-symbols.txt`. It is
  generated from the `RUCKIG_C_API` declarations in
  `include/ruckig_c/ruckig.h`. It was established from the `v0.2.5` baseline
  for `0.3.0` hardening and is expanded in `0.4.0` only through the separate
  original-parity design and
  `docs/abi/public-symbol-exceptions.txt`.

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

## 0.2.5 Strict Gate Design And Pre-0.3.0 Baseline

The `v0.2.4` release rolls the exported-symbol baseline forward again:

```text
docs/abi/v0.2.4/linux-symbols.txt
docs/abi/v0.2.4/windows-symbols.txt
```

Shared builds now write comparison artifacts under an `0.2.5` build-tree path,
for example:

```text
build_release_check_shared/artifacts/abi/0.2.5/windows-exports.txt
build_release_check_shared/artifacts/abi/0.2.5/windows-export-diff.txt
```

The `0.2.5` objective is to design a strict ABI diff gate, not to enable strict
CI failure immediately. The default remains warning/evidence-only so release
maintainers can review public header diffs, exported-symbol diffs, enum values,
and result-code values before deciding whether the process is mature enough to
fail CI automatically.

`v0.2.5` is the planned final `0.2.x` stabilization release before `0.3.0`,
so its release artifacts should be treated as the pre-`0.3.0`
C ABI baseline. After publication, save Linux and Windows exported-symbol
baselines under:

```text
docs/abi/v0.2.5/linux-symbols.txt
docs/abi/v0.2.5/windows-symbols.txt
```

Strict exported-symbol diff failure is still not enabled in `v0.2.5`. It
continues as a `0.3.0` trial/evidence topic or future emergency patch design
item after the exception process and reproducibility requirements below are
satisfied.

After publishing `v0.2.5`, `docs/abi/v0.2.5/` is the active comparison
baseline for `0.3.0`:

```text
docs/abi/v0.2.5/linux-symbols.txt
docs/abi/v0.2.5/windows-symbols.txt
```

The Linux `v0.2.5` baseline records the actual shared-library export set from
the release workflow. It includes implementation-internal `ruckig_*` symbols
that are currently visible on Linux because strict symbol visibility is not yet
enforced. Treat this as ABI evidence for drift review, not as approval to use
those internal symbols as public API. The public C API remains the declarations
in `include/ruckig_c/ruckig.h`.

`0.3.0` cleans up that Linux export surface. Non-Windows
shared builds use hidden symbol visibility so only `RUCKIG_C_API` declarations
are exported. Linux builds additionally use a linker version script generated
from `docs/abi/public-symbols.txt` so the dynamic symbol table is constrained
to the approved public allowlist. Removing the historical Linux
implementation-internal exports is not a public API removal because those
symbols were never declared in the public header or listed in
`docs/abi/public-symbols.txt`.

The `v0.2.5` Linux baseline contains `127` normalized `ruckig_*` symbols. The
approved public allowlist contains `66` symbols from the public header. The
remaining `61` Linux baseline symbols are implementation-internal or
test/debug allocation helpers that must not be treated as supported consumer
entry points. The detailed classification is recorded in
`docs/abi/v0.2.5/linux-symbol-review.md`.

Shared builds on `main` now write `0.3.0` comparison artifacts under:

```text
build_release_check_shared/artifacts/abi/0.3.0/windows-exports.txt
build_release_check_shared/artifacts/abi/0.3.0/windows-export-diff.txt
build_release_check_shared/artifacts/abi/0.3.0/windows-public-export-diff.txt
build-shared/artifacts/abi/0.3.0/linux-exports.txt
build-shared/artifacts/abi/0.3.0/linux-export-diff.txt
build-shared/artifacts/abi/0.3.0/linux-public-export-diff.txt
```

Strict exported-symbol diff enforcement can be enabled only after all of these
conditions are true:

- At least two consecutive patch releases have generated Linux and Windows ABI
  evidence from the release process.
- The baseline files can be regenerated reproducibly from the release tag.
- Public header diff review has been completed and recorded for those releases.
- An exception process exists for intentional ABI changes.
- CI output exposes a machine-readable diff summary that is easy to audit.

The public-only comparison target is:

```powershell
cmake --build build_release_check_shared --target ruckig_c_compare_public_exported_symbols
```

It compares the current shared-library exports against
`docs/abi/public-symbols.txt` and the `v0.2.5` platform baseline, then writes a
report such as:

```text
build_release_check_shared/artifacts/abi/0.3.0/windows-public-export-diff.txt
build-shared/artifacts/abi/0.3.0/linux-public-export-diff.txt
```

The report includes machine-readable summary fields and human-readable symbol
sections. Public symbol additions fail strict mode unless the symbol has an
explicit `allow-add ruckig_symbol_name` entry in
`docs/abi/public-symbol-exceptions.txt`; that file is only for separately
approved public API additions. By default the target is warning/evidence-only.
Setting `RUCKIG_C_STRICT_PUBLIC_ABI=ON` makes public ABI drift fail the target.
The dedicated Linux and Windows exported-symbol GitHub Actions jobs currently
run the public-only comparison through the strict comparison script in
non-blocking trial mode while uploading the same exported-symbol artifacts and
diff reports for review. Ordinary local shared builds default to
warning/evidence mode unless the option is explicitly enabled. Promote the CI
jobs to a strict fail gate only after the artifact review path is stable enough
to diagnose Linux and Windows drift without blocking routine maintenance.

macOS shared builds generate a Mach-O exported-symbol snapshot and verify the
public symbol allowlist, but they do not run a historical exported-symbol diff
for `v0.2.5` because no `docs/abi/v0.2.5/macos-symbols.txt` baseline exists.
The `0.3.0` macOS job is the first shared/export evidence bootstrap and should
be treated as the baseline starting point for future macOS ABI policy.

The tracked public allowlist can be verified against the current public header
without rewriting tracked documentation:

```powershell
cmake --build build_release_check_shared --target ruckig_c_verify_public_symbols
```

That target extracts declarations marked `RUCKIG_C_API` from
`include/ruckig_c/ruckig.h`, writes a generated allowlist artifact such as
`build_release_check_shared/artifacts/abi/0.3.0/public-symbols-from-header.txt`,
and fails if it differs from `docs/abi/public-symbols.txt`.

ABI exception policy:

- Allowed without separate version decision: version macro changes, comments,
  documentation-only edits, and release-evidence text.
- Forbidden in `0.2.x` patch releases: public function removal, public function
  signature changes, enum numeric-value changes, result-code numeric-value
  changes, and unreviewed exported-symbol removal.
- New public API in `0.2.x` requires a separate design and version decision,
  plus `CHANGELOG.md`, API compatibility documentation updates, public
  allowlist updates, and an explicit entry in
  `docs/abi/public-symbol-exceptions.txt`.
- Solver behavior changes require an oracle-proven bug fix and a retained
  regression case.

The full policy is tracked in `docs/abi/exceptions.md`.

## Public Header Diff

Review the public header against the previous release tag. For the `0.3.0`
hardening release, compare against the final planned `0.2.x` stabilization
baseline:

```powershell
git -c safe.directory=E:/Yww/DownLoad/source/ruckig_c diff v0.2.5 -- include/ruckig_c/ruckig.h
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

## 0.4.0 Public API Expansion

`0.4.0` intentionally expands the public C ABI after the `v0.3.0`
hardening release. This is not a `0.2.x` patch-release exception and not an
accidental exported-symbol drift. The approved design is
`docs/design/0.4.0_original_parity.md`.

The expansion adds public entry points for:

- waypoint-aware lifecycle constructors;
- `max_number_of_waypoints` inspection;
- global max/min position accessors;
- intermediate waypoint set/get/clear APIs;
- per-section constraint set/get/clear/has APIs;
- interrupt-calculation-duration storage APIs;
- multi-section trajectory section and intermediate-duration queries;
- deterministic local intermediate-position filtering.

The approved public allowlist is now the `0.4.0` public header surface
and contains `117` symbols. The `51` new symbols are listed as `allow-add`
entries in `docs/abi/public-symbol-exceptions.txt` so strict public ABI trials
can distinguish approved minor-version additions from unintended drift.

Compatibility rules for `0.4.0`:

- Existing `v0.3.0` public functions must not be removed or have their
  signatures changed.
- Existing enum numeric values and result-code numeric values must not change.
- New optimizer internals must not be exported.
- `original/ruckig-main` remains frozen as the Ruckig Community `0.17.3`
  reference baseline.
- No-waypoint target-solver behavior must remain covered by the frozen C++
  oracle tests.
- Waypoint optimizer evidence uses local invariants, fixed regression cases,
  section-level target-solver oracle checks, and optional non-blocking
  cloud/Pro black-box comparison evidence when available.

Current local build artifacts continue to use the historical design-line
artifact directory for `0.4.0` evidence:

```text
out/build/<preset>/artifacts/abi/0.4.0-design/
```

Use these checks during release closeout and follow-up ABI review:

```powershell
cmake --build out\build\windows-clang-ninja --target ruckig_c_verify_public_symbols
cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_compare_public_exported_symbols
```

## 0.4.1 Stabilization Policy

`0.4.1` is a no-new-public-C-API stabilization release for the `0.4.x`
original-surface parity line. It keeps the `v0.4.0` public C symbol set
unchanged and uses the deepened waypoint optimizer evidence as release
hardening, not as a new ABI expansion.

Compatibility rules for `0.4.1`:

- `include/ruckig_c/ruckig.h` may change only for version macros during
  release closeout.
- No public C functions may be added, removed, renamed, or signature-changed.
- Enum numeric values and result-code numeric values must remain unchanged.
- `docs/abi/public-symbols.txt` remains the authoritative `117`-symbol public
  allowlist.
- Exported-symbol comparison against the `v0.4.0` public set must report zero
  public additions and zero public removals.
- Optimizer diagnostics for `0.4.1` must remain internal, test-only, benchmark
  output, or release artifact summary evidence. They must not become public
  getters in this release.
- `interrupt_calculation_duration` remains a stored API-surface parity field.
  `0.4.1` does not claim soft interruption checkpoints, best-feasible timeout
  fallback, or hard real-time waypoint optimization.

`0.4.1` ABI artifact output paths use the release-specific directory:

```text
out/build/<preset>/artifacts/abi/0.4.1/
```

## 0.4.2 Coverage Closeout Policy

`0.4.2` is a no-new-public-C-API patch release for the `0.4.x`
original-surface parity line. It records coverage, tracking design scope, and
soft-interruption design boundaries without changing the exported C ABI.

Compatibility rules for `0.4.2`:

- `include/ruckig_c/ruckig.h` may change only for version macros during release
  closeout.
- No public C functions may be added, removed, renamed, or signature-changed.
- Enum numeric values and result-code numeric values must remain unchanged.
- `docs/abi/public-symbols.txt` remains the authoritative `117`-symbol public
  allowlist.
- Exported-symbol comparison against the approved public set must report zero
  public additions and zero public removals.
- Tracking interface material in `0.4.2` is design-only and must not add public
  symbols.
- `interrupt_calculation_duration` remains a stored API-surface parity field.
  `0.4.2` does not claim soft interruption checkpoints, best-feasible timeout
  fallback, or hard real-time waypoint optimization.

`0.4.2` ABI artifact output paths use the release-specific directory:

```text
out/build/<preset>/artifacts/abi/0.4.2/
```

## 0.5.0 Tracking API Baseline

`v0.5.0` stabilizes the public C tracking ABI that was added on the
`0.5.0-design` line. `v0.4.2` remains the prior stable baseline and original
parity coverage closeout. The `v0.5.0` release must not remove or modify any
existing `v0.4.2` public function, function signature, enum numeric value, or
result-code numeric value.

Compatibility rules for the `v0.5.0` tracking line:

- Existing `v0.4.2` public symbols must remain exported.
- New public symbols are limited to the accepted tracking ABI in
  `docs/design/tracking_interface.md`.
- `docs/abi/public-symbols.txt` contains 164 approved public symbols: the
  `v0.4.2` 117-symbol set plus 47 intentional tracking symbols.
- `docs/abi/public-symbol-exceptions.txt` records the intentional
  `0.5.0-design` tracking additions as `allow-add` entries.
- `RUCKIG_TRACKING_OPTIMIZED` is a public enum value but its `0.5.x` behavior
  is `RUCKIG_ERROR_UNSUPPORTED`, not an alias to Fast mode. Implementation is
  deferred to `0.6.0-design`.
- Tracking internals, workspace structures, and candidate state must not be
  exported.
- `original/ruckig-main` remains frozen as the Ruckig Community `0.17.3`
  reference baseline.
- Tracking evidence is local invariant and smoke evidence. It must not be
  described as source-level oracle parity or formal Ruckig Pro/cloud numerical
  equivalence.

`v0.5.0` ABI artifact output paths use the release evidence directory:

```text
out/build/<preset>/artifacts/abi/0.5.0/
```

Use these checks during release evidence review:

```powershell
cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_verify_public_symbols
cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_compare_public_exported_symbols
```

## v0.6.0 Optimized Tracking API Baseline

`v0.6.0` stabilizes the bounded local Optimized tracking MVP on top of the
stable `v0.5.0` tracking ABI. Existing `v0.5.0` public functions, function
signatures, enum numeric values, and result-code numeric values remain
unchanged.

Compatibility rules for the `v0.6.0` tracking line:

- Existing `v0.5.0` public symbols must remain exported.
- New public symbols are limited to the accepted Optimized tracking alpha API
  in `docs/design/tracking_optimized_mode.md`.
- `docs/abi/public-symbols.txt` contains 169 approved public symbols: the
  `v0.5.0` 164-symbol set plus 5 intentional Optimized tracking symbols.
- `docs/abi/public-symbol-exceptions.txt` records the intentional
  `0.6.0-design` additions as `allow-add` entries.
- `ruckig_tracking_calculation_status_t` is additive. Existing enum numeric
  values and result-code numeric values are unchanged.
- `RUCKIG_TRACKING_OPTIMIZED` has bounded local behavior. This is not a formal
  global optimality guarantee and not a Pro/cloud equivalence claim.
- Tracking internals, candidate buffers, scoring helpers, and workspace
  structures must not be exported.
- `original/ruckig-main` remains frozen as the Ruckig Community `0.17.3`
  reference baseline.

`v0.6.0` ABI artifact output paths use the release evidence directory:

```text
out/build/<preset>/artifacts/abi/0.6.0/
```

## 0.7.0-alpha.2 Tracking Strategy and Diagnostics API Baseline

`0.7.0-alpha.2` is an evidence line on `main`, not a stable release. It adds
high-level Optimized tracking strategy presets and one diagnostics snapshot
getter on top of the stable `v0.6.0` Optimized tracking ABI. Existing
`v0.6.0` public functions, function signatures, enum numeric values, and
result-code numeric values remain unchanged.

Compatibility rules for the `0.7.0-alpha.2` strategy and diagnostics line:

- Existing `v0.6.0` public symbols must remain exported.
- New public symbols are limited to the accepted strategy preset controls in
  `docs/design/tracking_optimized_mode.md`:
  `ruckig_tracking_set_optimized_strategy` and
  `ruckig_tracking_get_optimized_strategy`, plus the accepted diagnostics
  snapshot getter `ruckig_tracking_get_last_diagnostics`.
- `docs/abi/public-symbols.txt` contains 172 approved public symbols: the
  `v0.6.0` 169-symbol set plus 2 intentional Optimized tracking strategy
  symbols and 1 intentional diagnostics getter.
- `docs/abi/public-symbol-exceptions.txt` records the intentional
  `0.7.0-alpha` and `0.7.0-alpha.2` additions as `allow-add` entries.
- `ruckig_tracking_optimized_strategy_t` is additive. Existing enum numeric
  values and result-code numeric values are unchanged.
- `ruckig_tracking_diagnostics_t` is additive. Reserved fields are zeroed and
  kept for future-compatible snapshot expansion.
- The default strategy is `RUCKIG_TRACKING_OPTIMIZED_BALANCED`; invalid
  strategy values return `RUCKIG_ERROR_INVALID_INPUT`.
- Strategy presets are local quality controls. They are not a formal global
  optimality guarantee and not a Pro/cloud equivalence claim.
- Diagnostics expose score summary, aggregate step counts, and named
  candidate-family counters. They do not expose internal weights, family masks,
  raw tuning knobs, or optimizer workspace.
- Tracking internals, strategy config tables, candidate generation helpers,
  scoring helpers, and workspace structures must not be exported.
- `original/ruckig-main` remains frozen as the Ruckig Community `0.17.3`
  reference baseline.

`0.7.0-alpha.2` ABI artifact output paths use the design-line evidence
directory:

```text
out/build/<preset>/artifacts/abi/0.7.0-alpha.2/
```

## 0.7.0 Release Readiness ABI Review

`0.7.0-readiness` is a release-readiness evidence audit on `main`, not a
stable release and not a release candidate. It reviews the current
`0.7.0-alpha.2` strategy and diagnostics ABI as the intended future `v0.7.0`
stable baseline.

Compatibility rules for the readiness audit:

- Existing `v0.6.0` public symbols remain exported.
- The only public symbols intended for future `v0.7.0` stabilization beyond
  `v0.6.0` are the already-approved strategy preset controls and diagnostics
  getter:
  `ruckig_tracking_set_optimized_strategy`,
  `ruckig_tracking_get_optimized_strategy`, and
  `ruckig_tracking_get_last_diagnostics`.
- `docs/abi/public-symbols.txt` remains at 172 approved public symbols.
- No public C function, function signature, enum numeric value, or result-code
  numeric value changes in readiness.
- `CMakeLists.txt` project version and `RUCKIG_C_VERSION_*` macros remain
  `0.6.0`; a future stable closeout must update them to `0.7.0`.
- ABI artifact output paths remain on the design-line evidence directory until
  stable closeout:

```text
out/build/<preset>/artifacts/abi/0.7.0-alpha.2/
```

If stable `v0.7.0` closeout is accepted later, the closeout must move ABI
artifact output paths to:

```text
out/build/<preset>/artifacts/abi/0.7.0/
```
