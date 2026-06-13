# Post-v0.15.0 Solver-Adjacent Branch Coverage Checklist

Status: local implementation and evidence complete; ordinary remote push CI is
observed after the checklist commit is pushed.

This checklist records the `post-v0.15.0-solver-adjacent-branch-coverage`
quality slice after `v0.15.0` and after the solver skeleton refactor slice. It
adds deterministic solver-adjacent branch coverage for brake and lower-order
step paths, plus fixed public oracle cases for brake pre-trajectories. It does
not start `0.16.0-design`, change version metadata, expand the 184-symbol
public C ABI, create a tag, publish wrappers, change workflow behavior, edit
ABI allowlists, update the upstream baseline, or touch visualization assets.

## Scope

- [x] Kept the existing focused CLI entry `ruckig_c_tests --solver-branch-coverage`.
- [x] Kept the existing CTest selector `ruckig_c_solver_branch_coverage`.
- [x] Added deterministic direct tests for `brake.c`,
  `position_first_step1/2`, `position_second_step1/2`,
  `velocity_second_step1/2`, and additional `velocity_third_step1/2` paths.
- [x] Added fixed public oracle cases for brake pre-trajectory behavior in
  no-waypoint position and velocity solves.
- [x] Reached the branch coverage target of `72.5%+`.
- [x] Did not retain a production-code refactor. Tests reached the target, and
  no solver-adjacent helper extraction was clearly worth the extra churn.

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
| `brake.c` | Null/no-op paths, zero-limit no-op, acceleration high/low braking, velocity high/low braking, second-order velocity bound high/low braking, velocity-control acceleration high/low braking, and finalize null/no-segment/one-segment/two-segment branches. |
| First-order position | Step1 null and zero-limit rejection, forward and reverse direct profiles, step2 invalid duration rejection, valid timing, and velocity-limit rejection. |
| Second-order position | Step1 null rejection, no-motion, zero-velocity-limit rejection, valid direct profile, invalid and too-small step2 timing, valid stretched timing, and reverse-direction synchronization. |
| Second-order velocity | Step1 null and zero-acceleration rejection, zero/no-motion, positive and negative acceleration-limited profiles, invalid step2 timing, timing rejection, and valid positive/reverse synchronization. |
| Third-order velocity | Extra null rejection, zero-jerk single-step, reverse-direction direct profile, invalid step2 timing, valid stretched timing, and reverse-direction synchronization. |
| Oracle fixed cases | Public no-waypoint cases that force brake pre-trajectories for second-order position current-velocity violation, third-order position current velocity/acceleration violations, and third-order velocity current-acceleration violations. |

## Local Gates

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Pass |
| `cmake --build --preset windows-clang-ninja-shared` | Pass |
| `cmake --build --preset windows-clang-ninja-oracle` | Pass |
| `cmake --build --preset windows-clang-ninja-performance` | Pass |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure` | Pass, 63/63 |
| `ctest --test-dir out\build\windows-clang-ninja-shared --output-on-failure` | Pass, 63/63 |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_solver_branch_coverage\|ruckig_c_property_invariants\|ruckig_c_state_machine_branch_coverage\|ruckig_c_allocation_audit"` | Pass, 4/4 |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe` | Pass, waypoint section oracle 4 and fixed oracle 92 |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --random 100000 --seed 1` | Pass |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --random 100000 --seed 2` | Pass |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --random 100000 --seed 41` | Pass |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --random-per-dof 100000 --seed 1` | Pass |
| `.\tools\coverage\run_coverage.ps1 -CoverageLabel post-v0.15.0-solver-adjacent-branch-coverage` | Pass, 64/64 coverage CTest; summary at `out\coverage\post-v0.15.0-solver-adjacent-branch-coverage\coverage-summary.txt` |
| `out\build\windows-clang-ninja-performance\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1 --enforce-threshold` | Pass, average ratio `0.98175` below threshold `1.5` |
| `out\build\windows-clang-ninja-performance\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1 --waypoints` | Pass, waypoint average `3.31155e+06 ns` |
| `cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_verify_public_symbols` | Pass |
| `cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_compare_public_exported_symbols` | Pass |
| Python prototype against absolute shared DLL path | Pass, 24/24 |
| `cargo test --manifest-path bindings\rust\Cargo.toml` | Pass, 16/16 plus doc tests |
| `cargo test --manifest-path bindings\rust\Cargo.toml --examples` | Pass |

## Coverage Result

Baseline was commit `c934265 Add solver branch coverage and skeleton refactor`
with branch coverage `71.50%`.

The local coverage run for this slice produced:

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 7909 | 803 | 89.85% |
| Functions | 471 | 30 | 93.63% |
| Lines | 8525 | 919 | 89.22% |
| Branches | 4579 | 1239 | 72.94% |

The branch-coverage target of `72.5%+` is reached. Compared with the previous
solver branch coverage run, missed branches drop from `1305` to `1239`, and
branch coverage rises from `71.50%` to `72.94%`.

Relevant hotspot movement:

| File | Previous branch coverage | Current branch coverage |
| --- | ---: | ---: |
| `src/ruckig_c/brake.c` | 59.38% | 85.42% |
| `src/ruckig_c/position_first_step1.c` | 62.50% | 87.50% |
| `src/ruckig_c/position_first_step2.c` | 50.00% | 100.00% |
| `src/ruckig_c/position_second_step1.c` | 80.77% | 90.38% |
| `src/ruckig_c/position_second_step2.c` | 50.00% | 68.75% |
| `src/ruckig_c/velocity_second_step1.c` | 57.14% | 85.71% |
| `src/ruckig_c/velocity_second_step2.c` | 50.00% | 100.00% |
| `src/ruckig_c/velocity_third_step1.c` | 81.25% | 89.58% |
| `src/ruckig_c/velocity_third_step2.c` | 58.70% | 60.87% |

Remaining branch gaps are still concentrated in analytical long-tail solver
candidate alternatives, infeasible timing combinations, `tracking.c` and
`waypoint.c` state-machine branches outside this slice, and defensive platform
or output paths. Those are intentionally not pulled into this solver-adjacent
coverage slice.

## Remote CI

Ordinary remote push CI is observed after pushing the checklist commit to
`main`. The run URL and conclusion should be recorded in the delivery summary.

## Deferred

- Further analytical branch exhaustion in `position_third_step1/2.c` and
  `velocity_third_step2.c`.
- Automatic random failure shrinker or fixture materialization.
- Non-solver branch coverage in `input.c`, `output.c`, `tracking.c`, and
  `waypoint.c`.
- Public runtime clock hooks, wrapper publication, package recipes, upstream
  baseline upgrades, or `0.16.0-design` transition.
