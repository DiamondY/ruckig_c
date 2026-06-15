# Post-v0.16.0 Wrapper Public Diagnostics Prototype Smoke Checklist

Status: local verification complete; ordinary remote push CI is observed after
the checklist commit is pushed.

This checklist records the
`post-v0.16.0-wrapper-public-diagnostics-prototype-smoke` slice. It adds
minimal Python and Rust prototype wrapper smoke coverage for the stable
`v0.16.0` public diagnostics C API. It does not change the public C API,
public ABI, version metadata, release/tag state, workflow behavior, package
recipes, wrapper publication status, upstream baseline, or visualization
assets.

## Scope

- [x] Added Python `cffi` declarations for `ruckig_diagnostics_t`, the
  validate/calculate/update `_with_diagnostics` functions, and the tracking
  public diagnostics getters.
- [x] Added Python prototype `DiagnosticScope`, `DiagnosticCode`, and
  `Diagnostics` wrappers.
- [x] Added Python minimal diagnostics methods for `Ruckig`, `Tracking`, and
  `TrackingSequenceContinuation`.
- [x] Added Python smoke tests for invalid input diagnostics, successful
  calculate/update diagnostics, tracking public diagnostics, and unstarted or
  completed continuation diagnostics.
- [x] Added Rust FFI declarations and prototype diagnostics wrappers matching
  the stable C ABI.
- [x] Added Rust smoke tests for invalid input diagnostics, successful
  calculate/update diagnostics, tracking public diagnostics, and continuation
  diagnostics.
- [x] Kept existing specialized tracking diagnostics APIs unchanged.

## Public API / ABI Boundary

- [x] No exported C function is added.
- [x] No public C function signature is changed.
- [x] No enum numeric value or result-code numeric value is changed.
- [x] No public struct layout or public diagnostics layout is changed.
- [x] `include/ruckig_c/ruckig.h` is unchanged.
- [x] No ABI allowlist or public-symbol exception file is changed.
- [x] No version metadata, tag, GitHub Release, package-manager recipe,
  workflow, upstream baseline, or visualization asset is changed.
- [x] Python and Rust wrappers remain prototype-only.
- [x] No Python wheel or Rust crate is published.

## Local Gates

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja-shared` | Pass |
| `python bindings\python_prototype\test_prototype.py` with `RUCKIG_C_SHARED_LIBRARY` pointing at the shared build DLL | Pass, 26/26 |
| `cargo test --manifest-path bindings\rust\Cargo.toml` | Pass, 17/17 |
| `cargo test --manifest-path bindings\rust\Cargo.toml --examples` | Pass |

## Boundary Checks

| Check | Result |
| --- | --- |
| `git diff -- include/ruckig_c/ruckig.h docs/abi/public-symbols.txt docs/abi/public-symbol-exceptions.txt docs/abi/exceptions.md .github/workflows/ci.yml` | Clean |
| `git diff -- original/ruckig-main docs/assets/visualization` | Clean |
| `git diff --check` | Pass; Git reported expected CRLF normalization warnings only |

## Remote CI

Ordinary remote push CI is observed after pushing this checklist commit to
`main`. The run URL and conclusion should be recorded in the delivery summary.
