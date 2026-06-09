# Visualization Tools

`generate_gallery.py` creates local documentation PNGs from `ruckig_c` public C
ABI data through the Python `cffi` prototype. It is a documentation/evidence
tool, not a release gate and not a stable Python API.

The current `0.10.0-alpha` generator writes a 30-PNG Visualization v2 gallery
with NumPy and Matplotlib `Agg`. It replaces the `main` v1 gallery under
`docs/assets/visualization/`; the previous v1 assets remain traceable through
the `v0.9.0` tag. `0.10.0-readiness` keeps the committed gallery unchanged and
uses the local verifier, strict regeneration, and manual artifact workflow as
stable-review evidence.

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
`1400x900` PNG assets plus `manifest.json` under
`docs/assets/visualization/`. The manifest records file names, categories,
scenario titles, original example mapping, metrics, byte counts, and SHA-256
hashes. It intentionally avoids local absolute paths, virtualenv paths,
timestamps, generated dates, and other machine-specific fields.

## Verify

Default verification checks the committed gallery without regenerating it:

```powershell
.\_local\visualization-venv\Scripts\python.exe tools\visualization\verify_gallery.py --output docs\assets\visualization
```

The verifier uses Python stdlib PNG header parsing, so the committed-asset
check does not need Pillow. It validates the canonical 30-file inventory,
`1400x900` PNG dimensions, manifest byte counts, SHA-256 hashes, categories,
original example mapping, `11-13` exclusions, boundary flags, and absence of
local paths or timestamp fields.

Strict regeneration uses the current interpreter to run `generate_gallery.py`
into an ignored `out/` directory and compares the regenerated assets against
the committed PNGs and manifest:

```powershell
$env:RUCKIG_C_SHARED_LIBRARY=(Resolve-Path out\build\windows-clang-ninja-shared\ruckig_c.dll).Path
.\_local\visualization-venv\Scripts\python.exe tools\visualization\verify_gallery.py --output docs\assets\visualization --strict-regenerate
```

Strict regeneration requires `RUCKIG_C_SHARED_LIBRARY` or `--library`.

## Optional CI Artifact

`0.10.0-alpha.2` adds a manual-only GitHub Actions artifact path. It is not run
by ordinary push or pull-request CI.

```powershell
gh workflow run ci.yml --repo DiamondY/ruckig_c --ref main -f release_random=false -f visualization_artifacts=true
```

The CI job builds a shared library on Ubuntu, installs these requirements,
regenerates the gallery under `out/visualization-artifacts/gallery`, runs the
default verifier, runs strict regeneration, and uploads `visualization-v2-gallery`
with the regenerated PNGs, `manifest.json`, and logs.

`0.10.0-readiness` records this workflow as manual-only review evidence. It is
not a default push or pull-request gate and is not the stable release closeout.

## Scope

The v2 gallery covers:

- original examples `01-10` through local public C ABI equivalents;
- original tracking examples `14` and `15` through local Fast/Optimized
  tracking data;
- original speed-control example `16` as a local trajectory phase/braking
  visualization without claiming a speed-control C ABI;
- tracking quality and diagnostics plots from public tracking diagnostics;
- local waypoint section, per-section duration, constraint, and online section
  change plots;
- trajectory anatomy plots for jerk, extrema, phase spans, and synchronization;
- cross-topic summary plots for gallery coverage, inventory, and boundaries.

Original examples `11-13` are excluded because they demonstrate C++ Eigen and
custom vector ergonomics rather than behavior exposed through the C ABI.

The tool is local-first. The optional CI artifact path is manual-only and
review-oriented. It does not add a default CI gate, public C API, public symbol,
package publication, network dependency, cloud calculation path, or Pro/cloud
numerical equivalence claim.
