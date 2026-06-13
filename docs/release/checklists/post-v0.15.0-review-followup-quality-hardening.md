# post-v0.15.0-review-followup-quality-hardening Checklist

## Scope

This is a post-`v0.15.0` quality hardening slice. It addresses the accepted
external review follow-up items without starting `0.16.0-design` and without
changing public release state.

Boundary:

- Public C ABI remains the stable `v0.15.0` 184-symbol baseline.
- Public header, version metadata, workflow files, ABI allowlists/exceptions,
  tags, GitHub Releases, wrapper publication status, `original/ruckig-main`,
  and visualization assets are out of scope.
- Python and Rust wrappers remain prototype-only smoke targets.

Baseline before the slice: clean `main` after
`09d4a4a Add random repro materialization tools`.

## Accepted Review Follow-Up

| Review item | Resolution |
| --- | --- |
| `allocate_input_vectors` partial-allocation cleanup depends on zero initialization | Added an explicit comment documenting the calloc-zeroed owner invariant. Also moved repeated private double-vector allocation helpers to a shared `static inline` helper in `alloc.h`. |
| Waypoint branch queue fixed capacity alleged overflow | The overflow claim was disproven by the existing `insert_branch` saturation logic, which bounds `branch_count` to `RUCKIG_WAYPOINT_BRANCH_QUEUE_CAPACITY`. Added a public interrupted/resume saturation regression that drives the production branch queue to capacity and checks `branch_count <= capacity` under normal and internal-assert builds. |
| `roots.c` exact `0.0 == A` check | Replaced with the direct epsilon guard `fabs(A) < DBL_EPSILON`, per accepted decision. Roots audit and oracle random gates passed, so no scale-aware fallback was needed. |
| `waypoint_planning_identity_equals` long `&&` chain | Split into scalar/flag, base-array, optional per-DoF, and per-section comparison helpers without changing exact-equality semantics or field coverage. |
| Duplicated interrupt context structs | Added private header-only `interrupt_context.h` and replaced no-waypoint, waypoint, and tracking private interrupt contexts. No exported symbol was added. |
| `tracking.c` monolith | Split by behavior boundary into `tracking.c` for lifecycle/config/diagnostics, `tracking_update.c` for online Fast/Optimized update and candidate scoring, and `tracking_sequence.c` for sequence and continuation behavior. Added private `tracking_internal.h`; no helper uses `RUCKIG_C_API`. |
| `profile_check` parameter explosion | Added private context structs and `_ctx` implementations. Old long-parameter names are private macro shims that expand to context compound literals; `profile.c` only implements `_ctx` entry points. Added explicit context smoke coverage. |
| `ruckig_update` readability | Extracted `publish_new_trajectory_to_output`, `calculate_or_resume_output_trajectory`, and `sample_output_at_next_time` while preserving publication, interruption, time, section, and calculation-duration behavior. |
| Internal const-correctness | Changed private `ruckig_calculate_target` to take `const ruckig_t*`. Public `ruckig_calculate` and `ruckig_update` signatures are unchanged. |

Deferred/not adopted:

- No broad C89-to-C99 variable-declaration churn.
- No public `ruckig_calculate` or `ruckig_update` signature change.
- No package-manager, release, tag, workflow, or wrapper publication change.
- No automatic random shrinker.

## Added Tests

| Selector or test | Evidence |
| --- | --- |
| `ruckig_c_roots_numeric_audit` / `--roots-numeric-audit` | Covers resolvent `A == 0`, tiny-scale resolvent/cubic/quartic residuals, repeated/near-zero roots, non-negative filtering, sort stability, and no-allocation behavior. |
| `ruckig_c_state_machine_branch_coverage` extension | Adds waypoint branch queue saturation through public interrupted/resume behavior and verifies the private queue remains capacity-bounded. |
| `test_profile_context_entrypoints` | Explicitly covers first-order, second-order position, second-order velocity, third-order position, and third-order velocity `_ctx` profile-check entry points. |

## Local Gates

Build gates:

| Command | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed |
| `cmake --build --preset windows-clang-ninja-shared` | Passed |
| `cmake --build --preset windows-clang-ninja-oracle` | Passed |
| `cmake --build --preset windows-clang-ninja-performance` | Passed |
| `cmake --build --preset windows-clang-ninja-internal-asserts` | Passed |

Focused CTest gates:

| Command | Result |
| --- | --- |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_roots_numeric_audit\|ruckig_c_state_machine_branch_coverage\|ruckig_c_property_invariants\|ruckig_c_allocation_audit"` | 4/4 passed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_interrupt_boundary_audit\|ruckig_c_no_waypoint_interrupt_audit\|ruckig_c_interrupt_post_release_quality\|ruckig_c_waypoint_resume_stress"` | 4/4 passed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_tracking_sequence_continuation_api\|ruckig_c_tracking_sequence_fast_continuation\|ruckig_c_tracking_sequence_optimized_continuation\|ruckig_c_tracking_quality_hardening\|ruckig_c_tracking_random_replay_smoke"` | 5/5 passed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_tracking_api\|ruckig_c_tracking_online\|ruckig_c_tracking_optimized\|ruckig_c_tracking_offline\|ruckig_c_tracking_interrupt_audit\|ruckig_c_tracking_no_allocation"` | 6/6 passed |
| `ctest --test-dir out\build\windows-clang-ninja-internal-asserts --output-on-failure -R "ruckig_c_roots_numeric_audit\|ruckig_c_state_machine_branch_coverage\|ruckig_c_waypoint_resume_stress\|ruckig_c_tracking_sequence_fast_continuation\|ruckig_c_tracking_sequence_optimized_continuation"` | 5/5 passed |

Full and oracle gates:

| Command | Result |
| --- | --- |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure` | 66/66 passed |
| `ctest --test-dir out\build\windows-clang-ninja-shared --output-on-failure` | 66/66 passed |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe` | Waypoint section oracle 4 passed; fixed oracle 92 passed |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --random 100000 --seed 1` | Passed |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --random 100000 --seed 2` | Passed |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --random 100000 --seed 41` | Passed |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --random-per-dof 100000 --seed 1` | Passed |

Performance, ABI, and wrapper gates:

| Command | Result |
| --- | --- |
| `out\build\windows-clang-ninja-performance\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1 --enforce-threshold` | Passed; average ratio `1.03447` under `1.5` threshold |
| `out\build\windows-clang-ninja-performance\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1 --waypoints` | Passed; C-only waypoint alpha corpus completed |
| `cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_verify_public_symbols` | Passed |
| `cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_compare_public_exported_symbols` | Passed; public exported symbols match allowlist |
| `cargo test --manifest-path bindings\rust\Cargo.toml` | 16/16 passed plus doc tests |
| `cargo test --manifest-path bindings\rust\Cargo.toml --examples` | Rust examples built and test harnesses passed |
| `$env:RUCKIG_C_SHARED_LIBRARY='E:\Yww\DownLoad\source\ruckig_c\out\build\windows-clang-ninja-shared\ruckig_c.dll'; python bindings\python_prototype\test_prototype.py` | 24/24 passed |

## Boundary Checks

Expected clean boundaries:

- `include/ruckig_c/ruckig.h` unchanged.
- ABI allowlists/exceptions unchanged.
- `.github/workflows/ci.yml` unchanged.
- `original/ruckig-main` unchanged.
- `docs/assets/visualization` unchanged.
- Version metadata unchanged.

Remote ordinary push CI is observed after the single evidence commit is pushed;
the final delivery response records the run URL and conclusion rather than
creating a second post-CI evidence commit.
