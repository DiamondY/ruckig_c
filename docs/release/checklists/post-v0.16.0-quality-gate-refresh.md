# Post-v0.16.0 Quality Gate Refresh Checklist

This checklist records the docs-only quality gate refresh after constructor
capacity and `delta_time` hardening. It adds no production code and does not
change public ABI, release state, workflow, upstream baseline, wrappers,
package recipes, or visualization assets.

## Scope

- [x] Recorded constructor capacity overflow and invalid `delta_time` handling
  as covered baseline risks.
- [x] Recorded checked arithmetic as the default rule for public constructor
  derived counts.
- [x] Recorded that new `count + 1`, `count * dofs`, or `waypoints * dofs`
  constructor paths must use shared checked-size helpers.
- [x] Kept coverage-percentage work out of scope unless backed by public
  behavior, oracle evidence, a reproducible failure, or a stable invariant.
- [x] Kept sanitizer, static analyzer, performance, coverage, and heavy random
  gates as local/manual evidence unless a future CI policy slice accepts them.

## Boundaries

- [x] No production-code change.
- [x] No public C header change.
- [x] No public symbol allowlist or exception change.
- [x] No workflow, version metadata, tag, or GitHub Release change.
- [x] No update to `original/ruckig-main`.
- [x] No visualization asset change.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed; no work to do |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure` | Passed; 75/75 tests |
| `cmake --build --preset windows-clang-ninja-shared` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja-shared --output-on-failure -R "ruckig_c_constructor_boundaries\|ruckig_c_public_diagnostics\|ruckig_c_header_c\|ruckig_c_header_cpp"` | Passed; 4/4 tests |
| `cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_verify_public_symbols` | Passed; no work to do |
| `cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_compare_public_exported_symbols` | Passed; public exported symbols match the approved allowlist |
| Public header / ABI / workflow boundary diff | Empty |
| `original/ruckig-main` and visualization asset boundary diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |

## Optional Local Evidence

The internal-asserts preset remains optional/manual for this slice and is not a
new default CI gate.
