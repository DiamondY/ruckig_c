#include "ruckig_c/internal.h"

#include "ruckig_c/position_first.h"
#include "ruckig_c/velocity_second.h"

#include <math.h>
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

static bool input_is_first_order_position(const ruckig_input_t* input) {
    size_t i;
    if (!input || input->control_interface != RUCKIG_CONTROL_POSITION) {
        return false;
    }
    for (i = 0; i < input->dofs; ++i) {
        if (!isinf(input->max_acceleration[i]) || !isinf(input->max_jerk[i])) {
            return false;
        }
    }
    return true;
}

static bool input_is_no_jerk_position(const ruckig_input_t* input) {
    size_t i;
    if (!input || input->control_interface != RUCKIG_CONTROL_POSITION) {
        return false;
    }
    for (i = 0; i < input->dofs; ++i) {
        if (!isinf(input->max_jerk[i])) {
            return false;
        }
    }
    return true;
}

static bool input_is_no_jerk_velocity(const ruckig_input_t* input) {
    size_t i;
    if (!input || input->control_interface != RUCKIG_CONTROL_VELOCITY) {
        return false;
    }
    for (i = 0; i < input->dofs; ++i) {
        if (!isinf(input->max_jerk[i])) {
            return false;
        }
    }
    return true;
}

static double round_up_to_discrete_duration(double duration, double delta_time) {
    const double eps = 2.2204460492503131e-16;
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
    const double eps = 2.2204460492503131e-16;
    if (input->synchronization == RUCKIG_SYNCHRONIZATION_NONE
        && input->duration_discretization == RUCKIG_DURATION_CONTINUOUS) {
        return true;
    }
    if (input->synchronization == RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY
        && fabs(input->target_velocity[dof]) < eps
        && fabs(input->target_acceleration[dof]) < eps) {
        return true;
    }
    return false;
}

static double finalize_trajectory_duration(
    const ruckig_t* otg,
    const ruckig_input_t* input,
    double max_independent_duration
) {
    double duration = max_independent_duration;

    if (input->synchronization == RUCKIG_SYNCHRONIZATION_NONE) {
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
    const double eps = 2.2204460492503131e-16;
    size_t dof;

    if (!input || !trajectory || !limiting_dof || input->duration_discretization != RUCKIG_DURATION_DISCRETE) {
        return;
    }

    for (dof = 0; dof < input->dofs; ++dof) {
        double candidate;
        if (!input->enabled[dof] || input->synchronization == RUCKIG_SYNCHRONIZATION_NONE) {
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
    const double eps = 2.2204460492503131e-16;

    if (input->synchronization == RUCKIG_SYNCHRONIZATION_NONE
        && input->duration_discretization == RUCKIG_DURATION_CONTINUOUS) {
        return duration;
    }

    while (changed) {
        size_t dof;
        changed = false;
        for (dof = 0; dof < input->dofs; ++dof) {
            const ruckig_block_t* block = &trajectory->blocks[dof];
            double adjusted = duration;

            if (!input->enabled[dof] || !block->valid) {
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
    const double eps = 2.2204460492503131e-16;
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

static ruckig_result_t calculate_first_order_position(
    const ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory
) {
    size_t dof;
    size_t limiting_dof = 0;
    double sync_duration = 0.0;

    for (dof = 0; dof < input->dofs; ++dof) {
        ruckig_profile_t* profile = &trajectory->profiles[dof];
        double duration = 0.0;
        const double v_min = input->has_min_velocity ? input->min_velocity[dof] : -input->max_velocity[dof];

        ruckig_profile_init(profile);
        ruckig_profile_set_boundary(
            profile,
            input->current_position[dof],
            input->current_velocity[dof],
            input->current_acceleration[dof],
            input->target_position[dof],
            input->target_velocity[dof],
            input->target_acceleration[dof]
        );

        if (!input->enabled[dof]) {
            set_disabled_profile_state(profile, input, dof);
            trajectory->independent_min_durations[dof] = 0.0;
            continue;
        }

        if (!ruckig_position_first_step1_get_profile(
                profile,
                profile,
                &trajectory->blocks[dof],
                &duration,
                profile->p[0],
                profile->pf,
                input->max_velocity[dof],
                v_min
            )) {
            return input->max_velocity[dof] == 0.0 || v_min == 0.0 ? RUCKIG_ERROR_ZERO_LIMITS : RUCKIG_ERROR_EXECUTION_TIME_CALCULATION;
        }

        trajectory->independent_min_durations[dof] = duration;
        if (input->synchronization != RUCKIG_SYNCHRONIZATION_NONE && duration > sync_duration) {
            sync_duration = duration;
            limiting_dof = dof;
        }
        if (input->synchronization == RUCKIG_SYNCHRONIZATION_NONE && duration > sync_duration) {
            sync_duration = duration;
            limiting_dof = dof;
        }
    }

    sync_duration = finalize_trajectory_duration(otg, input, sync_duration);
    select_limiting_dof_for_duration(otg, input, trajectory, sync_duration, &limiting_dof);

    if (calculate_position_phase_sync(input, trajectory, sync_duration, limiting_dof)) {
        trajectory->duration = sync_duration;
        trajectory->cumulative_times[0] = sync_duration;
        trajectory->valid = true;
        return RUCKIG_WORKING;
    }

    for (dof = 0; dof < input->dofs; ++dof) {
        ruckig_profile_t* profile = &trajectory->profiles[dof];
        const double v_min = input->has_min_velocity ? input->min_velocity[dof] : -input->max_velocity[dof];
        const bool skip_time_sync = should_skip_time_synchronization(input, dof);
        const double own_duration = trajectory->independent_min_durations[dof];

        if (!input->enabled[dof] || skip_time_sync || fabs(sync_duration - own_duration) < 2.0 * 2.2204460492503131e-16) {
            continue;
        }

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
    }

    trajectory->duration = sync_duration;
    trajectory->cumulative_times[0] = sync_duration;
    trajectory->valid = true;
    return RUCKIG_WORKING;
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

    if (input->synchronization != RUCKIG_SYNCHRONIZATION_PHASE
        || limiting_dof >= input->dofs) {
        return false;
    }

    limiting_profile = &trajectory->profiles[limiting_dof];
    pd_limiting = limiting_profile->pf - limiting_profile->p[0];
    if (fabs(pd_limiting) < 2.2204460492503131e-16
        || limiting_profile->brake.duration > 0.0
        || limiting_profile->accel.duration > 0.0
        || isinf(input->max_acceleration[limiting_dof])
        || !isinf(input->max_jerk[limiting_dof])) {
        return false;
    }

    control_limiting = limiting_profile->a[0];
    if (fabs(control_limiting) < 2.2204460492503131e-16) {
        return false;
    }

    for (dof = 0; dof < input->dofs; ++dof) {
        ruckig_profile_t* profile = &trajectory->profiles[dof];
        const double pd = profile->pf - profile->p[0];
        const double phase_control = (control_limiting * pd) / pd_limiting;
        const double v_min = input->has_min_velocity ? input->min_velocity[dof] : -input->max_velocity[dof];
        const double a_min = input->has_min_acceleration ? input->min_acceleration[dof] : -input->max_acceleration[dof];
        size_t i;

        if (!input->enabled[dof] || dof == limiting_dof) {
            continue;
        }
        if (isinf(input->max_acceleration[dof])
            || !isinf(input->max_jerk[dof])
            || profile->brake.duration > 0.0
            || profile->accel.duration > 0.0
            || fabs(input->current_velocity[dof]) > 2.2204460492503131e-16
            || fabs(input->current_acceleration[dof]) > 2.2204460492503131e-16
            || fabs(input->target_velocity[dof]) > 2.2204460492503131e-16
            || fabs(input->target_acceleration[dof]) > 2.2204460492503131e-16) {
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
    const double eps = 2.2204460492503131e-16;
    ruckig_profile_t* limiting_profile;
    double pd_limiting;
    double control_limiting;
    size_t dof;

    if (input->synchronization != RUCKIG_SYNCHRONIZATION_PHASE
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

        if (!input->enabled[dof] || dof == limiting_dof) {
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

        if (!input->enabled[dof] || dof == limiting_dof) {
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
    const double eps = 2.2204460492503131e-16;
    ruckig_profile_t* limiting_profile;
    double scale_limiting;
    double control_limiting;
    size_t dof;
    int scale_kind = 0;

    if (input->synchronization != RUCKIG_SYNCHRONIZATION_PHASE
        || limiting_dof >= input->dofs) {
        return false;
    }

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

        if (!input->enabled[dof] || dof == limiting_dof) {
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

        if (!input->enabled[dof] || dof == limiting_dof) {
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
    size_t dof;
    size_t limiting_dof = 0;
    double sync_duration = 0.0;

    for (dof = 0; dof < input->dofs; ++dof) {
        ruckig_profile_t* profile = &trajectory->profiles[dof];
        double duration = 0.0;
        const double v_min = input->has_min_velocity ? input->min_velocity[dof] : -input->max_velocity[dof];
        const bool first_order = isinf(input->max_acceleration[dof]);

        ruckig_profile_init(profile);
        ruckig_profile_set_boundary(
            profile,
            input->current_position[dof],
            input->current_velocity[dof],
            input->current_acceleration[dof],
            input->target_position[dof],
            input->target_velocity[dof],
            input->target_acceleration[dof]
        );

        if (!input->enabled[dof]) {
            set_disabled_profile_state(profile, input, dof);
            trajectory->independent_min_durations[dof] = 0.0;
            continue;
        }

        if (first_order) {
            if (!ruckig_position_first_step1_get_profile(profile, profile, &trajectory->blocks[dof], &duration, profile->p[0], profile->pf, input->max_velocity[dof], v_min)) {
                return input->max_velocity[dof] == 0.0 || v_min == 0.0 ? RUCKIG_ERROR_ZERO_LIMITS : RUCKIG_ERROR_EXECUTION_TIME_CALCULATION;
            }
        } else {
            const double a_min = input->has_min_acceleration ? input->min_acceleration[dof] : -input->max_acceleration[dof];
            ruckig_brake_get_second_order_position_trajectory(&profile->brake, input->current_velocity[dof], input->max_velocity[dof], v_min, input->max_acceleration[dof], a_min);
            ruckig_brake_finalize_second_order(&profile->brake, &profile->p[0], &profile->v[0], &profile->a[0]);
            if (!ruckig_position_second_step1_get_profile(profile, profile, &trajectory->blocks[dof], &duration, profile->p[0], profile->v[0], profile->pf, profile->vf, input->max_velocity[dof], v_min, input->max_acceleration[dof], a_min)) {
                return input->max_acceleration[dof] == 0.0 || a_min == 0.0 ? RUCKIG_ERROR_ZERO_LIMITS : RUCKIG_ERROR_EXECUTION_TIME_CALCULATION;
            }
        }

        trajectory->independent_min_durations[dof] = duration;
        if (duration > sync_duration) {
            sync_duration = duration;
            limiting_dof = dof;
        }
    }

    sync_duration = finalize_trajectory_duration(otg, input, sync_duration);
    select_limiting_dof_for_duration(otg, input, trajectory, sync_duration, &limiting_dof);
    sync_duration = adjust_duration_for_blocks(sync_duration, input, trajectory);

    if (calculate_no_jerk_position_phase_sync(input, trajectory, sync_duration, limiting_dof)) {
        trajectory->duration = sync_duration;
        trajectory->cumulative_times[0] = sync_duration;
        trajectory->valid = true;
        return RUCKIG_WORKING;
    }

    for (dof = 0; dof < input->dofs; ++dof) {
        ruckig_profile_t* profile = &trajectory->profiles[dof];
        const double v_min = input->has_min_velocity ? input->min_velocity[dof] : -input->max_velocity[dof];
        const bool first_order = isinf(input->max_acceleration[dof]);
        const bool skip_time_sync = should_skip_time_synchronization(input, dof);
        const double own_duration = trajectory->independent_min_durations[dof];
        const double t_profile = sync_duration - profile->brake.duration - profile->accel.duration;

        if (!input->enabled[dof] || skip_time_sync || fabs(sync_duration - own_duration) < 2.0 * 2.2204460492503131e-16) {
            continue;
        }

        if (first_order) {
            if (!ruckig_position_first_step2_get_profile(profile, t_profile, profile->p[0], profile->pf, input->max_velocity[dof], v_min)) {
                return RUCKIG_ERROR_SYNCHRONIZATION_CALCULATION;
            }
        } else {
            const double a_min = input->has_min_acceleration ? input->min_acceleration[dof] : -input->max_acceleration[dof];
            if (select_block_profile_for_duration(profile, &trajectory->blocks[dof], sync_duration)) {
                continue;
            }
            if (!ruckig_position_second_step2_get_profile(profile, t_profile, profile->p[0], profile->v[0], profile->pf, profile->vf, input->max_velocity[dof], v_min, input->max_acceleration[dof], a_min)) {
                return RUCKIG_ERROR_SYNCHRONIZATION_CALCULATION;
            }
        }
    }

    trajectory->duration = sync_duration;
    trajectory->cumulative_times[0] = sync_duration;
    trajectory->valid = true;
    return RUCKIG_WORKING;
}

static ruckig_result_t calculate_no_jerk_velocity(
    const ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory
) {
    size_t dof;
    size_t limiting_dof = 0;
    double sync_duration = 0.0;

    for (dof = 0; dof < input->dofs; ++dof) {
        ruckig_profile_t* profile = &trajectory->profiles[dof];
        double duration = 0.0;
        const double a_min = input->has_min_acceleration ? input->min_acceleration[dof] : -input->max_acceleration[dof];

        ruckig_profile_init(profile);
        ruckig_profile_set_boundary_for_velocity(
            profile,
            input->current_position[dof],
            input->current_velocity[dof],
            input->current_acceleration[dof],
            input->target_velocity[dof],
            input->target_acceleration[dof]
        );

        if (!input->enabled[dof]) {
            set_disabled_profile_state(profile, input, dof);
            trajectory->independent_min_durations[dof] = 0.0;
            continue;
        }

        ruckig_brake_get_second_order_velocity_trajectory(&profile->brake);
        if (!ruckig_velocity_second_step1_get_profile(profile, profile, &duration, profile->v[0], profile->vf, input->max_acceleration[dof], a_min)) {
            return input->max_acceleration[dof] == 0.0 || a_min == 0.0 ? RUCKIG_ERROR_ZERO_LIMITS : RUCKIG_ERROR_EXECUTION_TIME_CALCULATION;
        }

        trajectory->independent_min_durations[dof] = duration;
        if (duration > sync_duration) {
            sync_duration = duration;
            limiting_dof = dof;
        }
    }

    sync_duration = finalize_trajectory_duration(otg, input, sync_duration);
    select_limiting_dof_for_duration(otg, input, trajectory, sync_duration, &limiting_dof);

    if (calculate_velocity_phase_sync(input, trajectory, sync_duration, limiting_dof)) {
        trajectory->duration = sync_duration;
        trajectory->cumulative_times[0] = sync_duration;
        trajectory->valid = true;
        return RUCKIG_WORKING;
    }

    for (dof = 0; dof < input->dofs; ++dof) {
        ruckig_profile_t* profile = &trajectory->profiles[dof];
        const double a_min = input->has_min_acceleration ? input->min_acceleration[dof] : -input->max_acceleration[dof];
        const bool skip_time_sync = should_skip_time_synchronization(input, dof);
        const double own_duration = trajectory->independent_min_durations[dof];
        const double t_profile = sync_duration - profile->brake.duration - profile->accel.duration;

        if (!input->enabled[dof] || skip_time_sync || fabs(sync_duration - own_duration) < 2.0 * 2.2204460492503131e-16) {
            continue;
        }

        if (!ruckig_velocity_second_step2_get_profile(profile, t_profile, profile->v[0], profile->vf, input->max_acceleration[dof], a_min)) {
            return RUCKIG_ERROR_SYNCHRONIZATION_CALCULATION;
        }
    }

    trajectory->duration = sync_duration;
    trajectory->cumulative_times[0] = sync_duration;
    trajectory->valid = true;
    return RUCKIG_WORKING;
}

static ruckig_result_t calculate_velocity(
    const ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory
) {
    size_t dof;
    size_t limiting_dof = 0;
    double sync_duration = 0.0;

    for (dof = 0; dof < input->dofs; ++dof) {
        ruckig_profile_t* profile = &trajectory->profiles[dof];
        double duration = 0.0;
        const double a_min = input->has_min_acceleration ? input->min_acceleration[dof] : -input->max_acceleration[dof];
        const bool second_order = isinf(input->max_jerk[dof]);

        ruckig_profile_init(profile);
        ruckig_profile_set_boundary_for_velocity(
            profile,
            input->current_position[dof],
            input->current_velocity[dof],
            input->current_acceleration[dof],
            input->target_velocity[dof],
            input->target_acceleration[dof]
        );

        if (!input->enabled[dof]) {
            set_disabled_profile_state(profile, input, dof);
            trajectory->independent_min_durations[dof] = 0.0;
            continue;
        }

        if (second_order) {
            ruckig_brake_get_second_order_velocity_trajectory(&profile->brake);
            if (!ruckig_velocity_second_step1_get_profile(profile, profile, &duration, profile->v[0], profile->vf, input->max_acceleration[dof], a_min)) {
                return input->max_acceleration[dof] == 0.0 || a_min == 0.0 ? RUCKIG_ERROR_ZERO_LIMITS : RUCKIG_ERROR_EXECUTION_TIME_CALCULATION;
            }
        } else {
            ruckig_brake_get_velocity_trajectory(&profile->brake, input->current_acceleration[dof], input->max_acceleration[dof], a_min, input->max_jerk[dof]);
            ruckig_brake_finalize(&profile->brake, &profile->p[0], &profile->v[0], &profile->a[0]);
            if (!ruckig_velocity_third_step1_get_profile(profile, profile, &trajectory->blocks[dof], &duration, profile->v[0], profile->a[0], profile->vf, profile->af, input->max_acceleration[dof], a_min, input->max_jerk[dof])) {
                return input->max_jerk[dof] == 0.0 ? RUCKIG_ERROR_ZERO_LIMITS : RUCKIG_ERROR_EXECUTION_TIME_CALCULATION;
            }
        }

        trajectory->independent_min_durations[dof] = duration;
        if (duration > sync_duration) {
            sync_duration = duration;
            limiting_dof = dof;
        }
    }

    sync_duration = finalize_trajectory_duration(otg, input, sync_duration);
    select_limiting_dof_for_duration(otg, input, trajectory, sync_duration, &limiting_dof);
    sync_duration = adjust_duration_for_blocks(sync_duration, input, trajectory);

    if (calculate_velocity_phase_sync(input, trajectory, sync_duration, limiting_dof)) {
        trajectory->duration = sync_duration;
        trajectory->cumulative_times[0] = sync_duration;
        trajectory->valid = true;
        return RUCKIG_WORKING;
    }

    for (dof = 0; dof < input->dofs; ++dof) {
        ruckig_profile_t* profile = &trajectory->profiles[dof];
        const double a_min = input->has_min_acceleration ? input->min_acceleration[dof] : -input->max_acceleration[dof];
        const bool second_order = isinf(input->max_jerk[dof]);
        const bool skip_time_sync = should_skip_time_synchronization(input, dof);
        const double own_duration = trajectory->independent_min_durations[dof];
        const double t_profile = sync_duration - profile->brake.duration - profile->accel.duration;

        if (!input->enabled[dof] || skip_time_sync || fabs(sync_duration - own_duration) < 2.0 * 2.2204460492503131e-16) {
            continue;
        }

        if (second_order) {
            if (!ruckig_velocity_second_step2_get_profile(profile, t_profile, profile->v[0], profile->vf, input->max_acceleration[dof], a_min)) {
                return RUCKIG_ERROR_SYNCHRONIZATION_CALCULATION;
            }
        } else {
            if (select_block_profile_for_duration(profile, &trajectory->blocks[dof], sync_duration)) {
                continue;
            }
            if (!ruckig_velocity_third_step2_get_profile(profile, t_profile, profile->v[0], profile->a[0], profile->vf, profile->af, input->max_acceleration[dof], a_min, input->max_jerk[dof])) {
                return RUCKIG_ERROR_SYNCHRONIZATION_CALCULATION;
            }
        }
    }

    trajectory->duration = sync_duration;
    trajectory->cumulative_times[0] = sync_duration;
    trajectory->valid = true;
    return RUCKIG_WORKING;
}

static ruckig_result_t calculate_position(
    const ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory
) {
    size_t dof;
    size_t limiting_dof = 0;
    double sync_duration = 0.0;

    for (dof = 0; dof < input->dofs; ++dof) {
        ruckig_profile_t* profile = &trajectory->profiles[dof];
        double duration = 0.0;
        const double v_min = input->has_min_velocity ? input->min_velocity[dof] : -input->max_velocity[dof];
        const double a_min = input->has_min_acceleration ? input->min_acceleration[dof] : -input->max_acceleration[dof];
        const bool first_order = isinf(input->max_acceleration[dof]);
        const bool second_order = isinf(input->max_jerk[dof]);

        ruckig_profile_init(profile);
        ruckig_profile_set_boundary(
            profile,
            input->current_position[dof],
            input->current_velocity[dof],
            input->current_acceleration[dof],
            input->target_position[dof],
            input->target_velocity[dof],
            input->target_acceleration[dof]
        );

        if (!input->enabled[dof]) {
            set_disabled_profile_state(profile, input, dof);
            trajectory->independent_min_durations[dof] = 0.0;
            continue;
        }

        if (first_order) {
            if (!ruckig_position_first_step1_get_profile(profile, profile, &trajectory->blocks[dof], &duration, profile->p[0], profile->pf, input->max_velocity[dof], v_min)) {
                return input->max_velocity[dof] == 0.0 || v_min == 0.0 ? RUCKIG_ERROR_ZERO_LIMITS : RUCKIG_ERROR_EXECUTION_TIME_CALCULATION;
            }
        } else if (second_order) {
            ruckig_brake_get_second_order_position_trajectory(&profile->brake, input->current_velocity[dof], input->max_velocity[dof], v_min, input->max_acceleration[dof], a_min);
            ruckig_brake_finalize_second_order(&profile->brake, &profile->p[0], &profile->v[0], &profile->a[0]);
            if (!ruckig_position_second_step1_get_profile(profile, profile, &trajectory->blocks[dof], &duration, profile->p[0], profile->v[0], profile->pf, profile->vf, input->max_velocity[dof], v_min, input->max_acceleration[dof], a_min)) {
                return input->max_acceleration[dof] == 0.0 || a_min == 0.0 ? RUCKIG_ERROR_ZERO_LIMITS : RUCKIG_ERROR_EXECUTION_TIME_CALCULATION;
            }
        } else {
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
                    &duration,
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
        }

        trajectory->independent_min_durations[dof] = duration;
        if (duration > sync_duration) {
            sync_duration = duration;
            limiting_dof = dof;
        }
    }

    sync_duration = finalize_trajectory_duration(otg, input, sync_duration);
    select_limiting_dof_for_duration(otg, input, trajectory, sync_duration, &limiting_dof);
    sync_duration = adjust_duration_for_blocks(sync_duration, input, trajectory);

    if (calculate_position_phase_sync(input, trajectory, sync_duration, limiting_dof)) {
        trajectory->duration = sync_duration;
        trajectory->cumulative_times[0] = sync_duration;
        trajectory->valid = true;
        return RUCKIG_WORKING;
    }

    for (dof = 0; dof < input->dofs; ++dof) {
        ruckig_profile_t* profile = &trajectory->profiles[dof];
        const double v_min = input->has_min_velocity ? input->min_velocity[dof] : -input->max_velocity[dof];
        const double a_min = input->has_min_acceleration ? input->min_acceleration[dof] : -input->max_acceleration[dof];
        const bool first_order = isinf(input->max_acceleration[dof]);
        const bool second_order = isinf(input->max_jerk[dof]);
        const bool skip_time_sync = should_skip_time_synchronization(input, dof);
        const double own_duration = trajectory->independent_min_durations[dof];
        const double t_profile = sync_duration - profile->brake.duration - profile->accel.duration;

        if (!input->enabled[dof] || skip_time_sync || fabs(sync_duration - own_duration) < 2.0 * 2.2204460492503131e-16) {
            continue;
        }

        if (first_order) {
            if (!ruckig_position_first_step2_get_profile(profile, t_profile, profile->p[0], profile->pf, input->max_velocity[dof], v_min)) {
                return RUCKIG_ERROR_SYNCHRONIZATION_CALCULATION;
            }
        } else if (second_order) {
            if (select_block_profile_for_duration(profile, &trajectory->blocks[dof], sync_duration)) {
                continue;
            }
            if (!ruckig_position_second_step2_get_profile(profile, t_profile, profile->p[0], profile->v[0], profile->pf, profile->vf, input->max_velocity[dof], v_min, input->max_acceleration[dof], a_min)) {
                return RUCKIG_ERROR_SYNCHRONIZATION_CALCULATION;
            }
        } else {
            if (select_block_profile_for_duration(profile, &trajectory->blocks[dof], sync_duration)) {
                continue;
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
        }
    }

    trajectory->duration = sync_duration;
    trajectory->cumulative_times[0] = sync_duration;
    trajectory->valid = true;
    return RUCKIG_WORKING;
}

ruckig_result_t ruckig_create(ruckig_t** otg, size_t dofs, double delta_time) {
    ruckig_t* value;
    if (!otg || dofs == 0) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }

    *otg = NULL;
    value = (ruckig_t*)ruckig_calloc(1, sizeof(*value));
    if (!value) {
        return RUCKIG_ERROR;
    }

    value->dofs = dofs;
    value->delta_time = delta_time;
    if (ruckig_input_create(&value->current_input, dofs) != RUCKIG_WORKING) {
        ruckig_destroy(value);
        return RUCKIG_ERROR;
    }

    *otg = value;
    return RUCKIG_WORKING;
}

void ruckig_destroy(ruckig_t* otg) {
    if (!otg) {
        return;
    }
    ruckig_input_destroy(otg->current_input);
    ruckig_free(otg);
}

ruckig_result_t ruckig_calculate(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory
) {
    if (!otg || !input || !trajectory || otg->dofs != input->dofs || trajectory->dofs != otg->dofs) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    trajectory->valid = false;
    if (ruckig_validate_input(otg, input, false, true) != RUCKIG_WORKING) {
        return RUCKIG_ERROR_INVALID_INPUT;
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

ruckig_result_t ruckig_update(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_output_t* output
) {
    const double calculation_start = calculation_duration_start();
    if (!otg || !input || !output || otg->dofs != input->dofs || output->dofs != otg->dofs) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    output->new_calculation = false;
    output->was_calculation_interrupted = false;
    if (!output->trajectory->valid || !otg->current_input_initialized || !ruckig_input_equals(input, otg->current_input)) {
        ruckig_result_t result = ruckig_calculate(otg, input, output->trajectory);
        if (result != RUCKIG_WORKING) {
            output->calculation_duration = calculation_duration_finish(calculation_start);
            return result;
        }
        ruckig_input_copy_state(input, otg->current_input);
        otg->current_input_initialized = true;
        output->time = 0.0;
        output->new_calculation = true;
    }

    output->time += otg->delta_time;
    {
        const size_t old_section = output->new_section;
        ruckig_result_t sample_result = ruckig_trajectory_at_time(
            output->trajectory,
            output->time,
            output->new_position,
            output->new_velocity,
            output->new_acceleration,
            output->new_jerk,
            &output->new_section
        );
        if (sample_result != RUCKIG_WORKING) {
            output->calculation_duration = calculation_duration_finish(calculation_start);
            return sample_result;
        }
        output->did_section_change = output->new_section > old_section;
    }
    ruckig_output_pass_to_input(output, otg->current_input);
    output->calculation_duration = calculation_duration_finish(calculation_start);

    return output->time > output->trajectory->duration ? RUCKIG_FINISHED : RUCKIG_WORKING;
}

void ruckig_reset(ruckig_t* otg) {
    if (otg) {
        otg->current_input_initialized = false;
    }
}
