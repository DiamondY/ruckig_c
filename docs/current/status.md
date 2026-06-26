# Current Maintenance Status

This is the short current-state entry point for maintainers. Historical
release checklists and evidence remain authoritative for the facts they record
at the time they were produced; this page summarizes the current baseline.

## Baseline

- Current stable release: `v0.16.0`.
- Public C ABI baseline: 190 approved `ruckig_*` symbols.
- Runtime scope: pure C99 `ruckig_c` library with no C++ runtime dependency.
- Upstream oracle: `original/ruckig-main` remains the frozen
  `0.17.3-line` baseline and is used only for tests.
- Supported integration surface: CMake install, pkg-config, static/DLL,
  shared install-tree consumers, examples, and prototype Python/Rust smoke.
- Exported-symbol policy: Linux and Windows shared builds compare against the
  baseline symbol snapshots; macOS currently uploads a Mach-O export snapshot
  as bootstrap evidence and is not yet a baseline diff gate.

## Supported Scope

- Local no-waypoint position and velocity trajectory generation.
- Waypoints, per-section constraints, local waypoint optimizer, and waypoint
  online interruption/resume behavior.
- Fast and bounded local Optimized tracking, including tracking sequence
  continuation.
- Opt-in public diagnostics through `ruckig_diagnostics_t` and diagnostics
  getter APIs.
- Local verification through CTest, focused selectors, oracle tests,
  allocation audit, ABI/export checks, and prototype wrapper smoke.

## Frozen Or Event-Driven Scope

- Python wheels and Rust crate publication remain prototype-only until a
  wrapper route with package/discovery ownership is accepted.
- Package-manager recipes remain frozen until concrete external demand and a
  maintenance owner exist.
- Upstream baseline upgrades require a separate readiness project; do not
  update `original/ruckig-main` during routine maintenance.
- Heavy random, coverage upload, visualization artifacts, and broader static
  analysis remain manual/event-driven, not default push CI.
- Coverage percentage alone is not a task trigger; new tests should be
  regression-backed, oracle-backed, public-behavior-backed, or invariant-backed.

## Default Local Verification

Routine local gate:

```powershell
cmake --build --preset windows-clang-ninja
ctest --test-dir out\build\windows-clang-ninja --output-on-failure
git diff --check
```

Oracle smoke gate:

```powershell
cmake --build --preset windows-clang-ninja-oracle
ctest --test-dir out\build\windows-clang-ninja-oracle --output-on-failure -R "ruckig_c_oracle_tests|ruckig_c_waypoint_section_oracle|ruckig_c_oracle_random_smoke|ruckig_c_oracle_random_per_dof_smoke"
```

Run shared ABI/export checks when touching the public header, CMake install
metadata, exported-symbol policy, or package metadata:

```powershell
cmake --build --preset windows-clang-ninja-shared
cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_verify_public_symbols
cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_compare_public_exported_symbols
```

## Touched-File Verification Matrix

Use this matrix to choose focused checks before relying on full CTest.

| Touched area | Focused checks |
| --- | --- |
| `src/ruckig_c/ruckig.c` | Default CTest; `ruckig_c_public_diagnostics`; `ruckig_c_interrupt_boundary_audit`; `ruckig_c_no_waypoint_interrupt_audit`; `ruckig_c_constructor_boundaries`; oracle fixed/smoke. |
| `src/ruckig_c/waypoint.c` | `ruckig_c_waypoint_optimizer`; `ruckig_c_per_section_constraints`; `ruckig_c_waypoint_resume_stress`; `ruckig_c_waypoint_resume_quality_audit`; `ruckig_c_state_machine_branch_coverage`; `ruckig_c_waypoint_section_oracle`. |
| `src/ruckig_c/tracking_update.c`, `src/ruckig_c/tracking_sequence.c` | `ruckig_c_tracking_api`; `ruckig_c_tracking_public_diagnostics`; `ruckig_c_tracking_sequence_continuation_api`; `ruckig_c_tracking_sequence_fast_continuation`; `ruckig_c_tracking_sequence_optimized_continuation`; `ruckig_c_tracking_no_allocation`; tracking quality/stability selectors when behavior changes. |
| Solver step files, `src/ruckig_c/roots.c`, `src/ruckig_c/profile.c` | `ruckig_c_solver_branch_coverage`; `ruckig_c_roots_numeric_audit`; `ruckig_c_profile`; oracle fixed/smoke. Use heavy random only for regression or release evidence. |
| Public header, CMake, install or package metadata | Header C/C++ tests; `ruckig_c_linked_library_smoke`; installed CMake/pkg-config consumer checks when available; shared ABI/export targets. |
| Tests only | Matching focused selector plus default CTest; preserve existing CTest names and selector flags. |

Targeted `clang-tidy` is preferred for touched high-risk implementation files
when locally available. If it is unavailable or noisy for unrelated reasons,
record that in the checklist and continue with the build, focused checks, and
boundary checks.
