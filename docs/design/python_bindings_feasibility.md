# Python Bindings Feasibility

This is a `0.3.0` feasibility document only. It does not approve a released
binding package, does not add a public binding API, does not change the C ABI,
and does not change solver behavior.

## Prerequisites

Python bindings should wait until the `0.2.x` maintenance line has repeatable
evidence for:

- At least one completed `0.2.x` patch release after `v0.2.1`.
- Public header diff review in a release checklist.
- Exported-symbol evidence generated on Windows and Linux.
- Stable installed CMake, pkg-config, static, DLL, and shared install-tree
  consumer paths.
- `docs/current/api_compatibility.md` used as part of a release closeout.
- Windows and Linux performance records below the `1.5` average-ratio
  threshold.

## Candidate Routes

### ctypes

`ctypes` is the lowest-build-friction route. It can load a shared `ruckig_c`
library and call the C ABI directly, which is useful for an early feasibility
prototype. Its drawbacks are more manual type declarations, weaker static
checking, and more runtime error surface around array sizes and ownership.

### cffi

`cffi` gives a cleaner C declaration model and can support ABI-mode prototypes
without compiling an extension module. It is a strong candidate for the first
prototype if packaging a shared library is already reliable across platforms.

Default prototype choice: use `cffi` ABI mode first. It keeps the prototype on
the existing pure C ABI, avoids compiling a CPython extension during feasibility
work, and lets the project validate ownership, array, error, and packaging
questions before committing to a wheel build strategy.

The current prototype workspace is `bindings/python_prototype/`. It is not a
released binding package and is not installed by CMake. It must load an
already-built shared `ruckig_c` library and stay close to the C ABI while
ownership, array, lifecycle, and error behavior are evaluated.

## Current Prototype Decisions

The `0.3.0` prototype remains experimental. It is not published, not installed
by CMake, not part of routine CI, and must not change the public C ABI.

- Low-level API shape: keep the prototype close to the C ABI with explicit
  `Ruckig`, `Input`, `Output`, and `Trajectory` wrappers around one opaque C
  handle each.
- Higher-level API shape: defer Pythonic convenience wrappers until the
  low-level ownership, lifecycle, array, error, and shared-library discovery
  rules have been proven.
- Result handling: treat `RUCKIG_WORKING` and `RUCKIG_FINISHED` as normal
  control flow. Map error result codes to typed Python exceptions while
  preserving the original integer result code for debugging and compatibility
  tests.
- Shared-library discovery: continue to require
  `RUCKIG_C_SHARED_LIBRARY` for routine prototype smoke tests. Opportunistic
  default library-name loading may stay in the prototype, but documented
  verification must use an explicit shared-library path.
- Array model: keep list/tuple copy-in/copy-out as the default. Buffer
  protocol, memoryview, and NumPy fast paths are deferred until lifetime rules
  and package strategy are stable.
- Package strategy: no wheel, source distribution, vendored library, or
  installer strategy is implemented in `0.3.0`. Wheel/package design
  remains a separate decision after shared-library loading is stable.

### CPython Extension

A CPython extension can provide the tightest user experience and fastest array
conversion, but it creates more packaging work and a larger compiled matrix. It
should be considered after the ownership, array, and error models are proven.

### pybind11

`pybind11` is not the default route because this project intentionally exposes a
pure C ABI and avoids introducing a C++ runtime dependency into `ruckig_c`
consumers. It may still be evaluated separately for a higher-level wrapper, but
it should not be the first binding path.

## Ownership Model

The Python layer should wrap the same opaque C handles:

- `ruckig_t`
- `ruckig_input_t`
- `ruckig_output_t`
- `ruckig_trajectory_t`

Each wrapper must own exactly one C handle and call the matching destroy
function. Context-manager support should release handles deterministically.
`destroy(NULL)` behavior in C is useful for cleanup, but Python wrappers should
avoid double-destroy through an internal closed flag.

Accessor-returned arrays are borrowed pointers. Python code should not expose
borrowed raw pointers beyond the lifetime of the owner handle. A safe initial
design can copy values in and out through Python sequences, then later add a
buffer or NumPy fast path.

Low-level wrapper shape for a later prototype:

```text
Ruckig(dofs: int, delta_time: float)
Input(dofs: int)
Output(dofs: int)
Trajectory(dofs: int)
```

The first prototype should stay close to the C API: explicit `calculate`,
`update`, `pass_to_input`, vector setters/accessors, and explicit result codes
or exceptions. Higher-level Python convenience classes can be layered after the
handle and array model is proven.

Lifecycle failure modes to test in the prototype:

- Double `close()` or `destroy()` calls must be harmless at the Python wrapper
  level and must not call the C destroy function twice.
- Methods called after close must fail with a Python-side lifecycle error.
- Borrowed arrays must not outlive the owning handle.
- Exceptions during construction must release any C handles already created.
- `with` blocks must close handles deterministically on normal exit and on
  exceptions.

## Array Model

The first prototype should support Python list or tuple copy-in/copy-out for
all fixed-size DoF vectors. It should validate lengths before calling setters
or writing accessor arrays.

An optional fast path can use the buffer protocol or NumPy arrays. NumPy should
not be a hard dependency for the first binding unless the package strategy
explicitly accepts it.

Initial list/tuple behavior:

- Setters copy Python sequence values into the C input arrays.
- Accessors return new Python lists copied from C-owned arrays.
- Length mismatches fail before calling the C API.
- Non-finite values follow the C API validation path unless a setter must reject
  them to avoid unsafe conversion.

Optional fast path:

- Accept writable buffer protocol objects for copy-in.
- Return memoryviews or NumPy arrays only when lifetime rules can be enforced.
- Keep NumPy optional; do not require it for the first wheel.

## Error Model

The binding should preserve the distinction between:

- `RUCKIG_WORKING`
- `RUCKIG_FINISHED`
- `RUCKIG_ERROR_INVALID_INPUT`
- `RUCKIG_ERROR_ZERO_LIMITS`
- execution-time, synchronization, trajectory-duration, positional-limit, and
  unsupported errors.

Feasibility work should decide whether normal calculation methods return a
result enum, raise exceptions for error codes, or use a hybrid model. The
online update loop must keep `RUCKIG_WORKING` and `RUCKIG_FINISHED` as normal
control-flow states rather than exceptions.

Preferred prototype error strategy:

- Return a Python enum for `RUCKIG_WORKING` and `RUCKIG_FINISHED`.
- Raise typed exceptions for error result codes from `calculate`, `update`, and
  validation helpers unless the method is explicitly documented as returning raw
  result codes.
- Preserve the original integer result code on exception objects for debugging
  and compatibility tests.

## Packaging Questions

The design must answer:

- Whether wheels vendor a static `ruckig_c` build or load a shared library.
- How Windows, macOS, and Linux wheels are built.
- How source distributions find or build the C library.
- Whether the package exposes low-level C-like wrappers, higher-level Pythonic
  classes, or both.
- How CI covers Python versions, operating systems, and CPU architectures.
- How MIT license notices and the frozen Ruckig Community baseline source are
  represented.

Packaging tradeoffs:

- Shared-library loading keeps the Python wrapper aligned with the C ABI, but
  it requires reliable library discovery on Windows, macOS, and Linux.
- Vendoring a static or shared `ruckig_c` build into wheels improves user
  installation ergonomics, but it expands CI and license-notice work.
- Source distributions should either build `ruckig_c` from source with CMake or
  document a clear system-library discovery path.

Estimated wheel CI matrix for a prototype:

- Windows x86_64 with a shared `ruckig_c` library.
- macOS x86_64 and arm64 if the build infrastructure can support both.
- Linux x86_64 manylinux-compatible wheels.
- Python versions limited to the active CPython range selected for the
  prototype; broad version coverage should wait until the package strategy is
  proven.

## Decision

The next binding-related step is a Python feasibility prototype design, not
implementation in the `0.2.x` maintenance line. The default prototype route is
`cffi` ABI mode loading a built `ruckig_c` shared library. `ctypes` remains a
fallback if `cffi` packaging friction is higher than expected. CPython
extensions and `pybind11` remain deferred.

Prototype acceptance criteria for a later implementation project:

- Create and destroy `ruckig_t`, `ruckig_input_t`, `ruckig_output_t`, and
  `ruckig_trajectory_t` handles.
- Run offline `ruckig_calculate`.
- Run an online `ruckig_update` loop with `ruckig_output_pass_to_input`.
- Set and read fixed-size DoF vectors through list or tuple copy-in/copy-out.
- Propagate `RUCKIG_WORKING` and `RUCKIG_FINISHED` as normal control flow.
- Map error result codes to a documented Python enum or exception strategy.
- Demonstrate wrapper lifecycle tests with no leaked handles.

The prototype test entry point is `bindings/python_prototype/test_prototype.py`.
It requires `RUCKIG_C_SHARED_LIBRARY` to point at a shared `ruckig_c` build and
is intentionally not part of routine CI until shared-library loading strategy
and public export hygiene are stable.

Local prototype smoke evidence:

```text
Python: 3.14.3 in build_python_proto_venv
cffi: 2.0.0
Library: build_release_check_shared/ruckig_c.dll
Command: python bindings/python_prototype/test_prototype.py
Result: Ran 8 tests in 0.009s, OK
```

Current `0.3.0` prototype coverage includes:

- Handle create/destroy and context-manager cleanup.
- Harmless double `close()` at the Python wrapper level.
- Python-side lifecycle errors for method calls after close.
- Offline `calculate`.
- Online `update` loops with `output_pass_to_input`.
- List and tuple copy-in plus list copy-out.
- Length mismatch checks before writing C-owned arrays.
- Typed Python exceptions for error result codes while preserving the original
  integer result code and operation name.

This expanded coverage still does not approve a released binding API, wheel,
source distribution, CMake install target, or routine CI job.

Rust bindings should remain deferred until Python feasibility clarifies the
ownership, packaging, and error model for one high-level FFI layer.
