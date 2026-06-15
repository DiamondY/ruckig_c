# Python cffi ABI-Mode Prototype

This directory is an experimental prototype over the public `ruckig_c` C ABI.
It covers the `0.4.x` waypoint ABI, the stable `v0.5.0` Fast tracking ABI, and
the `0.7.0-alpha.2` Optimized tracking strategy and diagnostics additions. It
is not a released binding API, not installed by CMake, and not part of the
public C ABI.

The prototype loads an already-built shared `ruckig_c` library with `cffi`
ABI mode and validates the minimum wrapper model:

- handle create/destroy for `ruckig_t`, `ruckig_input_t`, `ruckig_output_t`,
  and `ruckig_trajectory_t`;
- offline `ruckig_calculate`;
- online `ruckig_update` plus `ruckig_output_pass_to_input`;
- list/tuple copy-in and copy-out for DoF vectors;
- normal `RUCKIG_WORKING` / `RUCKIG_FINISHED` control flow;
- typed exceptions for error result codes;
- wrapper lifecycle safety for context managers, double close, and after-close
  method calls.
- `0.4.0` waypoint-aware constructors, intermediate positions,
  per-section constraints, intermediate duration queries, position extrema,
  first-time-at-position, and local intermediate-position filtering.
- tracking handles, online Fast tracking, offline target sequences,
  Optimized lookahead update, candidate-budget diagnostics, strategy presets,
  diagnostics snapshots, tracking lifecycle checks, and prototype smoke for
  no-waypoint plus Optimized online tracking interruption.
- tracking sequence continuation smoke for the `0.15.0-alpha.4` continuation
  handle and the Fast/Optimized interruptible/resume sequence API implemented
  in alpha.5 and alpha.6.
- prototype public diagnostics smoke for the stable `v0.16.0` C API, including
  `Diagnostics`, diagnostic scope/code enums, validate/calculate/update
  `_with_diagnostics`, tracking public diagnostics, and tracking sequence
  continuation diagnostics.

## Prerequisites

Install `cffi` into the Python environment used for the prototype and build a
shared `ruckig_c` library first.

Example:

```powershell
cmake -S . -B build-python-prototype-shared -DBUILD_SHARED_LIBS=ON -DBUILD_RUCKIG_C_ORACLE_TESTS=OFF -DBUILD_RUCKIG_C_PERFORMANCE_TESTS=OFF
cmake --build build-python-prototype-shared --config Release
$env:RUCKIG_C_SHARED_LIBRARY = (Resolve-Path build-python-prototype-shared\ruckig_c.dll).Path
python bindings\python_prototype\test_prototype.py
```

On Linux, point `RUCKIG_C_SHARED_LIBRARY` at the built `libruckig_c.so`. On
macOS, point it at the built `libruckig_c.dylib`.

## Boundaries

- No NumPy dependency.
- No CPython extension module.
- No pybind11.
- No formal packaging or wheel metadata.
- No formal Python package or wheel release.
- No stable tracking binding API commitment; tracking coverage is prototype
  smoke evidence for the C ABI.
- No stable tracking sequence continuation binding API commitment; the
  continuation wrapper is alpha smoke evidence only.
- No stable Python public diagnostics wrapper API commitment; diagnostics
  coverage remains prototype smoke evidence over the stable C ABI.
