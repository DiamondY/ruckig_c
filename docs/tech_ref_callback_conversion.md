# Callback and Lambda Conversion Reference

Document version: `0.1`

Applies to: Ruckig Community `0.17.3` under `original/ruckig-main`

Priority: implementation reference for Phase 5 only. It does not define public
API scope. If this document conflicts with `docs/c_rewrite_execution_plan.md`,
the execution plan takes precedence.

## Purpose

The original C++ trajectory implementation uses lambdas to share
`state_to_integrate_from` logic across several `Trajectory::at_time` overloads.
The C rewrite has no lambdas or overloads, so the implementation must choose a
C-compatible internal pattern.

The public C API remains array-based:

```c
ruckig_result_t ruckig_trajectory_at_time(
    const ruckig_trajectory_t* trajectory,
    double time,
    double* position,
    double* velocity,
    double* acceleration,
    double* jerk,
    size_t* section
);
```

Do not expose callbacks in the public C API for the first release.

## Original Pattern

The original C++ pattern is conceptually:

```cpp
state_to_integrate_from(time, new_section,
    [&](size_t dof, double t, double p, double v, double a, double j) {
        std::tie(new_position[dof], new_velocity[dof], new_acceleration[dof])
            = integrate(t, p, v, a, j);
        new_jerk[dof] = j;
    }
);
```

The lambda captures output vectors by reference and writes one DoF at a time.

## Preferred First-Port Strategy

Prefer a private helper that directly writes to output arrays. This avoids a
public callback API and keeps the hot path easy to review.

Suggested internal shape:

```c
static ruckig_result_t trajectory_at_time_full(
    const ruckig_trajectory_t* trajectory,
    double time,
    double* position,
    double* velocity,
    double* acceleration,
    double* jerk,
    size_t* section
);
```

Inside the helper:

1. Determine whether `time >= duration`.
2. Locate the active section from cumulative section times.
3. For each DoF, reproduce `Trajectory::state_to_integrate_from` behavior:
   - if `time >= duration`, integrate from the final profile state with constant
     acceleration and zero jerk;
   - if section 0 has a brake pre-trajectory and `time` is inside it, sample the
     correct brake segment before the main profile;
   - otherwise subtract brake duration before sampling the main profile;
   - if the DoF is not time-synchronized and its own profile has ended, integrate
     from the DoF final profile state with constant acceleration and zero jerk;
   - otherwise find the active main profile segment with the cumulative segment
     times.
4. Call `ruckig_integrate`.
5. Write position, velocity, acceleration, jerk, and section.

The original source contains commented-out accel post-trajectory logic. Do not
activate or reinvent that behavior in the first C port; preserve the original
effective behavior and rely on oracle tests before changing it.

This is the simplest route for the first implementation because the public API
already requests all outputs.

## Optional Internal Callback Pattern

If repeated trajectory sampling logic becomes hard to maintain, an internal
callback plus userdata pattern is acceptable:

```c
typedef void (*ruckig_integrate_cb_t)(
    size_t dof,
    double t,
    double p,
    double v,
    double a,
    double j,
    void* user_data
);
```

Example callback:

```c
typedef struct {
    double* position;
    double* velocity;
    double* acceleration;
    double* jerk;
} trajectory_at_time_ctx_t;

static void trajectory_at_time_cb(
    size_t dof,
    double t,
    double p,
    double v,
    double a,
    double j,
    void* user_data
) {
    trajectory_at_time_ctx_t* ctx = (trajectory_at_time_ctx_t*)user_data;

    ruckig_integrate(
        t, p, v, a, j,
        &ctx->position[dof],
        &ctx->velocity[dof],
        &ctx->acceleration[dof]
    );
    ctx->jerk[dof] = j;
}
```

Use this pattern only internally. Do not store `user_data` pointers after the
call returns.

## Performance Notes

1. A function pointer callback may prevent inlining. Do not assume it is zero
   cost.
2. Prefer direct loops for the first port unless the code becomes too
   duplicated.
3. Add single-DoF fast paths only after the generic dynamic-DoF path passes
   oracle tests and profiling shows value.
4. Do not allocate inside trajectory sampling.
5. Do not change the public API to optimize internals.

## Public API Compatibility

The first release should expose one full sampling function:

```c
ruckig_result_t ruckig_trajectory_at_time(
    const ruckig_trajectory_t* trajectory,
    double time,
    double* position,
    double* velocity,
    double* acceleration,
    double* jerk,
    size_t* section
);
```

Optional convenience functions such as position-only sampling can be added
later, but they are not required by the first-release PRD.

## Checklist

Before marking the trajectory port complete:

1. `t = 0` matches current state.
2. `t = duration` matches target state.
3. `t > duration` follows original constant-acceleration behavior.
4. Brake profiles are sampled correctly.
5. Non-time-synchronized DoFs are sampled correctly after their own profile end.
6. Section changes match original behavior.
7. No allocation occurs in `ruckig_trajectory_at_time`.
