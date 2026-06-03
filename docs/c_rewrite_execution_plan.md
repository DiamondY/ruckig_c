# Ruckig C Rewrite Execution Plan

Historical note: this document records the original `0.1.0` C rewrite plan and
the first-release scope decisions. It is retained for traceability, but it is no
longer the sole source of truth for current project scope. Current scope is
defined by `README.md`, `include/ruckig_c/ruckig.h`, `docs/roadmap.md`, the
active release checklists, and `docs/upstream_baseline_policy.md`. Per-DoF
control-interface and synchronization overrides were intentionally out of scope
for `0.1.0`, but they are implemented in `0.2.0`; see
`docs/design_per_dof_overrides.md`.

Document version: `0.1`

Applies to: Ruckig Community `0.17.3` under `original/ruckig-main`

Priority: authoritative PRD and execution plan. Technical reference documents
are implementation aids and must not expand first-release scope. If any
technical reference conflicts with this document, this document takes
precedence.

## 1. Purpose

This document defines an executable plan for rewriting the Ruckig 0.17.3 Community code in `original/ruckig-main` as a pure C library.

The final deliverable is a C implementation without a C++ runtime dependency. During implementation, the original C++ code will be kept as an oracle for differential testing. A temporary C ABI wrapper around the original C++ implementation may be built only to generate comparison results and drive tests; it is not the final product.

## 2. Confirmed Decisions

The following decisions are locked for the first implementation pass:

1. Final target: pure C implementation.
2. Development baseline: keep original C++ Ruckig as oracle for differential testing.
3. First release scope: local state-to-state trajectory generation only.
4. Waypoints: do not support `intermediate_positions` in the first release because the Community source delegates waypoint calculation to a cloud API.
5. C API model: runtime dynamic DoF only in the first release.
6. Memory model: allocate during `create`/`destroy` lifecycle paths; do not allocate during `calculate` or `update`.
7. Validation: use strict numerical equivalence against the original C++ implementation.
8. Bindings: do not implement Python or Rust bindings in the first release.
9. Build deliverable: C static/shared library, public C header, C examples, C tests, and CMake integration.

## 3. Source Inventory

Original source root:

`original/ruckig-main`

Important files:

1. Public result codes: `include/ruckig/result.hpp`
2. Public API and update loop: `include/ruckig/ruckig.hpp`
3. Input validation and input data model: `include/ruckig/input_parameter.hpp`
4. Output data model: `include/ruckig/output_parameter.hpp`
5. Trajectory sampling: `include/ruckig/trajectory.hpp`
6. Single-DoF profile state and checks: `include/ruckig/profile.hpp`
7. Calculator dispatcher: `include/ruckig/calculator.hpp`
8. Multi-DoF synchronization and target calculation: `include/ruckig/calculator_target.hpp`
9. Synchronization blocked intervals: `include/ruckig/block.hpp`
10. Polynomial/root utilities: `include/ruckig/roots.hpp`
11. Kinematic helpers: `include/ruckig/utils.hpp`
12. Brake/pre-trajectory logic: `include/ruckig/brake.hpp`, `src/ruckig/brake.cpp`
13. Position solver declarations and implementations:
    `include/ruckig/position.hpp`,
    `src/ruckig/position_first_step1.cpp`,
    `src/ruckig/position_first_step2.cpp`,
    `src/ruckig/position_second_step1.cpp`,
    `src/ruckig/position_second_step2.cpp`,
    `src/ruckig/position_third_step1.cpp`,
    `src/ruckig/position_third_step2.cpp`
14. Velocity solver declarations and implementations:
    `include/ruckig/velocity.hpp`,
    `src/ruckig/velocity_second_step1.cpp`,
    `src/ruckig/velocity_second_step2.cpp`,
    `src/ruckig/velocity_third_step1.cpp`,
    `src/ruckig/velocity_third_step2.cpp`
15. Main tests: `test/test_target.cpp`
16. Benchmark: `test/benchmark_target.cpp`

Out of scope for the first C release:

1. `include/ruckig/calculator_cloud.hpp`
2. `src/ruckig/cloud_client.cpp`
3. `src/wrapper/python.cpp`
4. `src/wrapper/rust.rs`
5. `src/wrapper/rust.hpp`
6. Python, Rust, and package publishing workflows

## 4. First Release Functional Scope

The first pure C release must support:

1. Offline calculation: `ruckig_calculate`
2. Online update loop: `ruckig_update`
3. Runtime dynamic DoF
4. Position control interface
5. Velocity control interface
6. First-order position profiles
7. Second-order position profiles
8. Third-order position profiles
9. Second-order velocity profiles
10. Third-order velocity profiles
11. Braking pre-trajectories for states outside or inevitably crossing limits
12. Directional velocity and acceleration limits
13. Disabled DoFs
14. Synchronization modes:
    `Time`, `TimeIfNecessary`, `Phase`, and `None`
15. Continuous and discrete duration synchronization
16. Global minimum duration
17. Trajectory sampling at arbitrary time
18. Position extrema query
19. First time at position query
20. Result codes matching the original result semantics
21. Input validation matching original validation semantics

The first pure C release must explicitly reject:

1. Intermediate waypoints
2. Per-section constraints
3. Cloud calculation
4. Pro-only local waypoint behavior
5. Python and Rust bindings

Per-section constraints and Pro-only local waypoint behavior are deferred
features, not hidden first-release requirements. The first release makes no
public API commitment for these features. Ruckig Community `0.17.3` does not
include the Pro local waypoint implementation, and this rewrite must not infer
or recreate that proprietary behavior.

The first `0.1.0` release supports global control interface and global
synchronization settings only. Per-DoF control-interface and per-DoF
synchronization overrides from the C++ API were deferred for `0.1.0` and are
implemented separately in `0.2.0`; see `docs/design_per_dof_overrides.md`.

## 5. Proposed C API

Public header:

`include/ruckig_c/ruckig.h`

Version macros:

```c
#define RUCKIG_C_VERSION_MAJOR 0
#define RUCKIG_C_VERSION_MINOR 2
#define RUCKIG_C_VERSION_PATCH 0
#define RUCKIG_C_VERSION_STRING "0.2.0"
```

Core public types:

```c
typedef enum ruckig_result {
    RUCKIG_WORKING = 0,
    RUCKIG_FINISHED = 1,
    RUCKIG_ERROR = -1,
    RUCKIG_ERROR_INVALID_INPUT = -100,
    RUCKIG_ERROR_TRAJECTORY_DURATION = -101,
    RUCKIG_ERROR_POSITIONAL_LIMITS = -102,
    /* Original C++ Result enum jumps from -102 to -104; -103 was never defined. */
    RUCKIG_ERROR_ZERO_LIMITS = -104,
    RUCKIG_ERROR_EXECUTION_TIME_CALCULATION = -110,
    RUCKIG_ERROR_SYNCHRONIZATION_CALCULATION = -111,
    RUCKIG_ERROR_UNSUPPORTED = -200
} ruckig_result_t;

typedef enum ruckig_control_interface {
    RUCKIG_CONTROL_POSITION = 0,
    RUCKIG_CONTROL_VELOCITY = 1
} ruckig_control_interface_t;

typedef enum ruckig_synchronization {
    RUCKIG_SYNCHRONIZATION_TIME = 0,
    RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY = 1,
    RUCKIG_SYNCHRONIZATION_PHASE = 2,
    RUCKIG_SYNCHRONIZATION_NONE = 3
} ruckig_synchronization_t;

typedef enum ruckig_duration_discretization {
    RUCKIG_DURATION_CONTINUOUS = 0,
    RUCKIG_DURATION_DISCRETE = 1
} ruckig_duration_discretization_t;

typedef struct ruckig_position_extrema {
    double min_position;
    double max_position;
    double time_min;
    double time_max;
} ruckig_position_extrema_t;
```

Opaque structs:

```c
typedef struct ruckig_input ruckig_input_t;
typedef struct ruckig_output ruckig_output_t;
typedef struct ruckig_trajectory ruckig_trajectory_t;
typedef struct ruckig ruckig_t;
```

Required lifecycle:

```c
ruckig_result_t ruckig_create(ruckig_t** otg, size_t dofs, double delta_time);
void ruckig_destroy(ruckig_t* otg);

ruckig_result_t ruckig_input_create(ruckig_input_t** input, size_t dofs);
void ruckig_input_destroy(ruckig_input_t* input);

ruckig_result_t ruckig_output_create(ruckig_output_t** output, size_t dofs);
void ruckig_output_destroy(ruckig_output_t* output);

ruckig_result_t ruckig_trajectory_create(ruckig_trajectory_t** trajectory, size_t dofs);
void ruckig_trajectory_destroy(ruckig_trajectory_t* trajectory);
```

Because the public structs are opaque, the public API uses `create`/`destroy`
rather than caller-provided object storage. `create` functions allocate the
handle and all first-release scratch buffers up front. `destroy(NULL)` is a
no-op. Destroying the same handle twice is undefined behavior. `dofs == 0` is
invalid and must return `RUCKIG_ERROR_INVALID_INPUT`.

Required operations:

```c
ruckig_result_t ruckig_validate_input(
    const ruckig_t* otg,
    const ruckig_input_t* input,
    bool check_current_state_within_limits,
    bool check_target_state_within_limits
);

ruckig_result_t ruckig_calculate(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory
);

ruckig_result_t ruckig_update(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_output_t* output
);

void ruckig_reset(ruckig_t* otg);

void ruckig_output_pass_to_input(
    const ruckig_output_t* output,
    ruckig_input_t* input
);
```

`ruckig_output_pass_to_input` copies the latest output kinematic state into the
input current state arrays so the next `ruckig_update` call can continue from
the last output state. It does not transfer ownership, does not keep pointers to
`output`, and must not modify target state, limits, optional constraints,
control interface, synchronization settings, or enabled flags.

Output accessors:

```c
size_t ruckig_output_get_dof_count(const ruckig_output_t* output);

const double* ruckig_output_new_position_data(const ruckig_output_t* output);
const double* ruckig_output_new_velocity_data(const ruckig_output_t* output);
const double* ruckig_output_new_acceleration_data(const ruckig_output_t* output);
const double* ruckig_output_new_jerk_data(const ruckig_output_t* output);

double ruckig_output_get_time(const ruckig_output_t* output);
size_t ruckig_output_get_new_section(const ruckig_output_t* output);
bool ruckig_output_did_section_change(const ruckig_output_t* output);
bool ruckig_output_new_calculation(const ruckig_output_t* output);
bool ruckig_output_was_calculation_interrupted(const ruckig_output_t* output);
double ruckig_output_get_calculation_duration(const ruckig_output_t* output);

const ruckig_trajectory_t* ruckig_output_get_trajectory(const ruckig_output_t* output);
```

`ruckig_output_get_trajectory` returns a read-only pointer owned by `output`.
The pointer remains valid until `ruckig_output_destroy` or the next successful
`ruckig_update` call using the same output object. Callers must not destroy or
mutate this trajectory pointer.

Input accessors:

```c
size_t ruckig_input_get_dof_count(const ruckig_input_t* input);

double* ruckig_input_current_position_data(ruckig_input_t* input);
double* ruckig_input_current_velocity_data(ruckig_input_t* input);
double* ruckig_input_current_acceleration_data(ruckig_input_t* input);
double* ruckig_input_target_position_data(ruckig_input_t* input);
double* ruckig_input_target_velocity_data(ruckig_input_t* input);
double* ruckig_input_target_acceleration_data(ruckig_input_t* input);
double* ruckig_input_max_velocity_data(ruckig_input_t* input);
double* ruckig_input_max_acceleration_data(ruckig_input_t* input);
double* ruckig_input_max_jerk_data(ruckig_input_t* input);
bool* ruckig_input_enabled_data(ruckig_input_t* input);

const double* ruckig_input_current_position_const_data(const ruckig_input_t* input);
const double* ruckig_input_current_velocity_const_data(const ruckig_input_t* input);
const double* ruckig_input_current_acceleration_const_data(const ruckig_input_t* input);
const double* ruckig_input_target_position_const_data(const ruckig_input_t* input);
const double* ruckig_input_target_velocity_const_data(const ruckig_input_t* input);
const double* ruckig_input_target_acceleration_const_data(const ruckig_input_t* input);
const double* ruckig_input_max_velocity_const_data(const ruckig_input_t* input);
const double* ruckig_input_max_acceleration_const_data(const ruckig_input_t* input);
const double* ruckig_input_max_jerk_const_data(const ruckig_input_t* input);
const bool* ruckig_input_enabled_const_data(const ruckig_input_t* input);

ruckig_result_t ruckig_input_set_control_interface(
    ruckig_input_t* input,
    ruckig_control_interface_t control_interface
);

ruckig_result_t ruckig_input_set_synchronization(
    ruckig_input_t* input,
    ruckig_synchronization_t synchronization
);

ruckig_result_t ruckig_input_set_duration_discretization(
    ruckig_input_t* input,
    ruckig_duration_discretization_t duration_discretization
);

ruckig_result_t ruckig_input_set_dof_enabled(
    ruckig_input_t* input,
    size_t dof,
    bool enabled
);

ruckig_result_t ruckig_input_set_min_velocity(
    ruckig_input_t* input,
    const double* min_velocity,
    size_t count
);

void ruckig_input_clear_min_velocity(ruckig_input_t* input);

ruckig_result_t ruckig_input_set_min_acceleration(
    ruckig_input_t* input,
    const double* min_acceleration,
    size_t count
);

void ruckig_input_clear_min_acceleration(ruckig_input_t* input);

ruckig_result_t ruckig_input_set_minimum_duration(
    ruckig_input_t* input,
    double minimum_duration
);

void ruckig_input_clear_minimum_duration(ruckig_input_t* input);
```

The `_data` accessors return pointers to DoF-sized storage owned by `input`.
They must not allocate and remain valid until `ruckig_input_destroy` or a future
explicit resize call. Setter functions for optional vectors copy from caller
memory into preallocated input-owned storage and return
`RUCKIG_ERROR_INVALID_INPUT` if `count` does not match `ruckig_input_get_dof_count`.
Disabled DoFs are represented by the `enabled` vector, which defaults to `true`
for every DoF and can be modified through `ruckig_input_enabled_data` or
`ruckig_input_set_dof_enabled`.

Setter functions that accept enums must return `RUCKIG_ERROR_INVALID_INPUT` for
values outside the defined enum range. Optional-vector setters require
`count == ruckig_input_get_dof_count(input)` and a non-NULL source pointer;
`count == 0` or `NULL` is invalid for setters. Use the matching `clear_*`
function to disable an optional vector.

Trajectory operations:

```c
size_t ruckig_trajectory_get_dof_count(const ruckig_trajectory_t* trajectory);

double ruckig_trajectory_get_duration(const ruckig_trajectory_t* trajectory);

ruckig_result_t ruckig_trajectory_at_time(
    const ruckig_trajectory_t* trajectory,
    double time,
    double* position,
    double* velocity,
    double* acceleration,
    double* jerk,
    size_t* section
);

ruckig_result_t ruckig_trajectory_get_position_extrema(
    const ruckig_trajectory_t* trajectory,
    ruckig_position_extrema_t* extrema,
    size_t extrema_count
);

ruckig_result_t ruckig_trajectory_get_first_time_at_position(
    const ruckig_trajectory_t* trajectory,
    size_t dof,
    double position,
    double time_after,
    double* time,
    bool* found
);
```

`ruckig_trajectory_get_position_extrema` writes one extrema record per DoF and
returns `RUCKIG_ERROR_INVALID_INPUT` if `extrema_count` is smaller than the
trajectory DoF count. Callers can query the required array length with
`ruckig_trajectory_get_dof_count`. `ruckig_trajectory_get_first_time_at_position`
returns `RUCKIG_WORKING` for a successful query, including the case where no
crossing exists. It sets `*found = false` when no crossing is found and sets
`*found = true` plus `*time` when a crossing is found. It returns
`RUCKIG_ERROR_INVALID_INPUT` for an invalid DoF, NULL required output pointer,
or negative `time_after`.

`ruckig_trajectory_at_time` returns `RUCKIG_ERROR_INVALID_INPUT` for
`time < 0.0`. Output arrays must have at least the trajectory DoF count when
provided. `position` must be non-NULL. `velocity`, `acceleration`, `jerk`, and
`section` may be NULL when the caller does not need those outputs.

If `ruckig_calculate` returns an error, the output `trajectory` contents are not
valid for query. The caller must treat that trajectory as unspecified until a
later successful `ruckig_calculate` call writes it again.

The first release must use opaque public structs plus accessor and setter
functions for stable ABI. Public headers must not expose algorithm state or
owned buffer layouts. Any future decision to expose fields must be treated as an
ABI-breaking change unless guarded behind a separate inspection/debug API.

Thread safety:

1. Individual `ruckig_t`, `ruckig_input_t`, `ruckig_output_t`, and
   `ruckig_trajectory_t` instances are not thread-safe for concurrent mutation.
2. Separate instances may be used concurrently by different threads.
3. Read-only trajectory queries may be called concurrently only if no thread is
   mutating or freeing the same trajectory object.
4. Callers must synchronize all shared-object access.
5. `ruckig_calculate` and `ruckig_update` use mutable state in `ruckig_t` and
   must not be called concurrently on the same `ruckig_t` instance.

## 6. Internal C Data Structures

Public handle types declared in `include/ruckig_c/ruckig.h` correspond to full
internal struct definitions in the implementation files. Public headers expose
only opaque declarations, enums, small value structs, and access functions.
Private algorithm state and owned buffer layouts must stay in internal
definitions or private headers.

Port these C++ concepts to C structures:

1. `BrakeProfile` -> `ruckig_brake_profile_t`
2. `Profile` -> `ruckig_profile_t`
3. `Block::Interval` -> `ruckig_block_interval_t`
4. `Block` -> `ruckig_block_t`
5. `TargetCalculator` -> `ruckig_target_calculator_t`
6. `Trajectory` -> `ruckig_trajectory_t`
7. `InputParameter` -> `ruckig_input_t`
8. `OutputParameter` -> `ruckig_output_t`
9. `Ruckig` -> `ruckig_t`

Fixed-size fields inside a single-DoF profile:

```c
double t[7];
double t_sum[7];
double j[7];
double a[8];
double v[8];
double p[8];
```

Dynamic per-DoF fields must be allocated during create:

1. Input vectors
2. Output vectors
3. Trajectory profile array
4. Independent minimum durations
5. Target calculator scratch arrays
6. Current input cache
7. Blocks
8. Synchronization candidate arrays

Optional values must use explicit flags:

```c
bool has_min_velocity;
double* min_velocity;

bool has_min_acceleration;
double* min_acceleration;

bool has_minimum_duration;
double minimum_duration;
```

Optional vector pointers such as `min_velocity` and `min_acceleration` must
point to DoF-sized storage owned by the initialized input object. They must not
be allocated lazily inside `ruckig_calculate`, `ruckig_update`, validation, or
trajectory sampling.

## 7. Implementation Phases

T-shirt sizing is a planning estimate, not an acceptance criterion:

| Phase | Size | Main risk driver |
| --- | --- | --- |
| Phase 0 | S | Repository and CMake setup. |
| Phase 1 | M | Numerical root solver equivalence. |
| Phase 2 | L | Profile checks, `set_limits`, brake behavior, and validation parity. |
| Phase 3 | XL | Formula-heavy single-DoF solvers, especially third-order Step 2. |
| Phase 4 | L | Multi-DoF synchronization and disabled DoF behavior. |
| Phase 5 | L | Trajectory sampling, input cache, and online update semantics. |
| Phase 6 | S | Examples and public documentation. |
| Phase 7 | M | Stress testing, performance baseline, and release hardening. |

### Phase 0: Repository Setup

Tasks:

1. Add `include/ruckig_c/ruckig.h`.
2. Add `src/ruckig_c/`.
3. Add `test/c/`.
4. Add CMake options:
   - `BUILD_RUCKIG_C`
   - `BUILD_RUCKIG_C_TESTS`
   - `BUILD_RUCKIG_C_EXAMPLES`
   - `BUILD_RUCKIG_C_ORACLE_TESTS`
5. Keep original C++ source under `original/` unchanged.
6. Add a temporary oracle target that can call original C++ Ruckig for comparison.

Exit criteria:

1. Empty C library target builds.
2. Public header compiles as C.
3. Public header compiles from C++ in `extern "C"` mode.

### Phase 1: Utility and Root Solver Port

Tasks:

1. Port `integrate`.
2. Port `pow2`.
3. Port cubic solver.
4. Port monic quartic solver.
5. Port polynomial evaluation.
6. Port polynomial derivative.
7. Port safe Newton interval shrink.
8. Add unit tests against selected known roots.
9. Add differential tests by calling original C++ root routines where feasible, or by evaluating polynomial residuals.

Exit criteria:

1. Root solvers return only non-negative roots as original code does.
2. Roots are sorted when iterated or returned.
3. Residuals are within numerical tolerance.
4. No allocations occur inside solver calls.

### Phase 2: Profile, Brake, and Validation

Tasks:

1. Port `ruckig_profile_t`.
2. Port profile boundary setters.
3. Port profile check functions. Use `docs/tech_ref_profile_check_conversion.md` as the implementation checklist for the required `profile_check*` function families.
4. Port internal profile position extrema calculations (`Profile::get_position_extrema`).
5. Port internal profile first-state-at-position helper (`Profile::get_first_state_at_position`).
6. Port `ruckig_brake_profile_t`.
7. Port brake trajectory generation.
8. Port input default initialization behavior inside `ruckig_input_create`.
9. Port input validation.

Exit criteria:

1. Input defaults match original C++ behavior.
2. Validation accepts and rejects the same fixed test cases as original.
3. Brake profile generated values match original on fixed cases.
4. Profile checks preserve original tolerances.
5. Each profile check family has fixed oracle tests for representative
   `ControlSigns` and `ReachedLimits` combinations.
6. Every mismatch found while porting profile checks is reduced to a regression
   input before Phase 2 is closed.

### Phase 3: Single-DoF Solvers

Port in this order:

1. `PositionFirstOrderStep1`
2. `PositionFirstOrderStep2`
3. `PositionSecondOrderStep1`
4. `PositionSecondOrderStep2`
5. `VelocitySecondOrderStep1`
6. `VelocitySecondOrderStep2`
7. `VelocityThirdOrderStep1`
8. `VelocityThirdOrderStep2`
9. `PositionThirdOrderStep1`
10. `PositionThirdOrderStep2`

Rules:

1. Prefer mechanical formula translation over algebraic refactoring.
2. Preserve original signs, branch order, and tolerances.
3. Preserve original fallback sequence.
4. Avoid "simplifying" expressions until after oracle tests pass.

Exit criteria:

1. Single-DoF `calculate` matches original C++ for fixed examples.
2. Random single-DoF tests pass for:
   - position third-order
   - position second-order
   - position first-order
   - velocity third-order
   - velocity second-order
3. No allocations occur after create.

### Phase 4: Multi-DoF Synchronization

Tasks:

1. Port `Block`.
2. Port blocked interval calculation.
3. Port synchronization candidate generation.
4. Port discrete duration rounding.
5. Port limiting DoF selection.
6. Port `Synchronization::None`.
7. Port `Synchronization::Time`.
8. Port `Synchronization::TimeIfNecessary`.
9. Port `Synchronization::Phase`.
10. Port enabled/disabled DoF behavior.
11. Port independent minimum durations.

Exit criteria:

1. Multi-DoF fixed tests match original duration and trajectory samples.
2. Phase synchronization fixed tests match original profile timing.
3. Discrete duration tests produce multiples of `delta_time`.
4. Disabled DoFs keep constant acceleration behavior as original.

### Phase 5: Trajectory and Online Update

Tasks:

1. Port trajectory storage.
2. Port `at_time`.
3. Port `get_duration`.
4. Expose trajectory-level position extrema query using the internal profile extrema calculations from Phase 2.
5. Expose trajectory-level first-time-at-position query (`Trajectory::get_first_time_at_position`) using the internal profile first-state-at-position helper from Phase 2 (`Profile::get_first_state_at_position`). The trajectory-level query returns the first matching time; it does not expose a full state object.
6. Port `ruckig_calculate`.
7. Port `ruckig_update`.
8. Port input cache and input comparison.
9. Port `pass_to_input`.
10. Port calculation duration measurement as optional compile-time feature.

Exit criteria:

1. Integration tests pass for the offline calculation path.
2. Integration tests pass for the online update path.
3. Output state matches original C++ at every sampled step within tolerance.
4. Recalculation behavior matches original when input changes.

### Phase 6: Examples and Documentation

Tasks:

1. Add C example for position control.
2. Add C example for offline calculation.
3. Add C example for velocity control.
4. Add C example for stop trajectory.
5. Add C example for minimum duration.
6. Document unsupported waypoints.
7. Document memory model.
8. Document error codes.
9. Document numerical range assumptions.
10. Add README sections for quick start, memory ownership, thread safety,
    unsupported features, and oracle-test expectations.

Exit criteria:

1. All examples build.
2. All examples run.
3. README or docs explain C API usage without referencing C++ templates.
4. README documents all required input setters/accessors and optional-field
   setters/clearers.

### Phase 7: Stress, Performance, and Release Hardening

Tasks:

1. Port fixed tests from `test/test_target.cpp`.
2. Port random test generators.
3. Add C++ oracle differential test executable.
4. Run at least `100,000` random trajectories for development.
5. Run at least `1,000,000` random trajectories before release.
6. Track average and worst calculation duration.
7. Add ASan/UBSan jobs where available.
8. Add valgrind/memcheck job on Linux where available.
9. Check Windows build with MSVC or clang-cl.
10. Establish a C++ oracle performance baseline before release-candidate
    performance comparisons.

Exit criteria:

1. Fixed tests pass.
2. Random tests pass with fixed seeds and zero unexplained oracle mismatches.
3. Oracle differential tests pass.
4. No sanitizer failures.
5. No calculation-path allocations.
6. Performance regression is measured and documented.
7. Random and stress tests stream oracle comparisons and do not retain all
   sampled trajectory states in memory.
8. Any random-test failure must either become a tracked regression case or be
   documented with a justified tolerance/platform explanation.
9. Performance measurements document compiler, optimization flags, CPU, OS,
   sample count, average duration, percentile or worst-case duration, and
   comparison against the C++ oracle baseline.
10. Release-candidate performance must define and record a provisional threshold
    against the C++ oracle baseline. Initial target: average calculation time no
    worse than `1.5x` the C++ oracle on the same benchmark corpus; p99 and worst
    case must be reported even if they are not release-blocking.

## 8. Numerical Equivalence Requirements

The C implementation must compare against original C++ Ruckig on the same input.

Compare:

1. `Result`
2. Trajectory duration
3. Independent minimum durations
4. Position at sampled times
5. Velocity at sampled times
6. Acceleration at sampled times
7. Jerk at sampled times
8. Section index where relevant
9. Final state
10. Limit adherence

Default tolerances:

1. Position: `1e-8`
2. Velocity: `1e-8`
3. Acceleration: `1e-10`
4. Limit adherence: `1e-12`
5. Duration: use `1e-12` initially. Relax only when a documented C/C++ libm difference or platform-specific oracle mismatch proves a wider tolerance is required.

Sampling strategy:

1. `t = 0`
2. `t = duration`
3. `t = duration / 2`
4. Every profile boundary time
5. Small offsets around boundaries: `boundary +/- 1e-9`
6. At least 16 evenly spaced samples per trajectory

## 9. Unsupported Feature Behavior

For first release (`0.1.0` historical scope):

1. If `intermediate_positions` are passed, return `RUCKIG_ERROR_UNSUPPORTED`.
2. If per-section constraints are passed, return `RUCKIG_ERROR_UNSUPPORTED`.
3. If cloud calculation is requested, return `RUCKIG_ERROR_UNSUPPORTED`.
4. If per-DoF control-interface or synchronization overrides are requested,
   return `RUCKIG_ERROR_UNSUPPORTED`.
5. Do not silently ignore unsupported fields.
6. Document every unsupported field in the public header.

Per-DoF control-interface and synchronization overrides were implemented
separately for `0.2.0`; see `docs/design_per_dof_overrides.md`. Waypoints,
per-section constraints, cloud calculation, and Python/Rust bindings remain
unsupported.

## 10. Memory and Real-Time Requirements

Required:

1. For the first release, all heap allocation must happen in create or destroy functions.
2. `ruckig_calculate` must not allocate.
3. `ruckig_update` must not allocate.
4. Root solvers must use fixed-capacity stack structs.
5. Profile candidate arrays must use fixed capacity.
6. Synchronization scratch arrays must be preallocated based on DoF.
7. API must fail cleanly from create functions if handle or scratch allocation fails.

No public resize API is required for the first release. If explicit resize functions are added in a later release, they may allocate, but the no-allocation rule for `ruckig_calculate`, `ruckig_update`, root solvers, profile solvers, and trajectory sampling remains unchanged.

Recommended debug checks:

1. Add optional allocator hooks in tests to detect forbidden allocations.
2. Add `RUCKIG_C_ENABLE_RUNTIME_ALLOCATION_CHECKS` for development builds.
3. Add a debug or CI allocation-symbol audit where feasible, for example by
   wrapping `malloc`, `calloc`, `realloc`, and `free` or checking linked symbols
   in core objects.

## 11. Build Plan

Minimum build requirements:

1. CMake minimum version: `3.16`.
2. Required external dependencies for the C library: C standard library and
   C99 math library (`libm` on platforms that require explicit linkage).
3. Oracle tests additionally require a C++20 compiler capable of building the
   original Ruckig source.
4. Public headers and the C library must be compiled in C99 mode in CI or the
   local release checklist.

Initial CMake targets:

1. `ruckig_c`
2. `ruckig_c_tests`
3. `ruckig_c_examples`
4. `ruckig_c_oracle_tests`

Option-to-target mapping:

1. `BUILD_RUCKIG_C` enables the main `ruckig_c` library target.
2. `BUILD_RUCKIG_C_TESTS` enables `ruckig_c_tests` and depends on `ruckig_c`.
3. `BUILD_RUCKIG_C_EXAMPLES` enables `ruckig_c_examples` and depends on `ruckig_c`.
4. `BUILD_RUCKIG_C_ORACLE_TESTS` enables `ruckig_c_oracle_tests`, depends on `ruckig_c`, and may additionally build or link the original C++ oracle target.

Recommended compiler modes:

1. C standard: C99 minimum.
2. Warnings:
   - MSVC: `/W4`
   - GCC/Clang: `-Wall -Wextra -Wpedantic`
3. Release math: do not enable unsafe fast math by default.
4. Debug sanitizers on Clang/GCC:
   - AddressSanitizer
   - UndefinedBehaviorSanitizer

Recommended CI matrix:

1. Windows with MSVC or clang-cl.
2. Linux with GCC and Clang.
3. macOS with Clang when available.
4. At least one job building only the pure C library without oracle tests.
5. At least one job building oracle differential tests.

Versioning and ABI policy:

1. Start the C library version at `0.1.0`.
2. Use semantic versioning for source-level releases.
3. Any public ABI break, including exposing or changing public struct fields,
   requires a major version bump once the library reaches `1.0.0`.
4. Before `1.0.0`, ABI breaks are allowed only when documented in release notes.
5. Public symbols must keep the `ruckig_` or `RUCKIG_` prefix.

## 12. Milestones

Milestone A: Skeleton and utilities

Deliverables:

1. CMake target
2. Public header
3. Utility functions
4. Root solver tests

Milestone B: Single-DoF offline

Deliverables:

1. Profile and brake port
2. Single-DoF `calculate`
3. Single-DoF oracle tests

Milestone C: Multi-DoF offline

Deliverables:

1. Synchronization port
2. Multi-DoF `calculate`
3. Phase/time/none fixed tests

Milestone D: Online update

Deliverables:

1. `ruckig_update`
2. Input cache
3. Trajectory stepping
4. C examples

Milestone E: Release candidate

Deliverables:

1. Full fixed test coverage
2. Random tests
3. Oracle differential suite
4. Documentation
5. Performance report

## 13. Risk Register

Risk: Third-order position Step 2 formula errors.

Mitigation:

1. Translate mechanically.
2. Compare every branch against C++ oracle.
3. Add branch-specific regression inputs when mismatches appear.

Risk: Numerical drift due to libm differences.

Mitigation:

1. Keep tolerances aligned with original.
2. Avoid fast math.
3. Test on Windows, Linux, and macOS if available.

Risk: Hidden allocations break real-time behavior.

Mitigation:

1. Allocate all scratch memory in init.
2. Add allocator instrumentation tests.
3. Review every path in `calculate` and `update`.
4. Add allocation-symbol audit where supported by the build platform.

Risk: C API exposes unstable internals.

Mitigation:

1. Prefer opaque structs.
2. Keep profile internals behind debug or inspection API.
3. Stabilize only input, output, trajectory, and result APIs.
4. Treat public field exposure as an ABI-breaking decision.

Risk: Shared objects are used concurrently without synchronization.

Mitigation:

1. Document that individual handles are not thread-safe for concurrent
   mutation.
2. Keep mutable scratch state owned by each `ruckig_t` instance.
3. Add thread-safety examples or notes showing one instance per control thread.

Risk: C99/C11 or platform ABI assumptions drift across compilers.

Mitigation:

1. Compile the public header and C library in C99 mode.
2. Keep C11 features out of public headers unless guarded.
3. Prefer opaque structs to avoid cross-compiler struct-layout ABI exposure.
4. Document version and ABI policy in release notes.

Risk: C rewrite is correct but unacceptably slower than the C++ oracle.

Mitigation:

1. Establish a C++ oracle performance baseline.
2. Measure C performance with documented compiler flags and hardware.
3. Use the release-candidate threshold from Phase 7 to decide whether a
   performance regression is release-blocking after correctness is stable.

Risk: Original C++ oracle source drifts from the documented baseline.

Mitigation:

1. Freeze the oracle baseline to Ruckig Community `0.17.3` under
   `original/ruckig-main`.
2. Do not update `original/ruckig-main` as part of ordinary C rewrite work.
3. Any upstream Ruckig version change requires a separate review, source
   inventory update, tolerance review, and regenerated oracle baseline.

Risk: Scope creep into waypoints or bindings.

Mitigation:

1. Treat waypoints and per-section constraints as unsupported.
2. Defer Python/Rust until C ABI is stable.

## 14. Immediate Next Actions

1. Create `include/ruckig_c/ruckig.h`.
2. Create `src/ruckig_c/ruckig.c` as a skeleton only; full `calculate` and `update` implementation belongs to Phase 5.
3. Create CMake target `ruckig_c`.
4. Add create/destroy lifecycle functions for input, output, trajectory, and otg state.
5. Port `integrate` and root solvers.
6. Add first C unit tests for utilities.
7. Add temporary oracle test harness that can invoke original C++ Ruckig.
8. Begin porting `Profile` and `BrakeProfile`.

---

## 15. Source File Mapping

The implementation should keep the original C++ source under `original/` unchanged and place the pure C rewrite under `include/ruckig_c/`, `src/ruckig_c/`, and `test/c/`.

Use this mapping when creating work items:

| Original C++ source | C rewrite target | Notes |
| --- | --- | --- |
| `include/ruckig/result.hpp` | `include/ruckig_c/ruckig.h` | Public result enum; preserve original numeric values. |
| `include/ruckig/error.hpp` | N/A | C API returns error codes instead of throwing exceptions. |
| `include/ruckig/utils.hpp` | `src/ruckig_c/utils.c` | Port `integrate`, `pow2`, and small math helpers. |
| `include/ruckig/roots.hpp` | `src/ruckig_c/roots.c` | Port cubic, quartic, polynomial, and safe Newton helpers. |
| `include/ruckig/profile.hpp` | `src/ruckig_c/profile.c` | Highest-risk profile state and check logic. |
| `include/ruckig/brake.hpp`, `src/ruckig/brake.cpp` | `src/ruckig_c/brake.c` | Brake/pre-trajectory generation. |
| `include/ruckig/block.hpp` | `src/ruckig_c/block.c` | Synchronization blocked intervals. |
| `include/ruckig/position.hpp`, `src/ruckig/position_*.cpp` | `src/ruckig_c/position_*.c` | Position first-, second-, and third-order solvers. |
| `include/ruckig/velocity.hpp`, `src/ruckig/velocity_*.cpp` | `src/ruckig_c/velocity_*.c` | Velocity second- and third-order solvers. |
| `include/ruckig/input_parameter.hpp` | `src/ruckig_c/input.c` | Input create defaults, validation, comparison, and cleanup. |
| `include/ruckig/output_parameter.hpp` | `src/ruckig_c/output.c` | Output state and `pass_to_input`. |
| `include/ruckig/trajectory.hpp` | `src/ruckig_c/trajectory.c` | Trajectory storage, sampling, extrema, and position lookup. |
| `include/ruckig/calculator_target.hpp`, `include/ruckig/calculator.hpp` | `src/ruckig_c/calculator.c` | Target calculator, synchronization, and dispatcher. |
| `include/ruckig/ruckig.hpp` | `src/ruckig_c/ruckig.c` | Public lifecycle, `calculate`, `update`, and reset. |
| `include/ruckig/calculator_cloud.hpp`, `src/ruckig/cloud_client.cpp` | Out of scope | Waypoint/cloud calculation is not part of the first release. |
| `src/wrapper/python.cpp`, `src/wrapper/rust.*` | Out of scope | Language bindings are deferred until the C ABI is stable. |

The C file names above are targets, not a requirement to copy the C++ file boundaries exactly. Keep formula-heavy solver files split enough to make oracle diffing and review practical.

---

## 16. Requirements Traceability Matrix

Use this matrix to connect first-release requirements to implementation phases,
source areas, and validation strategy.

| Requirement | Phase | Primary source mapping | Validation |
| --- | --- | --- | --- |
| Public C lifecycle and ABI | Phase 0, Phase 5 | `include/ruckig/ruckig.hpp` -> `include/ruckig_c/ruckig.h`, `src/ruckig_c/ruckig.c` | Header C/C++ compile tests, lifecycle unit tests. |
| Runtime dynamic DoF | Phase 0, Phase 4 | `StandardVector<T, DOFs>` usage -> preallocated runtime arrays | Multi-DoF fixed and random oracle tests. |
| Input validation | Phase 2 | `include/ruckig/input_parameter.hpp` -> `src/ruckig_c/input.c` | Fixed accept/reject cases against C++ oracle behavior. |
| Root solvers and math helpers | Phase 1 | `include/ruckig/roots.hpp`, `utils.hpp` -> `roots.c`, `utils.c` | Residual tests and selected oracle comparisons. |
| Profile checks and brake profiles | Phase 2 | `profile.hpp`, `brake.hpp`, `brake.cpp` -> `profile.c`, `brake.c` | Check-family oracle tests and brake fixed cases. |
| Position and velocity single-DoF solvers | Phase 3 | `position_*.cpp`, `velocity_*.cpp` -> `position_*.c`, `velocity_*.c` | Single-DoF fixed and random oracle tests. |
| Multi-DoF synchronization | Phase 4 | `calculator_target.hpp`, `block.hpp` -> `calculator.c`, `block.c` | Phase/time/none/discrete/disabled DoF oracle tests. |
| Trajectory sampling and extrema queries | Phase 5 | `trajectory.hpp` -> `trajectory.c` | Boundary sampling, extrema, first-time query, and online update tests. |
| Unsupported waypoints/cloud/bindings | Phase 0, Phase 6 | `calculator_cloud.hpp`, wrappers -> out of scope | Public API rejects unsupported inputs with `RUCKIG_ERROR_UNSUPPORTED`. |
| Release hardening | Phase 7 | All C targets plus oracle harness | Sanitizers, allocation checks, stress tests, and performance report. |

---

## Appendix B: Technical Reference Documents

The following reference documents were moved from the previous plan version. They contain specific implementation details for C++ to C translation.

These reference documents are implementation aids, not scope-defining documents. If a technical reference conflicts with this execution plan, this execution plan takes precedence.

1. **`docs/tech_ref_profile_check_conversion.md`** - Profile `check<>()` template specialization conversion to C `switch`-based function. Referenced heavily in Phase 2 profile port.

2. **`docs/tech_ref_callback_conversion.md`** - Lambda callback patterns (`state_to_integrate_from`) converted to C function-pointer + userdata patterns. Referenced in Phase 5 trajectory port.

3. **`docs/tech_ref_cpp_to_c_conversion_table.md`** - Quick-lookup equivalence table for C++ STL types, syntax features, math functions, and header replacements. Used during all phases.

---

*End of document.*
