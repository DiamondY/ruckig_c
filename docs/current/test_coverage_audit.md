# Test Coverage Audit

This audit records the current `ruckig_c` test coverage evidence against the
frozen Ruckig Community `0.17.3` source under `original/ruckig-main`. It is an
engineering evidence record, not a formal proof of behavioral equivalence.

Coverage is tracked in three different senses:

- Code coverage: local LLVM line/function/branch coverage over the C
  implementation.
- Behavior coverage: mapping original Community tests and examples to current
  `ruckig_c` tests, examples, wrappers, and oracle gates.
- Oracle coverage: frozen C++ differential comparisons against
  `original/ruckig-main`.

## 0.14.0 Design Baseline

Current `main` is `0.14.0-design - Unreleased` after the published `v0.13.0`
stable release. The accepted post-`v0.12.0` waypoint true-resume evidence
slices are `0.13.0-alpha.1` stress coverage and `0.13.0-alpha.2` private
engine rewrite quality-baseline hardening. `v0.13.0` keeps the public C ABI
unchanged and records stable release-candidate coverage under the `0.13.0`
label.

## v0.13.0 Release-Candidate Coverage

`v0.13.0` release-candidate local gates rerun the readiness gate after the
version bump and ABI artifact path update.

Release-candidate gate evidence:

| Area | Evidence |
| --- | --- |
| Focused waypoint gates | `ruckig_c_waypoint_optimizer`, per-section constraints, waypoint quality, resume stress, resume quality audit, allocation audit, platform clock custom, and solver branch coverage all passed. |
| Routine gates | Default and shared CTest each passed 48/48; duration-enabled CTest passed 48/48. |
| Oracle gates | Fixed oracle 82 plus waypoint section oracle 4 passed; random 100k seeds 1, 2, 41 and per-DoF 100k seed 1 passed; local 1M release-random seed 1 passed. |
| Performance and ABI | No-waypoint benchmark ratio `1.20798` stayed below the `1.5` threshold; waypoint benchmark was recorded as C-only trend; public exported-symbol diff stayed clean at 172 public symbols and 0 unapproved exports. |
| Wrappers and visualization | Visualization verifier and strict regeneration passed with the shared DLL; Python prototype passed 21 tests with the shared DLL; Rust wrapper tests passed 13/13 and examples built. |
| Boundary | Public header diff is limited to version macros/string; workflow diff is limited to ABI artifact path `0.13.0`; ABI docs, `original/ruckig-main`, and visualization assets remain unchanged. |

Local `0.13.0` release-candidate coverage summary after filtering out
generated, test, example, and original-reference code:

| Metric | Covered | Total | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 6566 | 7432 | 88.35% |
| Functions | 386 | 418 | 92.34% |
| Lines | 6792 | 7702 | 88.18% |
| Branches | 3013 | 4299 | 70.09% |

Artifacts are under `out/coverage/0.13.0/`, and the command log is recorded in
`docs/release/checklists/0.13.0.md`.

## 0.13.0-readiness Local Stable-Review Audit

`0.13.0-readiness` reruns the full local release-readiness gate set for the
post-`v0.12.0` waypoint true-resume line. It does not add public C ABI and
does not expand soft interruption beyond waypoint `ruckig_update`.

Readiness gate evidence:

| Area | Evidence |
| --- | --- |
| Alpha remote evidence | `0.13.0-alpha.1` ordinary push CI succeeded on `9d322ad`; `0.13.0-alpha.2` ordinary push CI succeeded on `6354c41`, run `27330887817`. |
| Focused waypoint gates | `ruckig_c_waypoint_optimizer`, per-section constraints, waypoint quality, resume stress, resume quality audit, allocation audit, platform clock custom, and solver branch coverage all passed. |
| Routine gates | Default, shared, and duration-enabled CTest each passed 48/48. |
| Oracle gates | Fixed oracle 82 plus waypoint section oracle 4 passed; random 100k seeds 1, 2, 41 and per-DoF 100k seed 1 passed; local 1M release-random seed 1 passed. |
| Performance and ABI | No-waypoint benchmark ratio `1.18379` stayed below the `1.5` threshold; waypoint benchmark was recorded as C-only trend; public exported-symbol diff stayed clean at 172 public symbols and 0 unapproved exports. |
| Wrappers and visualization | Visualization verifier and strict regeneration passed with the shared DLL; Python prototype passed 21 tests with the shared DLL; Rust wrapper tests passed 13/13 and examples built. |
| Boundary | Public header, ABI docs, workflow, `CMakeLists.txt`, package-manager paths, `original/ruckig-main`, and visualization assets remain outside the readiness documentation diff. |

Local `0.13.0-readiness` coverage summary after filtering out generated,
test, example, and original-reference code:

| Metric | Covered | Total | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 6566 | 7432 | 88.35% |
| Functions | 386 | 418 | 92.34% |
| Lines | 6792 | 7702 | 88.18% |
| Branches | 3013 | 4299 | 70.09% |

Artifacts are under `out/coverage/0.13.0-readiness/`, and the command log is
recorded in `docs/release/checklists/0.13.0-readiness.md`.

## 0.13.0-alpha.2 Waypoint True-Resume Engine Rewrite And Quality Baseline

`0.13.0-alpha.2` rewrites the private waypoint optimizer/resume state into a
single internal engine structure and adds deterministic complete-solve quality
baselines for the same public waypoint `ruckig_update` soft-interruption
true-resume behavior. It does not add public C ABI and does not expand soft
interruption beyond waypoint `ruckig_update`.

Added C coverage:

| Scenario | Evidence |
| --- | --- |
| Private engine state | Moves optimizer scratch buffers, resume cursors, branch queue, best candidate state, and waypoint diagnostics into the private `waypoint_engine` state under `ruckig_t`. |
| Publish transaction | Background resume writes a scratch trajectory first and only copies a complete valid improvement into the output trajectory. |
| Deterministic baseline | Adds `--waypoint-resume-quality-audit` and `ruckig_c_waypoint_resume_quality_audit` with 128 deterministic waypoint cases captured against commit `9d322ad`. |
| Complete-solve quality | Asserts each successful complete waypoint solve remains at or below the checked-in baseline duration within `1e-9`. |
| Resume quality | Records background publish, interrupted-without-publish, completion, and fresh full-solve reference counts across corpus online loops. |
| Allocation/API boundary | Reuses existing preallocated waypoint engine buffers and keeps no-waypoint update, tracking, public `ruckig_calculate`, and public ABI behavior outside this slice. |

Local alpha.2 quality audit evidence:

| Metric | Result |
| --- | --- |
| Corpus cases | 128 |
| Successful complete solves | 128 |
| Average duration ratio vs baseline | 1.0 |
| Max complete-solve regression | 0 |
| Background publishes | 113 |
| Interrupted-without-publish cycles | 559 |
| Resume completions | 32 |
| Fresh full-solve references | 672 |

Local verification for this slice is recorded in
`docs/release/checklists/0.13.0-alpha.2.md`.

## 0.13.0-alpha.1 Waypoint True-Resume Stress And Quality Audit

`0.13.0-alpha.1` is a local post-`v0.12.0` evidence slice for the existing
waypoint `ruckig_update` soft-interruption true-resume behavior. It does not
add public C ABI and does not expand soft interruption beyond waypoint
`ruckig_update`.

The slice was later pushed to `ruckig_c/main`; ordinary push CI for commit
`9d322ad` succeeded. No `v0.13.0*` tag, GitHub Release, version bump, or
manual `release-random` workflow was created for alpha.1.

Added C coverage:

| Scenario | Evidence |
| --- | --- |
| Focused selector | Adds `--waypoint-resume-stress` and `ruckig_c_waypoint_resume_stress` so true-resume stress can run independently of the broader waypoint selectors. |
| Multi-DoF/multi-waypoint stress | Covers a 4-DoF, three-waypoint online resume case with four constrained sections. |
| Per-section constraints | Published and sampled trajectories are checked against per-section velocity, acceleration, jerk, position, and minimum-duration constraints. |
| Budget matrix | Exercises zero budget, a tiny positive budget, zero-budget continuation, large-budget completion, runtime budget changes, and interrupt clear. |
| Background interrupted without publish | Uses the private test hook to force a stable no-publish background cycle that preserves the incumbent trajectory while reporting interruption. |
| Background publish quality | Checks that any background publish resets time through normal new-calculation semantics and improves over incumbent remaining duration. |
| Fresh full-solve reference | Solves the current remaining input completely as a local quality reference during active resume cycles without requiring equality against the resumed result. |
| Allocation guard | Runs initial interrupted update, interrupted-without-publish background resume, and background completion/publish paths under the allocation guard. |

Local verification for this slice is recorded in
`docs/release/checklists/0.13.0-alpha.1.md`.

## v0.12.0 Waypoint Soft Interruption True-Resume Release

`v0.12.0` stabilizes the waypoint soft-interruption true-resume and unified
waypoint optimizer evidence from `0.12.0-alpha.1`, `0.12.0-alpha.2`, and
`0.12.0-readiness`. It does not add public C ABI and does not expand soft
interruption beyond waypoint `ruckig_update`.

Stable release coverage interpretation:

| Area | Evidence |
| --- | --- |
| Initial interruption | Covered by zero-budget waypoint update cases that publish a complete feasible baseline or return execution-time calculation error without publishing invalid trajectories. |
| True resume | Covered by repeated online `pass_to_input` cycles that continue private optimizer cursors and publish only complete feasible improvements. |
| Unified engine | Covered by public waypoint `ruckig_calculate` and no-budget waypoint `ruckig_update` running the same step-driven engine to completion without active resume state. |
| Constraints | Covered by published resumed trajectories satisfying sampled per-section velocity, acceleration, jerk, position, and minimum-duration constraints. |
| Invalidation | Covered by target, waypoint, count, limits, per-section, enabled DoF, synchronization, duration discretization, interrupt clear, non-`pass_to_input`, and reset invalidation cases. |
| Isolation | Covered by no-waypoint update, tracking, and public `ruckig_calculate` behavior remaining outside soft-interruption resume semantics. |
| Allocation | Covered by allocation-guard tests for online interrupted update and background completion paths. |

Release evidence is recorded in `docs/release/checklists/0.12.0.md`,
including local release-candidate gates, coverage label `0.12.0`, ordinary CI,
manual release-random workflows, tag CI, and release publication evidence.

Local `0.12.0` release-candidate coverage summary after filtering out
`original/`, `test/`, `examples/`, `bindings/`, and `out/`:

| Metric | Covered | Total | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 6535 | 7417 | 88.11% |
| Functions | 385 | 417 | 92.33% |
| Lines | 6752 | 7700 | 87.69% |
| Branches | 2986 | 4287 | 69.65% |

Artifacts are written under `out/coverage/0.12.0/`; generated coverage files
remain untracked release evidence.

## 0.12.0-alpha.2 True-Resume Unified Engine Hardening

`0.12.0-alpha.2` hardens the waypoint soft-interruption true-resume line by
sharing one private step-driven waypoint optimizer engine between complete
waypoint solves and interruptible online resume. It does not add public C ABI
and does not expand soft interruption beyond waypoint `ruckig_update`.

Added C coverage:

| Scenario | Evidence |
| --- | --- |
| Unified complete solve | Public waypoint `ruckig_calculate` runs the step-driven engine to completion, ignores the interrupt field, matches existing fixed waypoint regression expectations, and leaves no active resume state. |
| No-budget waypoint update | Waypoint `ruckig_update` without an interrupt budget completes through the same engine, reports no interruption, and leaves no active resume state. |
| Multi-DoF/multi-waypoint resume | A 3-DoF, two-waypoint online case resumes across `pass_to_input`, can finish background optimization after changing only the interrupt duration, and publishes only an improved complete trajectory. |
| Per-section constraints during resume | Published trajectories in the multi-waypoint resume case satisfy sampled per-section velocity, acceleration, jerk, position, and minimum-duration constraints. |
| Fresh full-solve quality comparison | The current remaining input is also solved with a complete waypoint calculation; the resumed publish is checked against incumbent remaining duration while the fresh solve remains feasible. |
| Long online loop | Repeated zero-budget online cycles keep old trajectory sampling valid when no publish occurs and reset time through normal new-calculation semantics when publishing. |
| Invalidation matrix | Target, intermediate waypoints, waypoint count, limits, per-section constraints, enabled DoFs, synchronization, invalid control mode, invalid duration discretization, and interrupt clear all invalidate or clear stale resume state. |
| Isolation | Public waypoint `ruckig_calculate` clears active resume state; no-waypoint `ruckig_update` with the interrupt field set does not use resume state. |
| Allocation | Background completion after an interrupt-duration change runs under the allocation guard without runtime allocation. |

## 0.12.0-alpha.1 Waypoint Soft Interruption True Resume

`0.12.0-alpha.1` extends the waypoint `ruckig_update`
soft-interruption evidence from V1 one-shot interruption to private true
resume. It does not add public C ABI and does not affect public
`ruckig_calculate`, no-waypoint target solving, or tracking.

Added C coverage:

| Scenario | Evidence |
| --- | --- |
| Initial zero-budget waypoint update | Publishes the first complete feasible baseline trajectory, marks `new_calculation=true`, sets `was_calculation_interrupted=true`, and leaves private resume active. |
| Background publish | After `ruckig_output_pass_to_input`, the next online cycle continues the optimizer, publishes a better complete waypoint trajectory, resets output time through normal new-calculation semantics, and remains allocation-free. |
| Background interrupted without publish | A budget-interrupted background cycle with no publishable candidate preserves the old trajectory, returns `RUCKIG_WORKING`, keeps `new_calculation=false`, and sets `was_calculation_interrupted=true`. |
| Interrupt clear | Clearing the interrupt field drops private resume state and later sampling reports `was_calculation_interrupted=false`. |
| Non-normal current state | A current-state change that is not the normal `pass_to_input` progression does not reuse private resume state and starts from the baseline candidate again. |
| Reset | `ruckig_reset` clears private resume state. |
| No feasible initial candidate | Initial budget expiry before any feasible complete candidate still returns `RUCKIG_ERROR_EXECUTION_TIME_CALCULATION`, does not publish an invalid trajectory, and does not leave resume active. |

Local verification recorded for this slice:

| Command | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_waypoint_optimizer\|ruckig_c_per_section_constraints\|ruckig_c_waypoint_quality\|ruckig_c_allocation_audit\|ruckig_c_tests"` | 5/5 passed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure` | 46/46 passed |
| `ctest --test-dir out\build\windows-clang-ninja-shared --output-on-failure` | 46/46 passed |
| `ctest --test-dir out\build\windows-clang-ninja-duration --output-on-failure` | 46/46 passed with `RUCKIG_C_ENABLE_CALCULATION_DURATION=ON` |
| `cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_compare_public_exported_symbols` | Public symbol count remains `172`; unapproved exported symbol count `0` |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe` | Fixed oracle passed: 82 comparisons; waypoint section oracle passed: 4 comparisons |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --random 10000 --seed 1` | Passed |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --random-per-dof 10000 --seed 1` | Passed |
| `out\build\windows-clang-ninja-performance\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1` | Average ratio `1.02735`, under release threshold `1.5` |
| `out\build\windows-clang-ninja-performance\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1 --waypoints` | Waypoint alpha benchmark average `3.29881e+06 ns`, p99 `1.21819e+07 ns` |
| POSIX and custom platform clock `zig cc` probes for `src\ruckig_c\waypoint.c` | Passed |

## v0.11.0 Soft Interruption V1 Evidence

`v0.11.0` stabilizes local waypoint `ruckig_update` soft interruption through
the existing `interrupt_calculation_duration` input field and
`was_calculation_interrupted` output state. This does not add public symbols and
does not affect public `ruckig_calculate`, no-waypoint target solving, or
tracking.

Added C coverage:

| Scenario | Evidence |
| --- | --- |
| Cleared waypoint interrupt budget | Waypoint `ruckig_update` completes normally, reports `was_calculation_interrupted=false`, and evaluates more than the first candidate. |
| Zero budget with feasible candidate | Waypoint `ruckig_update` stops after the first complete feasible candidate, returns `RUCKIG_WORKING`, marks a new calculation, writes a sampleable trajectory, and sets `was_calculation_interrupted=true`. |
| Zero budget with no feasible candidate | Waypoint `ruckig_update` returns `RUCKIG_ERROR_EXECUTION_TIME_CALCULATION`, does not mark a new calculation, and sets `was_calculation_interrupted=true`. |
| Continued online sampling | The next no-recalculation update after `pass_to_input` clears `was_calculation_interrupted`. |
| No-waypoint update | Setting the field does not interrupt no-waypoint `ruckig_update`. |
| Public waypoint calculate | Setting the field does not interrupt public `ruckig_calculate`. |
| Allocation | The zero-budget feasible fallback path is covered under the runtime no-allocation guard. |
| Platform clock override | Internal custom provider/header injection compiles without adding public ABI. |

Local verification:

| Command | Result |
| --- | --- |
| `cmake --build out\build\windows-clang-ninja --config Debug` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_platform_clock_custom\|ruckig_c_waypoint_optimizer\|ruckig_c_per_section_constraints\|ruckig_c_waypoint_quality\|ruckig_c_allocation_audit"` | 5/5 passed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure` | 46/46 passed |
| `ctest --test-dir out\build\windows-clang-ninja-duration --output-on-failure` | 46/46 passed with `RUCKIG_C_ENABLE_CALCULATION_DURATION=ON` |
| `cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_compare_public_exported_symbols` | Public exported symbols match the approved allowlist |
| `zig cc -target x86_64-linux-gnu -std=c99 -Wall -Wextra -Wpedantic -D_POSIX_C_SOURCE=200809L -Iinclude -Isrc -c src\ruckig_c\waypoint.c` | POSIX monotonic timing branch compiled for a Linux target |
| `zig cc -target x86_64-linux-gnu -std=c99 -Wall -Wextra -Wpedantic -DRUCKIG_C_PLATFORM_CLOCK_HEADER=<platform_clock_custom_provider.h> -DRUCKIG_C_CUSTOM_MONOTONIC_TIME_US=ruckig_test_platform_clock_us -Iinclude -Isrc -Itest\c -c src\ruckig_c\waypoint.c` | Custom platform clock provider compiled through the waypoint soft-interruption path |

## 0.7.0-alpha.4 Targeted Solver Branch Coverage

`0.7.0-alpha.4` follows the `0.7.0-alpha.3` audit by adding targeted evidence
for the five lowest implementation files from that run. It is still
local-only coverage evidence, not a CI coverage job and not a hard release
gate.

Local coverage is generated by:

```powershell
.\tools\coverage\run_coverage.ps1 -CoverageLabel 0.7.0-alpha.4
```

The runner writes raw artifacts under:

```text
out/coverage/0.7.0-alpha.4/
```

Targeted additions:

| Target | Added evidence | Evidence type |
| --- | --- | --- |
| `position_second_step1.c` | Direct branch probes for zero-velocity-limit setup plus fixed oracle cases with nonzero target velocity and stretched timing. | Internal branch coverage plus frozen oracle behavior |
| `position_second_step2.c` | Fixed oracle cases for nonzero target velocity, explicit minimum duration stretching, and 2D time synchronization. | Frozen oracle behavior |
| `velocity_third_step1.c` | Direct branch probes for zero-jerk setup plus fixed oracle cases with nonzero target acceleration and negative-direction stretching. | Internal branch coverage plus frozen oracle behavior |
| `velocity_third_step2.c` | Direct timing probes for invalid and fallback timing paths plus fixed oracle cases for nonzero acceleration and 2D time synchronization. | Internal branch coverage plus frozen oracle behavior |
| `block.c` | Internal white-box candidate-set tests for 1/2/3/4/5-profile cases, tie-breaks, duplicate removal, blocked intervals, and profile selection. | Internal branch coverage |

Broad routine corpus:

| Corpus item | Result |
| --- | --- |
| CTest excluding release/development random and routine tracking random seeds | 43/43 passed |
| Fixed oracle comparisons | 82 passed |
| Waypoint section oracle comparisons | 4 passed |
| Random oracle supplement | 10000 seed 1 passed |
| Random per-DoF oracle supplement | 10000 seed 1 passed |
| Tracking random supplement | 10000 seed 1 passed, optimized 117, fallback 9883, candidates 136602 |

Implementation coverage summary after filtering out `original/`, `test/`,
`examples/`, `bindings/`, and `out/`:

| Metric | Covered | Total | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 6017 | 6881 | 87.44% |
| Functions | 361 | 393 | 91.86% |
| Lines | 6207 | 7077 | 87.71% |
| Branches | 2701 | 3900 | 69.26% |

Target-file line coverage delta versus `0.7.0-alpha.3`:

| File | Alpha.3 | Alpha.4 | Delta |
| --- | ---: | ---: | ---: |
| `position_second_step2.c` | 39.76% | 83.13% | +43.37 pp |
| `velocity_third_step2.c` | 39.76% | 80.72% | +40.96 pp |
| `block.c` | 57.85% | 95.87% | +38.02 pp |
| `velocity_third_step1.c` | 66.10% | 90.68% | +24.58 pp |
| `position_second_step1.c` | 71.90% | 89.26% | +17.36 pp |

Remaining uncovered lines are mostly rare invalid-input guards, infeasible
candidate alternatives, and branch combinations that are still protected by
random oracle comparison. The internal white-box probes are coverage hardening
only; they are not counted as original behavior parity without the frozen C++
oracle cases.

## 0.7.0-alpha.3 Local Coverage Summary

Local coverage is generated by:

```powershell
.\tools\coverage\run_coverage.ps1 -CoverageLabel 0.7.0-alpha.3
```

The runner configures and builds `windows-clang-ninja-coverage`, runs the broad
routine corpus, merges LLVM profiles, and writes raw artifacts under:

```text
out/coverage/0.7.0-alpha.3/
```

The generated HTML, `.profraw`, and `.profdata` files are intentionally not
committed. Only this summary and the release checklist evidence are tracked.
Coverage numbers are evidence-only and are not a hard gate.

Local toolchain:

| Tool | Version |
| --- | --- |
| C compiler | LLVM clang 21.1.8 |
| C++ compiler | LLVM clang++ 21.1.8 |
| Coverage tools | llvm-cov / llvm-profdata 21.1.8 |
| Generator | Ninja |
| Build type | Debug |

Broad routine corpus:

| Corpus item | Result |
| --- | --- |
| CTest excluding release/development random and routine tracking random seeds | 42/42 passed |
| Fixed oracle comparisons | 76 passed |
| Waypoint section oracle comparisons | 4 passed |
| Random oracle supplement | 10000 seed 1 passed |
| Random per-DoF oracle supplement | 10000 seed 1 passed |
| Tracking random supplement | 10000 seed 1 passed, optimized 117, fallback 9883, candidates 136602 |

Implementation coverage summary after filtering out `original/`, `test/`,
`examples/`, `bindings/`, and `out/`:

| Metric | Covered | Total | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 5841 | 6881 | 84.89% |
| Functions | 355 | 393 | 90.33% |
| Lines | 6039 | 7077 | 85.33% |
| Branches | 2614 | 3900 | 67.03% |

Lowest line-coverage implementation files in this run:

| File | Line coverage | Interpretation |
| --- | ---: | --- |
| `position_second_step2.c` | 39.76% | Synchronized second-order position timing alternatives are covered by oracle tests, but not all branch families are hit in the broad routine corpus. |
| `velocity_third_step2.c` | 39.76% | Third-order velocity synchronization alternatives need targeted branch-family cases if coverage is promoted later. |
| `block.c` | 57.85% | Block interval tie-break and multi-profile ordering paths are partially covered; they remain protected mainly by oracle comparisons. |
| `velocity_third_step1.c` | 66.10% | Some candidate families are long-tail cases in the random corpus. |
| `position_second_step1.c` | 71.90% | Second-order position setup has uncovered rare branch combinations. |

## Original Test Mapping

Original source: `original/ruckig-main/test/test_target.cpp`.

| Original test case | Current `ruckig_c` coverage | Status |
| --- | --- | --- |
| `trajectory` | C API trajectory tests, fixed oracle trajectory comparison, sampling, duration, independent minimum durations, first-time-at-position, and online update comparisons. | Strong |
| `input-validation` | C validation tests, invalid-input diagnostics, zero-limit tests, finite/infinite limit tests, and oracle result comparison. | Strong |
| `enabled` | Disabled DoF C tests, random per-DoF oracle cases, tracking disabled-DoF corpus, and examples. | Strong |
| `phase-synchronization` | Fixed C tests, fixed oracle cases, random oracle synchronization modes, and phase/none/time-if-necessary per-DoF coverage. | Strong |
| `dynamic-dofs` | All public C handles are dynamic DoF handles; C examples and oracle tests cover 1-3 DoF, waypoint tests cover up to 8 DoF. | Strong for C ABI |
| `zero-limits` | C zero-limit diagnostics and oracle result paths cover accepted and rejected zero-limit shapes. | Strong |
| `custom-vector-type` | Not directly applicable to C ABI; C uses flat arrays and dynamic handles. Header compile and wrapper smoke cover consumer ergonomics instead. | Out of C ABI scope |
| `random-discrete-3` | Random oracle generator covers continuous/discrete duration with 1-3 DoF inputs; release random reaches 1M seed 1. | Strong |
| `velocity-random-3` | Random oracle generator covers velocity control, second/third-order velocity paths, and online sampling. | Strong |
| `velocity-random-discrete-3` | Random oracle generator covers velocity control with discrete duration; fixed C tests cover discrete behavior. | Strong |
| `velocity-second-random-3` | Random oracle and fixed C tests cover no-jerk velocity cases. | Strong |
| `random-3-high` | Large/high-limit behavior is covered by fixed oracle regressions and random cases, but not with the exact original high-limit distribution as a named bucket. | Partial |
| `random-direction-3` | Directional min velocity/min acceleration are covered by fixed C tests, random oracle cases, and per-DoF random coverage. | Strong |
| `position-random-3` | Random oracle generator covers first-, second-, and third-order position cases with 1-3 DoF. | Strong |
| `position-second-random-3` | Random oracle and fixed C tests cover no-jerk second-order position paths. | Strong |

## Original Example Mapping

Original source: `original/ruckig-main/examples`.

| Original example | Current `ruckig_c` equivalent | Status |
| --- | --- | --- |
| `01_position` | `examples/c/01_position.c`; Rust `position.rs`; Python prototype smoke covers position solve. | Covered |
| `02_position_offline` | `examples/c/02_position_offline.c`; Rust `offline.rs`. | Covered |
| `03_waypoints` | `examples/c/09_waypoints_offline.c`; waypoint C tests. | Covered locally |
| `04_waypoints_online` | `examples/c/11_waypoints_online.c`. | Covered locally |
| `05_velocity` | `examples/c/05_velocity.c`; Rust `velocity.rs`. | Covered |
| `06_stop` | `examples/c/06_stop.c`. | Covered |
| `07_minimum_duration` | `examples/c/07_minimum_duration.c`. | Covered |
| `08_per_section_minimum_duration` | `examples/c/10_per_section_minimum_duration.c`; Rust `per_section_minimum_duration.rs`. | Covered |
| `09_dynamic_dofs` | C ABI is dynamic-DoF by construction; no separate template-style C example is needed. | Covered by API shape |
| `10_dynamic_dofs_waypoints` | `examples/c/14_dynamic_dofs_waypoints.c`. | Covered |
| `11_eigen_vector_type` | Not applicable to flat C ABI. | Out of C ABI scope |
| `12_custom_vector_type` | Not applicable to flat C ABI. | Out of C ABI scope |
| `13_custom_vector_type_dynamic_dofs` | Not applicable to flat C ABI. | Out of C ABI scope |
| `14_tracking` | `examples/c/15_tracking_online_fast_ramp.c`, `16_tracking_online_constant_acceleration.c`, `18_tracking_online_optimized_lookahead.c`, and `19_tracking_online_optimized_sinus.c`. | Local tracking evidence |
| `15_tracking_offline` | `examples/c/17_tracking_offline_sequence.c` and `20_tracking_offline_optimized_sequence.c`. | Local tracking evidence |
| `16_speed` | `ruckig_c_performance_benchmark` and release performance evidence. | Covered |

Original example trajectory PDFs and `examples/plotter.py` are visualization
assets rather than solver behavior tests. `v0.10.0` stabilizes a
project-owned Visualization v2 PNG gallery generated from the public C ABI and
Python prototype. The gallery covers original example mappings `01-10` and
`14-16`, plus tracking diagnostics, waypoint diagnostics, trajectory anatomy,
and summary plots. It remains local evidence and is not a default CI or release
gate.

## Interpretation

The strongest coverage area remains the no-waypoint target solver: fixed oracle
cases, routine 100k random seeds, per-DoF random coverage, and release/manual
1M random evidence directly compare C behavior to the frozen C++ oracle.

Coverage is weaker where the C runtime intentionally differs from, or
deliberately scopes out, original product surfaces:

- C++ template/static DoF, Eigen, and custom vector ergonomics do not map to
  the C ABI and are not counted against current scoped parity.
- Waypoint global optimality and proprietary Pro output equivalence are not
  claimed; local waypoint behavior is evaluated by interface/effect evidence.
- Tracking has local Fast and bounded Optimized evidence, but no frozen local
  source-level tracking oracle.
- Python and Rust remain prototype/alpha evidence. Package publication is
  frozen until a separate demand decision accepts it.

The coverage audit supports the current project-scoped original-surface parity
estimate; it does not create a claim of cloud runtime support, proprietary Pro
equivalence, or package ecosystem completion.
