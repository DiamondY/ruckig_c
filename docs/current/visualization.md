# Algorithm Visualization

This document records the `0.8.0-design` local visualization evidence. The
gallery is generated from `ruckig_c` public C ABI data through the Python
`cffi` prototype; it does not copy original Ruckig images, PDFs, or plotter
scripts as primary project evidence.

## Status

`0.8.0-alpha.2` replaces the first Pillow-only gallery with a Matplotlib
`Agg` gallery backed by NumPy arrays sampled from local public C ABI calls.
`0.8.0-alpha.3` adds a local verifier for the committed PNG/manifest assets.
`0.8.0-readiness` records focused local readiness evidence for the current
gallery and verifier. The gallery remains PNG-only and local-only.

```powershell
cmake --build --preset windows-clang-ninja-shared
python -m venv _local\visualization-venv
.\_local\visualization-venv\Scripts\python.exe -m pip install -r tools\visualization\requirements.txt
$env:RUCKIG_C_SHARED_LIBRARY=(Resolve-Path out\build\windows-clang-ninja-shared\ruckig_c.dll).Path
.\_local\visualization-venv\Scripts\python.exe tools\visualization\generate_gallery.py --output docs\assets\visualization
```

The generator writes:

- `docs/assets/visualization/01_position.png`
- `docs/assets/visualization/02_position_offline.png`
- `docs/assets/visualization/03_waypoints_local.png`
- `docs/assets/visualization/04_waypoints_online_local.png`
- `docs/assets/visualization/05_velocity.png`
- `docs/assets/visualization/06_stop.png`
- `docs/assets/visualization/07_minimum_duration.png`
- `docs/assets/visualization/08_per_section_minimum_duration.png`
- `docs/assets/visualization/09_dynamic_dofs.png`
- `docs/assets/visualization/10_dynamic_dofs_waypoints_local.png`
- `docs/assets/visualization/14_tracking_online_local.png`
- `docs/assets/visualization/15_tracking_offline_local.png`
- `docs/assets/visualization/16_speed_brake_phases.png`
- `docs/assets/visualization/manifest.json`

`manifest.json` records deterministic file names, original example mapping,
scenario metrics, byte counts, and SHA-256 hashes. It does not record local
absolute paths, virtualenv paths, timestamps, or generated raw sample data.

## Verify

Default verification checks the committed assets without requiring a shared
library:

```powershell
.\_local\visualization-venv\Scripts\python.exe tools\visualization\verify_gallery.py --output docs\assets\visualization
```

The verifier checks the canonical 13 PNG filenames, PNG header dimensions,
manifest byte counts and SHA-256 hashes, original example mapping, `11-13`
exclusions, boundary flags, and absence of local paths or timestamp fields.

Strict verification regenerates the gallery into an ignored `out/` directory
and compares the regenerated PNGs and manifest with committed assets:

```powershell
$env:RUCKIG_C_SHARED_LIBRARY=(Resolve-Path out\build\windows-clang-ninja-shared\ruckig_c.dll).Path
.\_local\visualization-venv\Scripts\python.exe tools\visualization\verify_gallery.py --output docs\assets\visualization --strict-regenerate
```

The verifier is a local evidence tool. It is not wired into default GitHub
Actions, CMake, or CTest.

`docs/release/checklists/0.8.0-readiness.md` records the focused readiness
audit. If the local focused gates and ordinary push CI are green, the current
gallery/verifier evidence is ready for a later `v0.8.0` stable closeout
decision.

## Original Example Mapping

| Original example | Current gallery asset | Mapping status |
| --- | --- | --- |
| `01_position` | `01_position.png` | Online 3-DoF position trajectory through public C ABI. |
| `02_position_offline` | `02_position_offline.png` | Offline trajectory with directional limits. |
| `03_waypoints` | `03_waypoints_local.png` | Local waypoint optimizer equivalent, no Pro/cloud equivalence claim. |
| `04_waypoints_online` | `04_waypoints_online_local.png` | Online local waypoint update path, interruption remains storage-only. |
| `05_velocity` | `05_velocity.png` | Velocity-control trajectory through public C ABI. |
| `06_stop` | `06_stop.png` | Stop command trajectory through public C ABI. |
| `07_minimum_duration` | `07_minimum_duration.png` | Minimum-duration trajectory. |
| `08_per_section_minimum_duration` | `08_per_section_minimum_duration.png` | Local waypoint trajectory with per-section minimum duration. |
| `09_dynamic_dofs` | `09_dynamic_dofs.png` | Dynamic-DoF C handle equivalent. |
| `10_dynamic_dofs_waypoints` | `10_dynamic_dofs_waypoints_local.png` | Dynamic-DoF local waypoint equivalent. |
| `11_eigen_vector_type` | excluded | C++ Eigen ergonomics, not C ABI visualization surface. |
| `12_custom_vector_type` | excluded | C++ custom vector ergonomics, not C ABI visualization surface. |
| `13_custom_vector_type_dynamic_dofs` | excluded | C++ custom vector ergonomics, not C ABI visualization surface. |
| `14_tracking` | `14_tracking_online_local.png` | Local bounded Optimized online tracking evidence. |
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

## Evidence Boundaries

- The images are generated from local `ruckig_c` data through the public C ABI.
- Original Ruckig `doc/*.png`, `examples/*_trajectory.pdf`, and
  `examples/plotter.py` remain references only; they are not copied as primary
  evidence.
- The generator uses NumPy and Matplotlib `Agg` locally. It is not a default
  GitHub Actions gate and does not add a workflow.
- The gallery is documentation/evidence work, not solver behavior work.
- PNG is the only committed output format. Raw sampled data, `.profraw`,
  Matplotlib caches, virtualenv files, PDF, and SVG outputs are not committed.
- Python `cffi` remains a prototype path used to drive the C ABI; this does
  not publish or stabilize a Python package.
- No public C API, public symbol, enum value, result code, ABI artifact path,
  version macro, or release gate changes are introduced by this visualization
  work.
- No cloud, Pro license, network image fetching, remote calculation, formal
  Pro/cloud equivalence claim, or formal global optimality claim is used.

## Follow-Up

Future visualization work can add optional CI artifact uploads, richer waypoint
diagnostics, additional tracking quality plots, or alternate output formats.
Those remain separate scope decisions because they affect dependency policy,
artifact volume, and release-gate boundaries.
