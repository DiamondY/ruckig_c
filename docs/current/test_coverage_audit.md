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

## 0.16.0 Stable Release Baseline

Current `main` has published the `v0.16.0` stable public diagnostics release.
The release promotes the 190-symbol diagnostics ABI reviewed during
`0.16.0-readiness`, including the alpha.3 core diagnostics API, alpha.4
state/resume diagnostics mapping, and alpha.5 tracking public diagnostics
getters.

The release candidate regenerated coverage at
`out/coverage/0.16.0/coverage-summary.txt`. Coverage percentage is not the
release objective; future additions should be oracle-backed,
public-behavior-backed, or regression-driven.

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 8284 | 879 | 89.39% |
| Functions | 491 | 30 | 93.89% |
| Lines | 9191 | 1030 | 88.79% |
| Branches | 4782 | 1280 | 73.23% |

## 0.15.0 Stable Release Baseline

`v0.15.0` is the previous stable tracking sequence continuation closeout. The
release promotes alpha.1 through alpha.8
interrupt and continuation evidence, including the alpha.4 public continuation
API expansion, Fast and Optimized continuation behavior, wrapper smoke, and
alpha.8 continuation hardening. `v0.15.0` moves the stable public C ABI
baseline from the `v0.14.0` 172-symbol interrupt-surface baseline to the
184-symbol tracking sequence continuation baseline.

## 0.16.0 Public Diagnostics Design-Line Coverage

The `0.16.0` public diagnostics line is stabilized after the `v0.15.0` stable
release. Alpha.3 expands the public ABI to 188 symbols with the core
diagnostics API; alpha.4 adds no public symbols and extends update diagnostics
for interruption and waypoint resume state; alpha.5 adds two tracking public
diagnostics getter symbols for the 190-symbol stable diagnostics ABI.

Added coverage:

| Area | Evidence |
| --- | --- |
| Core diagnostics selector | Adds `--public-diagnostics` and CTest `ruckig_c_public_diagnostics`. |
| Header visibility | C and C++ header compile tests instantiate `ruckig_diagnostics_t`, reference public enum constants, and call `ruckig_diagnostics_init`. |
| Initialization contract | Covers `ruckig_diagnostics_init(NULL)`, valid init, `NULL` diagnostics parity, and too-small `struct_size` rejection before the operation runs. |
| Validation diagnostics | Covers null arguments, DoF mismatch, non-finite values, negative limits, current-state out-of-limits, target-state out-of-limits, and invalid enum values. |
| Calculate/update diagnostics | Covers trajectory/output mismatch, waypoint capacity mismatch, zero-limit result mapping, success clearing to `NONE`, and legacy API parity. |
| State/resume diagnostics | Covers no-waypoint interruption, waypoint interruption, waypoint resume identity mismatch, limit/per-section/enabled-DoF resume mutations, and failed diagnostics preserving later resume state. |
| Tracking getter diagnostics | Adds `--tracking-public-diagnostics` and CTest `ruckig_c_tracking_public_diagnostics`, covering getter argument validation, too-small `struct_size`, Fast/Optimized success, tracking failure state, continuation empty/interrupted/complete/reset state, and failed resume preserving continuation diagnostics. |
| Boundary | Alpha.5 raises the expected public symbol count to 190 and does not expose solver profile branches, candidate ordering, waypoint queue internals, tracking score internals, optimizer phases, or random seed/sample state. |
| Readiness | `0.16.0-readiness` reruns normal/shared full CTest, fixed and random oracle gates, performance, ABI/export, Rust wrapper smoke, and Python prototype smoke before stable release promotion. |

The local checklists are `docs/release/checklists/0.16.0-alpha.3.md` and
`docs/release/checklists/0.16.0-alpha.4.md` and
`docs/release/checklists/0.16.0-alpha.5.md`. Readiness evidence is recorded in
`docs/release/checklists/0.16.0-readiness.md`.

## 0.15.0-alpha.1 Interrupt Post-Release Quality Baseline

`0.15.0-alpha.1` adds a focused local post-release quality selector for the
interrupt surfaces stabilized by `v0.14.0`. It does not change public API,
public ABI, version metadata, tag/release state, or remote workflow state.

Added C coverage:

| Scenario | Evidence |
| --- | --- |
| Focused selector | Adds `--interrupt-post-release-quality` and CTest `ruckig_c_interrupt_post_release_quality`. |
| Waypoint true-resume | Reuses the 128-case `ruckig_c_waypoint_resume_quality_audit` deterministic corpus. |
| No-waypoint interruption | Adds deterministic first-solve, zero-budget incumbent preservation, and full-budget reference comparison cases for complete-trajectory-boundary interruption. |
| Optimized online tracking | Reuses update and lookahead best-so-far candidate-boundary interruption checks and diagnostics consistency assertions. |
| Negative boundaries | Public `ruckig_calculate` still completes normally with an interrupt budget; `ruckig_tracking_calculate_sequence` remains deferred. |
| Duration build | The same selector is run in the normal and duration-enabled builds to keep interruption availability independent from public duration reporting. |

The local checklist is `docs/release/checklists/0.15.0-alpha.1.md`.

## 0.15.0-alpha.2 Tracking Sequence Interruption API Draft

`0.15.0-alpha.2` is docs-only API draft evidence. It adds
`docs/design/tracking_sequence_interruption_api.md` to record why
`ruckig_tracking_calculate_sequence` interruption cannot be implemented cleanly
through the current API-neutral surface.

Coverage impact:

| Area | Evidence |
| --- | --- |
| Behavioral tests | No new selector or CTest is added by this docs-only slice. |
| API boundary | The draft documents public carrier options and rejects implementation in alpha.2. |
| Latest behavioral baseline | Alpha.1 remains the latest interrupt post-release quality test evidence. |
| ABI/export boundary | Public header, ABI allowlist, source implementation, CMake, and workflow remain unchanged. |

The local checklist is `docs/release/checklists/0.15.0-alpha.2.md`.

## 0.15.0-alpha.3 Consumer And Wrapper Interrupt Smoke

`0.15.0-alpha.3` adds user-facing smoke coverage for the `v0.14.0` interrupt
surfaces. It keeps the C ABI unchanged and keeps Python/Rust wrappers
prototype-only.

Added coverage:

| Area | Evidence |
| --- | --- |
| C examples | Adds `21_no_waypoint_interrupt_boundary.c` and `22_tracking_interrupt_boundary.c`, wired into CMake and focused `ruckig_c_examples_*` CTest names. |
| Python prototype | Adds no-waypoint interrupt and Optimized tracking update/lookahead interrupt smoke tests over the shared library. |
| Rust wrapper | Adds no-waypoint interrupt and Optimized tracking update/lookahead interrupt unit tests. |
| Rust examples | Adds `interrupt_no_waypoint.rs` and `interrupt_tracking.rs` examples. |
| Boundary | No public C header, ABI allowlist, workflow, package-manager, upstream baseline, or visualization asset change. |

The local checklist is `docs/release/checklists/0.15.0-alpha.3.md`.

## 0.15.0-alpha.4 Tracking Sequence Continuation API Scaffold

`0.15.0-alpha.4` starts the public tracking sequence continuation ABI
expansion. It adds the opaque continuation handle, lifecycle/status accessors,
and interruptible/resume entry point declarations while leaving Fast and
Optimized behavior implementation to later slices.

Coverage impact:

| Area | Evidence |
| --- | --- |
| Focused selector | Adds `--tracking-sequence-continuation-api` and CTest `ruckig_c_tracking_sequence_continuation_api`. |
| Lifecycle/accessors | Covers create/destroy/reset, dof/capacity, active/interrupted/complete, completed count, and target count accessors. |
| Behavior boundary | Start/resume entry points return `RUCKIG_ERROR_UNSUPPORTED` in alpha.4 after scaffold validation. |
| ABI/export boundary | Public symbol baseline moves from 172 to 184 with 12 approved additions and no removals. |

The local checklist is `docs/release/checklists/0.15.0-alpha.4.md`.

## 0.15.0-alpha.5 Fast Tracking Sequence Continuation

`0.15.0-alpha.5` implements the Fast-mode behavior for the alpha.4
interruptible tracking sequence continuation API. It does not add public
symbols beyond the 184-symbol alpha.4 baseline and does not change enum values,
result-code values, public diagnostics layout, version metadata, release state,
or manual workflow state.

Added coverage:

| Area | Evidence |
| --- | --- |
| Focused selector | Adds `--tracking-sequence-fast-continuation` and CTest `ruckig_c_tracking_sequence_fast_continuation`. |
| Complete solve | Covers unset and large interrupt budgets completing without interruption. |
| Partial prefix | Covers zero-budget interruption publishing only complete sequence-step prefixes. |
| Resume | Covers repeated `ruckig_tracking_resume_sequence` calls until `is_complete=true` and `completed_count == target_count`. |
| Reset/invalid boundary | Covers reset clearing active state and invalid capacity/dof/resume-without-start cases. |
| Allocation guard | Runs interruptible start/resume paths after handle/input/output construction with allocation forbidden. |
| Regression | Focused gates include the existing tracking interrupt, online/offline, no-allocation, and post-release quality selectors; default CTest passes with the new CTest entry. |

The local checklist is `docs/release/checklists/0.15.0-alpha.5.md`.

## 0.15.0-alpha.6 Optimized Tracking Sequence Continuation

`0.15.0-alpha.6` implements Optimized-mode behavior for the alpha.4
interruptible tracking sequence continuation API. It keeps output publication
at complete sequence-step prefixes while preserving in-progress candidate
enumeration state privately inside the continuation handle.

Added coverage:

| Area | Evidence |
| --- | --- |
| Focused selector | Adds `--tracking-sequence-optimized-continuation` and CTest `ruckig_c_tracking_sequence_optimized_continuation`. |
| Complete equivalence | Verifies large-budget interruptible Optimized sequence output matches the old complete `ruckig_tracking_calculate_sequence` output. |
| Resume equivalence | Verifies zero-budget repeated resume reaches the same deterministic output sequence as a complete solve. |
| Candidate-boundary interruption | Exercises interruption while the current step is still in private candidate enumeration; public output count remains equal to completed prefix count. |
| Diagnostics consistency | Checks Optimized diagnostics stay self-consistent and record actual candidate/budget evidence. |
| Duration build | Runs the continuation selectors in the duration-enabled build to keep interruption availability independent from public duration reporting. |

The local checklist is `docs/release/checklists/0.15.0-alpha.6.md`.

## 0.15.0-alpha.7 Tracking Sequence Continuation Wrapper Coverage

`0.15.0-alpha.7` adds consumer-facing smoke coverage for the tracking sequence
continuation API implemented in alpha.4 through alpha.6. It keeps the public C
ABI at the 184-symbol alpha.4 baseline and keeps Python/Rust wrappers
prototype-only.

Added coverage:

| Area | Evidence |
| --- | --- |
| C example | Adds `examples/c/23_tracking_sequence_continuation.c`, wired into CMake, the `ruckig_c_examples` target, `example_ruckig_c_23_tracking_sequence_continuation`, and `ruckig_c_examples_tracking_sequence_continuation`. |
| Python prototype | Adds cffi declarations, `TrackingSequenceContinuation`, interruptible start/resume methods, and an Optimized sequence continuation smoke test over a shared `ruckig_c` library. |
| Rust wrapper | Adds FFI declarations, `TrackingSequenceContinuation`, interruptible start/resume methods, unit smoke coverage, and `examples/tracking_sequence_continuation.rs`. |
| Boundary | No new public C symbols, enum/result-code values, diagnostics fields, package-manager recipes, version metadata, tag, release, push, or manual workflow. |

The local checklist is `docs/release/checklists/0.15.0-alpha.7.md`.

## 0.15.0 Tracking Sequence Continuation Hardening

The post-alpha.7 hardening pass tightens the continuation contract without
adding exported C symbols, enum values, result-code values, public diagnostics
fields, or version metadata changes.

Added coverage:

| Area | Evidence |
| --- | --- |
| Delta-time contract | Fast and Optimized continuation tests reject resume with a same-DoF tracking handle created with a different `delta_time`, keep continuation state intact after the failed resume, and allow resume on a different handle with the same `delta_time`. |
| Optimized matrix | Optimized continuation equivalence now spans Stable, Balanced, and Aggressive strategies; 1/2/4 DoF cases; disabled DoF; large and zero interrupt budgets; and small/large optimized candidate limits. |
| Invalid boundaries | Optimized continuation tests cover capacity, DoF mismatch, non-finite targets, diagnostics error marking, and allocation-guarded interruptible start/resume paths. |
| Candidate engine | Complete Optimized tracking and Optimized sequence continuation share the same internal candidate-step implementation for candidate family order, horizon values, terminal blends, derivative damping, budget behavior, and scoring updates. |
| API ergonomics | Header compilation covers `RUCKIG_RESULT_IS_OK`, while diagnostics reserved fields and C++ opaque tag usage are documented as source-level API guidance without ABI symbol changes. |

The local checklist is `docs/release/checklists/0.15.0-alpha.8.md`.
Ordinary remote push CI later passed on head commit `5066290`, run
`27421851576`, conclusion `success`.

## 0.15.0-readiness Local Stable-Review Audit

`0.15.0-readiness` reruns the full local release-readiness gate set for the
alpha.1 through alpha.8 tracking sequence interruption and continuation line.
It keeps the public C ABI at the 184-symbol alpha.4 baseline and does not bump
version metadata, create a tag, publish a GitHub Release, trigger a manual
workflow, or make Python/Rust wrapper publication claims.

Readiness coverage summary:

| Area | Evidence |
| --- | --- |
| Build and CTest | Normal, shared, and duration-enabled builds passed; normal/shared/duration full CTest each passed 61/61; focused continuation/interrupt/tracking/allocation/solver/platform selector passed 14/14. |
| Oracle gates | Fixed oracle 82 plus waypoint section oracle 4 passed; random 100k seeds 1, 2, and 41 passed; per-DoF 100k seed 1 passed; local 1M release-random seed 1 passed. |
| ABI/export | Public header and allowlist both contain 184 symbols; current public exported symbols `184`; missing public symbols `0`; public removals `0`; unapproved exported symbols `0`. |
| Wrappers and visualization | Python prototype 24/24 passed against the shared DLL; Rust wrapper 16/16 plus doc tests passed; 10 Rust examples compiled and ran; visualization verify and strict regeneration passed for 30 PNG assets. |
| Boundary | Public header, ABI docs, workflow, `CMakeLists.txt`, upstream baseline, visualization assets, package-manager paths, version metadata, tags, releases, and manual workflows remain outside the readiness documentation diff. |

Local `0.15.0-readiness` coverage summary after filtering out generated, test,
example, binding, and output paths:

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 7990 | 932 | 88.34% |
| Functions | 451 | 32 | 92.90% |
| Lines | 8525 | 1042 | 87.78% |
| Branches | 4649 | 1400 | 69.89% |

Coverage artifacts are under `out/coverage/0.15.0-readiness/`. The readiness
checklist is `docs/release/checklists/0.15.0-readiness.md`.

## v0.15.0 Stable Release Coverage

`v0.15.0` stable closeout reruns the full local release-candidate gate set
after the version metadata and ABI artifact paths move to `0.15.0`. The
authoritative command-by-command release evidence is recorded in
`docs/release/checklists/0.15.0.md`.

Stable release coverage artifacts are generated under `out/coverage/0.15.0/`.

The `0.15.0` coverage run used the same filtered implementation scope as
readiness, excluding generated, test, example, binding, and output paths.

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 7990 | 932 | 88.34% |
| Functions | 451 | 32 | 92.90% |
| Lines | 8525 | 1042 | 87.78% |
| Branches | 4649 | 1400 | 69.89% |

The coverage checklist entry is `docs/release/checklists/0.15.0.md`.

## Post-v0.15.0 Quality Audit

The `post-v0.15.0-quality-audit` slice starts after the stable `v0.15.0`
release evidence. It is a code-quality and test-quality slice only: it does
not start `0.16.0-design`, change version metadata, expand the 184-symbol
public C ABI, tag a release, publish wrappers, edit ABI allowlists, or move
heavy random/coverage gates into default push CI.

Quality-audit coverage goals:

| Area | Evidence |
| --- | --- |
| Branch-risk focus | Branch coverage remains the main coverage metric to improve from the `69.89%` stable baseline, but the first priority is risk-relevant fixed cases and invariants rather than raw line-count gains. |
| Focused selector | Adds `--property-invariants` and CTest `ruckig_c_property_invariants` for deterministic no-waypoint, tracking continuation, and waypoint resume properties. |
| Internal invariants | Adds default-off `RUCKIG_C_ENABLE_INTERNAL_ASSERTS` and a local `windows-clang-ninja-internal-asserts` preset for private state-machine assertions. |
| Hotspots | Tracks `tracking.c`, `waypoint.c`, and `input.c` as the first quality-improvement targets, while deferring solver skeleton abstraction and deep `position_*_step*.c` refactors. |
| Random reproduction | Tracking random stress, tracking random audit representatives, and oracle random repro output now include seed/sample context for turning failures into fixed regression cases. |
| Coverage artifacts | Local coverage for this slice is stored under `out/coverage/post-v0.15.0-quality-audit/`. |

Remaining uncovered lines and branches are expected to stay concentrated in
rare invalid-input guards, infeasible candidate alternatives,
oracle-protected long-tail branches, and defensive paths that are deliberately
hard to reach through public behavior. Those paths should be documented when
coverage is regenerated rather than papered over with broad, low-value tests.

The local `post-v0.15.0-quality-audit` coverage run passed 63/63 coverage
CTest cases and produced:

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 8040 | 948 | 88.21% |
| Functions | 456 | 32 | 92.98% |
| Lines | 8609 | 1051 | 87.79% |
| Branches | 4683 | 1427 | 69.53% |

The first-stage `70.5%+` branch-coverage target was not reached in this slice.
The result is recorded as an honest coverage baseline after adding the property
selector and internal assertion plumbing. The risk-reduction value comes from
fixed invariant coverage and better random-failure materialization; the next
slice should add targeted branch cases before broad random or line-count
expansion.

The quality baseline and materialization workflow are recorded in
`docs/current/code_quality_audit.md`; the local checklist is
`docs/release/checklists/post-v0.15.0-quality-audit.md`.

## Post-v0.15.0 State-Machine Branch Coverage

The `post-v0.15.0-state-machine-branch-coverage` slice follows the broader
quality audit with targeted deterministic branch cases for `input.c`,
`waypoint.c`, and `tracking.c`. It does not start `0.16.0-design`, change
version metadata, expand the 184-symbol public C ABI, tag a release, publish
wrappers, edit ABI allowlists, change workflows, or move heavy random gates
into default push CI.

Added coverage:

| Area | Evidence |
| --- | --- |
| Focused selector | Adds `--state-machine-branch-coverage` and CTest `ruckig_c_state_machine_branch_coverage`. |
| Input boundaries | Per-section vector count/DoF/null handling, minimum-duration invalid values, per-section flag clearing when waypoint count changes, getter failure after clear, and null-tolerant clear helpers. |
| Waypoint resume identity | Interrupted waypoint resume rejects stale engine state when target, waypoint values/count, limits, enabled DoF, synchronization, per-section constraints, or interrupt state change. |
| Tracking continuation invalid states | Empty/unstarted continuation, wrong DoF, wrong output capacity, wrong `delta_time`, diagnostics error marking, and failed-resume state preservation are covered for Fast and Optimized paths. |
| Coverage artifacts | Local coverage for this slice is stored under `out/coverage/post-v0.15.0-state-machine-branch-coverage/`. |

The local coverage run passed 64/64 coverage CTest cases and produced:

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 8040 | 914 | 88.63% |
| Functions | 456 | 30 | 93.42% |
| Lines | 8609 | 1017 | 88.19% |
| Branches | 4683 | 1370 | 70.75% |

The branch-coverage target of `70.0%+` is reached. Compared with the prior
`post-v0.15.0-quality-audit` run, missed branches drop by `57`, and branch
coverage rises from `69.53%` to `70.75%`. Remaining branch gaps are still
mostly solver candidate alternatives, rare invalid-input guards, infeasible
candidate combinations, platform-clock defensive paths, and oracle-protected
long-tail branches.

The local checklist is
`docs/release/checklists/post-v0.15.0-state-machine-branch-coverage.md`.

## Post-v0.15.0 Solver Branch Coverage

The `post-v0.15.0-solver-branch-coverage` slice follows the state-machine
branch work with targeted solver fixed cases and a conservative internal
refactor of the repeated `ruckig.c` calculate skeleton. It keeps public ABI,
version metadata, release state, workflow state, ABI allowlists, upstream
baseline, visualization assets, and wrapper publication status unchanged.

Added coverage and parity protection:

| Area | Evidence |
| --- | --- |
| Focused selector | Extends the existing `--solver-branch-coverage` and CTest `ruckig_c_solver_branch_coverage` selector. |
| Third-order step1 | Direct deterministic cases cover null arguments, zero-duration no-motion, jerk-limited motion, reverse direction, zero-jerk single-step behavior, zero-limit failure, and block interval publication. |
| Third-order step2 | Direct deterministic cases cover invalid and too-small durations, zero-jerk rejection, valid stretched synchronization, and reverse-direction synchronization. |
| Oracle fixed cases | Adds third-order no-waypoint oracle cases for per-DoF synchronization mix, Phase fallback, and directional min velocity/acceleration limits. |
| Calculate skeleton parity | Full normal/shared CTest, fixed oracle, 100k random oracle seeds 1/2/41, per-DoF 100k seed 1, performance, ABI/export, Python prototype, and Rust wrapper gates passed after the private callback refactor. |
| Coverage artifacts | Local coverage for this slice is stored under `out/coverage/post-v0.15.0-solver-branch-coverage/`. |

The local coverage run passed 64/64 coverage CTest cases and produced:

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 7909 | 839 | 89.39% |
| Functions | 471 | 30 | 93.63% |
| Lines | 8525 | 957 | 88.77% |
| Branches | 4579 | 1305 | 71.50% |

The branch-coverage target of `71.5%+` is reached. Compared with the prior
state-machine branch coverage run, missed branches drop by `65`, and branch
coverage rises from `70.75%` to `71.50%`. The total branch count drops by
`104` because the repeated `ruckig.c` calculate loops are now one private
callback skeleton instead of copied synchronization/finalization branches.

The local checklist is
`docs/release/checklists/post-v0.15.0-solver-branch-coverage.md`.

## Post-v0.15.0 Solver-Adjacent Branch Coverage

The `post-v0.15.0-solver-adjacent-branch-coverage` slice follows the solver
skeleton refactor with coverage-priority tests for solver-adjacent low-coverage
paths. It keeps public ABI, version metadata, release state, workflow state,
ABI allowlists, upstream baseline, visualization assets, and wrapper
publication status unchanged.

Added coverage and parity protection:

| Area | Evidence |
| --- | --- |
| Focused selector | Extends the existing `--solver-branch-coverage` and CTest `ruckig_c_solver_branch_coverage` selector. |
| Brake pre-trajectories | Direct deterministic cases cover null/no-op paths, zero-limit no-op, acceleration and velocity bound braking in both directions, velocity-control acceleration braking, and finalize no-segment/one-segment/two-segment behavior. |
| Lower-order step paths | Direct deterministic cases cover first-order position, second-order position, second-order velocity, and extra third-order velocity invalid/synchronized boundaries. |
| Oracle fixed cases | Adds public no-waypoint oracle cases for second-order position current-velocity braking, third-order position current velocity/acceleration braking, and third-order velocity current-acceleration braking. |
| Production-code boundary | No production-code refactor is retained; the slice is test/evidence only after no clearly valuable mechanical helper extraction was identified. |
| Coverage artifacts | Local coverage for this slice is stored under `out/coverage/post-v0.15.0-solver-adjacent-branch-coverage/`. |

The local coverage run passed 64/64 coverage CTest cases and produced:

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 7909 | 803 | 89.85% |
| Functions | 471 | 30 | 93.63% |
| Lines | 8525 | 919 | 89.22% |
| Branches | 4579 | 1239 | 72.94% |

The branch-coverage target of `72.5%+` is reached. Compared with the prior
solver branch coverage run, missed branches drop by `66`, and branch coverage
rises from `71.50%` to `72.94%`.

The local checklist is
`docs/release/checklists/post-v0.15.0-solver-adjacent-branch-coverage.md`.

## Post-v0.15.0 Random Repro Materialization

The `post-v0.15.0-random-repro-materialization` slice follows the
coverage-focused solver-adjacent work with reproducibility tooling. It does
not target a coverage percentage increase. It keeps public ABI, version
metadata, release state, workflow state, ABI allowlists, upstream baseline,
visualization assets, and wrapper publication status unchanged.

Added test tooling:

| Area | Evidence |
| --- | --- |
| Oracle replay | Adds `--replay-random SAMPLE --seed S` and `--replay-random-per-dof SAMPLE --seed S` to run one generated oracle sample and print a fixture-ready `CaseData` initializer. |
| Tracking replay | Adds `--tracking-random-replay SAMPLE --seed S` and `--tracking-random-audit-replay SAMPLE --seed S` to run one generated tracking sample and print fixture-ready C initializers. |
| Routine smoke | Adds `ruckig_c_tracking_random_replay_smoke`, `ruckig_c_tracking_random_audit_replay_smoke`, `ruckig_c_oracle_random_replay_smoke`, and `ruckig_c_oracle_random_per_dof_replay_smoke` as single-sample CTest checks. |
| Compatibility | Existing `--random`, `--random-per-dof`, `--tracking-random`, and `--tracking-random-audit` behavior remains unchanged. |
| Deferred | Automatic failure-oriented shrinking and generated source-file writing remain out of scope for this replay/export slice. |

The local checklist is
`docs/release/checklists/post-v0.15.0-random-repro-materialization.md`.

## Post-v0.15.0 Random Shrinker MVP

The `post-v0.15.0-random-shrinker-mvp` slice adds local shrink tooling on top
of the replay/export commands. It does not target coverage growth and does not
add heavy random work to default CI. The shrinkers are pass-preserving: each
candidate simplification is accepted only if the same single-case oracle or
tracking audit check still passes.

Added test tooling:

| Area | Evidence |
| --- | --- |
| Oracle random shrink | Adds `--shrink-random SAMPLE --seed S`, reducing DoF/option vectors/modes/numeric values where the oracle comparison still passes and printing a fixture-ready `CaseData` initializer. |
| Oracle per-DoF shrink | Adds `--shrink-random-per-dof SAMPLE --seed S`, preserving the existing per-DoF comparison mode while emitting a reduced initializer. |
| Tracking audit shrink | Adds `--tracking-random-audit-shrink SAMPLE --seed S`, reducing audit config dimensions and flags while preserving diagnostics consistency. |
| Routine smoke | Adds `ruckig_c_oracle_random_shrink_smoke`, `ruckig_c_oracle_random_per_dof_shrink_smoke`, and `ruckig_c_tracking_random_audit_shrink_smoke` as deterministic single-sample CTest checks. |
| Compatibility | Existing replay and random commands keep their behavior and output contract. |
| Deferred | Generated source-file writing remains local-tool follow-up work, not default CI behavior. |

The local checklist is
`docs/release/checklists/post-v0.15.0-random-shrinker-mvp.md`.

## Post-v0.15.0 Failure Shrinker Prototype

The `post-v0.15.0-failure-shrinker-prototype` slice extends the oracle test
binary with failure-oriented shrink commands for random and per-DoF random
seed/sample reproductions. It is test/tooling-only and does not affect library
runtime behavior or public ABI.

Added commands:

```powershell
out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --shrink-random-failure SAMPLE --seed S
out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --shrink-random-per-dof-failure SAMPLE --seed S
```

The commands reject non-failing original samples and, for failing samples,
accept only simplifications that preserve the same coarse oracle failure class.
Small CTest smoke entries cover the expected non-failing sample rejection path
with `WILL_FAIL`, while real failure shrinking remains a local/manual debugging
workflow.

Automatic source-file fixture writing remains deferred. The local checklist is
`docs/release/checklists/post-v0.15.0-failure-shrinker-prototype.md`.

## Post-v0.16.0 Tooling Maintenance

The `post-v0.16.0-tooling-maintenance` slice extends failure-oriented shrinker
coverage to tracking random audit cases without changing public ABI or default
heavy CI behavior.

Added tooling:

| Area | Evidence |
| --- | --- |
| Tracking audit failure shrink | Adds `--tracking-random-audit-shrink-failure SAMPLE --seed S`, requiring the original sample to fail before accepting simplifications that preserve the same coarse tracking audit failure class. |
| Smoke coverage | Adds `ruckig_c_tracking_random_audit_shrink_failure_smoke` as an expected-failure CTest on a known non-failing sample. |
| Oracle output | Keeps existing oracle failure shrink CLIs while making the reduced fixed-case instructions explicit. |
| Compatibility | Existing replay, pass-preserving shrink, oracle shrink, and random commands keep their behavior. |
| Deferred | Generated fixture auto-write remains a separate tooling decision. |

The local checklist is
`docs/release/checklists/post-v0.16.0-tooling-maintenance.md`.

## Post-v0.16.0 Wrapper Public Diagnostics Prototype Smoke

The `post-v0.16.0-wrapper-public-diagnostics-prototype-smoke` slice extends
Python and Rust prototype smoke coverage to the stable `v0.16.0` public
diagnostics C API without changing public ABI or wrapper publication status.

Added wrapper smoke:

| Area | Evidence |
| --- | --- |
| Python diagnostics prototype | Covers invalid input diagnostics, successful calculate/update diagnostics, tracking public diagnostics, and tracking sequence continuation diagnostics through `bindings/python_prototype/test_prototype.py`. |
| Rust diagnostics prototype | Covers the same minimal diagnostics paths through `cargo test --manifest-path bindings\rust\Cargo.toml`. |
| Boundary | Existing specialized tracking diagnostics wrappers remain unchanged, and the new diagnostics wrappers do not expose solver/profile/candidate/queue internals. |

The local checklist is
`docs/release/checklists/post-v0.16.0-wrapper-public-diagnostics-prototype-smoke.md`.

## Post-v0.16.0 Consumer Install Diagnostics Smoke

The `post-v0.16.0-consumer-install-and-docs-refresh` slice upgrades installed
CMake and pkg-config consumer smoke sources so the install consumption paths
compile and run minimal public diagnostics usage.

Added consumer smoke:

| Area | Evidence |
| --- | --- |
| Installed CMake consumer | Validates an invalid-limit diagnostics path and a successful `ruckig_calculate_with_diagnostics` path through `test/consumer/cmake/main.c`. |
| pkg-config consumer | Mirrors the same public diagnostics usage through `test/consumer/pkg_config/main.c`; availability depends on the configured platform/toolchain. |
| Boundary | Install exports, package recipes, public ABI, and workflow behavior remain unchanged. |

The local checklist is
`docs/release/checklists/post-v0.16.0-consumer-install-and-docs-refresh.md`.

## Post-v0.16.0 CMake Package Version Policy

The `post-v0.16.0-cmake-package-version-policy` slice tightens installed CMake
package version matching to `SameMinorVersion` and adds
`ruckig_c_cmake_consumer_versioned`. The existing installed CMake consumer
continues to exercise unversioned `find_package(ruckig_c CONFIG REQUIRED)`;
the new focused CTest exercises `find_package(ruckig_c 0.16 CONFIG REQUIRED)`
against the installed package metadata.

The local checklist is
`docs/release/checklists/post-v0.16.0-cmake-package-version-policy.md`.

## Post-v0.16.0 Tracking Public Diagnostics Example

The `post-v0.16.0-tracking-diagnostics-examples-polish` slice adds a focused C
example for the stable tracking public diagnostics getter pattern.

Added example smoke:

| Area | Evidence |
| --- | --- |
| Tracking getter | `examples/c/25_tracking_public_diagnostics.c` performs a valid Fast tracking update and reads `ruckig_tracking_get_last_public_diagnostics`. |
| Continuation getter | The same example reads `ruckig_tracking_sequence_continuation_get_last_diagnostics` on an unstarted continuation and checks the stable tracking-sequence scope/code. |
| Boundary | No public ABI, specialized tracking diagnostics layout, tracking evaluator behavior, workflow, upstream baseline, or visualization asset changes. |

The local checklist is
`docs/release/checklists/post-v0.16.0-tracking-diagnostics-examples-polish.md`.

## Post-v0.16.0 Linked Library Smoke

The `post-v0.16.0-linked-library-smoke` slice adds CTest
`ruckig_c_linked_library_smoke`. Unlike the main white-box `ruckig_c_tests`
binary, the smoke includes only the public header and links the `ruckig_c`
target, covering lifecycle, calculate-with-diagnostics, update, and
`ruckig_output_pass_to_input` through a normal in-tree linked consumer path.
The `post-v0.16.0-linked-consumer-smoke-expansion` slice extends the same smoke
with minimum waypoint calculate and Fast tracking public diagnostics workflows
without inspecting private diagnostics, candidate ordering, score internals, or
queue internals.

The local checklist is
`docs/release/checklists/post-v0.16.0-linked-library-smoke.md`.

The expansion checklist is
`docs/release/checklists/post-v0.16.0-linked-consumer-smoke-expansion.md`.

## Post-v0.16.0 Installed Package Metadata Smoke

The `post-v0.16.0-installed-package-metadata-smoke` slice extends the installed
CMake consumer fixture with target metadata checks. Static installed CMake
target consumers now assert that `RUCKIG_C_STATIC_DEFINE` is propagated by the
exported target, while shared installed target consumers assert that the define
is absent. The fixture also rejects exported target usage requirements that
contain sanitizer or coverage link/compile flags, keeping instrumentation
opt-in instead of leaking to ordinary consumers.

Local focused evidence:

| Gate | Result |
| --- | --- |
| Static installed consumer metadata | Passed in `windows-clang-ninja` through `ruckig_c_cmake_consumer` and `ruckig_c_cmake_consumer_versioned`. |
| Windows manual static consumer | Passed in `windows-clang-ninja`. |
| Shared Windows DLL consumer metadata | Passed in `windows-clang-ninja-shared` through `ruckig_c_windows_dll_consumer`. |
| Shared install-tree consumer | Not configured in the Windows shared preset used for this local check; Unix CI remains the maintained install-tree path. |

The local checklist is
`docs/release/checklists/post-v0.16.0-installed-package-metadata-smoke.md`.

## Post-v0.16.0 Precision Constants Hardening

The `post-v0.16.0-precision-constants-hardening` slice is behavior-preserving
production-code maintenance. It centralizes selected private tolerance
constants and relies on normal C tests, roots numeric audit, solver branch
coverage, public diagnostics, and oracle fixed cases to prove no intended
numeric behavior change.

The local checklist is
`docs/release/checklists/post-v0.16.0-precision-constants-hardening.md`.

## Post-v0.16.0 Tracking Scenario Maintenance Triage

The `post-v0.16.0-tracking-scenario-maintenance` slice performs a docs-only
triage of candidate tracking maintenance cases. It adds no tests because the
selected audit and shrink evidence did not expose a compact new
public-behavior-backed or audit-backed regression case.

Triage evidence:

| Area | Evidence |
| --- | --- |
| Focused tracking gates | Tracking public diagnostics, quality hardening, stability, optimized continuation, and random audit replay smoke passed 5/5. |
| Random audit | `--tracking-random-audit 10000 --seed 1` passed thresholds with 9993 optimized, 7 fallback, 141520 candidates, 0 rejected, and 40298 budget-exhausted evaluations. |
| Shrink sample | `--tracking-random-audit-shrink 22 --seed 1` reduced to a 1-DoF stable/default optimized case; no failure or regression surfaced. |
| Boundary | No tracking source, public ABI, workflow, upstream baseline, visualization, or default heavy CI change. |

Future tracking additions require a reproducible audit/shrinker sample, public
behavior regression, user-reported workflow, or stable invariant. Coverage
percentage alone is not accepted as a reason to add cases.

The local checklist is
`docs/release/checklists/post-v0.16.0-tracking-scenario-maintenance.md`.

## Post-v0.16.0 Constructor Boundary Hardening

The `post-v0.16.0-constructor-boundary-hardening` slice adds focused boundary
coverage for public constructors that derive allocation counts from DoF and
waypoint/sequence capacities.

Added coverage:

| Area | Evidence |
| --- | --- |
| Focused selector | Adds `--constructor-boundaries` and CTest `ruckig_c_constructor_boundaries`. |
| Existing invalid boundaries | Confirms null output pointers and zero DoF continue to return `RUCKIG_ERROR_INVALID_INPUT`. |
| Waypoint capacity overflow | Rejects `SIZE_MAX` waypoint counts and `dofs=2`, `max_waypoints=SIZE_MAX/2+1` multiplication overflow shapes for input, trajectory, output, and OTG constructors. |
| Tracking sequence capacity overflow | Confirms target sequence, tracking output sequence, and tracking sequence continuation constructors reject overflowing `dofs * capacity` shapes. |
| Boundary | No public C ABI, workflow, upstream baseline, visualization asset, wrapper, package, or default heavy-CI change. |

The local checklist is
`docs/release/checklists/post-v0.16.0-constructor-boundary-hardening.md`.

## Post-v0.16.0 Delta-Time Constructor Policy

The `post-v0.16.0-delta-time-policy` slice extends constructor boundary
coverage to OTG `delta_time` values.

Added coverage:

| Area | Evidence |
| --- | --- |
| Focused selector | Reuses `--constructor-boundaries` and CTest `ruckig_c_constructor_boundaries`. |
| Invalid constructor values | Confirms `ruckig_create` and `ruckig_create_with_waypoints` reject negative, NaN, and infinite `delta_time` and leave the output handle `NULL`. |
| Zero compatibility | Confirms ordinary and waypoint OTG constructors still accept `delta_time == 0.0`. |
| Discrete duration boundary | Existing validation coverage continues to reject zero `delta_time` when `RUCKIG_DURATION_DISCRETE` is selected. |
| Boundary | No public C ABI, workflow, upstream baseline, visualization asset, wrapper, package, or default heavy-CI change. |

The local checklist is
`docs/release/checklists/post-v0.16.0-delta-time-policy.md`.

## Post-v0.16.0 Quality Gate Refresh

The `post-v0.16.0-quality-gate-refresh` slice records the constructor boundary
and `delta_time` hardening as covered baseline behavior. It adds no production
code and does not change public ABI, workflow, upstream baseline,
visualization assets, wrappers, or package recipes.

Refreshed local evidence:

| Gate | Result |
| --- | --- |
| Normal build | Passed; no work to do |
| Normal full CTest | Passed; 75/75 tests |
| Shared build | Passed |
| Shared focused CTest | Passed; constructor boundaries, public diagnostics, C header, and C++ header tests all passed 4/4 |
| ABI/export | `ruckig_c_verify_public_symbols` and `ruckig_c_compare_public_exported_symbols` passed; public exported symbols match the approved allowlist. |
| Sanitizer/static analyzer status | ASan/UBSan, Valgrind, and static analyzer runs remain local/manual evidence and were not added to default CI by this slice. |
| Boundary | Public header, ABI docs, workflow, `original/ruckig-main`, and visualization asset diffs are empty. |

Future quality work should keep checked arithmetic as the default for public
constructor derived counts, should not add solver/tracking probes for coverage
percentage alone, and should only promote sanitizer/static analyzer work into
CI after a separate CI policy decision.

The local checklist is
`docs/release/checklists/post-v0.16.0-quality-gate-refresh.md`.

## Post-v0.16.0 Build Instrumentation Scope Hardening

The `post-v0.16.0-build-instrumentation-scope-hardening` slice keeps sanitizer
and coverage instrumentation opt-in for the targets that request it while
removing CMake usage-requirement propagation to downstream consumers.

Added evidence:

| Area | Evidence |
| --- | --- |
| Sanitizer scope | `ruckig_c_apply_sanitizers` keeps compile and link instrumentation private to each instrumented target. |
| Coverage scope | `ruckig_c_apply_coverage` keeps LLVM coverage compile and link instrumentation private to each coverage target. |
| Consumer boundary | Normal and shared consumer/header diagnostics gates remain the verification path for non-instrumented consumers. |
| pkg-config boundary | Sanitizer entries remain in pkg-config private libs only for explicitly instrumented pkg-config builds. |
| Boundary | No public C ABI, workflow, upstream baseline, visualization asset, wrapper, package recipe, or default heavy-CI change. |

The local checklist is
`docs/release/checklists/post-v0.16.0-build-instrumentation-scope-hardening.md`.

## 0.14.0-alpha.1 Interrupt Boundary API-Neutral Audit

`0.14.0-alpha.1` adds a focused local audit for the existing
`interrupt_calculation_duration` boundary. It does not change source
implementation or public ABI. The audit proves waypoint `ruckig_update` with
intermediate waypoints remains the only path that can trigger waypoint
soft-interruption true-resume, while public `ruckig_calculate`, no-waypoint
`ruckig_update`, and tracking stay outside that behavior.

| Area | Evidence |
| --- | --- |
| Focused selector | Adds `--interrupt-boundary-audit` and CTest `ruckig_c_interrupt_boundary_audit`. |
| Waypoint positive path | Zero-budget waypoint `ruckig_update` publishes an initial complete candidate, marks interruption, and leaves private resume active. |
| No-waypoint isolation | No-waypoint update with interrupt set, changed, and cleared reports `was_calculation_interrupted=false` and clears private waypoint resume state. |
| Public calculate isolation | Waypoint `ruckig_calculate` ignores the interrupt budget, completes the solve, and leaves no active private resume state. |
| Tracking isolation | `ruckig_tracking_update`, `ruckig_tracking_update_with_lookahead`, and `ruckig_tracking_calculate_sequence` keep tracking diagnostics and do not use waypoint interruption state. |
| Allocation guard | Covers initial interrupted waypoint update, background interrupted-without-publish resume, and transition to no-waypoint update without online allocation. |
| Local gates | Main build passed; focused selector group passed 7/7; default CTest passed 49/49; duration-enabled selector group passed 3/3. |
| ABI/export | Public header symbol count remains `172`; public additions `0`; public removals `0`; unapproved exported symbols `0`. |
| Boundary | Public header, ABI docs, CI workflow, `original/ruckig-main`, and visualization assets remain unchanged; `CMakeLists.txt` only adds the focused CTest entry. |

The local evidence checklist is
`docs/release/checklists/0.14.0-alpha.1.md`. This slice was later covered by
cumulative ordinary remote push CI with alpha.2 on head commit `ea06684`, run
`27387177406`, conclusion `success`.

## 0.14.0-alpha.2 Future Interrupt Surfaces Design

`0.14.0-alpha.2` is docs-only design evidence. It adds
`docs/design/future_interrupt_surfaces.md` as a quasi-spec for possible future
no-waypoint complete-trajectory-boundary interruption and online tracking
candidate-boundary interruption. It does not change implementation, tests,
public API, public ABI, or active runtime behavior.

Coverage impact:

| Area | Evidence |
| --- | --- |
| Behavioral tests | No new selector or CTest is added by this design-only slice. |
| Behavioral tests | No behavioral coverage is added by this design-only slice; alpha.1 remains the boundary audit baseline before alpha.4 and alpha.5 implementation. |
| Future no-waypoint policy | Documented as complete-trajectory-boundary interruption without true-resume or waypoint-engine reuse. |
| Future tracking policy | Documented as online-only candidate-boundary interruption for `ruckig_tracking_update` and `ruckig_tracking_update_with_lookahead`. |
| Deferred tracking sequence | `ruckig_tracking_calculate_sequence` remains deferred until a separate public diagnostics/API decision. |
| ABI/export boundary | No public header, ABI allowlist, source, CMake, or workflow change is part of this slice. |

The local design checklist is
`docs/release/checklists/0.14.0-alpha.2.md`. Remote CI evidence for alpha.1
and alpha.2 succeeded cumulatively on head commit `ea06684`, run
`27387177406`, conclusion `success`.

## 0.14.0-alpha.3 Interrupt Implementation Readiness Audit

`0.14.0-alpha.3` is docs-only implementation-readiness evidence. It records
that no-waypoint complete-trajectory-boundary interruption and online tracking
best-so-far candidate-boundary interruption can proceed as conditional
API-neutral implementation slices if their local gates pass. It does not
change implementation, tests, public API, public ABI, or active runtime
behavior.

Coverage impact:

| Area | Evidence |
| --- | --- |
| Behavioral tests | No new selector or CTest is added by this docs-only audit. |
| No-waypoint go/no-go | Approved for a later alpha as complete-trajectory-boundary interruption without true-resume. |
| Tracking go/no-go | Approved for a later alpha as online-only best-so-far candidate-boundary interruption for update and lookahead update. |
| Deferred tracking sequence | `ruckig_tracking_calculate_sequence` remains deferred until a separate public diagnostics/API decision. |
| ABI/export boundary | No public header, ABI allowlist, source, CMake, or workflow change is part of this slice. |

The local readiness checklist is
`docs/release/checklists/0.14.0-alpha.3.md`.

## 0.14.0-alpha.4 No-Waypoint Interrupt Boundary Support

`0.14.0-alpha.4` adds API-neutral no-waypoint interruption for
`ruckig_update`. It uses the existing interrupt field and output flag, adds no
public ABI, and checks budget only after a complete target trajectory attempt.

Added C coverage:

| Scenario | Evidence |
| --- | --- |
| Focused selector | Adds `--no-waypoint-interrupt-audit` and CTest `ruckig_c_no_waypoint_interrupt_audit`. |
| First solve | Zero-budget first no-waypoint update publishes a complete candidate when no incumbent exists. |
| Incumbent preservation | Zero-budget changed-target update with a valid no-waypoint incumbent preserves the old trajectory, reports interruption, and does not mark a new calculation. |
| Budget matrix | Covers zero, tiny, large, changed, and cleared interrupt budgets without no-waypoint resume state. |
| Cross-surface isolation | Waypoint incumbents are not preserved when switching from waypoint input to no-waypoint input. |
| Tracking isolation | Tracking clears interrupt on its internal work input in this alpha, so online tracking behavior remains unchanged until alpha.5. |
| Allocation guard | No-waypoint first solve, interrupted incumbent preservation, and budget-clear paths run under the allocation guard. |
| Local gates | Main build passed; focused selector group passed 5/5; default CTest passed 50/50; duration-enabled selector group passed 3/3. |

The local implementation checklist is
`docs/release/checklists/0.14.0-alpha.4.md`.

## 0.14.0-alpha.5 Online Tracking Interrupt Boundary Support

`0.14.0-alpha.5` adds API-neutral online tracking interruption for Optimized
mode. It uses the existing interrupt field and output flag, adds no public ABI,
and checks budget only after complete tracking candidates.

Added C coverage:

| Scenario | Evidence |
| --- | --- |
| Focused selector | Adds `--tracking-interrupt-audit` and CTest `ruckig_c_tracking_interrupt_audit`. |
| Fast mode | Zero-budget Fast tracking update accepts the single complete candidate and does not report interruption. |
| Optimized update | Zero-budget Optimized tracking update publishes the best complete candidate evaluated so far, reports interruption, and records diagnostics for only evaluated candidates. |
| Optimized lookahead | Zero-budget Optimized lookahead update follows the same best-so-far complete-candidate boundary. |
| Sequence deferred | `ruckig_tracking_calculate_sequence` continues to ignore tracking interruption because there is no API-neutral sequence carrier. |
| Allocation guard | Fast, Optimized update, and Optimized lookahead interruption paths run under the allocation guard. |
| Local gates | Main build passed; focused selector group passed 6/6; default CTest passed 51/51; duration-enabled selector group passed 3/3. |
| ABI/export | Public header symbol count remains `172`; public exported symbols match the approved allowlist. |

The local implementation checklist is
`docs/release/checklists/0.14.0-alpha.5.md`.

Alpha.3 through alpha.5 were later covered together by ordinary remote push CI
on head commit `4e0e2fbf3cf0de9a4deddba672828d6f02f446cd`, run
`27391043296`, conclusion `success`.

## 0.14.0-readiness Local Stable-Review Audit

`0.14.0-readiness` reruns the full local stable-review gate set for the
completed alpha.1 through alpha.5 interrupt evidence. It does not change
implementation, public API, public ABI, version metadata, tag, release, or
manual workflow state.

Readiness gate evidence:

| Area | Evidence |
| --- | --- |
| Baseline | Commit `4e0e2fbf3cf0de9a4deddba672828d6f02f446cd`; latest stable release remains `v0.13.0`. |
| Remote alpha evidence | Alpha.1/alpha.2 cumulative push CI run `27387177406` passed; alpha.3-alpha.5 cumulative push CI run `27391043296` passed. |
| Build gates | Main, shared, oracle, performance, and duration builds passed. |
| Routine CTest | Default, shared, and duration-enabled CTest each passed 51/51. |
| Focused interrupt gates | Boundary, no-waypoint interrupt, tracking interrupt, waypoint resume stress/quality, tracking, allocation, platform clock, and solver branch selector group passed 12/12. |
| Oracle gates | Fixed oracle 82 plus waypoint section oracle 4 passed; random 100k seeds 1, 2, 41 passed; per-DoF 100k seed 1 passed; local 1M seed 1 passed. |
| Performance | No-waypoint average ratio `1.27986` stayed below the `1.5` threshold; waypoint trend recorded with average `3.29673e+06 ns`. |
| ABI/export | Public symbol count remains `172`; public additions `0`; public removals `0`; unapproved exported symbols `0`. |
| Wrappers and visualization | Visualization verifier and strict regeneration passed with the shared DLL; Python prototype passed 21/21 with the shared DLL; Rust wrapper tests passed 13/13 and examples built. |
| Boundary | Public header, ABI docs, workflow, CMake, source implementation, `original/ruckig-main`, and visualization assets remain unchanged. |

Local `0.14.0-readiness` coverage summary after filtering out generated, test,
example, and original-reference code:

| Metric | Covered | Total | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 6691 | 7577 | 88.31% |
| Functions | 392 | 424 | 92.45% |
| Lines | 6903 | 7846 | 87.98% |
| Branches | 3082 | 4397 | 70.09% |

Coverage artifacts are under `out/coverage/0.14.0-readiness/`. The readiness
checklist is `docs/release/checklists/0.14.0-readiness.md`.

## v0.14.0 Stable Release Coverage

`v0.14.0` stable release local gates rerun the readiness gate after the
version bump and ABI artifact path update.

Release-candidate gate evidence:

| Area | Evidence |
| --- | --- |
| Focused interrupt gates | Boundary, no-waypoint interrupt, tracking interrupt, waypoint resume stress/quality, tracking, allocation, platform clock, and solver branch selector group passed 12/12. |
| Routine gates | Default, shared, and duration-enabled CTest each passed 51/51. |
| Oracle gates | Fixed oracle 82 plus waypoint section oracle 4 passed; random 100k seeds 1, 2, 41 passed; per-DoF 100k seed 1 passed; local 1M release-random seed 1 passed. |
| Performance and ABI | No-waypoint benchmark ratio `1.31487` stayed below the `1.5` threshold; waypoint benchmark was recorded as C-only trend; public exported-symbol diff stayed clean at 172 public symbols and 0 unapproved exports. |
| Wrappers and visualization | Visualization verifier and strict regeneration passed with the shared DLL; Python prototype passed 21 tests with the shared DLL; Rust wrapper tests passed 13/13 and examples built. |
| Boundary | Public header diff is limited to version macros/string; workflow diff is limited to ABI artifact path `0.14.0`; ABI docs, `original/ruckig-main`, and visualization assets remain unchanged. |

Local `0.14.0` release-candidate coverage summary after filtering out
generated, test, example, and original-reference code:

| Metric | Covered | Total | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 6691 | 7577 | 88.31% |
| Functions | 392 | 424 | 92.45% |
| Lines | 6903 | 7846 | 87.98% |
| Branches | 3082 | 4397 | 70.09% |

Artifacts are under `out/coverage/0.14.0/`, and the command log is recorded in
`docs/release/checklists/0.14.0.md`.

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

## Post-v0.15.0 External Review Follow-Up Coverage

The `post-v0.15.0-review-followup-quality-hardening` slice is primarily a
maintainability and parity-protection slice, not a branch-coverage target
slice. It keeps the `v0.15.0` 184-symbol public C ABI baseline unchanged and
adds targeted evidence around the review items that could affect solver,
state-machine, and numerical behavior.

Added coverage and evidence:

| Area | Added evidence |
| --- | --- |
| Roots numerical robustness | New `ruckig_c_roots_numeric_audit` CTest selector and CLI `--roots-numeric-audit`, covering resolvent zero/tiny-A behavior, small-scale cubic/quartic residuals, repeated/near-zero roots, non-negative filtering, sorting, and no-allocation behavior. |
| Waypoint queue saturation | `ruckig_c_state_machine_branch_coverage` now includes a public interrupted/resume case that reaches a saturated private waypoint branch queue and verifies bounded queue count/index behavior under normal and internal-assert builds. |
| Tracking refactor parity | After splitting `tracking.c`, normal and shared full CTest passed, and focused tracking online/offline/optimized/interrupt/no-allocation plus Fast/Optimized continuation selectors passed. |
| Profile context conversion | `test_profile_context_entrypoints` explicitly exercises `_ctx` profile-check entry points for first-order, second-order position, second-order velocity, third-order position, and third-order velocity paths. |
| Roots guard parity | Fixed oracle 92 plus waypoint section oracle 4 passed; oracle random `100000` seeds `1`, `2`, and `41` passed; per-DoF random `100000` seed `1` passed. |

Local validation summary:

| Gate | Result |
| --- | --- |
| Normal full CTest | 66/66 passed |
| Shared full CTest | 66/66 passed |
| Internal-assert focused CTest | 5/5 passed |
| ABI/export comparison | Public exported symbols match the `v0.15.0` allowlist |
| Rust wrapper smoke | 16/16 tests plus examples passed |
| Python prototype smoke | 24/24 tests passed against the shared DLL |

No coverage artifact is recorded for this slice because the objective is
review follow-up hardening and behavior-preserving refactor parity, not a new
coverage-percentage target. The prior branch-coverage baseline remains
`72.94%` from
`out/coverage/post-v0.15.0-solver-adjacent-branch-coverage/coverage-summary.txt`.

## Post-v0.15.0 Quality Evidence Refresh

The `post-v0.15.0-quality-evidence-refresh` slice reruns coverage after the
external-review hardening work. It records the new evidence baseline after the
tracking behavior split, profile context conversion, roots numeric audit, and
waypoint branch-queue saturation regression. It is evidence-only and keeps
public ABI, version metadata, release state, workflow state, ABI allowlists,
upstream baseline, visualization assets, and wrapper publication status
unchanged.

Coverage artifacts are stored under
`out/coverage/post-v0.15.0-quality-evidence-refresh/`.

The local coverage run passed 69/69 coverage CTest cases and produced:

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 7939 | 773 | 90.26% |
| Functions | 472 | 30 | 93.64% |
| Lines | 8590 | 912 | 89.38% |
| Branches | 4591 | 1219 | 73.45% |

Compared with the previous coverage-bearing solver-adjacent slice, missed
branches drop from `1239` to `1219`, and branch coverage rises from `72.94%`
to `73.45%`. The delta is attributed to the review-followup tests and
refactor shape; this refresh slice adds no implementation behavior.

Current residual branch candidates:

| Area | Interpretation |
| --- | --- |
| `velocity_third_step2.c` | Low branch coverage remains in analytical timing alternatives; future cases should be oracle-backed or assert only stable timing invariants. |
| `position_second_step2.c` | Synchronization timing alternatives remain a focused solver candidate. |
| `output.c` | Public output boundary handling was a good compact fixed-case candidate and is covered by the residual branch slice below. |
| `trajectory.c` | Public create/accessor/intermediate-duration boundaries are covered by the residual branch slice below. |
| `tracking.c`, `tracking_sequence.c`, `waypoint.c` | Important state-machine surfaces, but further additions should be meaningful invariants rather than broad probes. |
| `platform_clock.h`, `alloc.c`, `utils.c` | Low percentages are mostly defensive, platform-specific, or tiny-denominator paths; platform probes and ABI/export evidence are higher value than forced branch coverage. |

## Post-v0.15.0 Residual Branch Coverage

The `post-v0.15.0-residual-branch-coverage` slice follows the refreshed
evidence with one compact deterministic public-boundary case. It does not add
heavy random CI work and does not change production code. The selected files
are `output.c` and `trajectory.c` because their missing branches are stable
public API boundary behavior, while the remaining solver step2 gaps are more
analytical and better left for oracle-backed cases.

Coverage artifacts are stored under
`out/coverage/post-v0.15.0-residual-branch-coverage/`.

The local coverage run passed 72/72 coverage CTest cases and produced:

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 7939 | 758 | 90.45% |
| Functions | 472 | 30 | 93.64% |
| Lines | 8590 | 906 | 89.45% |
| Branches | 4591 | 1198 | 73.91% |

Compared with the quality evidence refresh, missed branches drop from `1219`
to `1198`, and branch coverage rises from `73.45%` to `73.91%`.

Touched-file movement:

| File | Previous branch coverage | Current branch coverage |
| --- | ---: | ---: |
| `src/ruckig_c/output.c` | 60.42% | 85.42% |
| `src/ruckig_c/trajectory.c` | 76.00% | 82.00% |

The local checklist is
`docs/release/checklists/post-v0.15.0-residual-branch-coverage.md`.

## Post-v0.16.0 Oracle-Backed Long-Tail Coverage Triage

The `post-v0.16.0-oracle-backed-long-tail-coverage` slice reviews the remaining
long-tail candidates and adds no new tests. The decision is intentional:
coverage percentage is no longer a primary target after the post-`v0.15.0`
quality series and stable `v0.16.0` release.

Current triage from
`out/coverage/post-v0.15.0-residual-branch-coverage/coverage-summary.txt`:

| Candidate | Branch coverage | Decision |
| --- | ---: | --- |
| `velocity_third_step2.c` | 60.87% | Defer until an oracle mismatch or stable public timing invariant is identified. |
| `position_second_step2.c` | 68.75% | Defer for the same reason; do not force private analytical branches. |
| `roots.c` | 83.85% | Covered by `ruckig_c_roots_numeric_audit` for the review-followup risks. |
| `profile.c` | 80.16% | Covered by profile context smoke and solver/oracle gates for stable entry points. |
| `trajectory.c` | 82.00% | Public boundary cases were already added by the residual slice. |

Future cases must be oracle-backed, public-behavior-backed, or tied to a clear
stable invariant. Defensive/platform/tiny-denominator branches remain better
handled by portability/static evidence or code review than by fragile probes.

The local checklist is
`docs/release/checklists/post-v0.16.0-oracle-backed-long-tail-coverage.md`.

## Post-v0.15.0 Portability And Static Audit

The `post-v0.15.0-portability-static-audit` slice is not a coverage-growth
slice. It records portability, private-linkage, platform-clock, and ABI/export
evidence after the tracking split, profile context conversion, random shrinker
MVP, and residual coverage work.

Local evidence:

| Gate | Result |
| --- | --- |
| Normal build and full CTest | Passed, 67/67 |
| Shared build and full CTest | Passed, 67/67 |
| Oracle build | Passed |
| Windows ABI/export gates | `ruckig_c_verify_public_symbols` and `ruckig_c_compare_public_exported_symbols` passed against the stable 184-symbol `v0.15.0` allowlist. |
| Default/private platform-clock compile probe | `zig cc` compiled `src\ruckig_c\waypoint.c` with `RUCKIG_C_STATIC_DEFINE`. |
| Custom platform-clock compile probe | `zig cc` compiled `test\c\platform_clock_custom_compile.c` with `RUCKIG_C_STATIC_DEFINE` and the test custom-clock provider include path. |

The direct `waypoint.c` source-file probe must use `RUCKIG_C_STATIC_DEFINE`
or `RUCKIG_C_BUILDING_LIBRARY` on Windows because public function definitions
are not consumer-side `dllimport` declarations. This is a probe-context
requirement, not a source portability defect.

Static analysis remains a local/manual activity. No clang-tidy/cppcheck gate,
formatter rewrite, workflow change, public ABI change, version metadata change,
upstream baseline change, or visualization asset change is part of this slice.

The local checklist is
`docs/release/checklists/post-v0.15.0-portability-static-audit.md`.

## Post-v0.15.0 Quality Closeout

The `post-v0.15.0-quality-closeout` slice closes the coverage-driven
post-release quality series. It records the final baseline and keeps future
coverage work tied to high-value behavior rather than percentage targets.

Final coverage-bearing artifact:
`out/coverage/post-v0.15.0-residual-branch-coverage/coverage-summary.txt`.

| Metric | Total | Missed | Coverage |
| --- | ---: | ---: | ---: |
| Regions | 7939 | 758 | 90.45% |
| Functions | 472 | 30 | 93.64% |
| Lines | 8590 | 906 | 89.45% |
| Branches | 4591 | 1198 | 73.91% |

The closeout conclusion is that the major post-`v0.15.0` risk areas now have
appropriate evidence: state-machine boundaries, solver branch families,
solver-adjacent brake/lower-order paths, random replay/materialization,
pass-preserving shrink tooling, external-review hardening, residual public API
boundaries, ABI/export checks, and platform-clock compile probes.

Remaining low-coverage branches should be addressed only when they are backed
by public behavior, oracle comparison, or a concrete regression. The local
checklist is
`docs/release/checklists/post-v0.15.0-quality-closeout.md`.

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
