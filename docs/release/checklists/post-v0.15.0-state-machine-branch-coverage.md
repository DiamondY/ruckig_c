# Post-v0.15.0 State-Machine Branch Coverage Checklist

Status: local implementation and evidence complete.

This checklist records the `post-v0.15.0-state-machine-branch-coverage` slice
after the stable `v0.15.0` release and after the broader
`post-v0.15.0-quality-audit` slice. The slice adds deterministic fixed cases
for high-risk state-machine and boundary branches without starting
`0.16.0-design`, changing version metadata, expanding the 184-symbol public C
ABI, creating a tag, publishing wrappers, changing workflow behavior, or
editing ABI allowlists.

## Scope

- [x] Added focused CLI entry `ruckig_c_tests --state-machine-branch-coverage`.
- [x] Added CTest selector `ruckig_c_state_machine_branch_coverage`.
- [x] Added deterministic `input.c` boundary coverage for per-section
  setter/getter counts, null values, clear paths, and waypoint-count changes
  clearing per-section flags.
- [x] Added deterministic `waypoint.c` resume identity coverage for target,
  waypoint values/count, max/min limits, enabled DoF, synchronization,
  per-section constraints, and cleared interrupt state.
- [x] Added deterministic `tracking.c` continuation coverage for empty or
  unstarted continuation handles, wrong DoF/capacity/delta-time resume
  rejection, diagnostics error state, and failed-resume state preservation in
  Fast and Optimized modes.
- [x] Kept heavy random and coverage gates local/manual; default push CI cost
  is not intentionally increased beyond the small deterministic selector.

## Public API / ABI Boundary

- [x] No exported C function is added.
- [x] No public function signature is changed.
- [x] No enum numeric value or result-code numeric value is changed.
- [x] No public struct layout or public diagnostics layout is changed.
- [x] `include/ruckig_c/ruckig.h` is unchanged.
- [x] No ABI allowlist or public-symbol exception file is changed.
- [x] No version metadata, tag, GitHub Release, package-manager recipe,
  workflow, upstream baseline, or visualization asset is changed.
- [x] Python and Rust wrappers remain prototype-only.

## Test Additions

| Area | Fixed coverage |
| --- | --- |
| Input state boundaries | Per-section vector setters reject wrong section count, wrong DoF, null values, and accept exact section/DoF capacity; getters reject null output for nonzero waypoint data; per-section minimum duration rejects `NaN` and negative entries; clearing waypoint count resets per-section flags; clear helpers tolerate null. |
| Waypoint resume identity | Starts an interrupted waypoint solve under allocation guard, verifies unchanged input can continue, mutates one identity dimension at a time, verifies stale resume state is rejected, and checks valid mutations restart an interrupted search while invalid enabled-DoF mutation clears the engine through the invalid-input path. |
| Tracking continuation invalid states | Empty and unstarted continuation handles return `RUCKIG_ERROR_INVALID_INPUT` and mark diagnostics as error. |
| Tracking Fast continuation | Wrong `delta_time` resume is rejected without corrupting the continuation, and the correct handle can still resume to completion. |
| Tracking Optimized continuation | Wrong output capacity and wrong `delta_time` resume are rejected without corrupting the continuation, and the correct handle can still resume to completion with diagnostics self-consistency checks. |

## Local Gates

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Pass |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_state_machine_branch_coverage"` | Pass, 1/1 |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_state_machine_branch_coverage\|ruckig_c_property_invariants\|ruckig_c_tracking_sequence_continuation_api\|ruckig_c_tracking_sequence_fast_continuation\|ruckig_c_tracking_sequence_optimized_continuation\|ruckig_c_waypoint_resume_stress\|ruckig_c_allocation_audit"` | Pass, 7/7 |
| `cmake --build --preset windows-clang-ninja-internal-asserts` | Pass |
| `ctest --test-dir out\build\windows-clang-ninja-internal-asserts --output-on-failure -R "ruckig_c_state_machine_branch_coverage\|ruckig_c_property_invariants\|ruckig_c_tracking_sequence_fast_continuation\|ruckig_c_tracking_sequence_optimized_continuation\|ruckig_c_waypoint_resume_stress"` | Pass, 5/5 |
| `.\tools\coverage\run_coverage.ps1 -CoverageLabel post-v0.15.0-state-machine-branch-coverage` | Pass, 64/64 coverage CTest; summary at `out\coverage\post-v0.15.0-state-machine-branch-coverage\coverage-summary.txt` |
| `cmake --build --preset windows-clang-ninja-shared` | Pass |
| `cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_verify_public_symbols` | Pass |
| `cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_compare_public_exported_symbols` | Pass, public exported symbols match the approved allowlist |
| `git diff -- include/ruckig_c/ruckig.h docs/abi/public-symbols.txt docs/abi/public-symbol-exceptions.txt docs/abi/exceptions.md .github/workflows/ci.yml` | Pass, empty |
| `git diff -- original/ruckig-main docs/assets/visualization` | Pass, empty |
| `git diff --check` | Pass |

## Coverage Result

The local coverage run for this slice produced:

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 8040 | 914 | 88.63% |
| Functions | 456 | 30 | 93.42% |
| Lines | 8609 | 1017 | 88.19% |
| Branches | 4683 | 1370 | 70.75% |

The branch-coverage target of `70.0%+` is reached. Compared with the previous
`post-v0.15.0-quality-audit` coverage run, missed implementation branches drop
from `1427` to `1370`, and branch coverage rises from `69.53%` to `70.75%`.

Remaining branch gaps are still concentrated in solver candidate alternatives,
rare invalid-input guards, infeasible waypoint/tracking candidate combinations,
platform-clock defensive paths, and oracle-protected long-tail branches. Those
remain better handled by targeted fixed cases or oracle-derived regressions
than by broad random expansion in default CI.

## Deferred

- Solver-core branch-family expansion in `ruckig.c` and `position_*_step*.c`.
- Full automatic random-case shrinker.
- Further `input.c`, `waypoint.c`, or `tracking.c` internal helper extraction.
- Public diagnostics, runtime clock hooks, wrapper publication, package
  recipes, upstream baseline upgrades, or `0.16.0-design` transition.
