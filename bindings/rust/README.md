# Rust Alpha Wrapper

This directory is an experimental alpha wrapper over the public `ruckig_c` C
ABI. It covers the `0.4.x` waypoint ABI, the stable `v0.5.0` Fast tracking
ABI, and the `0.7.0-alpha.2` Optimized tracking strategy and diagnostics
additions. It is not a published crate and is not installed by CMake.

The wrapper owns the same opaque C handles as the C API:

- `Ruckig`
- `InputParameter`
- `OutputParameter`
- `Trajectory`
- `Tracking`
- `TargetState`
- `TargetStateSequence`
- `TrackingOutputSequence`

Current alpha coverage:

- offline position calculation;
- online update loop;
- waypoint-aware constructors;
- intermediate waypoint input, readback, clearing, and filtering;
- per-section minimum duration, velocity, acceleration, jerk, and position
  constraint setters used by tests;
- interrupt-calculation-duration storage APIs;
- trajectory duration, section count, intermediate durations, sampling, and
  position extrema and first-time-at-position;
- output position, velocity, acceleration, jerk, section, calculation-state,
  and calculation-duration accessors;
- tracking alpha online Fast update, offline sequence smoke, Optimized
  lookahead update, candidate-budget diagnostics, strategy presets,
  diagnostics snapshots, and fallback status smoke;
- Rust examples for position, offline, velocity, waypoints, per-section
  minimum duration, tracking Fast, and tracking Optimized alpha.

The build script links an already-built `ruckig_c` static library. By default
it looks in `out/build/windows-clang-ninja`. Set `RUCKIG_C_LIB_DIR` to override
the library directory.

Example:

```powershell
cmake --preset windows-clang-ninja
cmake --build --preset windows-clang-ninja
cargo test --manifest-path bindings\rust\Cargo.toml
cargo test --manifest-path bindings\rust\Cargo.toml --examples
```

Boundaries:

- No crate publication in alpha.
- No generated bindings.
- No direct dependency on original C++ Ruckig.
- No package-manager recipe.
- No stable Rust tracking API commitment before a deliberate wrapper
  publication decision.
