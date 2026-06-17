# Post-v0.16.0 Tracking Diagnostics Examples Polish Checklist

This checklist records the post-`v0.16.0` tracking diagnostics examples polish
slice. It adds a C example for the stable tracking public diagnostics getter
pattern without changing public ABI, tracking evaluator behavior, wrapper
publication status, upstream baseline, or visualization assets.

## Scope

- [x] Added `examples/c/25_tracking_public_diagnostics.c`.
- [x] Added CMake target `example-ruckig-c-25-tracking-public-diagnostics`.
- [x] Added focused CTest `ruckig_c_examples_tracking_public_diagnostics`.
- [x] Included the example in the aggregate `ruckig_c_examples` target.
- [x] Updated README and current diagnostics/test coverage docs.

## Example Coverage

- [x] Creates public handles through the C API, including `ruckig_t`,
  `ruckig_tracking_t`, `ruckig_input_t`, `ruckig_output_t`, and a sequence
  continuation handle.
- [x] Runs a valid Fast tracking update.
- [x] Initializes `ruckig_diagnostics_t` before each getter call.
- [x] Reads `ruckig_tracking_get_last_public_diagnostics` after tracking update.
- [x] Reads `ruckig_tracking_sequence_continuation_get_last_diagnostics` for an
  unstarted continuation state.
- [x] Uses only stable public diagnostics scope/code/result fields.

## Boundaries

- [x] No public C header change.
- [x] No public symbol allowlist or exception change.
- [x] No workflow, version metadata, tag, or GitHub Release change.
- [x] No change to specialized `ruckig_tracking_get_last_diagnostics`.
- [x] No Optimized evaluator, candidate order, scoring, or strategy behavior
  change.
- [x] No update to `original/ruckig-main`.
- [x] No visualization asset change.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_examples_tracking_public_diagnostics\|ruckig_c_tracking_public_diagnostics\|ruckig_c_public_diagnostics\|ruckig_c_header_c\|ruckig_c_header_cpp"` | Passed; 5/5 tests |
| Public header / ABI / workflow boundary diff | Empty |
| `original/ruckig-main` and visualization asset boundary diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |
