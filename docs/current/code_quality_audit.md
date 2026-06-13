# Code Quality Audit

This audit starts the `post-v0.15.0-quality-audit` slice after the stable
`v0.15.0` release. It is a quality and testability slice only: it does not
start `0.16.0-design`, change version metadata, expand the public C ABI, tag a
release, publish wrappers, or change package-manager scope.

## Baseline

Current `main` remains on `0.15.0` release evidence after publishing
`v0.15.0`. The stable public C ABI baseline is the 184-symbol continuation API
surface reviewed during `0.15.0-readiness` and stabilized in `v0.15.0`.

Local `0.15.0` implementation coverage, filtered to implementation sources and
excluding generated, test, example, binding, and output paths:

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 7990 | 932 | 88.34% |
| Functions | 451 | 32 | 92.90% |
| Lines | 8525 | 1042 | 87.78% |
| Branches | 4649 | 1400 | 69.89% |

The current largest implementation files are:

| File | Size |
| --- | ---: |
| `src/ruckig_c/tracking.c` | 94906 bytes |
| `src/ruckig_c/position_third_step2.c` | 65914 bytes |
| `src/ruckig_c/ruckig.c` | 62907 bytes |
| `src/ruckig_c/waypoint.c` | 51070 bytes |
| `src/ruckig_c/input.c` | 39043 bytes |
| `src/ruckig_c/position_third_step1.c` | 36634 bytes |
| `src/ruckig_c/profile.c` | 20867 bytes |
| `src/ruckig_c/roots.c` | 10682 bytes |
| `src/ruckig_c/trajectory.c` | 9783 bytes |
| `src/ruckig_c/brake.c` | 7286 bytes |

Size alone is not the highest-risk signal. The highest maintenance risks are
complex private state machines, long-tail candidate branches, error-path
publication behavior, and random failure reproduction quality.

## Local Quality Slice Evidence

The `post-v0.15.0-quality-audit` local coverage run is recorded at
`out/coverage/post-v0.15.0-quality-audit/coverage-summary.txt`. It passed 63/63
coverage CTest cases and produced the following implementation-source summary:

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 8040 | 948 | 88.21% |
| Functions | 456 | 32 | 92.98% |
| Lines | 8609 | 1051 | 87.79% |
| Branches | 4683 | 1427 | 69.53% |

The branch percentage did not improve over the stable `v0.15.0` baseline. This
is recorded as a real limitation of the slice, not a release blocker: the
implemented value is deterministic invariant coverage, assertion-enabled
private state checks, and better random failure reproduction. The next quality
slice should add targeted branch cases for uncovered high-risk paths before
expanding broad random or line-count-oriented tests.

## Risk Map

| Area | Risk | Current protection | Quality-audit action |
| --- | --- | --- | --- |
| Tracking continuation | Private state spans start, candidate-boundary resume, diagnostics, and output prefix publication. | Focused Fast/Optimized continuation tests, allocation guard, alpha.8 delta-time contract. | Add property selector and private invariant assertions for continuation state and diagnostics counters. |
| Waypoint resume engine | Active/complete/found/candidate state can drift across interrupted online updates. | Resume stress, quality audit, allocation guard. | Add private engine assertions and focused property checks around interrupted start plus complete resume. |
| Input API setters | Repeated DoF/count/null checks increase drift risk in boundary behavior. | Public API and validation tests. | Extract private helper checks without changing public behavior or return codes. |
| No-waypoint trajectory sampling | Core solver is strong but branch-heavy; disabled DoF and duration consistency should stay explicit. | Fixed C tests plus oracle random comparisons. | Add deterministic property tests for boundary samples, duration consistency, and disabled-DoF kinematics. |
| Random audit failures | Existing random tests print summary and representative fallback cases, but not every failing sample in fixture-ready form. | Seeded tracking random/audit and oracle random gates. | Print seed/sample/context on random failures and document materialization steps. |

## Test Selectors

The quality slice adds the focused deterministic selector
`ruckig_c_property_invariants`, backed by `ruckig_c_tests --property-invariants`.
It is intentionally small enough for routine CTest and separate from large
manual random gates.

Existing selectors that remain relevant to this audit:

| Selector | Purpose |
| --- | --- |
| `ruckig_c_property_invariants` | Deterministic property/metamorphic checks for no-waypoint, tracking continuation, and waypoint resume invariants. |
| `ruckig_c_solver_branch_coverage` | White-box branch probes for selected solver/block paths. |
| `ruckig_c_tracking_sequence_continuation_api` | Continuation lifecycle and accessor API coverage. |
| `ruckig_c_tracking_sequence_fast_continuation` | Fast continuation behavior, resume, invalid inputs, allocation guard, and delta-time contract. |
| `ruckig_c_tracking_sequence_optimized_continuation` | Optimized continuation equivalence, strategy/DoF/budget/candidate matrix, invalid inputs, and delta-time contract. |
| `ruckig_c_tracking_quality_hardening` | Deterministic Optimized tracking random-audit thresholds and representative cases. |
| `ruckig_c_tracking_no_allocation` | Tracking real-time path allocation guard. |
| `ruckig_c_waypoint_resume_stress` | Waypoint true-resume budget matrix, long online loop, and allocation guard. |
| `ruckig_c_allocation_audit` | Static allocation audit for real-time-sensitive paths. |

## Random Failure Materialization

Tracking random stress now prints a `failure_context` line when a sample adds a
test failure. The line includes seed, sample, DoF count, signal, lookahead,
reactiveness, strategy, disabled-DoF state, disabled DoF, and start time.

Tracking random audit representative lines include seed and sample alongside
strategy, DoF count, signal, lookahead, reactiveness, disabled-DoF state,
constraint mode, status, selected candidate family, near-tie state, candidate
counters, budget exhaustion, and score metrics.

Oracle random repro output includes kind, seed, sample, DoF count, delta time,
control interface, synchronization, duration discretization, state vectors,
limits, enabled vector, and per-DoF override vectors.

When a random gate fails:

1. Re-run the same command with the reported seed and sample count large enough
   to include the failing sample.
2. Copy the reported context into a fixed C or C++ regression case near the
   matching selector.
3. Reduce dimensions manually first: DoF count, waypoint count, target sequence
   count, disabled DoF, and candidate budget.
4. Reduce state and limit magnitudes only after the structural shape is fixed.
5. Keep the large random command as local/manual evidence; add only the reduced
   deterministic regression to routine CTest.

The first quality slice does not add a full automatic shrinker. A later
optional local tool can shrink DoF count, waypoint/target counts, then numeric
state vectors, but it must remain outside default push CI.

## Evidence Commands

Focused development gate:

```powershell
cmake --build --preset windows-clang-ninja
ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_property_invariants|ruckig_c_tracking_sequence_continuation_api|ruckig_c_tracking_sequence_fast_continuation|ruckig_c_tracking_sequence_optimized_continuation|ruckig_c_waypoint_resume_stress|ruckig_c_allocation_audit"
git diff --check
```

Internal assertion gate:

```powershell
cmake --build --preset windows-clang-ninja-internal-asserts
ctest --test-dir out\build\windows-clang-ninja-internal-asserts --output-on-failure -R "ruckig_c_property_invariants|ruckig_c_tracking_sequence_fast_continuation|ruckig_c_tracking_sequence_optimized_continuation|ruckig_c_waypoint_resume_stress"
```

Coverage evidence:

```powershell
.\tools\coverage\run_coverage.ps1 -CoverageLabel post-v0.15.0-quality-audit
```

Heavy random evidence remains local/manual:

```powershell
out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --random 100000 --seed 1
out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --random 100000 --seed 2
out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --random 100000 --seed 41
out\build\windows-clang-ninja\ruckig_c_tests.exe --tracking-random 100000 --seed 1
out\build\windows-clang-ninja\ruckig_c_tests.exe --tracking-random 100000 --seed 2
out\build\windows-clang-ninja\ruckig_c_tests.exe --tracking-random 100000 --seed 41
```

Release-scale `1000000 --seed 1` runs remain reserved for later release or
stable-line closeout decisions.
