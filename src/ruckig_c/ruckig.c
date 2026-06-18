#include "ruckig_c/internal.h"

#include "ruckig_c/interrupt_context.h"
#include "ruckig_c/position_first.h"
#include "ruckig_c/velocity_second.h"

#include <math.h>
#include <string.h>
#ifdef RUCKIG_C_ENABLE_CALCULATION_DURATION
#include <time.h>
#endif

static double calculation_duration_start(void) {
#ifdef RUCKIG_C_ENABLE_CALCULATION_DURATION
    return (double)clock();
#else
    return 0.0;
#endif
}

static double calculation_duration_finish(double start) {
#ifdef RUCKIG_C_ENABLE_CALCULATION_DURATION
    const clock_t stop = clock();
    if (stop == (clock_t)-1) {
        return 0.0;
    }
    return (((double)stop - start) * 1000000.0) / (double)CLOCKS_PER_SEC;
#else
    (void)start;
    return 0.0;
#endif
}

static ruckig_result_t ruckig_trajectory_copy_internal(
    ruckig_trajectory_t* dst,
    const ruckig_trajectory_t* src
) {
    if (!dst || !src || dst->dofs != src->dofs || src->section_count > dst->section_capacity) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    dst->section_count = src->section_count;
    dst->duration = src->duration;
    dst->valid = src->valid;
    memcpy(dst->profiles, src->profiles, sizeof(ruckig_profile_t) * src->section_count * src->dofs);
    memcpy(dst->blocks, src->blocks, sizeof(ruckig_block_t) * src->dofs);
    memcpy(dst->independent_min_durations, src->independent_min_durations, sizeof(double) * src->dofs);
    memcpy(dst->cumulative_times, src->cumulative_times, sizeof(double) * src->section_count);
    return RUCKIG_WORKING;
}

static void record_operation_result_diagnostics(
    ruckig_diagnostics_t* diagnostics,
    ruckig_result_t result,
    ruckig_diagnostic_scope_t scope
) {
    const ruckig_diagnostic_code_t code = ruckig_diagnostic_code_from_result(result);
    if (code == RUCKIG_DIAGNOSTIC_NONE) {
        ruckig_diagnostics_clear(diagnostics, result, RUCKIG_DIAGNOSTIC_SCOPE_NONE);
    } else {
        ruckig_diagnostics_record(diagnostics, result, scope, code, 0u, 0u, 0u, 0u, 0.0, 0.0);
    }
}

static bool waypoint_update_capacity_mismatch(
    const ruckig_t* otg,
    const ruckig_input_t* input,
    const ruckig_output_t* output,
    size_t* expected_count,
    size_t* actual_count
) {
    size_t capacity;
    if (!otg || !input || !output || !output->trajectory || input->waypoint_count == 0) {
        return false;
    }
    capacity = otg->max_number_of_waypoints;
    if (input->max_number_of_waypoints < capacity) {
        capacity = input->max_number_of_waypoints;
    }
    if (output->trajectory->max_number_of_waypoints < capacity) {
        capacity = output->trajectory->max_number_of_waypoints;
    }
    if (expected_count) {
        *expected_count = capacity;
    }
    if (actual_count) {
        *actual_count = input->waypoint_count;
    }
    return input->waypoint_count > capacity;
}

static void record_waypoint_resume_mismatch_diagnostics(
    ruckig_diagnostics_t* diagnostics,
    const ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_result_t result
) {
    ruckig_diagnostics_record(
        diagnostics,
        result,
        RUCKIG_DIAGNOSTIC_SCOPE_WAYPOINT,
        RUCKIG_DIAGNOSTIC_RESUME_IDENTITY_MISMATCH,
        0u,
        0u,
        otg && otg->waypoint_engine.identity_input ? otg->waypoint_engine.identity_input->waypoint_count : 0u,
        input ? input->waypoint_count : 0u,
        0.0,
        0.0
    );
}

static bool input_is_first_order_position(const ruckig_input_t* input) {
    size_t i;
    if (!input) {
        return false;
    }
    for (i = 0; i < input->dofs; ++i) {
        if ((input->has_per_dof_control_interface ? input->per_dof_control_interface[i] : input->control_interface) != RUCKIG_CONTROL_POSITION) {
            return false;
        }
        if (!isinf(input->max_acceleration[i]) || !isinf(input->max_jerk[i])) {
            return false;
        }
    }
    return true;
}

static bool input_is_no_jerk_position(const ruckig_input_t* input) {
    size_t i;
    if (!input) {
        return false;
    }
    for (i = 0; i < input->dofs; ++i) {
        if ((input->has_per_dof_control_interface ? input->per_dof_control_interface[i] : input->control_interface) != RUCKIG_CONTROL_POSITION) {
            return false;
        }
        if (!isinf(input->max_jerk[i])) {
            return false;
        }
    }
    return true;
}

static bool input_is_no_jerk_velocity(const ruckig_input_t* input) {
    size_t i;
    if (!input) {
        return false;
    }
    for (i = 0; i < input->dofs; ++i) {
        if ((input->has_per_dof_control_interface ? input->per_dof_control_interface[i] : input->control_interface) != RUCKIG_CONTROL_VELOCITY) {
            return false;
        }
        if (!isinf(input->max_jerk[i])) {
            return false;
        }
    }
    return true;
}

static ruckig_control_interface_t effective_control_interface(const ruckig_input_t* input, size_t dof) {
    return input->has_per_dof_control_interface ? input->per_dof_control_interface[dof] : input->control_interface;
}

static ruckig_synchronization_t effective_synchronization(const ruckig_input_t* input, size_t dof) {
    return input->has_per_dof_synchronization ? input->per_dof_synchronization[dof] : input->synchronization;
}

static double round_up_to_discrete_duration(double duration, double delta_time) {
    const double eps = RUCKIG_DBL_EPSILON;
    double remainder;
    if (delta_time <= 0.0 || isinf(duration)) {
        return duration;
    }
    remainder = fmod(duration, delta_time);
    if (remainder > eps) {
        duration += delta_time - remainder;
    }
    return duration;
}

static bool should_skip_time_synchronization(const ruckig_input_t* input, size_t dof) {
    const double eps = RUCKIG_DBL_EPSILON;
    const ruckig_synchronization_t synchronization = effective_synchronization(input, dof);
    if (synchronization == RUCKIG_SYNCHRONIZATION_NONE
        && input->duration_discretization == RUCKIG_DURATION_CONTINUOUS) {
        return true;
    }
    if (synchronization == RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY
        && fabs(input->target_velocity[dof]) < eps
        && fabs(input->target_acceleration[dof]) < eps) {
        return true;
    }
    return false;
}

static bool has_synchronized_dof(const ruckig_input_t* input) {
    size_t dof;
    for (dof = 0; dof < input->dofs; ++dof) {
        if (input->enabled[dof] && effective_synchronization(input, dof) != RUCKIG_SYNCHRONIZATION_NONE) {
            return true;
        }
    }
    return false;
}

static bool has_phase_synchronized_dof(const ruckig_input_t* input) {
    size_t dof;
    for (dof = 0; dof < input->dofs; ++dof) {
        if (input->enabled[dof] && effective_synchronization(input, dof) == RUCKIG_SYNCHRONIZATION_PHASE) {
            return true;
        }
    }
    return false;
}

static bool all_synchronized_dofs_are_phase_or_none(const ruckig_input_t* input) {
    size_t dof;
    for (dof = 0; dof < input->dofs; ++dof) {
        const ruckig_synchronization_t synchronization = effective_synchronization(input, dof);
        if (input->enabled[dof]
            && synchronization != RUCKIG_SYNCHRONIZATION_PHASE
            && synchronization != RUCKIG_SYNCHRONIZATION_NONE) {
            return false;
        }
    }
    return true;
}

static double finalize_trajectory_duration(
    const ruckig_t* otg,
    const ruckig_input_t* input,
    double max_independent_duration
) {
    double duration = max_independent_duration;

    if (!has_synchronized_dof(input)) {
        duration = input->has_minimum_duration ? input->minimum_duration : 0.0;
        if (input->duration_discretization == RUCKIG_DURATION_DISCRETE) {
            duration = round_up_to_discrete_duration(duration, otg->delta_time);
        }
        return max_independent_duration > duration ? max_independent_duration : duration;
    }

    if (input->has_minimum_duration && input->minimum_duration > duration) {
        duration = input->minimum_duration;
    }
    if (input->duration_discretization == RUCKIG_DURATION_DISCRETE) {
        duration = round_up_to_discrete_duration(duration, otg->delta_time);
    }
    return duration;
}

static void select_limiting_dof_for_duration(
    const ruckig_t* otg,
    const ruckig_input_t* input,
    const ruckig_trajectory_t* trajectory,
    double duration,
    size_t* limiting_dof
) {
    const double eps = RUCKIG_DBL_EPSILON;
    size_t dof;

    if (!input || !trajectory || !limiting_dof || input->duration_discretization != RUCKIG_DURATION_DISCRETE) {
        return;
    }

    for (dof = 0; dof < input->dofs; ++dof) {
        double candidate;
        if (!input->enabled[dof] || effective_synchronization(input, dof) == RUCKIG_SYNCHRONIZATION_NONE) {
            continue;
        }
        candidate = round_up_to_discrete_duration(trajectory->independent_min_durations[dof], otg->delta_time);
        if (fabs(candidate - duration) < 2.0 * eps) {
            *limiting_dof = dof;
        }
    }

    if (input->has_minimum_duration
        && fabs(round_up_to_discrete_duration(input->minimum_duration, otg->delta_time) - duration) < 2.0 * eps) {
        *limiting_dof = input->dofs;
    }
}

static double adjust_duration_for_blocks(
    double duration,
    const ruckig_input_t* input,
    const ruckig_trajectory_t* trajectory
) {
    bool changed = true;
    const double eps = RUCKIG_DBL_EPSILON;

    if (!has_synchronized_dof(input)
        && input->duration_discretization == RUCKIG_DURATION_CONTINUOUS) {
        return duration;
    }

    /* Step1 profiles define forbidden synchronization intervals; advance to the next valid synchronization duration candidate. */
    while (changed) {
        size_t dof;
        changed = false;
        for (dof = 0; dof < input->dofs; ++dof) {
            const ruckig_block_t* block = &trajectory->blocks[dof];
            double adjusted = duration;

            if (!input->enabled[dof]
                || effective_synchronization(input, dof) == RUCKIG_SYNCHRONIZATION_NONE
                || !block->valid) {
                continue;
            }
            if (adjusted < block->t_min) {
                adjusted = block->t_min;
            }
            if (block->a.valid && block->a.left < adjusted && adjusted < block->a.right) {
                adjusted = block->a.right;
            }
            if (block->b.valid && block->b.left < adjusted && adjusted < block->b.right) {
                adjusted = block->b.right;
            }
            if (adjusted > duration + 2.0 * eps) {
                duration = adjusted;
                changed = true;
            }
        }
    }

    return duration;
}

static bool select_block_profile_for_duration(
    ruckig_profile_t* profile,
    const ruckig_block_t* block,
    double duration
) {
    const double eps = RUCKIG_DBL_EPSILON;
    const ruckig_profile_t* base_profile;

    if (!profile || !block || !block->valid) {
        return false;
    }
    if (fabs(duration - block->t_min) < 2.0 * eps) {
        *profile = block->p_min;
        return true;
    }
    if (block->a.valid && fabs(duration - block->a.right) < 2.0 * eps) {
        *profile = block->a.profile;
        return true;
    }
    if (block->b.valid && fabs(duration - block->b.right) < 2.0 * eps) {
        *profile = block->b.profile;
        return true;
    }

    base_profile = ruckig_block_get_profile(block, duration);
    if (base_profile) {
        *profile = *base_profile;
    }
    return false;
}

static void update_sync_duration_candidate(
    const ruckig_input_t* input,
    size_t dof,
    double independent_duration,
    double* sync_duration,
    size_t* limiting_dof
) {
    if (!input->enabled[dof]) {
        return;
    }
    if (!has_synchronized_dof(input) || effective_synchronization(input, dof) != RUCKIG_SYNCHRONIZATION_NONE) {
        if (independent_duration > *sync_duration) {
            *sync_duration = independent_duration;
            *limiting_dof = dof;
        }
    }
}

static void apply_none_synchronization_duration(
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    double* duration,
    size_t* limiting_dof
) {
    size_t dof;
    for (dof = 0; dof < input->dofs; ++dof) {
        if (!input->enabled[dof] || effective_synchronization(input, dof) != RUCKIG_SYNCHRONIZATION_NONE) {
            continue;
        }
        if (trajectory->independent_min_durations[dof] > *duration) {
            *duration = trajectory->independent_min_durations[dof];
            *limiting_dof = dof;
        }
        if (trajectory->blocks[dof].valid) {
            trajectory->profiles[dof] = trajectory->blocks[dof].p_min;
        }
    }
}

static void set_disabled_profile_state(ruckig_profile_t* profile, const ruckig_input_t* input, size_t dof) {
    profile->p[0] = 0.0;
    profile->v[0] = 0.0;
    profile->a[0] = 0.0;
    profile->p[7] = input->current_position[dof];
    profile->v[7] = input->current_velocity[dof];
    profile->a[7] = input->current_acceleration[dof];
    profile->pf = 0.0;
    profile->vf = 0.0;
    profile->af = 0.0;
}

static bool calculate_position_phase_sync(
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    double sync_duration,
    size_t limiting_dof
);

static bool calculate_no_jerk_position_phase_sync(
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    double sync_duration,
    size_t limiting_dof
);

static bool calculate_velocity_phase_sync(
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    double sync_duration,
    size_t limiting_dof
);

typedef void (*ruckig_calculate_prepare_profile_fn)(
    const ruckig_input_t* input,
    size_t dof,
    ruckig_profile_t* profile
);

typedef ruckig_result_t (*ruckig_calculate_step1_fn)(
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    size_t dof,
    double* duration
);

typedef ruckig_result_t (*ruckig_calculate_step2_fn)(
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    size_t dof,
    double sync_duration
);

typedef bool (*ruckig_calculate_phase_sync_fn)(
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    double sync_duration,
    size_t limiting_dof
);

typedef struct ruckig_calculate_ops {
    ruckig_calculate_prepare_profile_fn prepare_profile;
    ruckig_calculate_step1_fn step1;
    ruckig_calculate_step2_fn step2;
    ruckig_calculate_phase_sync_fn phase_sync;
    bool adjust_duration_for_blocks;
} ruckig_calculate_ops_t;

static void prepare_position_profile(const ruckig_input_t* input, size_t dof, ruckig_profile_t* profile) {
    ruckig_profile_set_boundary(
        profile,
        input->current_position[dof],
        input->current_velocity[dof],
        input->current_acceleration[dof],
        input->target_position[dof],
        input->target_velocity[dof],
        input->target_acceleration[dof]
    );
}

static void prepare_velocity_profile(const ruckig_input_t* input, size_t dof, ruckig_profile_t* profile) {
    ruckig_profile_set_boundary_for_velocity(
        profile,
        input->current_position[dof],
        input->current_velocity[dof],
        input->current_acceleration[dof],
        input->target_velocity[dof],
        input->target_acceleration[dof]
    );
}

static void prepare_mixed_profile(const ruckig_input_t* input, size_t dof, ruckig_profile_t* profile) {
    if (effective_control_interface(input, dof) == RUCKIG_CONTROL_POSITION) {
        prepare_position_profile(input, dof, profile);
    } else {
        prepare_velocity_profile(input, dof, profile);
    }
}

static void finalize_calculated_trajectory(ruckig_trajectory_t* trajectory, double sync_duration) {
    trajectory->duration = sync_duration;
    trajectory->cumulative_times[0] = sync_duration;
    trajectory->valid = true;
}

static ruckig_result_t calculate_first_order_position_step1(
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    size_t dof,
    double* duration
) {
    ruckig_profile_t* profile = &trajectory->profiles[dof];
    const double v_min = input->has_min_velocity ? input->min_velocity[dof] : -input->max_velocity[dof];

    if (!ruckig_position_first_step1_get_profile(
            profile,
            profile,
            &trajectory->blocks[dof],
            duration,
            profile->p[0],
            profile->pf,
            input->max_velocity[dof],
            v_min
        )) {
        return input->max_velocity[dof] == 0.0 || v_min == 0.0 ? RUCKIG_ERROR_ZERO_LIMITS : RUCKIG_ERROR_EXECUTION_TIME_CALCULATION;
    }
    return RUCKIG_WORKING;
}

static ruckig_result_t calculate_first_order_position_step2(
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    size_t dof,
    double sync_duration
) {
    ruckig_profile_t* profile = &trajectory->profiles[dof];
    const double v_min = input->has_min_velocity ? input->min_velocity[dof] : -input->max_velocity[dof];

    if (!ruckig_position_first_step2_get_profile(
            profile,
            sync_duration,
            profile->p[0],
            profile->pf,
            input->max_velocity[dof],
            v_min
        )) {
        return RUCKIG_ERROR_SYNCHRONIZATION_CALCULATION;
    }
    return RUCKIG_WORKING;
}

static ruckig_result_t calculate_no_jerk_position_step1(
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    size_t dof,
    double* duration
) {
    ruckig_profile_t* profile = &trajectory->profiles[dof];
    const double v_min = input->has_min_velocity ? input->min_velocity[dof] : -input->max_velocity[dof];
    const bool first_order = isinf(input->max_acceleration[dof]);

    if (first_order) {
        return calculate_first_order_position_step1(input, trajectory, dof, duration);
    }

    {
        const double a_min = input->has_min_acceleration ? input->min_acceleration[dof] : -input->max_acceleration[dof];
        ruckig_brake_get_second_order_position_trajectory(
            &profile->brake,
            input->current_velocity[dof],
            input->max_velocity[dof],
            v_min,
            input->max_acceleration[dof],
            a_min
        );
        ruckig_brake_finalize_second_order(&profile->brake, &profile->p[0], &profile->v[0], &profile->a[0]);
        if (!ruckig_position_second_step1_get_profile(
                profile,
                profile,
                &trajectory->blocks[dof],
                duration,
                profile->p[0],
                profile->v[0],
                profile->pf,
                profile->vf,
                input->max_velocity[dof],
                v_min,
                input->max_acceleration[dof],
                a_min
            )) {
            return input->max_acceleration[dof] == 0.0 || a_min == 0.0 ? RUCKIG_ERROR_ZERO_LIMITS : RUCKIG_ERROR_EXECUTION_TIME_CALCULATION;
        }
    }

    return RUCKIG_WORKING;
}

static ruckig_result_t calculate_no_jerk_position_step2(
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    size_t dof,
    double sync_duration
) {
    ruckig_profile_t* profile = &trajectory->profiles[dof];
    const double v_min = input->has_min_velocity ? input->min_velocity[dof] : -input->max_velocity[dof];
    const bool first_order = isinf(input->max_acceleration[dof]);
    const double t_profile = sync_duration - profile->brake.duration - profile->accel.duration;

    if (first_order) {
        if (!ruckig_position_first_step2_get_profile(profile, t_profile, profile->p[0], profile->pf, input->max_velocity[dof], v_min)) {
            return RUCKIG_ERROR_SYNCHRONIZATION_CALCULATION;
        }
    } else {
        const double a_min = input->has_min_acceleration ? input->min_acceleration[dof] : -input->max_acceleration[dof];
        if (select_block_profile_for_duration(profile, &trajectory->blocks[dof], sync_duration)) {
            return RUCKIG_WORKING;
        }
        if (!ruckig_position_second_step2_get_profile(profile, t_profile, profile->p[0], profile->v[0], profile->pf, profile->vf, input->max_velocity[dof], v_min, input->max_acceleration[dof], a_min)) {
            return RUCKIG_ERROR_SYNCHRONIZATION_CALCULATION;
        }
    }

    return RUCKIG_WORKING;
}

static ruckig_result_t calculate_no_jerk_velocity_step1(
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    size_t dof,
    double* duration
) {
    ruckig_profile_t* profile = &trajectory->profiles[dof];
    const double a_min = input->has_min_acceleration ? input->min_acceleration[dof] : -input->max_acceleration[dof];

    ruckig_brake_get_second_order_velocity_trajectory(&profile->brake);
    if (!ruckig_velocity_second_step1_get_profile(profile, profile, duration, profile->v[0], profile->vf, input->max_acceleration[dof], a_min)) {
        return input->max_acceleration[dof] == 0.0 || a_min == 0.0 ? RUCKIG_ERROR_ZERO_LIMITS : RUCKIG_ERROR_EXECUTION_TIME_CALCULATION;
    }
    return RUCKIG_WORKING;
}

static ruckig_result_t calculate_no_jerk_velocity_step2(
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    size_t dof,
    double sync_duration
) {
    ruckig_profile_t* profile = &trajectory->profiles[dof];
    const double a_min = input->has_min_acceleration ? input->min_acceleration[dof] : -input->max_acceleration[dof];
    const double t_profile = sync_duration - profile->brake.duration - profile->accel.duration;

    if (!ruckig_velocity_second_step2_get_profile(profile, t_profile, profile->v[0], profile->vf, input->max_acceleration[dof], a_min)) {
        return RUCKIG_ERROR_SYNCHRONIZATION_CALCULATION;
    }
    return RUCKIG_WORKING;
}

static ruckig_result_t calculate_velocity_step1(
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    size_t dof,
    double* duration
) {
    ruckig_profile_t* profile = &trajectory->profiles[dof];
    const double a_min = input->has_min_acceleration ? input->min_acceleration[dof] : -input->max_acceleration[dof];
    const bool second_order = isinf(input->max_jerk[dof]);

    if (second_order) {
        return calculate_no_jerk_velocity_step1(input, trajectory, dof, duration);
    }

    ruckig_brake_get_velocity_trajectory(&profile->brake, input->current_acceleration[dof], input->max_acceleration[dof], a_min, input->max_jerk[dof]);
    ruckig_brake_finalize(&profile->brake, &profile->p[0], &profile->v[0], &profile->a[0]);
    if (!ruckig_velocity_third_step1_get_profile(
            profile,
            profile,
            &trajectory->blocks[dof],
            duration,
            profile->v[0],
            profile->a[0],
            profile->vf,
            profile->af,
            input->max_acceleration[dof],
            a_min,
            input->max_jerk[dof]
        )) {
        return input->max_jerk[dof] == 0.0 ? RUCKIG_ERROR_ZERO_LIMITS : RUCKIG_ERROR_EXECUTION_TIME_CALCULATION;
    }
    return RUCKIG_WORKING;
}

static ruckig_result_t calculate_velocity_step2(
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    size_t dof,
    double sync_duration
) {
    ruckig_profile_t* profile = &trajectory->profiles[dof];
    const double a_min = input->has_min_acceleration ? input->min_acceleration[dof] : -input->max_acceleration[dof];
    const bool second_order = isinf(input->max_jerk[dof]);
    const double t_profile = sync_duration - profile->brake.duration - profile->accel.duration;

    if (second_order) {
        return calculate_no_jerk_velocity_step2(input, trajectory, dof, sync_duration);
    }

    if (select_block_profile_for_duration(profile, &trajectory->blocks[dof], sync_duration)) {
        return RUCKIG_WORKING;
    }
    if (!ruckig_velocity_third_step2_get_profile(profile, t_profile, profile->v[0], profile->a[0], profile->vf, profile->af, input->max_acceleration[dof], a_min, input->max_jerk[dof])) {
        return RUCKIG_ERROR_SYNCHRONIZATION_CALCULATION;
    }
    return RUCKIG_WORKING;
}

static ruckig_result_t calculate_position_step1(
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    size_t dof,
    double* duration
) {
    ruckig_profile_t* profile = &trajectory->profiles[dof];
    const ruckig_control_interface_t control_interface = effective_control_interface(input, dof);
    const double v_min = input->has_min_velocity ? input->min_velocity[dof] : -input->max_velocity[dof];
    const double a_min = input->has_min_acceleration ? input->min_acceleration[dof] : -input->max_acceleration[dof];
    const bool first_order = isinf(input->max_acceleration[dof]);
    const bool second_order = isinf(input->max_jerk[dof]);

    if (control_interface == RUCKIG_CONTROL_POSITION && first_order) {
        return calculate_first_order_position_step1(input, trajectory, dof, duration);
    } else if (control_interface == RUCKIG_CONTROL_POSITION && second_order) {
        return calculate_no_jerk_position_step1(input, trajectory, dof, duration);
    } else if (control_interface == RUCKIG_CONTROL_POSITION) {
        ruckig_brake_get_position_trajectory(
            &profile->brake,
            input->current_velocity[dof],
            input->current_acceleration[dof],
            input->max_velocity[dof],
            v_min,
            input->max_acceleration[dof],
            a_min,
            input->max_jerk[dof]
        );
        ruckig_brake_finalize(&profile->brake, &profile->p[0], &profile->v[0], &profile->a[0]);
        if (!ruckig_position_third_step1_get_profile(
                profile,
                profile,
                &trajectory->blocks[dof],
                duration,
                profile->p[0],
                profile->v[0],
                profile->a[0],
                profile->pf,
                profile->vf,
                profile->af,
                input->max_velocity[dof],
                v_min,
                input->max_acceleration[dof],
                a_min,
                input->max_jerk[dof]
            )) {
            return input->max_jerk[dof] == 0.0 ? RUCKIG_ERROR_ZERO_LIMITS : RUCKIG_ERROR_EXECUTION_TIME_CALCULATION;
        }
    } else if (second_order) {
        return calculate_no_jerk_velocity_step1(input, trajectory, dof, duration);
    } else {
        return calculate_velocity_step1(input, trajectory, dof, duration);
    }

    return RUCKIG_WORKING;
}

static ruckig_result_t calculate_position_step2(
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    size_t dof,
    double sync_duration
) {
    ruckig_profile_t* profile = &trajectory->profiles[dof];
    const ruckig_control_interface_t control_interface = effective_control_interface(input, dof);
    const double v_min = input->has_min_velocity ? input->min_velocity[dof] : -input->max_velocity[dof];
    const double a_min = input->has_min_acceleration ? input->min_acceleration[dof] : -input->max_acceleration[dof];
    const bool first_order = isinf(input->max_acceleration[dof]);
    const bool second_order = isinf(input->max_jerk[dof]);
    const double t_profile = sync_duration - profile->brake.duration - profile->accel.duration;

    if (control_interface == RUCKIG_CONTROL_POSITION && first_order) {
        return calculate_no_jerk_position_step2(input, trajectory, dof, sync_duration);
    } else if (control_interface == RUCKIG_CONTROL_POSITION && second_order) {
        return calculate_no_jerk_position_step2(input, trajectory, dof, sync_duration);
    } else if (control_interface == RUCKIG_CONTROL_POSITION) {
        if (select_block_profile_for_duration(profile, &trajectory->blocks[dof], sync_duration)) {
            return RUCKIG_WORKING;
        }
        if (!ruckig_position_third_step2_get_profile(
                profile,
                t_profile,
                profile->p[0],
                profile->v[0],
                profile->a[0],
                profile->pf,
                profile->vf,
                profile->af,
                input->max_velocity[dof],
                v_min,
                input->max_acceleration[dof],
                a_min,
                input->max_jerk[dof]
            )) {
            return RUCKIG_ERROR_SYNCHRONIZATION_CALCULATION;
        }
    } else if (second_order) {
        return calculate_no_jerk_velocity_step2(input, trajectory, dof, sync_duration);
    } else {
        return calculate_velocity_step2(input, trajectory, dof, sync_duration);
    }

    return RUCKIG_WORKING;
}

static ruckig_result_t calculate_with_ops(
    const ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    const ruckig_calculate_ops_t* ops
) {
    size_t dof;
    size_t limiting_dof = 0;
    double sync_duration = 0.0;

    for (dof = 0; dof < input->dofs; ++dof) {
        ruckig_profile_t* profile = &trajectory->profiles[dof];
        double duration = 0.0;
        ruckig_result_t result;

        ruckig_profile_init(profile);
        ops->prepare_profile(input, dof, profile);

        if (!input->enabled[dof]) {
            set_disabled_profile_state(profile, input, dof);
            trajectory->independent_min_durations[dof] = 0.0;
            continue;
        }

        result = ops->step1(input, trajectory, dof, &duration);
        if (result != RUCKIG_WORKING) {
            return result;
        }

        trajectory->independent_min_durations[dof] = duration;
        update_sync_duration_candidate(input, dof, duration, &sync_duration, &limiting_dof);
    }

    sync_duration = finalize_trajectory_duration(otg, input, sync_duration);
    select_limiting_dof_for_duration(otg, input, trajectory, sync_duration, &limiting_dof);
    if (ops->adjust_duration_for_blocks) {
        sync_duration = adjust_duration_for_blocks(sync_duration, input, trajectory);
    }
    apply_none_synchronization_duration(input, trajectory, &sync_duration, &limiting_dof);

    if (ops->phase_sync
        && ops->phase_sync(input, trajectory, sync_duration, limiting_dof)
        && all_synchronized_dofs_are_phase_or_none(input)) {
        finalize_calculated_trajectory(trajectory, sync_duration);
        return RUCKIG_WORKING;
    }

    for (dof = 0; dof < input->dofs; ++dof) {
        const bool skip_time_sync = should_skip_time_synchronization(input, dof);
        const double own_duration = trajectory->independent_min_durations[dof];
        ruckig_result_t result;

        if (!input->enabled[dof] || skip_time_sync || fabs(sync_duration - own_duration) < RUCKIG_TIME_EPS) {
            continue;
        }

        result = ops->step2(input, trajectory, dof, sync_duration);
        if (result != RUCKIG_WORKING) {
            return result;
        }
    }

    finalize_calculated_trajectory(trajectory, sync_duration);
    return RUCKIG_WORKING;
}

static ruckig_result_t calculate_first_order_position(
    const ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory
) {
    static const ruckig_calculate_ops_t ops = {
        prepare_position_profile,
        calculate_first_order_position_step1,
        calculate_first_order_position_step2,
        calculate_position_phase_sync,
        false
    };
    return calculate_with_ops(otg, input, trajectory, &ops);
}

static bool calculate_no_jerk_position_phase_sync(
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    double sync_duration,
    size_t limiting_dof
) {
    size_t dof;
    ruckig_profile_t* limiting_profile;
    double pd_limiting;
    double control_limiting;

    if (!has_phase_synchronized_dof(input)
        || limiting_dof >= input->dofs) {
        return false;
    }

    /* Phase sync scales non-limiting DoFs from the limiting profile; fall back to time sync on any mismatch. */
    limiting_profile = &trajectory->profiles[limiting_dof];
    pd_limiting = limiting_profile->pf - limiting_profile->p[0];
    if (fabs(pd_limiting) < RUCKIG_DBL_EPSILON
        || limiting_profile->brake.duration > 0.0
        || limiting_profile->accel.duration > 0.0
        || isinf(input->max_acceleration[limiting_dof])
        || !isinf(input->max_jerk[limiting_dof])) {
        return false;
    }

    control_limiting = limiting_profile->a[0];
    if (fabs(control_limiting) < RUCKIG_DBL_EPSILON) {
        return false;
    }

    for (dof = 0; dof < input->dofs; ++dof) {
        ruckig_profile_t* profile = &trajectory->profiles[dof];
        const double pd = profile->pf - profile->p[0];
        const double phase_control = (control_limiting * pd) / pd_limiting;
        const double v_min = input->has_min_velocity ? input->min_velocity[dof] : -input->max_velocity[dof];
        const double a_min = input->has_min_acceleration ? input->min_acceleration[dof] : -input->max_acceleration[dof];
        size_t i;

        if (!input->enabled[dof]
            || dof == limiting_dof
            || effective_synchronization(input, dof) != RUCKIG_SYNCHRONIZATION_PHASE) {
            continue;
        }
        if (isinf(input->max_acceleration[dof])
            || !isinf(input->max_jerk[dof])
            || profile->brake.duration > 0.0
            || profile->accel.duration > 0.0
            || fabs(input->current_velocity[dof]) > RUCKIG_DBL_EPSILON
            || fabs(input->current_acceleration[dof]) > RUCKIG_DBL_EPSILON
            || fabs(input->target_velocity[dof]) > RUCKIG_DBL_EPSILON
            || fabs(input->target_acceleration[dof]) > RUCKIG_DBL_EPSILON) {
            return false;
        }

        for (i = 0; i < 7; ++i) {
            profile->t[i] = limiting_profile->t[i];
        }
        profile->control_signs = limiting_profile->control_signs;

        if (!ruckig_profile_check_for_second_order_with_timing_guarded(
                profile,
                profile->control_signs,
                RUCKIG_PROFILE_LIMITS_NONE,
                sync_duration,
                phase_control,
                -phase_control,
                input->max_velocity[dof],
                v_min,
                input->max_acceleration[dof],
                a_min
            )) {
            return false;
        }
        profile->limits = limiting_profile->limits;
    }

    return true;
}

static bool calculate_position_phase_sync(
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    double sync_duration,
    size_t limiting_dof
) {
    const double eps = RUCKIG_DBL_EPSILON;
    ruckig_profile_t* limiting_profile;
    double pd_limiting;
    double control_limiting;
    size_t dof;

    if (!has_phase_synchronized_dof(input)
        || limiting_dof >= input->dofs) {
        return false;
    }

    limiting_profile = &trajectory->profiles[limiting_dof];
    pd_limiting = limiting_profile->pf - limiting_profile->p[0];
    if (fabs(pd_limiting) < eps
        || limiting_profile->brake.duration > 0.0
        || limiting_profile->accel.duration > 0.0) {
        return false;
    }

    if (!isinf(input->max_jerk[limiting_dof])) {
        control_limiting = limiting_profile->direction == RUCKIG_PROFILE_DIRECTION_UP
            ? input->max_jerk[limiting_dof]
            : -input->max_jerk[limiting_dof];
    } else {
        control_limiting = limiting_profile->direction == RUCKIG_PROFILE_DIRECTION_UP
            ? input->max_acceleration[limiting_dof]
            : (input->has_min_acceleration ? input->min_acceleration[limiting_dof] : -input->max_acceleration[limiting_dof]);
    }
    if (fabs(control_limiting) < eps) {
        return false;
    }

    for (dof = 0; dof < input->dofs; ++dof) {
        const ruckig_profile_t* profile = &trajectory->profiles[dof];
        const double pd = profile->pf - profile->p[0];
        const double scale = pd / pd_limiting;

        if (!input->enabled[dof]
            || dof == limiting_dof
            || effective_synchronization(input, dof) != RUCKIG_SYNCHRONIZATION_PHASE) {
            continue;
        }
        if (profile->brake.duration > 0.0
            || profile->accel.duration > 0.0
            || fabs(input->current_velocity[dof] - input->current_velocity[limiting_dof] * scale) > eps
            || fabs(input->current_acceleration[dof] - input->current_acceleration[limiting_dof] * scale) > eps
            || fabs(input->target_velocity[dof] - input->target_velocity[limiting_dof] * scale) > eps
            || fabs(input->target_acceleration[dof] - input->target_acceleration[limiting_dof] * scale) > eps) {
            return false;
        }
    }

    for (dof = 0; dof < input->dofs; ++dof) {
        ruckig_profile_t* profile = &trajectory->profiles[dof];
        const double pd = profile->pf - profile->p[0];
        const double phase_control = (control_limiting * pd) / pd_limiting;
        const double v_min = input->has_min_velocity ? input->min_velocity[dof] : -input->max_velocity[dof];
        const double a_min = input->has_min_acceleration ? input->min_acceleration[dof] : -input->max_acceleration[dof];
        size_t i;

        if (!input->enabled[dof]
            || dof == limiting_dof
            || effective_synchronization(input, dof) != RUCKIG_SYNCHRONIZATION_PHASE) {
            continue;
        }

        for (i = 0; i < 7; ++i) {
            profile->t[i] = limiting_profile->t[i];
        }
        profile->control_signs = limiting_profile->control_signs;

        if (!isinf(input->max_jerk[dof])) {
            if (!ruckig_profile_check_with_timing_guarded(
                    profile,
                    profile->control_signs,
                    RUCKIG_PROFILE_LIMITS_NONE,
                    sync_duration,
                    phase_control,
                    input->max_velocity[dof],
                    v_min,
                    input->max_acceleration[dof],
                    a_min,
                    input->max_jerk[dof]
                )) {
                return false;
            }
        } else if (!isinf(input->max_acceleration[dof])) {
            if (!ruckig_profile_check_for_second_order_with_timing_guarded(
                    profile,
                    profile->control_signs,
                    RUCKIG_PROFILE_LIMITS_NONE,
                    sync_duration,
                    phase_control,
                    -phase_control,
                    input->max_velocity[dof],
                    v_min,
                    input->max_acceleration[dof],
                    a_min
                )) {
                return false;
            }
        } else {
            if (!ruckig_profile_check_for_first_order_with_timing_guarded(
                    profile,
                    profile->control_signs,
                    RUCKIG_PROFILE_LIMITS_NONE,
                    sync_duration,
                    phase_control,
                    input->max_velocity[dof],
                    v_min
                )) {
                return false;
            }
        }
        profile->limits = limiting_profile->limits;
    }

    return true;
}

static bool calculate_velocity_phase_sync(
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    double sync_duration,
    size_t limiting_dof
) {
    const double eps = RUCKIG_DBL_EPSILON;
    ruckig_profile_t* limiting_profile;
    double scale_limiting;
    double control_limiting;
    size_t dof;
    int scale_kind = 0;

    if (!has_phase_synchronized_dof(input)
        || limiting_dof >= input->dofs) {
        return false;
    }

    /* Velocity phase sync keeps the oracle's scale-source priority across target/current velocity and acceleration. */
    limiting_profile = &trajectory->profiles[limiting_dof];
    if (limiting_profile->brake.duration > 0.0 || limiting_profile->accel.duration > 0.0) {
        return false;
    }

    scale_limiting = input->target_velocity[limiting_dof];
    if (fabs(scale_limiting) < eps) {
        scale_limiting = input->current_velocity[limiting_dof];
        scale_kind = 1;
    }
    if (fabs(scale_limiting) < eps) {
        scale_limiting = input->current_acceleration[limiting_dof];
        scale_kind = 2;
    }
    if (fabs(scale_limiting) < eps) {
        scale_limiting = input->target_acceleration[limiting_dof];
        scale_kind = 3;
    }
    if (fabs(scale_limiting) < eps) {
        return false;
    }

    if (!isinf(input->max_jerk[limiting_dof])) {
        control_limiting = limiting_profile->direction == RUCKIG_PROFILE_DIRECTION_UP
            ? input->max_jerk[limiting_dof]
            : -input->max_jerk[limiting_dof];
    } else {
        control_limiting = limiting_profile->direction == RUCKIG_PROFILE_DIRECTION_UP
            ? input->max_acceleration[limiting_dof]
            : (input->has_min_acceleration ? input->min_acceleration[limiting_dof] : -input->max_acceleration[limiting_dof]);
    }
    if (fabs(control_limiting) < eps) {
        return false;
    }

    for (dof = 0; dof < input->dofs; ++dof) {
        const ruckig_profile_t* profile = &trajectory->profiles[dof];
        double scale;

        if (!input->enabled[dof]
            || dof == limiting_dof
            || effective_synchronization(input, dof) != RUCKIG_SYNCHRONIZATION_PHASE) {
            continue;
        }
        if (scale_kind == 0) {
            scale = input->target_velocity[dof] / scale_limiting;
        } else if (scale_kind == 1) {
            scale = input->current_velocity[dof] / scale_limiting;
        } else if (scale_kind == 2) {
            scale = input->current_acceleration[dof] / scale_limiting;
        } else {
            scale = input->target_acceleration[dof] / scale_limiting;
        }

        if (profile->brake.duration > 0.0
            || profile->accel.duration > 0.0
            || fabs(input->current_velocity[dof] - input->current_velocity[limiting_dof] * scale) > eps
            || fabs(input->current_acceleration[dof] - input->current_acceleration[limiting_dof] * scale) > eps
            || fabs(input->target_velocity[dof] - input->target_velocity[limiting_dof] * scale) > eps
            || fabs(input->target_acceleration[dof] - input->target_acceleration[limiting_dof] * scale) > eps) {
            return false;
        }
    }

    for (dof = 0; dof < input->dofs; ++dof) {
        ruckig_profile_t* profile = &trajectory->profiles[dof];
        double current_scale;
        double phase_control;
        const double a_min = input->has_min_acceleration ? input->min_acceleration[dof] : -input->max_acceleration[dof];
        size_t i;

        if (!input->enabled[dof]
            || dof == limiting_dof
            || effective_synchronization(input, dof) != RUCKIG_SYNCHRONIZATION_PHASE) {
            continue;
        }

        if (scale_kind == 0) {
            current_scale = input->target_velocity[dof];
        } else if (scale_kind == 1) {
            current_scale = input->current_velocity[dof];
        } else if (scale_kind == 2) {
            current_scale = input->current_acceleration[dof];
        } else {
            current_scale = input->target_acceleration[dof];
        }

        phase_control = (control_limiting * current_scale) / scale_limiting;
        for (i = 0; i < 7; ++i) {
            profile->t[i] = limiting_profile->t[i];
        }
        profile->control_signs = limiting_profile->control_signs;

        if (!isinf(input->max_jerk[dof])) {
            if (!ruckig_profile_check_for_velocity_with_timing_guarded(
                    profile,
                    profile->control_signs,
                    RUCKIG_PROFILE_LIMITS_NONE,
                    sync_duration,
                    phase_control,
                    input->max_acceleration[dof],
                    a_min,
                    input->max_jerk[dof]
                )) {
                return false;
            }
        } else {
            if (!ruckig_profile_check_for_second_order_velocity_with_timing_guarded(
                    profile,
                    profile->control_signs,
                    RUCKIG_PROFILE_LIMITS_NONE,
                    sync_duration,
                    phase_control,
                    input->max_acceleration[dof],
                    a_min
                )) {
                return false;
            }
        }
        profile->limits = limiting_profile->limits;
    }

    return true;
}

static ruckig_result_t calculate_no_jerk_position(
    const ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory
) {
    static const ruckig_calculate_ops_t ops = {
        prepare_position_profile,
        calculate_no_jerk_position_step1,
        calculate_no_jerk_position_step2,
        calculate_no_jerk_position_phase_sync,
        true
    };
    return calculate_with_ops(otg, input, trajectory, &ops);
}

static ruckig_result_t calculate_no_jerk_velocity(
    const ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory
) {
    static const ruckig_calculate_ops_t ops = {
        prepare_velocity_profile,
        calculate_no_jerk_velocity_step1,
        calculate_no_jerk_velocity_step2,
        calculate_velocity_phase_sync,
        false
    };
    return calculate_with_ops(otg, input, trajectory, &ops);
}

static ruckig_result_t calculate_velocity(
    const ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory
) {
    static const ruckig_calculate_ops_t ops = {
        prepare_velocity_profile,
        calculate_velocity_step1,
        calculate_velocity_step2,
        calculate_velocity_phase_sync,
        true
    };
    return calculate_with_ops(otg, input, trajectory, &ops);
}

static ruckig_result_t calculate_position(
    const ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory
) {
    static const ruckig_calculate_ops_t ops = {
        prepare_mixed_profile,
        calculate_position_step1,
        calculate_position_step2,
        calculate_position_phase_sync,
        true
    };
    return calculate_with_ops(otg, input, trajectory, &ops);
}

static ruckig_result_t ruckig_create_impl(
    ruckig_t** otg,
    size_t dofs,
    double delta_time,
    size_t max_number_of_waypoints
) {
    ruckig_t* value;
    size_t waypoint_values = 0;
    if (!otg || dofs == 0) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }

    *otg = NULL;
    if (!ruckig_checked_waypoint_counts(dofs, max_number_of_waypoints, NULL, NULL, &waypoint_values)) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    value = (ruckig_t*)ruckig_calloc(1, sizeof(*value));
    if (!value) {
        return RUCKIG_ERROR;
    }

    value->dofs = dofs;
    value->max_number_of_waypoints = max_number_of_waypoints;
    value->delta_time = delta_time;
    if (ruckig_input_create_with_waypoints(&value->current_input, dofs, max_number_of_waypoints) != RUCKIG_WORKING) {
        ruckig_destroy(value);
        return RUCKIG_ERROR;
    }
    if (ruckig_trajectory_create(&value->no_waypoint_scratch_trajectory, dofs) != RUCKIG_WORKING) {
        ruckig_destroy(value);
        return RUCKIG_ERROR;
    }
    if (ruckig_input_create(&value->waypoint_section_input, dofs) != RUCKIG_WORKING) {
        ruckig_destroy(value);
        return RUCKIG_ERROR;
    }
    if (ruckig_trajectory_create(&value->waypoint_section_trajectory, dofs) != RUCKIG_WORKING) {
        ruckig_destroy(value);
        return RUCKIG_ERROR;
    }
    if (waypoint_values > 0) {
        if (ruckig_input_create_with_waypoints(&value->waypoint_engine.identity_input, dofs, max_number_of_waypoints) != RUCKIG_WORKING) {
            ruckig_destroy(value);
            return RUCKIG_ERROR;
        }
        if (ruckig_trajectory_create_with_waypoints(&value->waypoint_engine.scratch_trajectory, dofs, max_number_of_waypoints) != RUCKIG_WORKING) {
            ruckig_destroy(value);
            return RUCKIG_ERROR;
        }
        value->waypoint_engine.branch_queue = (ruckig_waypoint_branch_t*)ruckig_calloc(
            RUCKIG_WAYPOINT_BRANCH_QUEUE_CAPACITY,
            sizeof(*value->waypoint_engine.branch_queue)
        );
        value->waypoint_engine.candidate_velocity = (double*)ruckig_calloc(waypoint_values, sizeof(double));
        value->waypoint_engine.candidate_acceleration = (double*)ruckig_calloc(waypoint_values, sizeof(double));
        value->waypoint_engine.best_velocity = (double*)ruckig_calloc(waypoint_values, sizeof(double));
        value->waypoint_engine.best_acceleration = (double*)ruckig_calloc(waypoint_values, sizeof(double));
        value->waypoint_engine.baseline_velocity = (double*)ruckig_calloc(waypoint_values, sizeof(double));
        value->waypoint_engine.baseline_acceleration = (double*)ruckig_calloc(waypoint_values, sizeof(double));
        if (!value->waypoint_engine.branch_queue
            || !value->waypoint_engine.candidate_velocity || !value->waypoint_engine.candidate_acceleration
            || !value->waypoint_engine.best_velocity || !value->waypoint_engine.best_acceleration
            || !value->waypoint_engine.baseline_velocity || !value->waypoint_engine.baseline_acceleration) {
            ruckig_destroy(value);
            return RUCKIG_ERROR;
        }
    }

    *otg = value;
    return RUCKIG_WORKING;
}

RUCKIG_C_API ruckig_result_t ruckig_create(ruckig_t** otg, size_t dofs, double delta_time) {
    return ruckig_create_impl(otg, dofs, delta_time, 0);
}

RUCKIG_C_API ruckig_result_t ruckig_create_with_waypoints(
    ruckig_t** otg,
    size_t dofs,
    double delta_time,
    size_t max_number_of_waypoints
) {
    return ruckig_create_impl(otg, dofs, delta_time, max_number_of_waypoints);
}

RUCKIG_C_API void ruckig_destroy(ruckig_t* otg) {
    if (!otg) {
        return;
    }
    ruckig_input_destroy(otg->current_input);
    ruckig_trajectory_destroy(otg->no_waypoint_scratch_trajectory);
    ruckig_input_destroy(otg->waypoint_section_input);
    ruckig_input_destroy(otg->waypoint_engine.identity_input);
    ruckig_trajectory_destroy(otg->waypoint_section_trajectory);
    ruckig_trajectory_destroy(otg->waypoint_engine.scratch_trajectory);
    ruckig_free(otg->waypoint_engine.branch_queue);
    ruckig_free(otg->waypoint_engine.candidate_velocity);
    ruckig_free(otg->waypoint_engine.candidate_acceleration);
    ruckig_free(otg->waypoint_engine.best_velocity);
    ruckig_free(otg->waypoint_engine.best_acceleration);
    ruckig_free(otg->waypoint_engine.baseline_velocity);
    ruckig_free(otg->waypoint_engine.baseline_acceleration);
    ruckig_free(otg);
}

RUCKIG_C_API size_t ruckig_get_max_number_of_waypoints(const ruckig_t* otg) {
    return otg ? otg->max_number_of_waypoints : 0;
}

ruckig_result_t ruckig_calculate_target(
    const ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory
) {
    if (!otg || !input || !trajectory || otg->dofs != input->dofs || trajectory->dofs != otg->dofs) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    trajectory->valid = false;
    trajectory->section_count = 1;
    if (ruckig_validate_input(otg, input, false, true) != RUCKIG_WORKING) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (input->has_per_dof_control_interface) {
        return calculate_position(otg, input, trajectory);
    }
    if (input->control_interface == RUCKIG_CONTROL_POSITION) {
        if (input_is_first_order_position(input)) {
            return calculate_first_order_position(otg, input, trajectory);
        }
        if (input_is_no_jerk_position(input)) {
            return calculate_no_jerk_position(otg, input, trajectory);
        }
        return calculate_position(otg, input, trajectory);
    }
    if (input->control_interface == RUCKIG_CONTROL_VELOCITY) {
        if (input_is_no_jerk_velocity(input)) {
            return calculate_no_jerk_velocity(otg, input, trajectory);
        }
        return calculate_velocity(otg, input, trajectory);
    }
    return RUCKIG_ERROR_INVALID_INPUT;
}

static ruckig_result_t ruckig_calculate_target_interruptible(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    bool* was_interrupted,
    bool* published
) {
    ruckig_result_t result;
    ruckig_interrupt_context_t interrupt_context;
    const bool has_incumbent = trajectory && trajectory->valid
        && otg && otg->current_input_initialized && otg->current_input->waypoint_count == 0;
    if (!otg || !input || !trajectory || !otg->no_waypoint_scratch_trajectory || !was_interrupted || !published) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }

    *was_interrupted = false;
    *published = false;
    interrupt_context = ruckig_interrupt_context_start(input, true);
    result = ruckig_calculate_target(otg, input, otg->no_waypoint_scratch_trajectory);
    if (result != RUCKIG_WORKING) {
        return result;
    }
    if (has_incumbent && ruckig_interrupt_context_elapsed(&interrupt_context)) {
        *was_interrupted = true;
        return RUCKIG_WORKING;
    }

    result = ruckig_trajectory_copy_internal(trajectory, otg->no_waypoint_scratch_trajectory);
    if (result != RUCKIG_WORKING) {
        return result;
    }
    *published = true;
    return RUCKIG_WORKING;
}

static ruckig_result_t ruckig_calculate_impl(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    ruckig_diagnostics_t* diagnostics
) {
    ruckig_result_t result;
    if (ruckig_diagnostics_validate_or_null(diagnostics) != RUCKIG_WORKING) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (!otg || !input || !trajectory) {
        ruckig_diagnostics_record(
            diagnostics,
            RUCKIG_ERROR_INVALID_INPUT,
            RUCKIG_DIAGNOSTIC_SCOPE_CALCULATION,
            RUCKIG_DIAGNOSTIC_NULL_ARGUMENT,
            0u,
            0u,
            0u,
            0u,
            0.0,
            0.0
        );
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (otg->dofs != input->dofs || trajectory->dofs != otg->dofs) {
        ruckig_diagnostics_record(
            diagnostics,
            RUCKIG_ERROR_INVALID_INPUT,
            RUCKIG_DIAGNOSTIC_SCOPE_CALCULATION,
            RUCKIG_DIAGNOSTIC_DOF_MISMATCH,
            0u,
            0u,
            otg->dofs,
            otg->dofs != input->dofs ? input->dofs : trajectory->dofs,
            0.0,
            0.0
        );
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    ruckig_waypoint_resume_clear(otg);
    if (input->waypoint_count > 0) {
        if (input->waypoint_count > otg->max_number_of_waypoints
            || input->waypoint_count > trajectory->max_number_of_waypoints) {
            const size_t capacity = otg->max_number_of_waypoints < trajectory->max_number_of_waypoints
                ? otg->max_number_of_waypoints
                : trajectory->max_number_of_waypoints;
            ruckig_diagnostics_record(
                diagnostics,
                RUCKIG_ERROR_INVALID_INPUT,
                RUCKIG_DIAGNOSTIC_SCOPE_CALCULATION,
                RUCKIG_DIAGNOSTIC_CAPACITY_MISMATCH,
                0u,
                0u,
                capacity,
                input->waypoint_count,
                0.0,
                0.0
            );
            return RUCKIG_ERROR_INVALID_INPUT;
        }
        result = ruckig_calculate_waypoints(otg, input, trajectory);
        if (result != RUCKIG_WORKING
            && result == RUCKIG_ERROR_INVALID_INPUT
            && ruckig_validate_input_with_diagnostics(otg, input, false, true, diagnostics) != RUCKIG_WORKING) {
            return result;
        }
        record_operation_result_diagnostics(diagnostics, result, RUCKIG_DIAGNOSTIC_SCOPE_CALCULATION);
        return result;
    }
    result = ruckig_calculate_target(otg, input, trajectory);
    if (result != RUCKIG_WORKING
        && result == RUCKIG_ERROR_INVALID_INPUT
        && ruckig_validate_input_with_diagnostics(otg, input, false, true, diagnostics) != RUCKIG_WORKING) {
        return result;
    }
    record_operation_result_diagnostics(diagnostics, result, RUCKIG_DIAGNOSTIC_SCOPE_CALCULATION);
    return result;
}

RUCKIG_C_API ruckig_result_t ruckig_calculate_with_diagnostics(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    ruckig_diagnostics_t* diagnostics
) {
    return ruckig_calculate_impl(otg, input, trajectory, diagnostics);
}

RUCKIG_C_API ruckig_result_t ruckig_calculate(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory
) {
    return ruckig_calculate_impl(otg, input, trajectory, NULL);
}

static void publish_new_trajectory_to_output(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_output_t* output
) {
    ruckig_input_copy_state(input, otg->current_input);
    otg->current_input_initialized = true;
    output->time = 0.0;
    output->new_section = 0;
    output->did_section_change = false;
    output->new_calculation = true;
}

static ruckig_result_t calculate_or_resume_output_trajectory(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_output_t* output,
    bool input_matches_current
) {
    if (!output->trajectory->valid
        || !otg->current_input_initialized
        || !input_matches_current) {
        bool published = true;
        ruckig_result_t result;
        if (input->waypoint_count > 0 && input->has_interrupt_calculation_duration) {
            result = ruckig_calculate_waypoints_interruptible(
                otg,
                input,
                output->trajectory,
                &output->was_calculation_interrupted
            );
        } else if (input->waypoint_count == 0 && input->has_interrupt_calculation_duration) {
            result = ruckig_calculate_target_interruptible(
                otg,
                input,
                output->trajectory,
                &output->was_calculation_interrupted,
                &published
            );
        } else {
            result = ruckig_calculate(otg, input, output->trajectory);
        }
        if (result != RUCKIG_WORKING) {
            return result;
        }
        if (!(input->waypoint_count > 0 && input->has_interrupt_calculation_duration)) {
            ruckig_waypoint_resume_clear(otg);
        }
        if (published) {
            publish_new_trajectory_to_output(otg, input, output);
        }
    } else if (input->waypoint_count > 0 && input->has_interrupt_calculation_duration) {
        bool published = false;
        const double incumbent_remaining_duration = output->trajectory->duration > output->time
            ? output->trajectory->duration - output->time
            : 0.0;
        const ruckig_result_t result = ruckig_waypoint_resume_continue(
            otg,
            input,
            output->trajectory,
            incumbent_remaining_duration,
            &output->was_calculation_interrupted,
            &published
        );
        if (result != RUCKIG_WORKING) {
            return result;
        }
        if (published) {
            publish_new_trajectory_to_output(otg, input, output);
        }
    }
    return RUCKIG_WORKING;
}

static ruckig_result_t sample_output_at_next_time(ruckig_t* otg, ruckig_output_t* output) {
    const size_t old_section = output->new_section;
    ruckig_result_t sample_result;

    output->time += otg->delta_time;
    sample_result = ruckig_trajectory_at_time(
        output->trajectory,
        output->time,
        output->new_position,
        output->new_velocity,
        output->new_acceleration,
        output->new_jerk,
        &output->new_section
    );
    if (sample_result != RUCKIG_WORKING) {
        return sample_result;
    }
    output->did_section_change = output->new_section > old_section;
    ruckig_output_pass_to_input(output, otg->current_input);
    return RUCKIG_WORKING;
}

static ruckig_result_t ruckig_update_impl(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_output_t* output,
    ruckig_diagnostics_t* diagnostics
) {
    const double calculation_start = calculation_duration_start();
    bool waypoint_resume_mismatch;
    size_t waypoint_capacity_expected = 0u;
    size_t waypoint_capacity_actual = 0u;
    if (ruckig_diagnostics_validate_or_null(diagnostics) != RUCKIG_WORKING) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (!otg || !input || !output) {
        ruckig_diagnostics_record(
            diagnostics,
            RUCKIG_ERROR_INVALID_INPUT,
            RUCKIG_DIAGNOSTIC_SCOPE_UPDATE,
            RUCKIG_DIAGNOSTIC_NULL_ARGUMENT,
            0u,
            0u,
            0u,
            0u,
            0.0,
            0.0
        );
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (otg->dofs != input->dofs || output->dofs != otg->dofs) {
        ruckig_diagnostics_record(
            diagnostics,
            RUCKIG_ERROR_INVALID_INPUT,
            RUCKIG_DIAGNOSTIC_SCOPE_UPDATE,
            RUCKIG_DIAGNOSTIC_DOF_MISMATCH,
            0u,
            0u,
            otg->dofs,
            otg->dofs != input->dofs ? input->dofs : output->dofs,
            0.0,
            0.0
        );
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    waypoint_resume_mismatch = otg->waypoint_engine.active
        && !otg->waypoint_engine.complete
        && !ruckig_waypoint_resume_can_continue(otg, input);
    (void)waypoint_update_capacity_mismatch(
        otg,
        input,
        output,
        &waypoint_capacity_expected,
        &waypoint_capacity_actual
    );
    output->new_calculation = false;
    output->was_calculation_interrupted = false;
    if (input->waypoint_count == 0 || !input->has_interrupt_calculation_duration) {
        ruckig_waypoint_resume_clear(otg);
    }
    const bool input_matches_current = otg->current_input_initialized
        && ((input->waypoint_count > 0 || otg->current_input->waypoint_count > 0)
            ? ruckig_input_equals_ignoring_interrupt(input, otg->current_input)
            : ruckig_input_equals(input, otg->current_input));
    {
        ruckig_result_t result = calculate_or_resume_output_trajectory(otg, input, output, input_matches_current);
        if (result != RUCKIG_WORKING) {
            output->calculation_duration = calculation_duration_finish(calculation_start);
            if (diagnostics && waypoint_capacity_actual > waypoint_capacity_expected) {
                ruckig_diagnostics_record(
                    diagnostics,
                    result,
                    RUCKIG_DIAGNOSTIC_SCOPE_WAYPOINT,
                    RUCKIG_DIAGNOSTIC_CAPACITY_MISMATCH,
                    0u,
                    0u,
                    waypoint_capacity_expected,
                    waypoint_capacity_actual,
                    0.0,
                    0.0
                );
                return result;
            }
            if (diagnostics && waypoint_resume_mismatch) {
                record_waypoint_resume_mismatch_diagnostics(diagnostics, otg, input, result);
                return result;
            }
            if (diagnostics && result == RUCKIG_ERROR_INVALID_INPUT
                && ruckig_validate_input_with_diagnostics(otg, input, false, true, diagnostics) != RUCKIG_WORKING) {
                return result;
            }
            record_operation_result_diagnostics(diagnostics, result, RUCKIG_DIAGNOSTIC_SCOPE_UPDATE);
            return result;
        }
    }
    {
        ruckig_result_t result = sample_output_at_next_time(otg, output);
        if (result != RUCKIG_WORKING) {
            output->calculation_duration = calculation_duration_finish(calculation_start);
            record_operation_result_diagnostics(diagnostics, result, RUCKIG_DIAGNOSTIC_SCOPE_UPDATE);
            return result;
        }
    }
    output->calculation_duration = calculation_duration_finish(calculation_start);

    {
        const ruckig_result_t result = output->time > output->trajectory->duration ? RUCKIG_FINISHED : RUCKIG_WORKING;
        if (waypoint_resume_mismatch) {
            record_waypoint_resume_mismatch_diagnostics(diagnostics, otg, input, result);
        } else if (output->was_calculation_interrupted) {
            ruckig_diagnostics_record(
                diagnostics,
                result,
                input->waypoint_count > 0 ? RUCKIG_DIAGNOSTIC_SCOPE_WAYPOINT : RUCKIG_DIAGNOSTIC_SCOPE_UPDATE,
                RUCKIG_DIAGNOSTIC_INTERRUPTED,
                0u,
                output->new_section,
                0u,
                0u,
                output->time,
                output->trajectory->duration
            );
        } else {
            ruckig_diagnostics_clear(diagnostics, result, RUCKIG_DIAGNOSTIC_SCOPE_NONE);
        }
        return result;
    }
}

RUCKIG_C_API ruckig_result_t ruckig_update_with_diagnostics(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_output_t* output,
    ruckig_diagnostics_t* diagnostics
) {
    return ruckig_update_impl(otg, input, output, diagnostics);
}

RUCKIG_C_API ruckig_result_t ruckig_update(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_output_t* output
) {
    return ruckig_update_impl(otg, input, output, NULL);
}

RUCKIG_C_API void ruckig_reset(ruckig_t* otg) {
    if (otg) {
        otg->current_input_initialized = false;
        ruckig_waypoint_resume_clear(otg);
    }
}
