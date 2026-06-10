# Algorithm Visualization

This document records the stable `v0.10.0` Visualization v2 local gallery
evidence. The gallery is generated from `ruckig_c` public C ABI data through
the Python `cffi` prototype, NumPy, and Matplotlib `Agg`. It does not copy
original Ruckig images, PDFs, or plotter scripts as primary project evidence.

## Status

`v0.10.0` stable-adopts the 30-PNG Visualization v2 set that was introduced in
`0.10.0-alpha` and reviewed during `0.10.0-readiness`. The previous
stable-adopted v1 gallery remains traceable in the `v0.9.0` tag; it is not
duplicated in a historical assets directory on `main`.

Visualization v2 is local-only, PNG-only, and evidence-only. It does not add a
default GitHub Actions plotting job, CMake/CTest gate, public C API, public
symbol, enum value, result-code value, Python package, Rust crate, package
recipe, cloud path, or proprietary Pro equivalence claim. `0.10.0-alpha.2` added an
optional manual GitHub Actions artifact path for review copies of the gallery;
it remains off for push and pull-request CI. `v0.10.0` records local verifier,
strict regeneration, manual artifact workflow, release-random, CI, tag, and
GitHub Release evidence in `docs/release/checklists/0.10.0.md`.

```powershell
cmake --build --preset windows-clang-ninja-shared
python -m venv _local\visualization-venv
.\_local\visualization-venv\Scripts\python.exe -m pip install -r tools\visualization\requirements.txt
$env:RUCKIG_C_SHARED_LIBRARY=(Resolve-Path out\build\windows-clang-ninja-shared\ruckig_c.dll).Path
.\_local\visualization-venv\Scripts\python.exe tools\visualization\generate_gallery.py --output docs\assets\visualization
```

The generator writes `1400x900` PNG assets and `manifest.json` under
`docs/assets/visualization/`. The stable release adopts the committed PNGs and
manifest byte-for-byte. The manifest label remains `0.10.0-alpha visualization
v2 evidence` intentionally as asset provenance. It records deterministic file
names, categories, original example mapping, scenario metrics, byte counts,
and SHA-256 hashes. It intentionally avoids local absolute paths, virtualenv
paths, timestamps, generated dates, and raw sample paths.

## Verify

Default verification checks the committed assets without requiring a shared
library:

```powershell
.\_local\visualization-venv\Scripts\python.exe tools\visualization\verify_gallery.py --output docs\assets\visualization
```

The verifier checks the canonical 30 PNG filenames, `1400x900` PNG header
dimensions, manifest category/original-example mapping, byte counts, SHA-256
hashes, `11-13` exclusions, boundary flags, and absence of local paths or
timestamp fields.

Strict verification regenerates the gallery into an ignored `out/` directory
and compares regenerated PNGs and manifest with committed assets:

```powershell
$env:RUCKIG_C_SHARED_LIBRARY=(Resolve-Path out\build\windows-clang-ninja-shared\ruckig_c.dll).Path
.\_local\visualization-venv\Scripts\python.exe tools\visualization\verify_gallery.py --output docs\assets\visualization --strict-regenerate
```

Strict regeneration requires `RUCKIG_C_SHARED_LIBRARY` or `--library`.

## Optional CI Artifact

`0.10.0-alpha.2` adds a manual-only CI artifact path. It is triggered only by
workflow dispatch with `visualization_artifacts=true`:

```powershell
gh workflow run ci.yml --repo DiamondY/ruckig_c --ref main -f release_random=false -f visualization_artifacts=true
```

The job runs on Ubuntu, builds a shared `ruckig_c` library, installs the
existing visualization requirements, regenerates the 30 PNG files and
`manifest.json` under `out/visualization-artifacts/gallery`, runs default
verification, runs strict regeneration, and uploads one artifact named
`visualization-v2-gallery` containing the regenerated gallery and logs.

The artifact is a review aid. It does not replace the committed gallery, does
not run on ordinary push or pull-request CI, and does not promote plotting into
a default CI gate. `v0.10.0` records the manual artifact workflow as stable
release evidence on the release candidate and tag.

## V2 Inventory

Original mapping reworked set:

- `01_position.png`
- `02_position_offline.png`
- `03_waypoints_local.png`
- `04_waypoints_online_local.png`
- `05_velocity.png`
- `06_stop.png`
- `07_minimum_duration.png`
- `08_per_section_minimum_duration.png`
- `09_dynamic_dofs.png`
- `10_dynamic_dofs_waypoints_local.png`
- `14_tracking_online_local.png`
- `15_tracking_offline_local.png`
- `16_speed_brake_phases.png`

Tracking v2 diagnostics:

- `17_tracking_strategy_quality.png`
- `18_tracking_candidate_families.png`
- `19_tracking_fallback_diagnostics.png`
- `20_tracking_near_tie_acceptance.png`
- `21_tracking_signal_response.png`

Waypoint v2 diagnostics:

- `22_waypoint_sections_timeline.png`
- `23_waypoint_per_section_duration.png`
- `24_waypoint_constraint_profiles.png`
- `25_waypoint_position_bounds.png`
- `26_waypoint_online_section_changes.png`

Trajectory anatomy:

- `27_trajectory_jerk_profile.png`
- `28_trajectory_extrema.png`
- `29_trajectory_phase_spans.png`
- `30_trajectory_synchronization.png`

Cross-topic summaries:

- `31_gallery_coverage_matrix.png`
- `32_gallery_metrics_summary.png`
- `33_gallery_boundary_summary.png`

## Original Example Mapping

| Original example | Current gallery asset | Mapping status |
| --- | --- | --- |
| `01_position` | `01_position.png`, `27-30` trajectory anatomy | Public C ABI position trajectory and derived anatomy views. |
| `02_position_offline` | `02_position_offline.png` | Offline trajectory with directional limits. |
| `03_waypoints` | `03_waypoints_local.png`, `22`, `25` | Local waypoint optimizer equivalent, no proprietary Pro equivalence claim. |
| `04_waypoints_online` | `04_waypoints_online_local.png`, `26` | Online local waypoint update path. Gallery generation uses a deliberately high soft-interruption budget for deterministic plots; V1 interruption behavior is covered by C tests. |
| `05_velocity` | `05_velocity.png` | Velocity-control trajectory through public C ABI. |
| `06_stop` | `06_stop.png` | Stop command trajectory through public C ABI. |
| `07_minimum_duration` | `07_minimum_duration.png` | Minimum-duration trajectory. |
| `08_per_section_minimum_duration` | `08_per_section_minimum_duration.png`, `23`, `24` | Local waypoint trajectory with per-section duration and constraint views. |
| `09_dynamic_dofs` | `09_dynamic_dofs.png` | Dynamic-DoF C handle equivalent. |
| `10_dynamic_dofs_waypoints` | `10_dynamic_dofs_waypoints_local.png` | Dynamic-DoF local waypoint equivalent. |
| `11_eigen_vector_type` | excluded | C++ Eigen ergonomics, not C ABI visualization surface. |
| `12_custom_vector_type` | excluded | C++ custom vector ergonomics, not C ABI visualization surface. |
| `13_custom_vector_type_dynamic_dofs` | excluded | C++ custom vector ergonomics, not C ABI visualization surface. |
| `14_tracking` | `14_tracking_online_local.png`, `17-21` tracking diagnostics | Local bounded Optimized online tracking and diagnostics evidence. |
| `15_tracking_offline` | `15_tracking_offline_local.png` | Local bounded Optimized offline tracking evidence. |
| `16_speed` | `16_speed_brake_phases.png` | Local trajectory phase/braking visualization, no speed-control C ABI claim. |

## Gallery

![01 position trajectory](../assets/visualization/01_position.png)

![02 offline position trajectory](../assets/visualization/02_position_offline.png)

![03 local waypoints](../assets/visualization/03_waypoints_local.png)

![04 online local waypoints](../assets/visualization/04_waypoints_online_local.png)

![05 velocity control](../assets/visualization/05_velocity.png)

![06 stop trajectory](../assets/visualization/06_stop.png)

![07 minimum duration](../assets/visualization/07_minimum_duration.png)

![08 per-section minimum duration](../assets/visualization/08_per_section_minimum_duration.png)

![09 dynamic DoFs](../assets/visualization/09_dynamic_dofs.png)

![10 dynamic DoFs local waypoints](../assets/visualization/10_dynamic_dofs_waypoints_local.png)

![14 online tracking local](../assets/visualization/14_tracking_online_local.png)

![15 offline tracking local](../assets/visualization/15_tracking_offline_local.png)

![16 speed brake phases](../assets/visualization/16_speed_brake_phases.png)

![17 tracking strategy quality](../assets/visualization/17_tracking_strategy_quality.png)

![18 tracking candidate families](../assets/visualization/18_tracking_candidate_families.png)

![19 tracking fallback diagnostics](../assets/visualization/19_tracking_fallback_diagnostics.png)

![20 tracking near-tie acceptance](../assets/visualization/20_tracking_near_tie_acceptance.png)

![21 tracking signal response](../assets/visualization/21_tracking_signal_response.png)

![22 waypoint sections timeline](../assets/visualization/22_waypoint_sections_timeline.png)

![23 waypoint per-section duration](../assets/visualization/23_waypoint_per_section_duration.png)

![24 waypoint constraint profiles](../assets/visualization/24_waypoint_constraint_profiles.png)

![25 waypoint position bounds](../assets/visualization/25_waypoint_position_bounds.png)

![26 waypoint online section changes](../assets/visualization/26_waypoint_online_section_changes.png)

![27 trajectory jerk profile](../assets/visualization/27_trajectory_jerk_profile.png)

![28 trajectory extrema](../assets/visualization/28_trajectory_extrema.png)

![29 trajectory phase spans](../assets/visualization/29_trajectory_phase_spans.png)

![30 trajectory synchronization](../assets/visualization/30_trajectory_synchronization.png)

![31 gallery coverage matrix](../assets/visualization/31_gallery_coverage_matrix.png)

![32 gallery metrics summary](../assets/visualization/32_gallery_metrics_summary.png)

![33 gallery boundary summary](../assets/visualization/33_gallery_boundary_summary.png)

## Evidence Boundaries

- The images are generated from local `ruckig_c` data through the public C ABI
  and Python prototype.
- Original Ruckig `doc/*.png`, `examples/*_trajectory.pdf`, and
  `examples/plotter.py` remain references only; they are not copied as primary
  evidence.
- The generator uses NumPy and Matplotlib `Agg` locally. The optional CI
  artifact workflow can regenerate review copies manually, but it is not a
  default GitHub Actions gate.
- PNG is the only committed output format. Raw sampled data, `.profraw`,
  Matplotlib caches, virtualenv files, PDF, and SVG outputs are not committed.
- Python `cffi` remains a prototype path used to drive the C ABI; this does
  not publish or stabilize a Python package.
- No public C API, public symbol, enum value, result code, or release gate
  changes are introduced by this visualization work.
- No cloud, Pro license, network image fetching, remote calculation,
  proprietary Pro equivalence claim, or formal global optimality claim is used.

## Follow-Up

Optional CI visualization artifacts now exist as manual-only review evidence.
Making visualization a default push or pull-request CI gate remains a separate
decision.
