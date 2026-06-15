# Post-v0.16.0 Oracle-Backed Long-Tail Coverage Checklist

Status: local verification complete; ordinary remote push CI is observed after
the checklist commit is pushed.

This checklist records the `post-v0.16.0-oracle-backed-long-tail-coverage`
triage slice. It intentionally adds no new tests because the remaining
candidate gaps are analytical long-tail branches, defensive paths, or private
state-machine branches without a compact public-behavior or oracle-backed fixed
case. It does not chase coverage percentage and does not change library runtime
behavior, public API, public ABI, version metadata, release/tag state, workflow
behavior, wrapper publication status, upstream baseline, or visualization
assets.

## Triage Input

The current implementation coverage reference is
`out/coverage/post-v0.15.0-residual-branch-coverage/coverage-summary.txt`:

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 7939 | 758 | 90.45% |
| Functions | 472 | 30 | 93.64% |
| Lines | 8590 | 906 | 89.45% |
| Branches | 4591 | 1198 | 73.91% |

Lowest relevant branch-coverage candidates from that artifact:

| File | Branch coverage | Triage |
| --- | ---: | --- |
| `src/ruckig_c/velocity_third_step2.c` | 60.87% | Remaining misses are analytical synchronization/timing alternatives. Future cases must be oracle-backed or assert stable public timing behavior; no compact new case was selected here. |
| `src/ruckig_c/position_second_step2.c` | 68.75% | Remaining misses are solver timing alternatives. Existing solver/oracle gates already protect representative public behavior. |
| `src/ruckig_c/roots.c` | 83.85% | Roots numeric audit already covers zero/tiny-A, small-scale cubic/quartic, repeated roots, filtering, sorting, residuals, and no-allocation behavior. |
| `src/ruckig_c/profile.c` | 80.16% | Profile context smoke and solver/oracle gates cover the stable profile-check entry points; remaining branches are private validation combinations. |
| `src/ruckig_c/trajectory.c` | 82.00% | The residual coverage slice already covered public trajectory create/accessor/intermediate-duration boundaries; remaining misses are defensive or hard-to-reach internal states. |

`tracking.c`, `tracking_sequence.c`, `waypoint.c`, and `input.c` have lower or
similar percentages in places, but they are not selected for this slice because
their remaining gaps are private state-machine, interruption, or defensive
paths already better protected by focused state/resume, tracking, and random
replay gates.

## Decision

- [x] No new coverage case is added in this slice.
- [x] No fragile white-box probe is added to force private polynomial or
  candidate-order branches.
- [x] Remaining solver long-tail work is deferred until a concrete oracle
  mismatch, public-behavior regression, or clearly stable timing invariant is
  identified.
- [x] Coverage percentage is treated as a risk signal, not a target.

## Future Acceptance Criteria

A future long-tail coverage case is acceptable only if it satisfies at least
one condition:

- It reproduces or prevents a concrete oracle mismatch.
- It verifies public API behavior rather than private branch shape.
- It asserts a stable timing/numeric invariant that does not depend on profile
  candidate ordering or polynomial internals.
- It is small enough to stay deterministic and focused.

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
| `cmake --build --preset windows-clang-ninja` | Pass; no rebuild needed |
| `cmake --build --preset windows-clang-ninja-oracle` | Pass; no rebuild needed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_roots_numeric_audit\|ruckig_c_solver_branch_coverage\|ruckig_c_property_invariants\|ruckig_c_public_diagnostics"` | Pass, 4/4 |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe` | Pass; 4 waypoint section comparisons and 92 oracle comparisons |

## Boundary Checks

| Check | Result |
| --- | --- |
| `git diff -- include/ruckig_c/ruckig.h docs/abi/public-symbols.txt docs/abi/public-symbol-exceptions.txt docs/abi/exceptions.md .github/workflows/ci.yml` | Clean |
| `git diff -- original/ruckig-main docs/assets/visualization` | Clean |
| `git diff --check` | Pass; Git reported expected CRLF normalization warnings only |

## Remote CI

Ordinary remote push CI is observed after pushing this checklist commit to
`main`. The run URL and conclusion should be recorded in the delivery summary.
