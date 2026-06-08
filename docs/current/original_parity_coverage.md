# Original Parity Coverage

This document records the current engineering estimate for `ruckig_c` coverage
against the frozen `original/ruckig-main` reference and the broader original
product surface. These percentages are not line coverage, branch coverage, or a
formal proof of numerical equivalence. They are release-review estimates backed
by source inventory, public API review, examples, tests, and release evidence.

The frozen source baseline remains Ruckig Community `0.17.3`. Product features
that are described by original examples or README text but not present in the
Community source tree are tracked separately so the project does not overstate
source-level parity.

## Summary

| Scope | Estimate | Current status |
| --- | ---: | --- |
| Community no-waypoint target solver behavior | 93-96% | Very high parity with fixed and random frozen C++ oracle evidence. |
| C runtime / motion API replacement surface | 82-87% | Most user-facing motion concepts are available through the C ABI. |
| Waypoint and per-section behavior versus full Pro/cloud behavior | 60-70% | Local optimizer exists, but no Pro/cloud equivalence claim or proof of global optimality. |
| Waypoint and per-section behavior versus declared local optimizer scope | 75-80% | Local invariants, section oracle, fixed corpus, and benchmark trend are in place. |
| Bindings and ecosystem | 38-48% | Python and Rust remain prototype/alpha only, now with Fast, Optimized tracking, `v0.7.0` strategy preset smoke, and diagnostics snapshot coverage; package-manager recipes are deferred. |
| Full original repository/product parity | 73-78% | Core runtime is strong; `v0.7.0` stabilizes bounded Optimized tracking strategy presets and diagnostics snapshots after alpha coverage/readiness evidence, and `0.8.0-alpha` adds local generated visualization gallery evidence. Formal bindings, package distribution, Pro/cloud equivalence, and C++ ergonomics remain gaps. |

## Coverage Matrix

| Area | Original source | Current `ruckig_c` status | Estimate | Evidence | Remaining gap | Next action |
| --- | --- | --- | ---: | --- | --- | --- |
| No-waypoint target solver | `include/ruckig/ruckig.hpp`, `calculator_target.hpp`, position/velocity/profile headers | Pure C target solver covers first-, second-, and third-order position/velocity cases, synchronization modes, min limits, disabled DoFs, discrete duration, minimum duration, offline, and online update. | 93-96% | Fixed oracle count 82; ordinary random 100000 seeds 1/2/41; per-DoF random 100000 seed 1; release random 1M seed 1; `docs/current/test_coverage_audit.md` maps original target tests to current oracle/C coverage and records `0.7.0-alpha.4` targeted branch coverage deltas. | Long-tail numerical cases, upstream random-history scale, Pro scaling and diagnostics are not reproduced. | Keep oracle gates and retain every new mismatch case as a regression or tolerance exception. |
| Waypoints, per-section constraints, and global position bounds | Original waypoint examples and input/trajectory surface; Pro/cloud behavior is not in the frozen Community source. | Public C ABI and local coupled waypoint optimizer are implemented; per-section constraints, intermediate durations, extrema, first-time, and filtering are covered. | 60-70% versus full Pro/cloud; 75-80% versus local scope | `docs/design/0.4.0_original_parity.md`; `0.4.1` waypoint fixed corpus; section-oracle comparisons; waypoint benchmark trend. | No formal global optimality proof, no cloud fallback, no Pro/cloud output equivalence claim, limited black-box evidence. | Keep local evidence as the routine gate; use optional Pro/cloud samples only as non-blocking evidence if available. |
| Trajectory query semantics | `include/ruckig/trajectory.hpp` | Duration, independent minimum durations, sampling, section count, intermediate durations, extrema, and first-time-at-position are exposed through C. | 80-88% | C tests, waypoint invariant tests, and release checklist evidence. | Internal profile visibility, exact C++ object ergonomics, and some diagnostic details are not exposed. | Maintain C-level trajectory query tests; do not expose internals without a separate API decision. |
| Validation and diagnostics | `input_parameter.hpp`, `output_parameter.hpp`, `result.hpp` | Public validation, result codes, output state, calculation duration, and interruption storage are exposed. | 75-85% | C API diagnostics tests and API compatibility policy. | C++ exception style, detailed diagnostics, and true waypoint optimizer interruption behavior are not implemented. | Follow `docs/design/interrupt_calculation_duration.md` before adding timeout behavior. |
| C examples | `original/ruckig-main/examples` | Position, offline, online, velocity, stop, minimum duration, per-DoF overrides, waypoints, per-section constraints, filtering, dynamic waypoint, tracking Fast, and Optimized tracking examples exist. `0.7.0-alpha` examples explicitly select strategy presets, and `0.8.0-alpha` generates a local PNG gallery from public C ABI data. | 82-89% | CMake example build and CTest example execution, including Fast and Optimized tracking examples; `docs/current/test_coverage_audit.md` maps original examples to current C/Rust/Python evidence; `docs/current/visualization.md` records local generated gallery evidence. | Eigen/custom vector examples do not map directly to C; Optimized examples are local MVP evidence, not stable Pro parity; original-style PDFs are not regenerated and original images are not copied. | Keep gallery generation local-only; expand scenarios only after dependency and artifact policy is accepted. |
| Python binding | Original Python examples and package distribution | `bindings/python_prototype` is a `cffi` ABI-mode prototype over the public C ABI, including Fast, Optimized tracking, lookahead, diagnostics snapshots, and `v0.7.0` strategy preset smoke. | 45-55% | Local prototype suite with online/offline, multi-DoF, invalid-parameter, Fast tracking, Optimized tracking, lookahead update, diagnostics snapshot, and strategy preset coverage. | No wheel, no stable Python API, no package metadata, no formal package tracking wrapper. | Keep prototype smoke; decide formal package scope separately after tracking stable-release evidence. |
| Rust binding | Original Rust examples/wrapper | `bindings/rust` is an alpha wrapper over `ruckig_c` handles, including Fast, Optimized tracking, lookahead, diagnostics snapshots, and `v0.7.0` strategy preset smoke. | 42-52% | Rust alpha smoke, examples, and local wrapper suite with online/offline, multi-DoF, invalid-parameter, Fast tracking, Optimized tracking, lookahead update, diagnostics snapshot, and strategy preset coverage. | No published crate, no stable Rust API contract, Optimized tracking wrapper is alpha only. | Keep alpha smoke; decide crate scope separately after C tracking stabilizes. |
| Packaging and consumers | CMake install, downstream use, package ecosystem | CMake install, pkg-config on Unix, shared install-tree, Windows static/DLL, clang-cl, and MinGW consumer paths are documented and tested. | 65-75% | `docs/current/packaging.md`; CI consumer matrix. | vcpkg, Conan, Homebrew, and formal package-manager recipes are deferred. | Keep existing consumer matrix; reopen package-manager work only as a separate project. |
| Cloud / Pro behavior | `calculator_cloud.hpp`, README cloud/pro text | Runtime cloud and remote calculation are intentionally not implemented. | 0% for cloud runtime | Roadmap and release notes explicitly exclude cloud. | No cloud client, no remote fallback, no formal Pro/cloud numerical equivalence claim. | Keep out of routine gates; optional black-box evidence must remain non-blocking. |
| Tracking interface | README Tracking Interface and examples `14_tracking.*`, `15_tracking_offline.*`; no `trackig.hpp` in frozen Community headers | `v0.5.0` stabilizes Fast tracking. `v0.6.0` stabilizes bounded local Optimized tracking with lookahead update, offline sliding window, candidate budget, fallback diagnostics, C examples, Python smoke, and Rust smoke. `v0.7.0` stabilizes Stable/Balanced/Aggressive strategy presets, diagnostics snapshots, stricter deterministic quality gates, and 100k tracking random stress evidence. | 60-70% local tracking scope; 34-44% versus full Pro surface | `docs/design/tracking_interface.md`; `docs/design/tracking_optimized_mode.md`; C tracking CTest gates including Optimized, diagnostics, strategy quality, random stress, no-allocation, 1M release-random readiness, ABI/export checks, and Python/Rust smoke. | No source-level oracle, no Pro/cloud equivalence, no package publication, and no formal optimized global optimality proof. | Keep strategy/diagnostics evidence in stable release checks; future work should deepen quality and visualization without changing the stable ABI by default. |
| C++ template, static DoF, custom vector, and Eigen ergonomics | `Ruckig<DOFs>`, `DynamicDOFs`, Eigen/custom vector examples | C runtime exposes dynamic DoF handles and flat C arrays instead of C++ templates and custom vector types. | Not directly applicable to C ABI; full-product gap if counted | Public C header and examples. | C++ compile-time ergonomics are intentionally not cloned by the C ABI. | Do not treat as a C runtime blocker; document as a product-surface difference. |

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
still not a stable release and does not create formal Pro/cloud equivalence.
`0.7.0-readiness` reruns the full local release-readiness gate set and records
the 172-symbol strategy/diagnostics ABI before stable closeout. `0.8.0-alpha`
adds local generated visualization gallery evidence with `ruckig_c`-owned
trajectory/tracking/waypoint PNGs instead of copying original images as primary
project evidence.
The project still must not claim complete original product parity because
formal bindings, package publication, cloud/Pro equivalence, formal global
optimality proof, and C++-specific ergonomics remain incomplete or
intentionally out of scope.

The next full-parity step is expanding visualization scenarios, deciding
whether plotting artifacts should become optional CI uploads, and continuing
tracking quality/stability deepening.
