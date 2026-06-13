# Post-v0.15.0 Residual Branch Coverage Checklist

Status: local evidence complete; ordinary remote push CI is observed after the
checklist commit is pushed.

This checklist records the `post-v0.15.0-residual-branch-coverage` slice after
the quality evidence refresh and random shrinker MVP. It adds a compact
deterministic public-boundary case for the remaining high-value `output.c` and
`trajectory.c` coverage gaps. It does not start `0.16.0-design`, change version
metadata, expand the 184-symbol public C ABI, create a tag, publish wrappers,
change workflow behavior, edit ABI allowlists, update the upstream baseline, or
touch visualization assets.

## Target Selection

The refreshed baseline from
`out/coverage/post-v0.15.0-quality-evidence-refresh/coverage-summary.txt` was:

| Metric | Coverage |
| --- | ---: |
| Regions | 90.26% |
| Functions | 93.64% |
| Lines | 89.38% |
| Branches | 73.45% |

Selected files:

| File | Baseline branch coverage | Reason |
| --- | ---: | --- |
| `src/ruckig_c/output.c` | 60.42% | Low branch coverage in public create/null/default boundary behavior; compact API tests can verify behavior without white-box internals. |
| `src/ruckig_c/trajectory.c` | 76.00% | Public trajectory create/accessor/intermediate-duration boundaries are meaningful and easy to validate through API behavior. |

Deferred files:

| File | Reason |
| --- | --- |
| `src/ruckig_c/velocity_third_step2.c` | Remaining gaps are analytical synchronization alternatives; new tests should be oracle-backed or assert only stable timing invariants, not force polynomial internals. |
| `src/ruckig_c/position_second_step2.c` | Same as above; remaining private branches are valid future candidates but were not needed to reach the slice target. |
| `tracking.c`, `tracking_sequence.c`, `waypoint.c`, `input.c` | Out of scope for this residual slice unless refreshed data showed them as the highest-value risk, which it did not. |

## Scope

- [x] Extended `ruckig_c_property_invariants` with public output and trajectory
  boundary invariants.
- [x] Covered invalid output/trajectory create calls.
- [x] Covered null output getters for time/section/calculation duration and
  vector/trajectory accessors.
- [x] Covered invalid trajectory intermediate-duration calls on both invalid
  and valid waypoint trajectories.
- [x] Added no new default heavy random gate.
- [x] Added no production code.

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

## Coverage Result

The coverage run:

```powershell
.\tools\coverage\run_coverage.ps1 -CoverageLabel post-v0.15.0-residual-branch-coverage
```

passed 72/72 coverage CTest cases and produced
`out\coverage\post-v0.15.0-residual-branch-coverage\coverage-summary.txt`.

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 7939 | 758 | 90.45% |
| Functions | 472 | 30 | 93.64% |
| Lines | 8590 | 906 | 89.45% |
| Branches | 4591 | 1198 | 73.91% |

Compared with the refreshed baseline, missed branches drop from `1219` to
`1198`, and branch coverage rises from `73.45%` to `73.91%`. The `73.5%+`
target is met without adding brittle solver probes.

Touched-file movement:

| File | Baseline branch coverage | Current branch coverage |
| --- | ---: | ---: |
| `src/ruckig_c/output.c` | 60.42% | 85.42% |
| `src/ruckig_c/trajectory.c` | 76.00% | 82.00% |

## Local Gates

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Pass |
| `cmake --build --preset windows-clang-ninja-oracle` | Pass |
| `cmake --build --preset windows-clang-ninja-shared` | Pass |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_solver_branch_coverage\|ruckig_c_roots_numeric_audit\|ruckig_c_property_invariants\|ruckig_c_state_machine_branch_coverage\|ruckig_c_allocation_audit"` | Pass, 5/5 |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe` | Pass |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --random 100000 --seed 1` | Pass |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --random 100000 --seed 2` | Pass |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --random 100000 --seed 41` | Pass |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --random-per-dof 100000 --seed 1` | Pass |
| `.\tools\coverage\run_coverage.ps1 -CoverageLabel post-v0.15.0-residual-branch-coverage` | Pass, 72/72 coverage CTest |
| `cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_verify_public_symbols` | Pass |
| `cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_compare_public_exported_symbols` | Pass |

## Boundary Checks

| Check | Result |
| --- | --- |
| `git diff -- include/ruckig_c/ruckig.h docs/abi/public-symbols.txt docs/abi/public-symbol-exceptions.txt docs/abi/exceptions.md .github/workflows/ci.yml` | Clean |
| `git diff -- original/ruckig-main docs/assets/visualization` | Clean |
| `git diff --check` | Pass; Git reported expected CRLF normalization warnings only |

## Remote CI

Ordinary remote push CI is observed after pushing this checklist commit to
`main`. The run URL and conclusion should be recorded in the delivery summary.
