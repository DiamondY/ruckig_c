# Code Quality Audit

This audit starts the `post-v0.15.0-quality-audit` slice after the stable
`v0.15.0` release. It is a quality and testability slice only: it does not
start `0.16.0-design`, change version metadata, expand the public C ABI, tag a
release, publish wrappers, or change package-manager scope.

## Baseline

Current `main` has published the `v0.16.0` stable public diagnostics release.
The stable public C ABI baseline has moved from the `v0.15.0` 184-symbol
continuation surface to the 190-symbol diagnostics surface released at
`v0.16.0`.

The `v0.16.0` stable release stabilizes the opt-in public diagnostics API
without adding further quality/coverage scope. It changed version metadata and
ABI artifact paths for the release, but kept package-manager recipes, wrapper
publication status, upstream baseline, visualization assets, and heavy random
CI policy unchanged.

Quality impact of the diagnostics line:

| Area | Evidence |
| --- | --- |
| Legacy behavior | Existing validate/calculate/update APIs route through shared implementations and keep return-code compatibility. |
| Public diagnostics | `ruckig_diagnostics_t` is caller-owned, initialized with `ruckig_diagnostics_init`, and uses `struct_size` for future extension. |
| Tracking boundary | Tracking uses generic getter-style public diagnostics rather than adding `_with_diagnostics` variants to every tracking operation. |
| Private detail boundary | Solver branches, waypoint queues, tracking candidate order, scoring internals, optimizer phases, and random seed/sample tooling remain private. |
| Verification | `0.16.0-readiness` gates pass normal/shared CTest, oracle fixed/random, performance, ABI/export, Rust, and Python prototype smoke evidence. |

The stable release does not continue coverage-percentage work. Future
coverage additions are reserved for concrete regressions, oracle-backed cases,
or public-behavior-backed cases rather than low-value branch probes.

The `post-v0.16.0-tracking-scenario-maintenance` triage keeps that policy for
tracking. The 10k tracking random audit and pass-preserving shrink sample did
not identify a new regression or stable public-behavior gap, so no tracking
source or test case is added for coverage percentage alone.

Release coverage is recorded at
`out/coverage/0.16.0/coverage-summary.txt`:

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 8284 | 879 | 89.39% |
| Functions | 491 | 30 | 93.89% |
| Lines | 9191 | 1030 | 88.79% |
| Branches | 4782 | 1280 | 73.23% |

Local `0.15.0` implementation coverage, filtered to implementation sources and
excluding generated, test, example, binding, and output paths:

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 7990 | 932 | 88.34% |
| Functions | 451 | 32 | 92.90% |
| Lines | 8525 | 1042 | 87.78% |
| Branches | 4649 | 1400 | 69.89% |

At the `v0.15.0` stable release baseline, the largest implementation files
were:

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

## State-Machine Branch Coverage Slice

The follow-up `post-v0.15.0-state-machine-branch-coverage` slice adds targeted
fixed cases for the high-risk state-machine group called out above:
`tracking.c`, `waypoint.c`, and `input.c`. It keeps the same release boundary:
no public C ABI change, no version metadata change, no tag, no workflow change,
no ABI allowlist edit, and no `0.16.0-design` transition.

The slice adds `ruckig_c_state_machine_branch_coverage`, backed by
`ruckig_c_tests --state-machine-branch-coverage`. The selector covers:

| Area | Added deterministic branch coverage |
| --- | --- |
| `input.c` | Per-section setter/getter null/count/capacity boundaries, minimum-duration invalid values, waypoint-count changes clearing per-section flags, and null-tolerant clear helpers. |
| `waypoint.c` | Interrupted resume identity mismatch for target, waypoint values/count, max/min limits, enabled DoF, synchronization, per-section constraints, and cleared interrupt state. |
| `tracking.c` | Empty/unstarted continuation, wrong DoF/capacity/delta-time resume rejection, diagnostics error-state marking, and failed-resume state preservation across Fast and Optimized continuation paths. |

Local coverage for this slice is recorded at
`out/coverage/post-v0.15.0-state-machine-branch-coverage/coverage-summary.txt`.
It passed 64/64 coverage CTest cases and produced:

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 8040 | 914 | 88.63% |
| Functions | 456 | 30 | 93.42% |
| Lines | 8609 | 1017 | 88.19% |
| Branches | 4683 | 1370 | 70.75% |

This reaches the branch target for the slice. Compared with the previous
quality-audit run, missed branches drop from `1427` to `1370` and branch
coverage rises from `69.53%` to `70.75%`.

## Solver Branch Coverage And Calculate Skeleton Slice

The `post-v0.15.0-solver-branch-coverage` slice follows the state-machine
coverage work with deterministic fixed cases for the solver branch group and a
conservative internal refactor of the repeated `ruckig.c` calculate skeleton.
It keeps the same release boundary: no public C ABI change, no version
metadata change, no tag, no workflow change, no ABI allowlist edit, no
upstream baseline change, no visualization asset change, and no
`0.16.0-design` transition.

The slice keeps the existing `ruckig_c_solver_branch_coverage` selector and
adds:

| Area | Added deterministic coverage or quality change |
| --- | --- |
| `position_third_step1.c` | Direct branch cases for null argument rejection, zero-duration no-motion, jerk-limited rest-to-rest, reverse-direction motion, zero-jerk single-step behavior, zero-limit failure, and block interval publication. |
| `position_third_step2.c` | Direct branch cases for invalid duration rejection, zero-jerk rejection, too-small synchronization duration, valid stretched synchronization, and reverse-direction synchronization. |
| Oracle fixed cases | Third-order no-waypoint cases for mixed per-DoF synchronization, Phase fallback on non-proportional state, and directional min velocity/acceleration limits. |
| `ruckig.c` | Private `static` callback skeleton for shared calculate synchronization/finalization while keeping candidate order, tolerances, disabled-DoF behavior, block selection, result codes, and public ABI unchanged. |

Local coverage for this slice is recorded at
`out/coverage/post-v0.15.0-solver-branch-coverage/coverage-summary.txt`. It
passed 64/64 coverage CTest cases and produced:

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 7909 | 839 | 89.39% |
| Functions | 471 | 30 | 93.63% |
| Lines | 8525 | 957 | 88.77% |
| Branches | 4579 | 1305 | 71.50% |

This reaches the `71.5%+` branch target for the slice. Compared with the
state-machine coverage run, missed branches drop from `1370` to `1305` and
branch coverage rises from `70.75%` to `71.50%`. The total branch count drops
from `4683` to `4579` because duplicated `ruckig.c` calculate skeleton
branches were collapsed into one private callback skeleton.

Hotspot movement:

| File | Previous branch coverage | Current branch coverage |
| --- | ---: | ---: |
| `src/ruckig_c/ruckig.c` | 69.52% | 72.35% |
| `src/ruckig_c/position_third_step1.c` | 65.97% | 69.10% |
| `src/ruckig_c/position_third_step2.c` | 79.15% | 80.40% |

## Solver-Adjacent Branch Coverage Slice

The `post-v0.15.0-solver-adjacent-branch-coverage` slice continues the
coverage-priority work without expanding the public ABI or release state. It
keeps the existing `ruckig_c_solver_branch_coverage` selector, adds fixed
direct probes for brake and lower-order step paths, and adds public oracle
cases for brake pre-trajectories. No production-code refactor was retained:
the tests reached the target, and no solver-adjacent helper extraction was
clearly worth the added churn.

Local coverage for this slice is recorded at
`out/coverage/post-v0.15.0-solver-adjacent-branch-coverage/coverage-summary.txt`.
It passed 64/64 coverage CTest cases and produced:

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 7909 | 803 | 89.85% |
| Functions | 471 | 30 | 93.63% |
| Lines | 8525 | 919 | 89.22% |
| Branches | 4579 | 1239 | 72.94% |

This reaches the `72.5%+` branch target for the slice. Compared with the
previous solver branch coverage run, missed branches drop from `1305` to
`1239`, and branch coverage rises from `71.50%` to `72.94%`.

Hotspot movement:

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

## Random Repro Materialization Slice

The `post-v0.15.0-random-repro-materialization` slice shifts the next quality
increment from coverage percentage to failure localization. It adds
single-sample replay/export commands for seeded oracle and tracking random
corpora while keeping public ABI, version metadata, workflow, tag/release
state, upstream baseline, and visualization assets unchanged.

Added developer tooling:

| Area | Replay command |
| --- | --- |
| Oracle random | `ruckig_c_oracle_tests --replay-random SAMPLE --seed S` |
| Oracle per-DoF random | `ruckig_c_oracle_tests --replay-random-per-dof SAMPLE --seed S` |
| Tracking random stress | `ruckig_c_tests --tracking-random-replay SAMPLE --seed S` |
| Tracking random audit | `ruckig_c_tests --tracking-random-audit-replay SAMPLE --seed S` |

Each replay command fast-forwards the existing generator to the requested
sample, executes the same single-case checks, and prints a fixture-ready
initializer for the matching fixed-case corpus. No generated source file is
written automatically. The existing random commands keep their output and
semantics unchanged.

## Random Shrinker MVP Slice

The `post-v0.15.0-random-shrinker-mvp` slice builds on the replay/export
commands with local pass-preserving shrink tooling. It is a developer aid, not
a coverage slice and not a default heavy CI gate.

Added shrink commands:

| Area | Shrink command |
| --- | --- |
| Oracle random | `ruckig_c_oracle_tests --shrink-random SAMPLE --seed S` |
| Oracle per-DoF random | `ruckig_c_oracle_tests --shrink-random-per-dof SAMPLE --seed S` |
| Tracking random audit | `ruckig_c_tests --tracking-random-audit-shrink SAMPLE --seed S` |

The shrinkers first confirm the requested seed/sample still passes the same
single-case oracle or tracking audit runner. They then accept only
simplifications that continue to pass: DoF count, masks and overrides,
tracking lookahead/config flags, and conservative numeric rounding. Output
includes the original seed/sample, reduced-case summary, a fixture-ready
initializer, and the replay command for the original generated case.

This MVP does not write generated fixture files automatically and does not
alter random corpus semantics. Failure-oriented shrink tooling is handled by
separate local-tool slices.

## Failure-Oriented Shrinker Prototype

The `post-v0.15.0-failure-shrinker-prototype` slice adds local
failure-oriented oracle shrink commands:

```powershell
out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --shrink-random-failure SAMPLE --seed S
out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --shrink-random-per-dof-failure SAMPLE --seed S
```

These commands replay a single random seed/sample and require the original case
to fail before shrinking begins. Candidate simplifications are accepted only
when the reduced case still fails with the same coarse oracle failure class,
such as result mismatch, duration mismatch, or sample/update vector mismatch.

The prototype prints the original seed/sample, original and reduced failure
summaries, the original replay command, reduced fixed-case instructions, and a
fixture-ready `CaseData` initializer. It still does not write generated source
files automatically.

The `post-v0.16.0-tooling-maintenance` slice extends failure-oriented shrinking
to tracking random audit cases:

```powershell
out\build\windows-clang-ninja\ruckig_c_tests.exe --tracking-random-audit-shrink-failure SAMPLE --seed S
```

The tracking failure shrinker replays the requested audit seed/sample first.
If the sample still passes, it returns the expected development error and
points callers to the pass-preserving `--tracking-random-audit-shrink` mode.
If the sample fails, candidate simplifications are accepted only when the
reduced case preserves the same coarse tracking audit failure class. Output
includes original and reduced summaries, the original replay command, and a
fixture-ready `tracking_audit_case_config_t` initializer.

The local checklist is
`docs/release/checklists/post-v0.15.0-failure-shrinker-prototype.md`; the
tracking follow-up checklist is
`docs/release/checklists/post-v0.16.0-tooling-maintenance.md`.

## Residual Branch Coverage Slice

The `post-v0.15.0-residual-branch-coverage` slice uses the refreshed coverage
map to add only high-value deterministic branch coverage. It targets public
boundary behavior in `output.c` and `trajectory.c` rather than analytical
solver long-tail branches.

Selection rationale:

| File | Why selected |
| --- | --- |
| `src/ruckig_c/output.c` | Branch coverage was `60.42%`, with missing public create/null/default getter paths. These are stable API contracts and low-risk to test. |
| `src/ruckig_c/trajectory.c` | Branch coverage was `76.00%`, with missing public create/accessor/intermediate-duration boundary paths. |

The coverage run
`.\tools\coverage\run_coverage.ps1 -CoverageLabel post-v0.15.0-residual-branch-coverage`
passed 72/72 coverage CTest cases and produced
`out/coverage/post-v0.15.0-residual-branch-coverage/coverage-summary.txt`.

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 7939 | 758 | 90.45% |
| Functions | 472 | 30 | 93.64% |
| Lines | 8590 | 906 | 89.45% |
| Branches | 4591 | 1198 | 73.91% |

Compared with the quality evidence refresh, missed branches drop from `1219`
to `1198`, and branch coverage rises from `73.45%` to `73.91%`. The `73.5%+`
target is met. Remaining `velocity_third_step2.c` and
`position_second_step2.c` gaps are still valid future candidates, but this
slice deliberately avoids fragile white-box probes that would assert internal
polynomial alternatives without public or oracle evidence.

## Post-v0.16.0 Long-Tail Coverage Triage

The `post-v0.16.0-oracle-backed-long-tail-coverage` slice performs a docs-only
triage of the remaining solver and trajectory long-tail gaps after the stable
`v0.16.0` release. It adds no tests because no compact, high-value
oracle-backed or public-behavior-backed case was identified.

The triage uses
`out/coverage/post-v0.15.0-residual-branch-coverage/coverage-summary.txt` as
the current reference. Remaining candidates are treated as follows:

| File | Decision |
| --- | --- |
| `velocity_third_step2.c` | Still the lowest solver branch coverage candidate, but remaining misses are analytical timing alternatives that need oracle-backed evidence or stable public timing invariants. |
| `position_second_step2.c` | Valid future candidate, but no new compact public case was selected. |
| `roots.c` | Roots numeric audit already covers the review-followup numerical risks. |
| `profile.c` | Profile context smoke and solver/oracle gates cover stable behavior; remaining gaps are private validation combinations. |
| `trajectory.c` | Public boundary coverage was already added; remaining misses are defensive or hard-to-reach internal states. |

Future long-tail coverage must be motivated by a concrete oracle mismatch,
public-behavior regression, or stable invariant. Coverage percentage alone is
not accepted as a reason to add white-box probes.

## Portability And Static Audit Slice

The `post-v0.15.0-portability-static-audit` slice records portability and
private-linkage evidence after the residual coverage work. It is evidence-only:
no production code, public C ABI, public header, version metadata, workflow,
upstream baseline, wrapper publication status, or visualization asset changed.

Local evidence:

| Area | Result |
| --- | --- |
| Normal build and CTest | `cmake --build --preset windows-clang-ninja` passed; normal CTest passed 67/67. |
| Shared build and CTest | `cmake --build --preset windows-clang-ninja-shared` passed; shared CTest passed 67/67. |
| Oracle build | `cmake --build --preset windows-clang-ninja-oracle` passed. |
| ABI/export boundary | `ruckig_c_verify_public_symbols` and `ruckig_c_compare_public_exported_symbols` passed against the `v0.15.0` 184-symbol allowlist. |
| Default/private compile probe | `zig cc` compiled `src\ruckig_c\waypoint.c` with `RUCKIG_C_STATIC_DEFINE`. |
| Custom clock compile probe | `zig cc` compiled `test\c\platform_clock_custom_compile.c` with `RUCKIG_C_STATIC_DEFINE` and the custom clock provider include path. |

The raw source-file `waypoint.c` probe requires `RUCKIG_C_STATIC_DEFINE` or
`RUCKIG_C_BUILDING_LIBRARY` on Windows. Otherwise `RUCKIG_C_API` expands to
consumer-side `__declspec(dllimport)`, which is invalid on public function
definitions but not a library source issue.

Static analysis remains local/manual evidence. This slice deliberately avoids
adding clang-tidy/cppcheck to default push CI, avoids formatter churn, and
does not perform broad C89-to-C99 declaration-style rewrites.

The local checklist is
`docs/release/checklists/post-v0.15.0-portability-static-audit.md`.

## Quality Series Closeout

The `post-v0.15.0-quality-closeout` slice closes the post-release quality
series as a maintenance phase. It adds no production code and keeps the
`v0.15.0` 184-symbol public C ABI baseline unchanged.

Final coverage-bearing baseline:

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 7939 | 758 | 90.45% |
| Functions | 472 | 30 | 93.64% |
| Lines | 8590 | 906 | 89.45% |
| Branches | 4591 | 1198 | 73.91% |

The closed quality series covers state-machine branch tests, solver and
solver-adjacent fixed cases, random replay/export, pass-preserving shrink
tooling, external-review hardening, residual public-boundary coverage, and
portability/static evidence. Future quality work should start from a concrete
regression, an oracle-backed solver case, a public-behavior invariant, or a
separate design decision. Coverage percentage alone is no longer a sufficient
reason to add tests.

The local checklist is
`docs/release/checklists/post-v0.15.0-quality-closeout.md`.

## Risk Map

| Area | Risk | Current protection | Quality-audit action |
| --- | --- | --- | --- |
| Tracking continuation | Private state spans start, candidate-boundary resume, diagnostics, and output prefix publication. | Focused Fast/Optimized continuation tests, allocation guard, alpha.8 delta-time contract. | Add property selector and private invariant assertions for continuation state and diagnostics counters. |
| Waypoint resume engine | Active/complete/found/candidate state can drift across interrupted online updates. | Resume stress, quality audit, allocation guard. | Add private engine assertions and focused property checks around interrupted start plus complete resume. |
| Input API setters | Repeated DoF/count/null checks increase drift risk in boundary behavior. | Public API and validation tests. | Extract private helper checks without changing public behavior or return codes. |
| No-waypoint trajectory sampling | Core solver is strong but branch-heavy; disabled DoF and duration consistency should stay explicit. | Fixed C tests plus oracle random comparisons. | Add deterministic property tests for boundary samples, duration consistency, and disabled-DoF kinematics. |
| Random audit failures | Seeded random failures must become fixed regressions without hand-reconstructing generator state. | Failure context output, replay/export commands, and MVP pass-preserving shrink commands for oracle and tracking audit corpora. | Keep replay/shrink commands covered by small CTest smoke; defer full failure-oriented shrinking and generated source-file writing. |

## Test Selectors

The quality slice adds the focused deterministic selector
`ruckig_c_property_invariants`, backed by `ruckig_c_tests --property-invariants`.
It is intentionally small enough for routine CTest and separate from large
manual random gates.

Existing selectors that remain relevant to this audit:

| Selector | Purpose |
| --- | --- |
| `ruckig_c_property_invariants` | Deterministic property/metamorphic checks for no-waypoint, tracking continuation, and waypoint resume invariants. |
| `ruckig_c_state_machine_branch_coverage` | Deterministic branch probes for `input.c`, `waypoint.c`, and `tracking.c` state-machine and boundary paths. |
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

Replay/export commands now materialize the same generator state without running
the full random range:

```powershell
out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --replay-random 17 --seed 1
out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --replay-random-per-dof 10 --seed 1
out\build\windows-clang-ninja\ruckig_c_tests.exe --tracking-random-replay 22 --seed 1
out\build\windows-clang-ninja\ruckig_c_tests.exe --tracking-random-audit-replay 22 --seed 1
```

These commands run one generated sample and print fixture-ready initializer
text for the fixed oracle or tracking regression corpus. They do not write
source files automatically.

## External Review Follow-Up Quality Hardening

The `post-v0.15.0-review-followup-quality-hardening` slice addresses the
accepted external review items after the random replay/export materialization
work. It remains a post-release quality slice: no `0.16.0-design`, no public C
ABI change, no public header change, no version metadata change, no workflow
change, no tag or release action, no wrapper publication claim, no upstream
baseline change, and no visualization asset change.

Review triage:

| Review item | Disposition |
| --- | --- |
| Partial `allocate_input_vectors` cleanup invariant | Reasonable maintainability concern; documented the calloc-zeroed owner invariant and unified repeated private double-vector allocation helpers. |
| Waypoint branch queue overflow | The production overflow claim is false: `insert_branch` saturates `branch_count` at `RUCKIG_WAYPOINT_BRANCH_QUEUE_CAPACITY` and discards worse branches once full. Added a saturation regression to keep that private invariant explicit. |
| `roots.c` exact `0.0 == A` check | Accepted as a numerical robustness cleanup. The implemented direct `fabs(A) < DBL_EPSILON` guard passed roots, oracle fixed, oracle random, and performance gates, so the scale-aware fallback was not needed. |
| `tracking.c` file size and mixed responsibilities | Accepted. Split into lifecycle/config/diagnostics, online update/candidate scoring, and sequence/continuation compilation units with private `tracking_internal.h` declarations only. |
| Repeated interrupt context | Accepted. Replaced no-waypoint, waypoint, and tracking private contexts with header-only `interrupt_context.h`; no exported symbol added. |
| `profile_check` parameter explosion | Accepted with a low-risk implementation shape: private context structs and `_ctx` implementations are now the single implementation entry points; old long-parameter names are private macro shims expanding to context compound literals. |
| `waypoint_planning_identity_equals` long boolean chain | Accepted. Split into scalar/flag, base array, optional per-DoF, and per-section helpers without changing exact equality semantics. |
| `ruckig_update` length | Accepted as a local readability cleanup. Extracted publish, calculate/resume, and sample helpers without changing public API or state publication behavior. |
| C89-style declaration churn | Deferred. Broad style-only churn remains lower value than targeted risk reduction. |

Added or changed protection:

| Area | Evidence |
| --- | --- |
| Roots numeric audit | New `ruckig_c_roots_numeric_audit` selector covers resolvent/cubic/quartic small-scale and repeated-root behavior with no heap allocation. |
| Waypoint branch queue saturation | `ruckig_c_state_machine_branch_coverage` now drives a 5-waypoint, 4-DoF interrupted/resume case that saturates the private branch queue and verifies bounded indices. |
| Tracking split | Normal full CTest, shared full CTest, tracking online/offline/optimized/interrupt/no-allocation, and sequence continuation gates passed after the split. |
| Profile context conversion | Explicit `_ctx` smoke covers first-order, second-order position, second-order velocity, third-order position, and third-order velocity profile checks. |
| ABI/export boundary | Shared-library exported public symbols still match the `v0.15.0` allowlist. |

Local gate summary for this slice:

| Gate | Result |
| --- | --- |
| Normal build and full CTest | Passed, 66/66 |
| Shared build and full CTest | Passed, 66/66 |
| Internal-asserts focused CTest | Passed, 5/5 |
| Oracle fixed and random | Fixed oracle 92 plus waypoint oracle 4 passed; 100k random seeds 1/2/41 and per-DoF seed 1 passed |
| Performance | No-waypoint ratio `1.03447` under `1.5`; waypoint C-only alpha corpus passed |
| Wrappers | Rust 16/16 plus examples; Python prototype 24/24 against shared DLL |

The checklist is
`docs/release/checklists/post-v0.15.0-review-followup-quality-hardening.md`.

## Quality Evidence Refresh

The `post-v0.15.0-quality-evidence-refresh` slice records the coverage and
hotspot baseline after the external-review hardening work. It is docs/evidence
only: no production code, public C ABI, version metadata, workflow, upstream
baseline, or visualization asset changed.

The refreshed coverage run is recorded at
`out/coverage/post-v0.15.0-quality-evidence-refresh/coverage-summary.txt`. It
passed 69/69 coverage CTest cases and produced:

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 7939 | 773 | 90.26% |
| Functions | 472 | 30 | 93.64% |
| Lines | 8590 | 912 | 89.38% |
| Branches | 4591 | 1219 | 73.45% |

Compared with the previous coverage-bearing slice,
`post-v0.15.0-solver-adjacent-branch-coverage`, missed branches drop from
`1239` to `1219`, and branch coverage rises from `72.94%` to `73.45%`. The
increase comes from the review-followup tests and refactor shape; this refresh
slice itself adds evidence only.

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

Remaining low-coverage interpretation:

| Area | Current reading |
| --- | --- |
| `velocity_third_step2.c` and `position_second_step2.c` | Still good residual solver candidates, but new cases should be backed by oracle behavior or stable timing invariants. |
| `output.c` | Public boundary gaps were covered by the residual branch slice; remaining misses are allocation-failure defensive paths. |
| `trajectory.c` | Public create/accessor/intermediate-duration gaps were covered by the residual branch slice; remaining misses are defensive or hard-to-reach invalid internal state paths. |
| `tracking.c`, `tracking_sequence.c`, and `waypoint.c` | Still important state-machine surfaces, but broad branch probing is no longer the priority after dedicated state-machine and review-followup slices. |
| `platform_clock.h`, `alloc.c`, and `utils.c` | Low percentages are mostly defensive, tiny-denominator, or platform-specific paths; platform probes and ABI/export gates are higher-value than forced branch tests. |

When a random gate fails:

1. Re-run the same command with the reported seed and sample count large enough
   to include the failing sample.
2. Re-run the matching replay command with the reported seed and sample.
3. Copy the emitted initializer into a fixed C or C++ regression case near the
   matching selector.
4. Reduce dimensions manually first: DoF count, waypoint count, target sequence
   count, disabled DoF, and candidate budget.
5. Reduce state and limit magnitudes only after the structural shape is fixed.
6. Keep the large random command as local/manual evidence; add only the reduced
   deterministic regression to routine CTest.

The MVP shrink commands can reduce passing seed/sample reproductions into
smaller fixture-ready initializers. Full automatic failure-oriented shrinking
and generated source-file writing remain deferred, and any larger shrink run
must remain outside default push CI.

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
.\tools\coverage\run_coverage.ps1 -CoverageLabel post-v0.15.0-state-machine-branch-coverage
.\tools\coverage\run_coverage.ps1 -CoverageLabel post-v0.15.0-quality-evidence-refresh
.\tools\coverage\run_coverage.ps1 -CoverageLabel post-v0.15.0-residual-branch-coverage
```

Portability/static audit evidence:

```powershell
cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_verify_public_symbols
cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_compare_public_exported_symbols
zig cc -std=c99 -Wall -Wextra -DRUCKIG_C_STATIC_DEFINE -Iinclude -Isrc -c src\ruckig_c\waypoint.c -o $env:TEMP\ruckig_waypoint_posix_probe.o
zig cc -std=c99 -Wall -Wextra -DRUCKIG_C_STATIC_DEFINE -Iinclude -Isrc -Itest\c -c test\c\platform_clock_custom_compile.c -o $env:TEMP\ruckig_custom_clock_probe.o
```

Random replay and shrink evidence:

```powershell
out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --replay-random 17 --seed 1
out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --replay-random-per-dof 10 --seed 1
out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --shrink-random 17 --seed 1
out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --shrink-random-per-dof 10 --seed 1
out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --shrink-random-failure SAMPLE --seed S
out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --shrink-random-per-dof-failure SAMPLE --seed S
out\build\windows-clang-ninja\ruckig_c_tests.exe --tracking-random-audit-shrink 22 --seed 1
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
