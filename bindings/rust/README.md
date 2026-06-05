# Rust Alpha Wrapper

This directory is an experimental `0.4.0-design` wrapper over the public
`ruckig_c` C ABI. It is not a published crate and is not installed by CMake.

The wrapper owns the same opaque C handles as the C API:

- `Ruckig`
- `InputParameter`
- `OutputParameter`
- `Trajectory`

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
- Rust examples for position, offline, velocity, waypoints, and per-section
  minimum duration.

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
