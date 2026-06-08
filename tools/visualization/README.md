# Visualization Tools

`generate_gallery.py` creates local documentation PNGs from `ruckig_c` public C
ABI data through the Python `cffi` prototype. It is a documentation/evidence
tool, not a release gate and not a stable Python API.

## Generate

Build the shared library first:

```powershell
cmake --build --preset windows-clang-ninja-shared
$env:RUCKIG_C_SHARED_LIBRARY=(Resolve-Path out\build\windows-clang-ninja-shared\ruckig_c.dll).Path
python tools\visualization\generate_gallery.py --output docs\assets\visualization
```

The script writes deterministic PNG assets and `manifest.json` under
`docs/assets/visualization/`.

## Scope

The gallery covers:

- no-waypoint position trajectory sampling;
- velocity-control trajectory;
- stop trajectory;
- minimum-duration constrained trajectory;
- local waypoint trajectory with section timing;
- Fast vs bounded Optimized tracking comparison with diagnostics summary.

The script uses Pillow for local PNG rendering. It does not require NumPy,
Matplotlib, network access, cloud/Pro samples, or changes to the public C ABI.
