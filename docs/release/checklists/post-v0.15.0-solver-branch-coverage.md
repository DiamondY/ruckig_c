# Post-v0.15.0 Solver Branch Coverage Checklist

Status: local implementation and evidence complete.

This checklist records the `post-v0.15.0-solver-branch-coverage` quality slice
after `v0.15.0` and after the state-machine branch coverage slice. It adds
deterministic solver branch cases and conservatively extracts the repeated
`ruckig.c` calculate synchronization skeleton into private static callbacks.
It does not start `0.16.0-design`, change version metadata, expand the
184-symbol public C ABI, create a tag, publish wrappers, change workflow
behavior, edit ABI allowlists, update the upstream baseline, or touch
visualization assets.

## Scope

- [x] Kept the existing focused CLI entry `ruckig_c_tests --solver-branch-coverage`.
- [x] Kept the existing CTest selector `ruckig_c_solver_branch_coverage`.
- [x] Added deterministic direct tests for the internal, non-exported
  `ruckig_position_third_step1_get_profile` and
  `ruckig_position_third_step2_get_profile` entry points through the existing
  private header declaration.
- [x] Added fixed oracle cases for third-order no-waypoint position behavior
  around mixed per-DoF synchronization, Phase fallback, directional min limits,
  and multi-DoF synchronization dispatch.
- [x] Extracted the repeated `ruckig.c` calculate synchronization/finalization
  skeleton into private `static` callback plumbing.
- [x] Kept solver candidate order, tolerance policy, disabled-DoF behavior,
  block selection, result-code mapping, synchronization semantics, and public
  ABI unchanged.

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
| Third-order position step1 | Null argument rejection, zero-duration no-motion, jerk-limited rest-to-rest, negative-direction motion, zero-jerk single-step branch, zero-limit failure, and block interval publication. |
| Third-order position step2 | Null profile, negative and non-finite duration rejection, too-small duration rejection, zero-jerk rejection, valid stretched rest-to-rest synchronization, and reverse-direction synchronization. |
| Oracle fixed cases | Third-order per-DoF synchronization mix, Phase fallback on non-proportional state, and directional min velocity/acceleration limits in 2 DoF. |
| Calculate skeleton parity | The direct solver selector, full normal/shared CTest, fixed oracle, 100k random oracle seeds, per-DoF oracle, performance, ABI/export, and wrapper smoke gates all cover the private callback skeleton after refactor. |

## Local Gates

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Pass |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure` | Pass, 63/63 |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_solver_branch_coverage\|ruckig_c_property_invariants\|ruckig_c_state_machine_branch_coverage\|ruckig_c_allocation_audit"` | Pass, 4/4 |
| `cmake --build --preset windows-clang-ninja-oracle` | Pass |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe` | Pass, waypoint section oracle 4 and fixed oracle 85 |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --random 100000 --seed 1` | Pass |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --random 100000 --seed 2` | Pass |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --random 100000 --seed 41` | Pass |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --random-per-dof 100000 --seed 1` | Pass |
| `.\tools\coverage\run_coverage.ps1 -CoverageLabel post-v0.15.0-solver-branch-coverage` | Pass, 64/64 coverage CTest; summary at `out\coverage\post-v0.15.0-solver-branch-coverage\coverage-summary.txt` |
| `cmake --build --preset windows-clang-ninja-shared` | Pass |
| `ctest --test-dir out\build\windows-clang-ninja-shared --output-on-failure` | Pass, 63/63 |
| `cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_verify_public_symbols` | Pass |
| `cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_compare_public_exported_symbols` | Pass, public exported symbols match the approved allowlist |
| `cmake --build --preset windows-clang-ninja-performance` | Pass |
| `out\build\windows-clang-ninja-performance\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1 --enforce-threshold` | Pass, average ratio `1.06064` below threshold `1.5` |
| `out\build\windows-clang-ninja-performance\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1 --waypoints` | Pass, waypoint average `3.35999e+06 ns` |
| Python prototype against `out\build\windows-clang-ninja-shared\ruckig_c.dll` | Pass, 24/24 |
| `cargo test --manifest-path bindings\rust\Cargo.toml` | Pass, 16/16 plus doc tests |
| `cargo test --manifest-path bindings\rust\Cargo.toml --examples` | Pass |
| `git diff -- include/ruckig_c/ruckig.h docs/abi/public-symbols.txt docs/abi/public-symbol-exceptions.txt docs/abi/exceptions.md .github/workflows/ci.yml` | Pass, empty |
| `git diff -- original/ruckig-main docs/assets/visualization` | Pass, empty |
| `git diff --check` | Pass |

## Coverage Result

The local coverage run for this slice produced:

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 7909 | 839 | 89.39% |
| Functions | 471 | 30 | 93.63% |
| Lines | 8525 | 957 | 88.77% |
| Branches | 4579 | 1305 | 71.50% |

The branch-coverage target of `71.5%+` is reached. Compared with the previous
`post-v0.15.0-state-machine-branch-coverage` run, missed branches drop from
`1370` to `1305`, and branch coverage rises from `70.75%` to `71.50%`.
The total implementation branch count also drops from `4683` to `4579` because
the repeated `ruckig.c` calculate skeleton branches are now represented by one
private callback skeleton instead of five copied loops.

Relevant hotspot movement:

| File | Previous branch coverage | Current branch coverage |
| --- | ---: | ---: |
| `src/ruckig_c/ruckig.c` | 69.52% | 72.35% |
| `src/ruckig_c/position_third_step1.c` | 65.97% | 69.10% |
| `src/ruckig_c/position_third_step2.c` | 79.15% | 80.40% |

Remaining branch gaps are still concentrated in rare solver candidate
alternatives, infeasible analytical branches, defensive invalid-input guards,
platform-clock defensive paths, and oracle-protected long-tail behavior. Those
remain better handled by targeted oracle-derived regressions or a later local
shrinker than by broad default-CI random expansion.

## Deferred

- Solver analytical branch exhaustion in `position_*_step*.c`.
- Large-scale random shrinker or automatic fixture materialization.
- Public runtime clock hooks, wrapper publication, package recipes, upstream
  baseline upgrades, or `0.16.0-design` transition.
