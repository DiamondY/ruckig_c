# Post-v0.15.0 Quality Evidence Refresh Checklist

Status: local evidence complete; ordinary remote push CI is observed after the
checklist commit is pushed.

This checklist records the `post-v0.15.0-quality-evidence-refresh` slice after
`post-v0.15.0-review-followup-quality-hardening`. It refreshes coverage and
hotspot evidence after the tracking split, profile-check context conversion,
private interrupt-context unification, roots guard cleanup, and related
maintainability work. It does not start `0.16.0-design`, change version
metadata, expand the 184-symbol public C ABI, create a tag, publish wrappers,
change workflow behavior, edit ABI allowlists, update the upstream baseline, or
touch visualization assets.

## Scope

- [x] Added no production code.
- [x] Added no default CI test burden.
- [x] Ran a fresh coverage pass with label
  `post-v0.15.0-quality-evidence-refresh`.
- [x] Refreshed current implementation coverage totals.
- [x] Refreshed largest implementation-file hotspot data after the tracking
  behavior split.
- [x] Recorded remaining low-coverage files and triaged the gaps into useful
  fixed-case candidates, oracle/random-protected long tails, and low-value
  defensive paths.

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

## Local Gates

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Pass |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_roots_numeric_audit\|ruckig_c_property_invariants\|ruckig_c_state_machine_branch_coverage\|ruckig_c_solver_branch_coverage\|ruckig_c_tracking_sequence_fast_continuation\|ruckig_c_tracking_sequence_optimized_continuation\|ruckig_c_waypoint_resume_stress\|ruckig_c_allocation_audit"` | Pass, 8/8 |
| `.\tools\coverage\run_coverage.ps1 -CoverageLabel post-v0.15.0-quality-evidence-refresh` | Pass, 69/69 coverage CTest; summary at `out\coverage\post-v0.15.0-quality-evidence-refresh\coverage-summary.txt` |

## Coverage Result

The previous coverage-bearing post-`v0.15.0` slice was
`post-v0.15.0-solver-adjacent-branch-coverage`, with branch coverage `72.94%`.
The external-review hardening slice deliberately did not record a coverage
artifact because its goal was maintainability and parity protection.

The refreshed coverage run produced:

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 7939 | 773 | 90.26% |
| Functions | 472 | 30 | 93.64% |
| Lines | 8590 | 912 | 89.38% |
| Branches | 4591 | 1219 | 73.45% |

Compared with the previous coverage-bearing slice, missed branches drop from
`1239` to `1219`, and branch coverage rises from `72.94%` to `73.45%`. The
increase comes from the external-review hardening tests and refactor shape,
not from a coverage-targeted implementation change in this slice.

## Refreshed Hotspots

Largest implementation `.c` files after the tracking split:

| File | Size |
| --- | ---: |
| `src/ruckig_c/position_third_step2.c` | 65914 bytes |
| `src/ruckig_c/ruckig.c` | 56652 bytes |
| `src/ruckig_c/waypoint.c` | 51955 bytes |
| `src/ruckig_c/tracking_sequence.c` | 43040 bytes |
| `src/ruckig_c/input.c` | 39496 bytes |
| `src/ruckig_c/position_third_step1.c` | 36634 bytes |
| `src/ruckig_c/tracking_update.c` | 28464 bytes |
| `src/ruckig_c/tracking.c` | 24091 bytes |
| `src/ruckig_c/profile.c` | 21482 bytes |
| `src/ruckig_c/roots.c` | 10695 bytes |
| `src/ruckig_c/trajectory.c` | 9682 bytes |
| `src/ruckig_c/brake.c` | 7286 bytes |

Lowest branch-coverage implementation files in the refreshed run:

| File | Branch coverage | Interpretation |
| --- | ---: | --- |
| `platform_clock.h` | 37.50% | Defensive platform and custom-clock compile paths; better covered by explicit platform probes than branch probes. |
| `output.c` | 60.42% | Public output setter/getter and invalid-output boundaries; good candidate for a small residual fixed-case slice if refresh data is used. |
| `velocity_third_step2.c` | 60.87% | Analytical synchronization alternatives; candidate for careful white-box plus oracle-backed fixed cases. |
| `tracking.c` | 62.40% | Lifecycle/config/diagnostics boundaries after split; broad state-machine coverage already exists, so remaining gaps need triage before new tests. |
| `tracking_sequence.c` | 65.10% | Continuation/sequence long-tail and invalid-state combinations; protected by continuation selectors and internal assertions. |
| `alloc.c` | 66.67% | Allocation failure and accounting branches; mostly defensive and not worth forced tests unless allocator fault injection is added. |
| `utils.c` | 66.67% | Tiny helper file with limited branch count; low priority. |
| `position_second_step2.c` | 68.75% | Synchronization timing alternatives; candidate for oracle-backed residual solver cases. |
| `waypoint.c` | 68.89% | Resume/optimizer branch families; already covered by stress and branch-queue saturation tests, with some long-tail candidates deferred. |

## Residual Coverage Guidance

High-value candidates for the next residual coverage slice are
`velocity_third_step2.c`, `position_second_step2.c`, and selected `output.c`
boundary cases. `tracking.c`, `tracking_sequence.c`, and `waypoint.c` remain
important, but new tests should be added only when the case is a meaningful
state-machine invariant rather than a low-value branch probe.

## Remote CI

Ordinary remote push CI is observed after pushing this checklist commit to
`main`. The run URL and conclusion should be recorded in the delivery summary.
