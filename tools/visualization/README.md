# Visualization Tools

`generate_gallery.py` creates local documentation PNGs from `ruckig_c` public C
ABI data through the Python `cffi` prototype. It is a documentation/evidence
tool, not a release gate and not a stable Python API.

The `0.8.0-alpha.2` generator uses NumPy plus Matplotlib with the `Agg`
backend. It replaces the earlier Pillow-only gallery and writes a
project-owned PNG set equivalent to original examples `01-10` and `14-16`
where those examples map to the C ABI.

## Setup

Build the shared library first, then install the optional local plotting
dependencies into an ignored virtual environment:

```powershell
cmake --build --preset windows-clang-ninja-shared
python -m venv _local\visualization-venv
.\_local\visualization-venv\Scripts\python.exe -m pip install -r tools\visualization\requirements.txt
```

`_local/` is ignored by the repository. Do not commit the virtual environment,
raw sampled data, generated PDF/SVG files, or Matplotlib caches.

## Generate

```powershell
$env:RUCKIG_C_SHARED_LIBRARY=(Resolve-Path out\build\windows-clang-ninja-shared\ruckig_c.dll).Path
.\_local\visualization-venv\Scripts\python.exe tools\visualization\generate_gallery.py --output docs\assets\visualization
```

The script removes old PNGs in the output directory and writes deterministic
PNG assets plus `manifest.json` under `docs/assets/visualization/`. The
manifest records file names, scenario titles, original example mapping, metrics,
byte counts, and SHA-256 hashes. It intentionally avoids local absolute paths,
virtualenv paths, timestamps, and other machine-specific fields.

## Scope

The gallery covers:

- original examples `01-10` through local public C ABI equivalents;
- original tracking examples `14` and `15` through local Fast/Optimized
  tracking data;
- original speed-control example `16` as a local trajectory phase/braking
  visualization without claiming a speed-control C ABI;
- local waypoint examples through the current local waypoint optimizer, without
  Pro/cloud equivalence claims.

Original examples `11-13` are excluded because they demonstrate C++ Eigen and
custom vector ergonomics rather than behavior exposed through the C ABI.

The tool is local-only. It does not add a default CI gate, public C API, public
symbol, package publication, network dependency, cloud calculation path, or
Pro/cloud numerical equivalence claim.
