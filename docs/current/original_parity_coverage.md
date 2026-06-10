# Original Parity Coverage

This document records the current engineering estimate for `ruckig_c` coverage
against the frozen `original/ruckig-main` reference and the broader original
product surface. These percentages are not line coverage, branch coverage, or a
formal proof of numerical equivalence. They are release-review estimates backed
by source inventory, public API review, examples, tests, and release evidence.

The frozen source baseline remains Ruckig Community `0.17.3`. Product features
that are described by original examples or README text but not present in the
Community source tree are evaluated as interface/effect targets only when the
current project deliberately provides a local implementation. They are not
treated as source-porting requirements because there is no upstream source to
copy or mechanically compare.

The current project-scoped parity target excludes cloud/remote runtime,
package-manager recipes, and direct C++ template/vector ergonomics from the
required completion denominator. Cloud/Pro-only surfaces such as waypoint and
tracking behavior are evaluated by the public interface shape, local behavior,
deterministic evidence, and documented boundaries.

## Summary

| Scope | Estimate | Current status |
| --- | ---: | --- |
| Community no-waypoint target solver behavior | 93-96% | Very high parity with fixed and random frozen C++ oracle evidence. |
| C runtime / motion API replacement surface | 82-87% | Most user-facing motion concepts are available through the C ABI. |
| Waypoint and per-section interface/effect behavior | 76-82% | Local optimizer, per-section constraints, local invariants, section oracle, fixed corpus, visualization, and benchmark trend are in place; no global optimality proof is claimed. |
| Tracking interface/effect behavior | 68-78% | Local Fast and bounded Optimized tracking, strategy presets, diagnostics, quality gates, stability corpus, examples, and visualization evidence are in place. |
| Prototype bindings | 45-55% | Python and Rust remain prototype/alpha only, now with Fast, Optimized tracking, strategy preset smoke, and diagnostics snapshot coverage. |
| Current project-scoped original-surface parity | 84-89% | Core runtime is strong; waypoint and tracking are local interface/effect implementations; visualization evidence is stable; package-manager recipes, cloud runtime, and direct C++ ergonomics are explicitly out of current scope. |

## Coverage Matrix

| Area | Original source | Current `ruckig_c` status | Estimate | Evidence | Remaining gap | Next action |
| --- | --- | --- | ---: | --- | --- | --- |
| No-waypoint target solver | `include/ruckig/ruckig.hpp`, `calculator_target.hpp`, position/velocity/profile headers | Pure C target solver covers first-, second-, and third-order position/velocity cases, synchronization modes, min limits, disabled DoFs, discrete duration, minimum duration, offline, and online update. | 93-96% | Fixed oracle count 82; ordinary random 100000 seeds 1/2/41; per-DoF random 100000 seed 1; release random 1M seed 1; `docs/current/test_coverage_audit.md` maps original target tests to current oracle/C coverage and records `0.7.0-alpha.4` targeted branch coverage deltas. | Long-tail numerical cases and upstream random-history scale are not fully reproduced. | Keep oracle gates and retain every new mismatch case as a regression or tolerance exception. |
| Waypoints, per-section constraints, and global position bounds | Original waypoint examples and input/trajectory surface; implementation source for Pro/cloud behavior is not in the frozen Community tree. | Public C ABI and local coupled waypoint optimizer are implemented; per-section constraints, intermediate durations, extrema, first-time, filtering, waypoint `ruckig_update` soft interruption V1, and visualization evidence are covered. | 78-84% for current interface/effect scope | `docs/design/0.4.0_original_parity.md`; `docs/design/interrupt_calculation_duration.md`; waypoint fixed corpus; section-oracle comparisons; waypoint benchmark trend; Visualization v2 waypoint plots. | No formal global optimality proof, no cross-cycle continuation, no hard real-time guarantee, and limited optional black-box evidence against proprietary implementations. Cloud runtime itself is out of current scope. | Keep local evidence as the routine gate; use optional Pro/cloud samples only as non-blocking effect comparison evidence if available. |
| Trajectory query semantics | `include/ruckig/trajectory.hpp` | Duration, independent minimum durations, sampling, section count, intermediate durations, extrema, and first-time-at-position are exposed through C. | 80-88% | C tests, waypoint invariant tests, and release checklist evidence. | Internal profile visibility, exact C++ object ergonomics, and some diagnostic details are not exposed. | Maintain C-level trajectory query tests; do not expose internals without a separate API decision. |
| Validation and diagnostics | `input_parameter.hpp`, `output_parameter.hpp`, `result.hpp` | Public validation, result codes, output state, calculation duration, interruption storage, and waypoint update interruption status are exposed. | 78-86% | C API diagnostics tests, waypoint soft interruption tests, and API compatibility policy. | C++ exception style, detailed diagnostics, cross-cycle waypoint continuation, and hard real-time behavior are not implemented. | Keep soft interruption semantics scoped to waypoint `ruckig_update` unless a later compatibility review expands them. |
| C examples | `original/ruckig-main/examples` | Position, offline, online, velocity, stop, minimum duration, per-DoF overrides, waypoints, per-section constraints, filtering, dynamic waypoint, tracking Fast, and Optimized tracking examples exist. `0.7.0-alpha` examples explicitly select strategy presets, `v0.8.0` stabilizes a Matplotlib PNG gallery generated from public C ABI data for original examples `01-10` and `14-16`, and `v0.10.0` stabilizes 30 local Visualization v2 PNGs covering original mappings, tracking diagnostics, waypoint diagnostics, trajectory anatomy, and summary plots. | 86-92% for C ABI behavior | CMake example build and CTest example execution, including Fast and Optimized tracking examples; `docs/current/test_coverage_audit.md` maps original examples to current C/Rust/Python evidence; `docs/current/visualization.md` records local generated gallery evidence, v2 inventory, example mapping, verifier commands, manual-only CI artifact commands, and v1 provenance through the `v0.9.0` tag. | Eigen/custom vector examples `11-13` do not map directly to C; original-style PDFs are not regenerated and original images are not copied. | Keep committed gallery generation and verification local-first; optional CI artifacts are manual-only review evidence, not a default push/PR gate. |
| Python binding | Original Python examples and wrapper shape | `bindings/python_prototype` is a `cffi` ABI-mode prototype over the public C ABI, including Fast, Optimized tracking, lookahead, diagnostics snapshots, and strategy preset smoke. | 45-55% prototype scope | Local prototype suite with online/offline, multi-DoF, invalid-parameter, Fast tracking, Optimized tracking, lookahead update, diagnostics snapshot, and strategy preset coverage. | No stable Python API decision and no formal package tracking wrapper. Wheel/package publication is frozen until accepted as a separate need. | Keep prototype smoke; do not count package publication against current scoped parity. |
| Rust binding | Original Rust examples/wrapper shape | `bindings/rust` is an alpha wrapper over `ruckig_c` handles, including Fast, Optimized tracking, lookahead, diagnostics snapshots, and strategy preset smoke. | 42-52% prototype scope | Rust alpha smoke, examples, and local wrapper suite with online/offline, multi-DoF, invalid-parameter, Fast tracking, Optimized tracking, lookahead update, diagnostics snapshot, and strategy preset coverage. | No stable Rust API contract; crate publication is frozen until accepted as a separate need. | Keep alpha smoke; do not count crate publication against current scoped parity. |
| Packaging and consumers | CMake install and downstream C use | CMake install, pkg-config on Unix, shared install-tree, Windows static/DLL, clang-cl, and MinGW consumer paths are documented and tested. Package-manager recipes are explicitly frozen. | Complete for current supported consumer scope | `docs/current/packaging.md`; CI consumer matrix. | vcpkg, Conan, Homebrew, and formal package-manager recipes are not current requirements. | Maintain existing consumer matrix; reopen package-manager recipes only after a separate demand decision. |
| Cloud / Pro-only interface surface | `calculator_cloud.hpp`, README cloud/pro text, waypoint/tracking examples | Runtime cloud and remote calculation are intentionally not implemented. The relevant public interface/effect surfaces are covered by local waypoint and tracking implementations where accepted by project scope. | Not a source-porting target | Roadmap and release notes explicitly exclude cloud runtime; waypoint/tracking local evidence tracks interface/effect behavior. | No cloud client or remote fallback by design. No proprietary output equivalence claim without optional black-box samples. | Keep cloud runtime out of routine gates; evaluate local implementations by public interface shape and deterministic behavior evidence. |
| Tracking interface | README Tracking Interface and examples `14_tracking.*`, `15_tracking_offline.*`; no `trackig.hpp` in frozen Community headers | `v0.5.0` stabilizes Fast tracking. `v0.6.0` stabilizes bounded local Optimized tracking with lookahead update, offline sliding window, candidate budget, fallback diagnostics, C examples, Python smoke, and Rust smoke. `v0.7.0` stabilizes Stable/Balanced/Aggressive strategy presets, diagnostics snapshots, stricter deterministic quality gates, and 100k tracking random stress evidence. `v0.9.0` stabilizes deterministic fallback-distribution audit evidence, bounded evaluator tuning with private attribution and hard 10k/100k/1M thresholds, and fixed stability regression evidence. | 68-78% for current interface/effect scope | `docs/design/tracking_interface.md`; `docs/design/tracking_optimized_mode.md`; `docs/current/tracking_quality_audit.md`; `docs/current/tracking_quality_hardening.md`; `docs/current/tracking_stability.md`; `docs/release/checklists/0.9.0.md`; C tracking CTest gates including Optimized, diagnostics, strategy quality, quality hardening, stability, random stress, random audit, no-allocation, 1M release-random readiness, ABI/export checks, and Python/Rust smoke. | No source-level oracle exists in the frozen Community tree, and no formal optimized global optimality proof is claimed. | Keep tracking quality/stability gates in release checks; add effect comparison samples only if useful and non-blocking. |
| C++ template, static DoF, custom vector, and Eigen ergonomics | `Ruckig<DOFs>`, `DynamicDOFs`, Eigen/custom vector examples | C runtime exposes dynamic DoF handles and flat C arrays instead of C++ templates and custom vector types. | Out of current C ABI scope | Public C header and examples. | C++ compile-time ergonomics are intentionally not cloned by the C ABI and are not counted against current scoped parity. | Document as a product-surface difference only. |

## Release Interpretation

`v0.4.2` can claim that the core C runtime has high coverage of the original
motion-generation surface and very high no-waypoint target-solver coverage.
`v0.5.0` adds stable local Fast tracking evidence, `v0.6.0` adds stable
bounded local Optimized tracking evidence, and `v0.7.0` stabilizes local
strategy presets plus diagnostics snapshots after deterministic stress.
`0.7.0-alpha.3` adds local LLVM coverage evidence and an
original Community test/example behavior mapping. `0.7.0-alpha.4` adds targeted
solver branch coverage for the five lowest files from that audit, raising the
local implementation line coverage evidence from `85.33%` to `87.71%`; it is
still not a stable release and does not create proprietary Pro equivalence.
`0.7.0-readiness` reruns the full local release-readiness gate set and records
the 172-symbol strategy/diagnostics ABI before stable closeout. `0.8.0-alpha`
adds local generated visualization gallery evidence, `0.8.0-alpha.2`
replaces that first gallery with a Matplotlib/NumPy PNG set covering local C ABI
equivalents of original examples `01-10` and `14-16`, `0.8.0-alpha.3`
adds local verifier evidence for the committed PNG/manifest assets, and
`v0.8.0` stabilizes that gallery/verifier evidence after focused readiness and
full closeout gates. Original images and PDFs remain references only and are
not copied as primary project evidence.
The project can claim high current-scope original-surface parity for the C
runtime and local interface/effect implementations. It must not claim cloud
runtime support, proprietary Pro numerical equivalence, formal global
optimality proof, or direct C++ template/vector ergonomics. Package-manager
recipes and package publication are frozen outside this parity denominator
until a separate demand decision accepts them.

The current full-parity step is tracking quality/stability deepening on the
`0.9.0-design` line. `0.9.0-alpha` starts that work with deterministic
fallback-distribution and diagnostics audit evidence; `0.9.0-alpha.2` follows
with bounded evaluator tuning, private attribution, and hard quality thresholds;
`0.9.0-alpha.3` freezes representative tuned behavior as stability regression
evidence, `0.9.0-readiness` reruns the full local readiness gate set, and
`v0.9.0` stabilizes that evidence. This modestly improves the local
tracking-scope estimate, but it still does not justify proprietary Pro
equivalence or a formal optimized global optimality claim. `0.10.0-alpha` starts the next
design line by replacing the current `main` gallery with local Visualization v2
PNG evidence and a stricter verifier. `0.10.0-alpha.2` adds manual-only CI
artifact publication for regenerated Visualization v2 review assets while
keeping visualization out of default push/PR gates. `0.10.0-readiness` records
full local stable-review evidence for that Visualization v2 line, and
`v0.10.0` stabilizes the 30-PNG gallery, verifier, strict regeneration, and
manual artifact workflow without changing public ABI, tracked gallery assets,
the frozen original baseline, or default CI behavior. The next design line is
`0.11.0-design`, with soft interruption checkpoint design and evidence
planning as the first priority.
