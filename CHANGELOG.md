# Changelog

## 0.15.0-design - Unreleased

Current `main` is now the `0.15.0-design - Unreleased` line after the
published `v0.14.0` stable release. `v0.14.0` is the current stable release,
and `0.14.1` is reserved for emergency patch fixes only.

- No `0.15.0` public C API, public ABI, exported-symbol, enum numeric,
  public diagnostics, runtime clock hook, or result-code numeric change has
  been accepted.
- Package-manager recipes, formal Python/Rust publication, cloud/remote
  calculation, Pro/cloud equivalence claims, hard real-time guarantees,
  runtime clock public hooks, public interrupt diagnostics, tracking sequence
  interruption, formal global optimality proof, and upstream baseline upgrades
  remain deferred unless separately accepted.
- `v0.14.0` release evidence, annotated tag evidence, GitHub Release
  publication, and post-tag ordinary push CI are recorded in
  `docs/release/checklists/0.14.0.md`.
- `0.15.0-alpha.1` adds a post-release interrupt quality baseline selector,
  `--interrupt-post-release-quality`, with CTest
  `ruckig_c_interrupt_post_release_quality`. It covers the stabilized
  `v0.14.0` waypoint true-resume, no-waypoint complete-trajectory-boundary,
  and Optimized online tracking candidate-boundary interrupt surfaces while
  keeping public `ruckig_calculate`, tracking sequence interruption, public
  ABI, version metadata, tags, releases, pushes, and manual workflows out of
  scope.
- `0.15.0-alpha.2` adds the docs-only
  `docs/design/tracking_sequence_interruption_api.md` draft. It records why
  `ruckig_tracking_calculate_sequence` interruption needs a public carrier
  decision, lists candidate API shapes and risks, and rejects implementation,
  public header changes, ABI expansion, tags, releases, pushes, and manual
  workflows for this slice.

## 0.14.0 - 2026-06-12

`0.14.0` is the stable API-neutral interrupt surface release. It keeps the
`v0.13.0` 172-symbol public C ABI unchanged while stabilizing the
`0.14.0-alpha.1` through `0.14.0-alpha.5` interrupt evidence reviewed during
`0.14.0-readiness`.

Release focus:

- Waypoint `ruckig_update` true-resume behavior remains the existing private
  waypoint engine behavior from the `v0.12.0`/`v0.13.0` line.
- No-waypoint `ruckig_update` now supports complete-trajectory-boundary
  interruption through the existing interrupt field and output flag, without
  no-waypoint true-resume or public API expansion.
- Optimized online tracking update and lookahead update now support
  best-so-far complete-candidate-boundary interruption through the existing
  interrupt field and output flag.
- Public `ruckig_calculate` remains complete and ignores interruption budgets.
- `ruckig_tracking_calculate_sequence` remains deferred.
- No `0.14.0` public C API, public ABI, exported-symbol, enum numeric, public
  diagnostics, runtime clock hook, or result-code numeric change is part of
  the release.
- Package-manager recipes, formal Python/Rust publication, cloud/remote
  calculation, Pro/cloud equivalence claims, hard real-time guarantees,
  runtime clock public hooks, public interrupt diagnostics, tracking sequence
  interruption, formal global optimality proof, and upstream baseline upgrades
  remain deferred unless separately accepted.

Release evidence:

- `0.14.0-alpha.1` local evidence adds the API-neutral
  `--interrupt-boundary-audit` selector and `ruckig_c_interrupt_boundary_audit`
  CTest. The audit proved the then-active boundary:
  `interrupt_calculation_duration` only enabled waypoint `ruckig_update` soft
  interruption when intermediate waypoints were present, while public
  `ruckig_calculate`, no-waypoint `ruckig_update`, and tracking remained
  outside waypoint resume semantics. No public ABI, version, tag, GitHub
  Release, remote CI, or manual `release-random` action is added by that slice.
- `0.14.0-alpha.2` local design evidence adds
  `docs/design/future_interrupt_surfaces.md`, a quasi-spec for possible future
  no-waypoint complete-trajectory-boundary interruption and online tracking
  candidate-boundary interruption. It is design-only, keeps
  `ruckig_tracking_calculate_sequence` deferred, and does not implement or
  approve any new runtime behavior, public ABI, version, tag, GitHub Release,
  remote CI, or manual `release-random` action. Alpha.1 and alpha.2 were later
  covered together by ordinary remote push CI on head commit `ea06684`, run
  `27387177406`, conclusion `success`.
- `0.14.0-alpha.3` implementation-readiness evidence records the gap audit for
  moving the future interrupt surfaces into conditional API-neutral
  implementation slices. It approves no-waypoint complete-trajectory-boundary
  interruption and online tracking best-so-far candidate-boundary interruption
  for later local alpha slices if their gates pass, while keeping
  `ruckig_tracking_calculate_sequence`, public diagnostics, public ABI,
  version, tag, GitHub Release, and manual `release-random` actions out of
  scope.
- `0.14.0-alpha.4` local implementation evidence adds no-waypoint
  `ruckig_update` complete-trajectory-boundary interruption through the
  existing interrupt field. It uses private preallocated scratch trajectory
  storage, preserves valid no-waypoint incumbents when budget expires, avoids
  no-waypoint true-resume state, keeps public `ruckig_calculate` complete, and
  keeps tracking isolated for the later alpha.5 tracking interruption slice.
- `0.14.0-alpha.5` local implementation evidence adds Optimized-mode online
  tracking candidate-boundary interruption for `ruckig_tracking_update` and
  `ruckig_tracking_update_with_lookahead`. It publishes the best complete
  candidate evaluated so far when budget expires, keeps Fast mode
  single-candidate behavior unchanged, leaves
  `ruckig_tracking_calculate_sequence` deferred, and does not add public
  diagnostics or public ABI.
- `0.14.0-alpha.3` through `0.14.0-alpha.5` were covered together by ordinary
  remote push CI on head commit `4e0e2fb`, run `27391043296`, conclusion
  `success`.
- `0.14.0-readiness` local stable-review audit evidence records full local
  build, CTest, oracle, release-random, performance, ABI/export, platform
  clock, visualization, wrapper, coverage, and boundary gates for alpha.1
  through alpha.5. This readiness slice does not bump version, create a tag,
  publish a GitHub Release, push, or trigger manual `release-random`.
- `0.14.0-readiness` ordinary remote push CI succeeded on commit
  `85b48b86db8a97f1284a6868501b1c72a06db6d9`, run `27393309247`.
- `v0.14.0` stable closeout moves ABI artifact output paths to
  `artifacts/abi/0.14.0`, keeps public additions/removals at `0`, and records
  local release gates, ordinary CI, manual release-random workflow evidence,
  annotated tag publication, and GitHub Release publication in
  `docs/release/checklists/0.14.0.md`.

## 0.13.0 - 2026-06-11

`0.13.0` is the stable waypoint true-resume stress and private engine rewrite
release. It keeps the `v0.12.0` 172-symbol public C ABI unchanged while
stabilizing the post-`v0.12.0` waypoint `ruckig_update` soft-interruption
stress, quality, and private engine hardening reviewed during
`0.13.0-readiness`.

Release focus:

- Waypoint `ruckig_update` soft-interruption true-resume stress coverage for
  multi-DoF, multi-waypoint, per-section, budget-matrix, fresh full-solve
  reference, long online-loop, and allocation-guarded behavior.
- Private waypoint optimizer/resume state rewritten into a single internal
  waypoint engine.
- Transaction-style background publishing that writes a scratch trajectory
  first and only replaces the incumbent with a complete valid improvement.
- A 128-case deterministic quality baseline proving complete waypoint solve
  durations are not regressed against the prior `9d322ad` private engine
  behavior within tolerance.
- No public C ABI expansion, no new exported symbols, and no changes to enum
  numeric values or result-code numeric values.
- No change to public `ruckig_calculate`, no-waypoint target solving, or
  tracking behavior when the interrupt field is set.

Release boundary:

- `v0.13.0` became the current stable release at `0.13.0` closeout.
- Public C ABI expansion is not part of `0.13.0`.
- `0.13.1` is reserved for emergency patch fixes only.
- `0.12.1` remains reserved for emergency `v0.12.0` patch fixes only.
- Package-manager recipes, formal Python/Rust publication, cloud/remote
  calculation, Pro/cloud equivalence claims, hard real-time guarantees,
  runtime clock public hooks, no-waypoint interruption, tracking interruption,
  formal global optimality proof, and upstream baseline upgrades remain
  deferred unless separately accepted.

`0.13.0-readiness` release readiness audit evidence:

- Records full local readiness evidence for deciding whether the post-`v0.12.0`
  waypoint true-resume stress and private engine rewrite evidence is ready for
  the completed `v0.13.0` stable closeout.
- Includes `0.13.0-alpha.1` remote push CI success on commit `9d322ad` and
  `0.13.0-alpha.2` remote push CI success on commit `6354c41`, run
  `27330887817`.
- Reruns local build, routine CTest, duration-enabled CTest, 100k oracle
  seeds, local 1M release-random, no-waypoint performance threshold,
  waypoint performance trend, ABI/export, platform clock probes,
  visualization, Python/Rust wrapper smoke, coverage, and boundary gates.
- Keeps readiness evidence-only: no version bump, tag, GitHub Release,
  manual release-random workflow, public C ABI change, workflow change,
  package-manager work, or `original/ruckig-main` change.

`v0.13.0` stable closeout:

- Moves ABI artifact output paths to `artifacts/abi/0.13.0`.
- Keeps the public C ABI unchanged at 172 symbols, with public additions `0`,
  public removals `0`, and unapproved exported symbols `0`.
- Records full local release gates, ordinary CI, manual release-random
  workflow evidence, annotated tag publication, and GitHub Release
  publication in `docs/release/checklists/0.13.0.md`.

`0.13.0-alpha.2` local evidence slice:

- Rewrites the private waypoint optimizer/resume state into an internal
  `waypoint_engine` structure while keeping the public C ABI, exported-symbol
  set, no-waypoint update, tracking, runtime clock public hook,
  package-manager, and Cloud/Pro boundaries unchanged.
- Adds a transaction-style publish boundary for waypoint true-resume so
  background optimization writes a scratch trajectory first and only copies a
  complete valid improvement into the output trajectory.
- Adds `--waypoint-resume-quality-audit` and
  `ruckig_c_waypoint_resume_quality_audit`, with a 128-case deterministic
  waypoint corpus captured against commit `9d322ad`.
- The quality audit asserts complete waypoint solves are not slower than the
  checked-in baseline within tolerance, and records resume publish,
  interrupted-without-publish, completion, and fresh full-solve reference
  evidence.
- The implementation slice was local-only; ordinary remote push CI evidence
  was later collected on commit `6354c41`, run `27330887817`, before
  `0.13.0-readiness`.

`0.13.0-alpha.1` local evidence slice:

- Adds a focused `--waypoint-resume-stress` C test selector and
  `ruckig_c_waypoint_resume_stress` CTest entry for post-`v0.12.0` waypoint
  true-resume stress and quality audit.
- Covers multi-DoF, multi-waypoint online resume with per-section velocity,
  acceleration, jerk, position, and minimum-duration constraints.
- Exercises zero, tiny, large, changed, and cleared interrupt budgets;
  background interrupted-without-publish behavior; background publish
  incumbent-improvement semantics; fresh full-solve quality references; and
  allocation-guarded resume paths.
- Keeps the `v0.12.0` public C ABI, exported-symbol set, no-waypoint update,
  tracking, runtime clock public hook, package-manager, and Cloud/Pro
  boundaries unchanged.
- Ordinary remote push CI evidence for commit `9d322ad` succeeded; no
  `v0.13.0*` tag, GitHub Release, or manual `release-random` workflow was
  created for the alpha.1 evidence slice.

## 0.12.0 - 2026-06-11

`0.12.0` is the stable waypoint soft-interruption true-resume release. It keeps
the `v0.9.0` 172-symbol public C ABI unchanged while stabilizing the
`0.12.0-alpha.1` true-resume and `0.12.0-alpha.2` unified-engine hardening
slices reviewed during `0.12.0-readiness`.

Release focus:

- Waypoint `ruckig_update` soft-interruption true-resume through the existing
  `interrupt_calculation_duration` input field and
  `ruckig_output_was_calculation_interrupted` output getter.
- Background continuation of interrupted waypoint optimizer work on later
  normal `pass_to_input` online cycles while the interrupt field remains
  enabled.
- Complete-candidate-boundary publish semantics: background resume only
  replaces the incumbent trajectory when a complete feasible candidate improves
  over remaining duration.
- A unified private step-driven waypoint optimizer engine shared by complete
  waypoint solves and soft-interruption resume.
- No public C ABI expansion, no new exported symbols, and no changes to enum
  numeric values or result-code numeric values.
- No change to public `ruckig_calculate`, no-waypoint target solving, or
  tracking behavior when the interrupt field is set.

`0.12.0-readiness` release readiness audit evidence:

- Records full local readiness evidence for deciding whether waypoint
  soft-interruption true-resume and the unified waypoint optimizer engine can
  enter stable closeout.
- Reruns local build, duration-enabled build, routine CTest, focused waypoint
  and platform-clock gates, oracle, local 1M release-random, performance,
  ABI/export, visualization verifier, Python smoke, Rust smoke, wrapper
  examples, coverage, and boundary diff gates.
- Confirms the public C ABI remains at 172 symbols with public additions `0`,
  public removals `0`, and unapproved exported symbols `0`.
- Keeps readiness evidence-only: no version bump, tag, GitHub Release, manual
  release-random workflow, public C API, public symbol, workflow, package, or
  Cloud/Pro runtime change is added.
- Readiness conclusion: ordinary push CI succeeded for the evidence commit, so
  the line entered `v0.12.0` stable closeout.

`v0.12.0` stable closeout:

- Stabilizes waypoint `ruckig_update` soft-interruption true-resume and
  background publish semantics at safe complete waypoint candidate boundaries.
- Stabilizes the unified waypoint optimizer engine for complete waypoint
  solves and soft-interruption resume.
- Moves ABI artifact output paths to `artifacts/abi/0.12.0`.
- Keeps the public C ABI unchanged at 172 symbols, with public additions `0`,
  public removals `0`, and unapproved exported symbols `0`.
- Records full local release gates, ordinary CI, manual release-random
  workflow evidence, annotated tag publication, and GitHub Release
  publication.

Release boundary:

- `v0.12.0` is the current stable release.
- Public C ABI expansion is not part of `0.12.0`.
- `0.12.1` is reserved for emergency patch fixes only.
- Package-manager recipes, formal Python/Rust publication, cloud/remote
  calculation, Pro/cloud equivalence claims, formal global optimality proof,
  hard real-time guarantees, runtime clock public hooks, no-waypoint
  interruption, tracking interruption, and upstream baseline upgrades remain
  deferred unless separately accepted.

## 0.11.0 - 2026-06-10

`0.11.0` is the stable waypoint soft-interruption and platform-clock evidence
release. It keeps the `v0.9.0` 172-symbol public C ABI unchanged while
stabilizing V1 soft interruption for waypoint `ruckig_update` new-trajectory
calculations and the internal platform clock abstraction reviewed during
`0.11.0-readiness`.

Release focus:

- Waypoint `ruckig_update` soft interruption through the existing
  `interrupt_calculation_duration` input field and
  `ruckig_output_was_calculation_interrupted` output getter.
- Internal monotonic platform clock abstraction with Windows/POSIX defaults
  and compile-time custom provider hooks for embedded/RTOS ports.
- No public C ABI expansion, no new exported symbols, and no changes to enum
  numeric values or result-code numeric values.
- No change to public `ruckig_calculate`, no-waypoint target solving, or
  tracking behavior when the interrupt field is set.

`0.11.0-readiness` release readiness audit evidence:

- Records full local readiness evidence for deciding whether waypoint
  soft-interruption V1 and the internal platform clock abstraction can enter
  stable closeout.
- Reruns local build, duration-enabled build, routine CTest, focused waypoint
  and platform-clock gates, oracle, performance, ABI/export, visualization
  verifier, Python smoke, Rust smoke, wrapper examples, coverage, and boundary
  diff gates.
- Confirms the public C ABI remains at 172 symbols with public additions `0`,
  public removals `0`, and unapproved exported symbols `0`.
- Keeps readiness evidence-only: no version bump, tag, GitHub Release, manual
  release-random workflow, public C API, public symbol, workflow, package, or
  Cloud/Pro runtime change is added.
- Readiness conclusion: ordinary push CI succeeded for the evidence commit, so
  the line entered `v0.11.0` stable closeout.

`v0.11.0` stable closeout:

- Stabilizes waypoint `ruckig_update` soft-interruption V1 at safe complete
  waypoint candidate boundaries.
- Stabilizes the internal platform clock abstraction for hosted Windows/POSIX
  builds and compile-time custom monotonic clock providers.
- Moves ABI artifact output paths to `artifacts/abi/0.11.0`.
- Keeps the public C ABI unchanged at 172 symbols, with public additions `0`,
  public removals `0`, and unapproved exported symbols `0`.
- Records full local release gates, ordinary CI, manual release-random
  workflow evidence, annotated tag publication, and GitHub Release
  publication.

Release boundary:

- `v0.11.0` is the current stable release.
- Public C ABI expansion was not part of `v0.11.0`.
- `0.11.1` remains reserved for emergency patch fixes only.
- Package-manager recipes, formal Python/Rust publication, cloud/remote
  calculation, Pro/cloud equivalence claims, formal global optimality proof,
  and upstream baseline upgrades remain deferred unless separately accepted.
- Cross-cycle waypoint continuation, no-waypoint interruption, tracking
  interruption, runtime platform clock setters, hard real-time guarantees, and
  proprietary output equivalence claims remain deferred unless separately
  accepted.

## 0.10.0 - 2026-06-09

`0.10.0` is the stable Visualization v2 evidence release. It keeps the
`v0.9.0` 172-symbol public C ABI unchanged while stabilizing the 30-PNG local
Visualization v2 gallery, local verifier, strict regeneration evidence, and
manual-only CI artifact workflow reviewed during `0.10.0-readiness`.

Release focus:

- Visualization v2 gallery stabilization without public C ABI expansion.
- Local Matplotlib `Agg` and NumPy PNG evidence generated from public C ABI
  data through the Python `cffi` prototype.
- Manual-only Visualization v2 CI artifact evidence, without making
  plotting/verifier jobs default push or pull-request gates.

`0.10.0-alpha` visualization v2 gallery evidence:

- Replaces the current `main` visualization gallery with a 30-PNG Matplotlib
  `Agg` and NumPy gallery under `docs/assets/visualization/`.
- Expands the gallery from the original example mapping set to tracking
  diagnostics, waypoint diagnostics, trajectory anatomy, and cross-topic
  summary plots.
- Updates the manifest label to `0.10.0-alpha visualization v2 evidence`,
  records deterministic categories, metrics, byte counts, and SHA-256 hashes,
  and keeps local paths/timestamps out of committed evidence.
- Updates the local verifier for the canonical 30 PNG files, `1400x900`
  dimensions, boundary flags, and optional strict regeneration.
- Keeps the work local-only and PNG-only: no public C ABI expansion, no default
  CI plotting gate, no CI artifact upload, no package publication, and no
  Pro/cloud equivalence claim.
- The previous v1 gallery provenance remains available through the `v0.9.0`
  tag rather than being duplicated on `main`.

`0.10.0-alpha.2` optional Visualization v2 CI artifact evidence:

- Adds a manual-only `visualization_artifacts` workflow input to generate and
  upload the Visualization v2 gallery as a CI artifact.
- Keeps push and pull-request CI unchanged: the gallery artifact job runs only
  for `workflow_dispatch` with `visualization_artifacts=true`.
- The manual artifact job builds a shared library on Ubuntu, installs the
  existing visualization requirements, regenerates the 30 PNG files and
  `manifest.json`, runs default verification, runs strict regeneration, and
  uploads the regenerated gallery plus logs.
- Keeps the committed gallery assets unchanged. The uploaded artifact is a
  regenerated review artifact, not a tracked replacement.
- Does not add public C API, public symbols, CMake/CTest visualization gates,
  package publication, release tags, or GitHub Releases.

`0.10.0-readiness` release readiness audit evidence:

- Records full local readiness evidence for deciding whether the current
  Visualization v2 gallery, verifier, and manual-only CI artifact path can
  enter stable closeout.
- Reruns local build, visualization verifier, strict regeneration, routine
  CTest, performance, ABI/export, Python smoke, Rust smoke, wrapper examples,
  optional manual visualization artifact workflow, and boundary diff gates.
- Confirms the committed 30-PNG gallery and `manifest.json` remain unchanged;
  the existing manifest label remains `0.10.0-alpha visualization v2 evidence`
  because readiness does not relabel or regenerate tracked assets.
- Keeps readiness evidence-only: no version bump, tag, GitHub Release, manual
  release-random workflow, public C API, public symbol, CMake gate, or default
  visualization CI gate is added.
- Readiness conclusion: ordinary push CI succeeded for the evidence commit, so
  the line entered `v0.10.0` stable closeout.

`v0.10.0` stable closeout:

- Stabilizes the existing 30 committed `1400x900` PNG files and
  `docs/assets/visualization/manifest.json` byte-for-byte; the manifest label
  remains `0.10.0-alpha visualization v2 evidence` as asset provenance.
- Moves ABI artifact output paths to `artifacts/abi/0.10.0`.
- Keeps the public C ABI unchanged at 172 symbols, with public additions `0`,
  public removals `0`, and unapproved exported symbols `0`.
- Records full local release gates, ordinary CI, manual release-random
  workflow evidence, manual Visualization v2 artifact workflow evidence,
  annotated tag publication, and GitHub Release publication.

Release boundary:

- `v0.10.0` is the current stable release.
- Public C ABI expansion was not part of `v0.10.0`.
- `0.10.1` remains reserved for emergency patch fixes only.
- Package-manager recipes, formal Python/Rust publication, cloud/remote
  calculation, Pro/cloud equivalence claims, formal global optimality proof,
  soft interruption checkpoints, and upstream baseline upgrades remain deferred
  unless separately accepted.

## 0.9.0 - 2026-06-09

`0.9.0` is the stable tracking quality and stability evidence release. It
keeps the `v0.8.0` 172-symbol public C ABI unchanged while stabilizing the
tracking random audit, tuned evaluator hardening, and fixed stability
regression evidence reviewed during `0.9.0-readiness`.

Release focus:

- Tracking quality and stability hardening without default public C ABI
  expansion.
- Deeper fixed quality cases, deterministic stress evidence, fallback
  diagnostics review, performance evidence, and no-allocation coverage for the
  existing tracking surface.

`0.9.0-alpha` tracking quality baseline evidence:

- Added `ruckig_c_tests --tracking-random-audit N --seed S`, a deterministic
  test-runner-only audit selector for Optimized tracking fallback and
  diagnostics evidence.
- The audit prints fixed text tables for overall behavior, strategy, DoF,
  signal, lookahead, reactiveness, disabled-DoF, and constraint-profile
  buckets, plus representative fallback cases with diagnostics and
  candidate-family counters.
- Added a lightweight routine CTest, `ruckig_c_tracking_random_audit`, using
  `--tracking-random-audit 10000 --seed 1`.
- Added fixed representative tracking audit C cases selected from
  `--tracking-random-audit 100000 --seed 1`; they verify diagnostics and
  constraints consistency without requiring quality improvement over Fast.
- Kept `--tracking-random` output unchanged and kept public C ABI unchanged at
  172 symbols.
- Recorded local 10k, 100k seed `1/2/41`, and manual 1M seed `1` audit
  evidence in `docs/current/tracking_quality_audit.md` and
  `docs/release/checklists/0.9.0-alpha.md`.

`0.9.0-alpha.2` tracking Optimized evaluator quality hardening evidence:

- Tuned the bounded local Optimized tracking evaluator after the alpha baseline
  audit, without adding public C API, public symbols, enum values, or
  result-code values.
- Added private candidate-family attribution counters on the opaque tracking
  handle for internal evidence only; these counters are not exported and do not
  enter the public diagnostics struct.
- Kept Stable and Balanced on strict tuned-evaluator-score improvement
  semantics, and added a stricter Aggressive near-tie path that requires the
  candidate to be within `1%` of the Fast score and to reduce terminal position
  error by at least half.
- Replaced the alpha audit table with threshold evidence that prints alpha.1
  baseline, required threshold, current value, and pass/fail per strategy.
- Added `ruckig_c_tests --tracking-quality-hardening` and the routine CTest
  `ruckig_c_tracking_quality_hardening`.
- Passed the hard per-strategy 10k, 100k seed `1/2/41`, and 1M seed `1`
  optimized-count and average-improvement thresholds.

`0.9.0-alpha.3` tracking stability regression evidence:

- Added `ruckig_c_tests --tracking-stability`, a fixed regression selector that
  freezes representative `0.9.0-alpha.2` tuned Optimized tracking behavior.
- Added routine CTest `ruckig_c_tracking_stability`.
- Locked fixed cases for optimized strict selection, Aggressive near-tie
  selection, Fast fallback, budget exhaustion, disabled DoF, `tight_valid`
  constraints, all strategies, all signals, all lookahead buckets, and all
  selected candidate families exposed by the alpha.2 audit.
- Kept evaluator scoring, candidate family generation, strategy weights,
  near-tie policy, public diagnostics structs, and public C ABI unchanged.

`0.9.0-readiness` evidence audit and stable closeout:

- Recorded full local release-readiness evidence for deciding whether the
  `0.9.0-design` tracking quality/stability line can enter a later `v0.9.0`
  stable closeout.
- Re-ran static/shared/oracle/performance builds, static and shared routine
  CTest, focused tracking gates, fixed and random oracle gates, local
  release-random evidence, tracking 10k/100k/1M quality audits, performance,
  coverage, ABI/export, and Python/Rust wrapper smoke.
- Confirmed the public C ABI remains unchanged at 172 symbols, with no public
  C API additions, no public removals, no enum/result-code numeric changes, no
  ABI allowlist changes, and no `include/ruckig_c/ruckig.h` changes.
- Readiness conclusion: if the ordinary push CI for the readiness evidence
  commit is green, the next separate project step can be `v0.9.0` stable
  closeout.
- No version bump, tag, GitHub Release, manual release-random workflow, CI
  workflow change, or `original/ruckig-main` change is made by this readiness
  audit.
- Stable closeout promotes project version and version macros to `0.9.0`,
  moves ABI artifact output paths to `artifacts/abi/0.9.0`, creates stable
  release notes and checklist evidence, and keeps public additions/removals at
  `0`.

Deferred unless separately accepted:

- Visualization v2, optional CI gallery artifacts, Python wheel publication,
  Rust crate publication, package-manager recipes, soft interruption
  checkpoints, cloud/remote calculation, formal Pro/cloud equivalence claims,
  formal global optimality proof, and upstream baseline upgrade.
- `0.9.1` remains reserved for emergency patch work only.

## 0.8.0 - 2026-06-09

`0.8.0` is the stable visualization/gallery evidence release. It keeps the
`v0.7.0` 172-symbol public C ABI unchanged while adopting the local
Matplotlib/NumPy PNG gallery and verifier evidence reviewed during
`0.8.0-readiness`. The release does not add solver public API, does not
relabel or regenerate the committed gallery assets, and does not add a default
CI plotting gate. `0.8.1` is reserved for emergency patch work only.

`0.8.0-alpha` visualization evidence:

- Added `tools/visualization/generate_gallery.py`, a local PNG gallery
  generator that uses the Python `cffi` prototype to sample public C ABI data.
- Added generated `ruckig_c`-owned gallery assets under
  `docs/assets/visualization/` with a deterministic `manifest.json`.
- Covered no-waypoint position, velocity-control, stop, minimum-duration,
  local waypoint section, and Fast vs bounded Optimized tracking scenarios.
- Added `docs/current/visualization.md` and
  `docs/release/checklists/0.8.0-alpha.md` to record the local generation
  command, asset inventory, dependency boundary, and deferred follow-up work.
- Kept visualization local-only: no CI gate, no public C API or symbol change,
  no Python/Rust publication, no copied original images, and no Pro/cloud
  equivalence claim.

`0.8.0-alpha.2` Matplotlib visualization gallery evidence:

- Replaced the first Pillow-only six-image gallery with a NumPy and Matplotlib
  `Agg` renderer.
- Expanded the generated PNG gallery to local C ABI equivalents of original
  examples `01-10` and `14-16`, excluding `11-13` because they are C++ Eigen
  and custom-vector ergonomics examples rather than C ABI visualization
  behavior.
- Added `tools/visualization/requirements.txt` for the optional local plotting
  environment and kept `_local/visualization-venv` outside tracked files.
- Updated `docs/current/visualization.md` and added
  `docs/release/checklists/0.8.0-alpha.2.md` to record the generation command,
  original example mapping, asset manifest, local dependency boundary, ABI
  unchanged evidence, and deferred items.
- Kept the gallery PNG-only and local-only: no CI workflow, no tag, no GitHub
  Release, no public C API or symbol change, no package publication, no copied
  original image/PDF assets, and no Pro/cloud equivalence claim.

`0.8.0-alpha.3` visualization verifier evidence:

- Added `tools/visualization/verify_gallery.py`, a local-only verifier for the
  committed Matplotlib PNG gallery and manifest.
- The default verifier checks the canonical 13 PNG list, PNG header dimensions,
  manifest byte counts and SHA-256 hashes, original example mappings,
  `11-13` exclusions, boundary flags, and absence of local paths or timestamp
  fields.
- Added optional `--strict-regenerate` mode to regenerate the gallery into an
  ignored `out/` directory and compare the regenerated PNGs and manifest with
  committed assets.
- Kept alpha.2 gallery assets and `manifest.json` unchanged: alpha.3 verifies
  them rather than relabeling or replacing them.
- Kept verification local-only: no CI workflow, no CMake/CTest change, no tag,
  no GitHub Release, no manual release-random workflow, no public C API or ABI
  change, and no `original/ruckig-main` change.

`0.8.0-readiness` evidence audit:

- Recorded focused local release-readiness evidence for deciding whether the
  current visualization gallery and verifier work can enter a later `v0.8.0`
  stable closeout.
- Re-ran static/shared builds, static and shared routine CTest, tracking and
  solver-branch CTest subsets, visualization verifier default and strict
  regeneration, ABI/export checks, Python prototype smoke, and Rust alpha
  wrapper smoke.
- Confirmed `0.8.0-readiness` does not change public C API, public symbols,
  enum values, result codes, CMake/CTest/CI workflow configuration, gallery
  assets, ABI artifact path, version macros, or `original/ruckig-main`.
- Readiness conclusion: if the ordinary push CI for the readiness evidence
  commit is green, the next separate decision can be `v0.8.0` stable closeout.
- No tag, GitHub Release, manual release-random workflow, version bump,
  package publication, or CI plotting gate is created by this readiness audit.

Stable closeout:

- Stabilizes the 13 committed Matplotlib PNG gallery assets and
  `docs/assets/visualization/manifest.json` as `v0.8.0` documentation evidence
  while retaining their original `0.8.0-alpha.2` provenance label and hashes.
- Stabilizes `tools/visualization/generate_gallery.py`,
  `tools/visualization/verify_gallery.py`, and the optional
  `tools/visualization/requirements.txt` local plotting environment as
  maintainership evidence tools.
- Keeps visualization local-only: no default GitHub Actions plotting or
  verifier job, no CMake/CTest gate, no copied original image/PDF assets, and
  no network, cloud, Pro license, or Pro/cloud equivalence claim.
- Keeps public C ABI unchanged from `v0.7.0`: 172 approved public symbols,
  public additions `0`, public removals `0`, and unchanged enum/result-code
  numeric values.
- ABI artifact paths now use `artifacts/abi/0.8.0` for stable release
  evidence.

Deferred unless separately accepted:

- Python wheel publication, Rust crate publication, package-manager recipes,
  soft interruption checkpoints, cloud/remote calculation, formal Pro/cloud
  equivalence claims, formal global optimality proof, and upstream baseline
  upgrade remain outside the default `0.8.0-design` entry scope.

## 0.7.0 - 2026-06-08

`0.7.0` is the stable Optimized tracking quality and diagnostics release. It
stabilizes the 172-symbol public C ABI reviewed during `0.7.0-readiness`:
the `v0.6.0` bounded local Optimized tracking surface plus high-level
Stable/Balanced/Aggressive strategy presets and the public diagnostics
snapshot getter. `0.7.1` is reserved for emergency patch work only.

Added:

- Stabilized public Optimized tracking strategy presets:
  `RUCKIG_TRACKING_OPTIMIZED_STABLE`,
  `RUCKIG_TRACKING_OPTIMIZED_BALANCED`, and
  `RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE`.
- Stabilized public strategy controls:
  `ruckig_tracking_set_optimized_strategy` and
  `ruckig_tracking_get_optimized_strategy`.
- Stabilized the public diagnostics snapshot:
  `ruckig_tracking_diagnostics_t` and
  `ruckig_tracking_get_last_diagnostics`.
- Kept bounded local Optimized tracking semantics, Fast fallback diagnostics,
  deterministic candidate budgets, local quality gates, and Python/Rust
  prototype smoke evidence.
- ABI artifact paths now use `artifacts/abi/0.7.0` for stable release
  evidence.

Compatibility:

- Existing `v0.6.0` public symbols, function signatures, enum numeric values,
  and result-code numeric values are unchanged.
- Public symbol allowlist: `172` approved public symbols.
- `RUCKIG_TRACKING_OPTIMIZED` remains a bounded deterministic local evaluator,
  not a formal global optimizer and not a Pro/cloud equivalence claim.
- `interrupt_calculation_duration` does not create tracking timeout,
  soft-interruption, or hard real-time guarantees.
- `original/ruckig-main` remains frozen as Ruckig Community `0.17.3`.

Algorithm visualization planning decision:

- Reviewed the frozen original Ruckig documentation and example visualization
  assets: `original/ruckig-main/doc/*.png`,
  `original/ruckig-main/examples/*_trajectory.pdf`, and the original
  `examples/plotter.py` helper.
- Decided not to implement trajectory/gallery generation in the stable
  `v0.7.0` closeout. The release remains limited to the readiness-approved
  172-symbol ABI, version/tag/release flow, and evidence gates.
- Deferred `ruckig_c`-generated trajectory/tracking/waypoint plots to a
  post-`v0.7.0` visualization evidence project. That later project should
  generate `ruckig_c`-owned images from local C/Python prototype data rather
  than copying original images as primary project evidence.
- No plotting dependency, image/PDF artifact, CI job, public C API, public
  symbol, or release gate is added by this release.

`0.7.0-readiness` evidence audit:

- Recorded full local release-readiness evidence for a future `v0.7.0` stable
  closeout without creating a tag, GitHub Release, release-candidate branch, or
  version bump.
- Treated the current 172-symbol public C ABI as the intended future `v0.7.0`
  stable baseline: the `0.7.0-alpha` strategy preset controls and
  `0.7.0-alpha.2` diagnostics snapshot are ready for stable review.
- Re-ran static/shared/performance/oracle builds, static and shared CTest,
  tracking and solver-branch gates, fixed oracle comparisons, random oracle
  seeds `1`, `2`, and `41`, per-DoF random seed `1`, local 1M release-random
  readiness gates, performance benchmarks, coverage, ABI/export checks, and
  Python/Rust wrapper smoke.
- Regenerated local readiness coverage under `out/coverage/0.7.0-readiness/`.
  The broad routine corpus records line `87.71%`, function `91.86%`, branch
  `69.26%`, and region `87.44%`.
- Promoted ABI artifact paths from the design-line
  `artifacts/abi/0.7.0-alpha.2` location to `artifacts/abi/0.7.0` during
  stable closeout.
- No public C API, public symbol, enum numeric value, result-code value,
  `original/ruckig-main` content, package publication, tag, or GitHub Release
  is changed by this readiness evidence.

`0.7.0-alpha.4` solver branch coverage evidence:

- Added targeted solver branch coverage for the five lowest implementation
  files from the `0.7.0-alpha.3` audit:
  `position_second_step2.c`, `velocity_third_step2.c`, `block.c`,
  `velocity_third_step1.c`, and `position_second_step1.c`.
- Added `ruckig_c_tests --solver-branch-coverage` and the
  `ruckig_c_solver_branch_coverage` CTest gate. The test uses internal
  white-box probes only for branch coverage hardening; original parity behavior
  remains backed by frozen C++ oracle comparisons.
- Added six fixed oracle comparison cases for second-order position and
  third-order velocity branch families. The fixed oracle corpus now records 82
  comparisons plus the existing waypoint section oracle comparisons.
- Regenerated local coverage under `out/coverage/0.7.0-alpha.4/`. The broad
  routine corpus records line `87.71%`, function `91.86%`, branch `69.26%`,
  and region `87.44%`.
- Coverage remains local-only evidence and is not a CI coverage job or a hard
  release gate. No public C API, public symbol, enum numeric value, or
  result-code value is changed.

`0.7.0-alpha.3` coverage audit evidence:

- Added a local LLVM coverage build option and
  `windows-clang-ninja-coverage` preset for evidence-only source coverage.
- Added `tools/coverage/run_coverage.ps1`, which configures the coverage
  preset, runs the broad routine C/oracle/example corpus plus 10k random
  supplements, and writes raw LLVM artifacts under
  `out/coverage/0.7.0-alpha.3/`.
- Added `docs/current/test_coverage_audit.md`, mapping the original Community
  `test_target.cpp` cases and examples to current C tests, frozen C++ oracle
  gates, examples, Python prototype smoke, and Rust alpha smoke.
- Recorded local implementation coverage summary for the broad routine corpus:
  line `85.33%`, function `90.33%`, branch `67.03%`, and region `84.89%`.
- Coverage is local-only evidence, not a CI job and not a hard release gate.
  Raw HTML/profile artifacts remain untracked under `out/coverage/`.
- No public C API, public symbol, enum numeric value, or result-code value is
  changed by this alpha evidence.

`0.7.0-alpha.2` hardening evidence:

- Added high-level Optimized tracking strategy presets:
  `RUCKIG_TRACKING_OPTIMIZED_STABLE`,
  `RUCKIG_TRACKING_OPTIMIZED_BALANCED`, and
  `RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE`.
- Added public C strategy controls:
  `ruckig_tracking_set_optimized_strategy` and
  `ruckig_tracking_get_optimized_strategy`. Existing `v0.6.0` public symbols,
  signatures, enum numeric values, and result-code numeric values are
  unchanged.
- Added a public tracking diagnostics snapshot:
  `ruckig_tracking_diagnostics_t` and
  `ruckig_tracking_get_last_diagnostics`. The snapshot covers Fast and
  Optimized online/offline calls, aggregate score fields, candidate-family
  counters, fallback/optimized/error step counts, and budget exhaustion.
- Reworked the bounded Optimized candidate evaluator around deterministic
  strategy configs, strategy-specific scoring weights, explicit candidate
  families, fixed tie-break behavior, and preserved Fast fallback semantics.
- Extended C quality evidence so Balanced must not be worse than Fast on the
  fixed corpus, Balanced must improve by at least `0.5%` on selected smooth
  lookahead cases, and Aggressive must improve over Balanced by at least `2%`
  on fixed oscillatory cases.
- Added deterministic tracking random stress through
  `ruckig_c_tests --tracking-random N --seed S` and routine CTest coverage for
  `--tracking-random 100000 --seed 1`, `--seed 2`, and `--seed 41`.
- Extended Python `cffi` prototype and Rust alpha wrapper smoke with strategy
  preset controls and diagnostics snapshots. No wheel, crate, tag, or GitHub
  Release is produced for this alpha evidence line.

Deferred unless separately accepted:

- Soft interruption implementation, formal Python/Rust publication,
  package-manager recipes, cloud/remote calculation, formal Pro/cloud
  equivalence claims, and upstream baseline upgrade remain outside the default
  `0.7.0-design` entry scope.

## 0.6.0 - 2026-06-07

`0.6.0` is the stable bounded local `Optimized` tracking release. It
stabilizes the public C Optimized tracking API added during `0.6.0-design`,
keeps the existing Fast tracking behavior, and does not claim formal global
optimality or Pro/cloud numerical equivalence. `0.6.1` is reserved for
emergency patch work only.

Added:

- Added a local `RUCKIG_TRACKING_OPTIMIZED` MVP with bounded
  deterministic candidate search, horizon-error scoring, and Fast fallback.
- Added online lookahead tracking through
  `ruckig_tracking_update_with_lookahead`.
- Added Optimized tracking candidate-budget and diagnostic public C APIs:
  `ruckig_tracking_set_max_optimized_candidates`,
  `ruckig_tracking_get_max_optimized_candidates`,
  `ruckig_tracking_get_last_calculation_status`, and
  `ruckig_tracking_get_last_candidate_count`.
- Added C tests for Optimized API lifecycle, validation, online lookahead,
  offline sliding-window calculation, quality dominance against Fast baseline,
  fallback diagnostics, and no-allocation prepared paths.
- Added Optimized tracking C examples and Python/Rust prototype smoke coverage.

Changed:

- `ruckig_tracking_update` now supports `RUCKIG_TRACKING_OPTIMIZED` with a
  single-sample lookahead instead of returning `RUCKIG_ERROR_UNSUPPORTED`.
- `ruckig_tracking_calculate_sequence` now uses sliding-window lookahead in
  Optimized mode.
- ABI artifact paths now use `artifacts/abi/0.6.0` for stable release
  evidence.

Still deferred:

- Pro/cloud numerical equivalence claims, cloud/remote calculation, soft
  interruption checkpoints, formal Python/Rust publication, package-manager
  recipes, and upstream baseline upgrade remain separate projects unless
  explicitly accepted.

## 0.5.0 - 2026-06-07

`0.5.0` is the stable tracking Fast-mode release. It stabilizes the public C
tracking ABI added during `0.5.0-design`, includes local online/offline Fast
tracking, and keeps `RUCKIG_TRACKING_OPTIMIZED` declared but unsupported until
the separate `0.6.0-design` implementation work.

Added:

- Added the accepted tracking public C ABI and first local
  implementation surface: opaque tracking handles, target-state handles,
  target-state sequences, tracking output sequences, online tracking update,
  and offline sequence calculation.
- Added local Fast-mode tracking using deterministic constant-acceleration
  lookahead prediction with default `look_ahead_cycles = 1`.
- Added C tracking tests for API lifecycle, validation, online update, offline
  sequence calculation, quality smoke, and no-allocation paths.
- Added `0.5.0-alpha.2` tracking hardening coverage: a deterministic C fixed
  corpus, tuned ramp and constant-acceleration quality gates, multi-DoF C
  no-allocation coverage, and stronger Python/Rust tracking smoke.
- Added C tracking examples for online ramp tracking, online
  constant-acceleration tracking, and offline sequence tracking.
- Extended the Python `cffi` prototype and Rust alpha wrapper with tracking
  smoke coverage.

Changed:

- ABI artifact output paths now use `artifacts/abi/0.5.0` for the stable
  tracking release evidence line.
- `docs/abi/public-symbols.txt` and public ABI exception approvals now include
  the intentional `0.5.0-design` tracking public symbols.
- `v0.5.0` becomes the current stable release. `v0.4.2` remains the
  original-parity coverage/evidence baseline before tracking stabilization.
- `Optimized` tracking implementation is tracked for `0.6.0-design`, not
  `0.5.x`.

Still deferred:

- Tracking `Optimized` mode implementation; the enum is declared but calls
  return `RUCKIG_ERROR_UNSUPPORTED` and implementation is deferred to
  `0.6.0-design`.
- Soft interruption checkpoints, released Python wheels, Rust crate
  publication, package-manager recipes, cloud/remote calculation, formal
  Pro/cloud numerical equivalence claims, and upstream baseline upgrades.

## 0.4.2 - 2026-06-06

`0.4.2` is an original-parity coverage and evidence closeout release for the
`0.4.x` line. It does not add public C API, does not change existing public
function signatures, does not change enum or result-code numeric values, and
does not update the frozen upstream oracle baseline.

Added:

- Added an original parity coverage matrix that separates no-waypoint target
  solver coverage, local waypoint optimizer coverage, trajectory semantics,
  bindings, packaging, cloud/Pro gaps, tracking, and C++-specific ergonomics.
- Added tracking interface design preparation for the future `0.5.0-design`
  line. Tracking is now recorded as a required full-original-parity gap, but no
  tracking public API or implementation is added in `0.4.2`.
- Added an `interrupt_calculation_duration` design note that documents the
  storage-only behavior in `0.4.2` and the future soft-interruption semantics
  that must be designed before implementation.
- Added `0.4.2` release checklist and release-note source material.

Changed:

- `CMakeLists.txt` and public version macros now point at `0.4.2`.
- ABI artifact output paths now use `artifacts/abi/0.4.2`.
- README, roadmap, API compatibility notes, and documentation index now treat
  `v0.4.2` as the coverage/evidence closeout baseline before
  `0.5.0-design`.

Still deferred:

- Tracking implementation and public tracking C API, soft interruption
  checkpoints, formal cloud/Pro numerical equivalence claims, cloud or remote
  calculation, hard real-time guarantees for waypoint optimization, released
  Python wheels, published Rust crate, package-manager recipes, new public C
  API expansion outside `0.5.0-design`, and upstream baseline upgrades.

## 0.4.1 - 2026-06-06

`0.4.1` is a stabilization and evidence release for the `0.4.x`
original-surface parity line. It does not add public C API, does not change
existing public function signatures, does not change enum or result-code
numeric values, and does not update the frozen upstream oracle baseline.

Added:

- Added deeper waypoint optimizer regression coverage for 1D, 2D, 4D, and 8D
  multi-waypoint cases, including tight per-section bounds, disabled DoFs,
  nonzero boundary derivatives, per-section minimum duration, and global
  position-bound stress.
- Added stronger waypoint trajectory invariant checks for intermediate
  duration ordering, waypoint section sampling, per-section sampled limits,
  position extrema, and first-time-at-position across sections.
- Expanded the local waypoint performance corpus from 5 to 10 deterministic
  case families, now covering up to 8 DoF and 3 intermediate waypoints.
- Strengthened the experimental Python `cffi` prototype smoke tests with a
  four-DoF mixed per-section waypoint scenario.
- Added `0.4.1` release checklist and release-note source material.

Changed:

- `CMakeLists.txt` and public version macros now point at `0.4.1`.
- ABI artifact output paths now use `artifacts/abi/0.4.1`.
- `0.4.1` keeps the `v0.4.0` public C symbol set unchanged; all optimizer
  diagnostics remain internal, test-only, or benchmark-output evidence.
- `interrupt_calculation_duration` remains a storage/API-surface parity field
  in this release. `0.4.1` documents the future soft-interruption design
  boundary but does not implement optimizer interruption checkpoints.

Still deferred:

- Formal cloud/Pro numerical equivalence claims, cloud or remote calculation,
  hard real-time guarantees for waypoint optimization, released Python wheels,
  published Rust crate, package-manager recipes, new public C API expansion,
  and upstream baseline upgrades.

## 0.4.0 - 2026-06-06

`0.4.0` starts the full original-surface parity line after `v0.3.0`. This
release intentionally expands the public C ABI for intermediate waypoints,
per-section constraints, global position bounds, and multi-section trajectory
queries. The waypoint optimizer is local-only; no cloud or remote calculation
client is implemented.

Added:

- Added waypoint-aware constructors for `ruckig_t`, `ruckig_input_t`,
  `ruckig_output_t`, and `ruckig_trajectory_t`.
- Added public C ABI access to global max/min position bounds.
- Added intermediate waypoint set/get/clear APIs.
- Added per-section max/min velocity, max/min acceleration, max jerk,
  max/min position, and per-section minimum-duration APIs.
- Added interrupt-calculation-duration storage APIs for original API surface
  parity; alpha behavior records the value but does not yet guarantee soft
  interruption.
- Added multi-section trajectory metadata APIs for section count and
  intermediate durations.
- Added deterministic local `ruckig_filter_intermediate_positions`.
- Added an experimental local coupled waypoint optimizer. It searches shared
  internal waypoint velocity/acceleration candidates, evaluates each candidate
  through the existing target solver section evaluator, rejects constraint
  violations, explores a deterministic internal branch queue around the best
  candidates, and selects the lowest-duration feasible candidate.
- Added C examples for waypoint offline calculation, waypoint online updates,
  per-section minimum duration, per-section limits, intermediate-position
  filtering, and dynamic DoFs with waypoints.
- Added focused CTest entries for waypoint optimizer, per-section constraints,
  and waypoint quality alpha checks.
- Added a waypoint alpha performance benchmark mode for the local C optimizer
  corpus. It is C-only evidence because Ruckig Community `0.17.3` has no local
  global waypoint optimizer oracle.
- Added CI coverage for `0.4.0` ABI/export artifact paths, Linux
  waypoint alpha performance output, Python prototype smoke, and Rust alpha
  wrapper smoke.
- Extended the experimental Python `cffi` ABI-mode prototype to cover the
  `0.4.0` waypoint-aware C ABI surface.
- Added an experimental Rust alpha wrapper over `ruckig_c` with smoke tests and
  examples for position, offline calculation, velocity, waypoints, and
  per-section minimum duration.
- Strengthened Python and Rust prototype smoke coverage for per-section
  position constraints, interrupt-calculation-duration storage,
  first-time-at-position, intermediate-position readback/filtering, and output
  calculation-state accessors.

Changed:

- `CMakeLists.txt` and public version macros now point at `0.4.0`.
- ABI artifact output paths now use `artifacts/abi/0.4.0-design`.
- `docs/abi/public-symbols.txt` and
  `docs/abi/public-symbol-exceptions.txt` now record the approved `0.4.0`
  public API expansion.
- `bindings/python_prototype/` remains prototype-only; it is not installed,
  packaged, or treated as a stable Python binding API.
- `bindings/rust/` is prototype-only; it is not published as a crate and does
  not wrap original C++ Ruckig.
- No-waypoint target-solver behavior remains on the existing frozen C++ oracle
  path and must not regress.

Still deferred:

- Formal cloud/Pro numerical equivalence claims, hard real-time guarantees for
  waypoint optimization, released Python wheels, published Rust crate,
  package-manager recipes, cloud API support, and upstream baseline upgrades.

## 0.3.0 - 2026-06-05

`0.3.0` is a hardening release. It promotes the completed `0.3.0-design`
engineering work into a versioned release boundary without adding public C API,
changing solver scope, publishing bindings, adding package-manager recipes, or
updating the frozen upstream oracle baseline.

Changed:

- `main` now prepares the `0.3.0` hardening release after publishing `v0.2.5`
  as the final planned `0.2.x` stabilization release.
- ABI comparison baselines now roll forward to `docs/abi/v0.2.5/`; strict
  public ABI diff failure remains opt-in for local builds, while the dedicated
  Linux/Windows exported-symbol CI jobs upload warning/evidence artifacts.
- `0.3.0` release priority starts with ABI/export hygiene and existing
  installed-package consumer paths before binding release work.
- Added `docs/abi/public-symbols.txt` as the approved public C ABI symbol
  allowlist derived from `include/ruckig_c/ruckig.h`.
- Added a public symbol allowlist verification target that extracts
  `RUCKIG_C_API` declarations from the public header and checks the tracked
  allowlist.
- Non-Windows shared builds now hide implementation-internal symbols by
  default and export only declarations marked with `RUCKIG_C_API`.
- Linux shared builds additionally link with a public-symbol version script
  generated from `docs/abi/public-symbols.txt`.
- Added a public exported-symbol comparison target for warning/evidence-only
  strict ABI gate trial artifacts, with the dedicated ABI CI jobs currently
  uploading public diff evidence without yet failing the workflow on drift.
- Added `docs/abi/public-symbol-exceptions.txt` as the explicit approval file
  for intentional future public symbol additions; it is empty by default.
- Added fixed oracle cases for 50s and 100s exact-target first-time boundaries;
  the 100s case is retained with a documented case-specific first-time
  tolerance exception.
- Added and smoke-tested a Python `cffi` ABI-mode prototype workspace against a
  local shared `ruckig_c` build.
- Retained the experimental vcpkg overlay prototype as frozen reference
  evidence after verifying local `x64-windows` shared/default and
  `x64-windows-static` consumer paths.
- Downgraded package-manager recipes and new package-manager prototypes to
  long-term optional work; the existing vcpkg overlay is retained as frozen
  reference evidence outside the active roadmap.
- Added opt-in MSVC `cl` standalone static and DLL consumer CTest gates and
  verified both locally; they remain outside routine CI.
- Added CMake presets that keep routine local builds under `out/build/`, plus
  a dry-run-first local cleanup script for ignored build trees, caches, and
  temporary files.
- Added a Windows-specific `windows-clang-ninja` preset as the default local
  README build path, plus a matching `windows-clang-ninja-shared` preset for
  ABI/export and Python prototype smoke validation.
- Recorded a `0.3.0` hardening pass covering the Windows preset,
  shared DLL/import-library consumer, public symbol allowlist verification,
  public exported-symbol comparison, Python `cffi` prototype smoke, and current
  MinGW/MSVC `cl` toolchain availability.
- Clarified that MSVC `cl` standalone consumer smokes remain opt-in local gates
  rather than routine CI.
- Added MinGW static and DLL/import-library consumer smoke support, verified
  both locally with GCC 15.2.0, and added a dedicated MSYS2 MinGW64 routine CI
  consumer gate.
- Completed the Python prototype design decisions for low-level ABI shape,
  future high-level wrappers, result/error handling, explicit shared-library
  discovery, copy-in/copy-out arrays, and deferred wheel/package strategy.
- Added a `0.3.0` release decision document and release checklist to keep the
  hardening-release scope separate from future feature, packaging, and binding
  projects.

Still deferred:

- Intermediate waypoints, per-section constraints, cloud calculation, Python
  binding release work, Rust bindings, package-manager recipes and new
  package-manager prototypes, strict exported-symbol fail gates, and upstream
  baseline upgrades.

## 0.2.5 - 2026-06-05

`0.2.5` is planned as the final `0.2.x` stabilization release before
`0.3.0-design`.

Added:

- `v0.2.4` Linux and Windows exported-symbol baselines for `0.2.5`
  warning/evidence-only ABI comparison.
- `0.2.5` release checklist with strict ABI gate design fields, consumer
  matrix evidence, performance trend comparison, and targeted oracle regression
  gates.
- `0.3.0` readiness decision document covering the post-`v0.2.5` design entry
  criteria, binding/package-manager priorities, ABI gate status, and deferred
  feature boundaries.
- Targeted fixed C++ oracle regression cases for high-DoF discrete
  minimum-duration synchronization, disabled-DoF online updates, large-duration
  first-time boundaries, and mixed first/second/third-order
  synchronization edges.

Changed:

- `0.2.5` ABI work is still warning/evidence-only by default; strict ABI diff
  enforcement remains a design target until the documented prerequisites are
  satisfied.
- Expanded Windows consumer documentation for existing `clang`/`clang-cl`
  coverage, planned MSVC `cl` standalone static/DLL smoke gates, and MinGW
  feasibility status.
- Expanded package-manager feasibility notes with vcpkg first, Conan second,
  Homebrew third, and FetchContent/vendored subdirectory guidance only.
- Expanded Python binding feasibility notes for the future `cffi` ABI-mode
  prototype without adding binding implementation.

Still deferred:

- Intermediate waypoints, per-section constraints, cloud calculation, Python
  binding implementation, Rust bindings, package-manager recipes, strict ABI
  fail gates, and upstream baseline upgrades.

## 0.2.4 - 2026-06-04

Added:

- `v0.2.3` Linux and Windows exported-symbol baselines for `0.2.4`
  warning/evidence-only ABI comparison.
- Windows `clang-cl` shared C-only CI coverage so the DLL/import-library
  consumer smoke also runs under the MSVC frontend variant.

Changed:

- Windows static and DLL consumer smoke scripts now support both GNU-like
  `clang` and `clang-cl` frontend modes.
- `0.2.4` ABI comparison remains warning/evidence only; it is not a strict CI
  fail gate.

Fixed:

- Windows `clang-cl` manual static consumer smoke now uses the dynamic CRT mode
  expected by the CMake-built static library, avoiding mixed CRT link failures.

Still deferred:

- Intermediate waypoints, per-section constraints, cloud calculation, Python
  binding implementation, Rust bindings, and upstream baseline upgrades.

## 0.2.3 - 2026-06-04

Added:

- Non-strict exported-symbol baseline comparison against the `v0.2.2`
  baseline for Linux and Windows shared builds. The comparison is warning and
  evidence only; it is not yet a strict CI fail gate.
- `0.2.3` release checklist template with ABI baseline comparison, consumer
  matrix, numerical regression, performance trend, and release-random evidence
  fields.
- Package-manager feasibility notes for vcpkg, Conan, Homebrew, FetchContent,
  and vendored subdirectory use. No package-manager recipe is implemented.
- Additional fixed C++ oracle regression cases for higher-DoF per-DoF
  synchronization, disabled DoFs, discrete minimum-duration edge cases, tiny
  nonzero limits with large position magnitude, long online update loops, and
  repeated first-time-at-position boundary queries.

Changed:

- Expanded Python binding feasibility design to select `cffi` ABI mode as the
  default prototype path and document prototype acceptance criteria.
- Clarified that `0.2.3` maintenance keeps `original/ruckig-main` frozen and
  does not add public C API.

Still deferred:

- Intermediate waypoints, per-section constraints, cloud calculation, Python
  binding implementation, Rust bindings, and upstream baseline upgrades.

## 0.2.2 - 2026-06-04

Added:

- `0.2.2` release checklist template with ABI/exported-symbol, consumer
  automation, performance trend, and per-DoF random oracle evidence fields.
- Shared-build exported-symbol evidence target `ruckig_c_exported_symbols`,
  using `nm` on Unix-like systems and `llvm-readobj` or `dumpbin` on Windows.
- GitHub Actions Linux/Windows exported-symbol snapshot job that runs the
  shared-build helper and uploads review artifacts.
- Windows consumer smoke CTest scripts for manual static linking and DLL
  import-library consumption where the release-check toolchain supports them.
- Additional fixed C++ oracle regression cases for 4-6 DoF mixed scenarios,
  long high-frequency online update loops, very small `delta_time` with
  per-DoF mixed synchronization, segment-boundary query coverage, and
  multi-disabled mixed-order inputs.
- Python bindings feasibility design for `0.3.0-design`; this is design-only
  and does not add binding code.
- `0.3.0-design` priority evaluation documenting that `0.2.x` package,
  consumer, ABI, performance, and regression evidence should mature before
  bindings work; Python bindings should be evaluated before Rust bindings once
  prerequisites are met.

Changed:

- Expanded `0.2.x` packaging, ABI, performance, and roadmap documentation for
  repeatable `0.2.2` maintenance evidence collection.

Still deferred:

- Intermediate waypoints, per-section constraints, cloud calculation, Python
  bindings, Rust bindings, and upstream baseline upgrades.

## 0.2.1 - 2026-06-04

Added:

- Routine per-DoF random oracle smoke coverage through
  `ruckig_c_oracle_tests --random-per-dof 100 --seed 1`.
- `0.2.1` release checklist template for patch-release evidence collection.
- Packaging and consumer guidance for installed CMake, pkg-config, Windows
  manual static linking, DLL consumers, and shared install-tree verification.
- API/ABI compatibility policy documentation for `0.2.x` patch releases.

Changed:

- Clarified that `docs/historical/c_rewrite_execution_plan.md` is a historical execution
  plan, while current scope is defined by README, public header, roadmap,
  release checklists, and upstream baseline policy.
- Documented post-`v0.2.0` hardening on `main`, including the fixed oracle suite
  increasing from 48 release-time cases to 59 cases, controlled
  `--random-per-dof N --seed S` stress, per-DoF clear behavior regression,
  per-DoF update recalculation stability regression, and
  `examples/c/08_per_dof_online.c`.
- Added fixed oracle regression coverage for large-magnitude positions,
  tiny nonzero limits, large discrete minimum duration, mixed first/second/third
  order per-DoF inputs, explicit first-time-at-position boundaries, and
  disabled DoF per-DoF overrides under discrete duration.

Still deferred:

- Intermediate waypoints, per-section constraints, cloud calculation, Python
  bindings, Rust bindings, and upstream baseline upgrades.

## 0.2.0 - 2026-06-03

Added:

- Public per-DoF control-interface override setters and clearers:
  `ruckig_input_set_per_dof_control_interface` and
  `ruckig_input_clear_per_dof_control_interface`.
- Public per-DoF synchronization override setters and clearers:
  `ruckig_input_set_per_dof_synchronization` and
  `ruckig_input_clear_per_dof_synchronization`.
- Fixed C++ oracle cases for mixed position/velocity control overrides and
  mixed `Time`/`None` synchronization overrides.
- C API tests for invalid per-DoF setter inputs, clear behavior, update
  recalculation, and the no-allocation runtime contract with per-DoF settings
  enabled.
- Minimal per-DoF override C example wired into CMake and CTest.
- The `0.1.1` stability queue additions are included in this `0.2.0` mainline
  release.

Changed:

- Fixed oracle suite now contains 48 deterministic cases.
- The target calculator dispatch now uses effective per-DoF control and
  synchronization settings when per-DoF vectors are enabled.

Still deferred:

- Intermediate waypoints, per-section constraints, cloud calculation, Python
  bindings, Rust bindings, and upstream baseline upgrades.

## 0.1.1 - Unreleased

This section is retained only for a possible `v0.1` maintenance branch. These
stability changes are already present on `main` through the `0.2.0` mainline
release or later `0.2.x` hardening work.

Added:

- Fixed oracle regression cases for 3 DoF high-frequency online updates,
  near-limit velocity control, very small discrete `delta_time`, mixed
  disabled/active DoFs, discrete minimum duration, and directional lower-limit
  edge values.
- C API diagnostic tests for invalid numerical inputs, zero-limit error paths,
  finite/infinite solver selection semantics, and `Synchronization::None`
  behavior.
- Minimal offline and online C examples wired into CMake and CTest.
- API diagnostics documentation in `docs/current/api_diagnostics.md`.
- Patch-release performance recording procedure in `docs/release/evidence/performance_report.md`.
- Frozen upstream baseline policy in `docs/current/upstream_baseline_policy.md`.
- Per-DoF override design gate in `docs/design/per_dof_overrides.md`.

Changed:

- Fixed oracle suite now contains 44 deterministic cases.
- CMake example tests now include the minimal offline and online examples.

## 0.1.0 - 2026-06-03

Initial public release for the pure C99 rewrite of Ruckig Community `0.17.3`
local state-to-state trajectory generation.

Added:

- Public opaque C ABI in `include/ruckig_c/ruckig.h`.
- Offline `ruckig_calculate` and online `ruckig_update` APIs.
- Position and velocity control for first-, second-, and third-order supported
  local state-to-state trajectories.
- Multi-DoF synchronization modes `Time`, `TimeIfNecessary`, `Phase`, and
  `None`.
- Continuous and discrete duration handling.
- Directional min velocity/min acceleration limits, disabled DoFs, global
  minimum duration, trajectory sampling, position extrema, and first-time query.
- CMake static/shared builds, C examples, C unit tests, C++ oracle differential
  tests, allocation audit, and performance benchmark.
- CMake package and pkg-config install metadata for downstream consumers.

Deferred:

- Intermediate waypoints, per-section constraints, cloud calculation, Python
  bindings, Rust bindings, and per-DoF control/synchronization overrides.

Verification:

- Windows clang/clang-cl validation is recorded in `docs/release/evidence/verification_report.md`.
- Linux/macOS and sanitizer/memcheck gates are captured in CI and
  `docs/release/checklists/0.1.0.md`.
- Performance results against the frozen C++ oracle are recorded in
  `docs/release/evidence/performance_report.md`.

Known scope limitations:

- The C library does not implement intermediate waypoints, per-section
  constraints, cloud calculation, Python/Rust bindings, or per-DoF
  control/synchronization overrides in `0.1.0`.
- `original/ruckig-main` remains a frozen test oracle and is not linked into
  the C library.
