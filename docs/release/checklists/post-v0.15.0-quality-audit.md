# Post-v0.15.0 Quality Audit Checklist

Status: local implementation and evidence complete.

This checklist records the `post-v0.15.0-quality-audit` slice after the stable
`v0.15.0` release. The slice improves code quality and test quality without
starting `0.16.0-design`, changing version metadata, expanding the 184-symbol
public C ABI, creating a tag, publishing wrappers, or changing package-manager
scope.

## Scope

- [x] Added `docs/current/code_quality_audit.md` as the quality baseline and
  risk map.
- [x] Recorded current implementation coverage baseline: Regions `88.34%`,
  Functions `92.90%`, Lines `87.78%`, Branches `69.89%`.
- [x] Recorded current largest implementation hotspots:
  `tracking.c`, `position_third_step2.c`, `ruckig.c`, `waypoint.c`, and
  `input.c`.
- [x] Identified highest-risk maintenance areas as private state machines,
  long-tail candidate branches, error paths, and random failure reproduction.
- [x] Added focused deterministic property selector
  `ruckig_c_property_invariants`.
- [x] Added default-off `RUCKIG_C_ENABLE_INTERNAL_ASSERTS`.
- [x] Added `windows-clang-ninja-internal-asserts` local preset for assertion
  builds.
- [x] Improved tracking random stress failure-context output.
- [x] Improved tracking random audit representative output with seed/sample
  context.
- [x] Improved oracle random repro output with kind/seed/sample and enabled
  vector context.
- [x] Kept large random and coverage gates as local/manual evidence, not a new
  default push-CI burden.

## Public API / ABI Boundary

- [x] No exported C function is added.
- [x] No public function signature is changed.
- [x] No enum numeric value or result-code numeric value is changed.
- [x] No public struct layout or public diagnostics layout is changed.
- [x] No ABI allowlist or public-symbol exception file is changed.
- [x] No version metadata, tag, GitHub Release, package-manager recipe,
  workflow, upstream baseline, or visualization asset is changed.
- [x] Python and Rust wrappers remain prototype-only.

## Test Additions

| Area | Evidence |
| --- | --- |
| No-waypoint properties | Boundary sampling at `t=0` and `t=duration`, finite legal-range samples, duration consistency between calculate and update, independent disabled-DoF duration, disabled-DoF kinematics, and `RUCKIG_RESULT_IS_OK` semantics. |
| Tracking continuation properties | Fast delta-time contract reuse plus Optimized Balanced 2-DoF interrupted/resume equivalence with disabled DoF and allocation guard. |
| Waypoint resume properties | Allocation-guarded interrupted waypoint update, fresh complete reference from progressed input, full-budget resume completion, section/sample invariants, and active-state completion. |
| Internal assertions | Private continuation, diagnostics, optimized step, and waypoint resume engine invariants are compiled only when `RUCKIG_C_ENABLE_INTERNAL_ASSERTS=ON`. |
| Input cleanup | Private DoF-array and DoF-index helpers consolidate repeated validation without changing public return codes. |

## Local Gates

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Pass |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_property_invariants\|ruckig_c_tracking_sequence_continuation_api\|ruckig_c_tracking_sequence_fast_continuation\|ruckig_c_tracking_sequence_optimized_continuation\|ruckig_c_waypoint_resume_stress\|ruckig_c_allocation_audit"` | Pass, 6/6 |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure` | Pass, 62/62 |
| `cmake --build --preset windows-clang-ninja-internal-asserts` | Pass |
| `ctest --test-dir out\build\windows-clang-ninja-internal-asserts --output-on-failure -R "ruckig_c_property_invariants\|ruckig_c_tracking_sequence_fast_continuation\|ruckig_c_tracking_sequence_optimized_continuation\|ruckig_c_waypoint_resume_stress"` | Pass, 4/4 |
| `ctest --test-dir out\build\windows-clang-ninja-internal-asserts --output-on-failure` | Pass, 62/62 |
| `cmake --build --preset windows-clang-ninja-shared` | Pass |
| `ctest --test-dir out\build\windows-clang-ninja-shared --output-on-failure` | Pass, 62/62 |
| `cmake --build --preset windows-clang-ninja-oracle` | Pass |
| `ctest --test-dir out\build\windows-clang-ninja-oracle --output-on-failure -R ruckig_c_oracle_random_development` | Pass, 1/1 |
| `cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_verify_public_symbols` | Pass |
| `cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_compare_public_exported_symbols` | Pass, public exported symbols match the approved allowlist |
| `out\build\windows-clang-ninja\ruckig_c_tests.exe --tracking-random-audit 10000 --seed 1` | Pass, 10000 samples, optimized 9993, fallback 7 |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --random 10000 --seed 1` | Pass |
| `.\tools\coverage\run_coverage.ps1 -CoverageLabel post-v0.15.0-quality-audit` | Pass, 63/63 coverage CTest; summary at `out\coverage\post-v0.15.0-quality-audit\coverage-summary.txt` |
| `cargo test --manifest-path bindings\rust\Cargo.toml` | Pass, 16/16 library tests plus doc-tests |
| `cargo test --manifest-path bindings\rust\Cargo.toml --examples` | Pass, examples compile/test smoke |
| Python cffi prototype against shared DLL | Pass, 24/24 with absolute `RUCKIG_C_SHARED_LIBRARY` path |
| `git diff -- include/ruckig_c/ruckig.h docs/abi/public-symbols.txt docs/abi/public-symbol-exceptions.txt docs/abi/exceptions.md .github/workflows/ci.yml` | Pass, empty |
| `git diff -- original/ruckig-main` | Pass, empty |
| `git diff -- docs/assets/visualization` | Pass, empty |
| `git diff --check` | Pass |

## Coverage Result

The local coverage run for this slice produced:

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 8040 | 948 | 88.21% |
| Functions | 456 | 32 | 92.98% |
| Lines | 8609 | 1051 | 87.79% |
| Branches | 4683 | 1427 | 69.53% |

This did not meet the first-stage aspirational branch-coverage target of
`70.5%+`. The slice still improves risk coverage by adding deterministic
property invariants, assertion-enabled state-machine checks, and reproducible
random failure context. The next quality slice should keep branch coverage as
the main numerical target and add fixed cases for low-value uncovered
long-tail branches only when they map to real maintenance risk.

## Deferred

- Full automatic random-case shrinker.
- Solver skeleton abstraction in `ruckig.c`.
- Deep refactors in `position_*_step*.c`.
- New public diagnostics, runtime clock hooks, wrapper publication, package
  recipes, upstream baseline upgrades, or `0.16.0-design` transition.
