# Profile Check Conversion Reference

Document version: `0.1`

Applies to: Ruckig Community `0.17.3` under `original/ruckig-main`

Priority: implementation reference for Phase 2 only. It does not define scope.
If this document conflicts with `docs/c_rewrite_execution_plan.md`, the
execution plan takes precedence.

## Purpose

The original C++ implementation uses template parameters in
`include/ruckig/profile.hpp` to specialize profile validation paths at compile
time. The C rewrite must preserve the same behavior without C++ templates.

This reference explains the conversion strategy. It is not a complete
replacement for `profile.hpp`, and its snippets must be treated as pseudocode.
The actual C implementation must be checked against the original C++ oracle.

## Original Pattern

The original profile checks use template parameters such as:

```cpp
template<ControlSigns control_signs, ReachedLimits limits>
bool check(double jf, double vMax, double vMin, double aMax, double aMin);

profile.check<Profile::ControlSigns::UDDU, Profile::ReachedLimits::VEL>(
    jMax, vMax, vMin, aMax, aMin
);
```

The template parameters affect:

1. Jerk sign sequence (`UDDU` or `UDUD`).
2. Required positive timing segments.
3. Whether velocity, acceleration, or no limit is reached.
4. Special state corrections such as forcing `a[3] = 0`.
5. Which final-state and limit checks are performed.

## Required C Strategy

Do not collapse all original check functions into one generic function during
the first port. Port each original check family as a distinct C function or as
clearly separated branches.

The first correct C port should preserve these check families:

1. `profile_check`
2. `profile_check_with_timing`
3. `profile_check_for_velocity`
4. `profile_check_for_velocity_with_timing`
5. `profile_check_for_second_order`
6. `profile_check_for_second_order_with_timing`
7. `profile_check_for_second_order_velocity`
8. `profile_check_for_second_order_velocity_with_timing`
9. `profile_check_for_first_order`
10. `profile_check_for_first_order_with_timing`

Only after oracle tests pass should any of these functions be merged or
optimized.

## Overload Resolution

The original C++ code overloads each `*_with_timing` family. The shorter
overload accepts `tf` but intentionally does not check it because each candidate
formula already solves for the requested duration. The longer overload performs
an extra bound guard and then calls the shorter overload.

C has no overloads, so the first port should use explicit names:

1. `*_with_timing` mirrors the shorter C++ overload and performs no additional
   jerk, acceleration, or velocity bound guard beyond the underlying profile
   check.
2. `*_with_timing_guarded` mirrors the longer C++ overload. It first applies the
   original guard, then calls the matching `*_with_timing` function.

Examples:

1. `profile_check_with_timing(...)` and
   `profile_check_with_timing_guarded(..., double j_max)`.
2. `profile_check_for_velocity_with_timing(...)` and
   `profile_check_for_velocity_with_timing_guarded(..., double j_max)`.
3. `profile_check_for_second_order_with_timing(...)` and
   `profile_check_for_second_order_with_timing_guarded(..., double a_max, double a_min)`.
4. `profile_check_for_second_order_velocity_with_timing(...)` and
   `profile_check_for_second_order_velocity_with_timing_guarded(..., double a_max, double a_min)`.
5. `profile_check_for_first_order_with_timing(...)` and
   `profile_check_for_first_order_with_timing_guarded(..., double v_max, double v_min)`.

Do not collapse guarded and unguarded behavior into one function unless the
argument contract makes the selected path explicit and oracle tests cover both
paths.

## Enum Dispatch

C code should replace C++ template parameters with internal enums:

```c
typedef enum {
    RUCKIG_PROFILE_SIGNS_UDDU = 0,
    RUCKIG_PROFILE_SIGNS_UDUD = 1
} ruckig_profile_control_signs_t;

typedef enum {
    RUCKIG_PROFILE_LIMITS_ACC0_ACC1_VEL = 0,
    RUCKIG_PROFILE_LIMITS_VEL,
    RUCKIG_PROFILE_LIMITS_ACC0,
    RUCKIG_PROFILE_LIMITS_ACC1,
    RUCKIG_PROFILE_LIMITS_ACC0_ACC1,
    RUCKIG_PROFILE_LIMITS_ACC0_VEL,
    RUCKIG_PROFILE_LIMITS_ACC1_VEL,
    RUCKIG_PROFILE_LIMITS_NONE
} ruckig_profile_reached_limits_t;
```

These enums are internal implementation details. They do not need to appear in
the public C API unless a debug or inspection API is added later.

These values mirror the original `Profile::ReachedLimits` combinations used by
the profile check template specializations. They are not public feature-scope
enumerators and do not map one-to-one to user-facing options. Together with the
required check families listed above, they cover the original profile validation
branches needed by the first-release position and velocity control interfaces,
including directional velocity and acceleration limit handling.

Original `Profile::ReachedLimits` uses implicit C++ enum values in declaration
order. The C port should preserve this order:

| Original value | C value |
| --- | --- |
| `ACC0_ACC1_VEL` | `RUCKIG_PROFILE_LIMITS_ACC0_ACC1_VEL = 0` |
| `VEL` | `RUCKIG_PROFILE_LIMITS_VEL = 1` |
| `ACC0` | `RUCKIG_PROFILE_LIMITS_ACC0 = 2` |
| `ACC1` | `RUCKIG_PROFILE_LIMITS_ACC1 = 3` |
| `ACC0_ACC1` | `RUCKIG_PROFILE_LIMITS_ACC0_ACC1 = 4` |
| `ACC0_VEL` | `RUCKIG_PROFILE_LIMITS_ACC0_VEL = 5` |
| `ACC1_VEL` | `RUCKIG_PROFILE_LIMITS_ACC1_VEL = 6` |
| `NONE` | `RUCKIG_PROFILE_LIMITS_NONE = 7` |

## Jerk Sign Assignment

The original `ControlSigns` template parameter maps to two jerk sign patterns:

```c
static void profile_assign_jerk(
    ruckig_profile_t* profile,
    ruckig_profile_control_signs_t signs,
    double jf
) {
    if (signs == RUCKIG_PROFILE_SIGNS_UDDU) {
        profile->j[0] = profile->t[0] > 0.0 ?  jf : 0.0;
        profile->j[1] = 0.0;
        profile->j[2] = profile->t[2] > 0.0 ? -jf : 0.0;
        profile->j[3] = 0.0;
        profile->j[4] = profile->t[4] > 0.0 ? -jf : 0.0;
        profile->j[5] = 0.0;
        profile->j[6] = profile->t[6] > 0.0 ?  jf : 0.0;
    } else {
        profile->j[0] = profile->t[0] > 0.0 ?  jf : 0.0;
        profile->j[1] = 0.0;
        profile->j[2] = profile->t[2] > 0.0 ? -jf : 0.0;
        profile->j[3] = 0.0;
        profile->j[4] = profile->t[4] > 0.0 ?  jf : 0.0;
        profile->j[5] = 0.0;
        profile->j[6] = profile->t[6] > 0.0 ? -jf : 0.0;
    }
}
```

The odd jerk indices (`j[1]`, `j[3]`, and `j[5]`) correspond to constant
acceleration or constant velocity segments in the UDDU/UDUD profile shape, so
their jerk is zero. This helper only illustrates sign assignment; the actual C
implementation must still preserve the original `set_limits` behavior and any
family-specific state corrections from `profile.hpp`.

This helper is safe to share across check families only if each family still
preserves its original timing and limit rules.

## Timing Segment Checks

The C port must preserve the original `if constexpr` timing checks. For
third-order position checks:

```c
static bool profile_requires_velocity_segment(ruckig_profile_reached_limits_t limits) {
    return limits == RUCKIG_PROFILE_LIMITS_ACC0_ACC1_VEL
        || limits == RUCKIG_PROFILE_LIMITS_ACC0_VEL
        || limits == RUCKIG_PROFILE_LIMITS_ACC1_VEL
        || limits == RUCKIG_PROFILE_LIMITS_VEL;
}

static bool profile_requires_acc0_segment(ruckig_profile_reached_limits_t limits) {
    return limits == RUCKIG_PROFILE_LIMITS_ACC0
        || limits == RUCKIG_PROFILE_LIMITS_ACC0_ACC1;
}

static bool profile_requires_acc1_segment(ruckig_profile_reached_limits_t limits) {
    return limits == RUCKIG_PROFILE_LIMITS_ACC1
        || limits == RUCKIG_PROFILE_LIMITS_ACC0_ACC1;
}
```

Then the corresponding family can apply:

```c
if (profile_requires_velocity_segment(limits) && profile->t[3] < DBL_EPSILON) {
    return false;
}
if (profile_requires_acc0_segment(limits) && profile->t[1] < DBL_EPSILON) {
    return false;
}
if (profile_requires_acc1_segment(limits) && profile->t[5] < DBL_EPSILON) {
    return false;
}
```

Do not reuse these exact rules for second-order, first-order, or velocity
checks without comparing against the original function. Those families have
different `t_sum`, acceleration, velocity, and target checks.

Do not use the timing-segment helper predicates to infer state-correction rules.
For third-order position checks, the original `a[3] = 0.0` correction applies to
`ACC0_ACC1_VEL`, `ACC0_ACC1`, `ACC0_VEL`, `ACC1_VEL`, and `VEL`. This set is
different from the velocity-segment requirement set because `ACC0_ACC1` also
requires the `a[3]` correction even though it does not require a positive
constant-velocity segment. Port this correction as a separate branch.

## Family Mapping

Use this mapping to track porting completeness:

| Original C++ family | C port target | Notes |
| --- | --- | --- |
| `check<signs, limits, set_limits>` | `profile_check(profile, signs, limits, set_limits, ...)` | Third-order position interface. `set_limits` is required only for this family; preserve `set_limits=true` calls from Step 1. |
| `check_with_timing<signs, limits>` | `profile_check_with_timing(...)` and `profile_check_with_timing_guarded(...)` | Guarded variant preserves jerk bound guard. |
| `check_for_velocity<signs, limits>` | `profile_check_for_velocity(...)` | Third-order velocity interface; no position target check. |
| `check_for_velocity_with_timing<signs, limits>` | `profile_check_for_velocity_with_timing(...)` and `profile_check_for_velocity_with_timing_guarded(...)` | Guarded variant preserves jerk bound guard. |
| `check_for_second_order<signs, limits>` | `profile_check_for_second_order(...)` | Position interface with acceleration constraints. |
| `check_for_second_order_with_timing<signs, limits>` | `profile_check_for_second_order_with_timing(...)` and `profile_check_for_second_order_with_timing_guarded(...)` | Guarded variant preserves acceleration bound guard. |
| `check_for_second_order_velocity<signs, limits>` | `profile_check_for_second_order_velocity(...)` | Velocity interface with acceleration constraints. |
| `check_for_second_order_velocity_with_timing<signs, limits>` | `profile_check_for_second_order_velocity_with_timing(...)` and `profile_check_for_second_order_velocity_with_timing_guarded(...)` | Guarded variant preserves acceleration bound guard. |
| `check_for_first_order<signs, limits>` | `profile_check_for_first_order(...)` | Position interface with velocity constraints only. |
| `check_for_first_order_with_timing<signs, limits>` | `profile_check_for_first_order_with_timing(...)` and `profile_check_for_first_order_with_timing_guarded(...)` | Guarded variant preserves velocity bound guard. |

`set_limits=true` is used by original third-order position Step 1 candidates,
not by all profile check families. The C port may represent it as a
`bool set_limits` parameter on `profile_check` or as separate specialized
helpers, but it must preserve the original forced boundary assignments for
`ACC1` and `ACC0_ACC1`.

## Porting Rules

1. Keep the original constants and tolerances unless oracle evidence requires a
   documented adjustment.
2. Preserve branch order when translating formulas from `.cpp` solver files.
3. Preserve special assignments such as `a[3] = 0.0` exactly where the original
   code applies them.
4. Treat `set_limits` behavior in the original C++ template as a required
   branch, not as an optimization.
5. Add regression inputs for every mismatch found by oracle testing.
6. Do not enable unsafe fast math for profile tests.

## Optimization Policy

Initial implementation should prefer clarity and oracle equivalence over speed.

Allowed initial approach:

1. Runtime enum dispatch.
2. Small helper functions for shared sign and timing checks.
3. Distinct functions for each original check family.

Potential later optimizations:

1. Split hot enum combinations into `static inline` specialized functions.
2. Use a function pointer table for stable hot combinations.
3. Generate repeated boilerplate if profiling proves the generic version is a
   bottleneck.

No optimization is acceptable unless fixed and random oracle tests remain
green.
